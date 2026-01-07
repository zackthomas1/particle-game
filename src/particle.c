#include "pch.h"
#include "particle.h"

#include "hash.h"

ParticleProps defaultParticleProps = {
    0.5f,                   // varaince
    100.0f,                 // lifetime
    { -100.0f, 0.0f },      // velocity
    10.0f,                  // mass
};

// Set up vertex data - unit square (±0.5), scaled in shader
static const float quadVertices[] = {
    // positions    // texCoords
    -0.5f,  0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 0.0f,
     0.5f, -0.5f, 1.0f, 0.0f,

    -0.5f,  0.5f, 0.0f, 1.0f,
     0.5f, -0.5f, 1.0f, 0.0f,
     0.5f,  0.5f, 1.0f, 1.0f,
};
static uint32_t quadVAO, quadVBO, instancePositionVBO, shaderId; //particle render state

static ParticlePool* ConstructParticlePool_() 
{
    ParticlePool *particles = (ParticlePool*)malloc(sizeof(ParticlePool));
    PASSERT(particles, LOG_FATAL, "Failed to allocate particle particles");
    if(!particles) { return NULL; }

    particles->activeCount = 0;

    for (int i = 0; i < MAX_PARTICLE_COUNT; i++) 
    {
        particles->pLifetimes[i]  = 0.0f;
        particles->pLifespans[i]  = 0.0f;

        particles->pPrevPositions[i] = (Vector2){ 0 };
        particles->pPositions[i]     = (Vector2){ 0 };
        particles->pVelocities[i]    = (Vector2){ 0 };

        particles->pMasses[i]  = 0.0f;
    }
    return particles;
}

static void DestructParticlePool_(ParticlePool *particles) 
{
    free(particles);
}

static void SwapParticles_(ParticlePool *particles, size_t i, size_t j)
{
    particles->pLifetimes[i]      = particles->pLifetimes[j];
    particles->pLifespans[i]      = particles->pLifespans[j];

    particles->pPrevPositions[i] = particles->pPrevPositions[j];
    particles->pPositions[i]     = particles->pPositions[j];
    particles->pVelocities[i]    = particles->pVelocities[j];

    particles->pMasses[i]        = particles->pMasses[j];
}

static void KillParticle_(ParticlePool *particles, size_t index) 
{
    particles->activeCount--;
    SwapParticles_(particles, index, particles->activeCount);
}

void ProjectSelfCollision(const Constraint *this, ParticlePool *particles, float deltaTime)
{
    PASSERTRETURN(this->participantCount == 2, LOG_WARNING, 
        "Incorrect number of participants in self collision constraint. Constraint participants must equal 2.");

    const size_t i = this->participants[0], j = this->participants[1];
    const Vector2 pi = particles->pPositions[i], pj = particles->pPositions[j];

    const Vector2 seperation    = Vector2Subtract(pj, pi);
    float distance              = Vector2Length(seperation);
    
    // Guard against degenerate case where particles are at the same position
    // Use a random direction to separate them
    Vector2 gradientC;
    const float minDistance = 1e-3;
    if (distance < minDistance)
    {
        // Generate a pseudo-random direction based on particle indices
        float angle = (float)(i * 73856093 ^ j * 19349663) * 0.0001f;
        gradientC = (Vector2){ cosf(angle), sinf(angle) };
        distance = minDistance; // Treat as minimum distance for constraint calculation
    } else
    {
        gradientC = Vector2Normalize(seperation);
    }
    
    const float restLength      = 2.0f * PARTICLE_RADIUS;
    const float constraintEval  = (distance - restLength);
    const float iInvMass        = 1.0f / particles->pMasses[i], jInvMass = 1.0f / particles->pMasses[j];
    
    float lambda = constraintEval / (iInvMass + jInvMass);
    
    // Clamp maximum displacement to prevent instability
    // Maximum displacement per iteration should not exceed particle radius / substeps
    const float maxDisplacement = PARTICLE_RADIUS / (float)PHYSICS_SUBSTEPS;
    const float maxLambda = maxDisplacement / fmaxf(iInvMass, jInvMass);
    lambda = Clamp(lambda, -maxLambda, maxLambda);

    Vector2 deltaPi = Vector2Scale( gradientC, (lambda * iInvMass));
    Vector2 deltaPj = Vector2Scale( gradientC, (-1.0f * lambda * jInvMass));

    particles->pPositions[i] = Vector2Add(pi, deltaPi);
    particles->pPositions[j] = Vector2Add(pj, deltaPj);
}

void ProjectSurfaceCollision(const Constraint *this, ParticlePool *particles, float deltaTime)
{
    PASSERTRETURN(this->participantCount == 1, LOG_WARNING,
        "Incorrect number of participants in self collision constraint. Constraint participants must equal 1.");
    const size_t i = this->participants[0];
    const Vector2 pi = particles->pPositions[i];

    Vector2 deltaPi = Vector2Scale(this->surfaceNormal, -1.0f * Vector2DotProduct(Vector2Subtract(pi, this->entryPoint), this->surfaceNormal));
    particles->pPositions[i] = Vector2Add(pi, deltaPi);
}

void ProjectDistance(const Constraint *this, ParticlePool *particles, float deltaTime)
{
    PASSERT(false, LOG_WARNING, "ProjectDistance function not implemented");
}

static Vector2 CalculateForces_(Vector2 pi, Vector2 vi, float mi, const Force *forces)
{
    Vector2 externalForces = (Vector2){ 0 };

    for(size_t j = 0; j < arrlenu(forces); j++){
        switch (forces[j].type)
        {
        case FORCE_GRAVITY:
            externalForces = Vector2Add(externalForces,
                Vector2Scale((Vector2){0.0, GRAVITIONAL_CONST}, mi));
            break;
        case FORCE_VISCOUS:
            externalForces = Vector2Add(externalForces,
                Vector2Scale(vi, (-6.0f * PI * forces[j].viscosity * PARTICLE_RADIUS)));
            break;
        case FORCE_ATTRACT:
        case FORCE_REPULSE:
            Vector2 forceDirection = Vector2Normalize(Vector2Subtract(forces[j].position, pi));
            const float distanceSqr = Vector2DistanceSqr(forces[j].position, pi);
            float strength = (mi * forces[j].mass) / distanceSqr;
            if(forces[j].type == FORCE_REPULSE) { strength *= -1.0; }
            externalForces = Vector2Add(externalForces, 
                                Vector2Scale(forceDirection, strength));
            break;
        default:
            break;
        }
    }

    PASSERT(isfinite(externalForces.x) && isfinite(externalForces.y), LOG_ERROR, 
        "externalForces invalid.");
    return externalForces;
}

static Vector2 CalculateEntryPoint_(Vector2 position, Vector2 velocity, Vector2 surfacePoint, Vector2 surfaceNormal)
{
    // t = ((surfacePoint - position) . surfaceNormal) / (velocity . surfaceNormal)
    const float t = Vector2DotProduct(Vector2Subtract(surfacePoint, position), surfaceNormal) / 
                        Vector2DotProduct(velocity, surfaceNormal);
    
    PASSERT(isfinite(t), LOG_ERROR, 
        "Entry point invalid because t = inf. |surfacePoint - position| = %.2f", Vector2Distance(surfacePoint, position));

    // position + (t*velocity)
    const Vector2 ep = Vector2Add(position, Vector2Scale(velocity, t));
    return ep;
}

static size_t GenerateCollisionConstraints_(ParticleSystem *system)
{
    size_t collisionCount = 0;

    // Check for particle self collision
    const float range = 2.0f * PARTICLE_RADIUS;
    const float collisionGracePeriod = 0.05f; // Skip collision for newly spawned particles
    
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        // Skip collision detection for particles in grace period
        if (system->particles_->pLifespans[i] < collisionGracePeriod) { continue; }
        
        QueryHashPoint(system->spatialHash, system->particles_->pPositions[i], 2.0f * PARTICLE_RADIUS);
        for (size_t j = 0; j < arrlenu(system->spatialHash->queryResults); j++)
        {
            const size_t pj = system->spatialHash->queryResults[j];
            // Only process pair once (i < pj) to avoid duplicate constraints
            if ( i >= pj) { continue; }
            // Skip collision if the other particle is also in grace period
            if (system->particles_->pLifespans[pj] < collisionGracePeriod) { continue; }
            
            const float dist = Vector2Distance(system->particles_->pPositions[i], system->particles_->pPositions[pj]);
            // Include degenerate case (dist near 0) - ProjectSelfCollision now handles it
            if ( dist < range)
            {
                AddSelfCollisionConstraint(system, i, pj);
                collisionCount++;
            }
        }
    }
    return collisionCount;
}

static void HandleBoundaryCollisions_(ParticleSystem *system)
{
    ParticlePool *particles = system->particles_;
    const float restitution = 0.8;
    const float friction = 0.1;
    for (size_t i = 0; i < particles->activeCount; i++)
    {
        Vector2 *pos = &particles->pPositions[i];
        Vector2 *vel = &particles->pVelocities[i];
        // Check X boundaries
        if (pos->x < system->boundaryBox.left + PARTICLE_RADIUS)
        {
            pos->x = system->boundaryBox.left + PARTICLE_RADIUS;
            vel->x *= -restitution;
            vel->y *= (1.0f - friction);
        } else if (system->boundaryBox.right - PARTICLE_RADIUS < pos->x)
        {
            pos->x = system->boundaryBox.right - PARTICLE_RADIUS;
            vel->x *= -restitution;
            vel->y *= (1.0f - friction);
        }

        // Check Y boundaries (separate from X)
        if (pos->y < system->boundaryBox.bottom + PARTICLE_RADIUS)
        {
            pos->y = system->boundaryBox.bottom + PARTICLE_RADIUS;
            vel->x *= (1.0f - friction);
            vel->y *= -restitution;
        } else if (system->boundaryBox.top - PARTICLE_RADIUS < pos->y)
        {
            pos->y = system->boundaryBox.top - PARTICLE_RADIUS;
            vel->x *= (1.0f - friction);
            vel->y *= -restitution;
        }
    }
}

static void UpdateParticlesLife_(ParticleSystem *system, float deltaTime)
{
    // Update lifespan of particles and deactivate/kill any particles whose
    // lifespan has exceeded its lifetime.
    size_t deadCount = 0;
    for (size_t i = 0; i < system->particles_->activeCount; i++) 
    {
        system->particles_->pLifespans[i] += deltaTime;
        if (system->particles_->pLifespans[i] > system->particles_->pLifetimes[i])
        { 
            deadCount++;
            SwapParticles_(system->particles_, i, (system->particles_->activeCount - deadCount));
        }
    }
    system->particles_->activeCount -= deadCount;
}

static void UpdateParticleAttributes_(ParticleSystem *system)
{
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        const float t = (system->particles_->pLifespans[i] / system->particles_->pLifetimes[i]);
    }
}

static void UpdateParticlesMotion_(ParticleSystem *system, float deltaTime)
{
    // Initial particle position estimate
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        const float inverseMass = 1.0f / system->particles_->pMasses[i];
        const Vector2 externalForces = CalculateForces_(system->particles_->pPositions[i],
            system->particles_->pVelocities[i],
            system->particles_->pMasses[i],
            system->forces_);
        const Vector2 deltaV = Vector2Scale(externalForces, (deltaTime * inverseMass));

        system->particles_->pVelocities[i]  = Vector2Add(system->particles_->pVelocities[i], deltaV);
        system->particles_->pPrevPositions[i] = system->particles_->pPositions[i];
        system->particles_->pPositions[i] = Vector2Add(system->particles_->pPositions[i], 
            Vector2Scale(system->particles_->pVelocities[i], deltaTime));
    }

    // Construct Spatial hash map of current particle positions.
    ClearHash(system->spatialHash);
    FillHash(system->spatialHash, system->particles_);

    // Generate self collision constraints
    size_t collisionCount = GenerateCollisionConstraints_(system);

    // Project constraints (solver)
    for (size_t i = 0; i < arrlenu(system->constraints_); i++)
    {
        const Constraint c = system->constraints_[i];
        c.ProjectFn(&c, system->particles_, deltaTime);
    }

    // Remove collision constraints
    arrsetlen(system->constraints_, (arrlen(system->constraints_) - collisionCount));
    PASSERT((arrlen(system->constraints_) >= 0), LOG_ERROR, "");

    // Update velocities after constraint solver
    const float maxVelocity = 1000.0f; // Maximum velocity magnitude in pixels/second
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        system->particles_->pVelocities[i] = Vector2Scale(
            Vector2Subtract(system->particles_->pPositions[i], system->particles_->pPrevPositions[i]), 
                (1.0f / deltaTime));
    }

    HandleBoundaryCollisions_(system);
}

ParticleSystem* ConstructParticleSystem(uint32_t left, uint32_t right, uint32_t bottom, uint32_t top)
{
    ParticleSystem* system = (ParticleSystem*)malloc(sizeof(ParticleSystem));
    PASSERT(system, LOG_FATAL, "Failed to allocate particle pool");
    if(!system) { return NULL; }

    system->boundaryBox.left = left;
    system->boundaryBox.right = right;
    system->boundaryBox.bottom = bottom;
    system->boundaryBox.top = top;
    system->spatialHash = ConstructHash(2.0f * PARTICLE_RADIUS);

    system->emitter.position    = (Vector2){ 0 };
    system->emitter.radius      = EMITTER_RADIUS;
    
    system->constraints_    = NULL;
    system->forces_         = NULL;
    system->particles_ = ConstructParticlePool_();

    return system;
}

void DestructParticleSystem(ParticleSystem *system)
{
    DestructHash(system->spatialHash);
    arrfree(system->constraints_);
    arrfree(system->forces_);
    DestructParticlePool_(system->particles_);
    free(system);
}

void EmitParticles(ParticleSystem *system, const ParticleProps *props, uint32_t count)
{
    for (size_t c = 0; c < count; c++)
    {
        size_t i = system->particles_->activeCount;
        PASSERTRETURN(i < MAX_PARTICLE_COUNT, LOG_WARNING, "active particle count exceeds MAX_PARTICLE_COUNT");

        system->particles_->activeCount += 1;

        PASSERT((props->variance > -EPSILON && props->variance < (1.0 + EPSILON)),
            LOG_WARNING, "variance value outside valid range [0.0, 1.0]. Clamping value to valid range.");
        const float variance = Clamp(props->variance, 0.0f, 1.0f);

        system->particles_->pLifetimes[i]    = props->lifetime + (props->lifetime * (GetRandomValueF() * variance));
        system->particles_->pLifespans[i]    = 0;

        system->particles_->pPositions[i]    = Vector2Add(system->emitter.position,
                Vector2Scale((Vector2){ GetRandomValueF(), GetRandomValueF() }, (system->emitter.radius * variance)));
        system->particles_->pVelocities[i]   = Vector2Add(props->velocity,
                                                Vector2Scale(props->velocity, GetRandomValueF() * variance));
        system->particles_->pMasses[i]       = props->mass;
    }
}

void UpdateParticles(ParticleSystem *system, float deltaTime)
{
    PASSERTRETURN((deltaTime > EPSILON), LOG_WARNING, "delta equal to zero. Skipping update step");

    UpdateParticlesLife_(system, deltaTime);
    UpdateParticleAttributes_(system);

    const int substeps = PHYSICS_SUBSTEPS;
    const float deltaTimeSubstep = deltaTime / (float)substeps;
    for(size_t i = 0; i < substeps; i++)
    {
        UpdateParticlesMotion_(system, deltaTimeSubstep);
    }
}

void InitParticleRender(const Shader *shader, float screenWidth, float screenHeight)
{
    shaderId = shader->id;
    quadVAO = rlLoadVertexArray();
    rlEnableVertexArray(quadVAO);
    quadVBO = rlLoadVertexBuffer(&quadVertices, sizeof(quadVertices), false);
    // aCoord
    rlEnableVertexAttribute(0);
    rlSetVertexAttribute(0, 2, RL_FLOAT, false, 4 * sizeof(float), 0);
    // aTexCoord
    rlEnableVertexAttribute(1);
    rlSetVertexAttribute(1, 2, RL_FLOAT, false, 4 * sizeof(float), 2 * sizeof(float));
    //  aPosition
    instancePositionVBO = rlLoadVertexBuffer(NULL, MAX_PARTICLE_COUNT * sizeof(Vector2), true);    // dynamic = true
    rlEnableVertexAttribute(2);
    rlSetVertexAttribute(2, 2, RL_FLOAT, false, 2 * sizeof(float), 0);
    rlSetVertexAttributeDivisor(2,1);

    rlDisableVertexBuffer();
    rlDisableVertexArray();

    float radius = PARTICLE_RADIUS;

    rlEnableShader(shader->id);
    rlSetUniform(GetShaderLocation(*shader, "uScreenWidth"), &screenWidth, RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(GetShaderLocation(*shader, "uScreenHeight"), &screenHeight, RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(GetShaderLocation(*shader, "uRadius"), &radius, RL_SHADER_UNIFORM_FLOAT, 1);
    rlDisableShader();
}

void ShutdownParticleRender()
{
    rlUnloadVertexArray(quadVAO);
    rlUnloadVertexBuffer(quadVBO);
    rlUnloadVertexBuffer(instancePositionVBO);

    quadVAO = 0;
    quadVBO = 0;
    instancePositionVBO = 0;
}

void DrawParticlesInstanced(const ParticleSystem *system)
{
    rlEnableShader(shaderId);
    rlEnableVertexArray(quadVAO);

    rlUpdateVertexBuffer(instancePositionVBO, 
        system->particles_->pPositions,
        system->particles_->activeCount * sizeof(Vector2),
        0);
    rlDrawVertexArrayInstanced(0, 6, system->particles_->activeCount);
    
    rlDisableVertexArray();
    rlDisableShader();
}

void DrawParticlesPoints(const ParticleSystem *system)
{
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        DrawPixelV(system->particles_->pPositions[i],
        RED);
    }
}

void DrawForces(const ParticleSystem *system)
{
    for (size_t i = 0; i < arrlenu(system->forces_); i++)
    {
        switch (system->forces_[i].type)
        {
        case FORCE_GRAVITY:
        DrawCircleV((Vector2){0.0f, 0.0f}, 8.0f, GREEN);
        break;
        case FORCE_VISCOUS:
        DrawCircleV((Vector2){0.0f, 0.0f}, 8.0f, BLUE);
            break;
        case FORCE_ATTRACT:
        case FORCE_REPULSE:
        DrawCircleV(system->forces_[i].position, 8.0f, YELLOW);
            break;
        default:
            break;
        }
    }
}

void AddSelfCollisionConstraint(ParticleSystem *system, size_t i, size_t j)
{
    Constraint c = { 0 };
    c.type = CONSTRAINT_SELF_COLLISION;
    c.participants[0] = i;
    c.participants[1] = j;
    c.participantCount = 2;
    c.ProjectFn = ProjectSelfCollision;

    arrput(system->constraints_, c);
}

void AddSurfaceCollisionConstraint(ParticleSystem *system, size_t i, Vector2 sn, Vector2 ep)
{
    Constraint c = { 0 };
    c.type = CONSTRAINT_SURFACE_COLLISION;
    c.participants[0] = i;
    c.participantCount = 1;
    c.ProjectFn = ProjectSurfaceCollision;

    c.surfaceNormal = sn;
    c.entryPoint = ep;
    
    arrput(system->constraints_, c);
}

void AddDistanceConstraint(ParticleSystem *system, size_t i, size_t j)
{
    Constraint c = { 0 };
    c.type = CONSTRAINT_DISTANCE;
    c.participants[0] = i;
    c.participants[1] = j;
    c.participantCount = 2;
    c.ProjectFn = ProjectDistance;

    arrput(system->constraints_, c);
}

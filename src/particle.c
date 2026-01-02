#include "pch.h"
#include "particle.h"

#include "hash.h"

ParticleProps defaultParticleProps = {
    0.5f,                   // varaince
    100.0f,                  // lifetime
    { -500.0f, 0.0f },        // velocity
    10.0f,                  // mass
    { 230, 41, 55, 255 },   // birthColor
    { 255, 161, 0, 0 },     // deathColor
};

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

        particles->pBirthColors[i]    = (Color){ 0 };
        particles->pDeathColors[i]    = (Color){ 0 };
        particles->pColors[i]  = (Color){ 0 };
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

    particles->pBirthColors[i]   = particles->pBirthColors[j];
    particles->pDeathColors[i]   = particles->pDeathColors[j];
    particles->pColors[i]        = particles->pColors[j];
}

static void KillParticle_(ParticlePool *particles, size_t index) 
{
    particles->activeCount--;
    SwapParticles_(particles, index, particles->activeCount);
}

void ProjectSelfCollision(const Constraint *this, ParticlePool *particles)
{
    PASSERT(this->participantCount == 2, LOG_WARNING, 
        "Incorrect number of participants in self collision constraint. Constraint participants must equal 2.");
    if (!(this->participantCount == 2)) { return; }

    const size_t i = this->participants[0], j = this->participants[1];
    const Vector2 pi = particles->pPositions[i], pj = particles->pPositions[j];

    const Vector2 seperation    = Vector2Subtract(pj, pi);
    const Vector2 gradientC     = Vector2Normalize(seperation);
    const float distance        = Vector2Length(seperation);
    const float restLength      = 2.0f * PARTICLE_RADIUS;
    const float constraintEval  = (distance - restLength);
    const float iInvMass        = 1.0f / particles->pMasses[i], jInvMass = 1.0f / particles->pMasses[j];
    
    const float lambda = constraintEval / (iInvMass + jInvMass);

    Vector2 deltaPi = Vector2Scale( gradientC, (lambda * iInvMass));
    Vector2 deltaPj = Vector2Scale( gradientC, (-1.0f * lambda * jInvMass));

    particles->pPositions[i] = Vector2Add(pi, deltaPi);
    particles->pPositions[j] = Vector2Add(pj, deltaPj);
}

void ProjectSurfaceCollision(const Constraint *this, ParticlePool *particles)
{
    PASSERT(this->participantCount == 1, LOG_WARNING,
        "Incorrect number of participants in self collision constraint. Constraint participants must equal 1.");
    if (!(this->participantCount == 1)) { return; }

    const size_t i = this->participants[0];
    const Vector2 pi = particles->pPositions[i];

    Vector2 deltaPi = Vector2Scale(this->surfaceNormal, -1.0f * Vector2DotProduct(Vector2Subtract(pi, this->entryPoint), this->surfaceNormal));
    particles->pPositions[i] = Vector2Add(pi, deltaPi);
}

void ProjectDistance(const Constraint *this, ParticlePool *particles)
{
    PASSERT(false, LOG_WARNING, "ProjectDistance function not implemented");

    PASSERT(this->participantCount == 2, LOG_ERROR, 
        "Incorrect number of participants in self collision constraint. Constraint participants must equal 2.");
    if (!(this->participantCount == 2)) { return; }
}

static Vector2 CalculateAcceleration_(Vector2 position, float mass, const Force *forces)
{
    Vector2 pAcceleration = (Vector2){ 0 };
    const float invMass = 1.0f / mass;

    for(size_t j = 0; j < arrlenu(forces); j++){
        switch (forces[j].type)
        {
        case FORCE_DIRECTION:
            pAcceleration = Vector2Add(pAcceleration, Vector2Scale(forces[j].direction, invMass));
            break;
        case FORCE_POINT:
            Vector2 forceDirection = Vector2Normalize(Vector2Subtract(forces[j].position, 
                                                    position));
            // float invDistanceSqr = 1.0f / Vector2DistanceSqr(system->forces_[j].position, 
            //                                      position);
            pAcceleration = Vector2Add(pAcceleration, 
                                Vector2Scale(forceDirection, 
                                    (invMass * forces[j].strength)));
            break;
        case FORCE_GRAVITY:
            pAcceleration = Vector2Add(pAcceleration, (Vector2){0.0, GRAVITY_CONST});
            break;
        default:
            break;
        }
    }
    return pAcceleration;
}

static Vector2 CalculateEntryPoint_(const Vector2 P, const Vector2 v, const Vector2 Q, const Vector2 sn)
{
    // t = ((Q - P) . sn) / (v . sn)
    const float t = Vector2DotProduct(Vector2Subtract(Q, P), sn) / Vector2DotProduct(v, sn);
    // P + (t*v)
    return Vector2Add(P, Vector2Scale(v, t));
}

static size_t GenerateCollisionConstraints_(ParticleSystem *system)
{
    size_t collisionCount = 0;
    Vector2 P, v, Q, sn, EP;
    const float range = 2.0f * PARTICLE_RADIUS;

    // left-wall (vertical)
    QueryHashRange(system->spatialHash,
        system->boundaryBox.left - PARTICLE_RADIUS, system->boundaryBox.left + PARTICLE_RADIUS,
        system->boundaryBox.top, system->boundaryBox.bottom);
    for (size_t i = 0; i < arrlenu(system->spatialHash->queryResults); i++)
    {
        size_t pi = system->spatialHash->queryResults[i];
        P = system->particles_->pPositions[pi];

        if(!(P.x < system->boundaryBox.left + PARTICLE_RADIUS)) { continue; }

        v = system->particles_->pVelocities[pi];
        Q = (Vector2){system->boundaryBox.left + PARTICLE_RADIUS, P.y };
        sn = (Vector2){ 1.0f, 0.0f };
        EP = CalculateEntryPoint_(P, v, Q, sn);
        AddSurfaceCollisionConstraint(system, pi, sn, EP);
        collisionCount++;
    }

    // right-wall (vertical)
    QueryHashRange(system->spatialHash, 
        system->boundaryBox.right - PARTICLE_RADIUS, system->boundaryBox.right + PARTICLE_RADIUS, 
        system->boundaryBox.top, system->boundaryBox.bottom);
    for (size_t i = 0; i < arrlenu(system->spatialHash->queryResults); i++)
    {
        size_t pi = system->spatialHash->queryResults[i];
        P = system->particles_->pPositions[pi];

        if(!(P.x > (system->boundaryBox.right - PARTICLE_RADIUS))) { continue; }

        v = system->particles_->pVelocities[pi];
        Q = (Vector2){system->boundaryBox.right - PARTICLE_RADIUS, P.y };
        sn = (Vector2){ -1.0f, 0.0f };
        EP = CalculateEntryPoint_(P, v, Q, sn);
        AddSurfaceCollisionConstraint(system, pi, sn, EP);
        collisionCount++;
    }

    // top-wall (horizontal)
    QueryHashRange(system->spatialHash, 
        system->boundaryBox.left, system->boundaryBox.right,
        system->boundaryBox.top - PARTICLE_RADIUS, system->boundaryBox.top + PARTICLE_RADIUS);
    for (size_t i = 0; i < arrlenu(system->spatialHash->queryResults); i++)
    {
        size_t pi = system->spatialHash->queryResults[i];
        P = system->particles_->pPositions[pi];

        if(!(P.y < (system->boundaryBox.top + PARTICLE_RADIUS))) { continue; }

        v = system->particles_->pVelocities[pi];
        Q = (Vector2){P.x, system->boundaryBox.top + PARTICLE_RADIUS };
        sn = (Vector2){ 0.0f, 1.0f };
        EP = CalculateEntryPoint_(P, v, Q, sn);
        AddSurfaceCollisionConstraint(system, pi, sn, EP);
        collisionCount++;
    }

    // bottom-wall (horizontal)
    QueryHashRange(system->spatialHash, 
        system->boundaryBox.left, system->boundaryBox.right,
        system->boundaryBox.bottom - PARTICLE_RADIUS, system->boundaryBox.bottom + PARTICLE_RADIUS);
    for (size_t i = 0; i < arrlenu(system->spatialHash->queryResults); i++)
    {
        size_t pi = system->spatialHash->queryResults[i];
        P = system->particles_->pPositions[pi];

        if(!(P.y > system->boundaryBox.bottom - PARTICLE_RADIUS)) { continue; }

        v = system->particles_->pVelocities[pi];
        Q = (Vector2){P.x, system->boundaryBox.bottom - PARTICLE_RADIUS };
        sn = (Vector2){ 0.0f, -1.0f };
        EP = CalculateEntryPoint_(P, v, Q, sn);
        AddSurfaceCollisionConstraint(system, pi, sn, EP);
        collisionCount++;
    }

    // Check for particle self collision
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        QueryHashPoint(system->spatialHash, system->particles_->pPositions[i], 2.0f * PARTICLE_RADIUS);
        for (size_t j = 0; j < arrlenu(system->spatialHash->queryResults); j++)
        {
            size_t pj = system->spatialHash->queryResults[j];
            if ( i == pj) { continue; }
            if (Vector2Distance(system->particles_->pPositions[i], system->particles_->pPositions[pj]) < range)
            {
                AddSelfCollisionConstraint(system, i, pj);
                collisionCount++;
            }
        }
    }

    return collisionCount;
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

static void UpdateParticlesMotion_(ParticleSystem *system, float deltaTime)
{
    // Initial particle position estimate
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        Vector2 pAcceleration = CalculateAcceleration_(system->particles_->pPositions[i], 
                                    system->particles_->pMasses[i], 
                                    system->forces_);

        system->particles_->pVelocities[i]  = Vector2Add(system->particles_->pVelocities[i], 
                                            Vector2Scale(pAcceleration, deltaTime));
        system->particles_->pPrevPositions[i] = system->particles_->pPositions[i];
        system->particles_->pPositions[i]   = Vector2Add(system->particles_->pPositions[i], 
                                            Vector2Scale(system->particles_->pVelocities[i], deltaTime));
    }

    ClearHash(system->spatialHash);
    FillHash(system->spatialHash, system->particles_);

    // Generate self collision constraints
    size_t collisionCount = GenerateCollisionConstraints_(system);

    // Project constraints (solver)
    for (size_t i = 0; i < arrlenu(system->constraints_); i++)
    {
        const Constraint c = system->constraints_[i];
        c.ProjectFn(&c, system->particles_);
    }

    // Remove collision constraints
    arrsetlen(system->constraints_, (arrlen(system->constraints_) - collisionCount));
    PASSERT((arrlen(system->constraints_) >= 0), LOG_ERROR, "");

    // Update velocities after constraint solver
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        system->particles_->pVelocities[i] = Vector2Scale(
            Vector2Subtract(system->particles_->pPositions[i], system->particles_->pPrevPositions[i]), 
                (1.0f / deltaTime));
    }
}

static void UpdateParticleAttributes_(ParticleSystem *system)
{
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        const float t = (system->particles_->pLifespans[i] / system->particles_->pLifetimes[i]);

        system->particles_->pColors[i]  = ColorLerp( system->particles_->pBirthColors[i],
                                system->particles_->pDeathColors[i], t);
    }
}

ParticleSystem* ConstructParticleSystem(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
{
    ParticleSystem* system = (ParticleSystem*)malloc(sizeof(ParticleSystem));
    PASSERT(system, LOG_FATAL, "Failed to allocate particle pool");
    if(!system) { return NULL; }

    system->boundaryBox.left = left;
    system->boundaryBox.right = right;
    system->boundaryBox.top = top;
    system->boundaryBox.bottom = bottom;
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
    arrfree(system->constraints_);
    arrfree(system->forces_);
    DestructParticlePool_(system->particles_);
    free(system);
}

void EmitParticle(ParticleSystem *system, const Vector2 position, const ParticleProps *props) 
{
    size_t i = system->particles_->activeCount;
    PASSERT(i < MAX_PARTICLE_COUNT, LOG_WARNING, "active particle count exceeds MAX_PARTICLE_COUNT")
    if(!(i < MAX_PARTICLE_COUNT)) { return; }

    system->particles_->activeCount += 1;

    PASSERT((props->variance > -EPSILON && props->variance < (1.0 + EPSILON)),
        LOG_WARNING, "variance value outside valid range [0.0, 1.0]. Clamping value to valid range.")
    const float variance = Clamp(props->variance, 0.0f, 1.0f);
    const float randomScalar = GetRandomValueF();

    system->particles_->pLifetimes[i]    = props->lifetime + (props->lifetime * (GetRandomValueF() * variance));
    system->particles_->pLifespans[i]    = 0;

    system->particles_->pPositions[i]    = position;
    system->particles_->pVelocities[i]   = Vector2Add(props->velocity,
                                            Vector2Scale(props->velocity, randomScalar * variance));
    system->particles_->pMasses[i]       = props->mass;

    system->particles_->pBirthColors[i]  = props->birthColor;
    system->particles_->pDeathColors[i]  = props->deathColor;
    system->particles_->pColors[i]       = system->particles_->pBirthColors[i];
}

void UpdateParticles(ParticleSystem *system, float deltaTime)
{
    PASSERT((deltaTime > EPSILON), LOG_WARNING, "delta equal to zero. Skipping update step");
    if(!(deltaTime > EPSILON)) { return; }

    UpdateParticlesLife_(system, deltaTime);
    UpdateParticleAttributes_(system);

    const int substeps = 4;
    const float deltaTimeSubstep = deltaTime / (float)substeps;
    for(size_t i = 0; i < substeps; i++)
    {
        UpdateParticlesMotion_(system, deltaTimeSubstep);
    }
}

void DrawParticles(const ParticleSystem *system)
{
    for (size_t i = 0; i < system->particles_->activeCount; i++)
    {
        DrawCircleV(system->particles_->pPositions[i], 
            PARTICLE_RADIUS, system->particles_->pColors[i]);
    }
}

void DrawForces(const ParticleSystem *system)
{
    for (size_t i = 0; i < arrlenu(system->forces_); i++)
    {
        switch (system->forces_[i].type)
        {
        case FORCE_DIRECTION:
        DrawCircleV(system->forces_[i].direction, 4.0f, YELLOW);
        break;
        case FORCE_POINT:
        DrawCircleV(system->forces_[i].position, 4.0f, YELLOW);
            break;
        case FORCE_GRAVITY:
        DrawCircleV(system->forces_[i].position, 4.0f, YELLOW);
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

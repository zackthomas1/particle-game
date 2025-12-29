#include "pch.h"
#include "particle.h"

ParticleProps defaultParticleProps = {
    0.5f,                   // varaince
    100.0f,                 // lifetime
    { 0.0f, 0.0f },         // position
    { 10.0f, 0.0f },         // velocity
    10.0f,                  // birthMass
    0.1f,                   // deathMass
    { 230, 41, 55, 255 },   // birthColor
    { 255, 161, 0, 0 },     // deathColor
};

void ProjectSelfCollision(const Constraint *this, ParticlePool *particles)
{
    PASSERT(arrlenu(this->participants) == 2, LOG_WARNING, 
        "Incorrect number of participants in self collision constraint. Constraint participants must equal 2.");
    if (!(arrlen(this->participants) == 2)) { return; }

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

void ProjectCollision(const Constraint *this, ParticlePool *particles)
{
    PASSERT(arrlenu(this->participants) == 1, LOG_WARNING,
        "Incorrect number of participants in self collision constraint. Constraint participants must equal 1.");
    if (!(arrlen(this->participants) == 1)) { return; }

    const size_t i = this->participants[0];
    const Vector2 pi = particles->pPositions[i];

    Vector2 deltaPi = Vector2Scale(this->surfaceNormal, -1.0f * Vector2DotProduct(Vector2Subtract(pi, this->entryPoint), this->surfaceNormal));
    particles->pPositions[i] = Vector2Add(pi, deltaPi);
}

void ProjectDistance(const Constraint *this, ParticlePool *particles)
{
    PASSERT(arrlenu(this->participants) == 2, LOG_ERROR, 
        "Incorrect number of participants in self collision constraint. Constraint participants must equal 2.");
    if (!(arrlen(this->participants) == 2)) { return; }
}

static ParticlePool* ConstructParticlePool_() 
{
    ParticlePool *pool = (ParticlePool*)malloc(sizeof(ParticlePool));
    PASSERT(pool, LOG_FATAL, "Failed to allocate particle pool");
    if(!pool) { return NULL; }

    for (int i = 0; i < MAX_PARTICLE_COUNT; i++) 
    {
        pool->pLifetimes[i]  = 0.0f;
        pool->pLifespans[i]  = 0.0f;

        pool->pPrevPositions[i] = (Vector2){ 0 };
        pool->pPositions[i]     = (Vector2){ 0 };
        pool->pVelocities[i]    = (Vector2){ 0 };
        
        pool->pBirthMasses[i]    = 0.0f;
        pool->pDeathMasses[i]    = 0.0f;
        pool->pMasses[i]  = 0.0f;

        pool->pBirthColors[i]    = (Color){ 0 };
        pool->pDeathColors[i]    = (Color){ 0 };
        pool->pColors[i]  = (Color){ 0 };
    }
    return pool;
}

static void DestructParticlePool_(ParticlePool *pool) 
{
    free(pool);
}

static void SwapParticles_(ParticlePool *pool, size_t i, size_t j)
{
    pool->pLifetimes[i]      = pool->pLifetimes[j];
    pool->pLifespans[i]      = pool->pLifespans[j];

    pool->pPrevPositions[i] = pool->pPrevPositions[j];
    pool->pPositions[i]     = pool->pPositions[j];
    pool->pVelocities[i]    = pool->pVelocities[j];

    pool->pBirthMasses[i]   = pool->pBirthMasses[j];
    pool->pDeathMasses[i]   = pool->pDeathMasses[j];
    pool->pMasses[i]        = pool->pMasses[j];

    pool->pBirthColors[i]   = pool->pBirthColors[j];
    pool->pDeathColors[i]   = pool->pDeathColors[j];
    pool->pColors[i]        = pool->pColors[j];
}

static void KillParticle_(ParticleSystem *system, size_t index) 
{
    system->activeCount--;
    SwapParticles_(system->particles_, index, system->activeCount);
}

static Vector2 CalculateAcceleration_(const ParticlePool *particles, const Force *forces, size_t pi)
{
    Vector2 pAcceleration = (Vector2){ 0 };
    const float invMass = 1.0f / particles->pMasses[pi];

    for(size_t j = 0; j < arrlenu(forces); j++){
        switch (forces[j].type)
        {
        case FORCE_DIRECTION:
            pAcceleration = Vector2Add(pAcceleration, Vector2Scale(forces[j].direction, invMass));
            break;
        case FORCE_POINT:
            Vector2 forceDirection = Vector2Normalize(Vector2Subtract(forces[j].position, 
                                                    particles->pPositions[pi]));
            // float invDistanceSqr = 1.0f / Vector2DistanceSqr(system->forces_[j].position, 
            //                                      system->particles_->pPositions[i]);
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

static Vector2 CalculateEntryPoint(const Vector2 P, const Vector2 v, const Vector2 Q, const Vector2 sn)
{
    // t = ((Q - P) . sn) / (v . sn)
    const float t = Vector2DotProduct(Vector2Subtract(Q, P), sn) / Vector2DotProduct(v, sn);
    // P + (t*v)
    return Vector2Add(P, Vector2Scale(v, t));
}

static size_t GenerateCollisionConstraints_(ParticleSystem *system)
{
    size_t collisionCount = 0;
    
    // Check for particle self collision
    const float minDistance = 2.0f * PARTICLE_RADIUS;
    for (size_t i = 0; i < system->activeCount; i++)
    {
        for (size_t  j = 0; j < system->activeCount; j++)
        {
            if ( i == j) { continue; }
            if (Vector2Distance(system->particles_->pPositions[i], system->particles_->pPositions[j]) < minDistance)
            {
                AddSelfCollisionConstraint(system, i, j);
                collisionCount++;
            }
        }
    }

    // check particle-boundary collision
    for (size_t i = 0; i < system->activeCount; i++)
    {
        Vector2 P, v, Q, sn, EP;
        if(system->particles_->pPositions[i].x < system->boundaryBox.left + PARTICLE_RADIUS)
        {
            P = system->particles_->pPositions[i];
            v = system->particles_->pVelocities[i];
            Q = (Vector2){system->boundaryBox.left + PARTICLE_RADIUS, P.y };
            sn = (Vector2){ 1.0f, 0.0f };
            EP = CalculateEntryPoint(P, v, Q, sn);
            AddCollisionConstraint(system, i, sn, EP);
        }else if(system->particles_->pPositions[i].x > (system->boundaryBox.right - PARTICLE_RADIUS))
        {
            P = system->particles_->pPositions[i];
            v = system->particles_->pVelocities[i];
            Q = (Vector2){system->boundaryBox.right - PARTICLE_RADIUS, P.y };
            sn = (Vector2){ -1.0f, 0.0f };
            EP = CalculateEntryPoint(P, v, Q, sn);
            AddCollisionConstraint(system, i, sn, EP);
        }else if(system->particles_->pPositions[i].y < (system->boundaryBox.top + PARTICLE_RADIUS))
        {
            P = system->particles_->pPositions[i];
            v = system->particles_->pVelocities[i];
            Q = (Vector2){P.x, system->boundaryBox.top + PARTICLE_RADIUS };
            sn = (Vector2){ 0.0f, 1.0f };
            EP = CalculateEntryPoint(P, v, Q, sn);
            AddCollisionConstraint(system, i, sn, EP);
        }else if(system->particles_->pPositions[i].y > system->boundaryBox.bottom - PARTICLE_RADIUS)
        {
            P = system->particles_->pPositions[i];
            v = system->particles_->pVelocities[i];
            Q = (Vector2){P.x, system->boundaryBox.bottom - PARTICLE_RADIUS };
            sn = (Vector2){ 0.0f, -1.0f };
            EP = CalculateEntryPoint(P, v, Q, sn);
            AddCollisionConstraint(system, i, sn, EP);
        }
    }

    return collisionCount;
}

static void UpdateParticlesLife_(ParticleSystem *system, float deltaTime)
{
    // Update lifespan of particles and deactivate/kill any particles whose
    // lifespan has exceeded its lifetime.
    size_t deadCount = 0;
    for (size_t i = 0; i < system->activeCount; i++) 
    {
        system->particles_->pLifespans[i] += deltaTime;
        if (system->particles_->pLifespans[i] > system->particles_->pLifetimes[i])
        { 
            deadCount++;
            SwapParticles_(system->particles_, i, (system->activeCount - deadCount));
        }
    }
    system->activeCount -= deadCount;
}

static void UpdateParticlesMotion_(ParticleSystem *system, float deltaTime)
{
    // Initial particle position estimate
    for (size_t i = 0; i < system->activeCount; i++)
    {
        Vector2 pAcceleration = CalculateAcceleration_(system->particles_, system->forces_, i);

        system->particles_->pVelocities[i]  = Vector2Add(system->particles_->pVelocities[i], 
                                            Vector2Scale(pAcceleration, deltaTime));
        system->particles_->pPrevPositions[i] = system->particles_->pPositions[i];
        system->particles_->pPositions[i]   = Vector2Add(system->particles_->pPositions[i], 
                                            Vector2Scale(system->particles_->pVelocities[i], deltaTime));
    }

    // Generate self collision constraints
    size_t collisionCount = GenerateCollisionConstraints_(system);

    // Project constraints (solver)
    for (size_t i = 0; i < arrlenu(system->constraints_); i++)
    {
        const Constraint c = system->constraints_[i];
        c.ProjectFn(&c, system->particles_);
    }
    for (size_t i = 0; i < collisionCount; i++) { arrpop(system->constraints_); }
    
    // Update velocities after constraint solver
    for (size_t i = 0; i < system->activeCount; i++)
    {
        system->particles_->pVelocities[i] = Vector2Scale(
            Vector2Subtract(system->particles_->pPositions[i], system->particles_->pPrevPositions[i]), 
                (1.0f / deltaTime));
    }
}
static void UpdateParticleAttributes_(ParticleSystem *system)
{
    for (size_t i = 0; i < system->activeCount; i++)
    {
        const float t = (system->particles_->pLifespans[i] / system->particles_->pLifetimes[i]);

        system->particles_->pMasses[i] = Lerp( system->particles_->pBirthMasses[i],
                                        system->particles_->pDeathMasses[i], t);
        system->particles_->pColors[i]  = ColorLerp( system->particles_->pBirthColors[i],
                                system->particles_->pDeathColors[i], t);
    }
}

ParticleSystem* ConstructParticleSystem(Boundary bb)
{
    ParticleSystem* system = (ParticleSystem*)malloc(sizeof(ParticleSystem));
    PASSERT(system, LOG_FATAL, "Failed to allocate particle pool");
    if(!system) { return NULL; }

    system->activeCount = 0;
    system->boundaryBox = bb;

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

void EmitParticle(ParticleSystem *system, const ParticleProps *props) 
{
    size_t i = system->activeCount;
    PASSERT(i < MAX_PARTICLE_COUNT, LOG_WARNING, "active particle count exceeds MAX_PARTICLE_COUNT")
    if(!(i < MAX_PARTICLE_COUNT)) { return; }

    system->activeCount += 1;

    PASSERT((props->variance > -EPSILON && props->variance < (1.0 + EPSILON)),
        LOG_WARNING, "variance value outside valid range [0.0, 1.0]. Clamping value to valid range.")
    const float variance = Clamp(props->variance, 0.0f, 1.0f);
    const float randomScalar = GetRandomValueF();

    system->particles_->pLifetimes[i]    = props->lifetime + (props->lifetime * (GetRandomValueF() * variance));
    system->particles_->pLifespans[i]    = 0;

    system->particles_->pPositions[i]    = Vector2Add(system->emitter.position,
                                            Vector2Scale((Vector2){ GetRandomValueF(), GetRandomValueF() },
                                                system->emitter.radius));
    system->particles_->pVelocities[i]   = Vector2Add(props->velocity,
                                            Vector2Scale(props->velocity, randomScalar * variance));

    system->particles_->pBirthMasses[i]  = props->birthMass + (props->birthMass * (randomScalar * variance));
    system->particles_->pDeathMasses[i]  = props->deathMass;
    system->particles_->pMasses[i]       = system->particles_->pBirthMasses[i];

    system->particles_->pBirthColors[i]  = props->birthColor;
    system->particles_->pDeathColors[i]  = props->deathColor;
    system->particles_->pColors[i]       = system->particles_->pBirthColors[i];
}

void UpdateParticles(ParticleSystem *system, float deltaTime)
{
    UpdateParticlesLife_(system, deltaTime);
    UpdateParticleAttributes_(system);

    const int substeps = 4;
    const float deltaTimeSubstep = deltaTime / (float)substeps;
    for(size_t i = 0; i < substeps; i++)
    {
        UpdateParticlesMotion_(system, deltaTimeSubstep);
    }
}

void DrawParticles(ParticleSystem *system)
{
    for (size_t i = 0; i < system->activeCount; i++)
    {
        DrawCircleV(system->particles_->pPositions[i], 
            PARTICLE_RADIUS, system->particles_->pColors[i]);
    }
}

void DrawForces(ParticleSystem *system)
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
    Constraint c;
    c.participants = NULL;
    c.type = CONSTRAINT_SELF_COLLISION;
    arrput(c.participants, i);
    arrput(c.participants, j);
    c.ProjectFn = ProjectSelfCollision;

    c.surfaceNormal = (Vector2){ 0 };
    c.entryPoint = (Vector2){ 0 };

    arrput(system->constraints_, c);
}

void AddCollisionConstraint(ParticleSystem *system, size_t i, Vector2 sn, Vector2 ep)
{
    Constraint c;
    c.participants = NULL;
    c.type = CONSTRAINT_COLLISION;
    arrput(c.participants, i);
    c.ProjectFn = ProjectCollision;
    
    c.surfaceNormal = sn;
    c.entryPoint = ep;

    arrput(system->constraints_, c);
}

void AddDistanceConstraint(ParticleSystem *system, size_t i, size_t j)
{
    Constraint c;
    c.participants = NULL;
    c.type = CONSTRAINT_DISTANCE;
    arrput(c.participants, i);
    arrput(c.participants, j);
    c.ProjectFn = ProjectDistance;
    
    c.surfaceNormal = (Vector2){ 0 };
    c.entryPoint = (Vector2){ 0 };

    arrput(system->constraints_, c);
}

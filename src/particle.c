#include "pch.h"
#include "particle.h"

ParticleProps defaultParticleProps = {
    0.5f,                   // varaince
    10.0f,                  // lifetime
    0.0f,                   // lifespan
    { 0.0f, 0.0f },         // position
    { 0.0f, 0.0f },         // velocity
    10.0f,                  // mass
    6.0f,                   // birthSize
    0.1f,                   // deathSize
    { 230, 41, 55, 255 },   // birthColor
    { 255, 161, 0, 0 },     // deathColor
};

static ParticlePool* ConstructParticlePool_() 
{
    ParticlePool *pool = (ParticlePool*)malloc(sizeof(ParticlePool));
    PASSERT(pool, LOG_FATAL, "Failed to allocate particle pool");
    if(!pool) { return NULL; }

    for (int i = 0; i < MAX_PARTICLE_COUNT; i++) 
    {
        pool->pLifetimes[i]  = 0.0f;
        pool->pLifespans[i]  = 0.0f;

        pool->pPositions[i]     = (Vector2){ 0 };
        pool->pVelocities[i]    = (Vector2){ 0 };
        pool->pMasses[i]        = 0.0f;
        
        pool->pBirthSizes[i]    = 0.0f;
        pool->pDeathSizes[i]    = 0.0f;
        pool->pCurrentSizes[i]  = 0.0f;

        pool->pBirthColors[i]    = (Color){ 0 };
        pool->pDeathColors[i]    = (Color){ 0 };
        pool->pCurrentColors[i]  = (Color){ 0 };
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

    pool->pPositions[i]     = pool->pPositions[j];
    pool->pVelocities[i]    = pool->pVelocities[j];
    pool->pMasses[i]        = pool->pMasses[j];

    pool->pBirthSizes[i]    = pool->pBirthSizes[j];
    pool->pDeathSizes[i]    = pool->pDeathSizes[j];
    pool->pCurrentSizes[i]  = pool->pCurrentSizes[j];

    pool->pBirthColors[i]    = pool->pBirthColors[j];
    pool->pDeathColors[i]    = pool->pDeathColors[j];
    pool->pCurrentColors[i]  = pool->pCurrentColors[j];
}

static void KillParticle_(ParticleSystem *system, size_t index) 
{
    system->activeCount--;
    SwapParticles_(system->pool_, index, system->activeCount);
}

static void UpdateParticlesLife_(ParticleSystem *system, float deltaTime)
{
    // Update lifespan of particles and deactivate/kill any particles whose
    // lifespan has exceeded its lifetime.
    size_t deadCount = 0;
    for (size_t i = 0; i < system->activeCount; i++) 
    {
        system->pool_->pLifespans[i] += deltaTime;
        if (system->pool_->pLifespans[i] > system->pool_->pLifetimes[i])
        { 
            deadCount++;
            SwapParticles_(system->pool_, i, (system->activeCount - deadCount));
        }
    }
    system->activeCount -= deadCount;
}

static void UpdateParticlesMotion_(ParticleSystem *system, float deltaTime)
{
    for (size_t i = 0; i < system->activeCount; i++)
    {
        Vector2 pAcceleration = (Vector2){ 0 };
        for(size_t j = 0; j < arrlenu(system->affectors_); j++){
            switch (system->affectors_[j].type)
            {
            case AFFECTOR_DIRECTION:
                pAcceleration = Vector2Add(pAcceleration, 
                    system->affectors_[j].direction);
                break;
            case AFFECTOR_POINT:
                Vector2 affectorParticleDirection = Vector2Normalize(Vector2Subtract(system->affectors_[j].position, 
                                                     system->pool_->pPositions[i]));
                float inverserDistanceSquared = 1.0f / Vector2DistanceSqr(system->affectors_[j].position, 
                                                     system->pool_->pPositions[i]);
                pAcceleration = Vector2Add(pAcceleration, 
                                    Vector2Scale(affectorParticleDirection, 
                                        (1.0f * system->affectors_[j].strength)));
                break;
            default:
                break;
            }
        }
        system->pool_->pVelocities[i]  = Vector2Add(system->pool_->pVelocities[i], 
                                            Vector2Scale(pAcceleration, deltaTime));
        system->pool_->pPositions[i]   = Vector2Add(system->pool_->pPositions[i], 
                                            Vector2Scale(system->pool_->pVelocities[i], deltaTime));
    }
}
static void UpdateParticleAttributes_(ParticleSystem *system)
{
    for (size_t i = 0; i < system->activeCount; i++)
    {
        float t = (system->pool_->pLifespans[i] / system->pool_->pLifetimes[i]);

        system->pool_->pCurrentSizes[i] = Lerp( system->pool_->pBirthSizes[i],
                                        system->pool_->pDeathSizes[i], t);
        system->pool_->pCurrentColors[i]  = ColorLerp( system->pool_->pBirthColors[i],
                                system->pool_->pDeathColors[i], t);
    }
}

ParticleSystem* ConstructParticleSystem()
{
    ParticleSystem* system = (ParticleSystem*)malloc(sizeof(ParticleSystem));
    PASSERT(system, LOG_FATAL, "Failed to allocate particle pool");
    if(!system) { return NULL; }

    system->activeCount = 0;

    system->emitter.position    = (Vector2){ 0 };
    system->emitter.radius      = 4.0f;
    
    system->affectors_ = NULL;

    system->pool_ = ConstructParticlePool_();

    return system;
}

void DestructParticleSystem(ParticleSystem *system)
{
    DestructParticlePool_(system->pool_);
    arrfree(system->affectors_);
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

    system->pool_->pLifetimes[i]    = props->lifetime + (props->lifetime * (GetRandomValueF() * variance));
    system->pool_->pLifespans[i]    = 0;

    system->pool_->pPositions[i]    = Vector2Add(system->emitter.position,
                                        Vector2Scale((Vector2){ GetRandomValueF(), GetRandomValueF() },
                                            system->emitter.radius));
    system->pool_->pVelocities[i]   = Vector2Add(props->velocity,
                                        Vector2Scale(props->velocity, randomScalar * variance));
    system->pool_->pMasses[i]       = props->mass + (props->mass * (randomScalar * variance));

    system->pool_->pBirthSizes[i]   = props->birthSize + (props->birthSize * (randomScalar * variance));
    system->pool_->pDeathSizes[i]   = props->deathSize;
    system->pool_->pCurrentSizes[i] = system->pool_->pBirthSizes[i];

    system->pool_->pBirthColors[i]   = props->birthColor;
    system->pool_->pDeathColors[i]   = props->deathColor;
    system->pool_->pCurrentColors[i] = system->pool_->pBirthColors[i];
}

void UpdateParticles(ParticleSystem *system, float deltaTime)
{
    UpdateParticlesLife_(system, deltaTime);
    UpdateParticlesMotion_(system, deltaTime);
    UpdateParticleAttributes_(system);
}

void DrawParticles(ParticleSystem *system)
{
    for (size_t i = 0; i < system->activeCount; i++)
    {
        DrawCircleV(system->pool_->pPositions[i], 
            system->pool_->pCurrentSizes[i], 
            system->pool_->pCurrentColors[i]);
    }
}

void AddAffector(ParticleSystem *system, Affector affector)
{
    arrput(system->affectors_, affector);
}

void RemoveAffector(ParticleSystem *system)
{

}

void DrawAffectors(ParticleSystem *system)
{
    for (size_t i = 0; i < arrlenu(system->affectors_); i++)
    {
        switch (system->affectors_[i].type)
        {
        case AFFECTOR_DIRECTION:
        DrawCircleV(system->affectors_[i].direction, 4.0f, YELLOW);
        break;
        case AFFECTOR_POINT:
        DrawCircleV(system->affectors_[i].position, 4.0f, YELLOW);
            break;
        default:
            break;
        }
    }
}

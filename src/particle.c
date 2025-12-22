#include "pch.h"
#include "particle.h"

ParticleProps defaultParticleProps = {
    { 10.0f, 0.0f,},    // lifetime
    { 0.0f, 0.0f},      // position
    { 0.0f, -50.0f},    // velocity
    { 0.0f, 9.8f},      // acceleration
    { 4.0f, 1.0f},      // size
    { RED, ORANGE}      // color
};

static ParticlePool* ConstructParticlePool_() 
{
    ParticlePool *pool = (ParticlePool*)malloc(sizeof(ParticlePool));

    for (int i = 0; i < MAX_PARTICLE_COUNT; i++) 
    {
        pool->pLifetimes[i]  = (ParticleLifetime){ 0 };

        pool->pPositions[i]      = (Vector2){ 0 };
        pool->pVelocities[i]     = (Vector2){ 0 };
        pool->pAccelations[i]    = (Vector2){ 0 };
        
        pool->pSizes[i]  = (ParticleSize){ 0 };
        pool->pColors[i] = (ParticleColor){ 0 };
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

    pool->pPositions[i]      = pool->pPositions[j];
    pool->pVelocities[i]     = pool->pVelocities[j];
    pool->pAccelations[i]    = pool->pAccelations[j];

    pool->pSizes[i]  = pool->pSizes[j];
    pool->pColors[i] = pool->pColors[j];
}

static void KillParticle_(ParticleSystem *system, size_t index) 
{
    system->activeCount--;
    SwapParticles_(system->pool_, index, system->activeCount);
}

ParticleSystem* ConstructParticleSystem()
{
    ParticleSystem* system = (ParticleSystem*)malloc(sizeof(ParticleSystem));
    
    system->emitter.position    = (Vector2){ 0 };
    system->emitter.radius      = 4.0f;
    
    system->pool_ = ConstructParticlePool_();

    return system;
}

void DestructParticleSystem(ParticleSystem *system)
{
    DestructParticlePool_(system->pool_);
    free(system);
}

void EmitParticle(ParticleSystem *system, const ParticleProps *props) 
{
    size_t i = system->activeCount;
    PASSERT(i < MAX_PARTICLE_COUNT, "active particle count greater than MAX_PARTICLE_COUNT")

    system->activeCount += 1;

    system->pool_->pLifetimes[i]      = props->lifetime;

    system->pool_->pPositions[i]      = system->emitter.position;
    system->pool_->pVelocities[i]     = props->velocity;
    system->pool_->pAccelations[i]    = props->acceleration;

    system->pool_->pSizes[i]  = props->size;
    system->pool_->pColors[i] = props->color;
}

void UpdateParticles(ParticleSystem *system, float deltaTime) 
{
    size_t deadCount = 0;
    for (size_t i = 0; i < system->activeCount; i++) 
    {
        system->pool_->pLifetimes[i].lifespan += deltaTime;
        if (system->pool_->pLifetimes[i].lifespan > system->pool_->pLifetimes[i].lifetime) 
        { 
            deadCount++;
            SwapParticles_(system->pool_, i, (system->activeCount - deadCount));
        }
    }
    system->activeCount -= deadCount;

    for (size_t i = 0; i < system->activeCount; i++)
    {
        system->pool_->pVelocities[i]  = Vector2Add(system->pool_->pVelocities[i], Vector2Scale(system->pool_->pAccelations[i], deltaTime));
        system->pool_->pPositions[i]   = Vector2Add(system->pool_->pPositions[i], Vector2Scale(system->pool_->pVelocities[i], deltaTime));
    }
}

void DrawParticles(ParticleSystem *system)
{
    for (size_t i = 0; i < system->activeCount; i++){
        float currentSize   = system->pool_->pSizes[i].startSize;
        Color currentColor  = system->pool_->pColors[i].startColor;
        DrawCircleV(system->pool_->pPositions[i], currentSize, currentColor);
    }
}
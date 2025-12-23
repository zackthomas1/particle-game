#include "pch.h"
#include "particle.h"

ParticleProps defaultParticleProps = {
    0.9f,   // varaince
    10.0f,  // lifetime
    { 0.0f, 0.0f},      // position
    { 0.0f, -50.0f},    // velocity
    { 0.0f, 9.8f},      // acceleration
    { 10.0f, 0.1f}, // size
    { (Color){ 230, 41, 55, 255 }, (Color){ 255, 161, 0, 0 } }  // color
};

static ParticlePool* ConstructParticlePool_() 
{
    ParticlePool *pool = (ParticlePool*)malloc(sizeof(ParticlePool));

    for (int i = 0; i < MAX_PARTICLE_COUNT; i++) 
    {
        pool->pLife[i]  = (ParticleLife){ 0 };

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
    pool->pLife[i]      = pool->pLife[j];

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
    
    system->activeCount = 0;

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
    PASSERT(i < MAX_PARTICLE_COUNT, LOG_WARNING, "active particle count exceeds MAX_PARTICLE_COUNT")

    system->activeCount += 1;

    PASSERT((props->variance > -EPSILON && props->variance < (1.0 + EPSILON)),
        LOG_WARNING, "variance value outside valid range [0.0, 1.0]. Clamping value to valid range.")
    float variance = Clamp(props->variance, 0.0f, 1.0f);

    system->pool_->pLife[i].lifetime    = props->lifetime + (props->lifetime * (GetRandomValueF() * variance));
    system->pool_->pLife[i].lifespan    = 0;

    system->pool_->pPositions[i]    = Vector2Add(system->emitter.position,
                                        Vector2Scale((Vector2){ GetRandomValueF(), GetRandomValueF() },
                                            system->emitter.radius));
    system->pool_->pVelocities[i]   = Vector2Add(props->velocity,
                                        Vector2Scale(props->velocity, (GetRandomValueF() * variance)));
    system->pool_->pAccelations[i]  = Vector2Add(props->acceleration,
                                        Vector2Scale(props->acceleration, (GetRandomValueF() * variance)));

    system->pool_->pSizes[i].startSize  = props->size.startSize + (props->size.startSize * (GetRandomValueF() * variance));
    system->pool_->pSizes[i].endSize  = props->size.endSize;
    system->pool_->pColors[i] = props->color;
}

void UpdateParticles(ParticleSystem *system, float deltaTime) 
{
    // Update lifespan of particles and deactivate/kill any particles whose
    // lifespan has exceeded its lifetime.
    size_t deadCount = 0;
    for (size_t i = 0; i < system->activeCount; i++) 
    {
        system->pool_->pLife[i].lifespan += deltaTime;
        if (system->pool_->pLife[i].lifespan > system->pool_->pLife[i].lifetime) 
        { 
            deadCount++;
            SwapParticles_(system->pool_, i, (system->activeCount - deadCount));
        }
    }
    system->activeCount -= deadCount;

    // Update physics sim
    for (size_t i = 0; i < system->activeCount; i++)
    {
        system->pool_->pVelocities[i]  = Vector2Add(system->pool_->pVelocities[i], 
                                            Vector2Scale(system->pool_->pAccelations[i], deltaTime));
        system->pool_->pPositions[i]   = Vector2Add(system->pool_->pPositions[i], 
                                            Vector2Scale(system->pool_->pVelocities[i], deltaTime));
    }
}

void DrawParticles(ParticleSystem *system)
{
    for (size_t i = 0; i < system->activeCount; i++){
        float t = (system->pool_->pLife[i].lifespan / system->pool_->pLife[i].lifetime);

        float currentSize   = Lerp( system->pool_->pSizes[i].startSize, 
                                system->pool_->pSizes[i].endSize,
                                t);
        Color currentColor  = ColorLerp( system->pool_->pColors[i].startColor, 
                                system->pool_->pColors[i].endColor,
                                t);
        DrawCircleV(system->pool_->pPositions[i], currentSize, currentColor);
    }
}
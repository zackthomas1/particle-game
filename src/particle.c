#include "pch.h"
#include "particle.h"

// #define PASSERT(expression, ...) PAssert()

// void PAssert(bool expr, const char *text) 
// {
//     assert(expr && text);
//     abort(); 
// }

static const Vector2 INITIAL_VELOCITY       = { 0.0f, -50.0f};
static const Vector2 INITIAL_ACCELERATION   = { 0.0f, 9.8f};
static const float INITIAL_LIFETIME         = INFINITY;
static const Color PARTICLE_COLOR           = RED;
static const float PARTICLE_RADIUS          = 4.0f;

ParticleFactory* ConstructParticleFactory() 
{
    ParticleFactory* factory = (ParticleFactory*)malloc(sizeof(ParticleFactory));
    
    factory->activeCount = 0;
    factory->position = (Vector2){ 0 };

    for (int i = 0; i < MAX_PARTICLE_COUNT; i++) 
    {
        factory->pPositions[i]      = (Vector2){ 0 };
        factory->pVelocities[i]     = (Vector2){ 0 };
        factory->pAccelations[i]    = (Vector2){ 0 };

        factory->pLifetimes[i]  = 0.0f;
    }

    return factory;
}

void DestructParticleFactory(ParticleFactory* factory) 
{
    factory->activeCount = 0;
    free(factory);
}

void SpawnParticle(ParticleFactory* factory) 
{
    PASSERT(factory->activeCount < MAX_PARTICLE_COUNT, "active particle count greater than MAX_PARTICLE_COUNT")
    size_t i = factory->activeCount;

    factory->pLifetimes[i]      = INITIAL_LIFETIME;

    factory->pPositions[i]      = factory->position;
    factory->pVelocities[i]     = INITIAL_VELOCITY;
    factory->pAccelations[i]    = INITIAL_ACCELERATION;

    factory->activeCount += 1;
}

void KillParticle(ParticleFactory* factory, size_t index) 
{
    factory->activeCount--;
    SwapParticles_(factory, index, factory->activeCount);
}

void UpdateParticles(ParticleFactory* factory, float deltaTime) 
{
    size_t deadCount = 0;
    for (size_t i = 0; i < factory->activeCount; i++) 
    {
        factory->pLifetimes[i]  -= deltaTime;
        if (factory->pLifetimes[i] < 0.0f) 
        { 
            deadCount++;
            SwapParticles_(factory, i, (factory->activeCount - deadCount));
        }
    }
    factory->activeCount -= deadCount;

    for (size_t i = 0; i < factory->activeCount; i++)
    {
        factory->pVelocities[i]  = Vector2Add(factory->pVelocities[i], Vector2Scale(factory->pAccelations[i], deltaTime));
        factory->pPositions[i]   = Vector2Add(factory->pPositions[i], Vector2Scale(factory->pVelocities[i], deltaTime));
    }
}

void DrawParticles(ParticleFactory* factory) 
{
    for (size_t i = 0; i < factory->activeCount; i++){
        DrawCircleV(factory->pPositions[i], PARTICLE_RADIUS, PARTICLE_COLOR);
    }
}

static void SwapParticles_(ParticleFactory* factory, size_t i, size_t j)
{
    factory->pLifetimes[i]      = factory->pLifetimes[j];

    factory->pPositions[i]      = factory->pPositions[j];
    factory->pVelocities[i]     = factory->pVelocities[j];
    factory->pAccelations[i]    = factory->pAccelations[j];
}
#include "pch.h"
#include "particle.h"

// #define PASSERT(expression, ...) PAssert()

// void PAssert(bool expr, const char *text) 
// {
//     assert(expr && text);
//     abort(); 
// }

static const Vector2 INITIAL_VELOCITY   = { 0.0f, -10.0f};
static const float INITIAL_LIFETIME     = 10.0f;
static const Color PARTICLE_COLOR       = RED;
static const float PARTICLE_RADIUS      = 4.0f;

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
        factory->pIsDead[i]     = false;
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
    size_t i = factory->activeCount;

    factory->pLifetimes[i]      = INITIAL_LIFETIME;
    factory->pIsDead[i]         = false;

    factory->pPositions[i]      = factory->position;
    factory->pVelocities[i]     = INITIAL_VELOCITY;
    // factory->pAccelations[i] = (Vector2){ 0 };

    factory->activeCount += 1;
}

void KillParticle(ParticleFactory* factory, int index) 
{
    factory->activeCount--;

    factory->pLifetimes[index]      = factory->pLifetimes[factory->activeCount];
    factory->pIsDead[index]         = factory->pIsDead[factory->activeCount];

    factory->pPositions[index]      = factory->pPositions[factory->activeCount];
    factory->pVelocities[index]     = factory->pVelocities[factory->activeCount];
    factory->pAccelations[index]    = factory->pAccelations[factory->activeCount];
}

void UpdateParticles(ParticleFactory* factory, float deltaTime) 
{
    uint32_t deadCount = 0;
    for (size_t i = 0; i < factory->activeCount; i++) 
    {
        factory->pLifetimes[i]  -= deltaTime;
        if (factory->pLifetimes[i] < 0.0f) 
        { 
            factory->pIsDead[i] = true; 
            continue; 
        }
    }

    for (size_t i = 0; i < factory->activeCount; i++)
    {
        factory->pVelocities[i]  = Vector2Add(factory->pVelocities[i], Vector2Scale(factory->pAccelations[i], deltaTime));
        factory->pPositions[i]   = Vector2Add(factory->pPositions[i], Vector2Scale(factory->pVelocities[i], deltaTime));
    }

    for(size_t i = 0; i < factory->activeCount; i++)
    {
        if (factory->pIsDead[i]) { KillParticle(factory, i); }
    }
}

void DrawParticles(ParticleFactory* factory) 
{
    for (int i = 0; i < factory->activeCount; i++){
        DrawCircleV(factory->pPositions[i], PARTICLE_RADIUS, PARTICLE_COLOR);
    }
}
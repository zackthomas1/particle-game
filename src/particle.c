#include "pch.h"
#include "particle.h"

ParticleProps defaultParticleProps = {
    { 10.0f, 10.0f,},   // lifetimeRemaining
    { 0.0f, 0.0f},    // position
    { 0.0f, -50.0f},    // velocity
    { 0.0f, 9.8f},      // acceleration
    { 4.0f, 1.0f},      // size
    { RED, ORANGE}      // color
 };

ParticleFactory* ConstructParticleFactory() 
{
    ParticleFactory* factory = (ParticleFactory*)malloc(sizeof(ParticleFactory));
    
    factory->activeCount = 0;
    factory->position = (Vector2){ 0 };

    for (int i = 0; i < MAX_PARTICLE_COUNT; i++) 
    {
        factory->pLifetimes[i]  = (ParticleLifetime){ 0 };

        factory->pPositions[i]      = (Vector2){ 0 };
        factory->pVelocities[i]     = (Vector2){ 0 };
        factory->pAccelations[i]    = (Vector2){ 0 };
        
        factory->pSizes[i]  = (ParticleSize){ 0 };
        factory->pColors[i] = (ParticleColor){ 0 };
    }

    return factory;
}

void DestructParticleFactory(ParticleFactory* factory) 
{
    factory->activeCount = 0;
    free(factory);
}

void SpawnParticle(ParticleFactory* factory, const ParticleProps *props) 
{
    PASSERT(factory->activeCount < MAX_PARTICLE_COUNT, "active particle count greater than MAX_PARTICLE_COUNT")
    size_t i = factory->activeCount;

    factory->activeCount += 1;

    factory->pLifetimes[i]      = props->lifetime;

    factory->pPositions[i]      = factory->position;
    factory->pVelocities[i]     = props->velocity;
    factory->pAccelations[i]    = props->acceleration;

    factory->pSizes[i]  = props->size;
    factory->pColors[i] = props->color;
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
        factory->pLifetimes[i].lifetimeRemaining -= deltaTime;
        if (factory->pLifetimes[i].lifetimeRemaining < 0.0f) 
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
        float currentSize = factory->pSizes[i].startSize;
        Color currentColor = factory->pColors[i].startColor;
        DrawCircleV(factory->pPositions[i], currentSize, currentColor);
    }
}

static void SwapParticles_(ParticleFactory* factory, size_t i, size_t j)
{
    factory->pLifetimes[i]      = factory->pLifetimes[j];

    factory->pPositions[i]      = factory->pPositions[j];
    factory->pVelocities[i]     = factory->pVelocities[j];
    factory->pAccelations[i]    = factory->pAccelations[j];

    factory->pSizes[i]  = factory->pSizes[j];
    factory->pColors[i] = factory->pColors[j];
}
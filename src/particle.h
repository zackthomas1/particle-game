#pragma once
#include "raylib.h"

#define MAX_PARTICLE_COUNT 10000

typedef enum { 
    WATER, 
    SAND,
} ParticleType;

typedef struct ParticleLifetime
{
    float lifetime, lifetimeRemaining;
}ParticleLifetime;

typedef struct ParticleSize
{
    float startSize, endSize;
}ParticleSize;

typedef struct ParticleColor
{
    Color startColor, endColor; 
}ParticleColor;

typedef struct ParticleProps
{
    ParticleLifetime lifetime;

    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;

    ParticleSize size;
    ParticleColor color;
}ParticleProps;

extern ParticleProps defaultParticleProps;

typedef struct ParticleFactory 
{
    Vector2 position;
    size_t activeCount;

    ParticleLifetime pLifetimes[MAX_PARTICLE_COUNT];

    Vector2 pPositions[MAX_PARTICLE_COUNT];
    Vector2 pVelocities[MAX_PARTICLE_COUNT];
    Vector2 pAccelations[MAX_PARTICLE_COUNT];

    ParticleSize pSizes[MAX_PARTICLE_COUNT];
    ParticleColor pColors[MAX_PARTICLE_COUNT];

}ParticleFactory;

type

typedef struct ParticleSystem 
{
    Vector2 gravity; 
    Vector2 wind;
    ParticleFactory *factory;

}ParticleSystem;

ParticleFactory* ConstructParticleFactory();
void DestructParticleFactory(ParticleFactory* factory);

void SpawnParticle(ParticleFactory* factory, const ParticleProps *props);
void KillParticle(ParticleFactory* factory, size_t index);
void UpdateParticles(ParticleFactory* factory, float deltaTime);
void DrawParticles(ParticleFactory* factory);

static void SwapParticles_(ParticleFactory* factory, size_t i, size_t j);
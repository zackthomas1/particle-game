#pragma once
#include "raylib.h"

#define MAX_PARTICLE_COUNT 1000

typedef enum { 
    WATER, 
    SAND,
} PType;

typedef struct ParticleFactory 
{
    Vector2 position;
    size_t activeCount;

    float pLifetimes[MAX_PARTICLE_COUNT];

    Vector2 pPositions[MAX_PARTICLE_COUNT];
    Vector2 pVelocities[MAX_PARTICLE_COUNT];
    Vector2 pAccelations[MAX_PARTICLE_COUNT];
}ParticleFactory;

typedef struct ParticleSystem 
{
    Vector2 gravity; 
    Vector2 wind;
    ParticleFactory *factory;

}ParticleSystem;

ParticleFactory* ConstructParticleFactory();
void DestructParticleFactory(ParticleFactory* factory);

void SpawnParticle(ParticleFactory* factory);
void KillParticle(ParticleFactory* factory, size_t index);
void UpdateParticles(ParticleFactory* factory, float deltaTime);
void DrawParticles(ParticleFactory* factory);

static void SwapParticles_(ParticleFactory* factory, size_t i, size_t j);
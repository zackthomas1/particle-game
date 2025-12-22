#pragma once
#include "raylib.h"

#define MAX_PARTICLE_COUNT 100

typedef enum { 
    WATER, 
    SAND,
} PType;

typedef struct ParticleFactory 
{
    Vector2 position;
    uint32_t activeCount;

    float pLifetimes[MAX_PARTICLE_COUNT];
    bool pIsDead[MAX_PARTICLE_COUNT];

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
void KillParticle(ParticleFactory* factory, int index);
void UpdateParticles(ParticleFactory* factory, float deltaTime);
void DrawParticles(ParticleFactory* factory);
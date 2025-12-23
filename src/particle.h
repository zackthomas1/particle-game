#pragma once
#include "raylib.h"

#define MAX_PARTICLE_COUNT 10000

// Particle system data
// -----------------
typedef enum { 
    WATER, 
    SAND,
} ParticleType;

typedef struct ParticleLife
{
    float lifetime, lifespan;
}ParticleLife;

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
    float variance;
 
    float lifetime;

    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;

    ParticleSize size;
    ParticleColor color;
}ParticleProps;

typedef struct ParticlePool
{
    ParticleLife pLife[MAX_PARTICLE_COUNT];

    Vector2 pPositions[MAX_PARTICLE_COUNT];
    Vector2 pVelocities[MAX_PARTICLE_COUNT];
    Vector2 pAccelations[MAX_PARTICLE_COUNT];

    ParticleSize pSizes[MAX_PARTICLE_COUNT];
    ParticleColor pColors[MAX_PARTICLE_COUNT];

}ParticlePool;

typedef struct ParticleEmitter
{
    Vector2 position;
    float radius;

    // TODO: implement particle pool chunking to enable the use of multiple 
    // particle emitter each of which is emit and kill particles within its allocated
    // index range of the particle pool.
    // size_t startIndex, size;
}ParticleEmitter;

typedef struct ParticleSystem 
{
    size_t activeCount;
    ParticleEmitter emitter;
    ParticlePool *pool_;
}ParticleSystem;

// default property values declaration
// -----------------
extern ParticleProps defaultParticleProps;

// Private methods
// -----------------
static ParticlePool* ConstructParticlePool_();
static void DestructParticlePool_(ParticlePool *factory);

static void SwapParticles_(ParticlePool *pool, size_t i, size_t j);
static void KillParticle_(ParticleSystem *system, size_t index);


// Interface methods
// -----------------
ParticleSystem* ConstructParticleSystem();
void DestructParticleSystem(ParticleSystem *system);

void EmitParticle(ParticleSystem *system, const ParticleProps *props);
void UpdateParticles(ParticleSystem *system, float deltaTime);
void DrawParticles(ParticleSystem *system);


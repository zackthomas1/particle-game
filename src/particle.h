#pragma once
#include "raylib.h"
#include "stb_ds.h"

#define MAX_PARTICLE_COUNT 10240

// Particle data
// -----------------
typedef enum { 
    WATER, 
    SAND,
} ParticleType;

typedef struct ParticleProps
{
    float variance;

    float lifetime, lifespan;

    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;

    float birthSize, deathSize;
    Color birthColor, deathColor;
}ParticleProps;

typedef struct ParticlePool
{
    float pLifetimes[MAX_PARTICLE_COUNT];
    float pLifespans[MAX_PARTICLE_COUNT];

    Vector2 pPositions[MAX_PARTICLE_COUNT];
    Vector2 pVelocities[MAX_PARTICLE_COUNT];
    Vector2 pAccelations[MAX_PARTICLE_COUNT];

    float pBirthSizes[MAX_PARTICLE_COUNT];
    float pDeathSizes[MAX_PARTICLE_COUNT];
    float pCurrentSizes[MAX_PARTICLE_COUNT];

    Color pBirthColors[MAX_PARTICLE_COUNT];
    Color pDeathColors[MAX_PARTICLE_COUNT];
    Color pCurrentColors[MAX_PARTICLE_COUNT];
}ParticlePool;

// Affectors
// ---------
typedef enum AffectorType
{
    AFFECTOR_DIRECTION,
    AFFECTOR_POINT,
}AffectorType;

typedef struct Affector
{
    AffectorType type;
    // uint32_t uid;

    // Directional affector
    Vector2 direction;

    // Point affector
    Vector2 position;
    float strength;
    float radius;
}Affector;

// System
// ----------
typedef struct ParticleEmitter
{
    Vector2 position;
    float radius;

    // TODO: implement particle pool chunking to enable the use of multiple 
    // particle emitter each of which is emit and kill particles within its allocated
    // index range of the particle pool.
    // size_t startIndex, size;
}ParticleEmitter;

typedef ParticleEmitter ParticleSink;

typedef struct ParticleSystem 
{
    size_t activeCount;
    ParticleEmitter emitter;

    Affector *affectors_;
    ParticlePool *pool_;
}ParticleSystem;

// declare extern variables
// -----------------
extern ParticleProps defaultParticleProps;

// Private methods
// -----------------
static ParticlePool* ConstructParticlePool_();
static void DestructParticlePool_(ParticlePool *factory);

static void SwapParticles_(ParticlePool *pool, size_t i, size_t j);
static void KillParticle_(ParticleSystem *system, size_t index);

static void UpdateParticlesLife_(ParticleSystem *system, float deltaTime);
static void UpdateParticlesMotion_(ParticleSystem *system, float deltaTime);
static void UpdateParticleAttributes_(ParticleSystem *system);

// Interface methods
// -----------------
ParticleSystem* ConstructParticleSystem();
void DestructParticleSystem(ParticleSystem *system);

void EmitParticle(ParticleSystem *system, const ParticleProps *props);
void UpdateParticles(ParticleSystem *system, float deltaTime);
void DrawParticles(ParticleSystem *system);

void AddAffector(ParticleSystem *system, const Affector affector);
void RemoveAffector(ParticleSystem *system);
void DrawAffectors(ParticleSystem *system);
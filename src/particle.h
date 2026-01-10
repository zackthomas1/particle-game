#pragma once
#include "raylib.h"
#include "config.h"

#define MAX_FORCES 10
#define MAX_PARTICIPANTS 4

// Forward declaration
typedef struct Hash Hash;

// Particles
// -----------------
typedef struct ParticleProps
{
    float variance;
    float lifetime;

    Vector2 velocity;
    float mass;
}ParticleProps;

typedef struct ParticlePool
{
    size_t activeCount;

    float pLifetimes[MAX_PARTICLE_COUNT];
    float pLifespans[MAX_PARTICLE_COUNT];

    Vector2 pPrevPositions[MAX_PARTICLE_COUNT];
    Vector2 pPositions[MAX_PARTICLE_COUNT];     // aPositions
    Vector2 pVelocities[MAX_PARTICLE_COUNT];
    float pMasses[MAX_PARTICLE_COUNT];
}ParticlePool;

// Forces
// ---------
typedef enum ForceType
{
    FORCE_GRAVITY,
    FORCE_VISCOUS,
    FORCE_ATTRACT,
    FORCE_REPULSE,
}ForceType;

typedef struct Force
{
    ForceType type;

    // FORCE_VISCOUS
    float viscosity; // Dynamic viscosity of the fluid

    // FORCE_ATTRACT/FORCE_REPULSE
    Vector2 position;
    float mass;
}Force;

typedef struct ForceObject
{
    Force force;
    struct ForceObject *next;
} ForceObject;

extern ForceObject forcePool[MAX_FORCES];
extern ForceObject *forceFreeList;

// Constraints
// -----------
typedef struct Constraint Constraint;

typedef void (*ProjectConstraintFn)(const Constraint *this, ParticlePool *particles,  float deltaTime);

typedef enum ConstraintType
{
    CONSTRAINT_SELF_COLLISION,
    CONSTRAINT_SURFACE_COLLISION,
    CONSTRAINT_DISTANCE,
}ConstraintType;

struct Constraint
{
    ConstraintType type;                    // unilateral (Cj(xi...xn) = 0) or bilateral (Cj(xi...xn) <= 0)
    size_t participants[MAX_PARTICIPANTS];  // set of indices

    size_t participantCount;                // cardinality

    ProjectConstraintFn ProjectFn;          // scalar constraint function

    // float kj             // stiffness parameter
};

void ProjectSelfCollision(const Constraint *this, ParticlePool *particles, float deltaTime);
void ProjectSurfaceCollision(const Constraint *this, ParticlePool *particles, float deltaTime);
void ProjectDistance(const Constraint *this, ParticlePool *particles, float deltaTime);

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

typedef struct ParticleSystem 
{
    struct {
        uint32_t left, right, bottom, top;
    } boundaryBox;
    Hash *spatialHash;

    ParticleEmitter emitter;

    Constraint *constraints_;
    ForceObject *forces_;
    ParticlePool *particles_;
}ParticleSystem;

// declare extern variables
// -----------------
extern ParticleProps defaultParticleProps;

// Interface methods
// -----------------
ParticleSystem* ConstructParticleSystem(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom);
void DestructParticleSystem(ParticleSystem *system);

void EmitParticles(ParticleSystem *system, const ParticleProps *props, uint32_t count);
void UpdateParticles(ParticleSystem *system, float deltaTime);
void KillParticles(ParticleSystem *system, Vector2 position, float radius);

Force* AddForce(ParticleSystem *system, ForceType type);
void RemoveForce(ParticleSystem *system, Force *f);

void InitParticleRender(const Shader *shader, float screenWidth, float screenHeight);
void CleanUpParticleRender();

void DrawParticlesInstanced(const ParticleSystem *system);

void AddSelfCollisionConstraint(ParticleSystem *system, size_t i, size_t j);
void AddSurfaceCollisionConstraint(ParticleSystem *system, size_t i, Vector2 sn, Vector2 ep);
void AddDistanceConstraint(ParticleSystem *system, size_t i, size_t j);
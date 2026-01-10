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

// Private methods
static ParticlePool* ConstructParticlePool_();
static void DestructParticlePool_(ParticlePool *particles);

static void SwapParticles_(ParticlePool *particles, size_t i, size_t j);
static void KillParticle_(ParticlePool *particles, size_t index);

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
    bool isAllocated;
    Force obj;
} ForceObject;

extern ForceObject forcePool[MAX_FORCES];

static Force* BorrowForce_(); 
static void ReturnForce_(Force *f);

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
    ConstraintType type;
    size_t participants[MAX_PARTICIPANTS];
    size_t participantCount;
    ProjectConstraintFn ProjectFn;

    // 
    Vector2 surfaceNormal;
    Vector2 entryPoint;

    // int nj               // carrdinality
    // ConstraintFn *Cj     // scalar constraint function
    // indices              // set of indices
    // float kj             // stiffness parameter
    // type                 // unilateral (Cj(xi...xn) = 0) or bilateral (Cj(xi...xn) <= 0)
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
    Force **forces_;
    ParticlePool *particles_;
}ParticleSystem;

// declare extern variables
// -----------------
extern ParticleProps defaultParticleProps;

// Private methods
// -----------------
static Vector2 CalculateForces_(Vector2 pi, Vector2 vi, float mi, Force **forces);
static Vector2 CalculateEntryPoint_(Vector2 position, Vector2 velocity, Vector2 surfacePoint, Vector2 surfaceNormal);

static size_t GenerateCollisionConstraints_(ParticleSystem *system);
static void HandleBoundaryCollisions_(ParticleSystem *system);

static void UpdateParticlesLife_(ParticleSystem *system, float deltaTime);
static void UpdateParticleAttributes_(ParticleSystem *system);
static void UpdateParticlesMotion_(ParticleSystem *system, float deltaTime);

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
void ShutdownParticleRender();

void DrawParticlesInstanced(const ParticleSystem *system);

void AddSelfCollisionConstraint(ParticleSystem *system, size_t i, size_t j);
void AddSurfaceCollisionConstraint(ParticleSystem *system, size_t i, Vector2 sn, Vector2 ep);
void AddDistanceConstraint(ParticleSystem *system, size_t i, size_t j);
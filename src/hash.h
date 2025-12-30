#pragma once
#include <stdint.h>
#include "config.h"

#define X_Prim 92837111
#define Y_Prim 689287499

// Forward declaration
typedef struct ParticlePool ParticlePool; 

typedef struct Hash
{
    bool isCleared;
    float spacing;
    uint32_t tableSize;

    uint32_t cellCount[MAX_PARTICLE_COUNT];
    size_t cellStart[MAX_PARTICLE_COUNT];
    size_t denseGrid[MAX_PARTICLE_COUNT];

    size_t *queryResults;
}Hash;

// Private methods
// -----------------
static inline int CalculateCellCoord_(float coord, float spacing)
{
    return (int)floor(coord / spacing);
}

static inline size_t HashCoords_(int xi, int yi, uint32_t tableSize)
{
    return abs((xi * X_Prim) ^ (yi * Y_Prim)) % tableSize;
}

// Interface methods
// -----------------
Hash* ConstructHash(float s);
void DestructHash(Hash *this);

void ClearHash(Hash *this);
void FillHash(Hash *this, const ParticlePool *particles);
size_t QueryHash(Hash *this, const ParticlePool *particles, size_t pi, uint32_t range);
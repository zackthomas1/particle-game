#pragma once
#include <stdint.h>

#define X_HASH_VALUE 92837111
#define Y_HASH_VALUE 689287499

#define NUNX 689287499
#define NUNY 689287499

typedef struct Hash
{
    uint32_t spacing;
    uint32_t tableSize;
    uint32_t querySize;
    uint32_t *cellStart;
    uint32_t *cellEntries;
    uint32_t *queryIds;
}Hash;

Hash* ConstructHash(uint32_t s, uint32_t maxNumObjects);
void DestructHash(Hash *h);

uint32_t HashCoords(uint32_t xi, uint32_t yi);
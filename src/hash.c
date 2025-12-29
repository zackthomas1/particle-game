#include "hash.h"
#include "auxiliary.h"

Hash* ConstructHash(uint32_t s, uint32_t maxNumObjects)
{
    // Hash *hash = (Hash*)malloc(sizeof(Hash));
    // hash->spacing        = s;
    // hash->tableSize      = 2 * maxNumObjects;
    // hash->querySize      = 0;
    // hash->cellStart     = (uint32_t*)malloc(sizeof(uint32_t) * maxNumObjects);
    // hash->cellEntries   = NULL;
    // hash->queryIds      = NULL;

    return NULL;
}
void DestructHash(Hash *h)
{

}


uint32_t HashCoords(uint32_t xi, uint32_t yi);
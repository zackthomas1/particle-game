#include "pch.h"
#include "hash.h"

#include  "particle.h"

Hash* ConstructHash(float s)
{
    Hash *hash = (Hash*)malloc(sizeof(Hash));

    hash->isCleared = true;
    hash->spacing   = s;
    hash->tableSize = MAX_PARTICLE_COUNT;

    for(size_t i = 0; i < hash->tableSize; i++)
    {
        hash->cellCount[i]  = 0;
        hash->cellStart[i]  = 0;
        hash->denseGrid[i]  = 0;
    }
    
    hash->queryResults = NULL;

    return hash;
}

void DestructHash(Hash *this)
{
    free(this);
}

void ClearHash(Hash *this)
{
    this->isCleared = true;

    for(size_t i = 0; i < this->tableSize; i++)
    {
        this->cellCount[i] = 0;
        this->cellStart[i] = 0;
        this->denseGrid[i] = 0;
    }

    arrsetlen(this->queryResults, 0);
}

void FillHash(Hash *this, const ParticlePool *particles)
{
    PASSERT(this->isCleared, LOG_WARNING, "Spatial Hash Map not cleared, before filling. ")
    if(!(this->isCleared)) { ClearHash(this); }

    //
    for(size_t i = 0; i < this->tableSize; i++)
    {

        uint32_t cell = HashCoords_(
            CalculateCellCoord_(particles->pPositions[i].x, this->spacing),
            CalculateCellCoord_(particles->pPositions[i].y, this->spacing),
            this->tableSize);
        this->cellCount[cell] += 1;
    }

    //
    uint32_t partialSum = 0; 
    for(size_t i = 0; i < this->tableSize; i++)
    {
        partialSum += this->cellCount[i];
        this->cellStart[i] += partialSum;
    }

    //
    for(size_t i = 0; i < this->tableSize; i++)
    {
        uint32_t cell = HashCoords_(
            CalculateCellCoord_(particles->pPositions[i].x, this->spacing),
            CalculateCellCoord_(particles->pPositions[i].y, this->spacing),
            this->tableSize);
        size_t index = --(this->cellStart[cell]);
        this->denseGrid[index] = i;
    }

    this->isCleared = false;
}

size_t QueryHash(Hash *this, const ParticlePool *particles, size_t pi, uint32_t range)
{
    int x0 = CalculateCellCoord_(particles->pPositions[pi].x - range, this->spacing);
    int y0 = CalculateCellCoord_(particles->pPositions[pi].y - range, this->spacing);

    int x1 = CalculateCellCoord_(particles->pPositions[pi].x + range, this->spacing);
    int y1 = CalculateCellCoord_(particles->pPositions[pi].y + range, this->spacing);

    for(size_t xi = x0; xi <= x1; xi++)
    {
        for(size_t yi = y0; yi <= y1; yi++)
        {
            size_t h = HashCoords_(xi, yi, this->tableSize);
            size_t start = this->cellStart[h];
            size_t end = start + this->cellCount[h];
        
            for(size_t i = start; i < end; i++)
            {
                arrput(this->queryResults, this->denseGrid[i]);
            }
        }
    }
    return arrlenu(this->queryResults);
}

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
    arrsetcap(hash->queryResults, hash->tableSize);

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

    // count the total number of particles in each cell
    for(size_t i = 0; i < particles->activeCount; i++)
    {
        float x = particles->pPositions[i].x, y = particles->pPositions[i].y;
        // PASSERT((x > EPSILON && y > EPSILON), LOG_ERROR, "Particle position less than 0.");

        uint32_t cell = HashCoords_(
            CalculateCellCoord_(x, this->spacing),
            CalculateCellCoord_(y, this->spacing),
            this->tableSize);
        PASSERT((cell >= 0 && cell < this->tableSize), LOG_ERROR, "Cell index out of range.");
        this->cellCount[cell] += 1;
    }

    // Computing a running partial sum of the total number of particles in the
    // previously traversed cells. In each index store the total number of particles 
    // seen so far.
    uint32_t partialSum = 0; 
    for(size_t i = 0; i < this->tableSize; i++)
    {
        partialSum += this->cellCount[i];
        this->cellStart[i] += partialSum;
    }

    // Using the previously calculate partial sums to determine the index 
    // of each particle in the dense array of particles. When complete the cellStart 
    // array which previously contained the partial sums will contain the start index 
    // of cell in the dense array
    for(size_t i = 0; i < particles->activeCount; i++)
    {
        uint32_t cell = HashCoords_(
            CalculateCellCoord_(particles->pPositions[i].x, this->spacing),
            CalculateCellCoord_(particles->pPositions[i].y, this->spacing),
            this->tableSize);
        PASSERT((cell >= 0 && cell < this->tableSize), LOG_ERROR, "Cell index out of range.");
        size_t index = --(this->cellStart[cell]);
        this->denseGrid[index] = i;
    }

    this->isCleared = false;
}

size_t QueryHashPoint(Hash *this, Vector2 position, float range)
{
    int xMin = position.x - range;
    int yMin = position.y - range;
    int xMax = position.x + range;
    int yMax = position.y + range;

    return QueryHashRange(this, xMin, xMax, yMin, yMax);
}

size_t QueryHashRange(Hash *this, float xMin, float xMax, float yMin, float yMax)
{
    PASSERT((xMin <= xMax), LOG_WARNING, "Spatial hash query invalid range. x-max is less than x-min.");
    PASSERT((yMin <= yMax), LOG_WARNING, "Spatial hash query invalid range. y-max is less than y-min.");

    arrsetlen(this->queryResults, 0);

    int x0 = CalculateCellCoord_(xMin, this->spacing);
    int y0 = CalculateCellCoord_(yMin, this->spacing);

    int x1 = CalculateCellCoord_(xMax, this->spacing);
    int y1 = CalculateCellCoord_(yMax, this->spacing);

    for(int xi = x0; xi <= x1; xi++)
    {
        for(int yi = y0; yi <= y1; yi++)
        {
            size_t h = HashCoords_(xi, yi, this->tableSize);
            PASSERT((h >= 0 && h < this->tableSize), LOG_ERROR, "Cell index out of range.");

            size_t start = this->cellStart[h];
            size_t end = start + this->cellCount[h];
            PASSERT((start <= end), LOG_ERROR, "end index is less than start index");
        
            for(size_t i = start; i < end; i++)
            {
                arrput(this->queryResults, this->denseGrid[i]);
            }
        }
    }
    return arrlenu(this->queryResults);
}

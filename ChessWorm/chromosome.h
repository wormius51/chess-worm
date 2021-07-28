#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include "types.h"
#include "constants.h"

Chromosome MakeChromosome ();
Chromosome ChildChromosome (Chromosome* father);
float ChromosomeMoveEval (Chromosome* chromosome, Move move);
void CopyChromosome (Chromosome* original, Chromosome* copy);
void LogChromosome (Chromosome* chromosome);

#endif // !CHROMOSOME_H


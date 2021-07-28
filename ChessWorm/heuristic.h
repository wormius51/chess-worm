#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "types.h"

int MoveSortingHeuristic (void* context ,const void* a, const void* b);
HeuristicContext MakeHeuristicContext (Position* position, Move* movesBuffer);
float ForcingMoveHeuristic(Move move, HeuristicContext* context);
void UpdateHistoryTable (Move move, HeuristicContext* context);

#endif // !HEURISTIC_H


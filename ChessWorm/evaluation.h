#ifndef EVALUATION_H
#define EVALUATION_H

#include "types.h"

float PieceValue (char piece);
float AlphaBeta (Position* position, Move* movesBuffer, int depth, float alpha, float beta, int* endDepth, float* stylePoints, HeuristicContext* context);
void ShuffleMoves(Move* moves, int moveCount);
float MaterialBalance (Position* position);
#endif // !EVALUATION_H


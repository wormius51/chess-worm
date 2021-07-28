#include "evaluation.h"
#include "moves.h"
#include "heuristic.h"
#include "multithreadEval.h"
#include "style.h"
#include <math.h>
#include <stdlib.h>
#include <Windows.h>

void ShuffleMoves (Move* moves, int moveCount);

float AlphaBeta (Position* position, Move* movesBuffer, int depth, float alpha, float beta, int* endDepth, float* stylePoints, HeuristicContext* context) {
	char colorSign = (position->flags & WHITE_TURN_FLAG) ? 1 : -1;
	Move moves[MAX_MOVES];
	int movesCount = WriteAllFilteredMoves(moves, 0, position);
	if (movesCount <= 0) {
		(*endDepth) = depth;
		(*stylePoints) = StylePoints(context->previousMove, position);
		return CheckCheck(position) ? 
		(colorSign == 1 ? -INFINITY : INFINITY) 
		: 0.0;
	}
	if (depth <= 0) {
		position->flags ^= WHITE_TURN_FLAG;
		int opponentMovesCount = WriteAllFilteredMoves(moves, 0, position);
		position->flags ^= WHITE_TURN_FLAG;
		return colorSign * 
			(((movesCount - opponentMovesCount) / (float)(movesCount + opponentMovesCount)) * MOVES_EVAL_VALUE +
				MaterialBalance(position) * MATERIAL_EVAL_VALUE);
	}

	context->position = position;
	context->depth = depth;
	ShuffleMoves(moves, movesCount);
	qsort_s(moves, movesCount, sizeof(Move), MoveSortingHeuristic, context);
	float eval = -INFINITY * colorSign;
	Move line[CALCULATION_DEPTH];
	Move bestLine[CALCULATION_DEPTH];
	Move bestMove = moves[0];
	int bestEndDepth = 10000000;
	if (movesCount > MAX_CANDIDATE_MOVES)
		movesCount = MAX_CANDIDATE_MOVES;
	for (int i = 0; i < movesCount; i++) {
		int innerEndDepth = 0;
		context->previousMove = moves[i];
		Position afterMove = PositionAfterMove(moves[i], position);
		float innerStylePoints = -INFINITY;
		float afterEval = AlphaBeta(&afterMove, line, depth - 1, alpha, beta, &innerEndDepth, &innerStylePoints, context);
		if (((colorSign == 1) ? (afterEval >= eval) : (afterEval <= eval)) && (innerStylePoints >= (*stylePoints))) {
			// If this player is getting mated and it's
			// a shorter checkmate then found, skip it.
			if (!(((colorSign == 1) ? (afterEval == -INFINITY) : (afterEval == INFINITY)) &&
				(bestEndDepth < innerEndDepth))) {
				eval = afterEval;
				bestMove = moves[i];
				for (int j = innerEndDepth; j < depth - 1; j++) {
					bestLine[j] = line[j];
				}
				bestEndDepth = innerEndDepth;
				(*stylePoints) = innerStylePoints;
			}
			
		}
		if ((colorSign == 1) ? (eval >= beta) : (eval <= alpha)) {
			context->position = position;
			context->depth = depth;
			UpdateHistoryTable(moves[i], context);
			break;
		}
		if (colorSign == 1)
			alpha = fmaxf(alpha, eval);
		else
			beta = fminf(beta, eval);
		if (globalEvalState.isInterapted) {
			if (i == 0)
				eval = AlphaBeta(position, movesBuffer, 0, alpha, beta, &innerEndDepth, stylePoints, context);
			break;
		}
	}
	movesBuffer[depth - 1] = bestMove;
	for (int j = 0; j < depth - 1; j++) {
		movesBuffer[j] = bestLine[j];
	}
	(*endDepth) = bestEndDepth;
	return eval;
}

void ShuffleMoves(Move* moves, int moveCount) {
	for (int i = 0; i < moveCount; i++) {
		int j = rand() % moveCount;
		Move temp = moves[i];
		moves[i] = moves[j];
		moves[j] = temp;
	}
}

float MaterialBalance (Position* position) {
	float balance = 0;
	for (int i = 0; i < BOARD_HEIGHT * BOARD_WIDTH; i++) {
		float pieceValue = PieceValue(position->squares[i]);
		if (pieceValue == INFINITY || pieceValue == 0)
			continue;
		char isBlack = position->squares[i] & LOWERCASE_FLAG;
		balance += pieceValue *
			(isBlack ? -1 : 1);
	}
	return balance;
}
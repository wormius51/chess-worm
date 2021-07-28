#include "multithreadEval.h"
#include "position.h"
#include "heuristic.h"
#include "evaluation.h"
#include "moves.h"
#include <math.h>
#include <stdio.h>

EvalState MakeEvalState () {
	EvalState evalState;
	evalState.depth = CALCULATION_DEPTH;
	evalState.endDepth = 0;
	evalState.eval = 0;
	evalState.isComplete = 0;
	evalState.isInterapted = 0;
	return evalState;
}

DWORD WINAPI MultiThreadEval (LPVOID param) {
	EvalState* evalState = (EvalState*)param;
	evalState->isComplete = 0;
	DWORD threadIds[MAX_CANDIDATE_MOVES];
	HANDLE threadHandles[MAX_CANDIDATE_MOVES];
	EvalState evalStates[MAX_CANDIDATE_MOVES];
	Position afterPositions[MAX_CANDIDATE_MOVES];
	Move moves[MAX_MOVES];
	int movesCount = WriteAllFilteredMoves(moves, 0, evalState->position);
	HeuristicContext context = MakeHeuristicContext(evalState->position, evalState->movesBuffer);
	ShuffleMoves(moves, movesCount);
	qsort_s(moves, movesCount, sizeof(Move), MoveSortingHeuristic, &context);
	if (movesCount > MAX_CANDIDATE_MOVES)
		movesCount = MAX_CANDIDATE_MOVES;
	for (int i = 0; i < movesCount; i++)  {
		evalStates[i] = MakeEvalState();
		evalStates[i].depth = evalState->depth - 1;
		afterPositions[i] = PositionAfterMove(moves[i], evalState->position);
		evalStates[i].position = afterPositions + i;
		evalStates[i].previousMove = moves[i];
		threadHandles[i] = CreateThread(NULL, 0, ThreadEval, evalStates + i, 0, threadIds + i);
		evalStates[i].threadHandle = threadHandles[i];
	}
	WaitForMultipleObjects(movesCount, threadHandles, TRUE, INFINITE);
	int bestEvalStateIndex = 0;
	for (int i = 0; i < movesCount; i++) {
		if (threadHandles[i] == 0)
			continue;
		CloseHandle(threadHandles[i]);
		if ((evalState->position->flags & WHITE_TURN_FLAG) ? 
			(evalStates[i].eval < evalStates[bestEvalStateIndex].eval) :
			evalStates[i].eval > evalStates[bestEvalStateIndex].eval) 
		continue;
		if (evalStates[i].eval == evalStates[bestEvalStateIndex].eval) {
			if (evalStates[i].endDepth < evalStates[bestEvalStateIndex].endDepth)
				continue;
			if (evalStates[i].stylePoints < evalStates[bestEvalStateIndex].stylePoints)
				continue;
		}
		bestEvalStateIndex = i;
	}
	evalState->eval = evalStates[bestEvalStateIndex].eval;
	evalState->endDepth = evalStates[bestEvalStateIndex].endDepth;
	evalState->stylePoints = evalStates[bestEvalStateIndex].stylePoints;
	for (int i = evalState->endDepth; i < CALCULATION_DEPTH - 1; i++) {
		evalState->movesBuffer[i] = evalStates[bestEvalStateIndex].movesBuffer[i];
	}
	evalState->movesBuffer[CALCULATION_DEPTH - 1] = moves[bestEvalStateIndex];
	evalState->isComplete = 1;
	return 0;
}


DWORD WINAPI ThreadEval (LPVOID param) {
	EvalState* evalState = (EvalState*)param;
	float window = INFINITY;
	float alpha = -window;
	float beta = window;
	Move line[CALCULATION_DEPTH];
	HeuristicContext context = MakeHeuristicContext(evalState->position, line);
	context.depth = evalState->depth;
	int endDepth = 0;
	evalState->isComplete = 0;
	evalState->eval = -100;
	context.previousMove = evalState->previousMove;
	float stylePoints = -INFINITY;
	float eval = AlphaBeta(evalState->position, line, evalState->depth, alpha, beta, &endDepth, &stylePoints, &context);
	evalState->stylePoints = stylePoints;
	evalState->eval = eval;
	evalState->endDepth = endDepth;
	for (int i = 0; i < CALCULATION_DEPTH; i++)
		evalState->movesBuffer[i] = line[i];
	evalState->isComplete = 1;
	return 0;
}
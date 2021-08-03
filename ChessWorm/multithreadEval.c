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
	evalState.endDepth = CALCULATION_DEPTH;
	evalState.eval = 0;
	evalState.isComplete = 0;
	evalState.isInterapted = 0;
	evalState.threadHandle = NULL;
	return evalState;
}

int BestEvalStateIndex () {
	int bestEvalStateIndex = 0;
	for (int i = 0; i < MAX_CANDIDATE_MOVES; i++) {
		if (evalStates[i].threadHandle == NULL)
			break;
		if ((globalCurrentPosition.flags & WHITE_TURN_FLAG) ? 
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
	return bestEvalStateIndex;
}


DWORD WINAPI MultiThreadEval (LPVOID param) {
	nodesSearched = 0;
	EvalState* evalState = (EvalState*)param;
	evalState->isComplete = 0;
	evalState->position = &globalCurrentPosition;
	DWORD threadIds[MAX_CANDIDATE_MOVES];
	HANDLE threadHandles[MAX_CANDIDATE_MOVES];
	for (int i = 0; i < MAX_CANDIDATE_MOVES; i++)
		threadHandles[i] = NULL;
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
		evalState[i].endDepth = evalState->endDepth - 1;
		afterPositions[i] = PositionAfterMove(moves[i], evalState->position);
		evalStates[i].position = afterPositions + i;
		evalStates[i].previousMove = moves[i];
		threadHandles[i] = CreateThread(NULL, 0, ThreadEval, evalStates + i, 0, threadIds + i);
		evalStates[i].threadHandle = threadHandles[i];
	}
	WaitForMultipleObjects(movesCount, threadHandles, TRUE, INFINITE);
	for (int i = 0; i < movesCount; i++) {
		if (threadHandles[i] == NULL)
			continue;
		CloseHandle(threadHandles[i]);
	}
	int bestEvalStateIndex = BestEvalStateIndex();
	evalState->eval = evalStates[bestEvalStateIndex].eval;
	evalState->endDepth = evalStates[bestEvalStateIndex].endDepth;
	evalState->stylePoints = evalStates[bestEvalStateIndex].stylePoints;
	for (int i = evalState->endDepth; i < CALCULATION_DEPTH; i++) {
		evalState->movesBuffer[i] = evalStates[bestEvalStateIndex].movesBuffer[i];
	}
	evalState->isComplete = 1;
	return 0;
}


DWORD WINAPI ThreadEval (LPVOID param) {
	EvalState* evalState = (EvalState*)param;
	float window = INFINITY;
	float alpha = -window;
	float beta = window;
	evalState->movesBuffer[evalState->depth] = evalState->previousMove;
	HeuristicContext context = MakeHeuristicContext(evalState->position, evalState->movesBuffer);
	context.depth = evalState->depth;
	evalState->isComplete = 0;
	evalState->eval = -100;
	context.previousMove = evalState->previousMove;
	evalState->stylePoints = -INFINITY;
	for (int i = 0; i < evalState->depth; i++) {
		context.depth = i;
		evalState->eval = AlphaBeta(evalState->position, evalState->movesBuffer + (evalState->depth - i - 1), i, alpha, beta, &(evalState->eval), &(evalState->endDepth), &(evalState->stylePoints), &context);
		evalState->endDepth = evalState->depth - i;
	}
	//evalState->eval = AlphaBeta(evalState->position, evalState->movesBuffer, evalState->depth - 1, alpha, beta, &(evalState->eval), &(evalState->endDepth), &(evalState->stylePoints), &context);
	evalState->isComplete = 1;
	return 0;
}
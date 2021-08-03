#include "uci.h"
#include "util.h"
#include "constants.h"
#include "multithreadEval.h"
#include "position.h"
#include "moves.h"
#include "evaluation.h"
#include <stdio.h>
#include <math.h>

void Identifiy ();
void UCIOK ();
void Isready ();
void SetPosition (char* input);
void SetEvalState ();
void StartEval ();
void StopEval ();
void BestMove ();
//void CurrMove ();
DWORD WINAPI InfoLoop (LPVOID param);

HANDLE infoLoopThreadHandle = NULL;
DWORD infoLoopThreadId;

void TakeInput (char* input) {
	if (StringStartsWith(input, "uci")) {
		Identifiy();
		SetEvalState();
		UCIOK();
	}
	else if (StringStartsWith(input, "isready")) {
		Isready();
	}
	else if (StringStartsWith(input, "position")) {
		SetPosition(input);
	}
	else if (StringStartsWith(input, "go")) {
		StartEval();
	}
	else if (StringStartsWith(input, "stop")) {
		StopEval();
	}
}

void Identifiy() {
	printf("id name %s\nid author %s\n", NAME, AUTHOR);
}

void UCIOK () {
	printf("uciok\n");
}

void Isready () {
	if (globalEvalState.isComplete)
		printf("readyok\n");
}

void SetEvalState () {
	globalEvalState = MakeEvalState();
	globalEvalState.depth = CALCULATION_DEPTH;
	globalEvalState.position = &globalCurrentPosition;
	globalEvalState.isComplete = 1;
}

void SetPosition (char* input) {
	int index = 0;
	char spacesFound = 0;
	char c = 0;
	while ((c = input[index++]) != NULL) {
		if (c == ' ') {
			if (spacesFound == 0)
				globalCurrentPosition = ParseFEN(&(input[index]));
			else if (spacesFound > 1) {
				Move move = ParseMove(&(input[index]));
				globalCurrentPosition = PositionAfterMove(move, &globalCurrentPosition);
			}
			spacesFound++;
		}
	}
	globalEvalState.position = &globalCurrentPosition;
}

void StartEval () {
	globalEvalState = MakeEvalState();
	DWORD threadId = 0;
	HANDLE threadHandle;
	threadHandle = CreateThread(NULL, 0, MultiThreadEval, &globalEvalState, 0, &threadId);
	if (threadHandle == NULL) {
		printf ("\nProblem in eval thread.\n");
		exit(-1);
	}
	globalEvalState.threadHandle = threadHandle;
	if (infoLoopThreadHandle == NULL) {
		infoLoopThreadHandle = CreateThread(NULL, 0, InfoLoop, &globalEvalState, 0, &infoLoopThreadId);
		if (infoLoopThreadHandle == NULL) {
			printf ("\nProblem in info thread.\n");
			exit(-1);
		}
	}
}

void StopEval () {
	if (globalEvalState.threadHandle == NULL)
		return;
	globalEvalState.isInterapted = 1;
	WaitForSingleObject(globalEvalState.threadHandle, INFINITE);
	CloseHandle(globalEvalState.threadHandle);
	if (infoLoopThreadHandle != NULL) {
		CloseHandle(infoLoopThreadHandle);
		infoLoopThreadHandle = NULL;
	}
}

void BestMove () {
	int bestEvalStateIndex = BestEvalStateIndex();
	int depth = evalStates[bestEvalStateIndex].depth;
	printf("bestmove ");
	LogMove(evalStates[bestEvalStateIndex].previousMove);
	printf(" ponder ");
	LogMove(evalStates[bestEvalStateIndex].movesBuffer[depth - 2]);
	printf("\n");
}

DWORD WINAPI InfoLoop (LPVOID param) {
	EvalState* evalState = (EvalState*)param;
	while (evalState->isComplete == 0) {
		Sleep(INFO_SLEEP_TIME);
		int bestEvalStateIndex = BestEvalStateIndex();
		int reachedDepth = evalStates[bestEvalStateIndex].depth - evalStates[bestEvalStateIndex].endDepth;
		int score = floorf(evalStates[bestEvalStateIndex].eval * 100);
		printf("info score cp %i depth %i nodes %i ", score, reachedDepth, nodesSearched);
		evalStates[bestEvalStateIndex].movesBuffer[evalStates[bestEvalStateIndex].depth - 1] = evalStates[bestEvalStateIndex].previousMove;
		if (reachedDepth > 0) {
			printf("pv ");
			for (int i = evalStates[bestEvalStateIndex].depth - 1; i >= evalStates[bestEvalStateIndex].endDepth; i--) {
				LogMove(evalStates[bestEvalStateIndex].movesBuffer[i]);
				printf(" ");
			}
		}
		printf("\n");
	}
	BestMove();
	return 0;
}
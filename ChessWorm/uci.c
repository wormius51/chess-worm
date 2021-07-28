#include "uci.h"
#include "util.h"
#include "constants.h"
#include "multithreadEval.h"
#include "position.h"
#include "moves.h"
#include <stdio.h>

void Identifiy ();
void UCIOK ();
void Isready ();
void SetPosition (char* input);
void SetEvalState ();
void StartEval ();
void StopEval ();
void BestMove ();
DWORD WINAPI InfoLoop (LPVOID param);

HANDLE infoLoopThreadHandle;
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
	/*
	infoLoopThreadHandle = CreateThread(NULL, 0, InfoLoop, &globalEvalState, 0, &infoLoopThreadId);
	if (infoLoopThreadHandle == NULL) {
		printf ("\nProblem in info thread.\n");
		exit(-1);
	}
	*/
}

void StopEval () {
	if (globalEvalState.threadHandle == NULL)
		return;
	globalEvalState.isInterapted = 1;
	WaitForSingleObject(globalEvalState.threadHandle, INFINITE);
	CloseHandle(globalEvalState.threadHandle);
	BestMove();
}

void BestMove () {
	printf("bestmove ");
	LogMove(globalEvalState.movesBuffer[CALCULATION_DEPTH - 1]);
	printf(" ponder ");
	LogMove(globalEvalState.movesBuffer[CALCULATION_DEPTH - 2]);
	printf("\n");
}

DWORD WINAPI InfoLoop (LPVOID param) {
	EvalState* evalState = (EvalState*)param;
	while (evalState->isComplete == 0) {
		Sleep(INFO_SLEEP_TIME);
	}
	return 0;
}
#include <stdio.h>
#include "position.h"
#include "moves.h"
#include "evaluation.h"
#include "heuristic.h"
#include "multithreadEval.h"
#include "uci.h"
#include "util.h"
#include <math.h>

void LogBoard (Position position);
void LogMovesArray (Move* moves, int depth, int endDepth );
void StartEval (char* fen);
void StopEval ();
void LogResult ();

main () {
	char playing = 1;
	/*char fen[FEN_MAX_LENGTH];
	while (playing) {
		fgets(fen, FEN_MAX_LENGTH, stdin);
		if (StringsEqual(fen, "quit"))
			playing = 0;
		else if (StringsEqual(fen, "go"))
			StopEval();
		else
			StartEval (fen);
	}*/

	char input[INPUT_MAX_LENGTH];
	while (playing) {
		fgets(input, INPUT_MAX_LENGTH, stdin);
		if (StringsEqual(input, "quit"))
			playing = 0;
		else
			TakeInput(input);
	}
}

void StartEval (char* fen) {
	globalEvalState = MakeEvalState();
	globalCurrentPosition = ParseFEN(fen);
	globalEvalState.depth = CALCULATION_DEPTH;
	globalEvalState.position = &globalCurrentPosition;
	DWORD threadId = 0;
	HANDLE threadHandle;
	threadHandle = CreateThread(NULL, 0, MultiThreadEval, &globalEvalState, 0, &threadId);
	if (threadHandle == NULL) {
		printf ("\nProblem in eval thread.\n");
		exit(-1);
	}
	globalEvalState.threadHandle = threadHandle;
}

void StopEval() {
	if (globalEvalState.threadHandle == NULL)
		return;
	globalEvalState.isInterapted = 1;
	WaitForSingleObject(globalEvalState.threadHandle, INFINITE);
	CloseHandle(globalEvalState.threadHandle);
	LogResult();
}

void LogResult() {
	
	if (globalEvalState.eval == INFINITY)
		printf("\nWhite mates in %i", (CALCULATION_DEPTH - globalEvalState.endDepth + 1) / 2);
	else if (globalEvalState.eval == -INFINITY)
		printf("\nBlack mates in %i", (CALCULATION_DEPTH - globalEvalState.endDepth + 1) / 2);
	else
		printf("\nEvaluation: %f\n", globalEvalState.eval);
	printf("\nMoves:\n");
	LogMovesArray(globalEvalState.movesBuffer, CALCULATION_DEPTH, globalEvalState.endDepth);
	printf("\n");
	
}

void LogMovesArray (Move* moves, int depth, int endDepth) {
	for (int i = depth - 1; i >= endDepth; i--) {
		LogMove(moves[i]);
		printf("\n");
	}
}

void LogBoard (Position position) {
	char positionString[BOARD_HEIGHT * (BOARD_WIDTH + 1) + 1];
	int positionStringIndex = 0;
	for (int rank = BOARD_HEIGHT - 1; rank >= 0; rank--) {
		for (int file = 0; file < BOARD_WIDTH; file++) {
			int squareIndex = GetIndexOfSquare(file, rank);
			char squareChar = position.squares[squareIndex];
			positionString[positionStringIndex++] = squareChar;
		}
		positionString[positionStringIndex++] = '\n';
	}
	positionString[positionStringIndex++] = 0; // 0 for end of string
	printf("%s\n", positionString);
	if (position.flags & WHITE_TURN_FLAG)
		printf("White to move");
	else
		printf("Black to move");
	if (position.flags & WHITE_CASTLING_RIGHT_SHORT)
		printf("\nWhite can castle short with the %c rook", 'a' + position.whiteCastleShortFile);
	if (position.flags & WHITE_CASTLING_RIGHT_LONG)
		printf("\nWhite can castle long with the %c rook", 'a' + position.whiteCastleLongFile);
	if (position.flags & BLACK_CASTLING_RIGHT_SHORT)
		printf("\nBlack can castle short with the %c rook", 'a' + position.blackCastleShortFile);
	if (position.flags & BLACK_CASTLING_RIGHT_LONG)
		printf("\nBlack can castle long with the %c rook", 'a' + position.blackCastleLongFile);
	printf("\nHalfMove Clock: %i", position.halfMoveClock);
	printf("\nFullMove Number: %i", position.fullMoveNumber);
}
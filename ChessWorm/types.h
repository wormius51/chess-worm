#ifndef TYPES_H
#define TYPES_H

#include "constants.h"
#include <Windows.h>

typedef struct position {
	char squares[BOARD_HEIGHT * BOARD_WIDTH];
	char flags;
	int halfMoveClock;
	int fullMoveNumber;
	char whiteCastleShortFile;
	char whiteCastleLongFile;
	char blackCastleShortFile;
	char blackCastleLongFile;
} Position;

typedef struct move {
	char pieceChar;
	char captureChar;
	int sourceFile;
	int sourceRank;
	int destinationFile;
	int destinationRank;
	// Pawns can promote to different stuff on the same square.
	// Castling and such...
	char special; 
	float heuristicValue;
	
} Move;

typedef struct heuristicContext {
	int depth;
	Position* position;
	Move previousMove;
	Move* movesBuffer;
	float heuristicsWeights[HEURISTICS_COUNT];
	int historyTable[PIECE_TYPE_COUNT][BOARD_HEIGHT * BOARD_WIDTH];
	Move counterMoves[PIECE_TYPE_COUNT][BOARD_HEIGHT * BOARD_WIDTH];
} HeuristicContext;

typedef float (*HeuristicFunction)(Move move, HeuristicContext* context);

typedef struct chromosome {
	float performance;
	float params[CHROMOSOME_POLINOM_DEGREE * CHROMOSOME_ARGUMENTS];
} Chromosome;

typedef struct evalState {
	float eval;
	float stylePoints;
	int depth;
	int endDepth;
	char isComplete;
	char isInterapted;
	Move movesBuffer[CALCULATION_DEPTH];
	Move previousMove;
	HANDLE threadHandle;
	Position* position;
} EvalState;

#endif // !TYPES_H


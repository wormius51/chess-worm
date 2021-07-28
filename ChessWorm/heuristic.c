#include "heuristic.h"
#include "moves.h"
#include "position.h"
#include "evaluation.h"
#include "multithreadEval.h"
#include <math.h>

float PieceValue (char piece);
int PieceIndex (char piece);
float ForcingMoveValue (Move move, Position* position, int depth);

float MoveHeuristicValue (Move move, HeuristicContext* context);

float KillerHeuristic (Move move, HeuristicContext* context);
float ForcingMoveHeuristic (Move move, HeuristicContext* context);
float ShallowSearchHeuristic (Move move, HeuristicContext* context);
float ResponseCountHeuristic (Move move, HeuristicContext* context);
float PieceLiberationHeuristic (Move move, HeuristicContext* context);
float HistoryHeuristic (Move move, HeuristicContext* context);
float CounterMoveHeuristic (Move move, HeuristicContext* context);

HeuristicContext MakeHeuristicContext(Position* position, Move* movesBuffer) {
	HeuristicContext context;
	context.depth = CALCULATION_DEPTH;
	context.position = position;
	context.movesBuffer = movesBuffer;
	context.heuristicsWeights[0] = 1;
	context.heuristicsWeights[1] = 8;
	context.heuristicsWeights[2] = 1;
	context.heuristicsWeights[3] = 1;
	context.heuristicsWeights[4] = 1;
	context.heuristicsWeights[5] = 1;
	return context;
}

int MoveSortingHeuristic (void* context, const void* a, const void* b) {
	Move* moveA = (Move*)a;
	Move* moveB = (Move*)b;
	HeuristicContext* heuristicContext = (HeuristicContext*)context;
	if (moveA->heuristicValue == 0)
		moveA->heuristicValue = MoveHeuristicValue(*moveA, heuristicContext);
	if (moveB->heuristicValue == 0)
		moveB->heuristicValue = MoveHeuristicValue(*moveB, heuristicContext);
	if (moveA->heuristicValue == moveB->heuristicValue)
		return 0;
	return (moveA->heuristicValue > moveB->heuristicValue) ? -1 : 1;
}

float MoveHeuristicValue(Move move, HeuristicContext* context) {
	HeuristicFunction functions[HEURISTICS_COUNT];
	functions[0] = KillerHeuristic;
	functions[1] = ForcingMoveHeuristic;
	functions[2] = ResponseCountHeuristic;
	functions[3] = HistoryHeuristic;
	functions[4] = PieceLiberationHeuristic;
	functions[5] = CounterMoveHeuristic;
	float value = 0;
	for (int i = 0; i < HEURISTICS_COUNT; i++) {
		float heuristicValue = (*(functions[i]))(move, context);
		value += context->heuristicsWeights[i] / (1 + expf(-heuristicValue));
	}
	return value;
}

float KillerHeuristic(Move move, HeuristicContext* context) {
	Move killerMove = context->movesBuffer[context->depth - 1];
	return MovesEqual(move, killerMove) ? INFINITY : -INFINITY;
}

float ForcingMoveHeuristic(Move move, HeuristicContext* context) {
	return ForcingMoveValue(move, context->position, FORCING_MOVE_DEPTH);
}

float ShallowSearchHeuristic(Move move, HeuristicContext* context) {
	Position afterMove = PositionAfterMove(move, context->position);
	int endDepth = 0;
	Move moveBuffer[CALCULATION_DEPTH];
	float stylePoints = -INFINITY;
	float eval = AlphaBeta(&afterMove, 0, moveBuffer, 0, 0, &endDepth, &stylePoints, context);
	return (context->position->flags & WHITE_TURN_FLAG) ? eval : -eval;
}

float ResponseCountHeuristic (Move move, HeuristicContext* context) {
	Position afterMove = PositionAfterMove(move, context->position);
	Move moves[MAX_MOVES];
	int moveCount = WriteAllFilteredMoves(moves, 0, &afterMove);
	return -moveCount;
}

float HistoryHeuristic(Move move, HeuristicContext* context) {
	int pieceIndex = PieceIndex(move.pieceChar);
	int destinationIndex = GetIndexOfSquare(move.destinationFile, move.destinationRank);
	return context->historyTable[pieceIndex][destinationIndex];
}

float CounterMoveHeuristic (Move move, HeuristicContext* context) {
	int pieceIndex = PieceIndex(context->previousMove.pieceChar);
	int destinationIndex = GetIndexOfSquare(context->previousMove.destinationFile, context->previousMove.destinationRank);
	Move counterMove = context->counterMoves[pieceIndex][destinationIndex];
	return (MovesEqual(counterMove, move) ? INFINITY : -INFINITY);
}

float PieceLiberationHeuristic (Move move, HeuristicContext* context) {
	Move moves[MAX_PIECE_MOVES];
	int pieceMovesCount = WriteFilteredMoves(moves, 0, context->position, move.sourceFile, move.sourceRank);
	Position afterMove = PositionAfterMove(move, context->position);
	afterMove.flags ^= WHITE_TURN_FLAG;
	int movesAfterCount = WriteFilteredMoves(moves, 0, context->position, move.destinationFile, move.destinationRank);
	return (float)movesAfterCount / (float)pieceMovesCount;
}

float PieceValue (char piece) {
	piece |= LOWERCASE_FLAG;
	switch (piece) {
	case BLACK_PAWN:
		return 1;
	case EN_PASSANTABLE:
		return 1;
	case BLACK_KNIGHT:
		return 3;
	case BLACK_BISHOP:
		return 3.5;
	case BLACK_ROOK:
		return 5;
	case BLACK_QUEEN:
		return 9;
	case BLACK_KING:
		return INFINITY;
	default:
		return 0;
	}
}

int PieceIndex (char piece) {
	switch (piece) {
	case WHITE_PAWN:
		return 0;
	case WHITE_KNIGHT:
		return 1;
	case WHITE_BISHOP:
		return 2;
	case WHITE_ROOK:
		return 3;
	case WHITE_QUEEN:
		return 4;
	case WHITE_KING:
		return 5;
	case BLACK_PAWN:
		return 6;
	case BLACK_KNIGHT:
		return 7;
	case BLACK_BISHOP:
		return 8;
	case BLACK_ROOK:
		return 9;
	case BLACK_QUEEN:
		return 10;
	case BLACK_KING:
		return 11;
	default:
		return 0;
	}
}

void UpdateHistoryTable(Move move, HeuristicContext* context) {
	if (PieceValue(move.captureChar) != 0)
		return;
	int pieceIndex = PieceIndex(move.pieceChar);
	int destinationIndex = GetIndexOfSquare(move.destinationFile, move.destinationRank);
	context->historyTable[pieceIndex][destinationIndex] += 1 << context->depth;
	pieceIndex = PieceIndex(context->previousMove.pieceChar);
	destinationIndex = GetIndexOfSquare(context->previousMove.destinationFile, context->previousMove.destinationRank);
	context->counterMoves[pieceIndex][destinationIndex] = move;
}

float ForcingMoveValue(Move move, Position* position, int depth) {
	if (depth <= 0) {
		float captureValue = PieceValue(move.captureChar) * CAPTURE_MOVE_BONUS;
		float checkValue = IsMoveCheck(move, position) * CHECK_MOVE_BONUS;
		float promotionValue = 0;
		if (move.special != EN_PASSANTABLE) {
			promotionValue = PieceValue(move.special) * CAPTURE_MOVE_BONUS;
			if (promotionValue != 0)
				promotionValue -= 1;
		}
		return captureValue + checkValue + promotionValue;
	}
	Position afterMove = PositionAfterMove(move, position);
	afterMove.flags ^= WHITE_TURN_FLAG;
	Move moves[MAX_MOVES];
	int moveCount = WriteAllFilteredMoves(moves, 0, &afterMove);
	float value = 0;
	for (int i = 0; i < moveCount; i++)
		value -= ForcingMoveValue(moves[i], &afterMove, depth - 1);
	return value;
}
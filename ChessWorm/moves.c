#include "moves.h"
#include "evaluation.h"
#include <stdio.h>


Move MakeMove (int sourceFile, int sourceRank,
	int destinationFile, int destinationRank, char special) {
	Move move;
	move.sourceFile = sourceFile;
	move.sourceRank = sourceRank;
	move.destinationFile = destinationFile;
	move.destinationRank = destinationRank;
	move.special = special;
	move.captureChar = 0;
	move.pieceChar = 0;
	move.heuristicValue = 0;
	return move;
}

Move ParseMove (char* input) {
	Move move = MakeMove(
		input[0] - 'a',
		input[1] - '1',
		input[2] - 'a',
		input[3] - '1',
		input[4]
	);
	return move;
}

int WriteAllFilteredMoves (Move* buffer, int writeToIndex, Position* position) {
	int bufferLength = WriteAllMoves(buffer, writeToIndex, position);
	return FilterIllegalMove(buffer, bufferLength, position);
}

int WriteFilteredMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	int bufferLength = WriteMoves(buffer, writeToIndex, position, file, rank);
	return FilterIllegalMove(buffer, bufferLength, position);
}

int WriteAllMoves (Move* buffer, int writeToIndex, Position* position) {
	int bufferIndex = writeToIndex;
	for (int rank = 0; rank < BOARD_HEIGHT; rank++) {
		for (int file = 0; file < BOARD_WIDTH; file++) {
			bufferIndex += WriteMoves(buffer, bufferIndex, position, file, rank);
		}
	}
	for (int i = writeToIndex; i < bufferIndex; i++) {
		if (buffer[i].captureChar != 0)
			continue;
		int pieceIndex = GetIndexOfSquare(buffer[i].sourceFile, buffer[i].sourceRank);
		int captureIndex = GetIndexOfSquare(buffer[i].destinationFile, buffer[i].destinationRank);
		buffer[i].captureChar = position->squares[captureIndex];
		if (buffer[i].pieceChar == 0)
			buffer[i].pieceChar = position->squares[pieceIndex];
	}
	return bufferIndex - writeToIndex;
}

int WriteMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	int squareIndex = GetIndexOfSquare(file, rank);
	char squareChar = position->squares[squareIndex];
	// If it is black to move, use the black pieces, not white.
	if (!(position->flags & WHITE_TURN_FLAG))
		squareChar ^= LOWERCASE_FLAG;
	switch (squareChar) {
	case WHITE_KING:
		return WriteKingMoves(buffer, writeToIndex, position, file, rank);
	case WHITE_QUEEN:
		return WriteQueenMoves(buffer, writeToIndex, position, file, rank);
	case WHITE_ROOK:
		return WriteRookMoves(buffer, writeToIndex, position, file, rank);
	case WHITE_BISHOP:
		return WriteBishopMoves(buffer, writeToIndex, position, file, rank);
	case WHITE_KNIGHT:
		return WriteKnightMoves(buffer, writeToIndex, position, file, rank);
	case WHITE_PAWN:
		return WritePawnMoves(buffer, writeToIndex, position, file, rank);
	default:
		return 0;
	}
}

int WriteKingMoves(Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	int bufferIndex = writeToIndex;
	Move steps[8];
	steps[0] = MakeMove(file, rank, file, rank + 1, 0);
	steps[1] = MakeMove(file, rank, file + 1, rank + 1, 0);
	steps[2] = MakeMove(file, rank, file - 1, rank + 1, 0);
	steps[3] = MakeMove(file, rank, file + 1, rank, 0);
	steps[4] = MakeMove(file, rank, file - 1, rank, 0);
	steps[5] = MakeMove(file, rank, file, rank - 1, 0);
	steps[6] = MakeMove(file, rank, file + 1, rank - 1, 0);
	steps[7] = MakeMove(file, rank, file - 1, rank - 1, 0);
	for (int i = 0; i < 8; i++) {
		if (IsMoveAvailable(steps[i], position)) {
			buffer[bufferIndex++] = steps[i];
		}
	}
	bufferIndex += WriteCastlingMoves(buffer, bufferIndex, position, file, rank);
	return bufferIndex - writeToIndex;
}

int WriteQueenMoves(Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	int bufferIndex = writeToIndex;
	bufferIndex += WriteRookMoves(buffer, bufferIndex, position, file, rank);
	bufferIndex += WriteBishopMoves(buffer, bufferIndex, position, file, rank);
	return bufferIndex - writeToIndex;
}

int WriteRookMoves(Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	int bufferIndex = writeToIndex;
	bufferIndex += WriteLinearMoves(buffer, bufferIndex, position, file, rank, 0, 1);
	bufferIndex += WriteLinearMoves(buffer, bufferIndex, position, file, rank, 0, -1);
	bufferIndex += WriteLinearMoves(buffer, bufferIndex, position, file, rank, 1, 0);
	bufferIndex += WriteLinearMoves(buffer, bufferIndex, position, file, rank, -1, 0);
	return bufferIndex - writeToIndex;
}

int WriteBishopMoves(Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	int bufferIndex = writeToIndex;
	bufferIndex += WriteLinearMoves(buffer, bufferIndex, position, file, rank, 1, 1);
	bufferIndex += WriteLinearMoves(buffer, bufferIndex, position, file, rank, 1, -1);
	bufferIndex += WriteLinearMoves(buffer, bufferIndex, position, file, rank, -1, 1);
	bufferIndex += WriteLinearMoves(buffer, bufferIndex, position, file, rank, -1, -1);
	return bufferIndex - writeToIndex;
}

int WriteKnightMoves(Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	Move jumps[8];
	jumps[0] = MakeMove(file, rank, file + 1, rank + 2, 0);
	jumps[1] = MakeMove(file, rank, file + 1, rank - 2, 0);
	jumps[2] = MakeMove(file, rank, file - 1, rank + 2, 0);
	jumps[3] = MakeMove(file, rank, file - 1, rank - 2, 0);
	jumps[4] = MakeMove(file, rank, file + 2, rank + 1, 0);
	jumps[5] = MakeMove(file, rank, file + 2, rank - 1, 0);
	jumps[6] = MakeMove(file, rank, file - 2, rank + 1, 0);
	jumps[7] = MakeMove(file, rank, file - 2, rank - 1, 0);
	int bufferIndex = writeToIndex;
	for (int i = 0; i < 8; i++) {
		if (IsMoveAvailable(jumps[i], position)) {
			buffer[bufferIndex++] = jumps[i];
		}
	}
	return bufferIndex - writeToIndex;
}

int WritePawnMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	int bufferIndex = writeToIndex;
	int colorSign = -1;
	if (position->flags & WHITE_TURN_FLAG)
		colorSign = 1;
	// Forward moves
	Move forwardMove = MakeMove(file, rank, file, rank + colorSign, 0);
	if (IsMoveAvailable(forwardMove, position) == 1) {
		buffer[bufferIndex++] = forwardMove;
		if ((colorSign == 1) ? (rank == 1) : (rank == BOARD_HEIGHT - 2)) {
			Move extendedMove = MakeMove(file, rank, file, rank + 2 * colorSign, EN_PASSANTABLE);
			if (IsMoveAvailable(extendedMove, position) == 1)
				buffer[bufferIndex++] = extendedMove;
		}
	}
	// Captures
	Move captures[2];
	captures[0] = MakeMove(file, rank, file + 1, rank + colorSign, 0);
	captures[1] = MakeMove(file, rank, file - 1, rank + colorSign, 0);
	for (int i = 0; i < 2; i++) {
		if (IsMoveAvailable(captures[i], position)) {
			buffer[bufferIndex++] = captures[i];
		}
	}
	// Promotions
	if ((colorSign == 1) ? (rank == BOARD_HEIGHT - 2) : (rank == 1)) {
		int movesCount = bufferIndex - writeToIndex;
		int startingIndex = writeToIndex;
		for (int i = 0; i < movesCount; i++) {
			Move vanillaMove = buffer[startingIndex + i];
			vanillaMove.special = (colorSign == 1) ? WHITE_QUEEN : BLACK_QUEEN;
			vanillaMove.pieceChar = vanillaMove.special;
			buffer[startingIndex + i] = vanillaMove;
			vanillaMove.special = (colorSign == 1) ? WHITE_ROOK : BLACK_ROOK;
			vanillaMove.pieceChar = vanillaMove.special;
			buffer[bufferIndex++] = vanillaMove;
			vanillaMove.special = (colorSign == 1) ? WHITE_BISHOP : BLACK_BISHOP;
			vanillaMove.pieceChar = vanillaMove.special;
			buffer[bufferIndex++] = vanillaMove;
			vanillaMove.special = (colorSign == 1) ? WHITE_KNIGHT : BLACK_KNIGHT;
			vanillaMove.pieceChar = vanillaMove.special;
			buffer[bufferIndex++] = vanillaMove;
		}
	}
	return bufferIndex - writeToIndex;
}

int WriteLinearMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank, int horizontalStep, int verticalStep) {
	int bufferIndex = writeToIndex;
	int available = 1;
	int i = 1;
	while (available == 1) {
		int dx = horizontalStep * i;
		int dy = verticalStep * i;
		Move move = MakeMove(file, rank, file + dx, rank + dy, 0);
		available = IsMoveAvailable(move, position);
		if (available) {
			buffer[bufferIndex++] = move;
			i++;
		}
	}
	return bufferIndex - writeToIndex;
}

int WriteCastlingMoves(Move* buffer, int writeToIndex, Position* position, int file, int rank) {
	int bufferIndex = writeToIndex;
	// If the player has kingside castling rights
	if ((position->flags & WHITE_TURN_FLAG) ? 
		(position->flags & WHITE_CASTLING_RIGHT_SHORT) :
		(position->flags & BLACK_CASTLING_RIGHT_SHORT)) {
		char castleFile = position->blackCastleShortFile;
		if (position->flags & WHITE_TURN_FLAG)
			castleFile = position->whiteCastleShortFile;
		char isPathClear = 1;
		// Checking path of the king.
		for (int f = file + 1; f < BOARD_WIDTH - 1; f++) {
			Move pathMove = MakeMove(file, rank, f, rank, 0);
			if (IsMoveAvailable(pathMove, position) == 0) {
				isPathClear = 0;
				break;
			}
		}
		// Checking the path of the rook.
		if (isPathClear) {
			for (int f = castleFile - 1; f >= BOARD_WIDTH - 3; f--) {
				Move pathMove = MakeMove(castleFile, rank, f, rank, 0);
				if (IsMoveAvailable(pathMove, position) == 0) {
					isPathClear = 0;
					break;
				}
			}
			buffer[bufferIndex++] = MakeMove(file, rank, BOARD_WIDTH - 2, rank, CASTLE_SHORT);
		}
	}
	// If the player has queenside castling rights
	if ((position->flags & WHITE_TURN_FLAG) ?
		(position->flags & WHITE_CASTLING_RIGHT_LONG) :
		(position->flags & BLACK_CASTLING_RIGHT_LONG)) {
		char castleFile = position->blackCastleLongFile;
		if (position->flags & WHITE_TURN_FLAG)
			castleFile = position->whiteCastleLongFile;
		char isPathClear = 1;
		// Checking path of the king.
		for (int f = file - 1; f > 1; f--) {
			Move pathMove = MakeMove(file, rank, f, rank, 0);
			if (IsMoveAvailable(pathMove, position) == 0) {
				isPathClear = 0;
				break;
			}
		}
		// Checking the path of the rook.
		if (isPathClear) {
			for (int f = castleFile + 1; f >= 5; f--) {
				Move pathMove = MakeMove(castleFile, rank, f, rank, 0);
				if (IsMoveAvailable(pathMove, position) == 0) {
					isPathClear = 0;
					break;
				}
			}
			buffer[bufferIndex++] = MakeMove(file, rank, 2, rank, CASTLE_LONG);
		}
	}
	return bufferIndex - writeToIndex;
}

int IsMoveAvailable (Move move, Position* position) {
	int destinationIndex = GetIndexOfSquare(move.destinationFile, move.destinationRank);
	if (destinationIndex == -1)
		return 0;
	int sourceIndex = GetIndexOfSquare(move.sourceFile, move.sourceRank);
	char pieceChar = position->squares[sourceIndex];
	char destinationChar = position->squares[destinationIndex];
	if (destinationChar == EMPTY_SQUARE)
		return 1;
	if (destinationChar == EN_PASSANTABLE) {
		if (pieceChar == BLACK_PAWN || pieceChar == WHITE_PAWN) 
			return 2;
		return 1;
	}
	char oppositeColors = (pieceChar ^ destinationChar) & LOWERCASE_FLAG;
	if (oppositeColors)
		return 2;
	return 0;
}

int IsPawnEmptyCapture (Move move, Position* position) {
	char pieceChar = position->squares[GetIndexOfSquare(move.sourceFile, move.sourceRank)];
	if ((pieceChar != WHITE_PAWN) && (pieceChar != BLACK_PAWN))
		return 0;
	if (move.sourceFile == move.destinationFile)
		return 0;
	if (IsMoveAvailable(move, position) == 2)
		return 0;
	return 1;
}

int IsIllegalCastle (Move move, Position* position) {
	if (move.special != CASTLE_SHORT && move.special != CASTLE_LONG)
		return 0;
	if (CheckCheck(position))
		return 1;
	int direction = move.destinationFile > move.sourceFile ? 1 : -1;
	// Starts from the move to the same spot because 
	// castling while in check is illegal too.
	for (int file = move.sourceFile; file != move.destinationFile; file += direction) {
		Move pathMove = MakeMove(move.sourceFile, move.sourceRank, file, move.destinationRank, 0);
		Position afterPathMove = PositionAfterMove(pathMove, position);
		if (CheckCheck(&afterPathMove) == -1)
			return 1;
	}
	return 0;
}

// Nothing implementations for now
int FilterIllegalMove (Move* buffer, int bufferLength, Position* position) {
	Move moves[MAX_MOVES];
	int bufferIndex = 0;
	for (int i = 0; i < bufferLength; i++) {
		moves[i] = buffer[i];
	}
	for (int i = 0; i < bufferLength; i++) {
		if (IsPawnEmptyCapture(moves[i], position))
			continue;
		if (IsIllegalCastle(moves[i], position))
			continue;
		Position afterMove = PositionAfterMove (moves[i], position);
		if (CheckCheck(&afterMove) == -1)
			continue;
		buffer[bufferIndex++] = moves[i];
	}
	return bufferIndex;
}

void CompleteCastlingMove (Move move, Position* position) {
	char pieceChar = position->squares[GetIndexOfSquare(move.sourceFile, move.sourceRank)];
	if (pieceChar == WHITE_KING || pieceChar == BLACK_KING) {
		position->flags &= ~(pieceChar == WHITE_KING ? WHITE_CASTLING_RIGHT_SHORT : BLACK_CASTLING_RIGHT_SHORT);
		position->flags &= ~(pieceChar == WHITE_KING ? WHITE_CASTLING_RIGHT_LONG : BLACK_CASTLING_RIGHT_LONG);
		int rookFile = 0;
		int rookDestinationFile = 0;
		if (move.special == CASTLE_SHORT) {
			rookFile = pieceChar == WHITE_KING ? position->whiteCastleShortFile : position->blackCastleShortFile;
			rookDestinationFile = 5;
		} else if (move.special == CASTLE_LONG) {
			rookFile = pieceChar == WHITE_KING ? position->whiteCastleLongFile : position->blackCastleLongFile;
			rookDestinationFile = 3;
		} else
			return;
		int rookIndex = GetIndexOfSquare(rookFile, move.sourceRank);
		int rookDestinationIndex = GetIndexOfSquare(rookDestinationFile, move.sourceRank);
		position->squares[rookDestinationIndex] = position->squares[rookIndex];
		position->squares[rookIndex] = EMPTY_SQUARE;
	}
	else if (pieceChar == WHITE_ROOK) {
		if (move.sourceFile == position->whiteCastleShortFile)
			position->flags &= ~WHITE_CASTLING_RIGHT_SHORT;
		else if (move.sourceFile == position->whiteCastleLongFile)
			position->flags &= ~WHITE_CASTLING_RIGHT_LONG;
	}
	else if (pieceChar == BLACK_ROOK) {
		if (move.sourceFile == position->blackCastleShortFile)
			position->flags &= ~BLACK_CASTLING_RIGHT_SHORT;
		else if (move.sourceFile == position->blackCastleLongFile)
			position->flags &= ~BLACK_CASTLING_RIGHT_LONG;
	}
}

void CompleteExtendedPawnMove (Move move, Position* position) {
	if (move.pieceChar != WHITE_PAWN && move.pieceChar != BLACK_PAWN)
		return;
	char moveLength = move.sourceRank - move.destinationRank;
	if (moveLength > 1 || moveLength < -1)
		move.special = EN_PASSANTABLE;
	ClearEnpassantable(position);
	if (move.special != EN_PASSANTABLE)
		return;
	char pieceChar = position->squares[GetIndexOfSquare(move.sourceFile, move.sourceRank)];
	int enpassantableIndex = 0;
	int enpassantOffset = pieceChar == WHITE_PAWN ? -1 : 1;
	enpassantableIndex = GetIndexOfSquare(move.destinationFile, move.destinationRank + enpassantOffset);
	position->squares[enpassantableIndex] = EN_PASSANTABLE;
}

void CompletePromotionMove (Move move, Position* position) {
	if (move.pieceChar != WHITE_PAWN && move.pieceChar != BLACK_PAWN)
		return;
	if (move.special == EN_PASSANTABLE)
		return;
	if (PieceValue(move.special) == 0)
		return;
	char areOpposites = (move.special ^ move.pieceChar) & LOWERCASE_FLAG;
	if (areOpposites)
		move.special ^= LOWERCASE_FLAG;
	int destinationIndex = GetIndexOfSquare(move.destinationFile, move.destinationRank);
	position->squares[destinationIndex] = move.special;
}

Position PositionAfterMove (Move move, Position* position) {
	Position afterMove = *position;
	afterMove.flags ^= WHITE_TURN_FLAG;
	CompleteExtendedPawnMove(move, &afterMove);
	CompleteCastlingMove(move, &afterMove);
	int sourceIndex = GetIndexOfSquare(move.sourceFile, move.sourceRank);
	int destinationIndex = GetIndexOfSquare(move.destinationFile, move.destinationRank);
	move.pieceChar = position->squares[sourceIndex];
	move.captureChar = position->squares[destinationIndex];
	afterMove.squares[destinationIndex] = move.pieceChar;
	afterMove.squares[sourceIndex] = EMPTY_SQUARE;
	CompletePromotionMove(move, &afterMove);
	if (move.captureChar == EN_PASSANTABLE) {
		char colorSign = (position->flags & WHITE_TURN_FLAG) ? 1 : -1;
		int pawnIndex = GetIndexOfSquare(move.destinationFile, move.destinationRank - colorSign);
		afterMove.squares[pawnIndex] = EMPTY_SQUARE;
	}
	return afterMove;
}

int CheckCheck (Position* position) {
	Move moves[MAX_MOVES];
	int movesCount = WriteAllMoves(moves, 0, position);
	for (int i = 0; i < movesCount; i++) {
		int squareIndex = GetIndexOfSquare(moves[i].destinationFile, moves[i].destinationRank);
		char destinationChar = position->squares[squareIndex];
		if (destinationChar == WHITE_KING || destinationChar == BLACK_KING)
			return -1;
	}
	Position counterPosition = *position;
	counterPosition.flags ^= WHITE_TURN_FLAG;

	movesCount = WriteAllMoves(moves, 0, &counterPosition);
	for (int i = 0; i < movesCount; i++) {
		int squareIndex = GetIndexOfSquare(moves[i].destinationFile, moves[i].destinationRank);
		char destinationChar = counterPosition.squares[squareIndex];
		if (destinationChar == WHITE_KING || destinationChar == BLACK_KING)
			return 1;
	}
	return 0;
}

int IsMoveCheck(Move move, Position* position) {
	Position afterMove = PositionAfterMove(move, position);
	if (CheckCheck(&afterMove))
		return 1;
	return 0;
}

int MovesEqual (Move a, Move b) {
	return (a.sourceFile == b.sourceFile && 
		a.sourceRank == b.sourceRank &&
		a.destinationFile == b.destinationFile && 
		a.destinationRank == b.destinationRank &&
		a.special == b.special) ? 1 : 0;
}

void LogMove (Move move) {
	char sourceFileChar = 'a' + move.sourceFile;
	char destinationFileChar = 'a' + move.destinationFile;
	char sourceRankChar = '1' + move.sourceRank;
	char destinationRankChar = '1' + move.destinationRank;
	printf("%c%c%c%c", sourceFileChar, sourceRankChar, destinationFileChar, destinationRankChar);
	if (move.special == move.pieceChar) {
		char blackPiece = move.special | LOWERCASE_FLAG;
		printf("%c", blackPiece);
	}
}

void LogAllMoves(Position* position) {
	Move moves[MAX_MOVES];
	int movesLength = WriteAllFilteredMoves(moves, 0, position);
	for (int i = 0; i < movesLength; i++) {
		LogMove(moves[i]);
		printf("\n");
	}
}
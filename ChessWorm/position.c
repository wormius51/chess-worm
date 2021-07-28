#include "position.h"
#include "util.h"
#include <stdio.h>


int GetIndexOfSquare (int file, int rank) {
	if (file < 0 || file >= BOARD_WIDTH ||
		rank < 0 || rank >= BOARD_HEIGHT)
		return -1;
	return file + rank * BOARD_WIDTH;
}

Position GetEmptyPosition () {
	Position position;
	position.flags = WHITE_TURN_FLAG;
	for (int i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++) {
		position.squares[i] = EMPTY_SQUARE;
	}
	position.halfMoveClock = 0;
	position.fullMoveNumber = 0;
	return position;
}

Position GetStartingPosition () {
	Position position = GetEmptyPosition();
	PlacePawns(&position);
	PlacePieces(&position);
	return position;
}

void PlacePawns (Position* position) {
	int whitePawnsRank = 1;
	int blackPawnsRank = BOARD_HEIGHT - 2;
	for (int i = 0; i < BOARD_WIDTH; i++) {
		int pawnIndex = GetIndexOfSquare(i, whitePawnsRank);
		position->squares[pawnIndex] = WHITE_PAWN;
		pawnIndex = GetIndexOfSquare(i, blackPawnsRank);
		position->squares[pawnIndex] = BLACK_PAWN;
	}
}

void PlacePieces (Position* position) {
	position->squares[0] = WHITE_ROOK;
	position->squares[1] = WHITE_KNIGHT;
	position->squares[2] = WHITE_BISHOP;
	position->squares[3] = WHITE_QUEEN;
	position->squares[4] = WHITE_KING;
	position->squares[5] = WHITE_BISHOP;
	position->squares[6] = WHITE_KNIGHT;
	position->squares[7] = WHITE_ROOK;
	int blackOffset = BOARD_WIDTH * (BOARD_HEIGHT - 1);
	position->squares[0 + blackOffset] = BLACK_ROOK;
	position->squares[1 + blackOffset] = BLACK_KNIGHT;
	position->squares[2 + blackOffset] = BLACK_BISHOP;
	position->squares[3 + blackOffset] = BLACK_QUEEN;
	position->squares[4 + blackOffset] = BLACK_KING;
	position->squares[5 + blackOffset] = BLACK_BISHOP;
	position->squares[6 + blackOffset] = BLACK_KNIGHT;
	position->squares[7 + blackOffset] = BLACK_ROOK;
}

Position ParseFEN (char* fen) {
	if (StringsEqual(fen, START_POSITION_STRING))
		return GetStartingPosition();
	Position position = GetEmptyPosition();
	PlacePiecesFEN(&position, fen);
	SetPositionFlags(&position, fen);
	SetPositionEnpassantable(&position, fen);
	SetPositionHalfMoveClock(&position, fen);
	SetPositionFullMoveNumber(&position, fen);
	return position;
}

void PlacePiecesFEN (Position* position, char* fen) {
	char c = NULL;
	int fenIndex = NULL;
	int rank = BOARD_HEIGHT - 1;
	int file = 0;
	while (fenIndex < FEN_MAX_LENGTH && (c = fen[fenIndex++]) != NULL) {
		if (c == '/')
			continue;
		// assuming numbers are single digit for now.
		if (c >= '1' && c <= '9') {
			file += c - '0';
		}
		else {
			int positionIndex = GetIndexOfSquare(file, rank);
			position->squares[positionIndex] = c;
			file++;
		}
		if (file >= BOARD_WIDTH) {
			file = 0;
			rank--;
		}
		if (rank < 0)
			break;
	}
}

void SetPositionFlags (Position* position, char* fen) {
	SetPositionTurn(position, fen);
	SetPositionCastlingRights(position, fen);
}

void SetPositionTurn (Position* position, char* fen) {
	char c = 0;
	int fenIndex = 0;
	while (fenIndex < FEN_MAX_LENGTH && (c = fen[fenIndex++]) != 0) {
		if (c == ' ') {
			if (fen[fenIndex] == 'w')
				position->flags |= WHITE_TURN_FLAG;
			else
				position->flags &= ~WHITE_TURN_FLAG;
			return;
		}
	}
	// If there are no flags assume it's white to move.
	position->flags |= WHITE_TURN_FLAG; 
}

void SetPositionCastlingRights (Position* position, char* fen) {
	int whiteKingFile = 0;
	int blackKingFile = 0;
	for (int f = 0; f < BOARD_WIDTH; f++) {
		char c = position->squares[GetIndexOfSquare(f, 0)];
		if (c == 'K')
			whiteKingFile = f;
		c = position->squares[GetIndexOfSquare(f, BOARD_HEIGHT - 1)];
		if (c == 'k')
			blackKingFile = f;
	}
	char c = 0;
	int fenIndex = 0;
	int stringSpaces = 0;
	// Disable all castling and enable the right ones laster.
	position->flags &= ~WHITE_CASTLING_RIGHT_SHORT;
	position->flags &= ~WHITE_CASTLING_RIGHT_LONG;
	position->flags &= ~BLACK_CASTLING_RIGHT_SHORT;
	position->flags &= ~BLACK_CASTLING_RIGHT_LONG;
	while (fenIndex < FEN_MAX_LENGTH && (c = fen[fenIndex++]) != 0) {
		if (c == ' ') {
			stringSpaces++;
			continue;
		}
		else if (stringSpaces == 2) {
			if (c == FEN_NULL)
				break;
			char isBlack = c & LOWERCASE_FLAG;
			// If this is white
			if (isBlack == 0) {
				if (c == 'K') {
					position->flags |= WHITE_CASTLING_RIGHT_SHORT;
					position->whiteCastleShortFile = BOARD_WIDTH - 1;
				}
				else if (c == 'Q') {
					position->flags |= WHITE_CASTLING_RIGHT_LONG;
					position->whiteCastleLongFile = 0;
				}
				else {
					char castleFile = c - 'A';
					// if c is more then the file of the king castle kingside
					if (castleFile > whiteKingFile) {
						position->flags |= WHITE_CASTLING_RIGHT_SHORT;
						position->whiteCastleShortFile = castleFile;
					}
					else {
						position->flags |= WHITE_CASTLING_RIGHT_LONG;
						position->whiteCastleLongFile = castleFile;
					}
				}
			}
			else { // Black castle
				if (c == 'k') {
					position->flags |= BLACK_CASTLING_RIGHT_SHORT;
					position->blackCastleShortFile = BOARD_WIDTH - 1;
				}
				else if (c == 'q') {
					position->flags |= BLACK_CASTLING_RIGHT_LONG;
					position->blackCastleLongFile = 0;
				}
				else {
					char castleFile = c - 'a';
					// if c is more then the file of the king castle kingside
					if (castleFile > whiteKingFile) {
						position->flags |= BLACK_CASTLING_RIGHT_SHORT;
						position->blackCastleShortFile = castleFile;
					}
					else {
						position->flags |= BLACK_CASTLING_RIGHT_LONG;
						position->blackCastleLongFile = castleFile;
					}
				}
			}
		}
		else if (stringSpaces > 2) {
			return;
		}
	}
	// If there is no castling info, grant all castling rights.
	if (stringSpaces < 2) {
		position->flags |= WHITE_CASTLING_RIGHT_SHORT;
		position->flags |= WHITE_CASTLING_RIGHT_LONG;
		position->flags |= BLACK_CASTLING_RIGHT_SHORT;
		position->flags |= BLACK_CASTLING_RIGHT_LONG;
	}
}

void ClearEnpassantable (Position* position) {
	for (int i = 0; i < BOARD_HEIGHT * BOARD_WIDTH; i++) {
		if (position->squares[i] == EN_PASSANTABLE)
			position->squares[i] = EMPTY_SQUARE;
	}
}

void SetPositionEnpassantable (Position* position, char* fen) {
	char c = 0;
	int fenIndex = 0;
	int stringSpaces = 0;
	ClearEnpassantable(position);
	while (fenIndex < FEN_MAX_LENGTH && (c = fen[fenIndex++]) != 0) {
		if (c == ' ') {
			stringSpaces++;
			continue;
		}
		else if (stringSpaces == 3) {
			if (c == FEN_NULL)
				return;
			int file = c - 'a';
			int rank = fen[fenIndex] - '1';
			int squareIndex = GetIndexOfSquare(file, rank);
			position->squares[squareIndex] = EN_PASSANTABLE;
			return;
		}
	}
}

void SetPositionHalfMoveClock (Position* position, char* fen) {
	char c = 0;
	int fenIndex = 0;
	int stringSpaces = 0;
	position->halfMoveClock = 0;
	while (fenIndex < FEN_MAX_LENGTH && (c = fen[fenIndex++]) != 0) {
		if (c == ' ') {
			stringSpaces++;
		}
		else if (stringSpaces == 4) {
			position->halfMoveClock *= 10;
			position->halfMoveClock += c - '0';
		}
		else if (stringSpaces > 4) {
			return;
		}
	}
}

void SetPositionFullMoveNumber (Position* position, char* fen) {
	char c = 0;
	int fenIndex = 0;
	int stringSpaces = 0;
	position->fullMoveNumber = 0;
	while (fenIndex < FEN_MAX_LENGTH && (c = fen[fenIndex++]) != 0) {
		if (c == '\n')
			break;
		if (c == ' ') {
			stringSpaces++;
		}
		else if (stringSpaces == 5) {
			position->fullMoveNumber *= 10;
			position->fullMoveNumber += c - '0';
		}
	}
}
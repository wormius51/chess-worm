#ifndef POSITION_H
#define POSITION_H

#include "constants.h"
#include "types.h"


Position globalCurrentPosition;

/// <summary>
/// Returns the index of the square with these coordinates.
/// If the square is out of bounds returns -1.
/// </summary>
int GetIndexOfSquare (int file, int rank);

Position GetEmptyPosition ();
Position GetStartingPosition ();
Position ParseFEN (char* fen);

void PlacePawns (Position* position);
void PlacePieces (Position* position);
void PlacePiecesFEN (Position* position, char* fen);
void SetPositionFlags (Position* position, char* fen);
void SetPositionTurn (Position* position, char* fen);
void SetPositionCastlingRights (Position* position, char* fen);
void SetPositionEnpassantable (Position* position, char* fen);
void SetPositionHalfMoveClock (Position* position, char* fen);
void SetPositionFullMoveNumber (Position* position, char* fen);

void ClearEnpassantable (Position* position);

#endif // !POSITION_H
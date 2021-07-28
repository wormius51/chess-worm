#ifndef MOVES_H
#define MOVES_H

#include "constants.h"
#include "types.h"

Move MakeMove (int sourceFile, int sourceRank,
	int destinationFile, int destinationRank, char special);

Move ParseMove (char* input);

/// <summary>
/// Writes all the legal moves of the player who is to move.
/// </summary>
int WriteAllFilteredMoves (Move* buffer, int writeToIndex, Position* position);
/// <summary>
/// Writes all the legal moves of the piece on the given square.
/// </summary>
int WriteFilteredMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
// All of these disregard the rule of not walking into check.
int WriteAllMoves (Move* buffer, int writeToIndex, Position* position);
int WriteMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
int WriteKingMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
int WriteQueenMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
int WriteRookMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
int WriteBishopMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
int WriteKnightMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
int WritePawnMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
/// <summary>
/// Writes moves along the line.
/// </summary>
int WriteLinearMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank, int horizontalStep, int verticalStep);
int WriteCastlingMoves (Move* buffer, int writeToIndex, Position* position, int file, int rank);
/// <summary>
/// Filters out moves that lead into being in check.
/// Filters castling out of check or through check.
/// Filters pawns capture nothing.
/// </summary>
int FilterIllegalMove (Move* buffer, int bufferLength, Position* position);

Position PositionAfterMove (Move move, Position* position);
/// <summary>
/// Returns 0 if no one is in check.
/// Returns 1 if the player who is to move is in check.
/// Returns -1 if the player who is not to move is in check 
/// (makes it an illegal position)
/// </summary>
int CheckCheck (Position* position);
int IsMoveCheck (Move move, Position* position);
/// <summary>
/// Checks if the move is available not acounting for check.
/// Returns 1 if it is, 0 if it's not.
/// Returns 2 if it's a capture.
/// </summary>
int IsMoveAvailable (Move move, Position* position);

/// <summary>
/// Returns 1 if the 2 moves are the same.
/// Returns 0 if not.
/// </summary>
int MovesEqual (Move a, Move b);

void LogMove (Move move);
void LogAllMoves (Position* position);

#endif // !MOVES_H
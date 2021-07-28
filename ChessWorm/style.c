#include "style.h"
#include "evaluation.h"
#include "constants.h"
#include <math.h>

float StylePoints (Move move, Position* position) {
	float points = MaterialBalance(position) * MATERIAL_STYLE_VALUE;
	float pieceValue = PieceValue(move.pieceChar);
	if (pieceValue == INFINITY)
		pieceValue = KING_STYLE_VALUE;
	points += pieceValue * PIECE_STYLE_VALUE;
	if (move.captureChar == EN_PASSANTABLE)
		points += EN_PASSANT_STYLE_VALUE;
	return points;
}
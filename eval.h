#pragma once
#include "defines.h"
#include "position.h"
#include "piece.h"
#include "pst.h"

class pieceInfo;

namespace Eval
{
	int evaluate(const Pos&);
	int evaluateHCE(const Pos& position);
	Score pieceSquareScore(pieceInfo, uint8_t);
	void updateScore(Score&, int, int);
}

INLINE void Eval::updateScore(Score& scores, const int openingBonus, const int endBonus)
{
	scores.first += openingBonus;
	scores.second += endBonus;
}

inline Score Eval::pieceSquareScore(const pieceInfo piece, uint8_t square)
{
	square = piece.Color == White ? square : Square::mirrorSquare(square);
	return std::make_pair(PST[piece.Type][OP][square], PST[piece.Type][EG][square]);
}

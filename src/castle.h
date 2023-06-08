#pragma once
#include "move.h"

namespace Castle
{
	constexpr uint8_t whiteCastleOO = 0x1;
	constexpr uint8_t whiteCastleOOO = 0x2;
	constexpr uint8_t blackCastleOO = 0x4;
	constexpr uint8_t blackCastleOOO = 0x8;
	constexpr uint8_t fullCastlingRights = 0xF;

	constexpr uint64_t whiteCastleMaskOO = 0x0000000000000060;
	constexpr uint64_t whiteCastleMaskOOO = 0x000000000000000E;
	constexpr uint64_t blackCastleMaskOO = 0x6000000000000000;
	constexpr uint64_t blackCastleMaskOOO = 0x0E00000000000000;

	constexpr uint64_t whiteKingShield = 0xe000;
	constexpr uint64_t whiteQueenShield = 0x700;
	constexpr uint64_t blackKingShield = 0xe0000000000000;
	constexpr uint64_t blackQueenShield = 0x7000000000000;

	constexpr uint64_t whiteKingSide = 0xe0;
	constexpr uint64_t whiteQueenSide = 0x7;
	constexpr uint64_t blackKingSide = 0xe000000000000000;
	constexpr uint64_t blackQueenSide = 0x700000000000000;

	const Move whiteCastlingOO(Square::E1, Square::G1, kingCastle);
	const Move whiteCastlingOOO(Square::E1, Square::C1, queenCastle);

	const Move blackCastlingOO(Square::E8, Square::G8, kingCastle);
	const Move blackCastlingOOO(Square::E8, Square::C8, queenCastle);
}

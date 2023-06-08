#include "piece.h"
#include "direction.h"
#include "position.h"

namespace Piece
{
	uint8_t getPiece(const char initial)
	{
		switch (initial)
		{
		case 'b':
			return Bishop;
		case 'n':
			return Knight;
		case 'q':
			return Queen;
		case 'r':
			return Rook;
		default:
			throw std::exception();
		}
	}

	char getType(const uint8_t type)
	{
		switch (type)
		{
		case Bishop:
			return 'b';
		case King:
			return 'k';
		case Knight:
			return 'n';
		case Pawn:
			return 'p';
		case Queen:
			return 'q';
		case Rook:
			return 'r';
		default:
			throw std::exception();
		}
	}

	char getColor(const uint8_t color)
	{
		switch (color)
		{
		case White:
			return 'w';
		case Black:
			return 'b';
		default:
			throw std::exception();
		}
	}

	char getInitial(const pieceInfo piece)
	{
		if (piece.Color == White)
		{
			switch (piece.Type)
			{
			case Bishop:
				return 'B';
			case King:
				return 'K';
			case Knight:
				return 'N';
			case Pawn:
				return 'P';
			case Queen:
				return 'Q';
			case Rook:
				return 'R';
			default:
				throw std::exception();
			}
		}
		switch (piece.Type)
		{
		case Bishop:
			return 'b';
		case King:
			return 'k';
		case Knight:
			return 'n';
		case Pawn:
			return 'p';
		case Queen:
			return 'q';
		case Rook:
			return 'r';
		default:
			throw std::exception();
		}
	}
}

pieceInfo::pieceInfo(const uint8_t color, const uint8_t type) :
	Color(color),
	Type(type)
{
}

pieceInfo::pieceInfo() noexcept :
	Color(noColor),
	Type(noType)
{
}

uint64_t Knight::getAllTargets(const uint64_t knights, const Pos& position)
{
	const uint64_t targets = Moves::knightAttacks[(BSF(knights))];
	return targets & ~position.ourPieces();
}

uint64_t Knight::targetsFrom(const uint8_t square, const uint8_t color, const Pos& position)
{
	const uint64_t targets = Moves::knightAttacks[square];
	return targets & ~position.Pieces(color);
}

uint64_t Knight::getKnightAttacks(const uint64_t knights)
{
	uint64_t east = Direction::oneStepEast(knights);
	uint64_t west = Direction::oneStepWest(knights);
	uint64_t attacks = (east | west) << 16;
	attacks |= (east | west) >> 16;
	east = Direction::oneStepEast(east);
	west = Direction::oneStepWest(west);
	attacks |= (east | west) << 8;
	attacks |= (east | west) >> 8;
	return attacks;
}

uint64_t Bishop::getAllTargets(const uint64_t bishops, const Pos& position)
{
	const uint64_t occupiedSquares = position.occupiedSquares;
	uint64_t targets = Empty;
	const uint8_t square = BSF(bishops);
	targets |= Moves::getA1H8DiagonalAttacks(occupiedSquares, square);
	targets |= Moves::getH1A8DiagonalAttacks(occupiedSquares, square);
	return targets & ~position.ourPieces();
}

uint64_t Bishop::targetsFrom(const uint8_t square, const uint8_t color, const Pos& position)
{
	const uint64_t occupiedSquares = position.occupiedSquares;
	uint64_t targets = Empty;
	targets |= Moves::getA1H8DiagonalAttacks(occupiedSquares, square);
	targets |= Moves::getH1A8DiagonalAttacks(occupiedSquares, square);
	return targets & ~position.Pieces(color);
}

uint64_t Rook::getAllTargets(const uint64_t rooks, const Pos& position)
{
	uint64_t occupiedSquares = position.occupiedSquares;
	uint64_t targets = Empty;
	uint8_t square = BSF(rooks);
#ifdef PEXT
	targets = Moves::getRookAttacks(occupiedSquares, square);
#else
	targets |= Moves::getRankAttacks(occupiedSquares, square);
	targets |= Moves::getFileAttacks(occupiedSquares, square);
#endif
	return targets & ~position.ourPieces();
}

uint64_t Rook::targetsFrom(uint8_t square, const uint8_t color, const Pos& position)
{
	uint64_t occupiedSquares = position.occupiedSquares;
	uint64_t targets = Empty;
#ifdef PEXT
	targets = Moves::getRookAttacks(occupiedSquares, square);
#else
	targets |= Moves::getRankAttacks(occupiedSquares, square);
	targets |= Moves::getFileAttacks(occupiedSquares, square);
#endif
	return targets & ~position.Pieces(color);
}

uint64_t Queen::getAllTargets(const uint64_t queens, const Pos& position)
{
	return Rook::getAllTargets(queens, position) | Bishop::getAllTargets(queens, position);
}

uint64_t Queen::targetsFrom(const uint8_t square, const uint8_t color, const Pos& position)
{
	return Rook::targetsFrom(square, color, position) | Bishop::targetsFrom(square, color, position);
}

uint64_t King::getAllTargets(const uint64_t king, const Pos& position)
{
	const uint64_t kingMoves = Moves::kingAttacks[(BSF(king))];
	return kingMoves & ~position.ourPieces();
}

uint64_t King::getKingAttacks(uint64_t king)
{
	uint64_t attacks = Direction::oneStepEast(king) | Direction::oneStepWest(king);
	king |= attacks;
	attacks |= Direction::oneStepNorth(king) | Direction::oneStepSouth(king);
	return attacks;
}

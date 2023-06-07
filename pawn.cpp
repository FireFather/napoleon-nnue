#include "position.h"
#include "ranks.h"

uint64_t Pawn::getAllTargets(const uint64_t pawns, const Pos& position)
{
	const uint64_t empty = position.emptySquares;
	return getQuietTargets(position.getSideToMove(), pawns, empty) | getAnyAttack(pawns, position);
}

uint64_t Pawn::getAnyAttack(const uint64_t pawns, const Pos& position)
{
	return (getEastAttacks(position.getSideToMove(), pawns) | getWestAttacks(position.getSideToMove(), pawns)) &
		position.enemyPieces();
}

uint64_t Pawn::getAnyAttack(const uint64_t pawns, const uint8_t color, const uint64_t squares)
{
	return (getEastAttacks(color, pawns) | getWestAttacks(color, pawns)) & squares;
}

uint64_t Pawn::getPawnsAbleToSinglePush(const uint8_t color, const uint64_t pawns, const uint64_t empty)
{
	switch (color)
	{
	case White:
		return Direction::oneStepSouth(empty) & pawns;
	case Black:
		return Direction::oneStepNorth(empty) & pawns;
	default:
		throw std::exception();
	}
}

uint64_t Pawn::getPawnsAbleToDoublePush(const uint8_t color, const uint64_t pawns, const uint64_t empty)
{
	switch (color)
	{
	case White:
	{
		const uint64_t emptyRank3 = Direction::oneStepSouth(empty & Ranks::Four) & empty;
		return getPawnsAbleToSinglePush(color, pawns, emptyRank3);
	}
	case Black:
	{
		const uint64_t emptyRank6 = Direction::oneStepNorth(empty & Ranks::Six) & empty;
		return getPawnsAbleToSinglePush(color, pawns, emptyRank6);
	}
	default:
		throw std::exception();
	}
}

uint64_t Pawn::getEastAttacks(const uint8_t color, const uint64_t pawns)
{
	return color == White ? Direction::oneStepNorthEast(pawns) : Direction::oneStepSouthEast(pawns);
}

uint64_t Pawn::getWestAttacks(const uint8_t color, const uint64_t pawns)
{
	return color == White ? Direction::oneStepNorthWest(pawns) : Direction::oneStepSouthWest(pawns);
}

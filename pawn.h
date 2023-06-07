#pragma once
#include "direction.h"
#include "ranks.h"

class Pos;

class Pawn
{
public:
	static uint64_t getAllTargets(uint64_t pawns, const Pos& position);
	static uint64_t getAnyAttack(uint64_t pawns, const Pos& position);
	static uint64_t getAnyAttack(uint64_t pawns, uint8_t color, uint64_t squares);
	static uint64_t getQuietTargets(uint8_t color, uint64_t pawns, uint64_t empty);

private:
	static uint64_t getSinglePushTargets(uint8_t color, uint64_t pawns, uint64_t empty);
	static uint64_t getDoublePushTargets(uint8_t color, uint64_t pawns, uint64_t empty);
	static uint64_t getPawnsAbleToSinglePush(uint8_t color, uint64_t pawns, uint64_t empty);
	static uint64_t getPawnsAbleToDoublePush(uint8_t color, uint64_t pawns, uint64_t empty);
	static uint64_t getEastAttacks(uint8_t color, uint64_t pawns);
	static uint64_t getWestAttacks(uint8_t color, uint64_t pawns);
};

INLINE uint64_t Pawn::getQuietTargets(const uint8_t color, const uint64_t pawns, const uint64_t empty)
{
	return getSinglePushTargets(color, pawns, empty) | getDoublePushTargets(color, pawns, empty);
}

INLINE uint64_t Pawn::getSinglePushTargets(const uint8_t color, const uint64_t pawns, const uint64_t empty)
{
	return color == White ? Direction::oneStepNorth(pawns) & empty : Direction::oneStepSouth(pawns) & empty;
}

INLINE uint64_t Pawn::getDoublePushTargets(const uint8_t color, const uint64_t pawns, const uint64_t empty)
{
	const uint64_t singlePush = getSinglePushTargets(color, pawns, empty);

	return color == White
		? Direction::oneStepNorth(singlePush) & empty & Ranks::Four
		: Direction::oneStepSouth(singlePush) & empty & Ranks::Five;
}

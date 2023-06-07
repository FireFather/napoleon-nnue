#include "hashentry.h"

HashEntry::HashEntry() noexcept
= default;

HashEntry::HashEntry(const uint64_t hash, const uint8_t depth, const int score, const Move bestMove,
	const ScoreType bound)
{
	Key = hash;
	Depth = depth;
	Score = score;
	Bound = bound;
	BestMove = bestMove;
}

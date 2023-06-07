#pragma once
#include <cstring>
#include "square.h"
#include "clock.h"

constexpr int maxPly = 1024;
class Move;

class SearchInfo
{
public:
	enum class Time : int { Infinite = -1 };

	explicit SearchInfo(int time = static_cast<int>(Time::Infinite), int depth = 1, int nodes = 0) noexcept;
	void newSearch(int time = static_cast<int>(Time::Infinite));
	void stopSearch();
	int incrementDepth();
	int maxDepth() const;
	int Nodes() const;
	bool timeOver() const;
	void resetNodes();
	void visitNode();
	void setKillers(Move, int);
	void setHistory(Move, uint8_t, int);
	void setDepthLimit(int);
	void setGameTime(int);
	Move firstKiller(int) const;
	Move secondKiller(int) const;
	int historyScore(Move, uint8_t) const;
	double elapsedTime() const;
	int SelDepth;

private:
	int depthLimit{};
	int depth;
	int nodes;
	int history[2][64 * 64]{};
	int allocatedTime;
	Move killers[maxPly][2];
	Clock timer;
};

inline bool SearchInfo::timeOver() const
{
	if (allocatedTime == static_cast<int>(Time::Infinite) && depth <= depthLimit)
		return false;

	return (timer.elapsedMilliseconds() >= allocatedTime || timer.elapsedMilliseconds() / allocatedTime >= 0.85);
}

inline int SearchInfo::Nodes() const
{
	return nodes;
}

inline void SearchInfo::visitNode()
{
	++nodes;
}

inline Move SearchInfo::firstKiller(const int depth) const
{
	return killers[depth][0];
}

inline Move SearchInfo::secondKiller(const int depth) const
{
	return killers[depth][1];
}

inline int SearchInfo::historyScore(const Move move, const uint8_t color) const
{
	return history[color][move.butterflyIndex()];
}

inline double SearchInfo::elapsedTime() const
{
	return timer.elapsedMilliseconds();
}

inline void SearchInfo::setKillers(const Move move, const int depth)
{
	if (move != killers[depth][0])
		killers[depth][1] = killers[depth][0];
	killers[depth][0] = move;
}

inline void SearchInfo::setHistory(const Move move, const uint8_t color, const int depth)
{
	history[color][move.butterflyIndex()] += (1 << depth);
}

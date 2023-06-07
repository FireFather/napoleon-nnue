#pragma once
#include <condition_variable>
#include <vector>
#include "searchinfo.h"
#include "smpinfo.h"

enum class SearchType
{
	Infinite,
	TimePerGame,
	TimePerMove,
	Ponder
};

enum class NodeType
{
	PV,
	NONPV,
	CUT,
	ALL
};

class Pos;
class hashTable;

namespace Search
{
	extern bool pondering;
	extern std::atomic<bool> ponderHit;
	extern std::atomic<bool> stopSignal;
	extern int moveTime;
	extern int gameTime[2];
	extern thread_local SearchInfo searchInfo;
	extern thread_local bool sendOutput;
	extern hashTable Hash;
	extern std::condition_variable smp;
	extern SMPInfo smpInfo;
	extern std::vector<std::thread> threads;
	extern int depth_limit;
	extern int cores;
	extern std::atomic<bool> quit;
	extern const int defaultCores;

	void initThreads(int = defaultCores);
	void killThreads();
	void signalThreads(int, int, int, const Pos&, bool);
	void smpSearch();
	int razorMargin(int);
	int futilityMargin(int);
	int predictTime(uint8_t);

	std::string getInfo(Pos&, Move, int, int, int);
	std::string getPV(Pos&, Move, int);
	Move getPonderMove(Pos&, Move);
	Move startThinking(SearchType, Pos&, bool = true);
	void stopThinking();
	Move iterativeSearch(Pos&);
	int searchRoot(int, int, int, Move&, Pos&);
	template <NodeType>
	int search(int, int, int, int, Pos&, bool);
	int quiescence(int, int, Pos&);

	constexpr int Infinity = 200000;
	constexpr int Unknown = 2 * Infinity;
	constexpr int Mate = std::numeric_limits<short>::max();
}

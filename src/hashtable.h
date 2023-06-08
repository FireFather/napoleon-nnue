#pragma once
#include <mutex>
#include <atomic>
#include <valarray>
#include "hashentry.h"

class SpinLock
{
public:
	INLINE void lock()
	{
		while (spinLock.test_and_set(std::memory_order_acquire))
		{
		}
	}

	INLINE void unlock()
	{
		spinLock.clear(std::memory_order_release);
	}

private:
	std::atomic_flag spinLock = ATOMIC_FLAG_INIT;
};

class hashTable
{
public:
	static constexpr int Unknown = -999999;
	static constexpr int bucketSize = 4;
	explicit hashTable(int size = 32) noexcept;
	void setSize(int);
	void Save(uint64_t, uint8_t, int, Move, ScoreType) const;
	void Clear() const;
	std::pair<int, Move> Probe(uint64_t, uint8_t, int, int) const;
	Move getPV(uint64_t) const;

private:
	uint64_t mask{};
	uint32_t entries{};
	uint32_t lockEntries{};
	HashEntry* table{};
	SpinLock* locks{};
	HashEntry* at(uint64_t, int = 0) const;
};

inline HashEntry* hashTable::at(const uint64_t key, const int index) const
{
	return table + (key & mask) + index;
}

inline double Log2(const double x)
{
	return std::log(x) / std::log(2.);
}

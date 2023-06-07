#pragma once
#include "files.h"
#include "defines.h"

namespace Direction
{
	constexpr uint64_t notAFile = ~Files::A;
	constexpr uint64_t notBFile = ~Files::B;
	constexpr uint64_t notCFile = ~Files::C;
	constexpr uint64_t notDFile = ~Files::D;
	constexpr uint64_t notEFile = ~Files::E;
	constexpr uint64_t notFFile = ~Files::F;
	constexpr uint64_t notGFile = ~Files::G;
	constexpr uint64_t notHFile = ~Files::H;
	constexpr uint64_t notABFile = notAFile | notBFile;
	constexpr uint64_t notGHFile = notGFile | notHFile;

	INLINE static uint64_t oneStepSouth(const uint64_t bitBoard)
	{
		return bitBoard >> 8;
	}

	INLINE static uint64_t oneStepNorth(const uint64_t bitBoard)
	{
		return bitBoard << 8;
	}

	INLINE static uint64_t oneStepWest(const uint64_t bitBoard)
	{
		return bitBoard >> 1 & notHFile;
	}

	INLINE static uint64_t oneStepEast(const uint64_t bitBoard)
	{
		return bitBoard << 1 & notAFile;
	}

	INLINE static uint64_t oneStepNorthEast(const uint64_t bitBoard)
	{
		return bitBoard << 9 & notAFile;
	}

	INLINE static uint64_t oneStepNorthWest(const uint64_t bitBoard)
	{
		return bitBoard << 7 & notHFile;
	}

	INLINE static uint64_t oneStepSouthEast(const uint64_t bitBoard)
	{
		return bitBoard >> 7 & notAFile;
	}

	INLINE static uint64_t oneStepSouthWest(const uint64_t bitBoard)
	{
		return bitBoard >> 9 & notHFile;
	}
}

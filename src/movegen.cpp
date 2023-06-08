#include "movegen.h"
#include "piece.h"

void moveGen::getQueenMoves(uint64_t queens, const Pos& position, Move moveList[], int& pos, const uint64_t target)
{
	while (queens != 0)
	{
		const uint8_t fromIndex = BSFReset(queens);
		uint64_t targets = Queen::getAllTargets(Masks::squareMask[fromIndex], position) & target;

		while (targets != 0)
		{
			const uint8_t toIndex = BSFReset(targets);
			moveList[pos++] = Move(fromIndex, toIndex);
		}
	}
}

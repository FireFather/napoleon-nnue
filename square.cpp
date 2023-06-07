#include <algorithm>
#include <string>
#include "square.h"

int Square::Parse(std::string square)
{
	std::transform(square.begin(), square.end(), square.begin(), tolower);
	const int x = square[0] - 'a';
	const int y = square[1] - '1';
	return getSquareIndex(x, y);
}

std::string Square::toAlgebraic(const uint8_t square)
{
	if (square == noSquare)
		return "none";
	std::string str;
	str += static_cast<char>(getFileIndex(square) + 'a');
	str += std::to_string(getRankIndex(square) + 1);
	return str;
}

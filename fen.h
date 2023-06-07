#pragma once
#include "piece.h"
#include "move.h"

using vstring = std::vector<std::string>;

static int pieceHere = 0;

class Fen
{
public:
	std::string fullString;
	pieceInfo piecePlacement[64];
	uint8_t sideToMove{};
	bool whiteCanCastleShort{};
	bool whiteCanCastleLong{};
	bool blackCanCastleShort{};
	bool blackCanCastleLong{};
	int getPassantSquare{};
	int halfMove{};
	explicit Fen(std::string);
	void Parse();
	static void split(vstring&, const std::string&, const std::string&, bool);

private:
	void parsePiecePlacement(const std::string&);
	void parsesideToMove(const std::string&);
	void parseCastling(const std::string&);
	void parseEnPassant(const std::string&);
	void parseHalfMove(const std::string&);
};

#include <sstream>
#include "fen.h"
#include "evalterms.h"
#include "square.h"
#include "position.h"

Fen::Fen(std::string str) :
	fullString(std::move(str))
{
	Parse();
}

void Fen::Parse()
{
	std::istringstream stream(fullString);

	std::string piecePlacement;
	std::string sideToMove;
	std::string castling;
	std::string enPassant;
	std::string fullMove;

	stream >> piecePlacement;
	stream >> sideToMove;
	stream >> castling;
	stream >> enPassant;

	std::string token;
	stream >> token;
	const std::string halfMove = token;
	stream >> fullMove;

	parsePiecePlacement(piecePlacement);
	parsesideToMove(sideToMove);
	parseCastling(castling);
	parseEnPassant(enPassant);

	if (!halfMove.empty())
		parseHalfMove(halfMove);
}

void Fen::split(vstring& result, const std::string& source, const std::string& separator, const bool skipEmpty)
{
	std::string copy = source;

	bool check(true);

	while (check)
	{
		const std::string::size_type pos = copy.find(separator);

		if (std::string::npos != pos)
		{
			std::string substr = copy.substr(0, pos);
			if (!(skipEmpty && substr.empty()))
				result.push_back(substr);
			copy.erase(0, pos + separator.length());
			check = true;
		}
		else
		{
			result.push_back(copy);
			check = false;
		}
	}
}

pieceInfo chrToPiece(const std::string::value_type& chr)
{
	pieceHere = 1;
	switch (chr)
	{
	case 'P':
		return { White, Pawn };
	case 'N':
		return { White, Knight };
	case 'B':
		return { White, Bishop };
	case 'R':
		return { White, Rook };
	case 'Q':
		return { White, Queen };
	case 'K':
		return { White, King };
	case 'p':
		return { Black, Pawn };
	case 'n':
		return { Black, Knight };
	case 'b':
		return { Black, Bishop };
	case 'r':
		return { Black, Rook };
	case 'q':
		return { Black, Queen };
	case 'k':
		return { Black, King };
	default:;
	}
	pieceHere = 0;
	return Null;
}

void Fen::parsePiecePlacement(const std::string& field)
{
	vstring lines;
	split(lines, field, "/", true);

	uint32_t linepos(8);
	auto itLine = lines.begin();

	for (auto& i : piecePlacement)
	{
		i = Null;
	}

	while (itLine != lines.end())
	{
		vstring::value_type& line = *itLine++;

		int32_t startpos(0);

		switch (linepos--)
		{
		case 8:
			startpos = Square::A8;
			break;
		case 7:
			startpos = Square::A7;
			break;
		case 6:
			startpos = Square::A6;
			break;
		case 5:
			startpos = Square::A5;
			break;
		case 4:
			startpos = Square::A4;
			break;
		case 3:
			startpos = Square::A3;
			break;
		case 2:
			startpos = Square::A2;
			break;
		case 1:
			startpos = Square::A1;
			break;
		default:;
		}

		auto it = line.begin();

		while (it != line.end())
		{
			std::string::value_type& chr = *it++;

			const pieceInfo piece = chrToPiece(chr);
			if (pieceHere == 1)
				piecePlacement[startpos++] = piece;
			else
			{
				switch (chr)
				{
				case '1':
					startpos += 1;
					break;
				case '2':
					startpos += 2;
					break;
				case '3':
					startpos += 3;
					break;
				case '4':
					startpos += 4;
					break;
				case '5':
					startpos += 5;
					break;
				case '6':
					startpos += 6;
					break;
				case '7':
					startpos += 7;
					break;
				case '8':
					startpos += 8;
					break;
				default:;
				}
			}
		}
	}
}

void Fen::parsesideToMove(const std::string& field)
{
	switch (field[0])
	{
	case 'w':
		sideToMove = White;
		break;
	case 'b':
		sideToMove = Black;
		break;
	default:;
	}
}

void Fen::parseCastling(const std::string& field)
{
	whiteCanCastleShort = false;
	whiteCanCastleLong = false;
	blackCanCastleShort = false;
	blackCanCastleLong = false;

	for (const char i : field)
	{
		switch (i)
		{
		case 'K':
			whiteCanCastleShort = true;
			break;
		case 'k':
			blackCanCastleShort = true;
			break;
		case 'Q':
			whiteCanCastleLong = true;
			break;
		case 'q':
			blackCanCastleLong = true;
			break;
		default:;
		}
	}
}

void Fen::parseEnPassant(const std::string& field)
{
	if (field.size() == 1)
	{
		if (field[0] == '-')
			getPassantSquare = Square::noSquare;
	}
	else
	{
		getPassantSquare = Square::Parse(field);
	}
}

void Fen::parseHalfMove(const std::string& field)
{
	halfMove = std::stoi(field);
}

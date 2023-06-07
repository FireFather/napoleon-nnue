#include "piece.h"
#include "fen.h"
#include "eval.h"
#include "castle.h"

Pos::Pos() noexcept
{
	Moves::initAttacks();
	Zobrist::Init();

	pieces[White] = Empty;
	pieces[Black] = Empty;
	occupiedSquares = Empty;
	emptySquares = Empty;

	for (uint8_t c = White; c < noColor; c++)
		for (uint8_t f = 0; f < 8; f++)
			pawnsOnFile[c][f] = 0;
}

void Pos::loadFen(const std::string& pos)
{
	const Fen fenString(pos);

	for (uint8_t c = White; c < noColor; c++)
		for (uint8_t t = Pawn; t < noType; t++)
			numPieces[c][t] = 0;

	for (uint8_t c = White; c < noColor; c++)
		for (uint8_t f = 0; f < 8; f++)
			pawnsOnFile[c][f] = 0;

	material[White] = 0;
	material[Black] = 0;
	allowNullMove = true;
	currentPly = 0;
	zobrist = 0;
	initializeCastlingStatus(fenString);
	initializesideToMove(fenString);
	initializePieceSet(fenString);
	initializeEnPassantSquare(fenString);
	initializeBitBoards(fenString);

	pstScore[White] = calculatePST(White);
	pstScore[Black] = calculatePST(Black);
}

Score Pos::calculatePST(const uint8_t color) const
{
	int pst[2][2] =
	{
		{0}
	};

	for (uint8_t sq = Square::A1; sq <= Square::H8; sq++)
	{
		const pieceInfo piece = pieceOnSquare(sq);

		if (piece.Type != noType)
		{
			const Score scores = Eval::pieceSquareScore(piece, sq);
			pst[piece.Color][0] += scores.first;
			pst[piece.Color][1] += scores.second;
		}
	}

	return std::make_pair(pst[color][0], pst[color][1]);
}

void Pos::addPiece(const pieceInfo piece, const uint8_t sq)
{
	pieceSet[sq] = piece;

	if (piece.Type != noType)
	{
		numPieces[piece.Color][piece.Type]++;
		material[piece.Color] += pieceValue[piece.Type];
		zobrist ^= Zobrist::pieceInfo[piece.Color][piece.Type][sq];

		if (piece.Type == Pawn)
			pawnsOnFile[piece.Color][Square::getFileIndex(sq)]++;
	}
}

void Pos::clearPieceSet()
{
	for (uint8_t i = 0; i < 64; i++)
	{
		pieceSet[i] = Null;
	}
}

void Pos::updateGenericBitBoards()
{
	pieces[White] =
		bitBoardSet[White][Pawn] | bitBoardSet[White][Knight]
		| bitBoardSet[White][Bishop] | bitBoardSet[White][Rook]
		| bitBoardSet[White][Queen] | bitBoardSet[White][King];

	pieces[Black] =
		bitBoardSet[Black][Pawn] | bitBoardSet[Black][Knight]
		| bitBoardSet[Black][Bishop] | bitBoardSet[Black][Rook]
		| bitBoardSet[Black][Queen] | bitBoardSet[Black][King];

	occupiedSquares = pieces[White] | pieces[Black];
	emptySquares = ~occupiedSquares;
}

void Pos::initializeCastlingStatus(const Fen& fenString)
{
	castlingStatus = 0;

	if (fenString.whiteCanCastleShort)
		castlingStatus |= Castle::whiteCastleOO;

	if (fenString.whiteCanCastleLong)
		castlingStatus |= Castle::whiteCastleOOO;

	if (fenString.blackCanCastleShort)
		castlingStatus |= Castle::blackCastleOO;

	if (fenString.blackCanCastleLong)
		castlingStatus |= Castle::blackCastleOOO;

	zobrist ^= Zobrist::Castling[castlingStatus];
}

void Pos::initializesideToMove(const Fen& fenString)
{
	sideToMove = fenString.sideToMove;

	if (sideToMove == Black)
		zobrist ^= Zobrist::Color;
}

void Pos::initializePieceSet(const Fen& fenString)
{
	for (uint8_t i = 0; i < 64; i++)
	{
		addPiece(fenString.piecePlacement[i], i);
	}
}

void Pos::initializeEnPassantSquare(const Fen& fenString)
{
	enPassantSquare = fenString.getPassantSquare;

	if (enPassantSquare != Square::noSquare)
		zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];
}

void Pos::initializeHalfMoveClock(const Fen& fenString)
{
	halfMoveClock = fenString.halfMove;
}

void Pos::initializeBitBoards(const Fen& fenString)
{
	for (uint8_t i = Pawn; i < noType; i++)
		for (uint8_t l = White; l < noColor; l++)
			bitBoardSet[l][i] = 0;

	for (uint8_t i = 0; i < 64; i++)
	{
		if (fenString.piecePlacement[i].Type == King)
			kingSquare[fenString.piecePlacement[i].Color] = i;

		if (fenString.piecePlacement[i].Color != noColor)
			bitBoardSet[fenString.piecePlacement[i].Color][fenString.piecePlacement[i].Type] |= Masks::squareMask[i];
	}

	updateGenericBitBoards();
}

Move Pos::parseMove(const std::string& str) const
{
	const uint8_t from = Square::Parse(str.substr(0, 2));
	const uint8_t to = Square::Parse(str.substr(2));
	Move move;

	if (to == enPassantSquare && pieceSet[from].Type == Pawn)
		move = Move(from, to, EnPassant);

	else if (str == "e1g1")
		move = Castle::whiteCastlingOO;

	else if (str == "e8g8")
		move = Castle::blackCastlingOO;

	else if (str == "e1c1")
		move = Castle::whiteCastlingOOO;

	else if (str == "e8c8")
		move = Castle::blackCastlingOOO;

	else if (str.size() == 5)
		move = Move(from, to, 0x8 | (Piece::getPiece(str[4]) - 1));

	else
		move = Move(from, to);

	return move;
}

void Pos::makeMove(const Move move)
{
	bool incrementClock = true;

	const uint8_t from = move.fromSquare();
	const uint8_t to = move.toSquare();
	const uint8_t captured = move.isEnPassant() ? static_cast<uint8_t>(Pawn) : pieceSet[to].Type;
	const uint8_t pieceMoved = pieceSet[from].Type;
	const uint8_t enemy = Piece::getOpposite(sideToMove);

	const bool capture = captured != noType;

	castlingStatusHistory[currentPly] = castlingStatus;
	enpSquaresHistory[currentPly] = enPassantSquare;
	halfMoveClockHistory[currentPly] = halfMoveClock;
	hashHistory[currentPly] = zobrist;
	capturedPieceHistory[currentPly] = captured;

	zobrist ^= Zobrist::Color;

	pieceSet[to] = pieceSet[from];
	pieceSet[from] = Null;

	updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceMoved), from));
	updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceMoved), to));

	const uint64_t From = Masks::squareMask[from];
	const uint64_t To = Masks::squareMask[to];
	const uint64_t FromTo = From | To;

	bitBoardSet[sideToMove][pieceMoved] ^= FromTo;
	zobrist ^= Zobrist::pieceInfo[sideToMove][pieceMoved][from];
	zobrist ^= Zobrist::pieceInfo[sideToMove][pieceMoved][to];

	pieces[sideToMove] ^= FromTo;

	if (pieceMoved == King)
	{
		kingSquare[sideToMove] = to;

		if (move.isCastle())
		{
			makeCastle(from, to);
		}

		if (sideToMove == White)
			castlingStatus &=
			~(Castle::whiteCastleOO | Castle::whiteCastleOOO);
		else
			castlingStatus &=
			~(Castle::blackCastleOO | Castle::blackCastleOOO);
	}
	else if (pieceMoved == Rook)
	{
		if (castlingStatus)
		{
			if (sideToMove == White)
			{
				if (from == Square::A1)
					castlingStatus &= ~Castle::whiteCastleOOO;

				else if (from == Square::H1)
					castlingStatus &= ~Castle::whiteCastleOO;
			}
			else
			{
				if (from == Square::A8)
					castlingStatus &= ~Castle::blackCastleOOO;

				else if (from == Square::H8)
					castlingStatus &= ~Castle::blackCastleOO;
			}
		}
	}
	else if (move.isPromotion())
	{
		const uint8_t promoted = move.piecePromoted();
		pieceSet[to] = pieceInfo(sideToMove, promoted);
		bitBoardSet[sideToMove][Pawn] ^= To;
		bitBoardSet[sideToMove][promoted] ^= To;
		numPieces[sideToMove][Pawn]--;
		numPieces[sideToMove][promoted]++;

		material[sideToMove] -= pieceValue[Pawn];
		material[sideToMove] += pieceValue[promoted];
		zobrist ^= Zobrist::pieceInfo[sideToMove][Pawn][to];
		zobrist ^= Zobrist::pieceInfo[sideToMove][promoted][to];
		updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, Pawn), to));
		updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, promoted), to));

		if (!capture)
			pawnsOnFile[sideToMove][Square::getFileIndex(from)]--;
		else
			pawnsOnFile[sideToMove][Square::getFileIndex(to)]--;
	}

	if (capture)
	{
		if (move.isEnPassant())
		{
			uint64_t piece;

			if (sideToMove == White)
			{
				piece = Masks::squareMask[enPassantSquare - 8];
				pieceSet[enPassantSquare - 8] = Null;
				updatePstScore
					<Sub>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, Pawn), enPassantSquare - 8));
				zobrist ^= Zobrist::pieceInfo[enemy][Pawn][enPassantSquare
					- 8];
			}
			else
			{
				piece = Masks::squareMask[enPassantSquare + 8];
				pieceSet[enPassantSquare + 8] = Null;
				updatePstScore
					<Sub>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, Pawn), enPassantSquare + 8));
				zobrist ^= Zobrist::pieceInfo[enemy][Pawn][enPassantSquare
					+ 8];
			}

			pieces[enemy] ^= piece;
			bitBoardSet[enemy][Pawn] ^= piece;
			occupiedSquares ^= FromTo ^ piece;
			emptySquares ^= FromTo ^ piece;

			pawnsOnFile[sideToMove][Square::getFileIndex(from)]--;
			pawnsOnFile[sideToMove][Square::getFileIndex(to)]++;
			pawnsOnFile[enemy][Square::getFileIndex(to)]--;
		}
		else
		{
			if (captured == Rook)
			{
				if (enemy == White)
				{
					if (to == Square::H1)
						castlingStatus &= ~Castle::whiteCastleOO;

					else if (to == Square::A1)
						castlingStatus &= ~Castle::whiteCastleOOO;
				}
				else
				{
					if (to == Square::H8)
						castlingStatus &= ~Castle::blackCastleOO;

					else if (to == Square::A8)
						castlingStatus &= ~Castle::blackCastleOOO;
				}
			}
			else if (captured == Pawn)
			{
				pawnsOnFile[enemy][Square::getFileIndex(to)]--;
			}

			if (pieceMoved == Pawn)
			{
				pawnsOnFile[sideToMove][Square::getFileIndex(from)]--;
				pawnsOnFile[sideToMove][Square::getFileIndex(to)]++;
			}

			updatePstScore<Sub>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, captured), to));
			bitBoardSet[enemy][captured] ^= To;
			pieces[enemy] ^= To;
			occupiedSquares ^= From;
			emptySquares ^= From;
			zobrist ^= Zobrist::pieceInfo[enemy][captured][to];
		}

		numPieces[enemy][captured]--;
		material[enemy] -= pieceValue[captured];
		incrementClock = false;
	}
	else
	{
		occupiedSquares ^= FromTo;
		emptySquares ^= FromTo;
	}

	if (enPassantSquare != Square::noSquare)
		zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];

	enPassantSquare = Square::noSquare;

	if (pieceMoved == Pawn)
	{
		incrementClock = false;
		const int sq = to - from;

		if (sq == 16 || sq == -16)
		{
			enPassantSquare = to - sq / 2;
			zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];
		}
	}

	if (castlingStatusHistory[currentPly] != castlingStatus)
		zobrist ^= Zobrist::Castling[castlingStatus];

	if (incrementClock)
		halfMoveClock++;
	else
		halfMoveClock = 0;

	sideToMove = enemy;
	currentPly++;
}

void Pos::undoMove(const Move move)
{
	const uint8_t from = move.fromSquare();
	const uint8_t to = move.toSquare();
	const uint8_t enemy = sideToMove;
	const bool promotion = move.isPromotion();
	uint8_t pieceMoved;

	currentPly--;

	const uint8_t captured = capturedPieceHistory[currentPly];
	const bool capture = captured != noType;

	zobrist ^= Zobrist::Color;

	if (castlingStatusHistory[currentPly] != castlingStatus)
		zobrist ^= Zobrist::Castling[castlingStatus];

	if (enPassantSquare != Square::noSquare)
		zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];

	if (enpSquaresHistory[currentPly] != Square::noSquare)
		zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enpSquaresHistory[currentPly])];

	halfMoveClock = halfMoveClockHistory[currentPly];

	if (promotion)
		pieceMoved = Pawn;
	else
		pieceMoved = pieceSet[to].Type;

	sideToMove = Piece::getOpposite(sideToMove);

	pieceSet[from] = pieceSet[to];

	if (!promotion)
	{
		updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceMoved), to));
		updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceMoved), from));
	}

	const uint64_t From = Masks::squareMask[from];
	const uint64_t To = Masks::squareMask[to];
	const uint64_t FromTo = From | To;

	bitBoardSet[sideToMove][pieceMoved] ^= FromTo;
	zobrist ^= Zobrist::pieceInfo[sideToMove][pieceMoved][from];
	zobrist ^= Zobrist::pieceInfo[sideToMove][pieceMoved][to];

	pieces[sideToMove] ^= FromTo;

	if (pieceMoved == King)
	{
		kingSquare[sideToMove] = from;

		if (move.isCastle())
		{
			undoCastle(from, to);
		}

		castlingStatus = castlingStatusHistory[currentPly];
	}
	else if (pieceMoved == Rook)
	{
		castlingStatus = castlingStatusHistory[currentPly];
	}
	else if (promotion)
	{
		const uint8_t promoted = move.piecePromoted();
		numPieces[sideToMove][Pawn]++;
		numPieces[sideToMove][promoted]--;

		material[sideToMove] += pieceValue[Pawn];
		material[sideToMove] -= pieceValue[promoted];
		pieceSet[from] = pieceInfo(sideToMove, Pawn);
		bitBoardSet[sideToMove][promoted] ^= To;
		bitBoardSet[sideToMove][Pawn] ^= To;
		zobrist ^= Zobrist::pieceInfo[sideToMove][Pawn][to];
		zobrist ^= Zobrist::pieceInfo[sideToMove][promoted][to];
		updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, Pawn), from));
		updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, promoted), to));

		if (!capture)
			pawnsOnFile[sideToMove][Square::getFileIndex(from)]++;
		else
			pawnsOnFile[sideToMove][Square::getFileIndex(to)]++;
	}

	enPassantSquare = enpSquaresHistory[currentPly];

	if (capture)
	{
		if (move.isEnPassant())
		{
			pieceSet[to] = Null;
			uint64_t piece;
			constexpr uint8_t offset = 8;

			if (sideToMove == White)
			{
				piece = Masks::squareMask[enPassantSquare - offset];
				pieceSet[enPassantSquare - offset] = pieceInfo(Black, Pawn);
				updatePstScore<Add>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, Pawn), enPassantSquare - offset));
				zobrist ^= Zobrist::pieceInfo[enemy][Pawn][enPassantSquare - offset];
			}
			else
			{
				piece = Masks::squareMask[enPassantSquare + offset];
				pieceSet[enPassantSquare + offset] = pieceInfo(White, Pawn);
				updatePstScore<Add>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, Pawn), enPassantSquare + offset));
				zobrist ^= Zobrist::pieceInfo[enemy][Pawn][enPassantSquare + offset];
			}

			pieces[enemy] ^= piece;
			bitBoardSet[enemy][Pawn] ^= piece;
			occupiedSquares ^= FromTo ^ piece;
			emptySquares ^= FromTo ^ piece;

			pawnsOnFile[sideToMove][Square::getFileIndex(from)]++;
			pawnsOnFile[sideToMove][Square::getFileIndex(to)]--;
			pawnsOnFile[enemy][Square::getFileIndex(to)]++;
		}
		else
		{
			if (captured == Rook)
			{
				castlingStatus = castlingStatusHistory[currentPly];
			}
			else if (captured == Pawn)
			{
				pawnsOnFile[enemy][Square::getFileIndex(to)]++;
			}

			if (pieceMoved == Pawn)
			{
				pawnsOnFile[sideToMove][Square::getFileIndex(from)]++;
				pawnsOnFile[sideToMove][Square::getFileIndex(to)]--;
			}

			updatePstScore<Add>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, captured), to));

			pieceSet[to] = pieceInfo(enemy, captured);
			bitBoardSet[enemy][captured] ^= To;

			pieces[enemy] ^= To;
			occupiedSquares ^= From;
			emptySquares ^= From;

			zobrist ^= Zobrist::pieceInfo[enemy][captured][to];
		}

		numPieces[enemy][captured]++;
		material[enemy] += pieceValue[captured];
	}
	else
	{
		pieceSet[to] = Null;
		occupiedSquares ^= FromTo;
		emptySquares ^= FromTo;
	}
}

void Pos::makeCastle(const uint8_t from, const uint8_t to)
{
	uint8_t fromR;
	uint8_t toR;

	if (from < to)
	{
		if (sideToMove == White)
		{
			fromR = Square::H1;
			toR = Square::F1;
		}
		else
		{
			fromR = Square::H8;
			toR = Square::F8;
		}
	}
	else
	{
		if (sideToMove == White)
		{
			fromR = Square::A1;
			toR = Square::D1;
		}
		else
		{
			fromR = Square::A8;
			toR = Square::D8;
		}
	}

	const uint64_t rook = Masks::squareMask[fromR] | Masks::squareMask[toR];
	pieces[sideToMove] ^= rook;
	bitBoardSet[sideToMove][Rook] ^= rook;
	occupiedSquares ^= rook;
	emptySquares ^= rook;
	pieceSet[fromR] = Null;
	pieceSet[toR] = pieceInfo(sideToMove, Rook);

	updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, Rook), fromR));
	updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, Rook), toR));

	zobrist ^= Zobrist::pieceInfo[sideToMove][Rook][fromR];
	zobrist ^= Zobrist::pieceInfo[sideToMove][Rook][toR];
	castled[sideToMove] = true;
}

void Pos::undoCastle(const uint8_t from, const uint8_t to)
{
	uint8_t fromR;
	uint8_t toR;

	if (from < to)
	{
		if (sideToMove == White)
		{
			fromR = Square::H1;
			toR = Square::F1;
		}
		else
		{
			fromR = Square::H8;
			toR = Square::F8;
		}
	}
	else
	{
		if (sideToMove == White)
		{
			fromR = Square::A1;
			toR = Square::D1;
		}
		else
		{
			fromR = Square::A8;
			toR = Square::D8;
		}
	}

	const uint64_t rook = Masks::squareMask[fromR] | Masks::squareMask[toR];
	pieces[sideToMove] ^= rook;
	bitBoardSet[sideToMove][Rook] ^= rook;
	occupiedSquares ^= rook;
	emptySquares ^= rook;
	pieceSet[fromR] = pieceInfo(sideToMove, Rook);
	pieceSet[toR] = Null;

	updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, Rook), fromR));
	updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, Rook), toR));

	castlingStatus = castlingStatusHistory[currentPly];

	zobrist ^= Zobrist::pieceInfo[sideToMove][Rook][fromR];
	zobrist ^= Zobrist::pieceInfo[sideToMove][Rook][toR];
	castled[sideToMove] = false;
}

bool Pos::isAttacked(uint64_t target, const uint8_t side) const
{
	const uint8_t enemyColor = Piece::getOpposite(side);

	while (target != 0)
	{
		uint8_t to = BSFReset(target);
		const uint64_t pawnAttacks = Moves::pawnAttacks[side][to];

		if ((Pieces(enemyColor, Pawn) & pawnAttacks) != 0)
			return true;

		if ((Pieces(enemyColor, Knight) & Moves::knightAttacks[to]) != 0)
			return true;

		if ((Pieces(enemyColor, King) & Moves::kingAttacks[to]) != 0)
			return true;

		uint64_t slidingAttackers = Pieces(enemyColor, Queen) | Pieces(enemyColor, Rook);

		if (slidingAttackers != 0)
		{
#ifdef PEXT
			if ((Moves::getRookAttacks(occupiedSquares, to) & slidingAttackers) != 0)
				return true;
#else
			if ((Moves::getRankAttacks(occupiedSquares, to) & slidingAttackers) != 0)
				return true;

			if ((Moves::getFileAttacks(occupiedSquares, to) & slidingAttackers) != 0)
				return true;
#endif
		}

		slidingAttackers = Pieces(enemyColor, Queen) | Pieces(enemyColor, Bishop);

		if (slidingAttackers != 0)
		{
			if ((Moves::getH1A8DiagonalAttacks(occupiedSquares, to) & slidingAttackers) != 0)
				return true;

			if ((Moves::getA1H8DiagonalAttacks(occupiedSquares, to) & slidingAttackers) != 0)
				return true;
		}
	}
	return false;
}

std::string Pos::getFen() const
{
	std::string fen;

	for (int r = 7; r >= 0; r--)
	{
		int empty = 0;

		for (int c = 0; c < 8; c++)
		{
			if (pieceSet[Square::getSquareIndex(c, r)].Type == noType)
				empty++;
			else
			{
				if (empty != 0)
				{
					fen += static_cast<char>(empty) + '0';
					empty = 0;
				}

				fen += Piece::getInitial(pieceSet[Square::getSquareIndex(c, r)]);
			}
		}

		if (empty != 0)
			fen += static_cast<char>(empty) + '0';

		if (r > 0)
			fen += '/';
	}

	fen += " ";

	if (sideToMove == White)
		fen += "w";
	else
		fen += "b";

	fen += " ";

	if (castlingStatus)
	{
		fen += (castlingStatus & Castle::whiteCastleOO ? "K" : "");
		fen += (castlingStatus & Castle::whiteCastleOOO ? "Q" : "");
		fen += (castlingStatus & Castle::blackCastleOO ? "k" : "");
		fen += (castlingStatus & Castle::blackCastleOOO ? "q" : "");
	}
	else
		fen += '-';

	fen += " ";

	if (enPassantSquare != Square::noSquare)
		fen += Square::toAlgebraic(enPassantSquare);
	else
		fen += '-';

	fen += " ";
	fen += "0 1";

	return fen;
}

void Pos::Display() const
{
	for (int r = 7; r >= 0; r--)
	{
		for (int c = 0; c <= 7; c++)
		{
			const pieceInfo piece = pieceSet[Square::getSquareIndex(c, r)];
			if (piece.Type != noType)
				std::cout << Piece::getInitial(piece);
			else
				std::cout << "-";
		}
		std::cout << std::endl;
	}
}

#pragma once
#include "pragma.h"
#include "position.h"
#include "move.h"
#include "piece.h"
#include "moves.h"
#include "hashtable.h"
#include "zobrist.h"
#include "uci.h"
#include "pawn.h"
#include "masks.h"
#include "ranks.h"
#include "evalterms.h"
#include "searchinfo.h"
#include "square.h"

constexpr int maxPhase = 256;
const pieceInfo Null = pieceInfo(noColor, noType);
const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr uint64_t Empty = 0x0000000000000000;

class MoveList;
class Fen;

enum GameStage
{
	OP = 0,
	EG = 1
};

enum Operation
{
	Add,
	Sub
};

class Pos
{
public:
	uint64_t occupiedSquares;
	uint64_t emptySquares;

	uint64_t zobrist{};

	Pos() noexcept;
	void Display() const;
	void loadFen(const std::string & = startPosition);
	void addPiece(pieceInfo, uint8_t);
	uint64_t ourPieces() const;
	uint64_t enemyPieces() const;
	uint64_t Pieces(uint8_t, uint8_t) const;
	uint64_t Pieces(uint8_t) const;
	uint64_t pinnedPieces() const;
	uint64_t kingAttackers(uint8_t, uint8_t) const;
	uint64_t attacksTo(uint8_t, uint8_t, uint64_t) const;
	uint64_t movesTo(uint8_t, uint8_t, uint64_t) const;
	std::pair<uint64_t, uint8_t> leastValuableAttacker(uint8_t, uint64_t) const;

	int See(Move) const;

	pieceInfo pieceOnSquare(uint8_t) const;
	const pieceInfo* pieceList() const;

	void makeMove(Move);
	void undoMove(Move);
	void makeNullMove();
	void undoNullMove();

	void setCheckState(bool);

	bool isCapture(Move) const;
	bool isMoveLegal(Move, uint64_t);
	bool isAttacked(uint64_t, uint8_t) const;
	bool isPromotingPawn() const;
	bool isOnSquare(uint8_t, uint8_t, uint8_t) const;
	bool isRepetition() const;

	uint8_t getKingSquare(uint8_t) const;

	uint8_t getSideToMove() const;
	uint8_t getCastlingStatus() const;
	uint8_t getPassantSquare() const;

	int getHalfMoveClock() const;
	int getCurrentPly() const;
	bool getAllowNullMove() const;
	void toggleNullMove();
	bool getIsCheck() const;

	Score getPstScore(uint8_t) const;
	int getNumPieces(uint8_t, uint8_t) const;
	int getNumPieces(uint8_t) const;
	int Material(uint8_t) const;
	int Material() const;
	int materialScore(uint8_t) const;
	int getPawnsOnFile(uint8_t, uint8_t) const;

	bool EG() const;
	bool hasCastled(uint8_t) const;
	int Phase() const;

	std::string getFen() const;
	Move parseMove(const std::string&) const;

private:
	uint8_t castlingStatusHistory[maxPly]{};
	uint8_t capturedPieceHistory[maxPly]{};
	uint8_t enpSquaresHistory[maxPly]{};
	uint64_t hashHistory[maxPly]{};
	int halfMoveClockHistory[maxPly]{};

	uint64_t bitBoardSet[2][7]{};
	uint8_t kingSquare[2]{};

	pieceInfo pieceSet[74];
	uint64_t pieces[2]{};

	uint8_t sideToMove{};
	uint8_t castlingStatus{};
	uint8_t enPassantSquare{};

	int halfMoveClock{};
	int currentPly{};
	bool allowNullMove{};
	bool isCheck{};
	bool castled[2] =
	{
		false, false
	};

	int numPieces[2][6]{};
	int pawnsOnFile[2][8]{};

	Score pstScore[2];
	int material[2]{};

	template <Operation>
	void updatePstScore(uint8_t, Score);

	void clearPieceSet();
	void updateGenericBitBoards();
	void initializeBitBoards(const Fen&);
	void initializesideToMove(const Fen&);
	void initializeCastlingStatus(const Fen&);
	void initializeEnPassantSquare(const Fen&);
	void initializeHalfMoveClock(const Fen&);
	void initializePieceSet(const Fen&);
	void makeCastle(uint8_t, uint8_t);
	void undoCastle(uint8_t, uint8_t);
	Score calculatePST(uint8_t) const;
};

INLINE uint64_t Pos::pinnedPieces() const
{
	const uint8_t enemy = Piece::getOpposite(sideToMove);
	const int kingSq = kingSquare[sideToMove];

	const uint64_t playerPieces = ourPieces();
	uint64_t pinned = 0;
	uint64_t pinners = ((bitBoardSet[enemy][Rook]
		| bitBoardSet[enemy][Queen]) & Moves::pseudoRookAttacks[kingSq])
		| ((bitBoardSet[enemy][Bishop]
			| bitBoardSet[enemy][Queen]) & Moves::pseudoBishopAttacks[kingSq]);

	while (pinners)
	{
		const int sq = BSFReset(pinners);
		const uint64_t b = Moves::obstructedTable[sq][kingSq] & occupiedSquares;

		if ((b != 0) && ((b & (b - 1)) == 0) && ((b & playerPieces) != 0))
		{
			pinned |= b;
		}
	}
	return pinned;
}

INLINE bool Pos::isMoveLegal(const Move move, const uint64_t pinned)
{
	if (pieceSet[move.fromSquare()].Type == King)
	{
		return !isAttacked(Masks::squareMask[move.toSquare()], sideToMove);
	}

	if (move.isEnPassant())
	{
		makeMove(move);
		const bool islegal =
			!isAttacked(bitBoardSet[Piece::getOpposite(sideToMove)][King], Piece::getOpposite(sideToMove));
		undoMove(move);
		return islegal;
	}

	return (pinned == 0) || ((pinned & Masks::squareMask[move.fromSquare()]) == 0)
		|| Moves::AreSquaresAligned(move.fromSquare(), move.toSquare(), kingSquare[sideToMove]);
}

INLINE uint64_t Pos::kingAttackers(uint8_t square, const uint8_t color) const
{
	const uint8_t opp = Piece::getOpposite(color);
	const uint64_t bishopAttacks =
		Moves::getA1H8DiagonalAttacks(occupiedSquares, square) | Moves::getH1A8DiagonalAttacks(occupiedSquares, square);

#ifdef PEXT
	uint64_t rookAttacks = Moves::getRookAttacks(occupiedSquares, square);
#else
	uint64_t rookAttacks =
		Moves::getFileAttacks(occupiedSquares, square) | Moves::getRankAttacks(occupiedSquares, square);
#endif

	return (Moves::pawnAttacks[color][square] & bitBoardSet[opp][Pawn])
		| (Moves::knightAttacks[square] & bitBoardSet[opp][Knight])
		| (bishopAttacks & (bitBoardSet[opp][Bishop] | bitBoardSet[opp][Queen]))
		| (rookAttacks & (bitBoardSet[opp][Rook] | bitBoardSet[opp][Queen]));
}

INLINE uint64_t Pos::attacksTo(uint8_t square, const uint8_t color, uint64_t occ) const
{
	const uint8_t opp = Piece::getOpposite(color);
	const uint64_t bishopAttacks = Moves::getA1H8DiagonalAttacks(occ, square) |
		Moves::getH1A8DiagonalAttacks(occ, square);

#ifdef PEXT
	uint64_t rookAttacks = Moves::getRookAttacks(occ, square);
#else
	uint64_t rookAttacks = Moves::getFileAttacks(occ, square) | Moves::getRankAttacks(occ, square);
#endif

	return (Moves::kingAttacks[square] & bitBoardSet[color][King])
		| (Moves::pawnAttacks[opp][square] & bitBoardSet[color][Pawn])
		| (Moves::knightAttacks[square] & bitBoardSet[color][Knight])
		| (bishopAttacks & (bitBoardSet[color][Bishop] | bitBoardSet[color][Queen]))
		| (rookAttacks & (bitBoardSet[color][Rook] | bitBoardSet[color][Queen]));
}

INLINE uint64_t Pos::movesTo(uint8_t square, const uint8_t color, uint64_t occ) const
{
	const uint64_t bishopAttacks = Moves::getA1H8DiagonalAttacks(occ, square) |
		Moves::getH1A8DiagonalAttacks(occ, square);

#ifdef PEXT
	uint64_t rookAttacks = Moves::getRookAttacks(occ, square);
#else
	uint64_t rookAttacks = Moves::getFileAttacks(occ, square) | Moves::getRankAttacks(occ, square);
#endif

	uint8_t pawnSquare;
	uint64_t pawn = 0;
	uint64_t pawnMove = 0;
	const uint8_t enemy = Piece::getOpposite(color);

	if (color == White)
	{
		if (square >= 16)
		{
			pawnSquare = square - 16;
			pawn = Masks::squareMask[pawnSquare] & bitBoardSet[color][Pawn];
			pawnMove = Pawn::getQuietTargets(color, pawn, emptySquares) & Masks::squareMask[square];
		}

		if (square >= 8)
		{
			pawnSquare = square - 8;
			pawn |= Masks::squareMask[pawnSquare] & bitBoardSet[color][Pawn];
			pawnMove |= Pawn::getQuietTargets(color, pawn, emptySquares) & Masks::squareMask[square];
		}
	}
	else
	{
		if (square <= (63 - 16))
		{
			pawnSquare = square + 16;
			pawn = Masks::squareMask[pawnSquare] & bitBoardSet[color][Pawn];
			pawnMove = Pawn::getQuietTargets(color, pawn, emptySquares) & Masks::squareMask[square];
		}

		if (square <= (63 - 8))
		{
			pawnSquare = square + 8;
			pawn |= Masks::squareMask[pawnSquare] & bitBoardSet[color][Pawn];
			pawnMove |= Pawn::getQuietTargets(color, pawn, emptySquares) & Masks::squareMask[square];
		}
	}

	return (Moves::kingAttacks[square] & bitBoardSet[color][King])
		| (Masks::squareMask[square] & Pieces(enemy)
			? (Moves::pawnAttacks[enemy][square] & bitBoardSet[color][Pawn])
			: 0)
		| (pawn) | (Moves::knightAttacks[square] & bitBoardSet[color][Knight])
		| (bishopAttacks & (bitBoardSet[color][Bishop] | bitBoardSet[color][Queen]))
		| (rookAttacks & (bitBoardSet[color][Rook] | bitBoardSet[color][Queen]));
}

inline void Pos::makeNullMove()
{
	hashHistory[currentPly] = zobrist;
	enpSquaresHistory[currentPly] = enPassantSquare;
	sideToMove = Piece::getOpposite(sideToMove);
	enPassantSquare = Square::noSquare;

	zobrist ^= Zobrist::Color;

	if (enPassantSquare != Square::noSquare)
		zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];

	if (enpSquaresHistory[currentPly] != Square::noSquare)
		zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enpSquaresHistory[currentPly])];

	allowNullMove = false;
	currentPly++;
}

inline void Pos::undoNullMove()
{
	currentPly--;
	sideToMove = Piece::getOpposite(sideToMove);
	enPassantSquare = enpSquaresHistory[currentPly];

	zobrist ^= Zobrist::Color;

	if (enpSquaresHistory[currentPly] != Square::noSquare)
		zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enpSquaresHistory[currentPly])];

	allowNullMove = true;
}

inline uint8_t Pos::getCastlingStatus() const
{
	return castlingStatus;
}

INLINE uint8_t Pos::getSideToMove() const
{
	return sideToMove;
}

inline uint8_t Pos::getPassantSquare() const
{
	return enPassantSquare;
}

inline int Pos::getHalfMoveClock() const
{
	return halfMoveClock;
}

inline int Pos::getCurrentPly() const
{
	return currentPly;
}

inline bool Pos::getAllowNullMove() const
{
	return allowNullMove;
}

inline void Pos::toggleNullMove()
{
	allowNullMove = !allowNullMove;
}

inline bool Pos::getIsCheck() const
{
	return isCheck;
}

inline void Pos::setCheckState(const bool isCheck)
{
	this->isCheck = isCheck;
}

inline uint64_t Pos::ourPieces() const
{
	return pieces[sideToMove];
}

inline uint64_t Pos::enemyPieces() const
{
	return pieces[Piece::getOpposite(sideToMove)];
}

inline uint64_t Pos::Pieces(const uint8_t color, const uint8_t type) const
{
	return bitBoardSet[color][type];
}

inline uint64_t Pos::Pieces(const uint8_t color) const
{
	return pieces[color];
}

inline pieceInfo Pos::pieceOnSquare(const uint8_t square) const
{
	return pieceSet[square];
}

inline const pieceInfo* Pos::pieceList() const
{
	return pieceSet;
}

inline bool Pos::isPromotingPawn() const
{
	const uint64_t rank = (sideToMove == White ? Ranks::Seven : Ranks::Two);
	return (bitBoardSet[sideToMove][Pawn] & rank);
}

inline bool Pos::isCapture(const Move move) const
{
	return (pieceSet[move.toSquare()].Type != noType || move.isEnPassant());
}

inline bool Pos::isOnSquare(const uint8_t color, const uint8_t type, const uint8_t sq) const
{
	return (bitBoardSet[color][type] & Masks::squareMask[sq]);
}

inline int Pos::getPawnsOnFile(const uint8_t color, const uint8_t file) const
{
	return pawnsOnFile[color][file];
}

inline uint8_t Pos::getKingSquare(const uint8_t color) const
{
	return kingSquare[color];
}

inline Score Pos::getPstScore(const uint8_t color) const
{
	return pstScore[color];
}

inline int Pos::getNumPieces(const uint8_t color, const uint8_t type) const
{
	return numPieces[color][type];
}

inline int Pos::getNumPieces(const uint8_t type) const
{
	return numPieces[White][type] + numPieces[Black][type];
}

inline int Pos::Material(const uint8_t color) const
{
	return material[color];
}

inline int Pos::Material() const
{
	return material[White] + material[Black];
}

inline int Pos::materialScore(const uint8_t color) const
{
	return material[color] - material[Piece::getOpposite(color)];
}

inline std::pair<uint64_t, uint8_t> Pos::leastValuableAttacker(const uint8_t color, const uint64_t attackers) const
{
	for (uint8_t type = Pawn; type < noType; type++)
	{
		const uint64_t set = Pieces(color, type) & attackers;

		if (set)
			return std::make_pair(set & -set, type);
	}

	return std::make_pair(Empty, noType);
}

template <>
inline void Pos::updatePstScore<Add>(const uint8_t color, const Score score)
{
	pstScore[color].first += score.first;
	pstScore[color].second += score.second;
}

template <>
inline void Pos::updatePstScore<Sub>(const uint8_t color, const Score score)
{
	pstScore[color].first -= score.first;
	pstScore[color].second -= score.second;
}

inline int Pos::See(const Move move) const
{
	const uint8_t to = move.toSquare();
	const uint8_t from = move.fromSquare();
	const pieceInfo captured = pieceOnSquare(to);
	uint8_t attackingPiece = pieceOnSquare(from).Type;

	int gain[100];
	int depth = 0;

	uint64_t occ = occupiedSquares;

	gain[depth++] = pieceValue[captured.Type];

	uint8_t side = Piece::getOpposite(sideToMove);
	occ ^= Masks::squareMask[from];

	uint64_t attackers = attacksTo(to, side, occ) & occ;

	while (attackers)
	{
		gain[depth] = pieceValue[attackingPiece] - gain[depth - 1];

		const std::pair<uint64_t, uint8_t> tuple = leastValuableAttacker(side, attackers);
		const uint64_t fromSet = tuple.first;
		attackingPiece = tuple.second;

		occ ^= fromSet;
		side = Piece::getOpposite(side);
		attackers = attacksTo(to, side, occ) & occ;
		depth++;
	}

	while (--depth)
	{
		gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);
	}

	return gain[0];
}

inline bool Pos::isRepetition() const
{
	if (halfMoveClock >= 4)
	{
		const int start = getSideToMove() == White ? 0 : 1;

		for (int i = start; i < currentPly; i += 2)
		{
			if (hashHistory[i] == zobrist)
				return true;
		}
	}

	return false;
}

inline bool Pos::EG() const
{
	return Material() <= endGameMaterial;
}

inline int Pos::Phase() const
{
	const static int kingMaterial = pieceValue[King] * 2;
	const int nonPawnMaterial = (Material() - getNumPieces(Pawn) * pieceValue[Pawn] - kingMaterial);
	const int openingPieceMaterial = (openingNonPawnMaterial - kingMaterial);
	const int phase = (nonPawnMaterial * maxPhase + openingPieceMaterial / 2) / openingPieceMaterial;
	return maxPhase - phase;
}

inline bool Pos::hasCastled(const uint8_t color) const
{
	return castled[color];
}

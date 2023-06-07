#include "eval.h"
#include "piece.h"
#include "castle.h"
#include "evalterms.h"
#include "nnue-probe/nnue.h"

// nnue eval()
/*
enum pieceType :
uint8_t
{
Pawn,
Knight,
Bishop,
Rook,
Queen,
King,
noType
};

enum pieceColor :
uint8_t
{
White,
Black,
noColor
};
*/
/*
* Piece codes are
*     wking=1, wqueen=2, wrook=3, wbishop= 4, wknight= 5, wpawn= 6,
*     bking=7, bqueen=8, brook=9, bbishop=10, bknight=11, bpawn=12,
*/

int Eval::evaluate(const Pos& pos)
{
	int pieces[33]{};
	int squares[33]{};
	int index = 2;
	for (uint8_t i = 0; i < 64; i++)
	{
		if (pos.pieceOnSquare(i).Color == White && pos.pieceOnSquare(i).Type == King)
		{
			pieces[0] = 1;
			squares[0] = i;
		}
		else if (pos.pieceOnSquare(i).Color == Black && pos.pieceOnSquare(i).Type == King)
		{
			pieces[1] = 7;
			squares[1] = i;
		}
		else if (pos.pieceOnSquare(i).Color == White && pos.pieceOnSquare(i).Type == Pawn)
		{
			pieces[index] = 6;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == Black && pos.pieceOnSquare(i).Type == Pawn)
		{
			pieces[index] = 12;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == White && pos.pieceOnSquare(i).Type == Knight)
		{
			pieces[index] = 5;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == Black && pos.pieceOnSquare(i).Type == Knight)
		{
			pieces[index] = 11;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == White && pos.pieceOnSquare(i).Type == Bishop)
		{
			pieces[index] = 4;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == Black && pos.pieceOnSquare(i).Type == Bishop)
		{
			pieces[index] = 10;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == White && pos.pieceOnSquare(i).Type == Rook)
		{
			pieces[index] = 3;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == Black && pos.pieceOnSquare(i).Type == Rook)
		{
			pieces[index] = 9;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == White && pos.pieceOnSquare(i).Type == Queen)
		{
			pieces[index] = 2;
			squares[index] = i;
			index++;
		}
		else if (pos.pieceOnSquare(i).Color == Black && pos.pieceOnSquare(i).Type == Queen)
		{
			pieces[index] = 8;
			squares[index] = i;
			index++;
		}
	}
	const int nnue_score = nnue_evaluate(pos.getSideToMove(), pieces, squares);
	return nnue_score;
}

int Eval::evaluateHCE(const Pos& position)
{
	Score score(0, 0);
	const auto wking_square = position.getKingSquare(White);
	const auto bking_square = position.getKingSquare(Black);

	const int material = position.materialScore(White);

	// bishop pair bonus
	if (position.getNumPieces(White, Bishop) == 2)
		updateScore(score, bishopPair[OP], bishopPair[EG]);

	if (position.getNumPieces(Black, Bishop) == 2)
		updateScore(score, -bishopPair[OP], -bishopPair[EG]);

	// PST evaluation
	const Score wPstScore = position.getPstScore(White);
	const Score bPstScore = position.getPstScore(Black);

	updateScore(score, material + (wPstScore.first - bPstScore.first),
		material + (wPstScore.second - bPstScore.second));

	// tempo bonus
	if (position.getSideToMove() == White)
		updateScore(score, tempoBonus[OP], tempoBonus[EG]);
	else
		updateScore(score, -tempoBonus[OP], -tempoBonus[EG]);

	// premature queen development penalty
	if (!position.isOnSquare(White, Queen, Square::D1))
		updateScore(score, -queenPenalty[OP], -queenPenalty[EG]);
	if (!position.isOnSquare(Black, Queen, Square::D8))
		updateScore(score, queenPenalty[OP], queenPenalty[EG]);

	// doubled/isolated pawns
	const uint64_t wpawns = position.Pieces(White, Pawn);
	const uint64_t bpawns = position.Pieces(Black, Pawn);

	for (uint8_t file = 0; file < 8; file++)
	{
		int pawns;

		if ((pawns = position.getPawnsOnFile(White, file)))
		{
			if (!(wpawns & Moves::adjacentFiles[file]))
				updateScore(score, -isolatedPawn[OP], -isolatedPawn[EG]);

			if (pawns > 1)
				updateScore(score, -doubledPawn[OP], -doubledPawn[EG]);
		}
	}

	for (uint8_t file = 0; file < 8; file++)
	{
		int pawns;

		if ((pawns = position.getPawnsOnFile(Black, file)))
		{
			if (!(bpawns & Moves::adjacentFiles[file]))
				updateScore(score, isolatedPawn[OP], isolatedPawn[EG]);

			if (pawns > 1)
				updateScore(score, doubledPawn[OP], doubledPawn[EG]);
		}
	}

	const auto pieceList = position.pieceList();

	for (uint8_t sq = Square::A1; sq <= Square::H8; sq++)
	{
		const pieceInfo piece = pieceList[sq];

		if (piece.Type != noType)
		{
			// passed pawns
			if (piece.Type == Pawn)
			{
				if ((Moves::passerSpan[piece.Color][sq] & position.Pieces(Piece::getOpposite(piece.Color), Pawn)) == 0)
				{
					const auto rank = Square::getRankIndex(sq);

					if (piece.Color == White)
						updateScore(score, passedPawn[OP][rank], passedPawn[EG][rank]);
					else
						updateScore(score, -passedPawn[OP][7 - rank], -passedPawn[EG][7 - rank]);
				}
			}
			else
			{
				constexpr uint64_t allSquares = 0xFFFFFFFFFFFFFFFF;
				const uint8_t us = piece.Color;
				const uint8_t them = Piece::getOpposite(us);
				uint64_t b = 0;

				// mobility
				switch (piece.Type)
				{
				case Knight:
					b = Knight::targetsFrom(sq, us, position) & ~Pawn::getAnyAttack(
						position.Pieces(them, Pawn), them, allSquares);
					break;
				case Bishop:
					b = Bishop::targetsFrom(sq, us, position) & ~Pawn::getAnyAttack(
						position.Pieces(them, Pawn), them, allSquares);
					break;
				case Rook:
					b = Rook::targetsFrom(sq, us, position) & ~Pawn::getAnyAttack(
						position.Pieces(them, Pawn), them, allSquares);
					break;
				case Queen:
					b = Queen::targetsFrom(sq, us, position) & ~Pawn::getAnyAttack(
						position.Pieces(them, Pawn), them, allSquares);
					break;
				default:;
				}

				const int count = popcount(b);
				if (piece.Color == White)
					updateScore(score, mobilityBonus[OP][piece.Type][count], mobilityBonus[EG][piece.Type][count]);
				else
					updateScore(score, -mobilityBonus[OP][piece.Type][count], -mobilityBonus[EG][piece.Type][count]);
			}
		}
	}

	// pawn shelter
	int shelter1 = 0, shelter2 = 0;
	uint64_t pawns = position.Pieces(White, Pawn);

	if (Masks::squareMask[wking_square] & Castle::whiteKingSide)
	{
		shelter1 = popcount(pawns & Castle::whiteKingShield);
		shelter2 = popcount(pawns & Direction::oneStepNorth(Castle::whiteKingShield));
	}
	else if (Masks::squareMask[wking_square] & Castle::whiteQueenSide)
	{
		shelter1 = popcount(pawns & Castle::whiteQueenShield);
		shelter2 = popcount(pawns & Direction::oneStepNorth(Castle::whiteQueenShield));
	}
	updateScore(score,
		shelter1 * pawnShelter[OP][0] + shelter2 * pawnShelter[OP][1],
		shelter1 * pawnShelter[EG][0] + shelter2 * pawnShelter[EG][1]);

	pawns = position.Pieces(Black, Pawn);

	if (Masks::squareMask[bking_square] & Castle::blackKingSide)
	{
		shelter1 = popcount(pawns & Castle::blackKingShield);
		shelter2 = popcount(pawns & Direction::oneStepSouth(Castle::blackKingShield));
	}
	else if (Masks::squareMask[bking_square] & Castle::blackQueenSide)
	{
		shelter1 = popcount(pawns & Castle::blackQueenShield);
		shelter2 = popcount(pawns & Direction::oneStepSouth(Castle::blackQueenShield));
	}
	updateScore(score,
		-(shelter1 * pawnShelter[OP][0] + shelter2 * pawnShelter[OP][1]),
		-(shelter1 * pawnShelter[EG][0] + shelter2 * pawnShelter[EG][1]));

	const int opening = score.first;
	const int endgame = score.second;
	const int phase = position.Phase();
	const int finalScore = (opening * (maxPhase - phase) + endgame * phase) / maxPhase;
	return finalScore * (1 - position.getSideToMove() * 2);
}

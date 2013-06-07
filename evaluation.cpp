#include "evaluation.h"
#include "utils.h"
#include "board.h"
#include "piece.h"

namespace Napoleon
{
    int Evaluation::pawnSquareValue[64] =
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 15,-20,-20, 15, 10,  5,
        5, -5,-10,  0, 0,-10, -5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5,  5, 10, 25, 25, 10,  5,  5,
        10, 10, 20, 30, 30, 20, 10, 10,
        50, 50, 50, 50, 50, 50, 50, 50,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    int Evaluation::knightSquareValue[64] =
    {
        -50,-20,-30,-30,-30,-30,-20,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    };

    int Evaluation::bishopSquareValue[64] =
    {

        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  0, 20, 10, 10, 20,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    };

    int Evaluation::rookSquareValue[64] =
    {
        0,  0,  0,  5,  5,  0,  0,  0,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        5,  10, 20, 20, 20, 20, 10, 5,
        0,   0,  0,  0,  0,  0,  0, 0
    };

    int Evaluation::queenSquareValue[64] =
    {
        -20, -5, -5,  0,  0, -5, -5, -20,
        -10,  0,  0,  0,  0,  0,  0, -10,
        -5,   0,  5,  5,  5,  5,  0,  -5,
        -5,   0,  5,  5,  5,  5,  0,  -5,
        -5,   0,  5,  5,  5,  5,  0,  -5,
        -5,   0,  0,  0,  0,  0,  0,  -5,
        0,   5, 10, 10, 10, 10,  5,   0,
        0,   0,  0,  0,  0,  0,  0,   0
    };

    int Evaluation::kingSquareValue[64] =
    {
        20, 30, 50,  0,  0, 10, 35, 20,
        20, 10,  0,  0,  0,  0, 10, 20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30
    };

    int Evaluation::multiPawnP[8] = { 0, 0, 30, 65, 150, 300, 300, 300 };

    int Evaluation::Evaluate(Board & board)
    {
        using namespace Utils::BitBoard;
        using namespace PieceColor;
        using namespace Constants::Squares;

        int score = 0;

        // material evaluation
        int material = board.Material(White) - board.Material(Black);

        // Piece Square Value evaluation
        int wM = board.PstValue(White);
        int bM = board.PstValue(Black);

        score += material + (wM - bM);

        // premature queen development
        if (board.IsOnSquare(White, PieceType::Queen, IntD1))
            if (board.Material(White) > Constants::Eval::MiddleGameMat)
                score -= 10;

        if (board.IsOnSquare(Black, PieceType::Queen, IntD8))
            if (board.Material(Black) > Constants::Eval::MiddleGameMat)
                score += 10;

        // tempo bonus
        if (board.SideToMove == White)
            score += 5;
        else
            score -= 5;

        /* PAWN STRUCTURE */

        // doubled/tripled pawns evaluation
        for (File f = 0; f<8; f++)
        {
            score -= multiPawnP[board.PawnsOnFile(White, f)];
            score += multiPawnP[board.PawnsOnFile(Black, f)];
        }

        return (score * (1-(board.SideToMove*2)) );
    }

    int Evaluation::EvaluatePiece(Piece piece, Square square, Board& board)
    {
        using namespace Utils::Square;
        using namespace PieceColor;
        using namespace Constants::Squares;

        switch(piece.Type)
        {
        case PieceType::Pawn:
            return piece.Color ==  White ? pawnSquareValue[square] : pawnSquareValue[GetSquareIndex(square % 8, 7 - (square/8))];

        case PieceType::Knight:
            return piece.Color == White ? knightSquareValue[square] : knightSquareValue[GetSquareIndex(square % 8, 7 - (square/8))];

        case PieceType::Bishop:
            return piece.Color == White ? bishopSquareValue[square] : bishopSquareValue[GetSquareIndex(square % 8, 7 - (square/8))];

        case PieceType::Rook:
            return piece.Color == White ? rookSquareValue[square] : rookSquareValue[GetSquareIndex(square % 8, 7 - (square/8))];

        case PieceType::Queen:
            return piece.Color == White ? queenSquareValue[square] : queenSquareValue[GetSquareIndex(square % 8, 7 - (square/8))];

        case PieceType::King:
            return piece.Color == White ? kingSquareValue[square] : kingSquareValue[GetSquareIndex(square % 8, 7 - (square/8))];
        }

        return 0;
    }

    template<Byte piece>
    int Evaluation::evaluateMobility(Board& board, BitBoard pieces)
    {
        using namespace Utils::BitBoard;
        int square;
        BitBoard b = 0;

        switch(piece)
        {
        case PieceType::Bishop:
            while (pieces)
            {
                square = BitScanForwardReset(pieces);

                b |= MoveDatabase::GetA1H8DiagonalAttacks(board.OccupiedSquares, square)
                        | MoveDatabase::GetH1A8DiagonalAttacks(board.OccupiedSquares, square);
            }

            return 0.5*PopCount(b);

        case PieceType::Queen:
            while(pieces)
            {
                square = BitScanForwardReset(pieces);

                b |= MoveDatabase::GetA1H8DiagonalAttacks(board.OccupiedSquares, square)
                        | MoveDatabase::GetH1A8DiagonalAttacks(board.OccupiedSquares, square)
                        | MoveDatabase::GetFileAttacks(board.OccupiedSquares, square)
                        | MoveDatabase::GetRankAttacks(board.OccupiedSquares, square);
            }
            return 0.01*PopCount(b);

        default:
            break;
        }

        return 0;
    }
}

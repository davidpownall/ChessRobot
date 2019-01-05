#include "util.h"
#include "chessboard.h"

static ChessBoard *cb;

ChessBoard::ChessBoard(void)
{
    this->pieces[WHITE_PAWN]    = WHITE_PAWN_START;
    this->pieces[BLACK_PAWN]    = BLACK_PAWN_START;
    this->pieces[WHITE_ROOK]    = WHITE_ROOK_START;
    this->pieces[BLACK_ROOK]    = BLACK_ROOK_START;
    this->pieces[WHITE_KNIGHT]  = WHITE_KNIGHT_START;
    this->pieces[BLACK_KNIGHT]  = BLACK_KNIGHT_START;
    this->pieces[WHITE_BISHOP]  = WHITE_BISHOP_START;
    this->pieces[BLACK_BISHOP]  = BLACK_BISHOP_START;
    this->pieces[WHITE_QUEEN]   = WHITE_QUEEN_START;
    this->pieces[BLACK_QUEEN]   = BLACK_QUEEN_START;
    this->pieces[WHITE_KING]    = WHITE_KING_START;
    this->pieces[BLACK_KING]    = BLACK_KING_START;


}

uint64_t initializeChessBoard(void)
{
    cb = new ChessBoard();
    
    if(cb == NULL)
    {
        return 1;
    }
    return 0;

}
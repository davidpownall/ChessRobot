#include <iostream>
#include "util.h"
#include "chessboard.h"

static ChessBoard *cb;

/**
 * Default constructor for the ChessBoard class. Creates a fresh board
 * from scratch.
 */
ChessBoard::ChessBoard(void)
{
    uint32_t pt;

    this->pieces[WHITE_PAWN]    = WHITE_PAWN_START;
    this->pieces[WHITE_ROOK]    = WHITE_ROOK_START;
    this->pieces[WHITE_KNIGHT]  = WHITE_KNIGHT_START;
    this->pieces[WHITE_BISHOP]  = WHITE_BISHOP_START;
    this->pieces[WHITE_QUEEN]   = WHITE_QUEEN_START;
    this->pieces[WHITE_KING]    = WHITE_KING_START;

    this->pieces[BLACK_PAWN]    = BLACK_PAWN_START;
    this->pieces[BLACK_ROOK]    = BLACK_ROOK_START;
    this->pieces[BLACK_KNIGHT]  = BLACK_KNIGHT_START;
    this->pieces[BLACK_BISHOP]  = BLACK_BISHOP_START;
    this->pieces[BLACK_QUEEN]   = BLACK_QUEEN_START;
    this->pieces[BLACK_KING]    = BLACK_KING_START;

    for(pt = 0; pt < NUM_PIECE_TYPES; ++pt)
    {
        if(pt < (NUM_PIECE_TYPES >> 1))
        {
            this->pieces[WHITE_PIECES] |= this->pieces[pt];
        }
        else
        {
            this->pieces[BLACK_PIECES] |= this->pieces[pt];
        }
        this->occupied |= this->pieces[pt];
    }
    this->empty = ~(this->occupied);
}

/**
 * ChessBoard constructor which takes in an existing board state
 * 
 * @param *pieces:  The current board position of all pieces
 * @param occupued: The set of all occupied squares
 */
ChessBoard::ChessBoard(uint64_t *pieces, uint64_t occupied)
{

    // Verify that our array is valid
    if(pieces == NULL)
    {
        std::cout << "Error in piece array input during board construction"
             << std::endl;
        return;
    } 

    // Copy the board over
    memcpy((void *) this->pieces, (void*) pieces, (NUM_PIECE_TYPES + 2) >> 1);

    // @todo: Figure out a way to determine if an occupied bitboard is valid
    this->occupied = occupied;
    this->empty = ~occupied;

}

/**
 * Generates the valid moves for a given chessboard state and color
 * 
 * @param pt:   The color you wish to generate possible moves for
 */
moveType_t ChessBoard::generateMoves(pieceType_t pt)
{
    moveType_t moveStart;
    moveType_t lastMove;

    // Determine validity of input
    if(!(pt == WHITE_PIECES || pt == BLACK_PIECES))
    {
        std::cout << "Error in piece type input: " << pt << std::endl;
        return moveStart;
    }

    // Generate possible plays
    generatePawnMoves(pt, &lastMove);
    generateRookMoves(pt, &lastMove);
    generateBishopMoves(pt, &lastMove);
    generateKnightMoves(pt, &lastMove);
    generateQueenMoves(pt, &lastMove);
    generateKingMoves(pt, &lastMove);

    return moveStart;
}

void ChessBoard::generatePawnMoves(pieceType_t pt, moveType_t *lastMove)
{
    // @todo
}

void ChessBoard::generateRookMoves(pieceType_t pt, moveType_t *lastMove)
{
    // @todo
}

void ChessBoard::generateBishopMoves(pieceType_t pt, moveType_t *lastMove)
{
    // @todo
}

void ChessBoard::generateKnightMoves(pieceType_t pt, moveType_t *lastMove)
{
    // @todo
}

void ChessBoard::generateQueenMoves(pieceType_t pt, moveType_t *lastMove)
{
    // @todo
}

void ChessBoard::generateKingMoves(pieceType_t pt, moveType_t *lastMove)
{
    // @todo
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
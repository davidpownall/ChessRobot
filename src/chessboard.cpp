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

//  x x x x x x x x
//  x x x x x x x x
//  x x x x x x x x
//  x x x x x x x x
//  x x x x x x x x
//  x x x x x x x x
//  x x x x x x x x
//  x x x x x x x x
//


/**
 * My general breakdown of move functions would be something along the lines of 
 * 
 * 1) Figure out which squares the piece can move to
 * 2) Does that square contain a friendly piece? If yes, discard and go to next
 *    candidate
 * 3) Allocate and store potential move in moveList
 * 4) Go to next candidate
 **/

void ChessBoard::generatePawnMoves(pieceType_t pt, moveType_t *lastMove)
{
    // Pawns can move forward, or diagonally to strike, or en passant (tricky)
    bool countUp = (pt == WHITE_PAWN), forwardPossible;
    uint64_t startIdx = 0, pawn;
    int32_t forward, diagLeft, diagRight;
    uint64_t pawns = this->pieces[pt];
    uint64_t friendlyPieces;

    // This should never happen
    if(lastMove == NULL)
    {
        std::cout << "Movetype ptr passed in was NULL, this should never happen!" << std::endl;
        return;
    }

    // All your pawns are dead, don't bother
    if(pawns == 0)
    {
        return;
    }

    if(pt == WHITE_PAWN)
    {
        startIdx = __builtin_clz(this->pieces[pt]);

        for(uint64_t i = startIdx; i < 64; ++i)
        {
            pawn = (1 << i);
        
            // There are no pawns of our color at this location
            if(pawns & pawn == 0)
            {
                continue;
            }

            // So now we have the location of our first available pawn, we need to check moves

            // Move forward one square
            if( i < 56
                && (this->pieces[WHITE_PIECES] & (pawn << 8)) == 0
                && (this->pieces[BLACK_PIECES] & (pawn << 8)) == 0)
            {
                // Valid move on this square -- Add move
                
                if( i < 16 
                    && (this->pieces[WHITE_PIECES] & (pawn << 8)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn << 8)) == 0)
                {
                    // Valid move on this square -- Add move
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn << 7)) == 1)
            {
                // Valid attack move on this square -- Add move
            }

            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn << 9)) == 1)
            {
                // Valid attack move on this square -- Add move
            }

            // En-passent @todo
        }

    }
    else
    {
        startIdx = __builtin_ctz(this->pieces[pt]);

        for(uint64_t i = startIdx; i < 64; --i)
        {
            pawn = (1 << i);
        
            // There are no pawns of our color at this location
            if(pawns & pawn == 0)
            {
                continue;
            }

            // So now we have the location of our first available pawn, we need to check moves

            // Move forward one square
            if( i >= 8
                && (this->pieces[WHITE_PIECES] & (pawn >> 8)) == 0
                && (this->pieces[BLACK_PIECES] & (pawn >> 8)) == 0)
            {
                // Valid move on this square -- Add move
                
                if( i >= 56 
                    && (this->pieces[WHITE_PIECES] & (pawn >> 8)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn >> 8)) == 0)
                {
                    // Valid move on this square -- Add move
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn >> 9)) == 1)
            {
                // Valid attack move on this square -- Add move
            }

            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn >> 7)) == 1)
            {
                // Valid attack move on this square -- Add move
            }

            // En-passent @todo
        }
    }


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
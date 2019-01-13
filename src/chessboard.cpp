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

    this->numMovesAtThisDepth = MAX_EVAL_MOVES;
    this->movesToEvaluateAtThisDepth = (moveType_t *) malloc(this->numMovesAtThisDepth * sizeof(moveType_t));

    if(this->movesToEvaluateAtThisDepth == NULL)
    {
        std::cout << "Failed to allocate move memory" << std::endl;
        return;        
    }

}

/**
 * ChessBoard constructor which takes in an existing board state
 * 
 * @param *pieces:  The current board position of all pieces
 * @param occupued: The set of all occupied squares
 */
ChessBoard::ChessBoard(uint64_t *pieces, uint64_t occupied, uint64_t numMovesToEval)
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

    this->numMovesAtThisDepth = numMovesToEval;
    this->movesToEvaluateAtThisDepth = (moveType_t *) malloc(this->numMovesAtThisDepth * sizeof(moveType_t));

    if(this->movesToEvaluateAtThisDepth == NULL)
    {
        std::cout << "Failed to allocate move memory" << std::endl;
        return;        
    }

}

/**
 * Generates the valid moves for a given chessboard state and color
 * 
 * @param pt:   The color you wish to generate possible moves for
 */
void ChessBoard::generateMoves(moveType_t *moveStart, pieceType_t pt)
{
    moveType_t lastMove;

    // Determine validity of input
    if(!(pt == WHITE_PIECES || pt == BLACK_PIECES))
    {
        std::cout << "Error in piece type input: " << pt << std::endl;
        return;
    }

    // Generate possible plays
    generatePawnMoves(pt, &lastMove);
    generateRookMoves(pt, &lastMove);
    generateBishopMoves(pt, &lastMove);
    generateKnightMoves(pt, &lastMove);
    generateQueenMoves(pt, &lastMove);
    generateKingMoves(pt, &lastMove);

}

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
    uint64_t pawn;
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

        for(uint64_t i = __builtin_clz(this->pieces[pt]); i < 64; ++i)
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
        for(uint64_t i = __builtin_ctz(this->pieces[pt]); i < 64; --i)
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
    // Rooks can move vertically and horizontally. Logic is mostly unified between color

    pieceType_t enemyPieces;
    uint64_t rooks = this->pieces[pt], rook, temp, rookIdx;
    (pt < 6) ? enemyPieces = BLACK_PIECES : enemyPieces = WHITE_PIECES;

    // No rooks left
    if(rooks == 0)
    {
        return;
    }

    // This should never happen
    if(lastMove == NULL)
    {
        std::cout << "Movetype ptr passed in was NULL, this should never happen!" << std::endl;
        return;
    }

    // Avenues of attack are up shift 8, down shift 8, right shift 1, left shift 1
    // All of these are until edge

    while(rooks > 0)
    {

        // Get and clear index
        rookIdx = __builtin_clz(rooks);
        rook = (1 << rookIdx);
        rooks ^= rook;

        temp = rook;
        // left
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx % 8 == 0)
            {
                break;
            }

            temp = temp >> 1;
            if((temp & this->occupied) == 0)
            {
                // Valid movement
            }
            else if((temp & this->pieces[enemyPieces]) != 0)
            {
                // Valid attack move
                break;
            }

        } while(temp % 8 > 0);

        // right
        temp = rook;
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx % 8 == 7)
            {
                break;
            }

            temp = temp << 1;
            if((temp & this->occupied) == 0)
            {
                // Valid movement
            }
            else if((temp & this->pieces[enemyPieces]) != 0)
            {
                // Valid attack move
                break;
            }

        } while(temp % 8 < 7);

        // down
        temp = rook;
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx < 8)
            {
                break;
            }

            temp = temp >> 8;
            if((temp & this->occupied) == 0)
            {
                // Valid movement
            }
            else if((temp & this->pieces[enemyPieces]) != 0)
            {
                // Valid attack move
                break;
            }

        } while(temp >= 8);

        // up
        temp = rook;
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx > 54)
            {
                break;
            }

            temp = temp << 8;
            if((temp & this->occupied) == 0)
            {
                // Valid movement
            }
            else if((temp & this->pieces[enemyPieces]) != 0)
            {
                // Valid attack move
                break;
            }

        } while(temp <= 54);
    }

}

void ChessBoard::generateBishopMoves(pieceType_t pt, moveType_t *lastMove)
{
    pieceType_t enemyPieces;
    uint64_t bishops = this->pieces[pt], bishopIdx, bishop, temp;
    (pt < 6) ? enemyPieces = BLACK_PIECES : enemyPieces = WHITE_PIECES;

    // No bishops left
    if(bishops == 0)
    {
        return;
    }

    // This should never happen
    if(lastMove == NULL)
    {
        std::cout << "Movetype ptr passed in was NULL, this should never happen!" << std::endl;
        return;
    }

    while(bishops > 0)
    {
        bishopIdx = __builtin_clz(bishops);
        bishop = (1 << bishopIdx);
        bishops ^= bishop;

        // Four directions of movement

        // Down left
        temp = bishop;
        do
        {
            if(temp % 8 == 0 || __builtin_clz(temp) < 8)
            {
                break;
            }

            temp = temp >> 9;
            if((temp & this->occupied) == 0)
            {
                // Valid movement
            }
            if((temp & this->pieces[enemyPieces]) != 0)
            {
                // Valid attack move
                break;
            }

        } while(temp > 0);

        // Down right
        temp = bishop;
        do
        {
            if(temp % 8 == 7 || __builtin_clz(temp) < 8)
            {
                break;
            }

            temp = temp >> 7;
            if((temp & this->occupied) == 0)
            {
                // Valid movement
            }
            if((temp & this->pieces[enemyPieces]) != 0)
            {
                // Valid attack move
                break;
            }

        } while(temp > 0);

        // Up left
        temp = bishop;
        do
        {
            if(temp % 8 == 0 || __builtin_clz(temp) >= 54)
            {
                break;
            }

            temp = temp << 7;
            if((temp & this->occupied) == 0)
            {
                // Valid movement
            }
            if((temp & this->pieces[enemyPieces]) != 0)
            {
                // Valid attack move
                break;
            }

        } while(temp > 0);

        // Up right
        temp = bishop;
        do
        {
            if(temp % 8 == 7 || __builtin_clz(temp) >= 54)
            {
                break;
            }

            temp = temp << 9;
            if((temp & this->occupied) == 0)
            {
                // Valid movement
            }
            if((temp & this->pieces[enemyPieces]) != 0)
            {
                // Valid attack move
                break;
            }

        } while(temp > 0);

    }
}

void ChessBoard::generateKnightMoves(pieceType_t pt, moveType_t *lastMove)
{
    // @todo
}

void ChessBoard::generateQueenMoves(pieceType_t pt, moveType_t *lastMove)
{
    // The queen can make any move that a rook or bishop can
    this->generateBishopMoves(pt, NULL);
    this->generateRookMoves(pt, NULL);
}

void ChessBoard::generateKingMoves(pieceType_t pt, moveType_t *lastMove)
{
    // While the king has basic movement, it cannot put itself into check,
    // we need an additional guard in place for that. Also castling behavior.

    pieceType_t enemyPieces;
    uint64_t king = this->pieces[pt], temp;
    (pt < 6) ? enemyPieces = BLACK_PIECES : enemyPieces = WHITE_PIECES;

    // Generate moves

    // Iterate through coverage list to check if that square is under threat

}

/**
 * Algorithm for selecting moves:
 * 
 *  We know: 
 *      -   the the current state of our chessboard after move i-1 C(i)
 *      -   the current value of our chessboard after move i-1 V(i)
 *      -   the possible moves for our position after move i-1 M(i)
 *              - each move denoted by m_j s.t. m_j is in M(i)
 * 
 *  We know that we have a list of possible moves available to use
 *      -   We at this point have made no determination about whether
 *          or not that move is any good
 *      -   We now want to evaluate all of our moves in M(i)
 * 
 *  For a basic evaluation of a given move, we need to consider what
 *  our board value will be at the start of our next move, or after our
 *  opponent has moved. This can be represented as V(i+2):
 * 
 *      Value(m_j) = V(i+2) - V(i) 
 * 
 *  For a general search depth of n moves into the future, this is:
 * 
 *      Value(m_j) = V(i+n) - V(i)
 * 
 *  Where generally:
 *  
 *      V(i) = V(i-1) + Value(m_prev)
 * 
 *  Which leads to the selection of our "best" move:
 * 
 *  m_next  = MAX_(m_j in M(i)) { Value(m_j) }
 *          = MAX_(m_j in M(i)) { V(i+n) - V(i) }
 * 
 *  Bringing in the recurrence:
 * 
 *  m_next = MAX_(m_j in M(i)) { V(i+n-1) + Value(m_(n-1)) - V(i) }
 *  m_next = MAX_(m_j in M(i)) { V(i+n-2) + Value(m_(n-2)) + Value(m_(n-1)) - V(i) }
 *  ...
 *  m_next = MAX_(m_j in M(i)) { SUM_{k=j}{j+n-1}( Value(m_(k)) ) }
 * 
 *  But value will be negative if our opponent is moving:
 * 
 *      Let X(i) be an objective function s.t. X(k) = 1     iff k-j % 2 == 0
 *                                                  = -1    otherwise
 * 
 *  m_next = MAX_(m_j in M(i)) { SUM_{k=j}{j+n-1}( Value(m_(k))X(k) ) }
 * 
 *  Obviously the move space will be exponential, so we want to
 *  prune this search tree as we go through it. 
 * 
 *  Let's ballpark this to 5 move future search to start:
 * 
 *      |M(i)|   = ||
 *      |M(i+1)| = 30
 *      |M(i+2)| = 20
 *      |M(i+3)| = 10
 *      |M(i+4)| = 5
 * 
 *      That is 1.2 million possible moves (lol)
 */


moveType_t *ChessBoard::getNextMove(pieceType_t pt)
{
    // Sort moves in descending order by value of the move
    
    // For the number of moves we are supposed to evaluate at this depth,
    // sort by current value

    // Update final value with result of search

    return NULL;
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
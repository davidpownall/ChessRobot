/* This file is responsible for the generation of chess moves given a board state */

#include <iostream>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"
#include "threatmap.h"

/**
 * Generates the valid moves for a given chessboard state and color
 * 
 * @param pt:   The color you wish to generate possible moves for
 *
 * @return      The list of moves available at this location
 */
moveType_t *ChessBoard::GenerateMoves(uint8_t pt)
{
    uint8_t nextPt;
    uint64_t i = 1;
    moveType_t *moveList = new moveType_t;

    // Use a known invalid move as a end marker
    Util_Assert(moveList != NULL, "Failed to allocate memory for a move");
    moveList->legalMove = false;

    if(pt == WHITE_PIECES)
    {
        nextPt = BLACK_PIECES;

        // Generate possible plays for white
        GeneratePawnMoves(WHITE_PAWN, &moveList);
        GenerateRookMoves(WHITE_ROOK, &moveList);
        GenerateBishopMoves(WHITE_BISHOP, &moveList);
        GenerateKnightMoves(WHITE_KNIGHT, &moveList);
        GenerateQueenMoves(WHITE_QUEEN, &moveList);
        GenerateKingMoves(WHITE_KING, &moveList);
    }
    else if (pt == BLACK_PIECES)
    {
        nextPt = WHITE_PIECES;

        // Generate possible plays for black
        GeneratePawnMoves(BLACK_PAWN, &moveList);
        GenerateRookMoves(BLACK_ROOK, &moveList);
        GenerateBishopMoves(BLACK_BISHOP, &moveList);
        GenerateKnightMoves(BLACK_KNIGHT, &moveList);
        GenerateQueenMoves(BLACK_QUEEN, &moveList);
        GenerateKingMoves(BLACK_KING, &moveList);
    }
    else
    {
        Util_Assert(0, "Error in input piece type");
    }

    return moveList;
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

void ChessBoard::GeneratePawnMoves(uint8_t pt, moveType_t **moveList)
{
    // Pawns can move forward, or diagonally to strike, or en passant (tricky)
    uint64_t i, pawn, pawns = this->pieces[pt];   
    moveType_t *lastMove;

    // All your pawns are dead, don't bother
    if(pawns == 0)
    {
        return;
    }

    Util_Assert(pt == WHITE_PAWN || pt == BLACK_PAWN, "Pawn move passed bad piecetype");

    //lastMove = GetLastMove();

    if(pt == WHITE_PAWN)
    {
        while(pawns != 0)
        {
            i = __builtin_ctzll(pawns);
            pawn = ((uint64_t) 1 << i);
            pawns ^= pawn;

            // So now we have the location of our first available pawn, we need to check moves

            // Move forward one square
            if( i < 56
                && (this->pieces[WHITE_PIECES] & (pawn << 8)) == 0
                && (this->pieces[BLACK_PIECES] & (pawn << 8)) == 0)
            {
                this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 8, MOVE_VALID, moveList);
                
                if( i < 16 
                    && (this->pieces[WHITE_PIECES] & (pawn << 16)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn << 16)) == 0)
                {
                    this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 16, MOVE_VALID, moveList);
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn << 7)) != 0)
            {
                this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 7, MOVE_VALID_ATTACK, moveList);
            }

            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn << 9)) != 0)
            {
                this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 9, MOVE_VALID_ATTACK, moveList), moveList;
            }

            // En passant check

            // Check if this is the first move
            if(lastMove == NULL)
            {
                continue;
            }

            // Was the last move a black pawn pushing up by two
            if( (lastMove != NULL)
                && (lastMove->pt == BLACK_PAWN )
                && (lastMove->endIdx == lastMove->startIdx - 16))
            {
                // Can we look to the left and did the pawn land there
                if((i % 8 > 0) && ((i - lastMove->endIdx) == 1))
                {
                    this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 7, MOVE_VALID_ATTACK, moveList);
                }
                // Can we look to the right and did the pawn land there
                if((i % 8 < 7) && ((lastMove->endIdx - i) == 1))
                {
                    this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 9, MOVE_VALID_ATTACK, moveList);
                }          
            }        
        }
    }
    else
    {
        while(pawns != 0)
        {
            i = __builtin_ctzll(pawns);
            pawn = ((uint64_t) 1 << i);
            pawns ^= pawn;
        
            // Move forward one square
            if( i >= 8
                && ((this->occupied & (pawn >> 8)) == 0))
            {
                this->BuildMove(BLACK_PAWN, i, i - 8, MOVE_VALID, moveList);
                
                if( i >= 48 
                    && (this->pieces[WHITE_PIECES] & (pawn >> 16)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn >> 16)) == 0)
                {
                    this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 16, MOVE_VALID, moveList);
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 7) && ((this->pieces[WHITE_PIECES] & (pawn >> 9)) != 0))
            {
                this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 9, MOVE_VALID_ATTACK, moveList);
            }

            if( (i % 8 != 0) && ((this->pieces[WHITE_PIECES] & (pawn >> 7)) != 0))
            {
                this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 7, MOVE_VALID_ATTACK, moveList);
            }

            // En passant check

            // Check if this is the first move
            if(lastMove == NULL)
            {
                continue;
            }

            // Was the last move a white pawn pushing up by two
            if( (lastMove != NULL)
                && (lastMove->pt == WHITE_PAWN )
                && (lastMove->endIdx == lastMove->startIdx + 16))
            {
                // Can we look to the left and did the pawn land there
                if((i % 8 > 0) && ((i - lastMove->endIdx) == 1))
                {
                    this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 9, MOVE_VALID_ATTACK, moveList);
                }
                // Can we look to the right and did the pawn land there
                if((i % 8 < 7) && ((lastMove->endIdx - i) == 1))
                {
                    this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 7, MOVE_VALID_ATTACK, moveList);
                }          
            }
        }
    }   
}

void ChessBoard::GenerateRookMoves(uint8_t pt, moveType_t **moveList)
{
    // Rooks can move vertically and horizontally. Logic is mostly unified between color
    uint8_t moveVal;
    uint8_t friendlyPieces, enemyPieces;
    uint64_t rooks = this->pieces[pt], temp, rookIdx;

    Util_AssignFriendAndFoe(pt, &friendlyPieces, &enemyPieces);

    // No rooks left
    if(rooks == 0)
    {
        return;
    }

    // Avenues of attack are up shift 8, down shift 8, right shift 1, left shift 1
    // All of these are until edge

    while(rooks > 0)
    {

        // Get and clear index
        rookIdx = __builtin_ctzll(rooks);
        rooks ^= ((uint64_t) 1 << rookIdx);

        temp = rookIdx;
        // left
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx % 8 == 0)
            {
                break;
            }

            --temp;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, friendlyPieces, enemyPieces);
            
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);

            // Cannot move/attack through other pieces
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp % 8 > 0);

        // right
        temp = rookIdx;
        do
        {
            // on the right side of the board, can't go that way
            if(rookIdx % 8 == 7)
            {
                break;
            }

            ++temp;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, friendlyPieces, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);

            // Cannot move/attack through other pieces
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp % 8 < 7);

        // down
        temp = rookIdx;
        do
        {
            // Cannot go down
            if(rookIdx < 8)
            {
                break;
            }

            temp -= 8;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, friendlyPieces, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);

            // Cannot move/attack through other pieces
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp >= 8);

        // up
        temp = rookIdx;
        do
        {
            // Cannot go up
            if(rookIdx > 54)
            {
                break;
            }

            temp += 8;
            moveVal = this->CheckSpaceForMoveOrAttack(temp,  friendlyPieces, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);
        
            // Cannot move/attack through other pieces
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp <= 54);
    }
}

void ChessBoard::GenerateBishopMoves(uint8_t pt, moveType_t **moveList)
{
    uint8_t moveVal;
    uint8_t friendlyPieces, enemyPieces;
    uint64_t bishops = this->pieces[pt], bishopIdx, bishop, temp;

    Util_AssignFriendAndFoe(pt, &friendlyPieces, &enemyPieces);

    // No bishops left
    if(bishops == 0)
    {
        return;
    }

    while(bishops > 0)
    {
        bishopIdx = __builtin_ctzll(bishops);
        bishops ^= ((uint64_t) 1 << bishopIdx);

        // Four directions of movement

        // Down left
        temp = bishopIdx;
        do
        {
            // Either left column or bottom row
            if(temp % 8 == 0 || temp < 8)
            {
                break;
            }

            temp -= 9;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, friendlyPieces, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, bishopIdx, temp, moveVal, moveList);
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp > 0);

        // Down right
        temp = bishopIdx;
        do
        {
            // Either right column or bottom row
            if(temp % 8 == 7 || temp < 8)
            {
                break;
            }

            temp -= 7;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, friendlyPieces, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, bishopIdx, temp, moveVal, moveList);
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp > 0);

        // Up left
        temp = bishopIdx;
        do
        {
            // Either left column or top row
            if(temp % 8 == 0 || temp > 55)
            {
                break;
            }

            temp += 7;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, friendlyPieces, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, bishopIdx, temp, moveVal, moveList);
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp < NUM_BOARD_INDICES);

        // Up right
        temp = bishopIdx;
        do
        {
            if(temp % 8 == 7 || temp > 55)
            {
                break;
            }

            temp += 9;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, friendlyPieces, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, bishopIdx, temp, moveVal, moveList);
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp < NUM_BOARD_INDICES);
    }
}

/**
 * Generates all available knight moves for a given piece type
 *  
 * @param pt:   The pieceType to generate the knight moves for
 * 
 * @requires    pt to be in {WHITE_KNIGHT, BLACK_KNIGHT}
 */
void ChessBoard::GenerateKnightMoves(uint8_t pt, moveType_t **moveList)
{
    // Knights can move two squares horizontally and one vertically, or
    // two squares vertically and one horizontally.

    uint8_t moveVal;
    uint8_t friendlyPieces, enemyPieces;
    uint64_t knights = this->pieces[pt], knightIdx, temp;

    Util_AssignFriendAndFoe(pt, &friendlyPieces, &enemyPieces);

    // No knights left
    if(knights == 0)
    {
        return;
    }

    // The only blocks for moves are from friendly pieces
    while(knights > 0)
    {
        knightIdx = __builtin_ctzll(knights);
        knights ^= ((uint64_t) 1 << knightIdx);

        // Assess upwards vertical moves
        if(knightIdx < NUM_BOARD_INDICES - 16)
        {
            // Can we move left
            if(knightIdx % 8 > 0)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 15, friendlyPieces, enemyPieces);
                if(moveVal != MOVE_INVALID)
                {
                    this->BuildMove(pt, knightIdx, knightIdx + 15, moveVal, moveList);
                }
                
            }
            // Can we move right
            if(knightIdx % 8 < 7)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 17, friendlyPieces, enemyPieces);
                if(moveVal != MOVE_INVALID)
                {
                    this->BuildMove(pt, knightIdx, knightIdx + 17, moveVal, moveList);
                }
            }
        }

        // Assess downwards vertical moves
        if(knightIdx >= 16)
        {
            // Can we move left
            if(knightIdx % 8 > 0)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 17, friendlyPieces, enemyPieces);

                if(moveVal != MOVE_INVALID)
                {
                    this->BuildMove(pt, knightIdx, knightIdx - 17, moveVal, moveList);
                }
            }
            // Can we move right
            if(knightIdx % 8 < 7)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 15, friendlyPieces, enemyPieces);
                if(moveVal != MOVE_INVALID)
                {
                    this->BuildMove(pt, knightIdx, knightIdx - 15, moveVal, moveList);
                }
            }            
        }

        // Assess left horizontal moves
        if(knightIdx % 8 > 1)
        {
            // Can we move up
            if(knightIdx < NUM_BOARD_INDICES - 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 6, friendlyPieces, enemyPieces);
                if(moveVal != MOVE_INVALID)
                {
                    this->BuildMove(pt, knightIdx, knightIdx + 6, moveVal, moveList);
                }
            }
            // Can we move down
            if(knightIdx >= 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 10, friendlyPieces, enemyPieces);
                if(moveVal != MOVE_INVALID)
                {
                    this->BuildMove(pt, knightIdx, knightIdx - 10, moveVal, moveList);
                }
            }
        }

        // Assess right horizontal moves
        if(knightIdx % 8 < 6)
        {
            // Can we move up
            if(knightIdx < NUM_BOARD_INDICES - 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 10, friendlyPieces, enemyPieces);
                if(moveVal != MOVE_INVALID)
                {
                    this->BuildMove(pt, knightIdx, knightIdx + 10, moveVal, moveList);
                }
            }
            // Can we move down
            if(knightIdx >= 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 6, friendlyPieces, enemyPieces);
                if(moveVal != MOVE_INVALID)
                {
                    this->BuildMove(pt, knightIdx, knightIdx - 6, moveVal, moveList);           
                }
            }
        }
    }
}

/**
 * Generates all possible moves for our queen(s)
 * 
 * @param pt:       The type of queen to generate the move for
 * @param moveList: The list of moves to append ours to
 */
void ChessBoard::GenerateQueenMoves(uint8_t pt, moveType_t **moveList)
{
    Util_Assert(pt == WHITE_QUEEN || pt == BLACK_QUEEN, "Queen type provided invalid");

    // The queen can make any move that a rook or bishop can, will actually
    // capture all possible moves if we have multiple queens
    this->GenerateBishopMoves(pt, moveList);
    this->GenerateRookMoves(pt, moveList);
}

/**
 * Generates all possible moves for our king
 * 
 * @param pt:       The type of king to generate the move for
 * @param moveList: The list of moves to append ours to
 */
void ChessBoard::GenerateKingMoves(uint8_t pt, moveType_t **moveList)
{
    // While the king has basic movement, it cannot put itself into check,
    // we need an additional guard in place for that. Also castling behavior.

    uint8_t friendlyPieces, enemyPieces, moveVal;
    uint64_t king = this->pieces[pt], shift = 1;
    uint64_t kingIdx = __builtin_ctzll(king);

    Util_AssignFriendAndFoe(pt, &friendlyPieces, &enemyPieces);

    /**
     * Logic as follows:
     * 
     * 1) Can we move in that direction
     * 2) Are we blocked by a friendly
     * 3) Would be putting ourselves into check if we did that.
     *    --> Note that the logic here does not take into account the fact
     *        that we might be in check when we are making this move. That is 
     *        handled when actually assessing which moves we want to take
     */

    // left
    if((kingIdx % 8 != 0 )
        && ((this->pieces[friendlyPieces] & (shift << (kingIdx - 1))) == 0)
        && (ThreatMap_IsIndexUnderThreat(kingIdx - 1, friendlyPieces == BLACK_PIECES) == false))
    {
        moveVal = this->CheckSpaceForMoveOrAttack(kingIdx - 1, friendlyPieces, enemyPieces);
        if(moveVal != MOVE_INVALID)
        {
            this->BuildMove(pt, kingIdx, kingIdx - 1, moveVal, moveList);           
        }
    }

    // right
    if((kingIdx % 8 != 7 )
        && ((this->pieces[friendlyPieces] & (shift << (kingIdx + 1))) == 0)
        && (ThreatMap_IsIndexUnderThreat(kingIdx + 1, friendlyPieces == BLACK_PIECES) == false))
    {
        moveVal = this->CheckSpaceForMoveOrAttack(kingIdx + 1, friendlyPieces, enemyPieces);
        if(moveVal != MOVE_INVALID)
        {
            this->BuildMove(pt, kingIdx, kingIdx + 1, moveVal, moveList);           
        }
    }

    // up
    if((kingIdx < NUM_BOARD_INDICES - 8 )
        && ((this->pieces[friendlyPieces] & (shift << (kingIdx + 8))) == 0)
        && (ThreatMap_IsIndexUnderThreat(kingIdx + 8, friendlyPieces == BLACK_PIECES) == false))
    {
        moveVal = this->CheckSpaceForMoveOrAttack(kingIdx + 8, friendlyPieces, enemyPieces);
        if(moveVal != MOVE_INVALID)
        {
            this->BuildMove(pt, kingIdx, kingIdx + 8, moveVal, moveList);           
        }
    }

    // down
    if((kingIdx >= 8 )
        && ((this->pieces[friendlyPieces] & (shift << (kingIdx - 8))) == 0)
        && (ThreatMap_IsIndexUnderThreat(kingIdx - 8, friendlyPieces == BLACK_PIECES) == false))
    {
        moveVal = this->CheckSpaceForMoveOrAttack(kingIdx - 8, friendlyPieces, enemyPieces);
        if(moveVal != MOVE_INVALID)
        {
            this->BuildMove(pt, kingIdx, kingIdx - 8, moveVal, moveList);           
        }
    }

    // down left
    if((kingIdx % 8 != 0 && kingIdx >= 8 )
        && ((this->pieces[friendlyPieces] & (shift << (kingIdx - 9))) == 0)
        && (ThreatMap_IsIndexUnderThreat(kingIdx - 9, friendlyPieces == BLACK_PIECES) == false))
    {
        moveVal = this->CheckSpaceForMoveOrAttack(kingIdx - 9, friendlyPieces, enemyPieces);
        if(moveVal != MOVE_INVALID)
        {
            this->BuildMove(pt, kingIdx, kingIdx - 9, moveVal, moveList);           
        }
    }

    // down right
    if((kingIdx % 8 != 7 && kingIdx >= 8 )
        && ((this->pieces[friendlyPieces] & (shift << (kingIdx - 7))) == 0)
        && (ThreatMap_IsIndexUnderThreat(kingIdx - 7, friendlyPieces == BLACK_PIECES) == false))
    {
        moveVal = this->CheckSpaceForMoveOrAttack(kingIdx - 7, friendlyPieces, enemyPieces);
        if(moveVal != MOVE_INVALID)
        {
            this->BuildMove(pt, kingIdx, kingIdx - 7, moveVal, moveList);           
        }
    }

    // up left
    if((kingIdx % 8 != 0 && kingIdx < NUM_BOARD_INDICES - 8 )
        && ((this->pieces[friendlyPieces] & (shift << (kingIdx + 7))) == 0)
        && (ThreatMap_IsIndexUnderThreat(kingIdx + 7, friendlyPieces == BLACK_PIECES) == false))
    {
        moveVal = this->CheckSpaceForMoveOrAttack(kingIdx + 7, friendlyPieces, enemyPieces);
        if(moveVal != MOVE_INVALID)
        {
            this->BuildMove(pt, kingIdx, kingIdx + 7, moveVal, moveList);           
        }
    }

    // up right
    if((kingIdx % 8 != 7 && kingIdx < NUM_BOARD_INDICES - 8 )
        && ((this->pieces[friendlyPieces] & (shift << (kingIdx + 9))) == 0)
        && (ThreatMap_IsIndexUnderThreat(kingIdx + 9, friendlyPieces == BLACK_PIECES) == false))
    {
        moveVal = this->CheckSpaceForMoveOrAttack(kingIdx + 9, friendlyPieces, enemyPieces);
        if(moveVal != MOVE_INVALID)
        {
            this->BuildMove(pt, kingIdx, kingIdx + 9, moveVal, moveList);           
        }
    }


}
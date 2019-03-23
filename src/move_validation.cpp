
/* This file is responsible for the validation of chess moves */

#include <iostream>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"

/**
 * Determines if the piece type at this location can make a valid move to the endIdx
 * 
 * @param pt            The piece type making the move
 * @param idxToAssess   The index of the piece being checked
 * @param endIdx        The index where the piece moved to
 * 
 * @return              True if the piece type at that location can actually make the move
 */
bool ChessBoard::IsValidMove(uint8_t pt, uint8_t idxToAssess, uint8_t endIdx)
{
    int8_t temp;
    bool status;

    switch(pt % (NUM_PIECE_TYPES/2))
    {
        // Pawn
        case 0:
            std::cout << "Pawn logic not implemented for IsValidMove!" << std::endl;
            status = false;
            break;
        // Rook
        case 1:
            status = ChessBoard::IsValidRookMove(this, idxToAssess, endIdx);
            break;
        // Knight
        case 2:
            status = ChessBoard::IsValidKnightMove(idxToAssess, endIdx);
            break;
        // Bishop
        case 3:
            status = ChessBoard::IsValidBishopMove(this, idxToAssess, endIdx);
            break;
        // Queen
        case 4:
            status = ChessBoard::IsValidRookMove(this, idxToAssess, endIdx)
                        || ChessBoard::IsValidBishopMove(this, idxToAssess, endIdx);
            break;
        // King
        case 5:
            // Assuming a single king (which is always true)
            status = true;
        default:
            return false;
    }

    return true;
}

/**
 * Determine if the rook at idxToAssess can actually go to endIdx
 * 
 * @param idxToAssess:  The index of a given rook
 * @param endIdx:       Where that rook would have to end up
 * 
 * @return              True if the rook could make that move
 */
bool ChessBoard::IsValidRookMove(ChessBoard *cb, uint8_t idxToAssess, uint8_t endIdx)
{
    uint8_t mask, largerIdx, smallerIdx;

    if(cb == NULL)
    {
        return false;
    }

    if(endIdx > idxToAssess)
    {
        largerIdx = endIdx;
        smallerIdx = idxToAssess;
    }
    else
    {
        largerIdx = idxToAssess;
        smallerIdx = endIdx;    
    }

    // Are we on the same rank or file AND are the indicies between these two
    // locations empty            
    if(endIdx % 8 == idxToAssess % 8) // same column
    {
        // Now we have the column values for our occupied squares
        mask = cb->occupied & (COLUMN_MASK << endIdx % 8);
        mask &= ((COLUMN_MASK << endIdx % 8) >> 8*(8 - largerIdx/8));
        mask &= ~((COLUMN_MASK << endIdx % 8) >> 8*(8 - smallerIdx/8 + 1));

        // if our mask == 0 then the move is valid
        return (mask == 0);
    }
    else if(endIdx >> 3 == idxToAssess >> 3)
    {
        // Now we have the row masked
        mask = cb->occupied & (0xFF << (endIdx/8));
        mask &= ~(BOARD_MASK << largerIdx);
        mask &= ~(BOARD_MASK >> (63 - smallerIdx));

        return (mask == 0);
    }

    return false;
}

/**
 * Determine if the knight at idxToAssess can actually go to endIdx
 * 
 * @param idxToAssess:  The index of a given knight
 * @param endIdx:       Where that knight would have to end up
 * 
 * @return              True if the knight could make that move
 */
bool ChessBoard::IsValidKnightMove(uint8_t idxToAssess, uint8_t endIdx)
{
    if(endIdx > idxToAssess)
    {
        // Assess upwards vertical moves
        if(idxToAssess < NUM_BOARD_INDICES - 16)
        {
            // Can we up then left or right
            if((idxToAssess + 15 % 8 < idxToAssess % 8 && idxToAssess + 15 == endIdx)
                || (idxToAssess + 17 % 8 > idxToAssess % 8 && idxToAssess + 17 == endIdx))
            {
                return true;
            }
        }
        // Assess horizontal upwards moves
        if(idxToAssess < NUM_BOARD_INDICES - 8)
        {
            // Can we move left then up or right then up
            if((idxToAssess % 8 >= 2 && idxToAssess + 6 < NUM_BOARD_INDICES && idxToAssess + 6 == endIdx)
                || (idxToAssess % 8 < 6 && idxToAssess + 10 < NUM_BOARD_INDICES && idxToAssess + 10 == endIdx))
            {
                return true;
            }
        }
    }
    else if(idxToAssess > endIdx)
    {
        // Assess downwards vertical moves
        if(idxToAssess >= 16)
        {
            // Can we down then left or right
            if((idxToAssess - 15 % 8 < idxToAssess % 8 && idxToAssess - 15 == endIdx)
                || (idxToAssess - 17 % 8 > idxToAssess % 8 && idxToAssess - 17 == endIdx))
            {
                return true;
            }
        }
        // Assess horizontal downwards moves
        if(idxToAssess >= 8)
        {
            // Can we move left then down or right then down
            if((idxToAssess % 8 >= 2 && idxToAssess - 6 < NUM_BOARD_INDICES && idxToAssess - 6 == endIdx)
                || (idxToAssess % 8 < 6 && idxToAssess - 10 < NUM_BOARD_INDICES && idxToAssess - 10 == endIdx))
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * Determine if the bishop at idxToAssess can actually go to endIdx
 * 
 * @param idxToAssess:  The index of a given bishop
 * @param endIdx:       Where that bishop would have to end up
 * 
 * @return              True if the bishop could make that move
 */
bool ChessBoard::IsValidBishopMove(ChessBoard *cb, uint8_t idxToAssess, uint8_t endIdx)
{
    int8_t temp;
    uint8_t largerIdx, smallerIdx;

    if(cb == NULL)
    {
        return false;
    }

    if(endIdx > idxToAssess)
    {
        largerIdx = endIdx;
        smallerIdx = idxToAssess;
    }
    else
    {
        largerIdx = idxToAssess;
        smallerIdx = endIdx;    
    }

    // Are the start and end indicies on the same diagonal
    if(largerIdx - smallerIdx % 7 == 0)
    {
        temp = 7;
    }
    else if(largerIdx - smallerIdx & 9 == 0)
    {
        temp = 9;
    }
    else
    {
        return false;
    }
    smallerIdx += temp;
    // This will traverse the diagonal and search for impediments
    while(smallerIdx != largerIdx)
    {
        // There is something in the way
        if(((uint64_t) 1 << smallerIdx) & cb->occupied)
        {
            return false;
        }
        smallerIdx += temp;
    }
    return true;
}
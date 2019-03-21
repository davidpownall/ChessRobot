/* This file is responsible for the application of moves to the chessboard */

#include <iostream>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"

/**
 * Applies the current move to the chessboard
 * 
 * @param *moveToApply: The move to apply
 * 
 * @returns: STATUS_SUCESS or STATUS_FAIL
 */
uint64_t ChessBoard::ApplyMoveToBoard(moveType_t *moveToApply)
{

    uint8_t friendlyPieces, enemyPieces, friendlyStart, enemyStart;

    if(moveToApply == NULL)
    {
        std::cout << "Move provided to board was NULL!" << std::endl;
        return STATUS_FAIL;
    }

    // We know that any chess move passed to us will have to be
    // generated to comform to the rules of chess, therefore minimal
    // checking should be required...but this is C++ and literally
    // anything can happen with memory overruns so we may as well
    // check for the obvious stuff.

    // 1) Are our indicies valid?
    if(moveToApply->startIdx >= NUM_BOARD_INDICIES
        || moveToApply->endIdx >= NUM_BOARD_INDICIES)
    {
        std::cout << "Move indicies were bad!" << std::endl;
        return STATUS_FAIL;        
    }

    // 2) Is our piecetype actually valid?
    if(moveToApply->pt >= NUM_PIECE_TYPES)
    {
        std::cout << "Piece type provided to board was bad" << std::endl;
        return STATUS_FAIL;
    }

    // 3) Is there actually a piece of this piece type actually at the
    //    start index?
    if(this->pieces[moveToApply->pt] & ((uint64_t) 1 << moveToApply->startIdx) == 0)
    {
        std::cout << "No piece of piecetype at expexted startIdx" << std::endl;
        return STATUS_FAIL;        
    }

    Util_AssignFriendAndFoe(moveToApply->pt, &friendlyPieces, &enemyPieces);

    if(moveToApply->pt < (NUM_PIECE_TYPES/2))
    {
        friendlyStart = WHITE_PAWN;
        enemyStart = BLACK_PAWN;
    }
    else
    {
        friendlyStart = BLACK_PAWN;
        enemyStart = WHITE_PAWN;
    }

    // 4) Is our end index occupied by our color
    Util_Assert((this->pieces[friendlyPieces] & ((uint64_t) 1 << moveToApply->endIdx)) == 0,
        "There was a friendly piece where we wanted to move!");

    Util_Assert((this->pieces[friendlyPieces] ^ this->pieces[enemyPieces]) == this->occupied,
        "Incoherence between piece states and state of actual board");

    // Apply the move for our piece type
    this->pieces[moveToApply->pt] ^= ((uint64_t) 1 << moveToApply->startIdx);
    this->pieces[moveToApply->pt] |= ((uint64_t) 1 << moveToApply->endIdx);

    // Apply the move for our color
    this->pieces[friendlyPieces] ^= ((uint64_t) 1 << moveToApply->startIdx);
    this->pieces[friendlyPieces] |= ((uint64_t) 1 << moveToApply->endIdx);

    if((moveToApply->moveVal & MOVE_VALID_ATTACK) != 0)
    {
        // If we are taking a piece, clear that square
        this->pieces[enemyPieces] &= ~((uint64_t) 1 << moveToApply->endIdx);
        for(uint8_t i = enemyStart; i < enemyStart + NUM_PIECE_TYPES/2; ++i) // @todo bug here find it
        {
            // We can only have at most 1 piece captured for a move
            if((this->pieces[i] & ~((uint64_t) 1 << moveToApply->endIdx)) != 0)
            {
                moveToApply->ptCaptured = i;
                this->pieces[i] &= ~((uint64_t) 1 << moveToApply->endIdx);
                break;
            }
        }
        this->occupied ^= ((uint64_t) 1 << moveToApply->startIdx);
    }
    else if(((moveToApply->moveVal & MOVE_VALID_UNDO) != 0) && moveToApply->ptCaptured < NUM_PIECE_TYPES)
    {
        Util_Assert((moveToApply->ptCaptured >= enemyStart)
             && (moveToApply->ptCaptured < enemyStart + NUM_PIECE_TYPES/2),
             "Passed in bad captured piece type when attempting to undo the move");

        // return our board state
        this->pieces[enemyPieces] |= ((uint64_t) 1 << moveToApply->startIdx);
        this->pieces[moveToApply->ptCaptured] |= ((uint64_t) 1 << moveToApply->startIdx);
    }
    else
    {
        this->occupied ^= ((uint64_t) 1 << moveToApply->startIdx);
    }
    

    // Ancillary bitboards also need to be updated
/*     if(moveToApply->moveVal != MOVE_VALID_UNDO)
    {   
        
    } */
    this->occupied |= ((uint64_t) 1 << moveToApply->endIdx);
    this->empty = ~(this->occupied);

    Util_Assert((this->pieces[BLACK_PIECES] & this->pieces[WHITE_PIECES]) == 0,
        "Pieces cannot overlap on the same spot");

    Util_Assert((this->pieces[friendlyPieces] ^ this->pieces[enemyPieces]) == this->occupied,
        "Incoherence between piece states and state of actual board");


    // Move is now applied 

    // @todo: Are we attempting to make a move while our king is in check
    //    that doesn't remove our king from check after applying the move

    return STATUS_SUCCESS;

}

/**
 * Removes the last move applied to this chessboard.
 * 
 * @returns STATUS_SUCCESS if successful, STATUS_FAIL otherwise
 */
uint64_t ChessBoard::UndoMoveFromBoard(moveType_t *moveToUndo)
{
    uint8_t idx, moveValCached;

    idx = moveToUndo->endIdx;
    moveToUndo->endIdx = moveToUndo->startIdx;
    moveToUndo->startIdx = idx;

    moveValCached = moveToUndo->moveVal;
    moveToUndo->moveVal = MOVE_VALID_UNDO;

    ApplyMoveToBoard(moveToUndo);

    idx = moveToUndo->endIdx;
    moveToUndo->endIdx = moveToUndo->startIdx;
    moveToUndo->startIdx = idx;
    moveToUndo->moveVal = moveValCached;

    return STATUS_SUCCESS;

}
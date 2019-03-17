#include <iostream>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"

/**
 * Generates a move given the piece type, the start index, and the end index
 * 
 * @param pt:           The piece you are moving
 * @param startIdx:     The start index of the piece you are moving
 * @param endIdx:       Where your piece is going to go
 * @param moveVal:      The type of move you are executing
 * @param moveList      The current movelist
 * 
 * @note:   Assumption at this point is that the move is valid within the 
 *          the rules of chess
 */
void ChessBoard::BuildMove(uint8_t pt, uint8_t startIdx, uint8_t endIdx,
                             uint8_t moveVal, moveType_t **moveList)
{

    moveType_t *newMove, *lastMove;
    bool legalMove = true;
    uint8_t friendlyPieces, enemyPieces;
    uint64_t mask = ((uint64_t) 1 << endIdx);
    (pt >= NUM_PIECE_TYPES/2) ? friendlyPieces = BLACK_PIECES : friendlyPieces = WHITE_PIECES;
    (pt >= NUM_PIECE_TYPES/2) ? enemyPieces = WHITE_PIECES : enemyPieces = BLACK_PIECES;


    //Util_Assert(moveVal != MOVE_INVALID, "Tried to build an invalid move");
    if(moveVal == MOVE_INVALID)
    {
        return;
    }

    Util_Assert(pt < NUM_PIECE_TYPES, "Invalid piece type for move");

    Util_Assert(startIdx < NUM_BOARD_INDICIES && endIdx < NUM_BOARD_INDICIES
        && startIdx != endIdx, "Invalid indicies provided for move");

    Util_Assert((this->pieces[friendlyPieces] & mask) == 0,
        "There was a friendly piece where we wanted to move!");

    if((moveVal & MOVE_VALID_ATTACK) > 0)
    {
        Util_Assert(((this->pieces[enemyPieces] & mask) != 0), "Invalid attack move");
    }
    else if((moveVal & MOVE_VALID) > 0)
    {
        Util_Assert(((this->occupied & mask) == 0),
            "Invalid move: Board was occupied where we expected empty");
        Util_Assert(((this->pieces[friendlyPieces] & mask) == 0),
            "Invalid move: Friendly piece where we expected empty");
        Util_Assert(((this->pieces[enemyPieces] & mask) == 0),
            "Invalid move: Enemy pieces where we expected empty");
    }

    // Now for an interesting quirk. If we have detected that we can actually directly attack
    // the king of our enemy, we actually know that the previous move that got us here is
    // actually illegal under the rules of chess, and MUST be discounted. We can actually
    // check for this rather simply. They key is that checkmate must be detected from 
    // our turn, not the response move

    //if((pt < NUM_PIECE_TYPES/2 && (((uint64_t) 1 << endIdx) & this->pieces[BLACK_KING] != 0))
    //    || (pt >= NUM_PIECE_TYPES/2 && (((uint64_t) 1 << endIdx) & this->pieces[WHITE_KING] != 0)))
    //{
    //    legalMove = false;
    //}

    newMove = new moveType_t;
    Util_Assert(newMove != NULL, "Failed to allocate memory for new move!");

    newMove->startIdx = startIdx;
    newMove->endIdx = endIdx;

    // Denote the piece type
    newMove->pt = pt;

    // Denote what type of move this is
    newMove->moveVal = moveVal;

    newMove->legalMove = legalMove;
    // Add this move to the list of possible moves at this board position
    newMove->adjMove = *moveList;
    newMove->ptCaptured = 0xF;

    *moveList = newMove;

}
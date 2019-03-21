/* This file is responsible for the handling of threat assessments on squares */

#include <iostream>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"

/**
 * Creates the initial threat map for a chess board
 */
void ChessBoard::GenerateThreatMap(void)
{

}

/**
 * Updates the threat map for the board given a move application
 * 
 * @param moveApplied: The given move to applied to the board
 */
void ChessBoard::UpdateThreatMap(moveType_t *moveApplied)
{
    Util_Assert(moveApplied != NULL, "Move passed to threatmap update was NULL!");
}

/**
 * Reverts the threat map to its previous state
 */
void ChessBoard::RevertThreatMap(void)
{

}

/**
 * Is a given piece index under threat from a given color
 * 
 * @param whiteThreat:  Are we checking for white attacking this index
 * @param idx:          The index to check for the attack
 * 
 */
void ChessBoard::IsIndexUnderThreat(bool whiteThreat, uint8_t idx)
{

}
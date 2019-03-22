/* This file is responsible for the handling of threat assessments on squares */

#include <iostream>
#include <array>
#include <list>
#include <get>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"

// Our persistent threat map for the life of the program. Index 0 is the current state.
static std::array<std::array<std::list<threatMapEntry_t>, NUM_BOARD_INDICIES>, SEARCH_DEPTH + 1> threatMap;

static uint8_t currentSearchDepth = 0;

/**
 * Creates the initial threat map for a chess board
 */
void ChessBoard::GenerateThreatMap(void)
{
    // Generate pawn threats

    // Generate rook threats

    // Generate bishop threats

    // Generate knight threats

    // Generate queen threats

    // Generate king threats
}

/**
 * Updates the threat map for the board given a move application
 * 
 * @param moveApplied: The given move to applied to the board
 */
void ChessBoard::UpdateThreatMap(moveType_t *moveApplied, uint8_t currentSearchDepth)
{
    Util_Assert(moveApplied != NULL, "Move passed to threatmap update was NULL!");

    /**
     * Check our current index for any present threat. If there is none, then we do
     * not need to worry about the threat assessment for other pieces attacking our squaree
     */

    if(IsIndexUnderThreat(currentSearchDepth, moveApplied->startIdx) == true)
    {
        // Update threat map for every attacking piece
    }

    // Update the threat map for our given piece that just moved

}

/**
 * Reverts the threat map to its previous state
 * 
 * @param: The current search depth we are looking at
 */
void ChessBoard::RevertThreatMap()
{
    Util_Assert(currentSearchDepth > 0, "Tried to revert into the past...");
}

/**
 * Is a given piece index under threat from any color
 * 
 * @param idx:          The index to check for the attack
 * 
 */
bool ChessBoard::IsIndexUnderThreat(uint8_t idx)
{
    Util_Assert(idx < NUM_BOARD_INDICIES, "Bad piece index provided in threat mapping!");

    return std::get<idx>(std::get<currentSearchDepth>(threatMap)).empty(); // Disgusting
}

/**
 * Is a given piece index under threat from a given color
 * 
 * @param idx:          The index to check for the attack
 * @param whiteThreat:  Are we checking for white attacking this index
 * 
 */
bool ChessBoard::IsIndexUnderThreat(uint8_t idx, bool whiteThreat)
{
    bool foundThreat;

    Util_Assert(idx < NUM_BOARD_INDICIES, "Bad piece index provided in threat mapping!");

    std::list<threatMapEntry_t> idxList = std::get<idx>(std::get<currentSearchDepth>(threatMap));
    for (std::list<threatMapEntry_t>::iterator it=idxList.begin(); it != idxList.end(); ++it)
    {   
        // For now, its just the king that uses this function and it does not care
        // which piece is attacking, just that there is a threat. May have come back
        // to this later when refining the search algorithm
        if(*it->threatPt < BLACK_PAWN && !whiteThreat)
        {
            return;
        }
        else if(*it->threatPt >= BLACK_PAWN && whiteThreat)
        {
            return;
        }
    }
    return false;
}

/**
 * Reverts the entire threat map stack.
*/
void Chessboard::WipeThreatMap(void)
{
    threatmap.clear(); // this may cause problems, tbd
    currentSearchDepth = 0;
}
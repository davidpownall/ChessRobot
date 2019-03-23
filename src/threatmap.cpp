/* This file is responsible for the handling of threat assessments on squares */

#include <iostream>
#include <array>
#include <list>
#include <get>
#include <memory>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"

// Our persistent threat map for the life of the program. Index 0 is the current state.
static std::array<std::array<std::list<threatMapEntry_t>, NUM_BOARD_INDICES>, SEARCH_DEPTH + 1> threatMap;

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
void ChessBoard::UpdateThreatMap(moveType_t *moveApplied)
{
    Util_Assert(moveApplied != NULL, "Move passed to threatmap update was NULL!");

    /**
     * Check our current index for any present threat. If there is none, then we do
     * not need to worry about the threat assessment for other pieces attacking our squaree
     */

    if(IsIndexUnderThreat(currentSearchDepth, moveApplied->startIdx) == true)
    {
        // Update threat map for every attacking piece, this can include
        // rooks, bishops, and queens

        
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
    --currentSearchDepth;
}

/**
 * Is a given piece index under threat from any color
 * 
 * @param idx:          The index to check for the attack
 * 
 */
bool ChessBoard::IsIndexUnderThreat(uint8_t idx)
{
    Util_Assert(idx < NUM_BOARD_INDICES, "Bad piece index provided in threat mapping!");

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

    Util_Assert(idx < NUM_BOARD_INDICES, "Bad piece index provided in threat mapping!");

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

/**
 * Updates the threat map for rooks at our current search depth given that the provided
 * move was applied to the board
 * 
 * @param moveApplied The move that was applied
 */
void ChessBoard::UpdateRooks(moveType_t *moveApplied)
{
    Util_Assert(moveApplied != NULL, "Move applied to UpdateRooks was NULL");

    //Just fan out here in all directions looking for a possible match
    uint8_t pt;
    uint64_t rookMask, shift = 1, rookMask = this->pieces[WHITE_ROOKS] | this->pieces[BLACK_ROOKS];
    uint64_t lslVal = (8 - (moveApplied->startIdx % 8)) % 8;
    uint64_t temp;
    bool foundPiece = false;
    bool goLeft = false; 
    int  dir = 0;

    // If there actually are no pieces of this piecetype on the board
    // then just immediately return, no reason to search
    if(rookMask == 0)
    {
        return;
    }

    // Determine if we have a rook within our line of vision from this end idx from all teams

    while(rookMask != 0)
    {
        // For each of our rooks
        rookIdx = __builtin_ctz(rookMask);
        rookMask ^= (shift << rookIdx);

        // Is this rook white or black
        if(rookMask & this->pieces[WHITE_ROOK] > 0)
        {
            pt = WHITE_ROOK;
        }
        else if(rookMask & this->pieces[BLACK_ROOK] > 0)
        {
            pt = BLACK_ROOK;
        }
        else
        {
            Util_Assert(0, "Invalid rook mask calculated");
        }
    
        // Is this rook in our column
        if( (COLUMN_MASK >> LSL_VAL) & (shift << rookIdx))
        {
            (rookIdx > startIdx) ? dir = -8 : dir = 8;
            temp = moveApplied->startIdx + dir;
            foundPiece = true;
        }
        // Is this rook in our row
        else if((ROW_MASK << moveApplied->startIdx) & (shift << rookIdx))
        {
            (rookIdx > startIdx) ? dir = -1 : dir = 1;
            temp = moveApplied->startIdx + dir;
            foundPiece = true;
        }

        while(foundPiece && temp < NUM_BOARD_INDICES)
        {
            // Create the entry and add it to the list of threatMapEntrys
            auto entry = std::make_shared<threatMapEntry_t>(threatMapEntry_t{ pt, rookIdx });
            std::get<idx>(std::get<currentSearchDepth>(threatMap)).add(entry); // May have to do more here

            // We cannot look any further down and therefore should break
            if((this->occupied & (shift << temp)) != 0)
            {
                break;
            }
            // Since temp is unsigned 64 bit we will wrap to MAX_INT when we hit the bottom row or go over the number
            // indices if we hit the top row
            temp += dir;
        }
    }
}

/**
 * Updates the threat map for bishops at our current search depth given that the provided
 * move was applied to the board
 * 
 * @param moveApplied The move that was applied
 */
void ChessBoard::UpdateBishops(moveType_t *moveApplied)
{
    
}

/**
 * Updates the threat map for queens at our current search depth given that the provided
 * move was applied to the board
 * 
 * @param moveApplied The move that was applied
 */
void ChessBoard::UpdateQueens(moveType_t *moveApplied)
{
    
}
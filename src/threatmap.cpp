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
static std::array<std::array<std::list<std::shared_ptr<threatMapEntry_t>, NUM_BOARD_INDICES>, SEARCH_DEPTH + 1> threatMap;

static uint8_t currentSearchDepth = 0;

static void ThreatMap_RemoveThreatFromMap(uint8_t pt, uint8_t threatIdx, uint8_t squareIdx)
{
    std::list<std::shared_ptr<threatMapEntry_t>> idxList = std::get<squareIdx>(std::get<currentSearchDepth>(threatMap));
    std::list<std::shared_ptr<threatMapEntry_t>>::iterator it = idxList.begin();
    while (it != idxList.end())
    {
        // Remove the given threat which matches the lists location
        if((pt == *it->threatPt) && (idx == *it->threatIdx))
        {
            idxList.erase(it++);
            return;
        }
    }
    Util_Assert(false, "Unable to find specific threat we wanted to remove from threatmap");
}

static void ThreatMap_AddThreatToMap(uint8_t pt,  uint8_t threatIdx, uint8_t squareIdx)
{
    auto entry = std::make_shared<threatMapEntry_t>(threatMapEntry_t{ pt, idx });

#if DEBUG_BUILD
    // This is worth checking for in DEBUG_BUILDs, but the performance hit is probably not worth it
    std::list<std::shared_ptr<threatMapEntry_t>> idxList = std::get<idx>(std::get<currentSearchDepth>(threatMap));
    for (std::list<std::shared_ptr<threatMapEntry_t>>::iterator it=idxList.begin(); it != idxList.end(); ++it)
    {   
        Util_Assert((pt != *it->threatPt) || (idx != *it->threatIdx),
                     "Duplicate threat found when adding threat");
    }
#endif

    std::get<tempIdx>(std::get<currentSearchDepth>(threatMap)).add(entry);
}

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
        // If there are none of these PTs attacking this square, they will return quickly
        UpdateRooks();
        UpdateBishops();
        UpdateQueens();
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

    std::list<std::shared_ptr<threatMapEntry_t>> idxList = std::get<idx>(std::get<currentSearchDepth>(threatMap));
    for (std::list<std::shared_ptr<threatMapEntry_t>>::iterator it=idxList.begin(); it != idxList.end(); ++it)
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

    uint8_t pt;
    uint64_t rookIdx, shift = 1, rookMask = this->pieces[WHITE_ROOKS] | this->pieces[BLACK_ROOKS];
    uint64_t lslVal = (8 - (moveApplied->startIdx % 8)) % 8;
    uint64_t tempIdx;
    bool foundPiece = false;
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
        if((shift << rookIdx) & this->pieces[WHITE_ROOK] > 0)
        {
            pt = WHITE_ROOK;
        }
        else if((shift << rookIdx) & this->pieces[BLACK_ROOK] > 0)
        {
            pt = BLACK_ROOK;
        }
        else
        {
            Util_Assert(0, "Invalid rook mask calculated");
        }
    
        // Is this rook in our column
        if( (COLUMN_MASK >> lslVal) & (shift << rookIdx))
        {
            (rookIdx > startIdx) ? dir = -8 : dir = 8;
            tempIdx = moveApplied->startIdx + dir;
            foundPiece = true;
        }
        // Is this rook in our row
        else if((ROW_MASK << moveApplied->startIdx) & (shift << rookIdx))
        {
            (rookIdx > startIdx) ? dir = -1 : dir = 1;
            tempIdx = moveApplied->startIdx + dir;
            foundPiece = true;
        }

        // @todo: this is wrong come back to this
        while(foundPiece && tempIdx < NUM_BOARD_INDICES)
        {
            // Create the entry and add it to the list of threatMapEntrys
            ThreatMap_AddThreatToMap(pt, rookIdx, tempIdx);

            // We cannot look any further down and therefore should break
            if((this->occupied & (shift << tempIdx)) != 0)
            {
                break;
            }
            // Since tempIdx is unsigned 64 bit we will wrap to MAX_INT when we hit the bottom row or go over the number
            // indices if we hit the top row
            tempIdx += dir;
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
    uint8_t pt;
    uint64_t bishopIdx, shift = 1, bishopMask = this->pieces[WHITE_BISHOP] | this->pieces[BLACK_BISHOP];
    uint64_t tempIdx;
    bool goingLeft = false;
    int dir = 0;

    // If there actually are no pieces of this piecetype on the board
    // then just immediately return, no reason to search
    if(bishopMask == 0)
    {
        return;
    }

    std::list<std::shared_ptr<threatMapEntry_t>> idxList 
        = std::get<moveApplied->startIdx>(std::get<currentSearchDepth>(threatMap));

    for (std::list<std::shared_ptr<threatMapEntry_t>>::iterator it=idxList.begin(); it != idxList.end(); ++it)
    {   
        if(*it->threatPt != WHITE_BISHOP && *it->threatPt != BLACK_BISHOP)
        {
            continue;
        }
        bishopIdx = *it->threatIdx;
        pt = *it->threatPt;
    
        // So now we have a bishop which is certainly attacking us, and we want to update the 
        // threatmap past the piece we just moved. So which direction to we go
        if(bishopIdx > moveApplied->startIdx)
        {
            if((bishopIdx % 8) > (moveApplied->startIdx % 8))
            {
                dir = -9;
                goingLeft = true;
            }
            else
            {
                dir = -7;
                goingLeft = false;
            }
        }
        else
        {
            if((bishopIdx % 8) > (moveApplied->startIdx % 8))
            {
                dir = 7;
                goingLeft = true;
            }
            else
            {
                dir = 9;
                goingLeft = false;
            }
        }
        tempIdx = moveApplied->startIdx;
        // Can we travel in the direction we want to go in
        while((tempIdx < NUM_BOARD_INDICES)
            && ((goingLeft && (tempIdx % 8 > 1)) || (!goingLeft && (tempIdx % 8 < 7)) ))
        {
            tempIdx += dir;
        
            // Create the entry and add it to the list of threatMapEntrys
            ThreatMap_AddThreatToMap(pt, bishopIdx, tempIdx);

            // We cannot look any further down and therefore should break
            if((this->occupied & (shift << tempIdx)) != 0)
            {
                break;
            }
        }
    }
}

/**
 * Updates the threat map for queens at our current search depth given that the provided
 * move was applied to the board
 * 
 * @param moveApplied The move that was applied
 */
void ChessBoard::UpdateQueens(moveType_t *moveApplied)
{
    
    uint8_t pt;
    uint64_t queenIdx, shift = 1, queenMask = this->pieces[WHITE_QUEEN] | this->pieces[BLACK_QUEEN];
    uint64_t tempIdx;
    uint8_t moveState = 0; // 0 == diagonal, 1 == row, 2 == column
    int dir = 0;

    // If there actually are no pieces of this piecetype on the board
    // then just immediately return, no reason to search
    if(queenMask == 0)
    {
        return;
    }

    std::list<threatMapEntry_t> idxList 
        = std::get<moveApplied->startIdx>(std::get<currentSearchDepth>(threatMap));

    for (std::list<threatMapEntry_t>::iterator it=idxList.begin(); it != idxList.end(); ++it)
    {   
        if(*it->threatPt != WHITE_QUEEN && *it->threatPt != BLACK_QUEEN)
        {
            continue;
        }
        queenIdx = *it->threatIdx;
        pt = *it->threatPt;
    
        // If the queen attacking like a bishop would attack or like a rook
        if( (COLUMN_MASK >> lslVal) & (shift << queenIdx))
        {
            (bishopIdx > startIdx) ? dir = -8 : dir = 8;
            moveState = 2;
        }
        // Is this queen in our row
        else if((ROW_MASK << moveApplied->startIdx) & (shift << queenIdx))
        {
            (queenIdx > startIdx) ? dir = -1 : dir = 1;
            moveState = 1;
        }
        else
        {
            // We know that it is a diagonal attack
            moveState = 0;
            if(queenIdx > moveApplied->startIdx)
            {
                if((queenIdx % 8) > (moveApplied->startIdx % 8))
                {
                    dir = -9;
                    goingLeft = true;
                }
                else
                {
                    dir = -7;
                    goingLeft = false;
                }
            }
            else
            {
                if((queenIdx % 8) > (moveApplied->startIdx % 8))
                {
                    dir = 7;
                    goingLeft = true;
                }
                else
                {
                    dir = 9;
                    goingLeft = false;
                }
            }
        }

        tempIdx = moveApplied->startIdx;

        // Sucks but I am too tired to think of a better way right now @todo: Fix this
        while((tempIdx < NUM_BOARD_INDICES)
            && (((goingLeft && (tempIdx % 8 > 1)) || (!goingLeft && (tempIdx % 8 < 7))) // Valid diagonal moves
            || (moveState == 1 && (tempIdx + dir % 8 != 0)) // Valid row moves
            || (moveState == 2 && (tempIdx + dir < NUM_BOARD_INDICES)))) // Valid column moves
        {

            tempIdx += dir;

            // Create the entry and add it to the list of threatMapEntrys
            ThreatMap_AddThreatToMap(pt, queenIdx, tempIdx);

            // We cannot look any further down and therefore should break
            if((this->occupied & (shift << tempIdx)) != 0)
            {
                break;
            }
        }
    }
}
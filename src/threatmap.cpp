/* This file is responsible for the handling of threat assessments on squares */

#include <iostream>
#include <array>
#include <list>
#include <memory>
#include "util.h"
#include "threatmap.h"
#include "chessboard_defs.h"
#include "chessboard.h"

/**
 * Some thoughts:
 * 
 * Each piece type needs a specific function for finding all of the threats associated with it,
 * but in reality this operation is actually shared with the function which removes all of the the
 * given threats from a location. So really what it is a generic update threat for a given piece type,
 * with the paramater being whether or not to remove all threats for that piece to add them.
 * 
 */

// Jump table so we can quickly translate from piecetype to specific threat update function
typedef void (*threatFunc_t)(uint8_t, uint8_t, uint64_t, threatOpcode_e);
static threatFunc_t threatJumpTable[NUM_PIECE_TYPES] = 
{
    ThreatMap_UpdatePawnThreat,
    ThreatMap_UpdateRookThreat,
    ThreatMap_UpdateKnightThreat,
    ThreatMap_UpdateBishopThreat,
    ThreatMap_UpdateQueenThreat,
    ThreatMap_UpdateKingThreat
};

/**
 * How threats are categorized in our threat system
 * 
 * First level: Location in time w.r.t. search depth
 * Second level: Individual board index for that location in time
 * Third level: Actual threat list for that board index at that location in time
 * Fourth level: Smart pointer to threat structure
 */

// Our persistent threat map for the life of the program. Index 0 is the current state.
static std::array<std::array<threatMapIndexList_t, NUM_BOARD_INDICES>, SEARCH_DEPTH + 1> threatMap;

// Variable to keep track of search depth during traversal
static uint8_t currentSearchDepth = 0;

/**
 * Removes a provided threat from the threatmap
 * 
 * @param pt:           The piecetype of the threat
 * @param threatIdx:    The index of the threat
 * @param mapIdx:       The index to remove the threat from
 */
void ThreatMap_RemoveThreatFromMap(uint8_t pt, uint8_t threatIdx, uint8_t mapIdx)
{
     Util_Assert(pt < NUM_PIECE_TYPES, "Bad piecetype provided to ThreatMap_RemoveThreatFromMap");
     Util_Assert(threatIdx < NUM_BOARD_INDICES, "Bad threatIdx provided to ThreatMap_RemoveThreatFromMap");
     Util_Assert(mapIdx < NUM_BOARD_INDICES, "Bad mapIdx provided to ThreatMap_RemoveThreatFromMap");

    threatMapIndexList_t idxList = threatMap[currentSearchDepth][mapIdx];    
    threatMapIndexList_t::iterator it = idxList.begin();
    while (it != idxList.end())
    {
        // Remove the given threat which matches the lists location
        if((pt == (*it)->threatPt) && (mapIdx == (*it)->threatIdx))
        {
            idxList.erase(it++);
            return;
        }
        ++it;
    }
    Util_Assert(false, "Unable to find specific threat we wanted to remove from threatmap");
}

/**
 * Routes threatmap operations depending on the opCode
 * 
 * @param pt:           The piecetype to route for
 * @param threatIdx:    The index the threat is on
 * @param mapIdx:       The index on the threatmap we are looking at
 * @param opCode:       The operation code to select
 */
static inline void ThreatMap_SelectThreatMapOperation(
    uint8_t pt, uint8_t threatIdx, uint8_t mapIdx, threatOpcode_e opCode)
{
    switch(opCode)
    {
        case THREAT_DELETE:
            ThreatMap_RemoveThreatFromMap(pt, threatIdx, mapIdx);
            break;
        case THREAT_CREATE:
        case THREAT_UPDATE:
            ThreatMap_AddThreatToMap(pt, threatIdx, mapIdx, opCode);
            break;
        default:
            Util_Assert(0, "Bad opdcode for threatmap routing!");
            break;
    }
}

/**
 * Adds a provided threat from the threatmap
 * 
 * @param pt:           The piecetype of the threat
 * @param threatIdx:    The index of the threat
 * @param mapIdx:       The index to remove the threat from
 */
void ThreatMap_AddThreatToMap(uint8_t pt,  uint8_t threatIdx, uint8_t mapIdx, threatOpcode_e opCode)
{
    Util_Assert(pt < NUM_PIECE_TYPES, "Bad piecetype provided to ThreatMap_AddThreatToMap");
    Util_Assert(threatIdx < NUM_BOARD_INDICES, "Bad threatIdx provided to ThreatMap_AddThreatToMap");
    Util_Assert(mapIdx < NUM_BOARD_INDICES, "Bad mapIdx provided to ThreatMap_AddThreatToMap");
    Util_Assert(opCode != THREAT_DELETE, "Wrong API for threatmap");

    threatMapIndexList_t idxList = threatMap[currentSearchDepth][mapIdx];    
    threatMapEntryPtr_t entry = std::make_shared<threatMapEntry_t>(threatMapEntry_t{ pt, mapIdx });

    // if we have the update opcode, iterate through the list to see if we already have an entry here
    if(opCode == THREAT_UPDATE)
    {
        threatMapIndexList_t::iterator it = idxList.begin();

        while (it != idxList.end())
        {   
            if((pt == (*it)->threatPt) && (mapIdx == (*it)->threatIdx))
            {
                return;
            }
            ++it;
        }
    }

    // If there is no duplicate OR if we have the create update, just create the entry
    idxList.push_front(entry);
}

void ThreatMap_UpdatePawnThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
//     // @todo: 
}

void ThreatMap_UpdateRookThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
    uint64_t tempIdx, shift = 1;

    tempIdx = idx;
    // left
    do
    {
        // on the left side of the board, can't go that way
        if(tempIdx % 8 == 0)
        {
            break;
        }

        --tempIdx;
        ThreatMap_SelectThreatMapOperation(pt, idx, tempIdx, opCode);
    } while((tempIdx % 8 > 0) && ((occupied & (shift << tempIdx)) == 0));

    // right
    tempIdx = idx;
    do
    {
        // on the right side of the board, can't go that way
        if(tempIdx % 8 == 7)
        {
            break;
        }

        ++tempIdx;
        ThreatMap_SelectThreatMapOperation(pt, idx, tempIdx, opCode);
    } while((tempIdx % 8 < 7) && ((occupied & (shift << tempIdx)) == 0));

    // down
    tempIdx = idx;
    do
    {
        // Cannot go down
        if(tempIdx < 8)
        {
            break;
        }

        tempIdx -= 8;
        ThreatMap_SelectThreatMapOperation(pt, idx, tempIdx, opCode);
    } while((tempIdx >= 8) && ((occupied & (shift << tempIdx)) == 0));

    // up
    tempIdx = idx;
    do
    {
        // Cannot go up
        if(tempIdx > 54)
        {
            break;
        }

        tempIdx += 8;
        ThreatMap_SelectThreatMapOperation(pt, idx, tempIdx, opCode);
    } while((tempIdx <= 54) && ((occupied & (shift << tempIdx)) == 0));
}

void ThreatMap_UpdateKnightThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
//     // @todo: 
}

void ThreatMap_UpdateBishopThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
    uint64_t tempIdx, shift = 1;

    // Down left
    tempIdx = idx;
    do
    {
        // Either left column or bottom row
        if(tempIdx % 8 == 0 || tempIdx < 8)
        {
            break;
        }

        tempIdx -= 9;
        ThreatMap_SelectThreatMapOperation(pt, idx, tempIdx, opCode);
    } while((tempIdx > 0) && ((occupied & (shift << tempIdx)) == 0));

    // Down right
    tempIdx = idx;
    do
    {
        // Either right column or bottom row
        if(tempIdx % 8 == 7 || tempIdx < 8)
        {
            break;
        }

        tempIdx -= 7;
        ThreatMap_SelectThreatMapOperation(pt, idx, tempIdx, opCode);
    } while((tempIdx > 0) && ((occupied & (shift << tempIdx)) == 0));

    // Up left
    tempIdx = idx;
    do
    {
        // Either left column or top row
        if(tempIdx % 8 == 0 || tempIdx > 55)
        {
            break;
        }

        tempIdx += 7;
        ThreatMap_SelectThreatMapOperation(pt, idx, tempIdx, opCode);
    } while((tempIdx < NUM_BOARD_INDICES) && ((occupied & (shift << tempIdx)) == 0));

    // Up right
    tempIdx = idx;
    do
    {
        if(tempIdx % 8 == 7 || tempIdx > 55)
        {
            break;
        }

        tempIdx += 9;
        ThreatMap_SelectThreatMapOperation(pt, idx, tempIdx, opCode);
    } while((tempIdx < NUM_BOARD_INDICES) && ((occupied & (shift << tempIdx)) == 0));

}

void ThreatMap_UpdateQueenThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
    // Queen behavior can just be simplified to rook and bishop
    ThreatMap_UpdateBishopThreat(pt, idx, occupied, opCode);
    ThreatMap_UpdateRookThreat(pt, idx, occupied, opCode);
}

void ThreatMap_UpdateKingThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
//     // @todo: 
}

void ThreatMap_UpdatePieceThreat(uint8_t pt, uint64_t pieces, uint64_t occupied, threatOpcode_e opCode)
{
    uint64_t mask, pieceIdx, shift = 1;

    // Generate threats for provided piece type;
    while(pieces != 0)
    {
        pieceIdx = __builtin_ctz(mask);
        pieces ^= (shift << pieceIdx);
        threatJumpTable[pt % 6](pt, pieceIdx, occupied, opCode);
    }
}

/**
 * Creates the initial threat map for a chess board. Called only during initialization
 */
void ChessBoard::GenerateThreatMap(void)
{
    uint64_t mask, pieceIdx, shift = 1;

    // Generate pawn threats
    ThreatMap_UpdatePieceThreat(WHITE_PAWN, this->pieces[WHITE_PAWN], this->occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceThreat(BLACK_PAWN, this->pieces[BLACK_PAWN], this->occupied, THREAT_CREATE);

    // Generate rook threats
    ThreatMap_UpdatePieceThreat(WHITE_ROOK, this->pieces[WHITE_ROOK], this->occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceThreat(BLACK_PAWN, this->pieces[BLACK_ROOK], this->occupied, THREAT_CREATE);

    // Generate bishop threats
    ThreatMap_UpdatePieceThreat(WHITE_BISHOP, this->pieces[WHITE_BISHOP], this->occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceThreat(BLACK_BISHOP, this->pieces[BLACK_BISHOP], this->occupied, THREAT_CREATE);

    // Generate knight threats
    ThreatMap_UpdatePieceThreat(WHITE_KNIGHT, this->pieces[WHITE_KNIGHT], this->occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceThreat(BLACK_KNIGHT, this->pieces[BLACK_KNIGHT], this->occupied, THREAT_CREATE);

    // Generate queen threats
    ThreatMap_UpdatePieceThreat(WHITE_QUEEN, this->pieces[WHITE_QUEEN], this->occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceThreat(BLACK_QUEEN, this->pieces[BLACK_QUEEN], this->occupied, THREAT_CREATE);
    
    // Generate king threats
    ThreatMap_UpdatePieceThreat(WHITE_PAWN, this->pieces[WHITE_KING],this->occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceThreat(BLACK_KING, this->pieces[BLACK_KING],this->occupied, THREAT_CREATE);

}

/**
 * Updates the threat map for the board given a move application
 * 
 * @param moveApplied: The given move to applied to the board
 */
void ChessBoard::UpdateThreatMap(moveType_t *moveApplied)
{

    Util_Assert(moveApplied != NULL, "Move passed to threatmap update was NULL!");
    Util_Assert(moveApplied->pt < NUM_PIECE_TYPES, "Move with bad PT given to UpdateThreatMap");
    
    /**
     * The threatmap for any square that this piece is threatening needs to be updated.
     * Obviously there could be overlap between moves, so there is a useful optimization
     * we can make, but tbh this is probably good enough for now.
     */

    threatJumpTable[moveApplied->pt % 6](
        moveApplied->pt, moveApplied->startIdx, this->occupied, THREAT_DELETE);

    /**
     * Check our current index for any present threat. If there is none, then we do
     * not need to worry about the threat assessment for other pieces attacking our squaree
     */
    if(IsIndexUnderThreat(currentSearchDepth, moveApplied->startIdx) == true)
    {
        // @todo: still target this for refactor
        ThreatMap_UpdatePieceThreat(WHITE_ROOK, this->pieces[WHITE_ROOK], this->occupied, THREAT_UPDATE);
        ThreatMap_UpdatePieceThreat(WHITE_BISHOP, this->pieces[WHITE_BISHOP], this->occupied, THREAT_UPDATE);
        ThreatMap_UpdatePieceThreat(WHITE_QUEEN, this->pieces[WHITE_QUEEN], this->occupied, THREAT_UPDATE);

        ThreatMap_UpdatePieceThreat(BLACK_ROOK, this->pieces[BLACK_ROOK], this->occupied, THREAT_UPDATE);
        ThreatMap_UpdatePieceThreat(BLACK_BISHOP, this->pieces[BLACK_BISHOP], this->occupied, THREAT_UPDATE);
        ThreatMap_UpdatePieceThreat(BLACK_QUEEN, this->pieces[BLACK_QUEEN], this->occupied, THREAT_UPDATE);
    }    

    // Update the threat map for our given piece that just moved --> Compiler "should" create a jump table for this
    threatJumpTable[moveApplied->pt % 6](moveApplied->pt, moveApplied->endIdx, this->occupied, THREAT_CREATE);
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
bool ChessBoard::IsIndexUnderThreat(uint8_t currentSearchDepth, uint8_t idx)
{
    Util_Assert(idx < NUM_BOARD_INDICES, "Bad piece index provided in threat mapping!");

    return threatMap[currentSearchDepth][idx].empty();
}

/**
 * Is a given piece index under threat from a given color
 * 
 * @param idx:          The index to check for the attack
 * @param whiteThreat:  Are we checking for white attacking this index
 * 
 */
bool ChessBoard::IsIndexUnderThreat(uint8_t currentSearchDepth, uint8_t idx, bool whiteThreat)
{
    bool foundThreat;

    Util_Assert(idx < NUM_BOARD_INDICES, "Bad piece index provided in threat mapping!");

    threatMapIndexList_t idxList = threatMap[currentSearchDepth][idx];    
    threatMapIndexList_t::iterator it = idxList.begin();
    while (it != idxList.end())
    {
        // For now, its just the king that uses this function and it does not care
        // which piece is attacking, just that there is a threat. May have come back
        // to this later when refining the search algorithm
        if((*it)->threatPt < BLACK_PAWN && !whiteThreat)
        {
            return true;
        }
        else if((*it)->threatPt >= BLACK_PAWN && whiteThreat)
        {
            return true;
        }
        ++it;
    }
    return false;
}

/**
 * Reverts the entire threat map stack.
*/
void ThreatMap_WipeMap(void)
{
    // We don't actually have to delete anything, just revert to depth 0
    currentSearchDepth = 0;
}

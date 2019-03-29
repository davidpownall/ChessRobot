/* This file is responsible for the handling of threat assessments on squares */

#include <iostream>
#include <array>
#include <list>
#include <memory>
#include "util.h"
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
typedef void (*generator)(uint8_t pt, uint8_t idx, uint64_t occupied, bool addThreat) threatFunc_t;
static threatFunc_t threatJumpTable[NUM_PIECE_TYPES] = 
    {
        ThreatMap_UpdatePawnThreat,
        ThreatMap_UpdateRookThreat,
        ThreatMap_UpdateKnightThreat,
        ThreatMap_UpdateBishopThreat,
        ThreatMap_UpdateQueenThreat,
        ThreatMap_UpdateKingThreat
    }

typedef enum
{
    THREAT_DELETE, // Delete all threats for a given piece
    THREAT_CREATE, // Create threats for a given piece
    THREAT_UPDATE  // Update theats for a given piece, ignores already created
} threatOpCode_t;

// Our persistent threat map for the life of the program. Index 0 is the current state.
static std::array<std::array<std::list<std::shared_ptr<threatMapEntry_t>, NUM_BOARD_INDICES>, SEARCH_DEPTH + 1> threatMap;

static uint8_t currentSearchDepth = 0;

/**
 * Removes a provided threat from the threatmap
 * 
 * @param pt:           The piecetype of the threat
 * @param threatIdx:    The index of the threat
 * @param mapIdx:       The index to remove the threat from
 */
static void ThreatMap_RemoveThreatFromMap(uint8_t pt, uint8_t threatIdx, uint8_t mapIdx)
{
    Util_Assert(pt < NUM_PIECE_TYPES, "Bad piecetype provided to ThreatMap_RemoveThreatFromMap");
    Util_Assert(threatIdx < NUM_BOARD_INDICES, "Bad threatIdx provided to ThreatMap_RemoveThreatFromMap");
    Util_Assert(mapIdx < NUM_BOARD_INDICES, "Bad mapIdx provided to ThreatMap_RemoveThreatFromMap");

    std::list<std::shared_ptr<threatMapEntry_t>> idxList = std::get<mapIdx>(std::get<currentSearchDepth>(threatMap));
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

/**
 * Adds a provided threat from the threatmap
 * 
 * @param pt:           The piecetype of the threat
 * @param threatIdx:    The index of the threat
 * @param mapIdx:       The index to remove the threat from
 */
static void ThreatMap_AddThreatToMap(uint8_t pt,  uint8_t threatIdx, uint8_t mapIdx, threatOpCode_t opCode)
{
    Util_Assert(pt < NUM_PIECE_TYPES, "Bad piecetype provided to ThreatMap_AddThreatToMap");
    Util_Assert(threatIdx < NUM_BOARD_INDICES, "Bad threatIdx provided to ThreatMap_AddThreatToMap");
    Util_Assert(mapIdx < NUM_BOARD_INDICES, "Bad mapIdx provided to ThreatMap_AddThreatToMap");
    Util_Assert(opCode != THREAT_DELETE, "Wrong API for threatmap");

    auto entry = std::make_shared<threatMapEntry_t>(threatMapEntry_t{ pt, idx });

    // if we have the update opcode, iterate through the list to see if we already have an entry here
    if(opCode == THREAT_UPDATE)
    {
        std::list<std::shared_ptr<threatMapEntry_t>> idxList = std::get<idx>(std::get<currentSearchDepth>(threatMap));
        for (std::list<std::shared_ptr<threatMapEntry_t>>::iterator it=idxList.begin(); it != idxList.end(); ++it)
        {   
            if((pt == *it->threatPt) && (idx == *it->threatIdx))
            {
                return;
            }
        }
    }

    // If there is no duplicate OR if we have the create update, just create the entry
    std::get<tempIdx>(std::get<currentSearchDepth>(threatMap)).add(entry);
}

static void ThreatMap_UpdatePawnThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpCode_t opCode)
{
    // @todo: 
}

static void ThreatMap_UpdateRookThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpCode_t opCode)
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
        switch(opCode)
        {
            case THREAT_DELETE:
                ThreatMap_RemoveThreatFromMap(pt, idx, tempIdx);
                break;
            case THREAT_CREATE:
            case THREAT_UPDATE:
                ThreatMap_AddThreatToMap(pt, idx, tempIdx, opCode);
                break;
        }
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
        switch(opCode)
        {
            case THREAT_DELETE:
                ThreatMap_RemoveThreatFromMap(pt, idx, tempIdx);
                break;
            case THREAT_CREATE:
            case THREAT_UPDATE:
                ThreatMap_AddThreatToMap(pt, idx, tempIdx, opCode);
                break;
        }
    } while((temp % 8 < 7) && ((occupied & (shift << tempIdx)) == 0));

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
        switch(opCode)
        {
            case THREAT_DELETE:
                ThreatMap_RemoveThreatFromMap(pt, idx, tempIdx);
                break;
            case THREAT_CREATE:
            case THREAT_UPDATE:
                ThreatMap_AddThreatToMap(pt, idx, tempIdx, opCode);
                break;
        }
    } while((temp >= 8) && ((occupied & (shift << tempIdx)) == 0));

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
        switch(opCode)
        {
            case THREAT_DELETE:
                ThreatMap_RemoveThreatFromMap(pt, idx, tempIdx);
                break;
            case THREAT_CREATE:
            case THREAT_UPDATE:
                ThreatMap_AddThreatToMap(pt, idx, tempIdx, opCode);
                break;
        }
    } while((temp <= 54) && && ((occupied & (shift << tempIdx)) == 0));
}

static void ThreatMap_UpdateKnightThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpCode_t opCode)
{
    // @todo: 
}

static void ThreatMap_UpdateBishopThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpCode_t opCode)
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
        switch(opCode)
        {
            case THREAT_DELETE:
                ThreatMap_RemoveThreatFromMap(pt, idx, tempIdx);
                break;
            case THREAT_CREATE:
            case THREAT_UPDATE:
                ThreatMap_AddThreatToMap(pt, idx, tempIdx, opCode);
                break;
        }

    } while((bishopIdx >) 0 && ((occupied & (shift << tempIdx)) == 0));

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
        switch(opCode)
        {
            case THREAT_DELETE:
                ThreatMap_RemoveThreatFromMap(pt, idx, tempIdx);
                break;
            case THREAT_CREATE:
            case THREAT_UPDATE:
                ThreatMap_AddThreatToMap(pt, idx, tempIdx, opCode);
                break;
        }
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
        switch(opCode)
        {
            case THREAT_DELETE:
                ThreatMap_RemoveThreatFromMap(pt, idx, tempIdx);
                break;
            case THREAT_CREATE:
            case THREAT_UPDATE:
                ThreatMap_AddThreatToMap(pt, idx, tempIdx, opCode);
                break;
        }
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
        switch(opCode)
        {
            case THREAT_DELETE:
                ThreatMap_RemoveThreatFromMap(pt, idx, tempIdx);
                break;
            case THREAT_CREATE:
            case THREAT_UPDATE:
                ThreatMap_AddThreatToMap(pt, idx, tempIdx, opCode);
                break;
        }
    } while((tempIdx < NUM_BOARD_INDICES) && ((occupied & (shift << tempIdx)) == 0));

}

static void ThreatMap_UpdateQueenThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpCode_t opCode)
{
    // Queen behavior can just be simplified to rook and bishop
    ThreatMap_UpdateBishopThreat(pt, idx, occupied, opCode);
    ThreatMap_UpdateRookThreat(pt, idx, occupied, opCode);
}

static void ThreatMap_UpdateKingThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpCode_t opCode)
{
    // @todo: 
}

static void ThreatMap_UpdatePieceThreat(uint8_t pt, uint64_t pieces, threatOpCode_t opCode)
{
    uint64_t mask, pieceIdx, shift = 1;

    // Generate threats for provided piece type;
    while(pieces != 0)
    {
        pieceIdx = __builtin_ctz(mask);
        pieces ^= (shift << pieceIdx);
        threatJumpTable[pt % 6](pt, pieceIdx, occupied, addThreat);
    }
}

/**
 * Creates the initial threat map for a chess board. Called only during initialization
 */
void ChessBoard::GenerateThreatMap(void)
{
    uint64_t mask, pieceIdx, shift = 1;

    // Generate pawn threats
    ThreatMap_UpdatePieceThreat(WHITE_PAWN, this->pieces[WHITE_PAWN], true);
    ThreatMap_UpdatePieceThreat(BLACK_PAWN, this->pieces[BLACK_PAWN], true);

    // Generate rook threats
    ThreatMap_UpdatePieceThreat(WHITE_ROOK, this->pieces[WHITE_ROOK], true);
    ThreatMap_UpdatePieceThreat(BLACK_PAWN, this->pieces[BLACK_ROOK], true);

    // Generate bishop threats
    ThreatMap_UpdatePieceThreat(WHITE_BISHOP, this->pieces[WHITE_BISHOP], true);
    ThreatMap_UpdatePieceThreat(BLACK_BISHOP, this->pieces[BLACK_BISHOP], true);

    // Generate knight threats
    ThreatMap_UpdatePieceThreat(WHITE_KNIGHT, this->pieces[WHITE_KNIGHT], true);
    ThreatMap_UpdatePieceThreat(BLACK_KNIGHT, this->pieces[BLACK_KNIGHT], true);

    // Generate queen threats
    ThreatMap_UpdatePieceThreat(WHITE_QUEEN, this->pieces[WHITE_QUEEN], true);
    ThreatMap_UpdatePieceThreat(BLACK_QUEEN, this->pieces[BLACK_QUEEN], true);
    
    // Generate king threats
    ThreatMap_UpdatePieceThreat(WHITE_PAWN, this->pieces[WHITE_KING], true);
    ThreatMap_UpdatePieceThreat(BLACK_KING, this->pieces[BLACK_KING], true);

}

/**
 * Updates the threat map for the board given a move application
 * 
 * @param moveApplied: The given move to applied to the board
 */
void ChessBoard::UpdateThreatMap(moveType_t *moveApplied)
{
    uint8_t enemyStart;

    Util_Assert(moveApplied != NULL, "Move passed to threatmap update was NULL!");
    Util_Assert(moveApplied->pt < NUM_PIECE_TYPES, "Move with bad PT given to UpdateThreatMap")

    (moveApplied->pt < BLACK_PAWN) ? enemyStart = BLACK_PAWN : enemyStart = WHITE_PAWN;
    
    /**
     * The threatmap for any square that this piece is threatening needs to be updated.
     * Obviously there could be overlap between moves, so there is a useful optimization
     * we can make, but tbh this is probably good enough for now.
     */

    threatJumpTable[moveApplied->pt % 6](moveApplied->pt, moveApplied->startIdx, this->occupied, false);

    /**
     * Check our current index for any present threat. If there is none, then we do
     * not need to worry about the threat assessment for other pieces attacking our squaree
     */
    if(IsIndexUnderThreat(currentSearchDepth, moveApplied->startIdx) == true)
    {
        // @todo: still target this for refactor
        ThreatMap_UpdatePieceThreat(enemyStart + 1, this->pieces[enemyStart + 1], true);
        ThreatMap_UpdatePieceThreat(enemyStart + 3, this->pieces[enemyStart + 3], true);
        ThreatMap_UpdatePieceThreat(enemyStart + 4, this->pieces[enemyStart + 4], true);
    }    

    // Update the threat map for our given piece that just moved --> Compiler "should" create a jump table for this
    threatJumpTable[moveApplied->pt % 6](moveApplied->pt, moveApplied->endIdx, this->occupied, true);
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

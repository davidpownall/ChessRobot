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

    threatMapIndexList_t::iterator it = threatMap[currentSearchDepth][mapIdx].begin();
    threatMapEntry_t *entry;
    while (it != threatMap[currentSearchDepth][mapIdx].end())
    {
        // Remove the given threat which matches the lists location
        if((pt == (*it)->threatPt) && (threatIdx == (*it)->threatIdx))
        {
            // Erase the threat
            entry = *it;
            Util_Assert(entry != NULL, "Found a null entry when trying to delete");
            delete(entry);

            // Erase the element in the list
            threatMap[currentSearchDepth][mapIdx].erase(it++);
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

    // if we have the update opcode, iterate through the list to see if we already have an entry here
    if(opCode == THREAT_UPDATE)
    {
        threatMapIndexList_t::iterator it = threatMap[currentSearchDepth][mapIdx].begin();

        while (it != threatMap[currentSearchDepth][mapIdx].end())
        {   
            if((pt == (*it)->threatPt) && (mapIdx == (*it)->threatIdx))
            {
                return;
            }
            ++it;
        }
    }

    // If there is no duplicate OR if we have the create update, just create the entry
    threatMap[currentSearchDepth][mapIdx].push_front(new threatMapEntry_t{pt, threatIdx});

}

/**
 * Update the threat layout for a provided pawn
 * 
 * @param pt:       The piecetype of the pawn
 * @param idx:      The index of the pawn
 * @param occupied: The current state of the board 
 *                  --> Not used but required for threatMapFunc_t type
 * @param opCode:   The operation to perform
 * 
 */
void ThreatMap_UpdatePawnThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
    Util_Assert((pt == WHITE_PAWN) || (pt == BLACK_PAWN), 
        "Pawn was not provided to ThreatMap_UpdatePawnThreat");
    Util_Assert(((idx >= 8) && (idx < NUM_BOARD_INDICES - 8)), 
        "Pawn was in last row and has not been converted to another piece");

    if(pt == WHITE_PAWN)
    {
        if((idx % 8 > 0))
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx + 7, opCode);
        }
        if(idx % 8 < 7)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx + 9, opCode);
        }
    }
    else if(pt == BLACK_PAWN)
    {
        if((idx % 8 > 0))
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx - 9, opCode);
        }
        if(idx % 8 < 7)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx - 7, opCode);
        }
    }
    else
    {
        Util_Assert(0, "Invalid pawn piece type provided");
    }
    
}

/**
 * Update the threat layout for a provided rook
 * 
 * @param pt:       The piecetype of the rook
 * @param idx:      The index of the rook
 * @param occupied: The current state of the board 
 * @param opCode:   The operation to perform
 * 
 */
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

/**
 * Update the threat layout for a provided knight
 * 
 * @param pt:       The piecetype of the knight
 * @param idx:      The index of the knight
 * @param occupied: The current state of the board 
 *                      --> Not used but required for threatMapFunc_t type
 * @param opCode:   The operation to perform
 * 
 */
void ThreatMap_UpdateKnightThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
    // Assess upwards vertical moves
    if(idx < NUM_BOARD_INDICES - 16)
    {
        // Can we move left
        if(idx % 8 > 0)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx + 15, opCode);
        }
        // Can we move right
        if(idx % 8 < 7)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx + 17, opCode);
        }
    }

    // Assess downwards vertical moves
    if(idx >= 16)
    {
        // Can we move left
        if(idx % 8 > 0)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx - 17, opCode);
        }
        // Can we move right
        if(idx % 8 < 7)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx - 15, opCode);
        }
    }

    // Assess left horizontal moves
    if(idx % 8 > 1)
    {
        // Can we move up
        if(idx < NUM_BOARD_INDICES - 8)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx + 6, opCode);
        }
        // Can we move down
        if(idx >= 8)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx - 10, opCode);
        }
    }

    // Assess right horizontal moves
    if(idx % 8 < 6)
    {
        // Can we move up
        if(idx < NUM_BOARD_INDICES - 8)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx + 10, opCode);
        }
        // Can we move down
        if(idx >= 8)
        {
            ThreatMap_SelectThreatMapOperation(pt, idx, idx - 6, opCode);
        }
    }
}

/**
 * Update the threat layout for a provided bishop
 * 
 * @param pt:       The piecetype of the bishop
 * @param idx:      The index of the bishop
 * @param occupied: The current state of the board 
 * @param opCode:   The operation to perform
 * 
 */
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

/**
 * Update the threat layout for a provided queen
 * 
 * @param pt:       The piecetype of the queen
 * @param idx:      The index of the queen
 * @param occupied: The current state of the board 
 * @param opCode:   The operation to perform
 * 
 */
void ThreatMap_UpdateQueenThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
    // Queen behavior can just be simplified to rook and bishop
    ThreatMap_UpdateBishopThreat(pt, idx, occupied, opCode);
    ThreatMap_UpdateRookThreat(pt, idx, occupied, opCode);
}

/**
 * Update the threat layout for a provided king
 * 
 * @param pt:       The piecetype of the king
 * @param idx:      The index of the king
 * @param occupied: The current state of the board 
 *                      --> Not used but required for threatMapFunc_t type
 * @param opCode:   The operation to perform
 * 
 */
void ThreatMap_UpdateKingThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode)
{
    // King can move in all directions
    
    // left
    if(idx % 8 != 0)
    {
        ThreatMap_SelectThreatMapOperation(pt, idx, idx - 1, opCode);
    }

    // right
    if(idx % 8 != 7)
    {
        ThreatMap_SelectThreatMapOperation(pt, idx, idx + 1, opCode);
    }

    // up
    if(idx < NUM_BOARD_INDICES - 8)
    {
        ThreatMap_SelectThreatMapOperation(pt, idx, idx + 8, opCode);
    }

    // down
    if(idx >= 8)
    {
        ThreatMap_SelectThreatMapOperation(pt, idx, idx - 8, opCode);
    }

    // down left
    if(idx % 8 != 0 && idx >= 8)
    {
        ThreatMap_SelectThreatMapOperation(pt, idx, idx - 9, opCode);
    }

    // down right
    if(idx % 8 != 7 && idx >= 8)
    {
        ThreatMap_SelectThreatMapOperation(pt, idx, idx - 7, opCode);
    }

    // up left
    if(idx % 8 != 0 && idx < NUM_BOARD_INDICES - 8)
    {
        ThreatMap_SelectThreatMapOperation(pt, idx, idx + 7, opCode);

    }

    // up right
    if(idx % 8 != 7 && idx < NUM_BOARD_INDICES - 8)
    {
        ThreatMap_SelectThreatMapOperation(pt, idx, idx + 9, opCode);
    }

}

/**
 * Update the threat layout for a provided pieceType
 * 
 * @param pt:       The piecetype
 * @param idx:      The mask of all the pieces of this type
 * @param occupied: The current state of the board 
 * @param opCode:   The operation to perform
 * 
 */
void ThreatMap_UpdatePieceTypeThreat(uint8_t pt, uint64_t pieces, uint64_t occupied, threatOpcode_e opCode)
{
    uint64_t pieceIdx, shift = 1;

    // Generate threats for provided piece type;
    while(pieces != 0)
    {
        pieceIdx = __builtin_ctzll(pieces);
        pieces ^= (shift << pieceIdx);
        threatJumpTable[pt % 6](pt, pieceIdx, occupied, opCode);
    }
}

/**
 * Creates the initial threat map for a chess board. Called only during initialization
 */
void ThreatMap_Generate(uint64_t *pieces, uint64_t occupied)
{
    uint64_t mask, pieceIdx, shift = 1;

    // Generate pawn threats
    ThreatMap_UpdatePieceTypeThreat(WHITE_PAWN, pieces[WHITE_PAWN], occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceTypeThreat(BLACK_PAWN, pieces[BLACK_PAWN], occupied, THREAT_CREATE);

    // Generate rook threats
    ThreatMap_UpdatePieceTypeThreat(WHITE_ROOK, pieces[WHITE_ROOK], occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceTypeThreat(BLACK_ROOK, pieces[BLACK_ROOK], occupied, THREAT_CREATE);

    // Generate bishop threats
    ThreatMap_UpdatePieceTypeThreat(WHITE_BISHOP, pieces[WHITE_BISHOP], occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceTypeThreat(BLACK_BISHOP, pieces[BLACK_BISHOP], occupied, THREAT_CREATE);

    // Generate knight threats
    ThreatMap_UpdatePieceTypeThreat(WHITE_KNIGHT, pieces[WHITE_KNIGHT], occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceTypeThreat(BLACK_KNIGHT, pieces[BLACK_KNIGHT], occupied, THREAT_CREATE);

    // Generate queen threats
    ThreatMap_UpdatePieceTypeThreat(WHITE_QUEEN, pieces[WHITE_QUEEN], occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceTypeThreat(BLACK_QUEEN, pieces[BLACK_QUEEN], occupied, THREAT_CREATE);
    
    // Generate king threats
    ThreatMap_UpdatePieceTypeThreat(WHITE_KING, pieces[WHITE_KING],occupied, THREAT_CREATE);
    ThreatMap_UpdatePieceTypeThreat(BLACK_KING, pieces[BLACK_KING],occupied, THREAT_CREATE);
}

/**
 * Updates the threat map for the board given a move application
 * 
 * @param moveApplied: The given move to applied to the board
 */
void ThreatMap_Update(moveType_t *moveApplied, uint64_t *pieces, uint64_t occupied, bool realMove)
{
    uint64_t passThroughThreatMask, shift = 1;

    Util_Assert(moveApplied != NULL, "Move passed to threatmap update was NULL!");
    Util_Assert(moveApplied->pt < NUM_PIECE_TYPES, "Move with bad PT given to ThreatMap_Update");
    
    if(realMove)
    {
        currentSearchDepth = 0; // If we have a real move, we are updating the real copy of the threatmap
    }
    else
    {
        currentSearchDepth++; // If we have a simulated move, we need to go to the next copy
        for(int i = 0; i < NUM_PIECE_TYPES; i++)
        {
            threatMap[currentSearchDepth][i].clear(); // So we can eliminate the references to unwanted threats

            // Now copy the list over
            std::copy (
                  threatMap[currentSearchDepth - 1][i].begin(), 
                  threatMap[currentSearchDepth - 1][i].end(), 
                  threatMap[currentSearchDepth][i].begin() 
                );
        }
    }
    /**
     * The threatmap for any square that this piece is threatening needs to be updated.
     * Obviously there could be overlap between moves, so there is a useful optimization
     * we can make, but tbh this is probably good enough for now.
     */

    threatJumpTable[moveApplied->pt % 6](
        moveApplied->pt, moveApplied->startIdx, occupied, THREAT_DELETE);

    /**
     * Check our current index for any present threat that could attack through the piece
     * being moved. The mask returned below will reflect which specific passthrough attaqcks
     * are going through our index.
     */

    passThroughThreatMask 
        = ThreatMap_AttackThroughPiecesTargetingIndex(currentSearchDepth, moveApplied->startIdx);

    if(passThroughThreatMask != 0)
    {
        if(passThroughThreatMask & (shift << WHITE_ROOK))
        {
            ThreatMap_UpdatePieceTypeThreat(
                WHITE_ROOK, pieces[WHITE_ROOK], occupied, THREAT_UPDATE);
            passThroughThreatMask ^= (shift << WHITE_ROOK);
        }

        if(passThroughThreatMask & (shift << WHITE_BISHOP))
        {
            ThreatMap_UpdatePieceTypeThreat(
                WHITE_BISHOP, pieces[WHITE_BISHOP], occupied, THREAT_UPDATE);
            passThroughThreatMask ^= (shift << WHITE_BISHOP);
        }

        if(passThroughThreatMask & (shift << WHITE_QUEEN))
        {
            ThreatMap_UpdatePieceTypeThreat(
                WHITE_QUEEN, pieces[WHITE_QUEEN], occupied, THREAT_UPDATE);           
            passThroughThreatMask ^= (shift << WHITE_QUEEN);
        }

        if(passThroughThreatMask & (shift << BLACK_ROOK))
        {
            ThreatMap_UpdatePieceTypeThreat(
                BLACK_ROOK, pieces[BLACK_ROOK], occupied, THREAT_UPDATE);
            passThroughThreatMask ^= (shift << BLACK_ROOK);

        }

        if(passThroughThreatMask & (shift << BLACK_BISHOP))
        {
            ThreatMap_UpdatePieceTypeThreat(
                BLACK_BISHOP, pieces[BLACK_BISHOP], occupied, THREAT_UPDATE);
            passThroughThreatMask ^= (shift << BLACK_BISHOP);
        }

        if(passThroughThreatMask & (shift << BLACK_QUEEN))
        {
            ThreatMap_UpdatePieceTypeThreat(
                BLACK_QUEEN, pieces[BLACK_QUEEN], occupied, THREAT_UPDATE);
            passThroughThreatMask ^= (shift << BLACK_QUEEN);

        }

        Util_Assert(passThroughThreatMask == 0, "An unknown threat appeared in our mask");
    }    

    // Update the threat map for our given piece that just moved --> Compiler "should" create a jump table for this
    threatJumpTable[moveApplied->pt % 6](moveApplied->pt, moveApplied->endIdx, occupied, THREAT_CREATE);
}

/**
 * Are rooks, bishops, or queens of any type attacking an index
 * 
 * @param currentSearchDepth:  The search depth to investigate
 * @param idx:                 The index on the board to investigate 
 */
uint64_t ThreatMap_AttackThroughPiecesTargetingIndex(uint8_t searchDepth, uint8_t idx)
{
    uint64_t mask, shift = 1;

    threatMapIndexList_t::iterator it = threatMap[searchDepth][idx].begin();
    while (it != threatMap[searchDepth][idx].end())
    {
        switch((*it)->threatPt)
        {
            case WHITE_ROOK:
                mask |= (shift << WHITE_ROOK);
                break;
            case WHITE_BISHOP:
                mask |= (shift << WHITE_BISHOP);
                break;
            case WHITE_QUEEN:
                mask |= (shift << WHITE_QUEEN);
                break;
            case BLACK_ROOK:
                mask |= (shift << BLACK_ROOK);
                break;
            case BLACK_BISHOP:
                mask |= (shift << BLACK_BISHOP);
                break;
            case BLACK_QUEEN:
                mask |= (shift << BLACK_QUEEN);
                break;
            default:
                break; // Do nothing
        }
        ++it;
    }    
    return mask;
}

/**
 * Reverts the threat map to its previous state
 * 
 * @param: The current search depth we are looking at
 */
void ThreatMap_RevertState()
{
    Util_Assert(currentSearchDepth > 0, "Tried to revert into the past...");
    --currentSearchDepth;
}

/**
 * Is a given piece index under threat from any color
 * 
 * @param searchDepth:  The search depth to investigate
 * @param idx:          The index to check for the attack
 * 
 */
bool ThreatMap_IsIndexUnderThreat(uint8_t searchDepth, uint8_t idx)
{
    Util_Assert(idx < NUM_BOARD_INDICES, "Bad piece index provided in threat mapping!");

    return threatMap[searchDepth][idx].empty();
}

/**
 * Is a given piece index under threat from a given color
 * 
 * @param idx:          The index to check for the attack
 * @param whiteThreat:  Are we checking for white attacking this index
 * 
 */
bool ThreatMap_IsIndexUnderThreat(uint8_t idx, bool whiteThreat)
{
    bool foundThreat;

    Util_Assert(idx < NUM_BOARD_INDICES, "Bad piece index provided in threat mapping!");

    threatMapIndexList_t::iterator it = threatMap[currentSearchDepth][idx].begin();
    while (it != threatMap[currentSearchDepth][idx].end())
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
    threatMapIndexList_t::iterator it;;
    threatMapEntry_t *entry;

    for(uint8_t i = 1; i < SEARCH_DEPTH; ++i)
    {
        for(uint8_t j = 0; j < NUM_BOARD_INDICES; ++j)
        {
            it = threatMap[i][j].begin();
            while(it != threatMap[i][j].end())
            {
                // Erase the threat
                entry = *it;
                Util_Assert(entry != NULL, "Found a null entry when trying to delete");
                delete(entry);

                // Erase the element in the list
                threatMap[i][j].erase(it++);
            }
        }
    }
    currentSearchDepth = 0;
}

/**
 * Is the provided king in check.
 * 
 * @param kingIdx:      Index containing the king
 * @param threatColor:  The color of the threat we are looking for    
 * 
 * @note: Assumes that the king is actually at hte position passed in
 * @note: Assumes current search depth (may change in the future)
 */
bool ThreatMap_IsKingInCheckAtIndex(uint8_t kingIdx, uint8_t threatColor)
{
    // Essentially just look at the threat map for the provided location
    Util_Assert(threatColor == WHITE_PIECES || threatColor == BLACK_PIECES,
        "Bad color provided to ThreatMap_IsKingInCheckAtIndex");

    return ThreatMap_IsIndexUnderThreat(kingIdx, threatColor == WHITE_PIECES);
}

/**
 * Is the provided king in checkmate at an index.
 * 
 * @param kingIdx:      Index containing the king
 * @param threatColor:  The color of the threat we are looking for    
 * @param pieces:       Pointer to board state of pieces
 * 
 * @note: Assumes that the king is actually at hte position passed in
 * @note: Assumes current search depth (may change in the future)
 */
bool ThreatMap_IsKingInCheckMateAtIndex(
        uint8_t kingIdx, uint8_t threatColor, uint64_t *pieces)
{
    // Essentially just look at the threat map for the provided location
    Util_Assert(threatColor == WHITE_PIECES || threatColor == BLACK_PIECES,
        "Bad color provided to ThreatMap_IsKingInCheckMateAtIndex");

    bool foundMove = false;
    uint64_t shift = 1;
    uint8_t friendlyColor;
    (threatColor == WHITE_PIECES) ? friendlyColor = BLACK_PIECES : friendlyColor = WHITE_PIECES;

    // Conditions for checkmate:

    // 1) Is the king in check
    if(ThreatMap_IsIndexUnderThreat(kingIdx, threatColor) == false)
    {
        // Not in check
        return false;
    }
    
    // 2) Can the king move?
    // The king can move if the location we want to go to is not occupied by a friend and
    // there is no active threatMap on that location

    // Can we move down
    if(kingIdx >= 8)
    {
        if((shift << (kingIdx - 8)) & pieces[friendlyColor] == 0)
        {
            foundMove = !ThreatMap_IsKingInCheckAtIndex(kingIdx - 8, threatColor);
        }        
    }

    // Can we move up
    if(!foundMove && (kingIdx < NUM_BOARD_INDICES - 8))
    {
        if((shift << (kingIdx + 8)) & pieces[friendlyColor] == 0)
        {
            foundMove = !ThreatMap_IsKingInCheckAtIndex(kingIdx + 8, threatColor);
        }
    }

    // Can we move left
    if(!foundMove && (kingIdx % 8 > 0))
    {
        if((shift << (kingIdx - 1)) & pieces[friendlyColor] == 0)
        {
            foundMove = !ThreatMap_IsKingInCheckAtIndex(kingIdx - 1, threatColor);
        }        
    }

    // Can we move right
    if(!foundMove && (kingIdx % 8 < 7))
    {
        if((shift << (kingIdx + 1)) & pieces[friendlyColor] == 0)
        {
            foundMove = !ThreatMap_IsKingInCheckAtIndex(kingIdx + 1, threatColor);
        }        
    }

    // Diag left down
    if(!foundMove && ((kingIdx >= 8) && (kingIdx % 8 > 0)))
    {
        if((shift << (kingIdx - 9)) & pieces[friendlyColor] == 0)
        {
            foundMove = !ThreatMap_IsKingInCheckAtIndex(kingIdx - 9, threatColor);
        }        
    }
    
    // Diag left up
    if(!foundMove && ((kingIdx < NUM_BOARD_INDICES - 8) && (kingIdx % 8 > 0)))
    {
        if((shift << (kingIdx + 7)) & pieces[friendlyColor] == 0)
        {
            foundMove = !ThreatMap_IsKingInCheckAtIndex(kingIdx + 7, threatColor);
        }        
    }

    // Diag right down
    if(!foundMove && ((kingIdx >= 8) && (kingIdx % 8 < 7)))
    {
        if((shift << (kingIdx - 7)) & pieces[friendlyColor] == 0)
        {
            foundMove = !ThreatMap_IsKingInCheckAtIndex(kingIdx - 7, threatColor);
        }                
    }

    // Diag right up
    if(!foundMove && ((kingIdx < NUM_BOARD_INDICES - 8) && (kingIdx % 8 < 7)))
    {
        if((shift << (kingIdx + 9)) & pieces[friendlyColor] == 0)
        {
            foundMove = !ThreatMap_IsKingInCheckAtIndex(kingIdx + 9, threatColor);
        }                
    }

    return  (foundMove == false);
}
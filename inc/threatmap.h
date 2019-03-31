#include <memory>

/**
 * Enum for opcodes when performing operations on threatmap
 */
typedef enum
{
    THREAT_DELETE, // Delete all threats for a given piece
    THREAT_CREATE, // Create threats for a given piece
    THREAT_UPDATE  // Update theats for a given piece, ignores already created
} threatOpcode_e;

/**
 * The structure for an entry in our threat map. Contains
 * 
 *  1) The piece type initiating the threat
 *  2) The location of the piece threatening the square
 */
typedef struct threatMapEntry_s
{
    uint8_t threatPt;
    uint8_t threatIdx;
} threatMapEntry_t;

/**
 * Simple shared_ptr typedef for our threatMap entires
 */
typedef std::shared_ptr<threatMapEntry_t> threatMapEntryPtr_t;
typedef std::list<threatMapEntryPtr_t> threatMapIndexList_t;

void ThreatMap_WipeMap(void);
void ThreatMap_RemoveThreatFromMap(uint8_t pt, uint8_t threatIdx, uint8_t mapIdx);
void ThreatMap_AddThreatToMap(uint8_t pt,  uint8_t threatIdx, uint8_t mapIdx, threatOpcode_e opCode);
void ThreatMap_UpdatePawnThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode);
void ThreatMap_UpdateRookThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode);
void ThreatMap_UpdateKnightThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode);
void ThreatMap_UpdateBishopThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode);
void ThreatMap_UpdateQueenThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode);
void ThreatMap_UpdateKingThreat(uint8_t pt, uint8_t idx, uint64_t occupied, threatOpcode_e opCode);
void ThreatMap_UpdatePieceThreat(uint8_t pt, uint64_t pieces, uint64_t occupied, threatOpcode_e opCode);

/* @todo: Convert remaining threatmap functions over */
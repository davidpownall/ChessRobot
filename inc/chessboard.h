#include <cstdint>
#include <string>
#include <queue>

#ifndef CHESSBOARD_DEFINE
#define CHESSBOARD_DEFINE

#define WHITE_PAWN_START    0xff00
#define BLACK_PAWN_START    0x00ff000000000000

#define WHITE_ROOK_START    0x81
#define BLACK_ROOK_START    0x8100000000000000

#define WHITE_KNIGHT_START  0x42
#define BLACK_KNIGHT_START  0x4200000000000000

#define WHITE_BISHOP_START  0x24
#define BLACK_BISHOP_START  0x2400000000000000

#define WHITE_QUEEN_START   0x10
#define BLACK_QUEEN_START   0x800000000000000

#define WHITE_KING_START    0x08
#define BLACK_KING_START    0x1000000000000000

#define BOARD_START_USED    0xffff00000000ffff
#define BOARD_START_EMPTY   0x0000FFFFFFFF0000

#define COLUMN_MASK         0x1010101010101010
#define BOARD_MASK          0xFFFFFFFFFFFFFFFF

#define NUM_PIECE_TYPES     12
#define MAX_EVAL_MOVES      40
#define NUM_BOARD_INDICIES  64

#define MOVE_INVALID            0x0
#define MOVE_VALID              0x1
#define MOVE_VALID_ATTACK       0x2
#define MOVE_VALID_CASTLE_KING  0x4
#define MOVE_VALID_CASTLE_QUEEN 0x8
#define MOVE_VALID_CHECK        0x10
#define MOVE_VALID_MATE         0x20
#define MOVE_VALID_UNDO         0x40

#define WHITE_PAWN 0x0
#define WHITE_ROOK 0x1
#define WHITE_BISHOP 0x2
#define WHITE_KNIGHT 0x3
#define WHITE_QUEEN 0x4
#define WHITE_KING 0x5
#define BLACK_PAWN 0x6
#define BLACK_ROOK 0x7
#define BLACK_BISHOP 0x8
#define BLACK_KNIGHT 0x9
#define BLACK_QUEEN 0xa
#define BLACK_KING 0xb
#define WHITE_PIECES 0xc
#define BLACK_PIECES 0xd

/**
 * The structure which defines a given move applied to a chessboard. In order to
 * maximize the performance of the engine, 16 bytes is the hard cap of structure size.
 */
typedef struct moveType_s
{
    // Forms the move list for chessbord state
    struct moveType_s *adjMove;

    uint32_t startIdx: 6; // Start index of our move
    uint32_t endIdx: 6; // End index of our move
    uint32_t pt: 4; // What piece type we are moving
    uint32_t ptCaptured: 4; // What piece type we captured, if any
    uint32_t moveVal: 7; // What type of move this is
    uint32_t legalMove: 1; // Is this move actually legal
    uint32_t reserved: 4;

    // Another 4 bytes of padding will be added by the compiler

} moveType_t;

class ChessBoard
{
private:
    /* 12 unique bitboard representations required + two general */
    uint64_t pieces[NUM_PIECE_TYPES + 2];
    uint64_t prevPieces[NUM_PIECE_TYPES + 2];

    /* Union of all bitboards */
    uint64_t occupied;

    /* Positions of all empty squares */
    uint64_t empty;

    // The current value of the chessboard
    //      Positive = white's advantage
    //      Negative = black's advantage
    int64_t value;
    int64_t prevValue;

    uint64_t threatMap;

public:

    ChessBoard(void);
    ChessBoard(uint64_t *pieces, uint64_t occupied, uint64_t searchDepth, moveType_t *lastMove);

    uint64_t GetPiece(uint8_t pt) const { return pieces[pt]; };
    uint64_t GetWhitePieces() const { return pieces[WHITE_PIECES]; };
    uint64_t GetBlackPieces() const { return pieces[BLACK_PIECES]; };

    uint64_t GetWhitePawns() const { return pieces[WHITE_PAWN]; };
    uint64_t GetWhiteRooks() const { return pieces[WHITE_ROOK]; };
    uint64_t GetWhiteKnights() const { return pieces[WHITE_KNIGHT]; }; 
    uint64_t GetWhiteBishops() const { return pieces[WHITE_BISHOP]; }; 
    uint64_t GetWhiteQueen() const { return pieces[WHITE_QUEEN]; };
    uint64_t GetWhiteKing() const { return pieces[WHITE_KING]; };
    
    uint64_t GetBlackPawns() const { return pieces[BLACK_PAWN]; };
    uint64_t GetBlackRooks() const { return pieces[BLACK_ROOK]; };
    uint64_t GetBlackKnights() const { return pieces[BLACK_KNIGHT]; };
    uint64_t GetBlackBishops() const { return pieces[BLACK_BISHOP]; };
    uint64_t GetBlackQueen() const { return pieces[BLACK_QUEEN]; };
    uint64_t GetBlackKing() const { return pieces[BLACK_KING]; };

    int64_t GetCurrentValue() const {return value;}
    static int64_t EvaluateCurrentBoardValue(ChessBoard *cb);

    int32_t GetBestMove(uint64_t depth, bool playerToMaximize,
                         moveType_t *movesToEvaluateAtThisDepth, int32_t alpha, int32_t beta);
    moveType_t *GenerateMoves(uint8_t pt);
    moveType_t *GetNextMove(uint8_t pt);
    void BuildMove(uint8_t pt, uint8_t startIdx, uint8_t endIdx, uint8_t moveVal, moveType_t **moveList);
    uint64_t ApplyMoveToBoard(moveType_t *moveToApply);
    uint64_t UndoMoveFromBoard(moveType_t *moveToUndo);

    static bool IsValidRookMove(ChessBoard *cb, uint8_t idxToFind, uint8_t endIdx);
    static bool IsValidKnightMove(uint8_t idxToFind, uint8_t endIdx);
    static bool IsValidBishopMove(ChessBoard *cb, uint8_t idxToFind, uint8_t endIdx);
    static bool IsValidQueenMove(ChessBoard *cb, uint8_t idxToFind, uint8_t endIdx);
    bool IsValidMove(uint8_t pt, uint8_t idxToFind, uint8_t endIdx);

    uint8_t CheckSpaceForMoveOrAttack(uint64_t idxToEval, uint8_t friendlyPieces, uint8_t enemyPieces);

    void GeneratePawnMoves(uint8_t pt, moveType_t **moveList);
    void GenerateRookMoves(uint8_t pt, moveType_t **moveList);
    void GenerateBishopMoves(uint8_t pt, moveType_t **moveList);
    void GenerateKnightMoves(uint8_t pt, moveType_t **moveList);
    void GenerateQueenMoves(uint8_t pt, moveType_t **moveList);
    void GenerateKingMoves(uint8_t pt, moveType_t **moveList);

};

std::string ConvertMoveToString(ChessBoard *cb, moveType_t *move);
moveType_t *GetOpponentMove(ChessBoard *cb, std::string str);
int64_t GetPositionValueFromTable(uint64_t pieceTypeBase, uint64_t idx);
void PlayGame(void);

#endif // CHESSBOARD_DEFINE
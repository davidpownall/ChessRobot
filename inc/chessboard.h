#include <cstdint>
#include <queue>

#ifndef CHESSBOARD_DEFINE
#define CHESSBOARD_DEFINE

#define WHITE_PAWN_START    0xff00
#define BLACK_PAWN_START    0x00ff000000000000

#define WHITE_ROOK_START    0x129
#define BLACK_ROOK_START    0x8100000000000000

#define WHITE_KNIGHT_START  0x42
#define BLACK_KNIGHT_START  0x4200000000000000

#define WHITE_BISHOP_START  0x100100
#define BLACK_BISHOP_START  0x2400000000000000

#define WHITE_QUEEN_START   0x10
#define BLACK_QUEEN_START   0x800000000000000

#define WHITE_KING_START    0x08
#define BLACK_KING_START    0x1000000000000000

#define BOARD_START_USED    0xffff00000000ffff
#define BOARD_START_EMPTY   0x0000FFFFFFFF0000

#define NUM_PIECE_TYPES     12
#define MAX_EVAL_MOVES      40
#define NUM_BOARD_INDICIES  64

class ChessBoard;

/* Enum defining results of potential move evaluation */
enum moveValidity_e
{
    MOVE_INVALID,
    MOVE_VALID,
    MOVE_VALID_ATTACK
};

/* Enum definining piece index */
enum pieceType_e
{
    WHITE_PAWN,
    WHITE_ROOK,
    WHITE_BISHOP,
    WHITE_KNIGHT,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_ROOK,
    BLACK_BISHOP,
    BLACK_KNIGHT,
    BLACK_QUEEN,
    BLACK_KING,
    WHITE_PIECES,
    BLACK_PIECES
};

typedef struct moveType_s
{

    // The ChessBoard where this move was made
    ChessBoard *currentCB;

    // The ChessBoard resulting from this move 
    ChessBoard *resultCB;

    // The value of the board resulting from the move
    uint64_t resultValue;

    // The actual chars containing the chess move
    char moveString[4];

    uint64_t startIdx;
    uint64_t endIdx;

    // What type of move is this?
    moveValidity_e moveVal;

    // What piece we are moving
    pieceType_e pt;

    // How we can compare two moves
    bool operator<(const struct moveType_s*& rhs) const
    {
        return resultValue < rhs->resultValue;
    }

} moveType_t;

class ChessBoard
{
private:
    /* 12 unique bitboard representations required + two general */
    uint64_t pieces[NUM_PIECE_TYPES + 2];

    /* Union of all bitboards */
    uint64_t occupied;

    /* Positions of all empty squares */
    uint64_t empty;

    // The current value of the chessboard
    //      Positive = white's advantage
    //      Negative = black's advantage
    int64_t value;

    uint64_t numMovesAtThisDepth;
    moveType_t *movesToEvaluateAtThisDepth;

    std::priority_queue<moveType_t*> availableMoves;
    moveType_t *lastMove;

public:

    ChessBoard(void);
    ChessBoard(uint64_t *pieces, uint64_t occupied, uint64_t numMovesToEval, moveType_t *lastMove);

    uint64_t getPiece(pieceType_e pt) const { return pieces[pt]; };
    uint64_t getWhitePieces() const { return pieces[WHITE_PIECES]; };
    uint64_t getBlackPieces() const { return pieces[BLACK_PIECES]; };

    uint64_t getWhitePawns() const { return pieces[WHITE_PAWN]; };
    uint64_t getWhiteRooks() const { return pieces[WHITE_ROOK]; };
    uint64_t getWhiteKnights() const { return pieces[WHITE_KNIGHT]; }; 
    uint64_t getWhiteBishops() const { return pieces[WHITE_BISHOP]; }; 
    uint64_t getWhiteQueen() const { return pieces[WHITE_QUEEN]; };
    uint64_t getWhiteKing() const { return pieces[WHITE_KING]; };
    
    uint64_t getBlackPawns() const { return pieces[BLACK_PAWN]; };
    uint64_t getBlackRooks() const { return pieces[BLACK_ROOK]; };
    uint64_t getBlackKnights() const { return pieces[BLACK_KNIGHT]; };
    uint64_t getBlackBishops() const { return pieces[BLACK_BISHOP]; };
    uint64_t getBlackQueen() const { return pieces[BLACK_QUEEN]; };
    uint64_t getBlackKing() const { return pieces[BLACK_KING]; };

    int64_t getCurrentValue() const {return value;}
    static int64_t evaluateCurrentBoardValue(ChessBoard *cb);

    void generateMoves(pieceType_e pt);
    moveType_t *getNextMove(pieceType_e pt);
    void buildMove(pieceType_e pt, uint64_t startIdx, uint64_t endIdx, moveValidity_e moveVal);
    uint64_t applyMoveToBoard(moveType_t *moveToApply);

    moveValidity_e checkSpaceForMoveOrAttack(uint64_t idxToEval, pieceType_e enemyPieces);

    void generatePawnMoves(pieceType_e pt);
    void generateRookMoves(pieceType_e pt);
    void generateBishopMoves(pieceType_e pt);
    void generateKnightMoves(pieceType_e pt);
    void generateQueenMoves(pieceType_e pt);
    void generateKingMoves(pieceType_e pt);

    void spawnNextChessBoard(moveType_t *moveToExecute);

};

uint64_t initializeChessBoard(void);

#endif // CHESSBOARD_DEFINE
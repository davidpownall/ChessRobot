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

    // Forms the move list for currentCB. Enforce that the list is always
    // in descending order based on the value of the output board
    struct moveType_s *adjMove;

    // What type of move is this?
    moveValidity_e moveVal;

    // What piece we are moving
    pieceType_e pt;

    // How we can compare two moves
    bool operator<(const struct moveType_s rhs) const
    {
        return resultValue < rhs.resultValue;
    }

    bool operator<=(const struct moveType_s rhs) const
    {
        return resultValue <= rhs.resultValue;
    }

    // @todo: Add more operators if necessary

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
    moveType_t *lastMove;

public:

    ChessBoard(void);
    ChessBoard(uint64_t *pieces, uint64_t occupied, uint64_t numMovesToEval, moveType_t *lastMove);

    uint64_t GetPiece(pieceType_e pt) const { return pieces[pt]; };
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

    void GenerateMoves(pieceType_e pt);
    moveType_t *GetNextMove(pieceType_e pt);
    void BuildMove(pieceType_e pt, uint64_t startIdx, uint64_t endIdx, moveValidity_e moveVal);
    uint64_t ApplyMoveToBoard(moveType_t *moveToApply);

    static void AddMoveToMoveList(ChessBoard *cb, moveType_t *moveToAdd);
    static void DeleteMove(moveType_t *moveToDelete);

    moveValidity_e CheckSpaceForMoveOrAttack(uint64_t idxToEval, pieceType_e enemyPieces);

    void GeneratePawnMoves(pieceType_e pt);
    void GenerateRookMoves(pieceType_e pt);
    void GenerateBishopMoves(pieceType_e pt);
    void GenerateKnightMoves(pieceType_e pt);
    void GenerateQueenMoves(pieceType_e pt);
    void GenerateKingMoves(pieceType_e pt);

    void SpawnNextChessBoard(moveType_t *moveToExecute);

};

uint64_t InitializeChessBoard(void);

#endif // CHESSBOARD_DEFINE
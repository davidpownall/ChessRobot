#include <cstdint>

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

/* Enum definining piece index */
enum pieceType_t
{
    WHITE_PIECES,
    BLACK_PIECES,
    WHITE_PAWN,
    BLACK_PAWN,
    WHITE_ROOK,
    BLACK_ROOK,
    WHITE_BISHOP,
    BLACK_BISHOP,
    WHITE_KNIGHT,
    BLACK_KNIGHT,
    WHITE_QUEEN,
    BLACK_QUEEN,
    WHITE_KING,
    BLACK_KING
};

class ChessBoard
{
private:
    /* 12 unique bitboard representations required + two general */
    uint64_t pieces[14];

    /* Union of all bitboards */
    uint64_t occupied;

    /* Positions of all empty squares */
    uint64_t empty;

public:

    ChessBoard(void);

    uint64_t getPiece(pieceType_t pt) const { return pieces[pt]; };
    uint64_t getWhitePieces() const { return pieces[WHITE_PIECES]; };
    uint64_t getBlackPieces() const { return pieces[BLACK_PIECES]; };

    uint64_t getWhitePawns() const { return pieces[WHITE_PAWN]; };
    uint64_t getBlackPawns() const { return pieces[BLACK_PAWN]; };
    uint64_t getWhiteRooks() const { return pieces[WHITE_ROOK]; };
    uint64_t getBlackRooks() const { return pieces[BLACK_ROOK]; };
    uint64_t getWhiteKnights() const { return pieces[WHITE_KNIGHT]; }; 
    uint64_t getBlackKnights() const { return pieces[BLACK_KNIGHT]; };
    uint64_t getWhiteBishops() const { return pieces[WHITE_BISHOP]; }; 
    uint64_t getBlackBishops() const { return pieces[BLACK_BISHOP]; };
    uint64_t getWhiteQueen() const { return pieces[WHITE_QUEEN]; };
    uint64_t getBlackQueen() const { return pieces[BLACK_QUEEN]; };
    uint64_t getWhiteKing() const { return pieces[WHITE_KING]; };
    uint64_t getBlackKing() const { return pieces[BLACK_KING]; };
};

uint64_t initializeChessBoard(void);
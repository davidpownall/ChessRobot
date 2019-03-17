#ifndef CHESSBOARD_DEFINITIONS_DEFINE
#define CHESSBOARD_DEFINITIONS_DEFINE

// How many moves into the future we want to look
#define SEARCH_DEPTH 5

// Starting positions for pieces
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

// Starting positions for board
#define BOARD_START_USED    0xffff00000000ffff
#define BOARD_START_EMPTY   0x0000ffffffff0000

// Useful masks
#define COLUMN_MASK         0x1010101010101010
#define BOARD_MASK          0xffffffffffffffff

// Game definitions
#define NUM_PIECE_TYPES     12
#define NUM_BOARD_INDICIES  64

// Move types
#define MOVE_INVALID            0x0
#define MOVE_VALID              0x1
#define MOVE_VALID_ATTACK       0x2
#define MOVE_VALID_CASTLE_KING  0x4
#define MOVE_VALID_CASTLE_QUEEN 0x8
#define MOVE_VALID_CHECK        0x10
#define MOVE_VALID_MATE         0x20
#define MOVE_VALID_UNDO         0x40

// Piece definitions
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

#endif // CHESSBOARD_DEFINITIONS_DEFINE
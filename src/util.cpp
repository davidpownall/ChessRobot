#include "util.h"
#include "chessboard.h"

static char pawnChar   = 'p'; // I know pawn technically doesn't use a letter.....
static char rookChar   = 'R';
static char bishopChar = 'B';
static char knightChar = 'N';
static char queenChar  = 'Q';
static char kingChar   = 'K';

char Util_ConvertPieceTypeToChar(uint8_t pt)
{
    // @todo Convert this to a macro
    switch(pt)
    {
        case WHITE_PAWN:
        case BLACK_PAWN:
            return pawnChar;
        
        case WHITE_ROOK:
        case BLACK_ROOK:
            return rookChar;
        
        case WHITE_BISHOP:
        case BLACK_BISHOP:
            return bishopChar;
        
        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            return knightChar;
        
        case WHITE_QUEEN:
        case BLACK_QUEEN:
            return queenChar;

        case WHITE_KING:
        case BLACK_KING:
            return kingChar;
        
        default:
            return 0;
    }
}

std::string Util_ConvertPieceTypeToString(uint8_t pt)
{

    switch(pt)
    {
        case WHITE_PAWN:
            return "WHITE_PAWN";
        case BLACK_PAWN:        
            return "BLACK_PAWN";
        case WHITE_ROOK:
            return "WHITE_ROOK";
        case BLACK_ROOK:       
            return "BLACK_ROOK"; 
        case WHITE_BISHOP:
            return "WHITE_BISHOP";
        case BLACK_BISHOP:
            return "BLACK_BISHOP";
        case WHITE_KNIGHT:
            return "WHITE_KNIGHT";
        case BLACK_KNIGHT:
            return "BLACK_KNIGHT";
        case WHITE_QUEEN:
            return "WHITE_QUEEN";
        case BLACK_QUEEN:
            return "BLACK_QUEEN";
        case WHITE_KING:
            return "WHITE_KING";
        case BLACK_KING:
            return "BLACK_KING";
        default:
            Util_Assert(false, "Bad piece type provided");
            return NULL;
    }
}

// Disgusting hack
static const unsigned char BitReverseTable256[256] = 
{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
    R6(0), R6(2), R6(1), R6(3)
};

/**
 * Bitwise reverse of the provided 64 bit integer
 * 
 * @param toReverse: The uint64_T to reverse
 * 
 * @note: Adapted from standford bitwise hacks https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
 */
void Util_Reverse64BitInteger(uint64_t *toReverse)
{
    unsigned int lower32 = *toReverse & 0xFFFFFFFF; // reverse 32-bit value, 8 bits at time
    unsigned int upper32 = (*toReverse & 0xFFFFFFFF00000000) >> 32; // Upper 32 bits to reverse
    unsigned int reversalResult;
    unsigned char * p = (unsigned char *) &upper32;
    unsigned char * q = (unsigned char *) &reversalResult;

    *toReverse = 0;
    q[3] = BitReverseTable256[p[0]]; 
    q[2] = BitReverseTable256[p[1]]; 
    q[1] = BitReverseTable256[p[2]]; 
    q[0] = BitReverseTable256[p[3]];

    *toReverse = (*toReverse | reversalResult);

    p = (unsigned char *) &lower32;
    q[3] = BitReverseTable256[p[0]]; 
    q[2] = BitReverseTable256[p[1]]; 
    q[1] = BitReverseTable256[p[2]]; 
    q[0] = BitReverseTable256[p[3]];

    *toReverse = (*toReverse | ((uint64_t) reversalResult << 32));    
}



/**
 * Basic Util_Assertion function for catching runtime errors. I would
 * use the C++ Util_Assert but that terminates the program, and I would
 * rather hang execution so I can see the callstack immediately on
 * failure. 
 * 
 * @param expr: The actual Util_Assertion which must be true
 * @param str:  The output string to go to console output
 */
void Util_Assert(bool expr, std::string str)
{
#if DEBUG_BUILD
    if(!expr)
    {
        std::cout << str << std::endl;
        std::cout << "ASSERT -- Program hanging" << std::endl;
        while(true)
        {
            ;
        }
    }
#endif // DEBUG_BUILD
}


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
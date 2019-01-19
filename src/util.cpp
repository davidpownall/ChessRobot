#include "util.h"
#include "chessboard.h"

static char pawnChar   = 'p'; // I know pawn technically doesn't use a letter.....
static char rookChar   = 'R';
static char bishopChar = 'B';
static char knightChar = 'N';
static char queenChar  = 'Q';
static char kingChar   = 'K';

char convertPieceTypeToChar(pieceType_e pt)
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
 * Basic assertion function for catching runtime errors. I would
 * use the C++ assert but that terminates the program, and I would
 * rather hang execution so I can see the callstack immediately on
 * failure. 
 * 
 * @param expr: The actual assertion which must be true
 * @param str:  The output string to go to console output
 */
void ASSERT(bool expr, std::string str)
{
#if DEBUG_BUILD
    if(!expr)
    {
        std::cout << str << std::endl;
        std::cout << "Program hanging" << std::endl;
        while(true)
        {
            ;
        }
    }
#endif // DEBUG_BUILD
}
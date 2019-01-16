#include "util.h"
#include "chessboard.h"

static char pawnChar   = 'p'; // I know pawn technically doesn't use a letter.....
static char rookChar   = 'R';
static char bishopChar = 'B';
static char knightChar = 'N';
static char queenChar  = 'Q';
static char kingChar   = 'K';

char convertPieceTypeToChar(pieceType_t pt)
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
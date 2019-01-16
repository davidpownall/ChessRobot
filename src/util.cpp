#include "util.h"
#include "chessboard.h"

static char pawnChar   = 'p';
static char rookChar   = 'r';
static char bishopChar = 'b';
static char knightChar = 'k';
static char queenChar  = 'q';
static char kingChar   = 'k';

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
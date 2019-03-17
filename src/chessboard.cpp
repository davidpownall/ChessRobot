#include <iostream>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"

static ChessBoard *cb;

/**
 * Default constructor for the ChessBoard class. Creates a fresh board
 * from scratch.
 */
ChessBoard::ChessBoard(void)
{
    uint32_t pt;

    this->pieces[WHITE_PAWN]    = WHITE_PAWN_START;
    this->pieces[WHITE_ROOK]    = WHITE_ROOK_START;
    this->pieces[WHITE_KNIGHT]  = WHITE_KNIGHT_START;
    this->pieces[WHITE_BISHOP]  = WHITE_BISHOP_START;
    this->pieces[WHITE_QUEEN]   = WHITE_QUEEN_START;
    this->pieces[WHITE_KING]    = WHITE_KING_START;
    this->pieces[WHITE_PIECES]  = 0;

    this->pieces[BLACK_PAWN]    = BLACK_PAWN_START;
    this->pieces[BLACK_ROOK]    = BLACK_ROOK_START;
    this->pieces[BLACK_KNIGHT]  = BLACK_KNIGHT_START;
    this->pieces[BLACK_BISHOP]  = BLACK_BISHOP_START;
    this->pieces[BLACK_QUEEN]   = BLACK_QUEEN_START;
    this->pieces[BLACK_KING]    = BLACK_KING_START;
    this->pieces[BLACK_PIECES]  = 0;

    for(pt = 0; pt < NUM_PIECE_TYPES; ++pt)
    {
        if(pt < (NUM_PIECE_TYPES/2))
        {
            this->pieces[WHITE_PIECES] |= this->pieces[pt];
        }
        else
        {
            this->pieces[BLACK_PIECES] |= this->pieces[pt];
        }
    }

    this->occupied = BOARD_START_USED;
    this->empty = BOARD_START_EMPTY;

    // Value is 0 at game start
    this->value = EvaluateCurrentBoardValue(this);

}

/**
 * ChessBoard constructor which takes in an existing board state
 * 
 * @param *pieces:          The current board position of all pieces
 * @param occupued:         The set of all occupied squares
 * @param searchDepth:   Number of moves to evaluate at this depth
 * @param *lastMove:        The move which generated this position
 */
ChessBoard::ChessBoard(uint64_t *pieces, uint64_t occupied, uint64_t searchDepth, moveType_t *lastMove)
{

    // Verify that our array is valid
    if(pieces == NULL)
    {
        std::cout << "Error in piece array input during board construction"
             << std::endl;
        return;
    } 

    if(lastMove == NULL)
    {
        std::cout << "Bad last move provided to CB" << std::endl;
        return;
    }

    ChessBoard::EvaluateCurrentBoardValue(this);
}



/**
 * Utility function for determining if a piece can be moved to or attacked on
 * 
 * @param idxToEval:        The index for us to evaluate
 * @param friendlyPieces:   The pieceType of our allies
 * @param enemyPieces:      The pieceType of the enemy
 * 
 * @return  The validity of the selected move
 */
uint8_t ChessBoard::CheckSpaceForMoveOrAttack(uint64_t idxToEval, uint8_t friendlyPieces, uint8_t enemyPieces)
{

    uint64_t mask = ((uint64_t) 1 << idxToEval);

    // We cannot move into our own team
    if((mask & this->pieces[friendlyPieces]) != 0)
    {
        return MOVE_INVALID;
    }

    // Otherwise is this move a move into an empty space
    if((mask & this->occupied == 0) && (mask & this->pieces[enemyPieces] == 0))
    {
        return MOVE_VALID;
    }
    else if((mask & this->pieces[friendlyPieces] == 0) && (mask & this->pieces[enemyPieces] != 0))
    {
        return MOVE_VALID_ATTACK;
    }   
    return MOVE_INVALID;
}

int64_t ChessBoard::EvaluateCurrentBoardValue(ChessBoard *cb)
{
    uint64_t idx, pieces;
    int64_t value = 0, sign;
    Util_Assert(cb != NULL, "NULL Chessboard provided to evaluation function");

    for(idx = 0; idx < NUM_PIECE_TYPES; ++idx)
    {
        // @todo: deal with lategame kings
        if((uint8_t) idx == WHITE_KING || (uint8_t) idx == BLACK_KING)
        {
            continue;
        }

        // Essentially our piece value table must be mirrored for black, so we handle
        // that here
        pieces = cb->pieces[idx];
        if(idx < NUM_PIECE_TYPES / 2)
        {
            sign = 1;
        }
        else
        {
            // Our piece tables assume we are white player, so we spoof ourselves as white
            Util_Reverse64BitInteger(&pieces);
            sign = -1;
        }
        while(pieces != 0)
        {  
            value += sign*GetPositionValueFromTable(idx % 6, __builtin_ctzll(pieces));
            pieces ^= (uint64_t) 1 << __builtin_ctzll(pieces);
        }
    }
    return value;
}

moveType_t *ConvertStringToMove(ChessBoard* cb, std::string str)
{
    moveType_t *move;
    uint64_t idxToFind, mask, count;
    
    Util_Assert(cb != NULL, "Was passed a bad chessboard");
    Util_Assert(!str.empty(), "Was passed empty string");

    move = new moveType_t;
    Util_Assert(move != NULL, "Failed to allocate move");

    move->endIdx = (str[str.length() - 2] - 'a') + ((int) str[str.length() - 1])*8;
    
    // Pawn move
    if(!isupper(str[0]))
    {
        move->pt = WHITE_PAWN;
    
        // Capture
        if(str[1] == 'x')
        {
            move->moveVal = MOVE_VALID_ATTACK;
            if((str[str.length() - 2] - 'a') - (str[0] - '0') > 0)
            {
                move->startIdx = move->endIdx - 7;
            }
            else
            {
                move->startIdx = move->endIdx - 9;
            }
        }
        else
        {
            move->moveVal = MOVE_VALID;

            // Pawn has to move forward, therefore find current index
            mask = cb->GetPiece(WHITE_PAWN) & (((uint64_t) COLUMN_MASK) << (str[str.length() - 2] - 'a'));
            mask ^= (uint64_t) 1 << move->endIdx;

            count = 1;

            // We now are looking two possibilities
            while(mask > 0 && count <= 2)
            {
                if((cb->GetPiece(WHITE_PAWN) & ((uint64_t) 1 << (move->endIdx - 8*count))) > 0)
                {
                    // Are we in the fourth row
                    if(count == 2 && move->endIdx/8 != 3)
                    {
                        count++;
                        break;
                    }
                    move->startIdx = (move->endIdx - 8*count);
                    break;
                }
                count++;
            }

            Util_Assert(count <= 2, "Invalid incoming pawn move");
        }
    }
    else
    {
        if(str[0] == 'R')
        {
            move->pt = WHITE_ROOK;    
        }
        else if(str[0] == 'N')
        {
            move->pt = WHITE_KNIGHT;
        }
        else if(str[0] == 'B')
        {
            move->pt = WHITE_BISHOP;
        }
        else if(str[0] == 'Q')
        {
            move->pt = WHITE_QUEEN;
        }
        else if(str[0] == 'K')
        {
            move->pt = WHITE_KING;
        }
        else
        {
            Util_Assert(0, "Bad piece type provided in move");
        }

        Util_Assert(cb->GetPiece(move->pt) != 0, "Illegal move!");

        // If second in str is a lower case letter or number, we can tell immediately what our start idx
        // if number
        if(str[1] - '0' < 8)
        {
            move->startIdx = __builtin_ctzll(cb->GetPiece(move->pt) & (0xFF << (str[1] - '0')*8));
        }
        // else if lower case letter
        else if(str[1] - 'a' < 8)
        {
            idxToFind = cb->GetPiece(move->pt);
            while(__builtin_ctzll(idxToFind) % 8 != str[1] - 'a')
            {
                idxToFind ^= ((uint64_t) 1 << __builtin_ctzll(idxToFind));
            }
            move->startIdx = (uint64_t) 1 << __builtin_ctzll(idxToFind);
        }
        else
        {
            // otherwise we need to find our start location. We know there wouldn't have been
            // additional info in the move string iff there was only one piece that could make that move
            idxToFind = cb->GetPiece(move->pt);
            while(!cb->IsValidMove(move->pt, __builtin_ctzll(idxToFind), move->endIdx))
            {
                idxToFind ^= ((uint64_t) 1 << __builtin_ctzll(idxToFind));
            }
            move->startIdx = __builtin_ctzll(idxToFind);
        }
    }

    return move;
}

std::string outputStr;
std::string ConvertMoveToString(ChessBoard *cb, moveType_t *move)
{
    std::string moveStr;
    uint64_t mask;

    Util_Assert(move != NULL, "Move provided was NULL");    
    Util_Assert(cb != NULL, "Chessboard provided was NULL");

    outputStr.clear();
    // temporary until I can flesh this out
    outputStr.append("\npt: ");
    outputStr.append( Util_ConvertPieceTypeToString(move->pt) + "\n");
    outputStr.append("start: ");
    outputStr.append(std::to_string(move->startIdx) + "\n");
    outputStr.append("end: ");
    outputStr.append(std::to_string(move->endIdx) + "\n");

    return outputStr;

    // Is this a castle?
    if(move->moveVal & MOVE_VALID_CASTLE_KING)
    {
        moveStr = "O-O";
    }
    // Is this a castle?
    else if(move->moveVal & MOVE_VALID_CASTLE_QUEEN)
    {
        moveStr = "O-O-O";
    }    
    // Is this a capture?
    else if(move->moveVal & MOVE_VALID_ATTACK)
    {
        moveStr = "x";
    }
    // Is this a regular move?
    else if(move->moveVal & MOVE_VALID)
    {
        moveStr = "";
    }
    else
    {
        Util_Assert(0, "Invalid move the provided");
    }
    
    // Was this move a check move
    if(move->moveVal & MOVE_VALID_CHECK)
    {

    }
    // Was this move a checkmate
    else if(move->moveVal & MOVE_VALID_MATE)
    {

    }

    // Are there are pieces of the same type that can make this move?
    if(move->pt % NUM_PIECE_TYPES/2 != 0 && move->pt % NUM_PIECE_TYPES/2 != 5)
    {
        
    }

    // If yes, specify at the start of move string
        // Common row
        // Common column
        // Knight special case

    // Get row and column from the endIdx

    return moveStr;
}

#include <iostream>
#include "util.h"
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

    this->pieces[BLACK_PAWN]    = BLACK_PAWN_START;
    this->pieces[BLACK_ROOK]    = BLACK_ROOK_START;
    this->pieces[BLACK_KNIGHT]  = BLACK_KNIGHT_START;
    this->pieces[BLACK_BISHOP]  = BLACK_BISHOP_START;
    this->pieces[BLACK_QUEEN]   = BLACK_QUEEN_START;
    this->pieces[BLACK_KING]    = BLACK_KING_START;

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
        this->occupied |= this->pieces[pt];
    }
    this->empty = ~(this->occupied);

    this->numMovesAtThisDepth = MAX_EVAL_MOVES;

    // @todo: Come back to this. I am not sure if this is the best approach.
    // this->movesToEvaluateAtThisDepth = (moveType_t *) malloc(this->numMovesAtThisDepth * sizeof(moveType_t));

    if(this->movesToEvaluateAtThisDepth == NULL)
    {
        std::cout << "Failed to allocate move memory" << std::endl;
        return;        
    }

    // Value is 0 at game start
    this->value = 0;

}

/**
 * ChessBoard constructor which takes in an existing board state
 * 
 * @param *pieces:          The current board position of all pieces
 * @param occupued:         The set of all occupied squares
 * @param numMovesToEval:   Number of moves to evaluate at this depth
 * @param *lastMove:        The move which generated this position
 */
ChessBoard::ChessBoard(uint64_t *pieces, uint64_t occupied, uint64_t numMovesToEval, moveType_t *lastMove)
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

    // Copy the board over
    memcpy((void *) this->pieces, (void*) pieces, (NUM_PIECE_TYPES + 2)*sizeof(uint64_t));

    // @todo: Figure out a way to determine if an occupied bitboard is valid
    this->occupied = occupied;
    this->empty = ~occupied;

    this->numMovesAtThisDepth = numMovesToEval;
    // @todo: Come back to this. I am not sure if this is the best approach.
    // this->movesToEvaluateAtThisDepth = (moveType_t *) malloc(this->numMovesAtThisDepth * sizeof(moveType_t));

    this->lastMove = lastMove;
    this->ApplyMoveToBoard(this->lastMove);


    if(this->movesToEvaluateAtThisDepth == NULL)
    {
        std::cout << "Failed to allocate move memory" << std::endl;
        return;        
    }

}

/**
 * Generates the valid moves for a given chessboard state and color
 * 
 * @param pt:   The color you wish to generate possible moves for
 */
void ChessBoard::GenerateMoves(pieceType_e pt)
{
    if(pt == WHITE_PIECES)
    {
        // Generate possible plays for white
        GeneratePawnMoves(WHITE_PAWN);
        GenerateRookMoves(WHITE_ROOK);
        GenerateBishopMoves(WHITE_BISHOP);
        GenerateKnightMoves(WHITE_KNIGHT);
        GenerateQueenMoves(WHITE_QUEEN);
        GenerateKingMoves(WHITE_KING);
    }
    else if (pt == BLACK_PIECES)
    {
        // Generate possible plays for black
        GeneratePawnMoves(BLACK_PAWN);
        GenerateRookMoves(BLACK_ROOK);
        GenerateBishopMoves(BLACK_BISHOP);
        GenerateKnightMoves(BLACK_KNIGHT);
        GenerateQueenMoves(BLACK_QUEEN);
        GenerateKingMoves(BLACK_KING);
    }
    else
    {
        Util_Assert(0, "Error in input piece type");
    }

    // So now we have all possible moves. We know that our moves are stored
    // as priority queue based on the value of the 

}

/**
 * Applies the current move to the chessboard
 * 
 * @param *moveToApply: The move to apply
 * 
 * @returns: STATUS_SUCESS or STATUS_FAIL
 */
uint64_t ChessBoard::ApplyMoveToBoard(moveType_t *moveToApply)
{

    uint64_t friendlyPieces, enemyPieces;

    if(moveToApply == NULL)
    {
        std::cout << "Move provided to board was NULL!" << std::endl;
        return STATUS_FAIL;
    }

    // We know that any chess move passed to us will have to be
    // generated to comform to the rules of chess, therefore minimal
    // checking should be required...but this is C++ and literally
    // anything can happen with memory overruns so we may as well
    // check for the obvious stuff.

    // 1) Are our indicies valid?
    if(moveToApply->startIdx >= NUM_BOARD_INDICIES
        || moveToApply->endIdx >= NUM_BOARD_INDICIES)
    {
        std::cout << "Move indicies were bad!" << std::endl;
        return STATUS_FAIL;        
    }

    // 2) Is our piecetype actually valid?
    if(moveToApply->pt >= NUM_PIECE_TYPES)
    {
        std::cout << "Piece type provided to board was bad" << std::endl;
        return STATUS_FAIL;
    }

    // 3) Is there actually a piece of this piece type actually at the
    //    start index?
    if(this->pieces[moveToApply->pt] & (1 << moveToApply->startIdx) == 0)
    {
        std::cout << "No piece of piecetype at expexted startIdx" << std::endl;
        return STATUS_FAIL;        
    }
    
    if(moveToApply->pt < (NUM_PIECE_TYPES/2))
    {
        friendlyPieces = WHITE_PIECES;
        enemyPieces = BLACK_PIECES;
    }
    else
    {
        friendlyPieces = BLACK_PIECES;
        enemyPieces = WHITE_PIECES;
    }

    // 4) Is our end index occupied by our color
    if(this->pieces[friendlyPieces] & (1 << moveToApply->endIdx) > 1)
    {
        std::cout << "There was a friendly piece where we wanted to move!" << std::endl;
        return STATUS_FAIL;
    }

    // Apply the move for our piece type
    this->pieces[moveToApply->pt] ^= (1 << moveToApply->startIdx);
    this->pieces[moveToApply->pt] |= (1 << moveToApply->endIdx);

    // Apply the move for our color
    this->pieces[friendlyPieces] ^= (1 << moveToApply->startIdx);
    this->pieces[friendlyPieces] |= (1 << moveToApply->endIdx);

    // If we are taking a piece, clear that square
    this->pieces[enemyPieces] &= ~(1 << moveToApply->endIdx);
    for(uint64_t i = (uint64_t) enemyPieces; i < NUM_PIECE_TYPES/2; ++i)
    {
        this->pieces[(pieceType_e) i] &= ~(1 << moveToApply->endIdx);
    }

    // Ancillary bitboards also need to be updated
    this->occupied &= ~(1 << moveToApply->endIdx);
    this->occupied |= (1 << moveToApply->endIdx);

    // Move is now applied 

    // @todo: Are we attempting to make a move while our king is in check
    //    that doesn't remove our king from check after applying the move

    return STATUS_SUCCESS;

}

/**
 * My general breakdown of move functions would be something along the lines of 
 * 
 * 1) Figure out which squares the piece can move to
 * 2) Does that square contain a friendly piece? If yes, discard and go to next
 *    candidate
 * 3) Allocate and store potential move in moveList
 * 4) Go to next candidate
 **/

void ChessBoard::GeneratePawnMoves(pieceType_e pt)
{
    // Pawns can move forward, or diagonally to strike, or en passant (tricky)
    uint64_t pawn, pawns = this->pieces[pt];   
    moveType_t *lastMove;

    // All your pawns are dead, don't bother
    if(pawns == 0)
    {
        return;
    }

    Util_Assert(pt == WHITE_PAWN || pt == BLACK_PAWN, "Pawn move passed bad piecetype");

    lastMove = this->lastMove;

    if(pt == WHITE_PAWN)
    {

        for(uint64_t i = __builtin_clz(this->pieces[pt]); i < 64; ++i)
        {
            pawn = (1 << i);
        
            // There are no pawns of our color at this location
            if(pawns & pawn == 0)
            {
                continue;
            }

            // So now we have the location of our first available pawn, we need to check moves

            // Move forward one square
            if( i < 56
                && (this->pieces[WHITE_PIECES] & (pawn << 8)) == 0
                && (this->pieces[BLACK_PIECES] & (pawn << 8)) == 0)
            {
                this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 8, MOVE_VALID);
                
                if( i < 16 
                    && (this->pieces[WHITE_PIECES] & (pawn << 8)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn << 8)) == 0)
                {
                    this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 16, MOVE_VALID);
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn << 7)) == 1)
            {
                this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 7, MOVE_VALID);
            }

            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn << 9)) == 1)
            {
                this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 9, MOVE_VALID);
            }

            // En passant check

            // Check if this is the first move
            if(lastMove == NULL)
            {
                continue;
            }

            // Was the last move a black pawn pushing up by two
            if( (lastMove != NULL)
                && (lastMove->pt == BLACK_PAWN )
                && (lastMove->endIdx == lastMove->startIdx - 16))
            {
                // Can we look to the left and did the pawn land there
                if((i % 8 > 0) && ((i - lastMove->endIdx) == 1))
                {
                    this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 7, MOVE_VALID_ATTACK);
                }
                // Can we look to the right and did the pawn land there
                if((i % 8 < 7) && ((lastMove->endIdx - i) == 1))
                {
                    this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 9, MOVE_VALID_ATTACK);
                }          
            }        
        }
    }
    else
    {
        for(uint64_t i = __builtin_ctz(this->pieces[pt]); i < 64; --i)
        {

            pawn = (1 << i);
        
            // There are no pawns of our color at this location
            if(pawns & pawn == 0)
            {
                continue;
            }

            // So now we have the location of our first available pawn, we need to check moves

            // Move forward one square
            if( i >= 8
                && (this->pieces[WHITE_PIECES] & (pawn >> 8)) == 0
                && (this->pieces[BLACK_PIECES] & (pawn >> 8)) == 0)
            {
                this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 8, MOVE_VALID);
                
                if( i >= 56 
                    && (this->pieces[WHITE_PIECES] & (pawn >> 8)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn >> 8)) == 0)
                {
                    this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 16, MOVE_VALID);
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn >> 9)) == 1)
            {
                this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 9, MOVE_VALID_ATTACK);
            }

            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn >> 7)) == 1)
            {
                this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 7, MOVE_VALID_ATTACK);
            }

            // En passant check

            // Check if this is the first move
            if(lastMove == NULL)
            {
                continue;
            }

            // Was the last move a white pawn pushing up by two
            if( (lastMove != NULL)
                && (lastMove->pt == WHITE_PAWN )
                && (lastMove->endIdx == lastMove->startIdx + 16))
            {
                // Can we look to the left and did the pawn land there
                if((i % 8 > 0) && ((i - lastMove->endIdx) == 1))
                {
                    this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 9, MOVE_VALID_ATTACK);
                }
                // Can we look to the right and did the pawn land there
                if((i % 8 < 7) && ((lastMove->endIdx - i) == 1))
                {
                    this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 7, MOVE_VALID_ATTACK);
                }          
            }
        }
    }   
}

void ChessBoard::GenerateRookMoves(pieceType_e pt)
{
    // Rooks can move vertically and horizontally. Logic is mostly unified between color
    moveValidity_e moveVal;
    pieceType_e enemyPieces;
    uint64_t rooks = this->pieces[pt], temp, rookIdx;
    (pt < (NUM_PIECE_TYPES/2)) ? enemyPieces = BLACK_PIECES : enemyPieces = WHITE_PIECES;

    // No rooks left
    if(rooks == 0)
    {
        return;
    }

    // Avenues of attack are up shift 8, down shift 8, right shift 1, left shift 1
    // All of these are until edge

    while(rooks > 0)
    {

        // Get and clear index
        rookIdx = __builtin_clz(rooks);
        rooks ^= (1 << rookIdx);

        temp = rookIdx;
        // left
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx % 8 == 0)
            {
                break;
            }

            temp -= 1;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);

            // Generate move
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp % 8 > 0);

        // right
        temp = rookIdx;
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx % 8 == 7)
            {
                break;
            }

            temp += 1;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);

            // Generate move
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp % 8 < 7);

        // down
        temp = rookIdx;
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx < 8)
            {
                break;
            }

            temp -= 8;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            this->BuildMove(pt, rookIdx, temp, moveVal);

            // Generate move
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp >= 8);

        // up
        temp = rookIdx;
        do
        {
            // on the left side of the board, can't go that way
            if(rookIdx > 54)
            {
                break;
            }

            temp += 8;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            this->BuildMove(pt, rookIdx, temp, moveVal);

            // Generate move
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp <= 54);
    }
}

void ChessBoard::GenerateBishopMoves(pieceType_e pt)
{
    moveValidity_e moveVal;
    pieceType_e enemyPieces;
    uint64_t bishops = this->pieces[pt], bishopIdx, bishop, temp;
    (pt < (NUM_PIECE_TYPES/2)) ? enemyPieces = BLACK_PIECES : enemyPieces = WHITE_PIECES;

    // No bishops left
    if(bishops == 0)
    {
        return;
    }

    while(bishops > 0)
    {
        bishopIdx = __builtin_clz(bishops);
        bishops ^= (1 << bishopIdx);

        // Four directions of movement

        // Down left
        temp = bishopIdx;
        do
        {
            if(temp % 8 == 0 || __builtin_clz(temp) < 8)
            {
                break;
            }

            temp -= 9;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            this->BuildMove(pt, bishopIdx, temp, moveVal);

            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp > 0);

        // Down right
        temp = bishopIdx;
        do
        {
            if(temp % 8 == 7 || __builtin_clz(temp) < 8)
            {
                break;
            }

            temp -= 7;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            this->BuildMove(pt, bishopIdx, temp, moveVal);

            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp > 0);

        // Up left
        temp = bishopIdx;
        do
        {
            if(temp % 8 == 0 || __builtin_clz(temp) >= 54)
            {
                break;
            }

            temp += 7;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            this->BuildMove(pt, bishopIdx, temp, moveVal);

            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp > 0);

        // Up right
        temp = bishopIdx;
        do
        {
            if(temp % 8 == 7 || __builtin_clz(temp) >= 54)
            {
                break;
            }

            temp += 9;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            this->BuildMove(pt, bishopIdx, temp, moveVal);

            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp > 0);
    }
}

/**
 * Generates all available knight moves for a given piece type
 *  
 * @param pt:   The pieceType to generate the knight moves for
 * 
 * @requires    pt to be in {WHITE_KNIGHT, BLACK_KNIGHT}
 */
void ChessBoard::GenerateKnightMoves(pieceType_e pt)
{
    // Knights can move two squares horizontally and one vertically, or
    // two squares vertically and one horizontally.

    // 6 up, 10 up, 15 up, 17 up
    // 6 down, 10 down, 15 down, 17 down

    moveValidity_e moveVal;
    pieceType_e enemyPieces;
    uint64_t knights = this->pieces[pt], knightIdx, temp;
    (pt < (NUM_PIECE_TYPES/2)) ? enemyPieces = BLACK_PIECES : enemyPieces = WHITE_PIECES;

    // No knights left
    if(knights == 0)
    {
        return;
    }

    // The only blocks for moves are from friendly pieces
    while(knights > 0)
    {
        knightIdx = __builtin_clz(knights);
        knights ^= (1 << knightIdx);

        // Assess upwards vertical moves
        if(knightIdx < NUM_BOARD_INDICIES - 16)
        {
            // Can we move left
            if(knightIdx + 15 % 8 < knightIdx % 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 15, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 15, moveVal);
                
            }
            // Can we move right
            if(knightIdx + 17 % 8 > knightIdx % 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 17, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 17, moveVal);
            }
        }

        // Assess downwards vertical moves
        if(knightIdx > 16)
        {
            // Can we move left
            if(knightIdx - 17 % 8 < knightIdx % 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 17, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 17, moveVal);
            }
            // Can we move right
            if(knightIdx + 10 % 8 > knightIdx % 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 15, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 15, moveVal);
            }            
        }

        // Assess left horizontal moves
        if(knightIdx % 8 > 1)
        {
            // Can we move up
            if(knightIdx + 6 < NUM_BOARD_INDICIES)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 6, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 6, moveVal);
            }
            // Can we move down
            if(knightIdx - 10 > 0)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 10, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 10, moveVal);
            }
        }

        // Assess right horizontal moves
        if(knightIdx % 8 < 6)
        {
            // Can we move up
            if(knightIdx + 10 < NUM_BOARD_INDICIES)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 10, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 10, moveVal);
            }
            // Can we move down
            if(knightIdx - 6 > 0)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 6, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 6, moveVal);           
            }
        }
    }
}

/**
 * Generates all possible moves for our queen(s)
 * 
 * @param pt:   The type of queen to generate the move for
 */
void ChessBoard::GenerateQueenMoves(pieceType_e pt)
{
    Util_Assert(pt == WHITE_QUEEN || pt == BLACK_QUEEN, "Queen type provided invalid");

    // The queen can make any move that a rook or bishop can, will actually
    // capture all possible moves if we have multiple queens
    this->GenerateBishopMoves(pt);
    this->GenerateRookMoves(pt);
}

void ChessBoard::GenerateKingMoves(pieceType_e pt)
{
    // While the king has basic movement, it cannot put itself into check,
    // we need an additional guard in place for that. Also castling behavior.

    pieceType_e enemyPieces;
    uint64_t king = this->pieces[pt], temp;
    (pt < (NUM_PIECE_TYPES/2)) ? enemyPieces = BLACK_PIECES : enemyPieces = WHITE_PIECES;

    // Generate moves

    // Iterate through coverage list to check if that square is under threat

}

/**
 * Utility function for determining if a piece can be moved to or attacked on
 * 
 * @param idxToEval:    The index for us to evaluate
 * @param enemyPieces:  The pieceType of the enemy
 * 
 * @return  The validity of the selected move
 */
moveValidity_e ChessBoard::CheckSpaceForMoveOrAttack(uint64_t idxToEval, pieceType_e enemyPieces)
{
    if((1 << idxToEval) & this->occupied == 0)
    {
        return MOVE_VALID;
    }
    else if((1 << idxToEval) & this->pieces[enemyPieces] != 0)
    {
        return MOVE_VALID_ATTACK;
    }   
    return MOVE_INVALID;
}

/**
 * Generates a move given the piece type, the start index, and the end index
 * 
 * @param pt:       The piece you are moving
 * @param startIdx: The start index of the piece you are moving
 * @param endIdx:   Where your piece is going to go
 * 
 * @note:   Assumption at this point is that the move is valid within the 
 *          the rules of chess
 */
void ChessBoard::BuildMove(pieceType_e pt, uint64_t startIdx, uint64_t endIdx, moveValidity_e moveVal)
{

    moveType_t *newMove;

    if(moveVal == MOVE_INVALID)
    {
        return;
    }

    Util_Assert(pt < NUM_PIECE_TYPES, "Invalid piece type for move");

    Util_Assert(startIdx < NUM_BOARD_INDICIES && endIdx < NUM_BOARD_INDICIES
        && startIdx != endIdx, "Invalid indicies provided for move");


    newMove = new moveType_t;
    Util_Assert(newMove != NULL, "Failed to allocate memory for new move!");

    newMove->startIdx = startIdx;
    newMove->endIdx = endIdx;

    // Move string, only for interace with UCI
    newMove->moveString[0] = Util_ConvertPieceTypeToChar(pt);
    newMove->moveString[1] = ((char) (endIdx >> 3)) + '0';
    newMove->moveString[2] = (endIdx % 8);

    // Denote the piece type
    newMove->pt = pt;

    // Denote what type of move this is
    newMove->moveVal = moveVal;
    
    // Generate the resultant board -- value will be calculated
    this->SpawnNextChessBoard(newMove);

    // Add this move to the list of possible moves at this board position
    ChessBoard::AddMoveToMoveList(this, newMove);
}

/**
 * Given add the given move to the movelist of the given chessboard
 * 
 * @param cb        The chessboard to add the move to
 * @param moveToAdd The move to add to the chessboard
 * 
 */
void ChessBoard::AddMoveToMoveList(ChessBoard *cb, moveType_t *moveToAdd)
{
    moveType_t *moveListIdx, *moveListTemp;

    // Confirm our parameters are good
    Util_Assert(cb != NULL, "Gave bad CB to AddMoveToMoveList!");
    Util_Assert(moveToAdd != NULL, "Gave bad move to AddMoveToMoveList");
    Util_Assert(moveToAdd->adjMove == NULL, "Move was already part of another board");

    if(cb->movesToEvaluateAtThisDepth == NULL)
    {
        moveToAdd->adjMove = NULL;
        cb->movesToEvaluateAtThisDepth = moveToAdd;
        return;
    }

    moveListIdx = cb->movesToEvaluateAtThisDepth;

    // We have the newest high value move, place at front
    if(*moveListIdx < *moveToAdd)
    {
        moveToAdd->adjMove = moveListIdx;
        cb->movesToEvaluateAtThisDepth = moveListIdx;
        return;
    }

    // Otherwise traverse until we find our spot
    moveListTemp = moveListIdx;
    moveListIdx = moveListIdx->adjMove;
    while(moveListIdx != NULL && *moveToAdd < *moveListIdx)
    {
        moveListTemp = moveListIdx;
        moveListIdx = moveListIdx->adjMove;
    }

    // We have two situations. First is if we have provided the new lowest value move.

    if(moveListIdx == NULL)
    {
        moveListTemp->adjMove = moveToAdd;
        moveToAdd->adjMove = NULL;
    }
    // Or we have found the point in the movelist where this move should go
    else if(*moveToAdd < *moveListTemp && *moveListIdx < *moveToAdd)
    {
        moveToAdd->adjMove = moveListIdx;
        moveListTemp->adjMove = moveToAdd;
    }
    else
    {
        Util_Assert(0, "Unknown situation on addition of move to move list!");
    }
}

int64_t ChessBoard::EvaluateCurrentBoardValue(ChessBoard *cb)
{
    //@todo
    return 0;
}


void ChessBoard::SpawnNextChessBoard(moveType_t *moveToExecute)
{
    // @todo: Write this function
    Util_Assert(moveToExecute != NULL, "Move provided was NULL");

    ChessBoard *cb = new ChessBoard(this->pieces, this->occupied, this->numMovesAtThisDepth - 10, moveToExecute);
    Util_Assert(cb != NULL, "Failed to allocate new chessboard");

}

/**
 * Algorithm for selecting moves:
 * 
 *  We know: 
 *      -   the the current state of our chessboard after move i-1 C(i)
 *      -   the current value of our chessboard after move i-1 V(i)
 *      -   the possible moves for our position after move i-1 M(i)
 *              - each move denoted by m_j s.t. m_j is in M(i)
 * 
 *  We know that we have a list of possible moves available to use
 *      -   We at this point have made no determination about whether
 *          or not that move is any good
 *      -   We now want to evaluate all of our moves in M(i)
 * 
 *  For a basic evaluation of a given move, we need to consider what
 *  our board value will be at the start of our next move, or after our
 *  opponent has moved. This can be represented as V(i+2):
 * 
 *      Value(m_j) = V(i+2) - V(i) 
 * 
 *  For a general search depth of n moves into the future, this is:
 * 
 *      Value(m_j) = V(i+n) - V(i)
 * 
 *  Where generally:
 *  
 *      V(i) = V(i-1) + Value(m_prev)
 * 
 *  Which leads to the selection of our "best" move:
 * 
 *  m_next  = MAX_(m_j in M(i)) { Value(m_j) }
 *          = MAX_(m_j in M(i)) { V(i+n) - V(i) }
 * 
 *  Bringing in the recurrence:
 * 
 *  m_next = MAX_(m_j in M(i)) { V(i+n-1) + Value(m_(n-1)) - V(i) }
 *  m_next = MAX_(m_j in M(i)) { V(i+n-2) + Value(m_(n-2)) + Value(m_(n-1)) - V(i) }
 *  ...
 *  m_next = MAX_(m_j in M(i)) { SUM_{k=j}{j+n-1}( Value(m_(k)) ) }
 * 
 *  But value will be negative if our opponent is moving:
 * 
 *      Let X(i) be an objective function s.t. X(k) = 1     iff k-j % 2 == 0
 *                                                  = -1    otherwise
 * 
 *  m_next = MAX_(m_j in M(i)) { SUM_{k=j}{j+n-1}( Value(m_(k))X(k) ) }
 * 
 *  Obviously the move space will be exponential, so we want to
 *  prune this search tree as we go through it. 
 * 
 *  Let's ballpark this to 5 move future search to start:
 * 
 *      |M(i)|   = ||
 *      |M(i+1)| = 30
 *      |M(i+2)| = 20
 *      |M(i+3)| = 10
 *      |M(i+4)| = 5
 * 
 *      That is 1.2 million possible moves (lol)
 */


moveType_t *ChessBoard::GetNextMove(pieceType_e pt)
{
    // Sort moves in descending order by value of the move
    
    // For the number of moves we are supposed to evaluate at this depth,
    // sort by current value

    // Update final value with result of search

    return NULL;
}

uint64_t InitializeChessBoard(void)
{
    cb = new ChessBoard();
    
    if(cb == NULL)
    {
        return 1;
    }
    return 0;

}
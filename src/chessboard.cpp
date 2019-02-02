#include <iostream>
#include "util.h"
#include "chessboard.h"

static ChessBoard *cb;
static moveType_t *globalBestMove;

static const uint64_t movesAtSearchDepth[] = 
{  
    40,
    30,
    20,
    10,
    5,
    1
};
#define SEARCH_DEPTH sizeof(movesAtSearchDepth) / sizeof(uint64_t)

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

    this->searchDepth = 0;

    // Value is 0 at game start
    this->value = 0;

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

    // Copy the board over
    //memcpy((void *) this->pieces, (void*) pieces, (NUM_PIECE_TYPES + 2)*sizeof(uint64_t));

    // @todo: Figure out a way to determine if an occupied bitboard is valid
    //this->occupied = occupied;
    //this->empty = ~occupied;

    //this->searchDepth = searchDepth;

    //this->lastMove = lastMove;
    //@todo: Make function static
    //this->ApplyMoveToBoard(this->lastMove);

    ChessBoard::EvaluateCurrentBoardValue(this);

}

/**
 * Generates the valid moves for a given chessboard state and color
 * 
 * @param pt:   The color you wish to generate possible moves for
 *
 * @return      The list of moves available at this location
 */
moveType_t *ChessBoard::GenerateMoves(uint8_t pt)
{
    moveType_t *moveList;
    uint8_t nextPt;
    uint64_t i = 1;

    if(pt == WHITE_PIECES)
    {
        nextPt = BLACK_PIECES;

        // Generate possible plays for white
        GeneratePawnMoves(WHITE_PAWN, &moveList);
        GenerateRookMoves(WHITE_ROOK, &moveList);
        GenerateBishopMoves(WHITE_BISHOP, &moveList);
        GenerateKnightMoves(WHITE_KNIGHT, &moveList);
        GenerateQueenMoves(WHITE_QUEEN, &moveList);
        GenerateKingMoves(WHITE_KING, &moveList);
    }
    else if (pt == BLACK_PIECES)
    {
        nextPt = WHITE_PIECES;

        // Generate possible plays for black
        GeneratePawnMoves(BLACK_PAWN, &moveList);
        GenerateRookMoves(BLACK_ROOK, &moveList);
        GenerateBishopMoves(BLACK_BISHOP, &moveList);
        GenerateKnightMoves(BLACK_KNIGHT, &moveList);
        GenerateQueenMoves(BLACK_QUEEN, &moveList);
        GenerateKingMoves(BLACK_KING, &moveList);
    }
    else
    {
        Util_Assert(0, "Error in input piece type");
    }

    return moveList;

    // So now we have all possible moves. We know that our moves are stored
    // as a linked list with the best moves already at the front. We want
    // to free up the moves & boards of all moves we do not want to use moving
    // forward at this depth. It almost certainly better to free up as much
    // in-use memory as possible prior to executing the next level of our DFS

    // If we have any illegal moves at the start, eliminate them
/*     moveListTemp = this->movesToEvaluateAtThisDepth;
    while(moveListTemp != NULL && !moveListTemp->legalMove)
    {
        moveListTemp = this->movesToEvaluateAtThisDepth->adjMove;
        DeleteMove(this->movesToEvaluateAtThisDepth);
        this->movesToEvaluateAtThisDepth = moveListTemp;
    }
    // Our firstMove is now confirmed legal, if not NULL
    if(moveListTemp != NULL)
    {
        moveListIdx = moveListTemp->adjMove;
        while(moveListIdx != NULL && i < movesAtSearchDepth[this->searchDepth])
        {
            // Illegal move detected -> we put ourself in checkmate
            if(!moveListIdx->legalMove)
            {
                moveListTemp->adjMove = moveListIdx->adjMove;
                DeleteMove(moveListIdx);
                // Advance two spaces if we need to pull a move, but do not count against search depth limit
                moveListIdx = moveListTemp->adjMove->adjMove;
            }
            else
            {
                moveListIdx = moveListIdx->adjMove;
                i++;
            }
        }

        moveListTemp->adjMove = NULL;

        // We know we have moves to delete if this is true
        if(moveListIdx != NULL && i == movesAtSearchDepth[this->searchDepth])
        {
            // So delete the moves
            while(moveListIdx != NULL){

                moveListTemp = moveListIdx->adjMove;
                ChessBoard::DeleteMove(moveListIdx);
                moveListIdx = moveListTemp;
            }
        }

        moveListIdx = this->movesToEvaluateAtThisDepth;

        // We are the end of our DFS and we have found a new best value
        if(movesAtSearchDepth[this->searchDepth] == 1 && *globalBestMove < *moveListIdx)
        {   
            DeleteMove(globalBestMove);
            globalBestMove = moveListIdx;
        }

        while(moveListIdx != NULL && i < movesAtSearchDepth[this->searchDepth])
        {
            Util_Assert(moveListIdx->resultCB != NULL, "Move had null Chessboard!");
            
            // This will create all moves for the next chessboard
            moveListIdx->resultCB->GenerateMoves(nextPt);
            moveListIdx = moveListIdx->adjMove;
            i++;
        }
    } */

    // Board is not needed
    //Add board to the deletion list
    //delete this;
}

/**
 * Determines the next best move via a minimax searc algorithm.
 * 
 * @param depth             The search depth to look
 * @param playerToMaximize  If we are attempting to maximize or minimize score
 *                          for this search depth
 * 
 * @return                  The score associated with the best move for this search
 *                          depth
 * 
 * @note                    At search depth 0, the move with the best score will be 
 *                          placed in a variable of the board globalBestMove;
 */
int64_t ChessBoard::GetBestMove(uint64_t depth, bool playerToMaximize, moveType_t *movesToEvaluateAtThisDepth)
{
    int64_t score, savedScore;
    moveType_t *moveToEvaluate, *movesToEvaluateAtNextDepth;

    if(depth == 0)
    {
        return -EvaluateCurrentBoardValue(this);
    }

    moveToEvaluate = movesToEvaluateAtThisDepth;
    if(playerToMaximize)
    {
        score = -1000000000000000;
        while(moveToEvaluate != NULL)
        {
            this->ApplyMoveToBoard(moveToEvaluate);
            movesToEvaluateAtNextDepth = this->GenerateMoves(WHITE_PIECES);
            savedScore = score;
            score = std::max(score, this->GetBestMove(depth - 1, !playerToMaximize, movesToEvaluateAtNextDepth));
            if(depth == SEARCH_DEPTH)
            {
                globalBestMove = moveToEvaluate;
            }
            //this->undoMove();
        }
    }
    else
    {
        score = 1000000000000000;
        while(moveToEvaluate != NULL)
        {
            this->ApplyMoveToBoard(moveToEvaluate);
            movesToEvaluateAtNextDepth = this->GenerateMoves(BLACK_PIECES);
            savedScore = score;
            score = std::min(score, this->GetBestMove(depth - 1, !playerToMaximize, movesToEvaluateAtNextDepth));
            if(depth == SEARCH_DEPTH && score < savedScore)
            {
                globalBestMove = moveToEvaluate;
            }
            //this->undoMove();
        }
    }
    return score;
}


/**
 * Runs through the deletion procedure for a given move
 * 
 * @param moveToDelete  The move to delete
 */
void ChessBoard::DeleteMove(moveType_t *moveToDelete)
{
    Util_Assert(moveToDelete != NULL, "Move to delete was already NULL");

    //@todo implement this

    return;
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
        this->pieces[(uint8_t) i] &= ~(1 << moveToApply->endIdx);
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

void ChessBoard::GeneratePawnMoves(uint8_t pt, moveType_t **moveList)
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

    //lastMove = GetLastMove();

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
                this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 8, MOVE_VALID, moveList);
                
                if( i < 16 
                    && (this->pieces[WHITE_PIECES] & (pawn << 8)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn << 8)) == 0)
                {
                    this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 16, MOVE_VALID, moveList);
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn << 7)) == 1)
            {
                this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 7, MOVE_VALID, moveList);
            }

            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn << 9)) == 1)
            {
                this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 9, MOVE_VALID, moveList), moveList;
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
                    this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 7, MOVE_VALID_ATTACK, moveList);
                }
                // Can we look to the right and did the pawn land there
                if((i % 8 < 7) && ((lastMove->endIdx - i) == 1))
                {
                    this->BuildMove(WHITE_PAWN, __builtin_clz(pawn), i + 9, MOVE_VALID_ATTACK, moveList);
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
                this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 8, MOVE_VALID, moveList);
                
                if( i >= 56 
                    && (this->pieces[WHITE_PIECES] & (pawn >> 8)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn >> 8)) == 0)
                {
                    this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 16, MOVE_VALID, moveList);
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn >> 9)) == 1)
            {
                this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 9, MOVE_VALID_ATTACK, moveList);
            }

            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn >> 7)) == 1)
            {
                this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 7, MOVE_VALID_ATTACK, moveList);
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
                    this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 9, MOVE_VALID_ATTACK, moveList);
                }
                // Can we look to the right and did the pawn land there
                if((i % 8 < 7) && ((lastMove->endIdx - i) == 1))
                {
                    this->BuildMove(BLACK_PAWN, __builtin_clz(pawn), i - 7, MOVE_VALID_ATTACK, moveList);
                }          
            }
        }
    }   
}

void ChessBoard::GenerateRookMoves(uint8_t pt, moveType_t **moveList)
{
    // Rooks can move vertically and horizontally. Logic is mostly unified between color
    uint8_t moveVal;
    uint8_t enemyPieces;
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
            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);

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
            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);

            // Generate move
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp <= 54);
    }
}

void ChessBoard::GenerateBishopMoves(uint8_t pt, moveType_t **moveList)
{
    uint8_t moveVal;
    uint8_t enemyPieces;
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
            this->BuildMove(pt, bishopIdx, temp, moveVal, moveList);

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
            this->BuildMove(pt, bishopIdx, temp, moveVal, moveList);

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
            this->BuildMove(pt, bishopIdx, temp, moveVal, moveList);

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
            this->BuildMove(pt, bishopIdx, temp, moveVal, moveList);

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
void ChessBoard::GenerateKnightMoves(uint8_t pt, moveType_t **moveList)
{
    // Knights can move two squares horizontally and one vertically, or
    // two squares vertically and one horizontally.

    // 6 up, 10 up, 15 up, 17 up
    // 6 down, 10 down, 15 down, 17 down

    uint8_t moveVal;
    uint8_t enemyPieces;
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
                this->BuildMove(pt, knightIdx, knightIdx + 15, moveVal, moveList);
                
            }
            // Can we move right
            if(knightIdx + 17 % 8 > knightIdx % 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 17, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 17, moveVal, moveList);
            }
        }

        // Assess downwards vertical moves
        if(knightIdx > 16)
        {
            // Can we move left
            if(knightIdx - 17 % 8 < knightIdx % 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 17, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 17, moveVal, moveList);
            }
            // Can we move right
            if(knightIdx + 10 % 8 > knightIdx % 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 15, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 15, moveVal, moveList);
            }            
        }

        // Assess left horizontal moves
        if(knightIdx % 8 > 1)
        {
            // Can we move up
            if(knightIdx + 6 < NUM_BOARD_INDICIES)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 6, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 6, moveVal, moveList);
            }
            // Can we move down
            if(knightIdx - 10 > 0)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 10, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 10, moveVal, moveList);
            }
        }

        // Assess right horizontal moves
        if(knightIdx % 8 < 6)
        {
            // Can we move up
            if(knightIdx + 10 < NUM_BOARD_INDICIES)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 10, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 10, moveVal, moveList);
            }
            // Can we move down
            if(knightIdx - 6 > 0)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 6, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 6, moveVal, moveList);           
            }
        }
    }
}

/**
 * Generates all possible moves for our queen(s)
 * 
 * @param pt:   The type of queen to generate the move for
 */
void ChessBoard::GenerateQueenMoves(uint8_t pt, moveType_t **moveList)
{
    Util_Assert(pt == WHITE_QUEEN || pt == BLACK_QUEEN, "Queen type provided invalid");

    // The queen can make any move that a rook or bishop can, will actually
    // capture all possible moves if we have multiple queens
    this->GenerateBishopMoves(pt, moveList);
    this->GenerateRookMoves(pt, moveList);
}

void ChessBoard::GenerateKingMoves(uint8_t pt, moveType_t **moveList)
{
    // While the king has basic movement, it cannot put itself into check,
    // we need an additional guard in place for that. Also castling behavior.

    uint8_t enemyPieces;
    uint64_t king = this->pieces[pt], temp;
    (pt < (NUM_PIECE_TYPES/2)) ? enemyPieces = BLACK_PIECES : enemyPieces = WHITE_PIECES;

    // This is where we get slightly tricky. The king cannot move into a space under threat
    // What this implies is that we need to know every single space which is under threat
    // by the enemy. This includes spaces we can move into freely, as well as captures.
    // Therefore, we need to generate a list of squares under threat at any time, regardless
    // of what piece type has done this.    

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
uint8_t ChessBoard::CheckSpaceForMoveOrAttack(uint64_t idxToEval, uint8_t enemyPieces)
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
 * @param pt:           The piece you are moving
 * @param startIdx:     The start index of the piece you are moving
 * @param endIdx:       Where your piece is going to go
 * @param moveVal:      The type of move you are executing
 * @param moveList      The current movelist
 * 
 * @note:   Assumption at this point is that the move is valid within the 
 *          the rules of chess
 */
void ChessBoard::BuildMove(uint8_t pt, uint8_t startIdx, uint8_t endIdx, uint8_t moveVal, moveType_t **moveList)
{

    moveType_t *newMove, *lastMove;
    bool legalMove = true;

    if(moveVal == MOVE_INVALID)
    {
        return;
    }

    Util_Assert(pt < NUM_PIECE_TYPES, "Invalid piece type for move");

    Util_Assert(startIdx < NUM_BOARD_INDICIES && endIdx < NUM_BOARD_INDICIES
        && startIdx != endIdx, "Invalid indicies provided for move");

    // Now for an interesting quirk. If we have detected that we can actually directly attack
    // the king of our enemy, we actually know that the previous move that got us here is
    // actually illegal under the rules of chess, and MUST be discounted. We can actually
    // check for this rather simply. They key is that checkmate must be detected from 
    // our turn, not the response move

    if((pt < NUM_PIECE_TYPES/2 && (1 << endIdx) & this->pieces[BLACK_KING] != 0)
        || (pt >= NUM_PIECE_TYPES/2 && (1 << endIdx) & this->pieces[WHITE_KING] != 0))
    {
        legalMove = false;
    }

    newMove = new moveType_t;
    Util_Assert(newMove != NULL, "Failed to allocate memory for new move!");

    newMove->startIdx = startIdx;
    newMove->endIdx = endIdx;

    // Move string, only for interace with UCI
    //newMove->moveString[0] = Util_ConvertPieceTypeToChar(pt);
    //newMove->moveString[1] = ((char) (endIdx >> 3)) + '0';
    //newMove->moveString[2] = (endIdx % 8);

    // Denote the piece type
    newMove->pt = pt;

    // Denote what type of move this is
    newMove->moveVal = moveVal;

    newMove->legalMove = legalMove;

    if(legalMove)
    {
        // Add this move to the list of possible moves at this board position
        newMove->adjMove = *moveList;
        *moveList = newMove;

    }
    else
    {
        //lastMove = GetLastMove();
        //if(lastMove == NULL)
        //{

        //}

        // If this move was illegal, then the last move HAS to be illegal
        //lastMove = GetLastMove;
        lastMove->legalMove = false;
    }
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

    // Confirm our parameters are good
    Util_Assert(cb != NULL, "Gave bad CB to AddMoveToMoveList!");
    Util_Assert(moveToAdd != NULL, "Gave bad move to AddMoveToMoveList");
    Util_Assert(moveToAdd->adjMove == NULL, "Move was already part of another board");

/*     if(cb->movesToEvaluateAtThisDepth == NULL)
    {
        moveToAdd->adjMove = NULL;
        cb->movesToEvaluateAtThisDepth = moveToAdd;
        return;
    }

    moveToAdd->adjMove = cb->movesToEvaluateAtThisDepth;
    cb->movesToEvaluateAtThisDepth = moveToAdd;
 */

    // We have the newest high value move, place at front
    /* if(*moveListIdx < *moveToAdd)
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
    } */
}

int64_t ChessBoard::EvaluateCurrentBoardValue(ChessBoard *cb)
{
    uint64_t idx, pieces;
    int64_t value = 0;
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
        if(idx <= NUM_PIECE_TYPES / 2 - 1)
        {
            while(pieces != 0)
            {
                value += GetPositionValueFromTable(idx & 6, __builtin_clz(pieces));
                pieces ^= __builtin_clz(pieces);
            }
        }
        else
        {
            while(pieces != 0)
            {
                value -= GetPositionValueFromTable(idx & 6, __builtin_ctz(pieces));
                pieces ^= __builtin_ctz(pieces);
            }
        }
    }
    return value;
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
 *      |M(i)|   = 40
 *      |M(i+1)| = 30
 *      |M(i+2)| = 20
 *      |M(i+3)| = 10
 *      |M(i+4)| = 5
 * 
 *      That is 1.2 million possible moves (lol)
 */

moveType_t *ChessBoard::GetNextMove(uint8_t pt)
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
#include <iostream>
#include <mutex>
#include "util.h"
#include "chessboard.h"

static ChessBoard *cb;
static moveType_t *globalBestMove;

#define SEARCH_DEPTH 1

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
        this->occupied |= this->pieces[pt];
    }

    this->occupied = BOARD_START_USED;
    this->empty = BOARD_START_EMPTY;

    this->searchDepth = 0;

    // Value is 0 at game start
    this->value = 0;

    // It is not possible for two subsequent chessboard states
    // to have the exact same bits set, so we can use this to
    // denote if we are on the first move
    this->prevOccupied = this->occupied;

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
 * Generates the valid moves for a given chessboard state and color
 * 
 * @param pt:   The color you wish to generate possible moves for
 *
 * @return      The list of moves available at this location
 */
moveType_t *ChessBoard::GenerateMoves(uint8_t pt)
{
    uint8_t nextPt;
    uint64_t i = 1;
    moveType_t *moveList = new moveType_t;

    // Use a known invalid move as a end marker
    Util_Assert(moveList != NULL, "Failed to allocate memory for a move");
    moveList->legalMove = false;

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
}

static uint64_t numMoves = 0;

/**
 * Determines the next best move via a minimax search algorithm.
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
    moveType_t *moveToEvaluate, *movesToEvaluateAtNextDepth, *tempMove;
    bool evaluationNeeded = depth > 1;

    std::cout << "Assessing depth: " << depth << " for " << playerToMaximize << std::endl;
    if(depth == 0)
    {
        return -EvaluateCurrentBoardValue(this);
    }

    moveToEvaluate = movesToEvaluateAtThisDepth;
    if(playerToMaximize)
    {
        score = -1000000000000000;
        while(moveToEvaluate != NULL && moveToEvaluate->legalMove)
        {
            numMoves++;
            this->ApplyMoveToBoard(moveToEvaluate);
            
            // We only need to evaluate moves if we have at least 2 to go
            if(evaluationNeeded)
            {
                movesToEvaluateAtNextDepth = this->GenerateMoves(BLACK_PIECES);
            }

            savedScore = score;
            score = std::max(score, this->GetBestMove(depth - 1, !playerToMaximize, movesToEvaluateAtNextDepth));
            if(depth == SEARCH_DEPTH)
            {
                globalBestMove = moveToEvaluate;
            }
            this->UndoMoveFromBoard();
            tempMove = moveToEvaluate->adjMove;

            //delete moveToEvaluate;
            moveToEvaluate = tempMove;
        }
    }
    else
    {
        score = 1000000000000000;
        while(moveToEvaluate != NULL && moveToEvaluate->legalMove)
        {
            numMoves++;
            this->ApplyMoveToBoard(moveToEvaluate);
            
            // We only need to evaluate moves if we have at least 2 to go
            if(evaluationNeeded)
            {
                movesToEvaluateAtNextDepth = this->GenerateMoves(WHITE_PIECES);
            }
            
            savedScore = score;
            score = std::min(score, this->GetBestMove(depth - 1, !playerToMaximize, movesToEvaluateAtNextDepth));
            if(depth == SEARCH_DEPTH && score < savedScore)
            {
                globalBestMove = moveToEvaluate;
            }

            this->UndoMoveFromBoard();
            tempMove = moveToEvaluate->adjMove;
            
            //delete moveToEvaluate;
            moveToEvaluate = tempMove;
        }
    }
    return score;
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

    uint8_t friendlyPieces, enemyPieces, friendlyStart, enemyStart;

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
    if(this->pieces[moveToApply->pt] & ((uint64_t) 1 << moveToApply->startIdx) == 0)
    {
        std::cout << "No piece of piecetype at expexted startIdx" << std::endl;
        return STATUS_FAIL;        
    }

    if(moveToApply->pt < (NUM_PIECE_TYPES/2))
    {
        friendlyPieces = WHITE_PIECES;
        friendlyStart = WHITE_PAWN;
        enemyPieces = BLACK_PIECES;
        enemyStart = BLACK_PAWN;
    }
    else
    {
        friendlyPieces = BLACK_PIECES;
        friendlyStart = BLACK_PAWN;
        enemyPieces = WHITE_PIECES;
        enemyStart = WHITE_PAWN;
    }

    // 4) Is our end index occupied by our color
    if((this->pieces[friendlyPieces] & ((uint64_t) 1 << moveToApply->endIdx)) > 1)
    {
        std::cout << "There was a friendly piece where we wanted to move!" << std::endl;
        return STATUS_FAIL;
    }

    memcpy(this->prevPieces, this->pieces, sizeof(uint64_t) * (NUM_PIECE_TYPES + 2));
    this->prevEmpty = this->empty;
    this->prevOccupied = this->occupied;

    // Apply the move for our piece type
    this->pieces[moveToApply->pt] ^= ((uint64_t) 1 << moveToApply->startIdx);
    this->pieces[moveToApply->pt] |= ((uint64_t) 1 << moveToApply->endIdx);

    // Apply the move for our color
    this->pieces[friendlyPieces] ^= ((uint64_t) 1 << moveToApply->startIdx);
    this->pieces[friendlyPieces] |= ((uint64_t) 1 << moveToApply->endIdx);

    // If we are taking a piece, clear that square
    this->pieces[enemyPieces] &= ~((uint64_t) 1 << moveToApply->endIdx);
    for(uint8_t i = enemyStart; i < enemyStart + NUM_PIECE_TYPES/2; ++i) // @todo bug here find it
    {
        this->pieces[i] &= ~((uint64_t) 1 << moveToApply->endIdx);
    }

    // Ancillary bitboards also need to be updated
    this->occupied &= ~((uint64_t) 1 << moveToApply->startIdx);
    this->occupied |= ((uint64_t) 1 << moveToApply->endIdx);

    // Move is now applied 

    // @todo: Are we attempting to make a move while our king is in check
    //    that doesn't remove our king from check after applying the move

    return STATUS_SUCCESS;

}

/**
 * Removes the last move applied to this chessboard.
 * 
 * @returns STATUS_SUCCESS if successful, STATUS_FAIL otherwise
 */
uint64_t ChessBoard::UndoMoveFromBoard(void)
{
    if(this->occupied == this->prevOccupied)
    {
        std::cout << "Tried to undo first move from board!" << std::endl;
        return STATUS_FAIL;
    }

    memcpy(this->pieces, this->prevPieces, sizeof(uint64_t) * (NUM_PIECE_TYPES + 2));
    this->empty = this->prevEmpty;
    this->occupied = this->prevOccupied;
    this->value = this->prevValue;

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
    uint64_t i, pawn, pawns = this->pieces[pt];   
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
        while(pawns != 0)
        {
            i = __builtin_ctzll(pawns);
            pawn = ((uint64_t) 1 << i);
            pawns ^= pawn;

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
                this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 8, MOVE_VALID, moveList);
                
                if( i < 16 
                    && (this->pieces[WHITE_PIECES] & (pawn << 8)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn << 8)) == 0)
                {
                    this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 16, MOVE_VALID, moveList);
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn << 7)) == 1)
            {
                this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 7, MOVE_VALID, moveList);
            }

            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn << 9)) == 1)
            {
                this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 9, MOVE_VALID, moveList), moveList;
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
                    this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 7, MOVE_VALID_ATTACK, moveList);
                }
                // Can we look to the right and did the pawn land there
                if((i % 8 < 7) && ((lastMove->endIdx - i) == 1))
                {
                    this->BuildMove(WHITE_PAWN, __builtin_ctzll(pawn), i + 9, MOVE_VALID_ATTACK, moveList);
                }          
            }        
        }
    }
    else
    {
        while(pawns != 0)
        {
            i = __builtin_ctzll(pawns);
            pawn = ((uint64_t) 1 << i);
            pawns ^= pawn;
        
            // Move forward one square
            if( i >= 8
                && ((this->occupied & (pawn >> 8)) == 0))
            {
                this->BuildMove(BLACK_PAWN, i, i - 8, MOVE_VALID, moveList);
                
                if( i >= 48 
                    && (this->pieces[WHITE_PIECES] & (pawn >> 8)) == 0
                    && (this->pieces[BLACK_PIECES] & (pawn >> 8)) == 0)
                {
                    this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 16, MOVE_VALID, moveList);
                }
            } 

            // Move left-diagonal to attack
            if( (i % 8 != 7) && (this->pieces[BLACK_PIECES] & (pawn >> 9)) == 1)
            {
                this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 9, MOVE_VALID_ATTACK, moveList);
            }

            if( (i % 8 != 0) && (this->pieces[BLACK_PIECES] & (pawn >> 7)) == 1)
            {
                this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 7, MOVE_VALID_ATTACK, moveList);
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
                    this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 9, MOVE_VALID_ATTACK, moveList);
                }
                // Can we look to the right and did the pawn land there
                if((i % 8 < 7) && ((lastMove->endIdx - i) == 1))
                {
                    this->BuildMove(BLACK_PAWN, __builtin_ctzll(pawn), i - 7, MOVE_VALID_ATTACK, moveList);
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
        rookIdx = __builtin_ctzll(rooks);
        rooks ^= ((uint64_t) 1 << rookIdx);

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
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);
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
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);
            if(moveVal == MOVE_VALID_ATTACK)
            {
                break;
            }
        } while(temp % 8 < 7);

        // down
        temp = rookIdx;
        do
        {
            // Can we go down
            if(rookIdx < 8)
            {
                break;
            }

            temp -= 8;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);
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
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

            this->BuildMove(pt, rookIdx, temp, moveVal, moveList);
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
        bishopIdx = __builtin_ctzll(bishops);
        bishops ^= ((uint64_t) 1 << bishopIdx);

        // Four directions of movement

        // Down left
        temp = bishopIdx;
        do
        {
            if(temp % 8 == 0 || temp < 8)
            {
                break;
            }

            temp -= 9;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

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
            if(temp % 8 == 7 || temp < 8)
            {
                break;
            }

            temp -= 7;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

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
            if(temp % 8 == 0 || temp >= 54)
            {
                break;
            }

            temp += 7;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

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
            if(temp % 8 == 7 || temp >= 54)
            {
                break;
            }

            temp += 9;
            moveVal = this->CheckSpaceForMoveOrAttack(temp, enemyPieces);
            if(moveVal == MOVE_INVALID)
            {
                break;
            }

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
        knightIdx = __builtin_ctzll(knights);
        knights ^= ((uint64_t) 1 << knightIdx);

        // Assess upwards vertical moves
        if(knightIdx < NUM_BOARD_INDICIES - 16)
        {
            // Can we move left
            if(knightIdx % 8 > 0)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 15, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 15, moveVal, moveList);
                
            }
            // Can we move right
            if(knightIdx % 8 < 7)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 17, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 17, moveVal, moveList);
            }
        }

        // Assess downwards vertical moves
        if(knightIdx > 16)
        {
            // Can we move left
            if(knightIdx % 8 > 0)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 17, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 17, moveVal, moveList);
            }
            // Can we move right
            if(knightIdx % 8 < 7)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 15, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 15, moveVal, moveList);
            }            
        }

        // Assess left horizontal moves
        if(knightIdx % 8 > 1)
        {
            // Can we move up
            if(knightIdx < 54)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 6, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 6, moveVal, moveList);
            }
            // Can we move down
            if(knightIdx >= 8)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx - 10, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx - 10, moveVal, moveList);
            }
        }

        // Assess right horizontal moves
        if(knightIdx % 8 < 6)
        {
            // Can we move up
            if(knightIdx < 54)
            {
                moveVal = this->CheckSpaceForMoveOrAttack(knightIdx + 10, enemyPieces);
                this->BuildMove(pt, knightIdx, knightIdx + 10, moveVal, moveList);
            }
            // Can we move down
            if(knightIdx >= 8)
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
    if((((uint64_t) 1 << idxToEval) & this->occupied) == 0)
    {
        return MOVE_VALID;
    }
    else if((((uint64_t) 1 << idxToEval) & this->pieces[enemyPieces]) != 0)
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

    //if((pt < NUM_PIECE_TYPES/2 && (((uint64_t) 1 << endIdx) & this->pieces[BLACK_KING] != 0))
    //    || (pt >= NUM_PIECE_TYPES/2 && (((uint64_t) 1 << endIdx) & this->pieces[WHITE_KING] != 0)))
    //{
    //    legalMove = false;
    //}

    newMove = new moveType_t;
    Util_Assert(newMove != NULL, "Failed to allocate memory for new move!");

    newMove->startIdx = startIdx;
    newMove->endIdx = endIdx;

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

bool ChessBoard::IsValidRookMove(ChessBoard *cb, uint8_t idxToFind, uint8_t endIdx)
{
    uint8_t mask, largerIdx, smallerIdx;

    if(cb == NULL)
    {
        return false;
    }

    if(endIdx > idxToFind)
    {
        largerIdx = endIdx;
        smallerIdx = idxToFind;
    }
    else
    {
        largerIdx = idxToFind;
        smallerIdx = endIdx;    
    }

    // IAre we on the same rank or file AND are the indicies between these two
    // locations empty            
    if(endIdx % 8 == idxToFind % 8) // same column
    {
        // Now we have the column values for our occupied squares
        mask = cb->occupied & (COLUMN_MASK << endIdx % 8);
        mask &= ((COLUMN_MASK << endIdx % 8) >> 8*(8 - largerIdx/8));
        mask &= ~((COLUMN_MASK << endIdx % 8) >> 8*(8 - smallerIdx/8 + 1));

        // if our mask == 0 then the move is valid
        return (mask == 0);
    }
    else if(endIdx >> 3 == idxToFind >> 3)
    {
        // Now we have the row masked
        mask = cb->occupied & (0xFF << (endIdx/8));
        mask &= ~(BOARD_MASK << largerIdx);
        mask &= ~(BOARD_MASK >> (63 - smallerIdx));

        return (mask == 0);
    }

    return false;
}

bool ChessBoard::IsValidKnightMove(uint8_t idxToFind, uint8_t endIdx)
{
    if(endIdx > idxToFind)
    {
        // Assess upwards vertical moves
        if(idxToFind < NUM_BOARD_INDICIES - 16)
        {
            // Can we up then left or right
            if((idxToFind + 15 % 8 < idxToFind % 8 && idxToFind + 15 == endIdx)
                || (idxToFind + 17 % 8 > idxToFind % 8 && idxToFind + 17 == endIdx))
            {
                return true;
            }
        }
        // Assess horizontal upwards moves
        if(idxToFind < NUM_BOARD_INDICIES - 8)
        {
            // Can we move left then up or right then up
            if((idxToFind % 8 >= 2 && idxToFind + 6 < NUM_BOARD_INDICIES && idxToFind + 6 == endIdx)
                || (idxToFind % 8 < 6 && idxToFind + 10 < NUM_BOARD_INDICIES && idxToFind + 10 == endIdx))
            {
                return true;
            }
        }
    }
    else if(idxToFind > endIdx)
    {
        // Assess downwards vertical moves
        if(idxToFind >= 16)
        {
            // Can we down then left or right
            if((idxToFind - 15 % 8 < idxToFind % 8 && idxToFind - 15 == endIdx)
                || (idxToFind - 17 % 8 > idxToFind % 8 && idxToFind - 17 == endIdx))
            {
                return true;
            }
        }
        // Assess horizontal downwards moves
        if(idxToFind >= 8)
        {
            // Can we move left then down or right then down
            if((idxToFind % 8 >= 2 && idxToFind - 6 < NUM_BOARD_INDICIES && idxToFind - 6 == endIdx)
                || (idxToFind % 8 < 6 && idxToFind - 10 < NUM_BOARD_INDICIES && idxToFind - 10 == endIdx))
            {
                return true;
            }
        }
    }
    return false;
}

bool ChessBoard::IsValidBishopMove(ChessBoard *cb, uint8_t idxToFind, uint8_t endIdx)
{
    int8_t temp;
    uint8_t largerIdx, smallerIdx;

    if(cb == NULL)
    {
        return false;
    }

    if(endIdx > idxToFind)
    {
        largerIdx = endIdx;
        smallerIdx = idxToFind;
    }
    else
    {
        largerIdx = idxToFind;
        smallerIdx = endIdx;    
    }

    // Are the start and end indicies on the same diagonal
    if(largerIdx - smallerIdx % 7 == 0)
    {
        temp = 7;
    }
    else if(largerIdx - smallerIdx & 9 == 0)
    {
        temp = 9;
    }
    else
    {
        return false;
    }
    smallerIdx += temp;
    // This will traverse the diagonal and search for impediments
    while(smallerIdx != largerIdx)
    {
        // There is something in the way
        if(((uint64_t) 1 << smallerIdx) & cb->occupied)
        {
            return false;
        }
        smallerIdx += temp;
    }
    return true;
}

/**
 * Determines if the piece type at this location can make a valid move to the endIdx
 * 
 * @param pt        The piece type making the move
 * @param idxToFind The index of the piece being checked
 * @param endIdx    The index where the piece moved to
 */
bool ChessBoard::IsValidMove(uint8_t pt, uint8_t idxToFind, uint8_t endIdx)
{
    int8_t temp;
    bool status;

    switch(pt % (NUM_PIECE_TYPES/2))
    {
        // Pawn
        case 0:
            std::cout << "Pawn logic not implemented for IsValidMove!" << std::endl;
            status = false;
            break;
        // Rook
        case 1:
            status = ChessBoard::IsValidRookMove(this, idxToFind, endIdx);
            break;
        // Knight
        case 2:
            status = ChessBoard::IsValidKnightMove(idxToFind, endIdx);
            break;
        // Bishop
        case 3:
            status = ChessBoard::IsValidBishopMove(this, idxToFind, endIdx);
            break;
        // Queen
        case 4:
            status = ChessBoard::IsValidRookMove(this, idxToFind, endIdx)
                        || ChessBoard::IsValidBishopMove(this, idxToFind, endIdx);
            break;
        // King
        case 5:
            // Assuming a single king (which is always true)
            status = true;
        default:
            return false;
    }

    return true;
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

/**
 * Entry point where we run the game from. Split into two stages
 *  
 * 1) Accept opponent input and interpret
 * 
 * 2) Search for best move and generate response
 * 
 * 3) Send response to player
 */
void PlayGame(void)
{
    moveType_t *tempMove, *ourMoves, selectedMove;
    std::string str;

    // Get the board
    ChessBoard *cb = new ChessBoard();
    if(cb == NULL)
    {
        return;
    }

    // Main loop of the chess game, we are playing as black
    while(true)
    {
        // State 1

        str.clear();
        std::cout << "Please enter move: ";
        std::cin >> str;
        tempMove = ConvertStringToMove(cb, str);

        Util_Assert(tempMove != NULL, "Received bad move!");

        cb->ApplyMoveToBoard(tempMove);
        ourMoves = cb->GenerateMoves(BLACK_PIECES);

        // State 2
        cb->GetBestMove(SEARCH_DEPTH, false, ourMoves);
        selectedMove = *globalBestMove;

        // Actually apply our chosen move to the board
        cb->ApplyMoveToBoard(globalBestMove);

        // Cleanup
        while(ourMoves != NULL && ourMoves->legalMove)
        {
            tempMove = ourMoves->adjMove;
            delete ourMoves;
            ourMoves = tempMove;
        }

        // State 3
        std::cout << "Response:" << ConvertMoveToString(cb, &selectedMove) << std::endl;
    }
}
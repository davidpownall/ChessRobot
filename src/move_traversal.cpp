#include <iostream>
#include "util.h"
#include "chessboard_defs.h"
#include "chessboard.h"

static uint64_t numMoves = 0;

/**
 * Determines the next best move via a minimax search algorithm.
 * 
 * @param depth                         The search depth to look
 * @param playerToMaximize              If we are attempting to maximize or minimize score
 *                                      for this search depth
 * @param movesToEvaluateAtThisDepth    See name
 * @param alpha                         Alpha param for alpha beta pruning
 * @param beta                          Beta param for alpha beta pruning
 * 
 * @return                  The score associated with the best move for this search
 *                          depth
 * 
 * @note                    At search depth 0, the move with the best score will be 
 *                          placed in a variable of the board bestMove;
 */
int32_t ChessBoard::GetBestMove(uint64_t depth, bool playerToMaximize,
                                 moveType_t *movesToEvaluateAtThisDepth, int32_t alpha, int32_t beta)
{
    int32_t score, savedScore, value;
    moveType_t *moveToEvaluate, *movesToEvaluateAtNextDepth, *tempMove;
    bool evaluationNeeded = depth > 1;

    if(depth >= 6)
    {
        std::cout << "Assessing depth at: " << SEARCH_DEPTH - depth
             << " # Moves Assessed: " << numMoves << std::endl;
    }

    if(depth == 0)
    {
        return -EvaluateCurrentBoardValue(this);
    }

    moveToEvaluate = movesToEvaluateAtThisDepth;
    if(playerToMaximize)
    {
        score = INT32_MIN;
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
            value = this->GetBestMove(depth - 1, !playerToMaximize, movesToEvaluateAtNextDepth, alpha, beta);
            score = std::max(score, value);
            alpha = std::max(alpha, value);

            if(depth == SEARCH_DEPTH && score >= savedScore)
            {
                this->bestMove = moveToEvaluate;
            }
            this->UndoMoveFromBoard(moveToEvaluate);

            if(beta <= alpha)
            {
                break;
            }

            moveToEvaluate = moveToEvaluate->adjMove;

        }
    }
    else
    {
        score = INT32_MAX;
        while(moveToEvaluate != NULL && moveToEvaluate->legalMove)
        {
            numMoves++;
            this->ApplyMoveToBoard(moveToEvaluate);
            
            if(evaluationNeeded)
            {
                movesToEvaluateAtNextDepth = this->GenerateMoves(BLACK_PIECES);
            }

            savedScore = score;
            value = this->GetBestMove(depth - 1, !playerToMaximize, movesToEvaluateAtNextDepth, alpha, beta);
            score = std::max(score, value);
            beta = std::max(beta, value);

            if(depth == SEARCH_DEPTH && score >= savedScore)
            {
                this->bestMove = moveToEvaluate;
            }
            this->UndoMoveFromBoard(moveToEvaluate);

            if(beta <= alpha)
            {
                break;
            }

            moveToEvaluate = moveToEvaluate->adjMove;

        }
    }

//    std::cout << "Examined " << numMoves << " moves during search" << std::endl;
//    numMoves = 0;
    return score;
}

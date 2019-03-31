#include <iostream>
#include "util.h"
#include "chessboard.h"
#include "chessboard_test.h"
#include "threatmap.h"

void PlayGame(void);

int main() 
{
    uint64_t status;

    std::cout << "Welcome to the ChessRobot by David Pownall\n\n" << std::endl;

#if DEBUG_BUILD
    std::cout << "Starting ChessRobot test suite\n" << std::endl;

    status = executeTestSuite();
    if(status == STATUS_SUCCESS)
    {
        std::cout << "Test suite passed" << std::endl;
    }
    else
    {
        std::cout << "Test suite failed" << std::endl;
    }

    std::cout << "Finishing ChessRobot test suite\n" << std::endl;
#endif 

    std::cout << "Creating the Universal Chess Interface\n" << std::endl;
    // @todo: Create UCI

    std::cout << "Creating the board representation" << std::endl;
    std::cout << "Status: " << status << std::endl;

    PlayGame();
    while(1)
    {
        ;
    }

    return 0;
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

    ThreatMap_Generate(cb->GetPieces(), cb->GetOccupied());

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
        ThreatMap_Update(tempMove, cb->GetPieces(), cb->GetOccupied(), true);
        ourMoves = cb->GenerateMoves(BLACK_PIECES);

        // State 2
        cb->GetBestMove(SEARCH_DEPTH, false, ourMoves, INT32_MIN, INT32_MAX);

        Util_Assert(cb->GetAddrOfBestMove() != NULL, "Failed to find valid move!");

        // Save a copy of our best move
        selectedMove = *(cb->GetAddrOfBestMove());

        // Actually apply our chosen move to the board
        cb->ApplyMoveToBoard(&selectedMove);
        ThreatMap_Update(&selectedMove, cb->GetPieces(), cb->GetOccupied(), true);

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
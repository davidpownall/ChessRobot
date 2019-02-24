#include <iostream>
#include "util.h"
#include "chessboard.h"
#include "chessboard_test.h"

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
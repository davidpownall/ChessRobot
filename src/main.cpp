#include <iostream>
#include "util.h"
#include "chessboard.h"

int main() 
{
    uint64_t status;

    std::cout << "Welcome to the ChessRobot by David Pownall" << std::endl;
    std::cout << "Creating the Universal Chess Interface" << std::endl;
    std::cout << "Creating the board represntation" << std::endl;

    status = initializeChessBoard();
    std::cout << "Status: " << status << std::endl;


    while(1)
    {
        ;
    }

    return 0;
}
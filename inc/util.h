#include <iostream>
#include <string>
#include "chessboard.h"

#ifndef UTIL_DEFINE
#define UTIL_DEFINE

// @todo: Move this to build flag eventually
#define DEBUG_BUILD (1)

#define STATUS_SUCCESS  (0)
#define STATUS_FAIL     (1)

char Util_ConvertPieceTypeToChar(uint8_t pt);
std::string Util_ConvertPieceTypeToString(uint8_t pt);
void Util_Reverse64BitInteger(uint64_t *toReverse);
void Util_Assert(bool expr, std::string str);

#endif // UTIL_DEFINE
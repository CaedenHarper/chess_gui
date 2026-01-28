#include "Utils.hpp"

std::string Utils::intToAlgebraicNotation(const int square) {
    const int col = getCol(square);
    const std::string file = std::string{static_cast<char>('a' + col)};
    
    // reflect square over middle of chessboard
    const int row = 7 - getRow(square);
    const std::string rank = std::string{ static_cast<char>('1' + row)};

    return file + rank;
}

int Utils::algebraicNotationToInt(const std::string& square) {
    // Assume length of exactly 2
    const char colC = square.at(0);
    const char rowC = square.at(1);
    
    const int col = colC - 'a';
    // reflect row over middle of chessboard
    const int row = 7 - (rowC - '1');

    return getSquareIndex(col, row);
}
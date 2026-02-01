#pragma once

#include <array>
#include <string>

namespace Utils {
    // --- Constant variables ---

    // Number of squares on a chessboard.
    static constexpr int NUM_SQUARES = 64;

    // Width and height of a chessboard.
    constexpr int BOARD_WIDTH = 8;
    constexpr int BOARD_HEIGHT = 8;

    // string_view instead of string for constexpr
    // Starting game's FEN string.
    static constexpr std::string_view STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Constants for castling.
    static constexpr int WHITE_KING_STARTING_SQUARE = 60;
    static constexpr int BLACK_KING_STARTING_SQUARE = 4;

    static constexpr int WHITE_KINGSIDE_TARGET_SQUARE = 62;
    static constexpr int BLACK_KINGSIDE_TARGET_SQUARE = 6;

    static constexpr int WHITE_QUEENSIDE_TARGET_SQUARE = 58;
    static constexpr int BLACK_QUEENSIDE_TARGET_SQUARE = 2;

    static constexpr int WHITE_KINGSIDE_PASSING_SQUARE = 61;
    static constexpr int BLACK_KINGSIDE_PASSING_SQUARE = 5;

    static constexpr int WHITE_QUEENSIDE_PASSING_SQUARE = 59;
    static constexpr int BLACK_QUEENSIDE_PASSING_SQUARE = 3;

    static constexpr int WHITE_KINGSIDE_ROOK_STARTING_SQUARE = 63;
    static constexpr int BLACK_KINGSIDE_ROOK_STARTING_SQUARE = 7;

    static constexpr int WHITE_QUEENSIDE_ROOK_STARTING_SQUARE = 56;
    static constexpr int BLACK_QUEENSIDE_ROOK_STARTING_SQUARE = 0;

    // Constants for directions (e.g., NORTH -> one square up)
    constexpr int NORTH = 8;
    constexpr int EAST = 1;
    constexpr int SOUTH = -8;
    constexpr int WEST = -1;
    constexpr int NORTH_EAST = 9;
    constexpr int NORTH_WEST = 7;
    constexpr int SOUTH_EAST = -7;
    constexpr int SOUTH_WEST = -9;

    // Constants for some piece's movements
    static constexpr std::array<std::array<int, 2>, 8> knightDeltas {{
        // col (x), row (y)
        {-2, -1}, // left up
        {-1, -2}, // up left
        {1, -2}, // up right
        {2, -1}, // right up
        {2, 1}, // right down
        {1, 2}, // down right
        {-1, 2}, // down left
        {-2, 1}, // left down
    }};

    static constexpr std::array<std::array<int, 2>, 4> bishopDeltas {{
        // col (x), row (y)
        {-1, -1}, // up left
        {1, -1}, // up right
        {-1, 1}, // down left
        {1, 1}, // down right
    }};

    static constexpr std::array<std::array<int, 2>, 4> rookDeltas {{
        // col (x), row (y)
        {0, 1}, // up
        {0, -1}, // down
        {1, 0}, // right
        {-1, 0}, // left
    }};

    static constexpr std::array<std::array<int, 2>, 8> queenDeltas {{
        // col (x), row (y)
        // rook moves
        {0, 1}, // up
        {0, -1}, // down
        {1, 0}, // right
        {-1, 0}, // left
        // bishop moves
        {-1, -1}, // up left
        {1, -1}, // up right
        {-1, 1}, // down left
        {1, 1}, // down right
    }};

    static constexpr std::array<std::array<int, 2>, 8> kingDeltas {{
        // col (x), row (y)
        {-1, -1}, // up left
        {0, -1}, // up
        {1, -1}, // up right
        {-1, 0}, // left
        {1, 0}, // right
        {-1, 1}, // down left
        {0, 1}, // down
        {1, 1}, // down right
    }};

    // Get column of square.
    constexpr int getCol(int square) noexcept {
        // equivalent to square % 8
        constexpr int COLUMN_MASK = 0b111;

        return square & COLUMN_MASK;
    }
    // Get row of square.
    constexpr int getRow(int square) noexcept {
        // equivalent to square / 8
        constexpr int ROW_SHIFT = 3;

        return square >> ROW_SHIFT;
    }
    // Get square index from column and row.
    constexpr int getSquareIndex(int col, int row) noexcept {
        // equivalent to (8 * row) + col
        constexpr int ROW_SHIFT = 3;
        
        return (row << ROW_SHIFT) | col;
    }

    // If the square is on the board, in bounds.
    constexpr bool onBoard(int square) noexcept {
        return 0 <= square && square <= NUM_SQUARES;
    }
    // If a column and row is on the board, in bounds.
    constexpr bool onBoard(int col, int row) noexcept {
        return 0 <= col && col < BOARD_WIDTH && 0 <= row && row < BOARD_HEIGHT;
    }

    // Mirror square White -> Black or Black -> White
    constexpr int mirrorSquare(int square) noexcept {
        // Flip the first three bits and leave the last three untouched, flipping the rank from white <-> black.
        constexpr int FLIP_CONSTANT = 0b111000;
        return square ^ FLIP_CONSTANT;
    }

    // Retrieve algebraic notation from a given square. E.g., 0 -> "a8".
    std::string intToAlgebraicNotation(int square);

    // Retrieve square int from a given algebraic notation. E.g., "a8" -> 0.
    int algebraicNotationToInt(const std::string& square);
} // namespace Utils
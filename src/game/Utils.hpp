#pragma once

#include <array>
#include <string>

namespace Utils {
    // Get column of square.
    constexpr int getCol(int square) noexcept {
        // equivalent to square % 8
        return square & 7;
    }
    // Get row of square.
    constexpr int getRow(int square) noexcept {
        // equivalent to square / 8
        return square >> 3;
    }
    // Get square index from column and row.
    constexpr int getSquareIndex(int col, int row) noexcept {
        // equivalent to (8 * row) + col
        return (row << 3) | col;
    }

    // If the square is on the board, in bounds.
    constexpr bool onBoard(int square) noexcept {
        return 0 <= square && square <= 63;
    }
    // If a column and row is on the board, in bounds.
    constexpr bool onBoard(int col, int row) noexcept {
        return 0 <= col && col <= 7 && 0 <= row && row <= 7;
    }

    // Retrieve algebraic notation from a given square. E.g., 0 -> "a8".
    std::string intToAlgebraicNotation(int square);

    // Retrieve square int from a given algebraic notation. E.g., "a8" -> 0.
    int algebraicNotationToInt(const std::string& square);

    // --- Constant variables ---

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

} // namespace Utils
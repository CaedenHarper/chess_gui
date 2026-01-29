#pragma once

#include <cstdint>
#include <string>

// Colors a piece can have. None represents an empty square.
enum class Color : uint8_t  {
    None,
    White,
    Black
};

// Types a piece can have. None represents an empty square.
enum class PieceType: uint8_t  {
    None,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

// A chess piece, with a piece type and color. Stored in uint8_t as (PieceType | Color).
class Piece {
public:
    // Construct an empty piece, which represents an empty square.
    constexpr Piece() noexcept : packed_{0} {}
    // Construct a piece given a type and color.
    constexpr Piece(PieceType type, Color color) noexcept : packed_{pack_(type, color)} {}

    // If a piece is equal to another piece. A piece is equal if the type and color matches.
    constexpr bool operator==(Piece other) const noexcept { return packed_ == other.packed_; }

    // Setters use bitwise ops to quickly extract info from packed_.
    constexpr PieceType type() const noexcept { return static_cast<PieceType>(packed_ & TYPE_MASK); }
    constexpr Color color() const noexcept { return static_cast<Color>((packed_ >> 3) & COLOR_MASK); }
    // Retrieve if the piece exists. i.e., if the piece is not an empty square.
    constexpr bool exists() const noexcept { return (packed_ & TYPE_MASK) != 0; } // 0 -> PieceType::None
    constexpr uint8_t raw() const noexcept { return packed_; }

    // Retrieve a string of length one which represents the piece. Uppercase for white, lowercase for black. E.g., white pawn -> "P"
    std::string to_string_short() const;
    // Retrieve a string which represents the piece. E.g., white pawn -> "White Pawn"
    std::string to_string_long() const;
    
    // Determine piece type from char. E.g., 'P' -> Pawn
    static PieceType charToPieceType(char piece);
    // Determine piece from char. Uppercase for white, lowercase for black. E.g., 'P' -> White Pawn
    static Piece charToPiece(char piece);

protected:
    // packed representation of PieceType | Color. 
    uint8_t packed_;

    // 3 bits for type, 2 for color, and corresponding bitmasks
    static constexpr uint8_t TYPE_BITS = 3;
    static constexpr uint8_t COLOR_BITS = 2;
    static constexpr uint8_t TYPE_MASK = (1 << TYPE_BITS) - 1;
    static constexpr uint8_t COLOR_MASK = (1 << COLOR_BITS) - 1;

    // Pack PieceType and Color into uint8_t
    static constexpr uint8_t pack_(PieceType type, Color color) noexcept {
        return (static_cast<uint8_t>(type) & TYPE_MASK) | ((static_cast<uint8_t>(color) & COLOR_MASK) << 3);
    }
};
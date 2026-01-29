#pragma once

#include <array>
#include <cassert>

#include "Piece.hpp"

enum class MoveFlag : uint8_t {
    Normal,  // no special flags
    Capture,
    DoublePawnPush,
    KingCastle,
    QueenCastle,
    EnPassant,
    Promotion,  // non-capture promotion
    PromotionCapture
};

enum class Promotion : uint8_t { 
    None,
    Knight,
    Bishop,
    Rook,
    Queen
};

class Game; // forward declare for Move

// A chess move, with information for squares, pieces, and special flags like promotion and castling.
class Move {
public:
    // Create a default move that should not be used; only needed for MoveList to quickly initialize many default Moves
    constexpr Move() noexcept : packed_{0} {}
    // Create a move given a source square, target square, flag, and promotion.
    constexpr Move(uint8_t sourceSquare, uint8_t targetSquare, MoveFlag flag, Promotion promotion) noexcept
                : packed_{pack_(sourceSquare, targetSquare, flag, promotion)} {}
    constexpr Move(int sourceSquare, uint8_t targetSquare, MoveFlag flag, Promotion promotion) noexcept
                : packed_{pack_(sourceSquare, targetSquare, flag, promotion)} {}
    constexpr Move(int sourceSquare, int targetSquare, MoveFlag flag, Promotion promotion) noexcept
                : packed_{pack_(static_cast<uint8_t>(sourceSquare), static_cast<uint8_t>(targetSquare), flag, promotion)} {}

    // Create a move using Pieces for context for flag + promotion. This is slower, only use if necessary / not in hot loop.
    static Move fromPieces(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece);

    // Has to have the same starting and target squares, pieces, and any special flags.
    constexpr bool operator==(Move other) const noexcept { return packed_ == other.packed_; }

    // Getters use bitwise ops to quickly extract info from packed_.
    constexpr uint8_t sourceSquare() const noexcept { return (packed_ >> SOURCE_SHIFT) & SOURCE_MASK; }
    constexpr uint8_t targetSquare() const noexcept { return (packed_ >> TARGET_SHIFT) & TARGET_MASK; }
    constexpr MoveFlag flag() const noexcept { return static_cast<MoveFlag>((packed_ >> FLAG_SHIFT) & FLAG_MASK); }
    constexpr Promotion promotion() const noexcept { return static_cast<Promotion>((packed_ >> PROMO_SHIFT) & PROMO_MASK); }
    constexpr bool isPromotion() const noexcept { return flag() == MoveFlag::Promotion || flag() == MoveFlag::PromotionCapture; }
    constexpr bool isEnPassant() const noexcept { return flag() == MoveFlag::EnPassant; }
    constexpr bool isDoublePawn() const noexcept { return flag() == MoveFlag::DoublePawnPush; }
    constexpr bool isKingSideCastle() const noexcept { return flag() == MoveFlag::KingCastle; }
    constexpr bool isQueenSideCastle() const noexcept { return flag() == MoveFlag::QueenCastle; }
    constexpr bool isCapture() const noexcept { return flag() == MoveFlag::Capture || flag() == MoveFlag::PromotionCapture || flag() == MoveFlag::EnPassant; }

    // NOTE: not defined here; defined in Game because
    // it requires a reference to Game and don't want to include Game in this file
    // Retrieve a string representation of the move. E.g., "White Pawn on e2 to Empty Square on e4".
    std::string to_string(const Game& game) const;
    // Retrieve a long algebraic representation of the move. E.g., "e2e4"
    std::string toLongAlgebraic() const;

    // Get piece type from promotion class.
    static constexpr PieceType promotionToPieceType(Promotion promotion) noexcept {
        switch (promotion) {
            case Promotion::Knight: return PieceType::Knight;
            case Promotion::Bishop: return PieceType::Bishop;
            case Promotion::Rook: return PieceType::Rook;
            case Promotion::Queen: return PieceType::Queen;
            default: assert(false); return PieceType::None; // should never happen
        }
    }

private:
    uint32_t packed_;

    // Constants to improve readability in MOVE
    static constexpr int SOURCE_BITS = 6;
    static constexpr int TARGET_BITS = 6;
    static constexpr int FLAG_BITS = 4;
    static constexpr int PROMO_BITS = 3;

    static constexpr int SOURCE_SHIFT = 0;
    static constexpr int TARGET_SHIFT = SOURCE_SHIFT + SOURCE_BITS; // 6
    static constexpr int FLAG_SHIFT = TARGET_SHIFT + TARGET_BITS; // 12
    static constexpr int PROMO_SHIFT = FLAG_SHIFT   + FLAG_BITS;   // 16

    static constexpr uint32_t SOURCE_MASK = (1U << SOURCE_BITS) - 1; // 0x3F
    static constexpr uint32_t TARGET_MASK = (1U << TARGET_BITS) - 1; // 0x3F
    static constexpr uint32_t FLAG_MASK = (1U << FLAG_BITS)   - 1; // 0xF
    static constexpr uint32_t PROMO_MASK  = (1U << PROMO_BITS)  - 1; // 0x7

    static constexpr uint32_t pack_(uint8_t sourceSquare, uint8_t targetSquare, MoveFlag flag, Promotion promotion) noexcept {
        return (static_cast<uint32_t>(sourceSquare) & SOURCE_MASK)
             | ((static_cast<uint32_t>(targetSquare) & TARGET_MASK) << TARGET_SHIFT)
             | ((static_cast<uint32_t>(flag) & FLAG_MASK) << FLAG_SHIFT)
             | ((static_cast<uint32_t>(promotion) & PROMO_MASK) << PROMO_SHIFT);
    }
};

// A list of moves. Wrapper for std::array<> for quick lookups. 
struct MoveList {
    // Max amount of moves; somewhat arbitrary, but should be enough for any pseudo-legal move count
    static constexpr int kMaxMoves = 256;
    std::array<Move, kMaxMoves> data;
    // Only moves between [0, MoveList.size) are valid; the rest are placeholder Moves which have undefined behavior
    int size = 0;

    constexpr void clear() noexcept { size = 0; }

    constexpr void push_back(const Move& move) noexcept {
        data[size++] = move;
    }
} __attribute__((aligned(128))); // NOLINT[magic numbers] align to 128 bytes
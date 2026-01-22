#pragma once
#include <cstdint>
#include <iostream>

class Bitboard{
public:
    constexpr Bitboard() : bitboard_{0} {}
    constexpr explicit Bitboard(uint64_t bitboard) : bitboard_{bitboard} {}

    // Helper utility functions for bitboards

    // --- Queries ---
    // If two bitboards intersect.
    constexpr bool intersects(const Bitboard& other) const { return (bitboard_ & other.bitboard_) != 0; }
    // If bitboard has the given bit.
    constexpr bool hasBit(uint64_t bit) const { return (bitboard_ & bit) != 0; }
    // If bitboard contains the given chess square.
    constexpr bool containsSquare(int square) const { return (bitboard_ & (1ULL << square)) != 0; }
    // Is bitboard empty.
    constexpr bool empty() const { return bitboard_ == 0; }
    // Raw representation of bitboard.
    constexpr uint64_t raw() const { return bitboard_; }

    // --- Mutations ---
    // Set given chess square.
    constexpr void setSquare(int square) { bitboard_ |= (1ULL << square); }
    // Clear given chess square.
    constexpr void clearSquare(int square) { bitboard_ &= ~(1ULL << square); }
    // Set given bit.
    constexpr void setBit(uint64_t bit) { bitboard_ |= bit; }
    // Clear given bit.
    constexpr void clearBit(uint64_t bit) { bitboard_ &= ~bit; }
    // Clear entire bitboard.
    constexpr void clear() { bitboard_ = 0; }
    // Merge given bitboard and create a new bitboard, without updating this bitboard.
    constexpr Bitboard merge(const Bitboard& other) const { return Bitboard(bitboard_ | other.bitboard_); }
    // Set this bitboard to this bitboard merged with another.
    constexpr void mergeIn(const Bitboard& other) { bitboard_ |= other.bitboard_; }
    // Get LSB of bitboard. Bitboard must be nonzero.
    constexpr int popLsb() {
        // __builtin_ctzll requires nonzero input
        const int lsb = __builtin_ctzll(bitboard_);
        bitboard_ &= bitboard_ - 1;
        return lsb;
    }

    // Get bit from square.
    static constexpr uint64_t bit(int square) { return 1ULL << square; }

private:
    uint64_t bitboard_; // NOLINT(cppcoreguidelines-use-default-member-init, modernize-use-default-member-init) we want constexpr constructor
};

#pragma once
#include <bitset>
#include <cstdint>

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
    // Set given bit.
    constexpr void setBit(uint64_t bit) { bitboard_ |= bit; }

    // Clear given chess square.
    constexpr void clearSquare(int square) { bitboard_ &= ~(1ULL << square); }
    // Clear given bit.
    constexpr void clearBit(uint64_t bit) { bitboard_ &= ~bit; }
    // Clear entire bitboard.
    constexpr void clear() { bitboard_ = 0; }

    // Mask current bitboard with other bitboard.
    constexpr Bitboard mask(Bitboard other) const { return Bitboard{bitboard_ & other.bitboard_}; }

    // Merge given bitboard and create a new bitboard, without updating this bitboard.
    constexpr Bitboard merge(const Bitboard& other) const { return Bitboard(bitboard_ | other.bitboard_); }
    // Set this bitboard to this bitboard merged with another.
    constexpr void mergeIn(const Bitboard& other) { bitboard_ |= other.bitboard_; }

    // Return bitboard with 0's to 1's and 1's to 0's flipped; return bitwise negation of the bitboard.
    constexpr Bitboard flip() const { return Bitboard{~bitboard_}; }

    // Shift to the right by `numBits` bits
    constexpr Bitboard rightShift(uint64_t numBits) const { return Bitboard{bitboard_ >> numBits}; }
    // Shift to the left by `numBits` bits
    constexpr Bitboard leftShift(uint64_t numBits) const { return Bitboard{bitboard_ << numBits}; }

    // Get LSB of bitboard. Bitboard must be nonzero.
    constexpr int popLsb() {
        // __builtin_ctzll requires nonzero input
        const int lsb = __builtin_ctzll(bitboard_);
        bitboard_ &= bitboard_ - 1;
        return lsb;
    }

    // Get bit from square.
    static constexpr uint64_t bit(int square) { return 1ULL << square; }

    // Get string representation of bitboard.
    std::string to_string() const { return std::bitset<64>(raw()).to_string(); }

    // Helper bitmasks
    static constexpr uint64_t FileA = 0x0101010101010101ULL;
    static constexpr uint64_t FileB = 0x0202020202020202ULL;
    static constexpr uint64_t FileC = 0x0404040404040404ULL;
    static constexpr uint64_t FileD = 0x0808080808080808ULL;
    static constexpr uint64_t FileE = 0x1010101010101010ULL;
    static constexpr uint64_t FileF = 0x2020202020202020ULL;
    static constexpr uint64_t FileG = 0x4040404040404040ULL;
    static constexpr uint64_t FileH = 0x8080808080808080ULL;
    
    static constexpr uint64_t Rank1 = 0xff00000000000000ULL;
    static constexpr uint64_t Rank2 = 0x00ff000000000000ULL;
    static constexpr uint64_t Rank3 = 0x0000ff0000000000ULL;
    static constexpr uint64_t Rank4 = 0x000000ff00000000ULL;
    static constexpr uint64_t Rank5 = 0x00000000ff000000ULL;
    static constexpr uint64_t Rank6 = 0x0000000000ff0000ULL;
    static constexpr uint64_t Rank7 = 0x000000000000ff00ULL;
    static constexpr uint64_t Rank8 = 0x00000000000000ffULL;

private:
    uint64_t bitboard_;
};

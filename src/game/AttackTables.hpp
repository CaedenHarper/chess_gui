#pragma once
#include <array>

#include "Bitboard.hpp"

struct AttackTables {
    std::array<Bitboard, 64> knightFrom{};
    std::array<Bitboard, 64> kingFrom{};

    // pawnFrom[color][square]
    std::array<std::array<Bitboard, 64>, 2> pawnFrom{};

    // pawnTo[color][target]
    std::array<std::array<Bitboard, 64>, 2> pawnTo{};

    // rookRays[square][direction]
    std::array<std::array<Bitboard, 4>, 64> rookRays{};
    std::array<std::array<Bitboard, 4>, 64> bishopRays{};
};

extern AttackTables gAtt;
void initAttackTables() noexcept;

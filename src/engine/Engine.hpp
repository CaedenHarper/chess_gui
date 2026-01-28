#pragma once

#include "../game/Game.hpp"

class Engine {
public:
    Engine() = default;
    // TODO: this should be const Game& game once we fix game move gen being non-const
    // Get the best move in the current position.
    Move bestMove(Game& game) const;
    // Evaluate the current position.
    float evaluatePosition(Game& game) const;

    // Piece cost constants
    static constexpr int PAWN_COST = 1;
    static constexpr int KNIGHT_COST = 3;
    static constexpr int BISHOP_COST = 3;
    static constexpr int ROOK_COST = 5;
    static constexpr int QUEEN_COST = 9;

private:
    // Evalute the current position's piece costs. E.g., 1 -> pawn, 3 -> bishop / knight, ... 
    int evaluatePieceSum_(Game& game, Color color) const;
    // Get popcount for an integer
    static constexpr int popcount_(uint64_t n) {
        return __builtin_popcountll(n);
    }
};
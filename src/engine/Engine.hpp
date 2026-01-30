#pragma once

#include "../game/Game.hpp"
#include <optional>

// Contains best move, if it exists, and best move's eval
class SearchResult {
public:
    constexpr SearchResult(std::optional<Move> bestMove_, int eval_) : bestMove{bestMove_}, eval{eval_} {}
    std::optional<Move> bestMove;
    int eval{0};
};

class Engine {
public:
    constexpr Engine() noexcept = default;
    // TODO: this should be const Game& game once we fix game move gen being non-const
    // Get the best move in the current position.
    SearchResult bestMove(Game& game);
    // Evaluate the current position.
    int evaluatePosition(Game& game) const;
    // Search for moves in the current position.
    SearchResult search(Game& game, int depth);
    // Search a bit more to ensure we end on a quiet move.
    int quiesce(Game& game, int alpha, int beta);

    // Piece cost constants
    static constexpr int PAWN_COST = 100;
    static constexpr int KNIGHT_COST = 300;
    static constexpr int BISHOP_COST = 300;
    static constexpr int ROOK_COST = 500;
    static constexpr int QUEEN_COST = 900;

    // Search return value constants
    // TODO: is a number different than the arbitrary 2^20 better?
    static constexpr int CHECKMATE = 1'048'576;
    static constexpr int STALEMATE = 0;

private:
    // Evalute the current position's piece costs. E.g., 1 -> pawn, 3 -> bishop / knight, ... 
    int evaluatePieceSum_(Game& game, Color color) const;
    // internal negaMax alpha beta search that search() implements
    int alphaBeta_(Game& game, int alpha, int beta, int depth, int ply);
    // Get popcount for an integer
    static constexpr int popcount_(uint64_t n) {
        return __builtin_popcountll(n);
    }
};
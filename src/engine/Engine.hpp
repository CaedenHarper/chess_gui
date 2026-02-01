#pragma once

#include "../game/Game.hpp"
#include <optional>

struct SearchStats {
    uint64_t nodes = 0;       // main search nodes
    uint64_t qnodes = 0;      // quiescence nodes
    // Clear the stats
    constexpr void clear() noexcept {
        nodes = 0;
        qnodes = 0;
    }
} __attribute__((aligned(16))); // NOLINT[magic numbers] align to 16 bytes

// Contains helpers for evaluation. Constants and helper functions.
namespace Eval {
    // Piece cost constants
    static constexpr int PAWN_COST = 100;
    // 320 and 330 based on https://www.chessprogramming.org/Simplified_Evaluation_Function; see rationale on page
    static constexpr int KNIGHT_COST = 320;
    static constexpr int BISHOP_COST = 330;
    static constexpr int ROOK_COST = 500;
    static constexpr int QUEEN_COST = 900;
    // TODO: king cost?

    // Basic piece square tables, also based on Simplified_Evaluation_Function
    static constexpr std::array<int, Utils::NUM_SQUARES> BLACK_PAWN_EVAL_TABLE = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    static constexpr std::array<int, Utils::NUM_SQUARES> BLACK_KNIGHT_EVAL_TABLE = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    };

    static constexpr std::array<int, Utils::NUM_SQUARES> BLACK_BISHOP_EVAL_TABLE = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    };

    static constexpr std::array<int, Utils::NUM_SQUARES> BLACK_ROOK_EVAL_TABLE = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    };

    static constexpr std::array<int, Utils::NUM_SQUARES> BLACK_QUEEN_EVAL_TABLE = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };

    static constexpr std::array<int, Utils::NUM_SQUARES> BLACK_KING_EVAL_TABLE = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    };

    // Search return value constants
    // TODO: is a number different than the arbitrary 2^20 better?
    static constexpr int CHECKMATE = 1'048'576;
    static constexpr int STALEMATE = 0;
    
    // If the evaluation shows there will be a mate.
    constexpr bool isMate(const int eval) noexcept {
        // we assume MAX_PLY (or max depth we recurse) is 256
        constexpr int MAX_PLY = 256;
        return abs(eval) >= CHECKMATE - MAX_PLY;
    }


    inline std::string evalToString(const int eval, const Color color) noexcept {
        // we assume MAX_PLY (or max depth we recurse) is 256
        if(isMate(eval)) {
            const int pliesToMate = CHECKMATE - abs(eval);
            // round plies to full moves
            int movesToMate = (pliesToMate + 1) / 2;
            // black is negative
            movesToMate = color == Color::White ? movesToMate: -movesToMate;

            // #{moves} for white, #-moves for black
            return "#" + std::to_string(movesToMate);
        }

        // not mate, we can return as-is, negative for black
        return std::to_string(eval * (color == Color::White ? 1 : -1));
    }
}; // namespace Eval

// Contains best move, if it exists, and best move's eval
struct SearchResult {
    std::optional<Move> bestMove;
    int eval{0};
    SearchStats stats;
} __attribute__((aligned(32))); // NOLINT[magic numbers] align to 16 bytes

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
    int quiesce(Game& game, int alpha, int beta, int ply);
    // Order moves to improve Alpha Beta pruning.
    void orderMoves(Game& game, const MoveList& moves, std::array<int, MoveList::kMaxMoves>& indices);

    // Get piece value from piece.
    static constexpr int pieceValueFromType(const Piece piece) {
        switch(piece.type()) {
            case PieceType::Pawn: return Eval::PAWN_COST;
            case PieceType::Knight: return Eval::KNIGHT_COST;
            case PieceType::Bishop: return Eval::BISHOP_COST;
            case PieceType::Rook: return Eval::ROOK_COST;
            case PieceType::Queen: return Eval::QUEEN_COST;
            default: return 0; // king and empty square
        }
    }
    // TODO: move to MoveOrdering.hpp
    // Move bonus based on victim and attacker: most valuable victim, least valuable attacker.
    static constexpr int mvv_lva_bonus(const Game& game, const Move& move) {
        // only intended for captures
        if (!move.isCapture()) {
            return 0;
        }

        const int victim = pieceValueFromType(game.mailbox()[move.targetSquare()]);
        const int attacker = pieceValueFromType(game.mailbox()[move.sourceSquare()]);
    
        // scale to ensure captures rank above quiet moves
        constexpr int VICTIM_BONUS = 100;
        return (VICTIM_BONUS * victim) - attacker;
    }

private:
    // Evalute the current position's piece costs. E.g., 1 -> pawn, 3 -> bishop / knight, ... 
    int evaluatePieceSum_(Game& game, Color color) const;
    // Evaluate the current position's piece placements.
    int evaluatePiecePlacementBonus_(Game& game, Color color) const;
    // internal negaMax alpha beta search that search() implements
    int alphaBeta_(Game& game, int alpha, int beta, int depth, int ply);
    // Get popcount for an integer
    static constexpr int popcount_(uint64_t n) {
        return __builtin_popcountll(n);
    }

    // Keep track of search stats (e.g., how many positions evaluated)
    SearchStats stats_;
};
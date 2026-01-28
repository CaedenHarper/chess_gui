// NOLINTBEGIN(readability-convert-member-functions-to-static) Engines prefer engine.foo() over Engine::foo()

#include "Engine.hpp"

#include <random>

Move Engine::bestMove(Game& game) const {
    // right now we just pick a random move
    MoveList moves;
    game.generateLegalMoves(moves);
    
    std::random_device rnd;
    std::mt19937 gen(rnd());
    std::uniform_int_distribution<int> dist(0, moves.size-1);
    const int random_num = dist(gen);

    return moves.data[random_num];
}

float Engine::evaluatePosition(Game& game) const {
    const float whiteEval = evaluatePieceSum_(game, Color::White);
    const float blackEval = evaluatePieceSum_(game, Color::Black);

    const float totalEval = whiteEval - blackEval; 
    return totalEval;
}

int Engine::evaluatePieceSum_(Game& game, const Color color) const {
    const bool isWhite = color == Color::White;
    int sum = 0;
    sum += popcount_((isWhite ? game.bbWhitePawns() : game.bbBlackPawns()).raw()) * PAWN_COST;
    sum += popcount_((isWhite ? game.bbWhiteKnights() : game.bbBlackKnights()).raw()) * KNIGHT_COST;
    sum += popcount_((isWhite ? game.bbWhiteBishops() : game.bbBlackBishops()).raw()) * BISHOP_COST;
    sum += popcount_((isWhite ? game.bbWhiteRooks() : game.bbBlackRooks()).raw()) * ROOK_COST;
    sum += popcount_((isWhite ? game.bbWhiteQueens() : game.bbBlackQueens()).raw()) * QUEEN_COST;
    return sum;
}

// NOLINTEND(readability-convert-member-functions-to-static)
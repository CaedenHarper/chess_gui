#include <cstdint>
#include <iostream>

#include "Perft.hpp"
#include "../src/game/Game.hpp"

// limit of 9,223,372,036,854,775,807
int64_t Perft::perft(Game& game, int depth) { // NOLINT(misc-no-recursion) 
    if (depth <= 0) {
        return 1;
    }

    unsigned int numPositions = 0;

    for (const Move& move : game.generateAllLegalMoves()) {
        const UndoInfo flags = game.getUndoInfo();

        game.makeMove(move);
        numPositions += perft(game, depth - 1);
        game.undoMove(move, flags);
    }

    return numPositions;
}


// Prints each root move and its subtree count; matches Stockfish for debugging
int64_t Perft::perftDivide(Game& game, int depth) {
    if (depth <= 0) {
        return 1;
    }

    int64_t total = 0;

    for (const Move& move : game.generateAllLegalMoves()) {
        const UndoInfo flags = game.getUndoInfo();

        game.makeMove(move);
        const int64_t moveNodes = perft(game, depth - 1);
        game.undoMove(move, flags);

        std::cout << move.toLongAlgebraic() << ": " << moveNodes << "\n";

        total += moveNodes;
    }

    return total;
}
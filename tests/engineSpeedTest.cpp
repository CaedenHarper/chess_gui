#include <chrono>
#include <iostream>
#include <optional>
#include <string>

#include "../src/engine/Engine.hpp"
#include "../src/game/Game.hpp"
#include "../src/game/Move.hpp"
#include "../src/game/Utils.hpp"

void runEngineSelfPlayTest(int maxPlies) {
    Game game;
    game.loadFEN(std::string{Utils::STARTING_FEN});

    Engine engine;

    std::cerr << "Starting self-play test from starting FEN\n";
    std::cerr << "Max plies: " << maxPlies << "\n\n";

    int ply = 0;
    const std::chrono::steady_clock::time_point tStart = std::chrono::steady_clock::now();
    while (ply < maxPlies && !game.isFinished()) {
        const auto tMoveStart = std::chrono::steady_clock::now();

        // Get engine move
        const auto [maybeMove, eval, stats] = engine.bestMove(game);

        const auto tMoveThinkEnd = std::chrono::steady_clock::now();
        const auto thinkMs = std::chrono::duration_cast<std::chrono::milliseconds>(tMoveThinkEnd - tMoveStart).count();

        if (!maybeMove.has_value()) {
            std::cerr << "Engine reports no legal move (game finished) at ply " << ply << "\n";
            break;
        }

        const Move move = maybeMove.value();
        // generate moveString before making move, because it depends on game state
        const std::string moveString = move.to_string(game);

        // Try to make move — use tryMove to ensure legality
        if (!game.tryMove(move)) {
            std::cerr << "Engine produced illegal move at ply " << ply << ": "
                      << moveString << "\n";
            break;
        }

        // Print move info
        const char sideChar = (game.sideToMove() == Color::White) ? 'b' : 'w'; 
        // note: side printed is the side who just moved (we inverted after tryMove above),
        // adjust if your game.sideToMove() semantics differ.
        std::cerr << "Ply " << ply << " (" << sideChar << "): "
                  << moveString
                  << "   eval=" << eval
                  << "   think_ms=" << thinkMs << "\n";

        ply++;
    }

    const auto tEnd = std::chrono::steady_clock::now();
    const auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

    std::cerr << "\nSelf-play finished after " << ply << " plies. Total time: " << totalMs << " ms\n";

    if (game.isFinished()) {
        if (game.isInCheck(game.sideToMove())) {
            std::cerr << "Result: checkmate. Side to move is checkmated.\n";
        } else {
            std::cerr << "Result: draw / stalemate / insufficient material (game reports finished).\n";
        }
    } else {
        std::cerr << "Result: reached ply limit (" << maxPlies << ").\n";
    }
}

int main() {
    const int maxPlies = 20;
    runEngineSelfPlayTest(maxPlies);
}
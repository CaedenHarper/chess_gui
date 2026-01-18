#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>

#include "../src/game/Game.hpp"

int64_t getNumPositions(Game& game, int depth) { // NOLINT(misc-no-recursion) 
    if (depth == 0) {
        return 1;
    }

    int64_t nodes = 0;
    #pragma unroll 64
    for(int squareIndex = 0; squareIndex < 64; squareIndex++) {
        // only check legal moves from current side
        if(game.board()[squareIndex].color() != game.currentTurn()) { // NOLINT allow [] in hot loop
            continue;
        }
        
        for (const Move& move : game.generateLegalMoves(squareIndex)) {
            game.makeMove(move);
            nodes += getNumPositions(game, depth - 1);
            game.undoMove(move);
        }
    }

    return nodes;
}

// verify the number of positions up to 13 ply is correct
int main() {
    // 13 ply is the most that will fit in an int64
    const std::map<int, int64_t> numPlyToNumPositions {
        {1, 20},
        {2, 400},
        {3, 8'902},
        {4, 197'281},
        {5, 4'865'609},
        {6, 119'060'324},
        {7, 3'195'901'860},
        {8, 84'998'978'956},
        {9, 2'439'530'234'167},
        {10, 69'352'859'712'417},
        {11, 2'097'651'003'696'806},
        {12, 62'854'969'236'701'747},
        {13, 1'981'066'775'000'396'239},
    };

    // init game
    Game game;
    game.loadFEN(std::string{Game::STARTING_FEN});
    // the most I could verify was 8
    for(int depth = 1; depth < 8; depth++) {
        const int64_t expectedNumPositions = numPlyToNumPositions.at(depth);
        const int64_t numPositions = getNumPositions(game, depth);
        
        if(numPositions != expectedNumPositions) {
            std::cerr << "Got numPositions " << numPositions << ", expected " << expectedNumPositions << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Ply " << depth << ": " << numPositions << " moves" << "\n";
    }
}
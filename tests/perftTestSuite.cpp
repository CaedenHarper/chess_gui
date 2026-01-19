#include <cstdlib>
#include <iostream>

#include "Perft.hpp"

// All positions from https://www.chessprogramming.org/Perft_Results
// All perft maps are std::array where ply = num positions (ply 0 -> 0)

bool checkPosition(const std::vector<int64_t>& perfMap, const std::string& name, const std::string& FEN, int maxDepth) {
    // init game
    Game game;
    game.loadFEN(FEN);
    for(int depth = 1; depth <= maxDepth; depth++) {
        const int64_t expectedNumPositions = perfMap.at(depth);
        const int64_t numPositions = Perft::perft(game, depth);
        
        if(numPositions != expectedNumPositions) {
            std::cerr << name << ": Got numPositions " << numPositions << ", expected " << expectedNumPositions << "\n";
            return false;
        }
        std::cout << name << ": Ply " << depth << ": " << numPositions << " moves" << "\n";
    }

    return true;
}

int main() {
    const std::vector<int64_t> positionStartPerfts{0, 20, 400, 8'902, 197'281, 4'865'609, 119'060'324, 3'195'901'860, 84'998'978'956, 2'439'530'234'167, 69'352'859'712'417, 2'097'651'003'696'806,62'854'969'236'701'747};
    const std::vector<int64_t> position2Perfts {0, 48, 2'039, 97'862, 4'085'603, 193'690'690, 8'031'647'685};
    const std::vector<int64_t> position3Perfts {0, 14, 191, 2'812, 43'238, 674'624, 11'030'083, 178'633'661, 3'009'794'393};

    // if(!checkPosition(positionStartPerfts, "Start Position", std::string{Game::STARTING_FEN}, 6)) {
    //     return EXIT_FAILURE;
    // }

    if(!checkPosition(position2Perfts, "Position 2", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 7)) {
        return EXIT_FAILURE;
    }

    // if(!checkPosition(position3Perfts, "Position 3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 8)) {
    //     return EXIT_FAILURE;
    // }
}
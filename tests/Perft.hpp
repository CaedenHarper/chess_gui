#include <cstdint>

#include "../src/game/Game.hpp"

// See https://www.chessprogramming.org/Perft for more information
// collection of perft-related helpers for testing
class Perft{
public:
    static int64_t perft(Game& game, int depth);
};
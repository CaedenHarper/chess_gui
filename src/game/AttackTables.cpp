#include "AttackTables.hpp"
#include "Bitboard.hpp"
#include "Game.hpp"

AttackTables gAtt;

static Bitboard buildRay(int startSq, int dCol, int dRow) noexcept {
    const int startRow = startSq >> 3;
    const int startCol = startSq & 7;

    Bitboard ray{0};
    int curCol = startCol + dCol;
    int curRow = startRow + dRow;
    while (Game::onBoard(curCol, curRow)) {
        ray.setBit(Bitboard::bit(Game::getSquareIndex(curCol, curRow)));
        curCol += dCol;
        curRow += dRow;
    }
    return ray;
}

void initAttackTables() noexcept {
    for (int square = 0; square < Game::NUM_SQUARES; square++) {
        const int col = Game::getCol(square);
        const int row = Game::getRow(square);

        // Knight FROM
        Bitboard knightMoves{0};
        for (int i = 0; i < 8; ++i) {
            const int curCol = col + Game::knightDeltas[i][0];
            const int curRow = row + Game::knightDeltas[i][1];
            if (Game::onBoard(curCol, curRow)) {
                knightMoves.setBit(Bitboard::bit(Game::getSquareIndex(curCol, curRow)));
            }
        }
        gAtt.knightFrom[square] = knightMoves;

        // King FROM
        Bitboard kingMoves{0};
        for (int i = 0; i < 8; ++i) {
            const int curCol = col + Game::kingDeltas[i][0];
            const int curRow = row + Game::kingDeltas[i][1];
            if (Game::onBoard(curCol, curRow)) {
                kingMoves.setBit(Bitboard::bit(Game::getSquareIndex(curCol, curRow)));
            }
        }
        gAtt.kingFrom[square] = kingMoves;

        // Pawn FROM
        Bitboard whitePawnMoves{0};
        if (Game::onBoard(col - 1, row - 1)) {
                whitePawnMoves.setBit(Bitboard::bit(Game::getSquareIndex(col - 1, row - 1)));
        }
        if (Game::onBoard(col + 1, row - 1)) {
            whitePawnMoves.setBit(Bitboard::bit(Game::getSquareIndex(col + 1, row - 1)));
        }
        gAtt.pawnFrom[0][square] = whitePawnMoves;

        Bitboard blackPawnMoves{0};
        if (Game::onBoard(col - 1, row + 1)) {
            blackPawnMoves.setBit(Bitboard::bit(Game::getSquareIndex(col - 1, row + 1)));
        }
        if (Game::onBoard(col + 1, row + 1)) {
            blackPawnMoves.setBit(Bitboard::bit(Game::getSquareIndex(col + 1, row + 1)));
        }
        gAtt.pawnFrom[1][square] = blackPawnMoves;

        // Sliders
        // Rook rays
        gAtt.rookRays[square][0] = buildRay(square,  0, -1); // N
        gAtt.rookRays[square][1] = buildRay(square,  0, +1); // S
        gAtt.rookRays[square][2] = buildRay(square, +1,  0); // E
        gAtt.rookRays[square][3] = buildRay(square, -1,  0); // W

        // Bishop rays: NE, NW, SE, SW
        gAtt.bishopRays[square][0] = buildRay(square, +1, -1); // NE
        gAtt.bishopRays[square][1] = buildRay(square, -1, -1); // NW
        gAtt.bishopRays[square][2] = buildRay(square, +1, +1); // SE
        gAtt.bishopRays[square][3] = buildRay(square, -1, +1); // SW
    }

    // pawnTo
    // populate board with zeros
    for (int color = 0; color < 2; ++color) {
        for (int target = 0; target < 64; ++target) {
            gAtt.pawnTo[color][target].clear();
        }
    }

    for (int color = 0; color < 2; color++) {
        for (int from = 0; from < 64; from++) {
            Bitboard attacks = gAtt.pawnFrom[color][from];
            while (!attacks.empty()) {
                const int target = attacks.popLsb();
                gAtt.pawnTo[color][target].setBit(Bitboard::bit(from));
            }
        }
    }
}

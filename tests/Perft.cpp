#include <cstdint>
#include <iostream>

#include "Perft.hpp"
#include "../src/game/Game.hpp"

// limit of 9,223,372,036,854,775,807
int64_t Perft::perft(Game& game, int depth) { // NOLINT(misc-no-recursion)
    if(depth <= 0) {
        return 1;
    }
    
    int64_t numPositions = 0;
    MoveList moves;
    game.generateAllPseudoLegalMoves(moves);

    for (int i = 0; i < moves.size; i++) {
        const Move& move = moves.data[i];
        const UndoInfo undoInfo = game.getUndoInfo(game.board()[move.targetSquare()]);
        const Color currentTurn = game.currentTurn();
        const Color enemyColor = Game::oppositeColor(currentTurn);

        game.makeMove(move);

        // Check any relevant checks

        const int KING_STARTING_SQUARE = (currentTurn == Color::White) ? Game::WHITE_KING_STARTING_SQUARE : Game::BLACK_KING_STARTING_SQUARE;

        if(move.isKingSideCastle()) {
            const int KINGSIDE_PASSING_SQUARE = (currentTurn == Color::White) ? Game::WHITE_KINGSIDE_PASSING_SQUARE : Game::BLACK_KINGSIDE_PASSING_SQUARE;
            const int KINGSIDE_TARGET_SQUARE = (currentTurn == Color::White) ? Game::WHITE_KINGSIDE_TARGET_SQUARE : Game::BLACK_KINGSIDE_TARGET_SQUARE;
    
            if(
                game.isSquareAttacked(KING_STARTING_SQUARE, enemyColor) ||   // king cant start in check
                game.isSquareAttacked(KINGSIDE_PASSING_SQUARE, enemyColor) ||   // king cant pass through check 
                game.isSquareAttacked(KINGSIDE_TARGET_SQUARE, enemyColor)      // king cant end in check
            ) {
                game.undoMove(move, undoInfo);
                continue;
            }
        }

        if(move.isQueenSideCastle()) {
            const int QUEENSIDE_PASSING_SQUARE = (currentTurn == Color::White) ? Game::WHITE_QUEENSIDE_PASSING_SQUARE : Game::BLACK_QUEENSIDE_PASSING_SQUARE;
            const int QUEENSIDE_TARGET_SQUARE = (currentTurn == Color::White) ? Game::WHITE_QUEENSIDE_TARGET_SQUARE : Game::BLACK_QUEENSIDE_TARGET_SQUARE;
            if(
                game.isSquareAttacked(KING_STARTING_SQUARE, enemyColor) ||   // king cant start in check
                game.isSquareAttacked(QUEENSIDE_PASSING_SQUARE, enemyColor) ||   // king cant pass through check 
                game.isSquareAttacked(QUEENSIDE_TARGET_SQUARE, enemyColor)      // king cant end in check
            ) {
                game.undoMove(move, undoInfo);
                continue;
            }
        }
        
        if (game.isInCheck(currentTurn)) {
            game.undoMove(move, undoInfo);
            continue;
        }

        // no checks, we can continue recursing
        numPositions += perft(game, depth - 1);
        
        game.undoMove(move, undoInfo);
    }

    return numPositions;
}


// Prints each root move and its subtree count; matches Stockfish for debugging
int64_t Perft::perftDivide(Game& game, int depth) {
    if (depth <= 0) {
        return 1;
    }

    int64_t numPositions = 0;
    MoveList moves;
    game.generateAllPseudoLegalMoves(moves);

    for (int i = 0; i < moves.size; i++) {
        const Move& move = moves.data[i];
        const UndoInfo undoInfo = game.getUndoInfo(game.board()[move.targetSquare()]);
        const Color currentTurn = game.currentTurn();
        const Color enemyColor = Game::oppositeColor(currentTurn);

        game.makeMove(move);


        // Check any relevant checks

        const int KING_STARTING_SQUARE = (currentTurn == Color::White) ? Game::WHITE_KING_STARTING_SQUARE : Game::BLACK_KING_STARTING_SQUARE;

        if(move.isKingSideCastle()) {
            const int KINGSIDE_PASSING_SQUARE = (currentTurn == Color::White) ? Game::WHITE_KINGSIDE_PASSING_SQUARE : Game::BLACK_KINGSIDE_PASSING_SQUARE;
            const int KINGSIDE_TARGET_SQUARE = (currentTurn == Color::White) ? Game::WHITE_KINGSIDE_TARGET_SQUARE : Game::BLACK_KINGSIDE_TARGET_SQUARE;
    
            if(
                game.isSquareAttacked(KING_STARTING_SQUARE, enemyColor) ||   // king cant start in check
                game.isSquareAttacked(KINGSIDE_PASSING_SQUARE, enemyColor) ||   // king cant pass through check 
                game.isSquareAttacked(KINGSIDE_TARGET_SQUARE, enemyColor)      // king cant end in check
            ) {
                game.undoMove(move, undoInfo);
                continue;
            }
        }

        if(move.isQueenSideCastle()) {
            const int QUEENSIDE_PASSING_SQUARE = (currentTurn == Color::White) ? Game::WHITE_QUEENSIDE_PASSING_SQUARE : Game::BLACK_QUEENSIDE_PASSING_SQUARE;
            const int QUEENSIDE_TARGET_SQUARE = (currentTurn == Color::White) ? Game::WHITE_QUEENSIDE_TARGET_SQUARE : Game::BLACK_QUEENSIDE_TARGET_SQUARE;
            if(
                game.isSquareAttacked(KING_STARTING_SQUARE, enemyColor) ||   // king cant start in check
                game.isSquareAttacked(QUEENSIDE_PASSING_SQUARE, enemyColor) ||   // king cant pass through check 
                game.isSquareAttacked(QUEENSIDE_TARGET_SQUARE, enemyColor)      // king cant end in check
            ) {
                game.undoMove(move, undoInfo);
                continue;
            }
        }
        
        if (game.isInCheck(currentTurn)) {
            game.undoMove(move, undoInfo);
            continue;
        }

        // no checks, we can continue recursing
        const int64_t moveNodes = perft(game, depth - 1);

        game.undoMove(move, undoInfo);

        std::cout << move.toLongAlgebraic() << ": " << moveNodes << "\n";

        numPositions += moveNodes;
    }

    return numPositions;
}
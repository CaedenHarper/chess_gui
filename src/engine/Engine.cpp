// NOLINTBEGIN(readability-convert-member-functions-to-static) Engines prefer engine.foo() over Engine::foo()

#include "Engine.hpp"

#include <optional>

SearchResult Engine::bestMove(Game& game) {
    return search(game, 4);
}

int Engine::quiesce(Game& game, int alpha, int beta) { // NOLINT(misc-no-recursion)
    const int standPat = evaluatePosition(game);

    if (standPat >= beta) {
        return standPat;  // fail-soft
    }

    if (standPat > alpha) {
        alpha = standPat;
    }

    // generate all legal moves; TODO: faster to have a dedicated generateCaptures() function here
    MoveList moves;
    game.generateLegalMoves(moves);

    for (int i = 0; i < moves.size; ++i) {
        const Move move = moves.data[i];
        // only check captures + promotions; TODO: should we also look at checks?
        if (!(move.isCapture() || move.isPromotion())) {
            continue;
        }

        const UndoInfo undoInfo = game.makeMoveWithUndoInfo(move);
        const int score = -quiesce(game, -beta, -alpha);
        game.undoMove(move, undoInfo);

        if (score >= beta) {
            return score;  // fail-soft
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}


int Engine::evaluatePosition(Game& game) const {
    const int whiteEval = evaluatePieceSum_(game, Color::White);
    const int blackEval = evaluatePieceSum_(game, Color::Black);

    const int totalEval = whiteEval - blackEval;
    // Positive for white, negative for black
    return (game.sideToMove() == Color::White) ? totalEval : -totalEval;
}

SearchResult Engine::search(Game& game, int depth) {
    Move bestMove;

    MoveList moves;
    game.generateLegalMoves(moves);

    // if we don't have moves, we're done; return nullopt
    if (moves.size == 0) {
        if (game.isInCheck(game.sideToMove())) {
            return SearchResult{std::nullopt, -CHECKMATE};
        }

        // not in check; stalemate
        return SearchResult{std::nullopt, STALEMATE};
    }

    // start with the worst possible move
    int bestScore = -CHECKMATE;
    for (int moveIndex = 0; moveIndex < moves.size; moveIndex++) {
        // make move
        const Move move = moves.data[moveIndex];
        const UndoInfo undoInfo = game.makeMoveWithUndoInfo(move);

        // init search with alpha = worst move, beta = best move
        const int score = -alphaBeta_(game, -CHECKMATE, CHECKMATE, depth-1, 1);
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        game.undoMove(move, undoInfo);
    }

    return SearchResult{bestMove, bestScore};
}

int Engine::alphaBeta_(Game& game, int alpha, int beta, int depth, int ply) { // NOLINT(misc-no-recursion)
    if (depth == 0) {
        return quiesce(game, alpha, beta);
    }

    MoveList moves;
    game.generateLegalMoves(moves);
    
    // we don't have any legal moves in the position; return early with checkmate / stalemate
    if(moves.size == 0) {
        // checkmate
        if(game.isInCheck(game.sideToMove())) {
            // Move gets worse if ply is larger
            return -CHECKMATE + ply;
        }

        // not in check; return stalemate
        return STALEMATE;
    }

    for (int moveIndex = 0; moveIndex < moves.size; moveIndex++)  {
        // make move
        const Move move = moves.data[moveIndex];
        const UndoInfo undoInfo = game.makeMoveWithUndoInfo(move);

        const int score = -alphaBeta_(game, -beta, -alpha, depth-1, ply + 1);
        game.undoMove(move, undoInfo);

        if(score > alpha) {
            alpha = score;
        }
        
        if( score >= beta ) {
            return alpha;  // fail soft
        }

    }

    return alpha;
}

// TODO: incrementally update material in Game
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
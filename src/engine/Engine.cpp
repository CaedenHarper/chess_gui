// NOLINTBEGIN(readability-convert-member-functions-to-static) Engines prefer engine.foo() over Engine::foo()

#include "Engine.hpp"

#include <optional>

SearchResult Engine::bestMove(Game& game) {
    return search(game, 6);
}

int Engine::quiesce(Game& game, int alpha, int beta, int ply) { // NOLINT(misc-no-recursion)
    // increment stats
    stats_.qnodes++;

    // we're in check, so position is not quiet for us; we need to resolve the check before continuing
    if(game.isInCheck(game.sideToMove())) {
        MoveList moves;
        game.generatePseudoLegalMoves(moves);

        // order moves to greatly improve alpha-beta pruning
        // TODO: maybe in-check specific move ordering here?
        std::array<int, MoveList::kMaxMoves> indices{};
        orderMoves(game, moves, indices);

        bool legalMoveExists = false;

        for (int moveIndex = 0; moveIndex < moves.size; moveIndex++) {
            // make move from indices
            const Move move = moves.data[indices[moveIndex]];
            const UndoInfo undoInfo = game.getUndoInfo(move);

            game.makeMove(move);

            // this move is not legal
            if(game.doesMovePutUsInCheck(move)) {
                game.undoMove(move, undoInfo);
                continue;
            }

            // we have at least one legal move
            legalMoveExists = true;
            
            const int score = -quiesce(game, -beta, -alpha, ply + 1);
            game.undoMove(move, undoInfo);

            if (score >= beta) {
                return score;  // fail-soft
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        // No legal evasions means we've been checkmated
        if (!legalMoveExists) {
            return -Eval::CHECKMATE + ply;
        }

        return alpha;
    }

    // we're not in check, so we can probe normally
    const int standPat = evaluatePosition(game);
    
    if (standPat >= beta) {
        return standPat;  // fail-soft
    }

    if (standPat > alpha) {
        alpha = standPat;
    }

    // generate all legal moves; TODO: faster to have a dedicated generateCaptures() function here
    MoveList moves;
    game.generatePseudoLegalMoves(moves);

    // order moves to greatly improve alpha-beta pruning
    // TODO: quiesce-specific move ordering here?
    std::array<int, MoveList::kMaxMoves> indices{};
    orderMoves(game, moves, indices);

    for (int moveIndex = 0; moveIndex < moves.size; moveIndex++) {
        // make move from indices
        const Move move = moves.data[indices[moveIndex]];
        const UndoInfo undoInfo = game.getUndoInfo(move);

        // only check captures + promotions; TODO: should we also look at checks?
        if (!(move.isCapture() || move.isPromotion())) {
            continue;
        }

        game.makeMove(move);

        // this move is not legal
        if(game.doesMovePutUsInCheck(move)) {
            game.undoMove(move, undoInfo);
            continue;
        }

        const int score = -quiesce(game, -beta, -alpha, ply + 1);
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
    // Start with piece sum
    int eval = evaluatePieceSum_(game, Color::White);
    eval -= evaluatePieceSum_(game, Color::Black);

    // add piece placement bonus
    eval += evaluatePiecePlacementBonus_(game, Color::White);
    eval -= evaluatePiecePlacementBonus_(game, Color::Black);

    // Positive for white, negative for black
    return (game.sideToMove() == Color::White) ? eval : -eval;
}

SearchResult Engine::search(Game& game, int depth) {
    // reset stats counter once at root
    stats_.clear();

    Move bestMove{};  // NOTE: this starts as a junk move

    bool legalMoveExists = false;
    MoveList moves;
    game.generatePseudoLegalMoves(moves);
    
    // order moves to greatly improve alpha-beta pruning
    std::array<int, MoveList::kMaxMoves> indices{};
    orderMoves(game, moves, indices);

    // start with the worst possible move
    int bestScore = -Eval::CHECKMATE;
    for (int moveIndex = 0; moveIndex < moves.size ; moveIndex++) {
        // make move from indices
        const Move move = moves.data[indices[moveIndex]];
        const UndoInfo undoInfo = game.getUndoInfo(move);
    
        game.makeMove(move);

        // this move is not legal
        if(game.doesMovePutUsInCheck(move)) {
            game.undoMove(move, undoInfo);
            continue;
        }

        // we have at least one legal move
        legalMoveExists = true;

        // init search with alpha = worst move, beta = best move
        const int score = -alphaBeta_(game, -Eval::CHECKMATE, Eval::CHECKMATE, depth-1, 1);
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        game.undoMove(move, undoInfo);
    }

    // if we don't have a legal move, we're done; return nullopt
    if (!legalMoveExists) {
        if (game.isInCheck(game.sideToMove())) {
            return SearchResult{std::nullopt, -Eval::CHECKMATE, stats_};
        }

        // not in check; stalemate
        return SearchResult{std::nullopt, Eval::STALEMATE, stats_};
    }

    return SearchResult{bestMove, bestScore, stats_};
}

void Engine::orderMoves(Game& game, const MoveList& moves, std::array<int, MoveList::kMaxMoves>& indices) {
    const int numMoves = moves.size;

    // return early if we don't have any moves
    if(numMoves == 0) {
        return;
    }
    
    std::array<int, MoveList::kMaxMoves> scores{};
    for(int moveIndex = 0; moveIndex < numMoves; moveIndex++) {
        // init indices values while iterating
        indices[moveIndex] = moveIndex;

        const Move move = moves.data[moveIndex];

        int score = 0;

        // Boost good captures
        if(move.isCapture()) {
            constexpr int CAPTURE_BONUS = 2000;
            score += CAPTURE_BONUS + mvv_lva_bonus(game, move);
        }

        // Boost promotions
        if (move.isPromotion()) {
            constexpr int PROMOTION_BONUS = 1000;
            // queen promotion > others
            const int QUEEN_PROMOTION_BONUS = move.promotion() == Promotion::Queen ? 100 : 0;
            score += PROMOTION_BONUS + QUEEN_PROMOTION_BONUS;
        }

        scores[moveIndex] = score;
    }

    // Selection ordering: bring best score to front, one by one
    for (int picked = 0; picked < numMoves; picked++) {
        int best = picked;
        for (int j = picked + 1; j < numMoves; j++) {
            if (scores[j] > scores[best]) {
                best = j;
            }
        }
        if (best != picked) {
            std::swap(scores[picked], scores[best]);
            std::swap(indices[picked], indices[best]);
        }
    }
}

int Engine::alphaBeta_(Game& game, int alpha, int beta, int depth, int ply) { // NOLINT(misc-no-recursion)
    stats_.nodes++;

    if (depth == 0) {
        return quiesce(game, alpha, beta, ply + 1);
    }

    bool legalMoveExists = false;
    MoveList moves;
    game.generatePseudoLegalMoves(moves);

    // order moves to greatly improve alpha-beta pruning
    std::array<int, MoveList::kMaxMoves> indices{};
    orderMoves(game, moves, indices);

    for (int moveIndex = 0; moveIndex < moves.size; moveIndex++)  {
        // make move from indices
        const Move move = moves.data[indices[moveIndex]];
        const UndoInfo undoInfo = game.getUndoInfo(move);

        game.makeMove(move);

        // this move is not legal
        if(game.doesMovePutUsInCheck(move)) {
            game.undoMove(move, undoInfo);
            continue;
        }

        // we have at least one legal move
        legalMoveExists = true;

        const int score = -alphaBeta_(game, -beta, -alpha, depth-1, ply + 1);
        game.undoMove(move, undoInfo);

        if(score > alpha) {
            alpha = score;
        }
        
        if( score >= beta ) {
            return alpha;  // fail soft
        }

    }

    // we don't have any legal moves in the position; return with checkmate / stalemate
    // we put this after the loop so we can generate pseudo legal moves at first, which is faster
    if(!legalMoveExists) {
        // checkmate
        if(game.isInCheck(game.sideToMove())) {
            // Move gets worse if ply is larger
            return -Eval::CHECKMATE + ply;
        }

        // not in check; return stalemate
        return Eval::STALEMATE;
    }

    return alpha;
}

// TODO: incrementally update material in Game
int Engine::evaluatePieceSum_(Game& game, const Color color) const {
    const bool isWhite = color == Color::White;
    int sum = 0;
    sum += popcount_((isWhite ? game.bbWhitePawns() : game.bbBlackPawns()).raw()) * Eval::PAWN_COST;
    sum += popcount_((isWhite ? game.bbWhiteKnights() : game.bbBlackKnights()).raw()) * Eval::KNIGHT_COST;
    sum += popcount_((isWhite ? game.bbWhiteBishops() : game.bbBlackBishops()).raw()) * Eval::BISHOP_COST;
    sum += popcount_((isWhite ? game.bbWhiteRooks() : game.bbBlackRooks()).raw()) * Eval::ROOK_COST;
    sum += popcount_((isWhite ? game.bbWhiteQueens() : game.bbBlackQueens()).raw()) * Eval::QUEEN_COST;
    return sum;
}

int Engine::evaluatePiecePlacementBonus_(Game& game, const Color color) const {
    const bool isWhite = (color == Color::White);

    int score = 0;

    Bitboard currentBitboard;

    // Pawns
    currentBitboard = isWhite ? game.bbWhitePawns() : game.bbBlackPawns();
    while (!currentBitboard.empty()) {
        const int square = currentBitboard.popLsb();
        score += Eval::BLACK_PAWN_EVAL_TABLE[isWhite ? Utils::mirrorSquare(square) : square];
    }

    // Knights
    currentBitboard = isWhite ? game.bbWhiteKnights() : game.bbBlackKnights();
    while (!currentBitboard.empty()) {
        const int square = currentBitboard.popLsb();
        score += Eval::BLACK_KNIGHT_EVAL_TABLE[isWhite ? Utils::mirrorSquare(square) : square];
    }

    // Bishops
    currentBitboard = isWhite ? game.bbWhiteBishops() : game.bbBlackBishops();
    while (!currentBitboard.empty()) {
        const int square = currentBitboard.popLsb();
        score += Eval::BLACK_BISHOP_EVAL_TABLE[isWhite ? Utils::mirrorSquare(square) : square];
    }

    // Rooks
    currentBitboard = isWhite ? game.bbWhiteRooks() : game.bbBlackRooks();
    while (!currentBitboard.empty()) {
        const int square = currentBitboard.popLsb();
        score += Eval::BLACK_ROOK_EVAL_TABLE[isWhite ? Utils::mirrorSquare(square) : square];
    }

    // Queens
    currentBitboard = isWhite ? game.bbWhiteQueens() : game.bbBlackQueens();
    while (!currentBitboard.empty()) {
        const int square = currentBitboard.popLsb();
        score += Eval::BLACK_QUEEN_EVAL_TABLE[isWhite ? Utils::mirrorSquare(square) : square];
    }

    // King
    currentBitboard = isWhite ? game.bbWhiteKing() : game.bbBlackKing();
    while (!currentBitboard.empty()) {
        const int square = currentBitboard.popLsb();
        score += Eval::BLACK_KING_EVAL_TABLE[isWhite ? Utils::mirrorSquare(square) : square];
    }

    return score;
}


// NOLINTEND(readability-convert-member-functions-to-static)
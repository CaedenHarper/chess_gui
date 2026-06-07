#include "GameScreen.hpp"

#include "GameInputHandler.hpp"
#include "GameRenderer.hpp"

#include <iostream>

void GameScreen::update() {
    handleEngineTurn();
    updateMoveAnimation();
}

void GameScreen::handleEngineTurn() {
    if(!isEngineTurn()) {
        return;
    }

    if(!engineThread_.thinking) {
        startEngineSearch();
        return;
    }

    // Don't apply engine result if we're paused
    if(isPaused_) {
        return;
    }

    if(auto result = tryTakeEngineResult()) {
        applyEngineResult(*result);
    }
}

void GameScreen::updateMoveAnimation() {
    if(!moveAnimation_) {
        return;
    }

    if(moveAnimation_->clock.getElapsedTime() >= moveAnimation_->duration) {
        moveAnimation_.reset();
    }
}

ScreenCommand GameScreen::handleEvent(const sf::Event& event) {
    const GameInputResult result = inputHandler_.handleEvent(
        event, game_, playerColor_, playerColor_, inputMode()
    ); // take playercolor == displaycolor
    switch(result.type()) {
        case GameInputResult::Type::None:
        case GameInputResult::Type::InvalidMove: // TODO: consider an invalid move sound
            break;
        case GameInputResult::Type::MoveMade:
            pieceMovementSound_.play();
            break;
        case GameInputResult::Type::RedHighlight:
            if(!result.square()) {
                assert(false);
                break;
            }

            redHighlightSquares_.at(*result.square()) = !redHighlightSquares_.at(*result.square());
            break;
        case GameInputResult::Type::Pause:
            isPaused_ = !isPaused_;
            break;
    }

    if(result.clearRedHighlights()) {
        redHighlightSquares_.fill(false);
    }

    return ScreenCommand::None;
}

void GameScreen::render() {
    const GameRenderState state{
        playerColor_,
        playerColor_, // take displayColor == playerColor
        inputHandler_.heldPiece(),
        currentEval_,
        engineSearchTime(),
        engineThread_.thinking,
        redHighlightSquares_,
        moveAnimation_ ? &*moveAnimation_ : nullptr, // only pass pointer here if moveAnimation_ exists
        isPaused_
    };
    renderer_.render(game_, state);
}

void GameScreen::startEngineSearch() {
    assert(!engineThread_.thread.joinable());

    game_.copyInto(engineThread_.gameCopy);

    engineThread_.thinking = true;
    engineThread_.resultReady = false;
    engineThread_.searchClock.restart();

    engineThread_.thread = std::thread([this]() mutable {
        const auto result = engine_.bestMove(engineThread_.gameCopy);

        {
            const std::scoped_lock lock(engineThread_.mutex);
            engineThread_.result = result;
            engineThread_.resultReady = true;
        }
    });
}

std::optional<SearchResult> GameScreen::tryTakeEngineResult() {
    if(!engineThread_.thinking) {
        return std::nullopt;
    }

    std::optional<SearchResult> result;

    {
        const std::scoped_lock lock(engineThread_.mutex);

        if(!engineThread_.resultReady) {
            return std::nullopt;
        }

        result = engineThread_.result;
        engineThread_.resultReady = false;
    }

    if(engineThread_.thread.joinable()) {
        engineThread_.thread.join();
    }

    engineThread_.thinking = false;
    engineThread_.lastSearchDuration = engineThread_.searchClock.getElapsedTime();

    return result;
}

void GameScreen::applyEngineResult(const SearchResult& result) {
    const auto [possibleEngineMove, possibleCurrentEval, possibleCurrentStats] = result;

    if(!possibleEngineMove) {
        return;
    }

    const Move engineMove = *possibleEngineMove;
    const int fromSquare = engineMove.sourceSquare();
    const int toSquare = engineMove.targetSquare();
    const Piece piece = game_.pieceAtSquareForGui(fromSquare);

    if(!game_.tryMove(engineMove)) {
        std::cerr << "Engine tried to make move: " << engineMove.to_string(game_);
        assert(false);
        return;
    }

    currentEval_ = possibleCurrentEval;
    currentStats_ = possibleCurrentStats;

    moveAnimation_ = MoveAnimation{piece, fromSquare, toSquare, sf::Clock{}, sf::milliseconds(200)};

    pieceMovementSound_.play();
}

bool GameScreen::isEngineTurn() {
    return !game_.isFinished() && game_.sideToMove() != playerColor_;
}

bool GameScreen::isPlayerTurn() {
    return !game_.isFinished() && game_.sideToMove() == playerColor_;
}

sf::Time GameScreen::engineSearchTime() const {
    if(engineThread_.thinking) {
        return engineThread_.searchClock.getElapsedTime();
    }

    return engineThread_.lastSearchDuration;
}

InputMode GameScreen::inputMode() {
    if(isPaused_) {
        return InputMode::Disabled;
    }

    if(!isPlayerTurn()) {
        return InputMode::BoardAnnotationsOnly;
    }

    return InputMode::FullGameplay;
}
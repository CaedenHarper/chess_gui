#include "GameScreen.hpp"

#include <iostream>

void GameScreen::handleEngineTurn() {
    if(!isEngineTurn()) {
        return;
    }

    if(!engineThread_.thinking) {
        startEngineSearch();
        return;
    }

    if(auto result = tryTakeEngineResult()) {
        applyEngineResult(*result);
    }
}

ScreenCommand GameScreen::handleEvent(const sf::Event& event) {
    const InputMode mode{isPlayerTurn() ? InputMode::FullGameplay : InputMode::BoardAnnotationsOnly};
    const GameInputResult result =
        inputHandler_.handleEvent(event, game_, playerColor_, playerColor_, mode); // take playercolor == displaycolor
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

            redHighlightSquares.at(*result.square()) = !redHighlightSquares.at(*result.square());
            break;
    }

    if(result.clearRedHighlights()) {
        redHighlightSquares.fill(false);
    }

    return ScreenCommand::None;
}

void GameScreen::update() {
    handleEngineTurn();
}

void GameScreen::render() {
    const GameRenderState state{
        playerColor_,
        inputHandler_.heldPiece(),
        currentEval,
        currentStats,
        engineSearchTime(),
        engineThread_.thinking,
        redHighlightSquares
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

    if(!game_.tryMove(engineMove)) {
        std::cerr << "Engine tried to make move: " << engineMove.to_string(game_);
        assert(false);
        return;
    }

    currentEval = possibleCurrentEval;
    currentStats = possibleCurrentStats;

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
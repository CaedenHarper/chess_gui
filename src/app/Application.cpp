#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "../engine/Engine.hpp"
#include "../game/Game.hpp"
#include "../gui/InputHandler.hpp"
#include "../gui/Renderer.hpp"

#include "Application.hpp"


void Application::run() {
    while (window_.isOpen()) {
        handleEngineTurn();
        handleEvents();
        render();
    }
}

void Application::handleEngineTurn() {
    if(!isEngineTurn()) {
        return;
    }
    
    if (!engineThread_.thinking) {
        startEngineSearch();
        return;
    }

    if (auto result = tryTakeEngineResult()) {
        applyEngineResult(*result);
    }
}

void Application::handleEvents() {
    while (const std::optional<sf::Event> event = window_.pollEvent()) {
        if(!event) {
            continue;
        }

        handleEvent(*event);
    }
}

void Application::handleEvent(sf::Event event) {
    // Window closing event is special and should be handled here, not in InputHandler
    if (event.is<sf::Event::Closed>()) {
        window_.close();
        return;
    }

    const InputMode mode{isPlayerTurn() ? InputMode::FullGameplay : InputMode::BoardAnnotationsOnly};
    const InputResult result = inputHandler_.handleEvent(event, game_, playerColor_, mode);
    switch(result.type()) {
        case InputResult::Type::None:
        case InputResult::Type::InvalidMove: // TODO: consider an invalid move sound
            break;
        case InputResult::Type::MoveMade:
            pieceMovementSound_.play();
            break;
        case InputResult::Type::RedHighlight:
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
}

void Application::render() {
    const RenderState state{
        playerColor_,
        inputHandler_.heldPiece(),
        currentEval,
        currentStats,
        redHighlightSquares
    };
    renderer_.render(game_, state);
}

void Application::startEngineSearch() {
    assert(!engineThread_.thread.joinable());

    game_.copyInto(engineThread_.gameCopy);

    engineThread_.thinking = true;
    engineThread_.resultReady = false;

    engineThread_.thread = std::thread(
        [this]() mutable {
            const auto result = engine_.bestMove(engineThread_.gameCopy);

            {
                const std::scoped_lock lock(engineThread_.mutex);
                engineThread_.result = result;
                engineThread_.resultReady = true;
            }
        }
    );
}

std::optional<SearchResult> Application::tryTakeEngineResult() {
    if (!engineThread_.thinking) {
        return std::nullopt;
    }

    std::optional<SearchResult> result;

    {
        const std::scoped_lock lock(engineThread_.mutex);

        if (!engineThread_.resultReady) {
            return std::nullopt;
        }

        result = engineThread_.result;
        engineThread_.resultReady = false;
    }

    if (engineThread_.thread.joinable()) {
        engineThread_.thread.join();
    }

    engineThread_.thinking = false;

    return result;
}

void Application::applyEngineResult(const SearchResult& result) {
    const auto [possibleEngineMove, possibleCurrentEval, possibleCurrentStats] = result;

    if (!possibleEngineMove) {
        // No legal move. Ideally mark/show game over, not just return.
        // game_.finish();
        return;
    }

    const Move engineMove = *possibleEngineMove;

    if (!game_.tryMove(engineMove)) {
        std::cerr << "Engine tried to make move: "
                  << engineMove.to_string(game_);
        assert(false);
        return;
    }

    currentEval = possibleCurrentEval;
    currentStats = possibleCurrentStats;

    pieceMovementSound_.play();
}

bool Application::isEngineTurn() {
    return !game_.isFinished() && game_.sideToMove() != playerColor_;
}

bool Application::isPlayerTurn() {
    return !game_.isFinished() && game_.sideToMove() == playerColor_;
}
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
    // main game loop
    while (window_.isOpen()) {
        const bool isEngineTurn = !game_.isFinished() && game_.sideToMove() != playerColor_;

        // Start search
        if (isEngineTurn && !engineThread_.thinking) {
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

        if (isEngineTurn && engineThread_.thinking) {
            bool ready = false;
            SearchResult result{};

            {
                const std::scoped_lock lock(engineThread_.mutex);

                if (engineThread_.resultReady) {
                    ready = true;
                    result = engineThread_.result;
                    engineThread_.resultReady = false;
                }
            }

            if (ready) {
                if (engineThread_.thread.joinable()) {
                    engineThread_.thread.join();
                }
                
                const auto [possibleEngineMove, possibleCurrentEval, possibleCurrentStats] = result;

                if(!possibleEngineMove) {
                    // game is finished
                    break;
                }
                const Move engineMove = possibleEngineMove.value();
                // try to make move and post error message if move could not be made
                if(!game_.tryMove(engineMove)) {
                    std::cerr << "Engine tried to make move: " << engineMove.to_string(game_);
                    assert(false);
                }
                // This is a valid move, we can make it and update the stats
                currentEval = possibleCurrentEval;
                currentStats = possibleCurrentStats;

                pieceMovementSound_.play();

                // Move made; done thinking
                engineThread_.thinking = false;
                engineThread_.resultReady = false;
            }
        }
        
        handleEvents();
        
        const RenderState state{inputHandler_.heldPiece(), currentEval, currentStats, redHighlightSquares};
        renderer_.render(game_, state);
    }
}

void Application::handleEvents() {
    const bool isPlayerTurn = !game_.isFinished() && game_.sideToMove() == playerColor_;

    while (const std::optional<sf::Event> event = window_.pollEvent()) {
        // Window closing event is special and should be handled here, not in InputHandler
        if (event->is<sf::Event::Closed>()) {
            window_.close();
            continue;
        }

        // Skip any chess inputs if it's not the player's turn
        if(!isPlayerTurn) {
            continue;
        }

        const InputResult result = inputHandler_.handleEvent(event.value(), game_);
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
    }
}
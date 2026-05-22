#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "../engine/Engine.hpp"
#include "../game/Game.hpp"
#include "../game/Utils.hpp"
#include "../gui/InputHandler.hpp"
#include "../gui/Renderer.hpp"
#include "AppUtils.hpp"

#include "Application.hpp"


void Application::run() {
    // init sounds
    // TODO: potentially throw / recover from file missing
    const sf::SoundBuffer PIECE_MOVEMENT_BUFFER{std::string{AppUtils::PIECE_MOVEMENT_SOUND_FILE}};
    sf::Sound PIECE_MOVEMENT_SOUND{PIECE_MOVEMENT_BUFFER};
    PIECE_MOVEMENT_SOUND.setVolume(AppUtils::VOLUME_PERCENTAGE);

    // TODO: we assume player 1 (non-engine) is always white; create a way to change this
    const Color player1Color = Color::White; 

    // init engine's current eval of the position to show to player
    int currentEval = 0;
    SearchStats currentStats{};

    // init highlighted squares which are passed to renderer
    std::array<bool, Utils::NUM_SQUARES> redHighlightSquares{};

    // Engine thread managers
    std::thread engineThread;
    std::mutex engineMutex;
    Game engineGameCopy;

    bool engineThinking = false;
    bool engineResultReady = false;
    SearchResult engineResult;

    // main game loop
    while (window_.isOpen()) {
        const bool isEngineTurn = !game_.isFinished() && game_.sideToMove() != player1Color;
        const bool isPlayerTurn = !game_.isFinished() && game_.sideToMove() == player1Color;

        // Start search
        if (isEngineTurn && !engineThinking) {
            game_.copyInto(engineGameCopy);

            engineThinking = true;
            engineResultReady = false;

            engineThread = std::thread(
                [this,
                &engineMutex,
                &engineResult,
                &engineResultReady,
                &engineGameCopy]() mutable {
                    const auto result = engine_.bestMove(engineGameCopy);

                    // Lock to share result
                    {
                        const std::scoped_lock lock(engineMutex);
                        engineResult = result;
                        engineResultReady = true;
                    }
                }
            );
        }

        if (isEngineTurn && engineThinking) {
            bool ready = false;
            SearchResult result{};

            {
                const std::scoped_lock lock(engineMutex);

                if (engineResultReady) {
                    ready = true;
                    result = engineResult;
                    engineResultReady = false;
                }
            }

            if (ready) {
                if (engineThread.joinable()) {
                    engineThread.join();
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

                PIECE_MOVEMENT_SOUND.play();

                // Move made; done thinking
                engineThinking = false;
                engineResultReady = false;
            }
        }
        
        // handle events
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
                    PIECE_MOVEMENT_SOUND.play();
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
        
        const RenderState state{inputHandler_.heldPiece(), currentEval, currentStats, redHighlightSquares};
        renderer_.render(game_, state);
    }
}
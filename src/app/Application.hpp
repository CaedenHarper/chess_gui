#pragma once

#include <SFML/System/Vector2.hpp>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <mutex>
#include <thread>

#include "../game/Game.hpp"
#include "../engine/Engine.hpp"
#include "../gui/Renderer.hpp"

#include "AppUtils.hpp"

struct EngineThreadState {
    std::thread thread;
    std::mutex mutex;

    Game gameCopy;

    bool thinking = false;
    bool resultReady = false;
    SearchResult result{};

    ~EngineThreadState() {
        if (thread.joinable()) {
            thread.join();
        }
    }

    EngineThreadState() = default;

    EngineThreadState(const EngineThreadState&) = delete;
    EngineThreadState& operator=(const EngineThreadState&) = delete;

    EngineThreadState(EngineThreadState&&) = delete;
    EngineThreadState& operator=(EngineThreadState&&) = delete;
} __attribute__((packed));

class Application{
public:
    explicit Application(Color playerColor) :
    window_{sf::VideoMode{{AppUtils::STARTING_WINDOW_WIDTH, AppUtils::STARTING_WINDOW_HEIGHT}}, std::string{AppUtils::WINDOW_TITLE}},
    renderer_{&window_},

    playerColor_{playerColor},

    pieceMovementSoundBuffer_{std::string{AppUtils::PIECE_MOVEMENT_SOUND_FILE}}, 
    pieceMovementSound_{pieceMovementSoundBuffer_} {
        game_.loadFEN(std::string{Utils::STARTING_FEN});

        window_.setVerticalSyncEnabled(true);

        pieceMovementSound_.setVolume(AppUtils::VOLUME_PERCENTAGE);
    }

    void run();

private:
    void handleEvents();

    Game game_;
    sf::RenderWindow window_;
    Engine engine_;
    EngineThreadState engineThread_;
    Renderer renderer_;
    InputHandler inputHandler_;

    Color playerColor_;

    sf::SoundBuffer pieceMovementSoundBuffer_;
    sf::Sound pieceMovementSound_;

    std::array<bool, Utils::NUM_SQUARES> redHighlightSquares{};

    int currentEval{};
    SearchStats currentStats;
};
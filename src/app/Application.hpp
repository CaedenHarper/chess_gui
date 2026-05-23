#pragma once

#include <SFML/System/Vector2.hpp>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <mutex>
#include <thread>

#include "../game/Game.hpp"
#include "../engine/Engine.hpp"
#include "../gui/GameRenderer.hpp"

#include "AppUtils.hpp"

struct EngineThreadState {
    std::thread thread;
    std::mutex mutex;

    Game gameCopy;

    bool thinking = false;
    bool resultReady = false;
    SearchResult result{};

    sf::Clock searchClock;
    sf::Time lastSearchDuration = sf::Time::Zero;

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
        
        window_.setFramerateLimit(AppUtils::MAX_FPS);

        pieceMovementSound_.setVolume(AppUtils::VOLUME_PERCENTAGE);
    }

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    ~Application() {
        // Make sure engine thread is finished before deleting
        if (engineThread_.thread.joinable()) {
            engineThread_.thread.join();
        }
    }
    
    void run();

private:
    void handleEngineTurn();
    void handleEvents();
    void handleEvent(sf::Event event);
    void render();

    void startEngineSearch();
    std::optional<SearchResult> tryTakeEngineResult();
    void applyEngineResult(const SearchResult& result);

    bool isEngineTurn();
    bool isPlayerTurn();
    sf::Time engineSearchTime() const;

    Game game_;
    sf::RenderWindow window_;
    Engine engine_;
    EngineThreadState engineThread_;
    GameRenderer renderer_;
    InputHandler inputHandler_;

    Color playerColor_;

    sf::SoundBuffer pieceMovementSoundBuffer_;
    sf::Sound pieceMovementSound_;

    std::array<bool, Utils::NUM_SQUARES> redHighlightSquares{};

    int currentEval{};
    SearchStats currentStats;
};
#pragma once

#include <SFML/Audio.hpp>

#include <mutex>
#include <thread>

#include "../game/Game.hpp"
#include "../engine/Engine.hpp"
#include "GameRenderer.hpp"

#include "InputHandler.hpp"
#include "SoundUtils.hpp"

#include "Screen.hpp"

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

class GameScreen : public Screen {
public:
    GameScreen(sf::RenderWindow& window, Color playerColor)
    : renderer_{&window},
      playerColor_{playerColor},
      pieceMovementSoundBuffer_{std::string{SoundUtils::PIECE_MOVEMENT_SOUND_FILE}}, 
      pieceMovementSound_{pieceMovementSoundBuffer_}
    {
        game_.loadFEN(std::string{Utils::STARTING_FEN});
        pieceMovementSound_.setVolume(SoundUtils::VOLUME_PERCENTAGE);
    }

    ~GameScreen() override {
        // Make sure engine thread is finished before deleting
        if (engineThread_.thread.joinable()) {
            engineThread_.thread.join();
        }
    }

    GameScreen(const GameScreen& other) = delete;
    GameScreen& operator=(const GameScreen& other) = delete;
    GameScreen(GameScreen&& other) = delete;
    GameScreen& operator=(GameScreen&& other) = delete;

    void handleEvent(const sf::Event& event) override;
    void update() override;
    void render() override;
private:
    void handleEngineTurn();

    void startEngineSearch();
    std::optional<SearchResult> tryTakeEngineResult();
    void applyEngineResult(const SearchResult& result);

    bool isEngineTurn();
    bool isPlayerTurn();
    sf::Time engineSearchTime() const;

    Game game_;
    Engine engine_;
    EngineThreadState engineThread_;
    InputHandler inputHandler_;
    GameRenderer renderer_;

    Color playerColor_;

    sf::SoundBuffer pieceMovementSoundBuffer_;
    sf::Sound pieceMovementSound_;

    std::array<bool, Utils::NUM_SQUARES> redHighlightSquares{};

    int currentEval{};
    SearchStats currentStats;
};
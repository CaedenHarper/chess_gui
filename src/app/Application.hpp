#pragma once

#include <SFML/System/Vector2.hpp>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "../game/Game.hpp"
#include "../engine/Engine.hpp"
#include "../gui/Renderer.hpp"

#include "AppUtils.hpp"

class Application{
public:
    Application() :
    window_{sf::VideoMode{{AppUtils::STARTING_WINDOW_WIDTH, AppUtils::STARTING_WINDOW_HEIGHT}}, std::string{AppUtils::WINDOW_TITLE}},
    renderer_{&window_} {
        game_.loadFEN(std::string{Utils::STARTING_FEN});
        window_.setVerticalSyncEnabled(true);
    }

    void run();

private:
    Game game_;
    sf::RenderWindow window_;
    Engine engine_;
    Renderer renderer_;
    InputHandler inputHandler_;
};
#pragma once

#include <SFML/System/Vector2.hpp>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>

#include "../gui/Screen.hpp"
#include "AppUtils.hpp"

class Application{
public:
    explicit Application()
        : window_{sf::VideoMode{{AppUtils::STARTING_WINDOW_WIDTH, AppUtils::STARTING_WINDOW_HEIGHT}}, std::string{AppUtils::WINDOW_TITLE}}
    {
        window_.setFramerateLimit(AppUtils::MAX_FPS);
        switchTo(ScreenID::MainMenu);
    }

    void run();

private:
    void handleEvents();
    void update();
    void render();
    void switchTo(ScreenID screen);

    sf::RenderWindow window_;
    std::unique_ptr<Screen> currentScreen_;
};
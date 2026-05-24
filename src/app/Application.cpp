#include "Application.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "../gui/game/GameScreen.hpp"
#include "../gui/main_menu/MainMenuScreen.hpp"

#include <optional>


void Application::run() {
    while(window_.isOpen()) {
        handleEvents();
        update();
        render();
    }
}

void Application::handleEvents() {
    while(const std::optional<sf::Event> event = window_.pollEvent()) {
        if(!event) {
            continue;
        }

        if(event->is<sf::Event::Closed>()) {
            window_.close();
            return;
        }

        currentScreen_->handleEvent(*event);
    }
}

void Application::update() {
    currentScreen_->update();
}

void Application::render() {
    currentScreen_->render();
}

void Application::switchTo(ScreenID screen) {
    switch(screen) {
        case ScreenID::MainMenu:
            currentScreen_ = std::make_unique<MainMenuScreen>(window_);
            break;
        case ScreenID::Game: {
            const Color playerColor = Color::White;
            currentScreen_ = std::make_unique<GameScreen>(window_, playerColor);
            break;
        }
        case ScreenID::GameOver:
            break;
    }
}
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

        const ScreenCommand command = currentScreen_->handleEvent(*event);
        handleScreenCommand(command);
    }
}

void Application::update() {
    currentScreen_->update();
}

void Application::render() {
    currentScreen_->render();
}

void Application::switchToMainMenu() {
    currentScreen_ = std::make_unique<MainMenuScreen>(window_);
}

void Application::switchToGame(Color color) {
    if(color == Color::White) {
        currentScreen_ = std::make_unique<GameScreen>(window_, Color::White);
        return;
    }

    if(color == Color::Black) {
        currentScreen_ = std::make_unique<GameScreen>(window_, Color::Black);
        return;
    }

    assert(false);
}

void Application::handleScreenCommand(ScreenCommand command) {
    switch(command) {
        case ScreenCommand::None:
            break;
        case ScreenCommand::StartWhiteGame:
            switchToGame(Color::White);
            break;
        case ScreenCommand::StartBlackGame:
            switchToGame(Color::Black);
            break;
    }
}
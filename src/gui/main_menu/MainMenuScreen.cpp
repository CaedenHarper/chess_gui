#include "MainMenuScreen.hpp"

#include "MainMenuInputHandler.hpp"
#include "MainMenuRenderer.hpp"

ScreenCommand MainMenuScreen::handleEvent(const sf::Event& event) {
    const MainMenuInputResult result = inputHandler_.handleEvent(event);
    switch(result) {
        case MainMenuInputResult::None:
            return ScreenCommand::None;
        case MainMenuInputResult::WhiteButton:
            return ScreenCommand::StartWhiteGame;
        case MainMenuInputResult::BlackButton:
            return ScreenCommand::StartBlackGame;
    }
}

void MainMenuScreen::update() {
}

void MainMenuScreen::render() {
    renderer_.render();
}

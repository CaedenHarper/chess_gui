#include "MainMenuInputHandler.hpp"

#include "MainMenuLayout.hpp"

MainMenuInputResult MainMenuInputHandler::handleEvent(const sf::Event& event) {
    if(const auto* mouseClicked = event.getIf<sf::Event::MouseButtonPressed>()) {
        return mouseClickEvent(*mouseClicked);
    }

    return MainMenuInputResult::None;
}

MainMenuInputResult MainMenuInputHandler::mouseClickEvent(const sf::Event::MouseButtonPressed& event) {
    if(event.button == sf::Mouse::Button::Left) {
        return leftClickEvent(event);
    }

    return MainMenuInputResult::None;
}

MainMenuInputResult MainMenuInputHandler::leftClickEvent(const sf::Event::MouseButtonPressed& event) {
    // Convert from int to float
    const sf::Vector2f mousePos{static_cast<float>(event.position.x), static_cast<float>(event.position.y)};

    if(MainMenuLayout::whiteButton.contains(mousePos)) {
        return MainMenuInputResult::WhiteButton;
    }

    if(MainMenuLayout::blackButton.contains(mousePos)) {
        return MainMenuInputResult::BlackButton;
    }

    return MainMenuInputResult::None;
}
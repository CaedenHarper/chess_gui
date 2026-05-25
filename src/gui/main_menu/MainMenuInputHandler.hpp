#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

enum MainMenuInputResult { None, WhiteButton, BlackButton };

class MainMenuInputHandler {
public:
    MainMenuInputHandler() = default;

    MainMenuInputResult handleEvent(const sf::Event& event);

private:
};
#pragma once

#include <SFML/Graphics.hpp>

#include "MainMenuLayout.hpp"

class MainMenuRenderer {
public:
    explicit MainMenuRenderer(sf::RenderWindow* window) : window_{window} {
    }

    void render();

private:
    void drawTitle();
    void drawWhiteButton();
    void drawBlackButton();

    void clearWindow(const sf::Color& backgroundColor);
    void drawText(const std::string& str, const sf::Vector2f& position, int size);
    void drawButton(MainMenuButton button);

    sf::RenderWindow* window_;
};
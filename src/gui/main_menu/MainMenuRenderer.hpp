#pragma once

#include <SFML/Graphics.hpp>

#include "MainMenuLayout.hpp"

class MainMenuRenderer {
public:
    explicit MainMenuRenderer(sf::RenderWindow* window) : window_{window} {
    }

    void render();

private:
    void drawBackground();
    void drawTitle();
    void drawSubtitle();
    void drawFooter();

    void clearWindow(const sf::Color& backgroundColor);
    void drawText(
        const std::string& str,
        const sf::Vector2f& position,
        int size,
        const sf::Color& color,
        bool centered
    );
    void drawButton(const MainMenuButton& button);

    sf::RenderWindow* window_;
};
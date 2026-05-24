#pragma once

#include <SFML/Graphics.hpp>

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
    void drawButton(
        const sf::Vector2f& size,
        const sf::Vector2f& position,
        const sf::Color& fillColor,
        const sf::Color& outlineColor,
        float outlineThickness
    );
    void drawButtonText(
        const std::string& str,
        const sf::Vector2f& buttonPosition,
        int textSize,
        const sf::Vector2f& buttonSize
    );

    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
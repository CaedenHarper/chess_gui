#pragma once

#include <SFML/Graphics.hpp>

class MainMenuRenderer {
public:
    explicit MainMenuRenderer(sf::RenderWindow* window) : window_{window} {
    }

    void render();

private:
    void clearWindow(sf::Color backgroundColor);

    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
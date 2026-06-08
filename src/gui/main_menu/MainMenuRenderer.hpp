#pragma once

#include <SFML/Graphics.hpp>

class MainMenuRenderer {
public:
    explicit MainMenuRenderer(sf::RenderWindow* window) : window_{window} {
    }

    void render();

private:
    void drawBackground();
    void drawTitle();

    sf::RenderWindow* window_;
};
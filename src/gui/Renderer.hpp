#pragma once

#include <SFML/Graphics.hpp>

#include "BoardView.hpp"
#include "../engine/Engine.hpp"

class Renderer {
public:
    Renderer(BoardView* board, sf::RenderWindow* window) : board_{board}, window_{window} {

    };

    void render(std::optional<int> heldSquare, bool isDragging, int currentEval, SearchStats currentStats, sf::Vector2f dragPosPx, Color sideToMove);

private:
    const BoardView* board_;
    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
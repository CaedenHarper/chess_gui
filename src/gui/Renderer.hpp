#pragma once

#include <SFML/Graphics.hpp>

#include "BoardView.hpp"
#include "../engine/Engine.hpp"

#include "InputHandler.hpp"


struct RenderState {
    std::optional<HeldPieceState> heldPiece;
    int currentEval{};
    SearchStats currentStats;
    Color sideToMove{};
} __attribute__((aligned(64))); // NOLINT[magic numbers] align to 64 bytes

class Renderer {
public:
    Renderer(BoardView* board, sf::RenderWindow* window) : board_{board}, window_{window} {}

    void render(const RenderState& state);

private:
    void clearWindow(sf::Color backgroundColor);
    void drawBoard(std::optional<HeldPieceState> heldPiece);
    void drawEngineEval(int currentEval, Color sideToMove);
    void drawEngineStats(SearchStats currentStats);

    const BoardView* board_;
    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
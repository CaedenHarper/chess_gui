#pragma once

#include <SFML/Graphics.hpp>

#include "BoardView.hpp"
#include "../engine/Engine.hpp"

#include "InputHandler.hpp"


struct RenderState {
    std::optional<HeldPieceState> heldPiece;

    int currentEval{};
    SearchStats currentStats;

    std::optional<int> redHighlightSquare;
    std::optional<int> selectedSquare;
} __attribute__((aligned(128))); // NOLINT[magic numbers] align to 64 bytes

class Renderer {
public:
    Renderer(BoardView* board, sf::RenderWindow* window) : board_{board}, window_{window} {}

    void render(const Game& game, const RenderState& state);

private:
    void clearWindow(sf::Color backgroundColor);
    void drawBoard(std::optional<HeldPieceState> heldPiece);
    void drawDraggedPiece(std::optional<HeldPieceState> heldPiece);
    void drawEngineEval(int currentEval, Color sideToMove);
    void drawEngineStats(SearchStats currentStats);
    void drawSquare(int squareIndex);
    void drawSquare(int squareIndex, std::optional<Highlight> highlight);

    void drawHighlights(RenderState state);
    void drawSelectedSquareHighlights(std::optional<int> selectedSquare);
    void drawLegalMoveHighlights();
    void drawCheckHighlights();
    void drawRedHighlights(std::optional<int> redHighlightSquare);

    const BoardView* board_;
    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
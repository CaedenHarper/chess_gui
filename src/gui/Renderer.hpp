#pragma once

#include <SFML/Graphics.hpp>

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
    explicit Renderer(sf::RenderWindow* window) : window_{window} {}

    void render(const Game& game, const RenderState& state);

private:
    void clearWindow(sf::Color backgroundColor);
    void drawBoard(const Game& game, std::optional<HeldPieceState> heldPiece);
    void drawDraggedPiece(const Game& game, std::optional<HeldPieceState> heldPiece);
    void drawEngineEval(int currentEval, Color sideToMove);
    void drawEngineStats(SearchStats currentStats);
    void drawPieceOnSquare(const Game& game, int squareIndex, std::optional<HeldPieceState> heldPiece);
    void drawSquare(int squareIndex);

    void drawHighlights(RenderState state);
    void drawSelectedSquareHighlights(std::optional<int> selectedSquare);
    void drawLegalMoveHighlights();
    void drawCheckHighlights();
    void drawRedHighlights(std::optional<int> redHighlightSquare);

    static sf::Sprite makePieceSprite(Piece piece);

    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
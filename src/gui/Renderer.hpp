#pragma once

#include <SFML/Graphics.hpp>

#include "../engine/Engine.hpp"

#include "Highlight.hpp"
#include "InputHandler.hpp"


struct RenderState {
    std::optional<HeldPieceState> heldPiece;

    int currentEval{};
    SearchStats currentStats;

    std::array<bool, Utils::NUM_SQUARES> redHighlightSquares{};
} __attribute__((aligned(128))); // NOLINT[magic numbers] align to 64 bytes

class Renderer {
public:
    explicit Renderer(sf::RenderWindow* window) : window_{window} {}

    void render(const Game& game, const RenderState& state);

private:
    void clearWindow(sf::Color backgroundColor);
    void drawSquares();
    void drawPieces(const Game& game, std::optional<HeldPieceState> heldPiece);
    void drawDraggedPiece(const Game& game, std::optional<HeldPieceState> heldPiece);
    void drawEngineEval(int currentEval, Color sideToMove);
    void drawEngineStats(SearchStats currentStats);

    void drawPieceOnSquare(const Game& game, int square, std::optional<HeldPieceState> heldPiece);
    void drawSquare(int square);
    void drawSquare(int square, sf::Color color);

    void drawHighlights(RenderState state);
    void drawSelectedSquareHighlight(std::optional<HeldPieceState> heldPiece);
    void drawLegalMoveHighlights();
    void drawCheckHighlights();
    void drawRedHighlights(std::array<bool, Utils::NUM_SQUARES> redHighlightSquares);
    void highlightSquare(int square, Highlight highlight);

    static sf::Sprite makePieceSprite(Piece piece);
    static sf::RectangleShape makeSquareShape(int square, sf::Color color);

    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
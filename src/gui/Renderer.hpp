#pragma once

#include <SFML/Graphics.hpp>

#include "../engine/Engine.hpp"

#include "Highlight.hpp"
#include "InputHandler.hpp"


struct RenderState {
    Color playerColor{};

    std::optional<HeldPieceState> heldPiece;

    int currentEval{};
    SearchStats currentStats;

    std::array<bool, Utils::NUM_SQUARES> redHighlightSquares{};
} __attribute__((aligned(128))); // NOLINT[magic numbers] align to 128 bytes

// NOTE: all Game& should be const Game& and should not be expected to modify Game.
//     However, they require generateLegalMoves...() which is not yet const qualified, due to a design decision in Game which has not yet been fixed.
//     TODO: Once this is fixed, this should be updated.
class Renderer {
public:
    explicit Renderer(sf::RenderWindow* window) : window_{window} {}

    void render(Game& game, const RenderState& state);

private:
    void clearWindow(sf::Color backgroundColor);
    void drawSquares();
    void drawPieces(Game& game, std::optional<HeldPieceState> heldPiece);
    void drawDraggedPiece(Game& game, std::optional<HeldPieceState> heldPiece);
    void drawEngineEval(int currentEval, Color playerColor);
    void drawEngineStats(SearchStats currentStats);

    void drawPieceOnSquare(Game& game, int square, std::optional<HeldPieceState> heldPiece);
    void drawSquare(int square);
    void drawSquare(int square, sf::Color color);

    void drawHighlights(Game& game, RenderState state);
    void drawSelectedSquareHighlight(std::optional<HeldPieceState> heldPiece);
    void drawLegalMoveHighlights(Game& game, std::optional<HeldPieceState> heldPiece);
    void drawCheckHighlights(Game& game);
    void drawRedHighlights(std::array<bool, Utils::NUM_SQUARES> redHighlightSquares);
    void highlightSquare(int square, Highlight highlight);

    static sf::Sprite makePieceSprite(Piece piece);
    static sf::RectangleShape makeSquareShape(int square, sf::Color color);

    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
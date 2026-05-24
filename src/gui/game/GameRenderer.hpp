#pragma once

#include <SFML/Graphics.hpp>

#include "../../engine/Engine.hpp"
#include "GameInputHandler.hpp"
#include "Highlight.hpp"

struct GameRenderState {
    Color playerColor{};

    std::optional<HeldPieceState> heldPiece;

    int currentEval{};
    SearchStats currentStats;
    sf::Time engineSearchTime;
    bool engineThinking{};

    std::array<bool, Utils::NUM_SQUARES> redHighlightSquares{};
} __attribute__((aligned(128))); // NOLINT[magic numbers] align to 128 bytes

// NOTE: all Game& should be const Game& and should not be expected to modify Game.
//     However, they require generateLegalMoves...() which is not yet const qualified, due to a design decision in Game
//     which has not yet been fixed.
//     TODO: Once this is fixed, this should be updated.
class GameRenderer {
public:
    explicit GameRenderer(sf::RenderWindow* window) : window_{window} {
    }

    void render(Game& game, const GameRenderState& state);

private:
    void clearWindow(const sf::Color& backgroundColor);
    void drawSquares(Color displayColor);
    void drawPieces(Game& game, std::optional<HeldPieceState> heldPiece, Color displayColor);
    void drawDraggedPiece(Game& game, std::optional<HeldPieceState> heldPiece);
    void drawEngineEval(int currentEval, Color playerColor);
    void drawEngineStats(SearchStats currentStats);
    void drawEngineTimer(sf::Time elapsed, bool thinking);

    void drawPieceOnSquare(Game& game, int square, std::optional<HeldPieceState> heldPiece, Color displayColor);
    void drawSquare(int square, Color displayColor);
    void drawSquare(int square, sf::Color color, Color displayColor);
    void drawText(const std::string& str, const sf::Vector2f& position, int size);

    void drawHighlights(Game& game, GameRenderState state, Color displayColor);
    void drawSelectedSquareHighlight(std::optional<HeldPieceState> heldPiece, Color displayColor);
    void drawLegalMoveHighlights(Game& game, std::optional<HeldPieceState> heldPiece, Color displayColor);
    void drawCheckHighlights(Game& game, Color displayColor);
    void drawRedHighlights(std::array<bool, Utils::NUM_SQUARES> redHighlightSquares, Color displayColor);
    void highlightSquare(int square, Highlight highlight, Color displayColor);

    static sf::Sprite makePieceSprite(Piece piece);
    static sf::RectangleShape makeSquareShape(int square, sf::Color color);

    sf::RenderWindow* window_;
    sf::Font font_{"assets/fonts/LiberationSans-Regular.ttf"};
};
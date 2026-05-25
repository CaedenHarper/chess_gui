#pragma once

#include <SFML/Graphics.hpp>

#include "../../engine/Engine.hpp"
#include "GameInputHandler.hpp"
#include "Highlight.hpp"

// For engine move animation
struct MoveAnimation {
    Piece piece;

    int fromSquare;
    int toSquare;

    sf::Clock clock;
    sf::Time duration = sf::milliseconds(200);
};

struct GameRenderState {
    Color playerColor{};
    Color displayColor{};

    const std::optional<HeldPieceState>& heldPiece;

    int currentEval{};
    const SearchStats& currentStats;
    const sf::Time& engineSearchTime;
    bool engineThinking{};

    const std::array<bool, Utils::NUM_SQUARES>& redHighlightSquares;

    const MoveAnimation* moveAnimation; // nullptr if none exists
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
    void drawPieces(
        Game& game,
        const std::optional<HeldPieceState>& heldPiece,
        Color displayColor,
        const MoveAnimation* animation
    );
    void drawMoveAnimation(const MoveAnimation& animation);
    void drawDraggedPiece(Game& game, const std::optional<HeldPieceState>& heldPiece);
    void drawEngineEval(int currentEval, Color playerColor);
    void drawEngineStats(const SearchStats& currentStats);
    void drawEngineTimer(const sf::Time& elapsed, bool thinking);

    void drawPieceOnSquare(Game& game, int square, Color displayColor);
    void drawSquare(int square, Color displayColor);
    void drawSquare(int square, const sf::Color& color, Color displayColor);
    void drawText(const std::string& str, const sf::Vector2f& position, int size);

    void drawHighlights(Game& game, const GameRenderState& state);
    void drawSelectedSquareHighlight(const std::optional<HeldPieceState>& heldPiece, Color displayColor);
    void drawLegalMoveHighlights(Game& game, const std::optional<HeldPieceState>& heldPiece, Color displayColor);
    void drawCheckHighlights(Game& game, Color displayColor);
    void drawRedHighlights(const std::array<bool, Utils::NUM_SQUARES>& redHighlightSquares, Color displayColor);
    void highlightSquare(int square, const Highlight& highlight, Color displayColor);

    static sf::Sprite makePieceSprite(Piece piece);
    static sf::RectangleShape makeSquareShape(int square, const sf::Color& color);

    sf::RenderWindow* window_;
};
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <optional>

#include "../engine/Engine.hpp"
#include "InputHandler.hpp"

#include "RenderUtils.hpp"

#include "Renderer.hpp"
#include "TextureCache.hpp"

void Renderer::render(const Game& game, const RenderState& state) {
    clearWindow(sf::Color::Black);

    drawSquares();
    drawHighlights(state);
    drawPieces(game, state.heldPiece);
    drawDraggedPiece(game, state.heldPiece);
    drawEngineEval(state.currentEval, game.sideToMove());
    drawEngineStats(state.currentStats);

    window_->display();
}

void Renderer::clearWindow(sf::Color backgroundColor) {
    window_->clear(backgroundColor);
}

void Renderer::drawSquares() {
    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        drawSquare(square);
    }
}

void Renderer::drawPieces(const Game& game, std::optional<HeldPieceState> heldPiece) {
    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        drawPieceOnSquare(game, square, heldPiece);
    }
}

void Renderer::drawDraggedPiece(const Game& game, std::optional<HeldPieceState> heldPiece) {
    if(!heldPiece) {
        return;
    }

    if(!heldPiece->isDragging) {
        return;
    }
    
    sf::Sprite dragSprite = makePieceSprite(game.pieceAtSquareForGui(heldPiece->heldSquare));
    dragSprite.setPosition(heldPiece->mousePos);
    window_->draw(dragSprite);
}

void Renderer::drawEngineEval(int currentEval, Color sideToMove) {
    constexpr sf::Vector2f evalTextPosition = {100.F, 850.F};
    const int evalTextFontSize = 50;

    // load currentEval into string with 2 decimal places
    sf::Text evalText{font_};
    evalText.setString(Eval::evalToString(currentEval, sideToMove));
    evalText.setPosition(evalTextPosition);
    evalText.setFillColor(sf::Color::White);
    evalText.setCharacterSize(evalTextFontSize);
    window_->draw(evalText);
}

void Renderer::drawEngineStats(SearchStats currentStats) {
    constexpr sf::Vector2f statsTextPosition = {400.F, 850.F};
    const int statsTextFontSize = 25;

    // load currentStats into string with 2 decimal places
    sf::Text statsText{font_};
    // Nodes Searched: n
    // QNodes Searched: q
    // Positions Searched: n + q
    statsText.setString("Nodes Searched: " + std::to_string(currentStats.nodes) +
                        "\nQNodes Searched: " + std::to_string(currentStats.qnodes) +
                        "\nPositions Searched: " + std::to_string(currentStats.nodes + currentStats.qnodes));
    statsText.setPosition(statsTextPosition);
    statsText.setFillColor(sf::Color::White);
    statsText.setCharacterSize(statsTextFontSize);
    window_->draw(statsText);
}

void Renderer::drawPieceOnSquare(const Game& game, int square, std::optional<HeldPieceState> heldPiece) {
    if(game.isSquareEmpty(square)) {
        return;
    }

    // skip squares with a held piece that is being dragged
    if(heldPiece && heldPiece->isDragging && heldPiece->heldSquare == square) {
        return;
    }

    const Piece piece = game.pieceAtSquareForGui(square);

    if (!piece.exists()) {
        assert(false); // should never happen; we've already verified it's not empty
        return;
    }

    sf::Sprite sprite = makePieceSprite(piece);
    sprite.setPosition(RenderUtils::squareCenterPx(square));

    window_->draw(sprite);
}

// Overload for normal squares without specific colors.
void Renderer::drawSquare(int square) {
    const sf::Color color = RenderUtils::isSquareLight(square) ? RenderUtils::LIGHT_SQUARE_COLOR : RenderUtils::DARK_SQUARE_COLOR;
    drawSquare(square, color);
}

void Renderer::drawSquare(int square, sf::Color color) {
    const sf::RectangleShape squareShape = makeSquareShape(square, color);
    window_->draw(squareShape);
}

void Renderer::drawHighlights(RenderState state) {
    drawSelectedSquareHighlight(state.heldPiece);
    drawLegalMoveHighlights();
    drawCheckHighlights();
    drawRedHighlights(state.redHighlightSquares);
}

void Renderer::drawSelectedSquareHighlight(std::optional<HeldPieceState> heldPiece) {
    if(!heldPiece) {
        return;
    }
    
    highlightSquare(heldPiece->heldSquare, RenderUtils::SELECTED_HIGHLIGHT);
}

void Renderer::drawLegalMoveHighlights() {

}

void Renderer::drawCheckHighlights() {

}

void Renderer::drawRedHighlights(std::array<bool, Utils::NUM_SQUARES> redHighlightSquares) {
    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        if(!redHighlightSquares.at(square)) {
            continue;
        }

        highlightSquare(square, RenderUtils::RIGHT_CLICK_HIGHLIGHT);
    }
}

void Renderer::highlightSquare(int square, Highlight highlight) {
    const sf::Color color = RenderUtils::isSquareLight(square) ? highlight.lightHighlight() : highlight.darkHighlight();
    drawSquare(square, color);
}

sf::Sprite Renderer::makePieceSprite(const Piece piece) {
    sf::Sprite sprite{TextureCache::get(piece.type(), piece.color())};

    const sf::FloatRect bounds = sprite.getLocalBounds();

    const int BOUNDS_CENTER_X_OFFSET = bounds.size.x / 2.F;
    const int BOUNDS_CENTER_Y_OFFSET = bounds.size.y / 2.F;

    sprite.setOrigin({
        bounds.position.x + BOUNDS_CENTER_X_OFFSET,
        bounds.position.y + BOUNDS_CENTER_Y_OFFSET
    });

    const float scaleX = RenderUtils::SQUARE_WIDTH_PX / bounds.size.x;
    const float scaleY = RenderUtils::SQUARE_HEIGHT_PX / bounds.size.y;
    const float scale = std::min(scaleX, scaleY) * RenderUtils::SPRITE_SCALE_FACTOR;

    sprite.setScale({scale, scale});

    return sprite;
}

sf::RectangleShape Renderer::makeSquareShape(int square, sf::Color color) {
    const int row = Utils::getRow(square);
    const int col = Utils::getCol(square);

    sf::RectangleShape squareShape{{RenderUtils::SQUARE_WIDTH_PX, RenderUtils::SQUARE_HEIGHT_PX}};
    squareShape.setFillColor(color);

    const float xpos = RenderUtils::SQUARE_WIDTH_PX * col;
    const float ypos = RenderUtils::SQUARE_HEIGHT_PX * row;
    squareShape.setPosition({xpos, ypos});

    return squareShape;
}
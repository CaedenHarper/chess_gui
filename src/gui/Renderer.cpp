#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <optional>

#include "../engine/Engine.hpp"
#include "InputHandler.hpp"

#include "RenderUtils.hpp"

#include "Renderer.hpp"
#include "TextureCache.hpp"

void Renderer::render(const Game& game, const RenderState& state) {
    clearWindow(sf::Color::Black);

    drawBoard(game, state.heldPiece);

    drawHighlights(state);

    drawDraggedPiece(game, state.heldPiece);

    drawEngineEval(state.currentEval, game.sideToMove());

    drawEngineStats(state.currentStats);

    // end the current frame
    window_->display();
}

void Renderer::clearWindow(sf::Color backgroundColor) {
    window_->clear(backgroundColor);
}

void Renderer::drawBoard(const Game& game, std::optional<HeldPieceState> heldPiece) {
        // draw row by row
    for(int squareIndex = 0; squareIndex < Utils::NUM_SQUARES; squareIndex++) {
        drawSquare(squareIndex);
        drawPieceOnSquare(game, squareIndex, heldPiece);
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

void Renderer::drawPieceOnSquare(const Game& game, int squareIndex, std::optional<HeldPieceState> heldPiece) {
    if(game.isSquareEmpty(squareIndex)) {
        return;
    }

    // skip squares with a held piece that is being dragged
    if(heldPiece && heldPiece->isDragging && heldPiece->heldSquare == squareIndex) {
        return;
    }

    const Piece piece = game.pieceAtSquareForGui(squareIndex);

    if (!piece.exists()) {
        assert(false); // should never happen; we've already verified it's not empty
        return;
    }

    sf::Sprite sprite = makePieceSprite(piece);
    sprite.setPosition(RenderUtils::squareCenterPx(squareIndex));

    window_->draw(sprite);
}

void Renderer::drawSquare(int squareIndex) {
    const int row = Utils::getRow(squareIndex);
    const int col = Utils::getCol(squareIndex);

    sf::RectangleShape squareShape{{RenderUtils::SQUARE_WIDTH_PX, RenderUtils::SQUARE_HEIGHT_PX}};

    const bool isLight = row%2 == col%2;
    squareShape.setFillColor(isLight ? RenderUtils::LIGHT_SQUARE_COLOR : RenderUtils::DARK_SQUARE_COLOR);

    // set position based on row/col
    const float xpos = RenderUtils::SQUARE_WIDTH_PX * col;
    const float ypos = RenderUtils::SQUARE_HEIGHT_PX * row;
    squareShape.setPosition({xpos, ypos});
    window_->draw(squareShape);
}

void Renderer::drawHighlights(RenderState state) {
    drawSelectedSquareHighlights(state.selectedSquare);
    drawLegalMoveHighlights();
    drawCheckHighlights();
    drawRedHighlights(state.redHighlightSquare);
}

void Renderer::drawSelectedSquareHighlights(std::optional<int> selectedSquare) {
    if(!selectedSquare) {
        return;
    }
    
    const int row = Utils::getRow(*selectedSquare);
    const int col = Utils::getCol(*selectedSquare);
    
}

void Renderer::drawLegalMoveHighlights() {

}

void Renderer::drawCheckHighlights() {

}

void Renderer::drawRedHighlights(std::optional<int> redHighlightSquare) {

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
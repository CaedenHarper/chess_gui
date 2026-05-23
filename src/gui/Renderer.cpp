#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <sstream>

#include "../engine/Engine.hpp"
#include "InputHandler.hpp"

#include "RenderUtils.hpp"

#include "Renderer.hpp"
#include "TextureCache.hpp"

void Renderer::render(Game& game, const RenderState& state) {
    clearWindow(sf::Color::Black);

    // We take playerColor == displayColor here
    drawSquares(state.playerColor);
    drawHighlights(game, state, state.playerColor);
    drawPieces(game, state.heldPiece, state.playerColor);
    drawDraggedPiece(game, state.heldPiece);
    drawEngineEval(state.currentEval, state.playerColor);
    drawEngineStats(state.currentStats);
    drawEngineTimer(state.engineSearchTime, state.engineThinking);

    window_->display();
}

void Renderer::clearWindow(sf::Color backgroundColor) {
    window_->clear(backgroundColor);
}

void Renderer::drawSquares(Color displayColor) {
    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        drawSquare(square, displayColor);
    }
}

void Renderer::drawPieces(Game& game, std::optional<HeldPieceState> heldPiece, Color displayColor) {
    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        drawPieceOnSquare(game, square, heldPiece, displayColor);
    }
}

void Renderer::drawDraggedPiece(Game& game, std::optional<HeldPieceState> heldPiece) {
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

void Renderer::drawEngineEval(int currentEval, Color playerColor) {
    constexpr sf::Vector2f evalTextPosition = {20.F, 800.F};
    const int evalTextFontSize = 50;

    int whiteRelativeEval = currentEval;

    if (playerColor == Color::White) { // engine color == black, so flip sign
        whiteRelativeEval = -whiteRelativeEval;
    }

    drawText("Eval: " + Eval::evalToString(whiteRelativeEval), evalTextPosition, evalTextFontSize);
}

void Renderer::drawEngineStats(SearchStats currentStats) {
    constexpr sf::Vector2f statsTextPosition = {20.F, 875.F};
    const int statsTextFontSize = 25;

    // Nodes Searched: n
    // QNodes Searched: q
    // Positions Searched: n + q
    drawText("Nodes Searched: " + std::to_string(currentStats.nodes) +
                        "\nQNodes Searched: " + std::to_string(currentStats.qnodes) +
                        "\nPositions Searched: " + std::to_string(currentStats.nodes + currentStats.qnodes), statsTextPosition, statsTextFontSize);
}

void Renderer::drawEngineTimer(sf::Time elapsed, bool thinking) {
    constexpr sf::Vector2f engineTimerPosition = {350.F, 800.F};
    constexpr int engineTextFontSize = 50;

    const float seconds = elapsed.asSeconds();

    std::ostringstream out;
    out << std::fixed << std::setprecision(2);

    if (thinking) {
        out << "Thinking: " << seconds << "s";
    } else {
        out << "Last move: " << seconds << "s";
    }

    drawText(out.str(), engineTimerPosition, engineTextFontSize);
}

void Renderer::drawText(const std::string& str, const sf::Vector2f& position, int size) {
    sf::Text text{font_};

    text.setString(str);
    text.setPosition(position);
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(size);

    window_->draw(text);
}

void Renderer::drawPieceOnSquare(Game& game, int square, std::optional<HeldPieceState> heldPiece, Color displayColor) {
    if(game.isSquareEmpty(square)) {
        return;
    }

    // skip squares with a held piece that is being dragged
    if(heldPiece && heldPiece->isDragging && heldPiece->heldSquare == square) {
        return;
    }

    const Piece piece = game.pieceAtSquareForGui(square);

    if (!piece.exists()) {
        assert(false);
        return;
    }

    sf::Sprite sprite = makePieceSprite(piece);
    sprite.setPosition(RenderUtils::squareCenterPx(RenderUtils::getSquareFromDisplayPerspective(square, displayColor)));

    window_->draw(sprite);
}

// Overload for normal squares without specific colors.
void Renderer::drawSquare(int square, Color displayColor) {
    const sf::Color color = RenderUtils::isSquareLight(square) ? RenderUtils::LIGHT_SQUARE_COLOR : RenderUtils::DARK_SQUARE_COLOR;
    drawSquare(square, color, displayColor);
}

void Renderer::drawSquare(int square, sf::Color color, Color displayColor) {
    const sf::RectangleShape squareShape = makeSquareShape(RenderUtils::getSquareFromDisplayPerspective(square, displayColor), color);
    window_->draw(squareShape);
}

void Renderer::drawHighlights(Game& game, RenderState state, Color displayColor) {
    drawSelectedSquareHighlight(state.heldPiece, displayColor);
    drawLegalMoveHighlights(game, state.heldPiece, displayColor);
    drawCheckHighlights(game, displayColor);
    drawRedHighlights(state.redHighlightSquares, displayColor);
}

void Renderer::drawSelectedSquareHighlight(std::optional<HeldPieceState> heldPiece, Color displayColor) {
    if(!heldPiece) {
        return;
    }
    
    highlightSquare(heldPiece->heldSquare, RenderUtils::SELECTED_HIGHLIGHT, displayColor);
}

void Renderer::drawLegalMoveHighlights(Game& game, std::optional<HeldPieceState> heldPiece, Color displayColor) {
    if(!heldPiece) {
        return;
    }

    MoveList legalMoves;
    game.generateLegalMovesFromSquare(heldPiece->heldSquare, legalMoves);
    for(int i = 0; i < legalMoves.size; i++) {
        const Move move = legalMoves.data[i];
        highlightSquare(move.targetSquare(), RenderUtils::LEGAL_HIGHLIGHT, displayColor);
    }
}

void Renderer::drawCheckHighlights(Game& game, Color displayColor) {
    if(!game.isInCheck(game.sideToMove())) {
        return;
    }

    const int kingSquare = game.findKingSquare(game.sideToMove());
    highlightSquare(kingSquare, RenderUtils::CHECK_HIGHLIGHT, displayColor);
}

void Renderer::drawRedHighlights(std::array<bool, Utils::NUM_SQUARES> redHighlightSquares, Color displayColor) {
    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        if(!redHighlightSquares.at(square)) {
            continue;
        }

        highlightSquare(square, RenderUtils::RIGHT_CLICK_HIGHLIGHT, displayColor);
    }
}

void Renderer::highlightSquare(int square, Highlight highlight, Color displayColor) {
    const sf::Color color = RenderUtils::isSquareLight(square) ? highlight.lightHighlight() : highlight.darkHighlight();
    drawSquare(square, color, displayColor);
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
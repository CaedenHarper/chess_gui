#include "GameRenderer.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

#include "../../engine/Engine.hpp"
#include "../resources/RenderUtils.hpp"
#include "../resources/TextureCache.hpp"
#include "GameInputHandler.hpp"

#include <optional>
#include <sstream>

void GameRenderer::render(Game& game, const GameRenderState& state) {
    clearWindow(sf::Color::Black);

    drawSquares(state.displayColor);
    drawHighlights(game, state);
    drawPieces(game, state.heldPiece, state.displayColor, state.moveAnimation);

    if(state.moveAnimation != nullptr) {
        drawMoveAnimation(*state.moveAnimation);
    }

    drawDraggedPiece(game, state.heldPiece);
    drawEngineEval(state.currentEval, state.displayColor);
    drawEngineStats(state.currentStats);
    drawEngineTimer(state.engineSearchTime, state.engineThinking);

    window_->display();
}

void GameRenderer::clearWindow(const sf::Color& backgroundColor) {
    window_->clear(backgroundColor);
}

void GameRenderer::drawSquares(Color displayColor) {
    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        drawSquare(square, displayColor);
    }
}

void GameRenderer::drawPieces(
    Game& game,
    const std::optional<HeldPieceState>& heldPiece,
    Color displayColor,
    const MoveAnimation* animation
) {
    // -1 sentinel which never gets hit by square loop if does not exist
    int heldPieceSkipSquare = heldPiece && heldPiece->isDragging ? heldPiece->heldSquare : -1;
    int animationSkipSquare = animation != nullptr ? animation->toSquare : -1;

    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        if(square == heldPieceSkipSquare) {
            continue;
        }

        if(square == animationSkipSquare) {
            continue;
        }

        drawPieceOnSquare(game, square, displayColor);
    }
}

void GameRenderer::drawMoveAnimation(const MoveAnimation& animation) {
    const float elapsed = animation.clock.getElapsedTime().asSeconds();
    const float duration = animation.duration.asSeconds();

    const float progress = std::min(elapsed / duration, 1.0F);

    const sf::Vector2f fromPx = RenderUtils::squareCenterPx(animation.fromSquare);
    const sf::Vector2f toPx = RenderUtils::squareCenterPx(animation.toSquare);

    const sf::Vector2f position{fromPx.x + (toPx.x - fromPx.x) * progress, fromPx.y + (toPx.y - fromPx.y) * progress};

    sf::Sprite sprite = makePieceSprite(animation.piece);
    sprite.setPosition(position);

    window_->draw(sprite);
}

void GameRenderer::drawDraggedPiece(Game& game, const std::optional<HeldPieceState>& heldPiece) {
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

void GameRenderer::drawEngineEval(int currentEval, Color playerColor) {
    constexpr sf::Vector2f evalTextPosition = {20.F, 800.F};
    const int evalTextFontSize = 50;

    int whiteRelativeEval = currentEval;

    if(playerColor == Color::White) { // engine color == black, so flip sign
        whiteRelativeEval = -whiteRelativeEval;
    }

    drawText("Eval: " + Eval::evalToString(whiteRelativeEval), evalTextPosition, evalTextFontSize);
}

void GameRenderer::drawEngineStats(const SearchStats& currentStats) {
    constexpr sf::Vector2f statsTextPosition = {20.F, 875.F};
    const int statsTextFontSize = 25;

    // Nodes Searched: n
    // QNodes Searched: q
    // Positions Searched: n + q
    drawText(
        "Nodes Searched: " + std::to_string(currentStats.nodes) +
            "\nQNodes Searched: " + std::to_string(currentStats.qnodes) +
            "\nPositions Searched: " + std::to_string(currentStats.nodes + currentStats.qnodes),
        statsTextPosition,
        statsTextFontSize
    );
}

void GameRenderer::drawEngineTimer(const sf::Time& elapsed, bool thinking) {
    constexpr sf::Vector2f engineTimerPosition = {350.F, 800.F};
    constexpr int engineTextFontSize = 50;

    const float seconds = elapsed.asSeconds();

    std::ostringstream out;
    out << std::fixed << std::setprecision(2);

    if(thinking) {
        out << "Thinking: " << seconds << "s";
    } else {
        out << "Last move: " << seconds << "s";
    }

    drawText(out.str(), engineTimerPosition, engineTextFontSize);
}

void GameRenderer::drawText(const std::string& str, const sf::Vector2f& position, int size) {
    sf::Text text{RenderUtils::FONT};

    text.setString(str);
    text.setPosition(position);
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(size);

    window_->draw(text);
}

void GameRenderer::drawPieceOnSquare(Game& game, int square, Color displayColor) {
    if(game.isSquareEmpty(square)) {
        return;
    }

    const Piece piece = game.pieceAtSquareForGui(square);

    assert(piece.exists());

    sf::Sprite sprite = makePieceSprite(piece);
    sprite.setPosition(RenderUtils::squareCenterPx(RenderUtils::getSquareFromDisplayPerspective(square, displayColor)));

    window_->draw(sprite);
}

// Overload for normal squares without specific colors.
void GameRenderer::drawSquare(int square, Color displayColor) {
    const sf::Color color =
        RenderUtils::isSquareLight(square) ? RenderUtils::LIGHT_SQUARE_COLOR : RenderUtils::DARK_SQUARE_COLOR;
    drawSquare(square, color, displayColor);
}

void GameRenderer::drawSquare(int square, const sf::Color& color, Color displayColor) {
    const sf::RectangleShape squareShape =
        makeSquareShape(RenderUtils::getSquareFromDisplayPerspective(square, displayColor), color);
    window_->draw(squareShape);
}

void GameRenderer::drawHighlights(Game& game, const GameRenderState& state) {
    drawSelectedSquareHighlight(state.heldPiece, state.displayColor);
    drawLegalMoveHighlights(game, state.heldPiece, state.displayColor);
    drawCheckHighlights(game, state.displayColor);
    drawRedHighlights(state.redHighlightSquares, state.displayColor);
}

void GameRenderer::drawSelectedSquareHighlight(const std::optional<HeldPieceState>& heldPiece, Color displayColor) {
    if(!heldPiece) {
        return;
    }

    highlightSquare(heldPiece->heldSquare, RenderUtils::SELECTED_HIGHLIGHT, displayColor);
}

void GameRenderer::drawLegalMoveHighlights(
    Game& game,
    const std::optional<HeldPieceState>& heldPiece,
    Color displayColor
) {
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

void GameRenderer::drawCheckHighlights(Game& game, Color displayColor) {
    if(!game.isInCheck(game.sideToMove())) {
        return;
    }

    const int kingSquare = game.findKingSquare(game.sideToMove());
    highlightSquare(kingSquare, RenderUtils::CHECK_HIGHLIGHT, displayColor);
}

void GameRenderer::drawRedHighlights(
    const std::array<bool, Utils::NUM_SQUARES>& redHighlightSquares,
    Color displayColor
) {
    for(int square = 0; square < Utils::NUM_SQUARES; square++) {
        if(!redHighlightSquares.at(square)) {
            continue;
        }

        highlightSquare(square, RenderUtils::RIGHT_CLICK_HIGHLIGHT, displayColor);
    }
}

void GameRenderer::highlightSquare(int square, const Highlight& highlight, Color displayColor) {
    const sf::Color color = RenderUtils::isSquareLight(square) ? highlight.lightHighlight() : highlight.darkHighlight();
    drawSquare(square, color, displayColor);
}

sf::Sprite GameRenderer::makePieceSprite(Piece piece) {
    sf::Sprite sprite{TextureCache::get(piece.type(), piece.color())};

    const sf::FloatRect bounds = sprite.getLocalBounds();

    const int BOUNDS_CENTER_X_OFFSET = bounds.size.x / 2.F;
    const int BOUNDS_CENTER_Y_OFFSET = bounds.size.y / 2.F;

    sprite.setOrigin({bounds.position.x + BOUNDS_CENTER_X_OFFSET, bounds.position.y + BOUNDS_CENTER_Y_OFFSET});

    const float scaleX = RenderUtils::SQUARE_WIDTH_PX / bounds.size.x;
    const float scaleY = RenderUtils::SQUARE_HEIGHT_PX / bounds.size.y;
    const float scale = std::min(scaleX, scaleY) * RenderUtils::SPRITE_SCALE_FACTOR;

    sprite.setScale({scale, scale});

    return sprite;
}

sf::RectangleShape GameRenderer::makeSquareShape(int square, const sf::Color& color) {
    const int row = Utils::getRow(square);
    const int col = Utils::getCol(square);

    sf::RectangleShape squareShape{{RenderUtils::SQUARE_WIDTH_PX, RenderUtils::SQUARE_HEIGHT_PX}};
    squareShape.setFillColor(color);

    const float xpos = RenderUtils::SQUARE_WIDTH_PX * col;
    const float ypos = RenderUtils::SQUARE_HEIGHT_PX * row;
    squareShape.setPosition({xpos, ypos});

    return squareShape;
}
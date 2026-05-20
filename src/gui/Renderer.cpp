#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <optional>

#include "BoardView.hpp"
#include "../engine/Engine.hpp"
#include "InputHandler.hpp"

#include "Constants.hpp"

#include "Renderer.hpp"



void Renderer::render(const Game& game, const RenderState& state) {
    clearWindow(sf::Color::Black);

    drawBoard(state.heldPiece);

    drawHighlights(state);

    drawDraggedPiece(state.heldPiece);

    drawEngineEval(state.currentEval, game.sideToMove());

    drawEngineStats(state.currentStats);

    // end the current frame
    window_->display();
}

void Renderer::clearWindow(sf::Color backgroundColor) {
    window_->clear(backgroundColor);
}

void Renderer::drawBoard(std::optional<HeldPieceState> heldPiece) {
        // draw row by row
    for(int squareIndex = 0; squareIndex < Utils::NUM_SQUARES; squareIndex++) {
        // get row and col from index
        drawSquare(squareIndex);

        if(squareObject.isEmpty()) {
            return;
        }

        // skip squares with a held piece that is being dragged
        if(heldPiece && heldPiece->isDragging && heldPiece->heldSquare == squareIndex) {
            return;
        }

        if (const sf::Sprite* sprite = squareObject.pieceSprite().sprite()) {
            window_->draw(*sprite);
        }
    }
}

void Renderer::drawDraggedPiece(std::optional<HeldPieceState> heldPiece) {
    if(heldPiece && heldPiece->isDragging) {
        if (const sf::Sprite* sprite = board_->at(heldPiece->heldSquare).pieceSprite().sprite()) {
            sf::Sprite dragSprite = *sprite;
            dragSprite.setPosition(heldPiece->mousePos);
            window_->draw(dragSprite);
        }
    }
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

void Renderer::drawSquare(int squareIndex) {
    drawSquare(squareIndex, std::nullopt);
}

void Renderer::drawSquare(int squareIndex, std::optional<Highlight> highlight) {
    const int row = Utils::getRow(squareIndex);
    const int col = Utils::getCol(squareIndex);
    Square squareObject = board_->at(squareIndex);

    sf::RectangleShape squareShape{{Constants::SQUARE_WIDTH_PX, Constants::SQUARE_HEIGHT_PX}};

    const bool isLight = row%2 == col%2;
    squareShape.setFillColor(isLight ? Constants::LIGHT_SQUARE_COLOR : Constants::DARK_SQUARE_COLOR);

    // set position based on row/col
    const float xpos = Constants::SQUARE_WIDTH_PX * col;
    const float ypos = Constants::SQUARE_HEIGHT_PX * row;
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
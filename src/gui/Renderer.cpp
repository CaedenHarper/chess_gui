#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "BoardView.hpp"
#include "../engine/Engine.hpp"
#include "InputHandler.hpp"

#include "Constants.hpp"

#include "Renderer.hpp"



void Renderer::render(const RenderState& state) {
    const std::optional<HeldPieceState> heldPiece = state.heldPiece;
    const bool isDragging = heldPiece ? heldPiece->isDragging : false;
    const int currentEval = state.currentEval;
    const SearchStats currentStats = state.currentStats;
    const Color sideToMove = state.sideToMove;

    clearWindow(sf::Color::Black);

    drawBoard(heldPiece);

    // if holding a piece and we are dragging, we copy sprite to mouse
    if(isDragging) {
        if (const sf::Sprite* sprite = board_->at(heldPiece->heldSquare).pieceSprite().sprite()) {
            sf::Sprite dragSprite = *sprite;
            dragSprite.setPosition(heldPiece->mousePos);
            window_->draw(dragSprite);
        }
    }

    drawEngineEval(currentEval, sideToMove);

    drawEngineStats(currentStats);

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
        const int row = Utils::getRow(squareIndex);
        const int col = Utils::getCol(squareIndex);
        const bool isLight = row%2 == col%2;
        Square squareObject = board_->at(squareIndex);

        sf::RectangleShape squareShape{{Constants::SQUARE_WIDTH_PX, Constants::SQUARE_HEIGHT_PX}};

        // determine square color
        sf::Color color;
        if(squareObject.hasHighlight()) {
            color = isLight ? squareObject.highlight().lightHighlight() : squareObject.highlight().darkHighlight();
        } else {
            color = isLight ? BoardView::LIGHT_SQUARE_COLOR : BoardView::DARK_SQUARE_COLOR;
        }
        squareShape.setFillColor(color);

        // set position based on row/col
        const float xpos = Constants::SQUARE_WIDTH_PX * col;
        const float ypos = Constants::SQUARE_HEIGHT_PX * row;
        squareShape.setPosition({xpos, ypos});
        window_->draw(squareShape);

        // skip empty squares or square that is currently held
        if(squareObject.isEmpty() || (heldPiece && heldPiece->heldSquare == squareIndex)) {
            continue;
        }

        if (const sf::Sprite* sprite = squareObject.pieceSprite().sprite()) {
            window_->draw(*sprite);
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
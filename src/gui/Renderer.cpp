#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "BoardView.hpp"
#include "../engine/Engine.hpp"
#include "InputHandler.hpp"

#include "Renderer.hpp"


void Renderer::render(const RenderState& state) {
    const std::optional<HeldPieceState> heldPiece = state.heldPiece;
    const bool isDragging = heldPiece ? heldPiece->isDragging : false;
    const int currentEval = state.currentEval;
    const SearchStats currentStats = state.currentStats;
    const Color sideToMove = state.sideToMove;

    clearWindow(sf::Color::Black);

    // draw board without heldSqaure iff we are dragging
    board_->draw(*window_, isDragging ? std::optional<int>{heldPiece->heldSquare} : std::nullopt);

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
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "BoardView.hpp"
#include "../engine/Engine.hpp"

#include "Renderer.hpp"


void Renderer::render(const RenderState& state) {
        const std::optional<int> heldSquare = state.heldSquare;
        const bool isDragging = state.isDragging;
        const int currentEval = state.currentEval;
        const SearchStats currentStats = state.currentStats;
        const sf::Vector2f dragPosPx = state.dragPosPx;
        const Color sideToMove = state.sideToMove;

        // clear the window
        window_->clear(sf::Color::Black);

        // draw board without heldSqaure iff we are dragging
        board_->draw(*window_, isDragging ? heldSquare : std::nullopt);

        // if heldSquare and we are dragging, we copy sprite to mouse
        if(heldSquare && isDragging) {
            if (const sf::Sprite* sprite = board_->at(*heldSquare).pieceSprite().sprite()) {
                sf::Sprite dragSprite = *sprite;
                dragSprite.setPosition(dragPosPx);
                window_->draw(dragSprite);
            }
        }

        // draw the engine's evaluation of the position
        constexpr sf::Vector2f evalTextPosition = {100.F, 850.F};
        constexpr sf::Vector2f statsTextPosition = {400.F, 850.F};
        const int evalTextFontSize = 50;
        const int statsTextFontSize = 25;
        // load currentEval into string with 2 decimal places
        sf::Text evalText{font_};
        evalText.setString(Eval::evalToString(currentEval, sideToMove));
        evalText.setPosition(evalTextPosition);
        evalText.setFillColor(sf::Color::White);
        evalText.setCharacterSize(evalTextFontSize);
        window_->draw(evalText);

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

        // end the current frame
        window_->display();
}
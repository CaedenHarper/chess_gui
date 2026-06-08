#include "MainMenuRenderer.hpp"

#include <SFML/Graphics/Color.hpp>

#include "MainMenuLayout.hpp"

void MainMenuRenderer::render() {
    RenderUtils::clearWindow(window_, sf::Color::Black);

    drawBackground();
    RenderUtils::drawButton(window_, MainMenuLayout::menuCard);
    drawTitle();
    RenderUtils::drawButton(window_, MainMenuLayout::whiteButton);
    RenderUtils::drawButton(window_, MainMenuLayout::blackButton);

    window_->display();
}

void MainMenuRenderer::drawBackground() {
    constexpr float squareSize = 125.F;

    for(int row = 0; row < 8; ++row) {
        for(int col = 0; col < 8; ++col) {
            sf::RectangleShape square{{squareSize, squareSize}};
            square.setPosition({col * squareSize, row * squareSize});

            const bool light = (row + col) % 2 == 0;
            square.setFillColor(light ? MainMenuLayout::lightBackgroundColor : MainMenuLayout::darkBackgroundColor);

            window_->draw(square);
        }
    }
}

void MainMenuRenderer::drawTitle() {
    constexpr sf::Vector2f titlePosition{500.F, 220.F};
    constexpr int titleSize = 96;

    // Shadow
    RenderUtils::drawText(
        window_, "CHESS", {titlePosition.x + 4.F, titlePosition.y + 4.F}, titleSize, sf::Color{20, 20, 20}, true
    );

    // Main title
    RenderUtils::drawText(window_, "CHESS", titlePosition, titleSize, sf::Color{245, 245, 245}, true);

    // Gold underline
    constexpr sf::Vector2f underlineSize{180.F, 3.F};
    constexpr sf::Vector2f underlinePosition{titlePosition.x - underlineSize.x / 2.F, titlePosition.y + 40.F};

    sf::RectangleShape underline{underlineSize};
    underline.setPosition(underlinePosition);
    underline.setFillColor(sf::Color{212, 175, 55});

    window_->draw(underline);
}
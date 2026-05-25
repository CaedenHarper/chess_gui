#include "MainMenuRenderer.hpp"

#include <SFML/Graphics/Color.hpp>

#include "MainMenuLayout.hpp"

void MainMenuRenderer::render() {
    clearWindow(sf::Color::Black);

    drawTitle();
    drawWhiteButton();
    drawBlackButton();

    window_->display();
}

void MainMenuRenderer::drawTitle() {
    constexpr int titleFontSize = 96;
    constexpr sf::Vector2f titlePosition{350.F, 120.F};

    drawText("Chess", titlePosition, titleFontSize);
}

void MainMenuRenderer::drawWhiteButton() {
    drawButton(MainMenuLayout::whiteButton);
}

void MainMenuRenderer::drawBlackButton() {
    drawButton(MainMenuLayout::blackButton);
}

void MainMenuRenderer::clearWindow(const sf::Color& backgroundColor) {
    window_->clear(backgroundColor);
}

void MainMenuRenderer::drawText(const std::string& str, const sf::Vector2f& position, int size) {
    sf::Text text{RenderUtils::FONT};

    text.setString(str);
    text.setPosition(position);
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(size);

    window_->draw(text);
}

void MainMenuRenderer::drawButton(MainMenuButton button) {
    window_->draw(button.button());
    window_->draw(button.text());
}
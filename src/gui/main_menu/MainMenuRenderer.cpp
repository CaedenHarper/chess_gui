#include "MainMenuRenderer.hpp"

#include <SFML/Graphics/Color.hpp>

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
    constexpr sf::Vector2f buttonSize{300.F, 80.F};
    constexpr sf::Vector2f buttonPosition{350.F, 350.F};
    constexpr sf::Color offWhite{230, 230, 230}; // rgb(230, 230, 230)
    constexpr float outlineThickness = 2.F;
    constexpr int textSize = 36;

    drawButton(buttonSize, buttonPosition, offWhite, sf::Color::White, outlineThickness);
    drawButtonText("Play as White", buttonPosition, textSize, buttonSize);
}

void MainMenuRenderer::drawBlackButton() {
    constexpr sf::Vector2f buttonSize{300.F, 80.F};
    constexpr sf::Vector2f buttonPosition{350.F, 470.F};
    constexpr sf::Color darkGray{40, 40, 40}; // rgb(40, 40, 40)
    constexpr float outlineThickness = 2.F;
    constexpr int textSize = 36;

    drawButton(buttonSize, buttonPosition, darkGray, sf::Color::White, outlineThickness);
    drawButtonText("Play as Black", buttonPosition, textSize, buttonSize);
}

void MainMenuRenderer::clearWindow(const sf::Color& backgroundColor) {
    window_->clear(backgroundColor);
}

void MainMenuRenderer::drawText(const std::string& str, const sf::Vector2f& position, int size) {
    sf::Text text{font_};

    text.setString(str);
    text.setPosition(position);
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(size);

    window_->draw(text);
}

void MainMenuRenderer::drawButton(
    const sf::Vector2f& size,
    const sf::Vector2f& position,
    const sf::Color& fillColor,
    const sf::Color& outlineColor,
    float outlineThickness
) {
    sf::RectangleShape button{size};

    button.setPosition(position);
    button.setFillColor(fillColor);
    button.setOutlineColor(outlineColor);
    button.setOutlineThickness(outlineThickness);

    window_->draw(button);
}

void MainMenuRenderer::drawButtonText(
    const std::string& str,
    const sf::Vector2f& buttonPosition,
    int textSize,
    const sf::Vector2f& buttonSize
) {
    sf::Text text{font_};

    text.setString(str);
    text.setCharacterSize(textSize);
    text.setFillColor(sf::Color::Black);

    const sf::FloatRect whiteTextBounds = text.getLocalBounds();
    text.setOrigin(
        {whiteTextBounds.position.x + whiteTextBounds.size.x / 2.F,
         whiteTextBounds.position.y + whiteTextBounds.size.y / 2.F}
    );
    text.setPosition({buttonPosition.x + buttonSize.x / 2.F, buttonPosition.y + buttonSize.y / 2.F});

    window_->draw(text);
}
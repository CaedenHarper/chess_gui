#include "MainMenuRenderer.hpp"

#include <SFML/Graphics/Color.hpp>

#include "MainMenuLayout.hpp"

void MainMenuRenderer::render() {
    clearWindow(sf::Color::Black);

    drawBackground();
    drawButton(MainMenuLayout::menuCard);
    drawTitle();
    drawSubtitle();
    drawButton(MainMenuLayout::whiteButton);
    drawButton(MainMenuLayout::blackButton);
    drawFooter();

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
    drawText("CHESS", {titlePosition.x + 4.F, titlePosition.y + 4.F}, titleSize, sf::Color{20, 20, 20}, true);

    // Main title
    drawText("CHESS", titlePosition, titleSize, sf::Color{245, 245, 245}, true);

    // Gold underline
    constexpr sf::Vector2f underlineSize{180.F, 3.F};
    constexpr sf::Vector2f underlinePosition{titlePosition.x - underlineSize.x / 2.F, titlePosition.y + 40.F};

    sf::RectangleShape underline{underlineSize};
    underline.setPosition(underlinePosition);
    underline.setFillColor(sf::Color{212, 175, 55});

    window_->draw(underline);
}

void MainMenuRenderer::drawSubtitle() {
    drawText("Choose your side", {500.F, 300.F}, 28, sf::Color{180, 180, 180}, true);
}

void MainMenuRenderer::drawFooter() {
    drawText("SFML Chess Engine", {500.F, 625.F}, 20, sf::Color{120, 120, 120}, true);
}

void MainMenuRenderer::clearWindow(const sf::Color& backgroundColor) {
    window_->clear(backgroundColor);
}

void MainMenuRenderer::drawText(
    const std::string& str,
    const sf::Vector2f& position,
    int size,
    const sf::Color& color,
    bool centered
) {
    sf::Text text{RenderUtils::FONT};

    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);

    if(centered) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.position.x + bounds.size.x / 2.F, bounds.position.y + bounds.size.y / 2.F});
    }

    text.setPosition(position);

    window_->draw(text);
}

void MainMenuRenderer::drawButton(const MainMenuButton& button) {
    window_->draw(button.button());
    if(button.hasText()) {
        window_->draw(*button.text());
    }
}
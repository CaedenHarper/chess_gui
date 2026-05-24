#include "MainMenuRenderer.hpp"

void MainMenuRenderer::render() {
    clearWindow(sf::Color::Blue);

    window_->display();
}

void MainMenuRenderer::clearWindow(sf::Color backgroundColor) {
    window_->clear(backgroundColor);
}
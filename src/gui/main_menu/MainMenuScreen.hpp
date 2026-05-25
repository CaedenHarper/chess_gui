#pragma once

#include <SFML/Audio.hpp>

#include "../resources/Screen.hpp"
#include "MainMenuInputHandler.hpp"
#include "MainMenuRenderer.hpp"

class MainMenuScreen : public Screen {
public:
    explicit MainMenuScreen(sf::RenderWindow& window) : renderer_{&window} {
    }

    ScreenCommand handleEvent(const sf::Event& event) override;
    void update() override;
    void render() override;

private:
    MainMenuRenderer renderer_;
    MainMenuInputHandler inputHandler_;
};
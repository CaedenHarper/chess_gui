#pragma once

#include <SFML/Graphics.hpp>

enum class ScreenID : std::uint8_t { MainMenu, Game, GameOver };

class Screen {
public:
    Screen() = default;

    virtual ~Screen() = default;
    Screen(const Screen& other) = delete;
    Screen& operator=(const Screen& other) = delete;
    Screen(Screen&& other) = delete;
    Screen& operator=(Screen&& other) = delete;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update() = 0;
    virtual void render() = 0;
};
#pragma once

#include <SFML/Graphics.hpp>

enum class ScreenID { MainMenu, WhiteGame, BlackGame, GameOver };
enum class ScreenCommand { None, StartWhiteGame, StartBlackGame };

class Screen {
public:
    Screen() = default;

    virtual ~Screen() = default;
    Screen(const Screen& other) = delete;
    Screen& operator=(const Screen& other) = delete;
    Screen(Screen&& other) = delete;
    Screen& operator=(Screen&& other) = delete;

    virtual ScreenCommand handleEvent(const sf::Event& event) = 0;
    virtual void update() = 0;
    virtual void render() = 0;
};
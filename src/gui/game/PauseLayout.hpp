#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "../resources/Button.hpp"
#include "../resources/RenderUtils.hpp"

namespace PauseLayout {
inline Button restartGameButton{
    {300.F, 80.F},
    {350.F, 470.F},
    {40, 40, 40},
    sf::Color::White,
    2.F,
    RenderUtils::FONT,
    "Restart Game",
    36,
    sf::Color::White
};

inline Button mainMenuButton{
    {350.F, 80.F},
    {350.F, 590.F},
    {40, 40, 40},
    sf::Color::White,
    2.F,
    RenderUtils::FONT,
    "Back to Main Menu",
    36,
    sf::Color::White
};

} // namespace PauseLayout
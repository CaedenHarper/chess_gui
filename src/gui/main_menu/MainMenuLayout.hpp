#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "../resources/Button.hpp"
#include "../resources/RenderUtils.hpp"


namespace MainMenuLayout {
inline Button menuCard{{420.F, 560.F}, {290.F, 120.F}, sf::Color{8, 8, 8}, sf::Color{212, 175, 55}, 2.F};

inline Button whiteButton{
    {300.F, 80.F},
    {350.F, 350.F},
    {246, 245, 236},
    sf::Color::Black,
    2.F,
    RenderUtils::FONT,
    "Play as White",
    36,
    sf::Color::Black
};

inline Button blackButton{
    {300.F, 80.F},
    {350.F, 470.F},
    {40, 40, 40},
    sf::Color::White,
    2.F,
    RenderUtils::FONT,
    "Play as Black",
    36,
    sf::Color::White
};

inline sf::Color lightBackgroundColor{28, 28, 28};
inline sf::Color darkBackgroundColor{18, 18, 18};

} // namespace MainMenuLayout
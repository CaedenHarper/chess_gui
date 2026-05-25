#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "../resources/RenderUtils.hpp"

class MainMenuButton {
public:
    MainMenuButton(
        const sf::Vector2f& size,
        const sf::Vector2f& position,
        const sf::Color& fillColor,
        const sf::Color& outlineColor,
        float outlineThickness,
        const sf::Font& font,
        const std::string& str,
        int textSize,
        const sf::Color& textColor
    )
        : size_{size}, position_{position}, button_{size}, text_{font} {
        button_.setPosition(position);
        button_.setFillColor(fillColor);
        button_.setOutlineColor(outlineColor);
        button_.setOutlineThickness(outlineThickness);

        text_.setString(str);
        text_.setCharacterSize(textSize);
        text_.setFillColor(textColor);
        const sf::FloatRect whiteTextBounds = text_.getLocalBounds();
        text_.setOrigin(
            {whiteTextBounds.position.x + whiteTextBounds.size.x / 2.F,
             whiteTextBounds.position.y + whiteTextBounds.size.y / 2.F}
        );
        text_.setPosition({position_.x + size_.x / 2.F, position_.y + size_.y / 2.F});
    }

    const sf::RectangleShape& button() const {
        return button_;
    }

    const sf::Text& text() const {
        return text_;
    }

    bool contains(const sf::Vector2f& position) const {
        return button_.getGlobalBounds().contains(position);
    }

private:
    const sf::Vector2f size_;
    const sf::Vector2f position_;

    sf::RectangleShape button_;
    sf::Text text_;
};

namespace MainMenuLayout {
inline MainMenuButton whiteButton{
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

inline MainMenuButton blackButton{
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

} // namespace MainMenuLayout
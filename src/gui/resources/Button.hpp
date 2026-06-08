#pragma once

#include <SFML/Graphics.hpp>

class Button {
public:
    Button(
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

        text_->setString(str);
        text_->setCharacterSize(textSize);
        text_->setFillColor(textColor);

        // center text
        const sf::FloatRect whiteTextBounds = text_->getLocalBounds();
        text_->setOrigin(
            {whiteTextBounds.position.x + whiteTextBounds.size.x / 2.F,
             whiteTextBounds.position.y + whiteTextBounds.size.y / 2.F}
        );
        text_->setPosition({position_.x + size_.x / 2.F, position_.y + size_.y / 2.F});
    }

    // No text overload
    Button(
        const sf::Vector2f& size,
        const sf::Vector2f& position,
        const sf::Color& fillColor,
        const sf::Color& outlineColor,
        float outlineThickness
    )
        : size_{size}, position_{position}, button_{size} {
        button_.setPosition(position);
        button_.setFillColor(fillColor);
        button_.setOutlineColor(outlineColor);
        button_.setOutlineThickness(outlineThickness);
    }

    const sf::RectangleShape& button() const {
        return button_;
    }

    const sf::Text* text() const {
        return text_ ? &*text_ : nullptr;
    }

    bool hasText() const {
        return text_.has_value();
    }

    bool contains(const sf::Vector2f& position) const {
        return button_.getGlobalBounds().contains(position);
    }

private:
    const sf::Vector2f size_;
    const sf::Vector2f position_;

    sf::RectangleShape button_;
    std::optional<sf::Text> text_;
};
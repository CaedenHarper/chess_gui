#pragma once

#include <SFML/Graphics.hpp>

class Highlight {
public:
    // Construct a Highlight.
    constexpr Highlight(sf::Color light, sf::Color dark) : lightHighlight_{light}, darkHighlight_{dark} {};
    // Compare a Highlight to another. Two highlights are equal if both of their colors are the same.
    bool operator==(Highlight other) const {
        return lightHighlight_ == other.lightHighlight_ && darkHighlight_ == other.darkHighlight_;
    };
    sf::Color lightHighlight() const {
        return lightHighlight_;
    };
    sf::Color darkHighlight() const {
        return darkHighlight_;
    };

private:
    // Color for the light squares.
    sf::Color lightHighlight_;
    // Color for the dark squares.
    sf::Color darkHighlight_;
};
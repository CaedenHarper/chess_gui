#pragma once

#include <SFML/Graphics.hpp>

#include "../../game/Piece.hpp"

// Cache which maintains and retrieves textures for Pieces.
class TextureCache {
public:
    // Retrieve texture from piece type and color.
    static const sf::Texture& get(PieceType type, Color color);

private:
    // Retrieve path to texture from piece type and color.
    static std::string makePath_(PieceType type, Color color);
};
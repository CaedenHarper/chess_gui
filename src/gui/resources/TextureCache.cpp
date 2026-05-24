#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

#include <SFML/Graphics.hpp>

#include "TextureCache.hpp"


const sf::Texture& TextureCache::get(const PieceType type, const Color color) {
    static std::map<std::string, sf::Texture> cache;

    const std::string path = makePath_(type, color);
    // see if we can find it early from cache
    auto possibleTextureIter = cache.find(path);
    if (possibleTextureIter != cache.end()) {
        return possibleTextureIter->second;
    }

    // error if we can't load texture
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        throw std::runtime_error("Failed to load " + path);
    }

    texture.setSmooth(true);

    // Generate mipmap to increase texture quality when downscaled
    if (!texture.generateMipmap()) {
        std::cerr << "Warning: mipmap generation failed for " << path << "\n";
    }

    // insert into cache and return
    auto [insertedIt, alreadyExisted] = cache.emplace(path, std::move(texture));
    return insertedIt->second;
}

std::string TextureCache::makePath_(const PieceType type, const Color color) {
    std::string path = "assets/pieces/";
    path += (color == Color::White ? "w" : "b");
    switch (type) {
        case PieceType::None: return ""; // unused
        case PieceType::Pawn: path += "P"; break;
        case PieceType::Knight: path += "N"; break;
        case PieceType::Bishop: path += "B"; break;
        case PieceType::Rook: path += "R"; break;
        case PieceType::Queen: path += "Q"; break;
        case PieceType::King: path += "K"; break;
    }
    path += ".png";
    return path;
}

// void PieceSprite::rebuild() {
//     // if we need to destroy sprite
//     if (type() == PieceType::None) {
//         sprite_.reset();
//         return;
//     }

//     // we need sprite
//     // either reuse existing texture or create new one if it does not exist
//     if(sprite_) {
//         sprite_.value().setTexture(TextureCache::get(type(), color()), true);
//     } else {
//         sprite_ = sf::Sprite{TextureCache::get(type(), color())};
//     }
// }

// void PieceSprite::fitToSquare(const float squareSize) {
//     if (!sprite_) {
//         return;
//     }

//     // size of the texture region the sprite uses
//     const sf::FloatRect bounds = sprite_->getLocalBounds(); // {left, top, width, height}

//     const float xSize = squareSize / bounds.size.x;
//     const float ySize = squareSize / bounds.size.y;

//     // preserve aspect ratio
//     const float finalSize = std::min(xSize, ySize);

//     sprite_->setScale({finalSize, finalSize});
// }

// void PieceSprite::centerOrigin() {
//     if (!sprite_) {
//         return;
//     }
//     const sf::FloatRect bounds = sprite_->getLocalBounds(); // {left, top, width, height}

//     const int BOUNDS_CENTER_X_OFFSET = bounds.size.x / 2.F;
//     const int BOUNDS_CENTER_Y_OFFSET = bounds.size.y / 2.F;

//     sprite_->setOrigin({bounds.position.x + BOUNDS_CENTER_X_OFFSET, bounds.position.y + BOUNDS_CENTER_Y_OFFSET});
// }

// void PieceSprite::updateSpritePosition(const float xPos, const float yPos) {
//     if (sprite_) {
//         sprite_.value().setPosition({xPos, yPos});
//     }
// }
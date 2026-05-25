#include "TextureCache.hpp"

#include <SFML/Graphics.hpp>

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

const sf::Texture& TextureCache::get(const PieceType type, const Color color) {
    static std::map<std::string, sf::Texture> cache;

    const std::string path = makePath_(type, color);
    // see if we can find it early from cache
    auto possibleTextureIter = cache.find(path);
    if(possibleTextureIter != cache.end()) {
        return possibleTextureIter->second;
    }

    // error if we can't load texture
    sf::Texture texture;
    if(!texture.loadFromFile(path)) {
        throw std::runtime_error("Failed to load " + path);
    }

    texture.setSmooth(true);

    // Generate mipmap to increase texture quality when downscaled
    if(!texture.generateMipmap()) {
        std::cerr << "Warning: mipmap generation failed for " << path << "\n";
    }

    // insert into cache and return
    auto [insertedIt, alreadyExisted] = cache.emplace(path, std::move(texture));
    return insertedIt->second;
}

std::string TextureCache::makePath_(const PieceType type, const Color color) {
    std::string path = "assets/pieces/";
    path += (color == Color::White ? "w" : "b");
    switch(type) {
        case PieceType::None:
            return ""; // unused
        case PieceType::Pawn:
            path += "P";
            break;
        case PieceType::Knight:
            path += "N";
            break;
        case PieceType::Bishop:
            path += "B";
            break;
        case PieceType::Rook:
            path += "R";
            break;
        case PieceType::Queen:
            path += "Q";
            break;
        case PieceType::King:
            path += "K";
            break;
    }
    path += ".png";
    return path;
}
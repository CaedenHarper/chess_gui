#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>

#include <SFML/Graphics.hpp>

#include "Board.hpp"
#include "../game/Utils.hpp"

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

PieceSprite::PieceSprite(Piece piece) : Piece(piece.type(), piece.color()) {
    rebuild();
}

const sf::Sprite* PieceSprite::sprite() const {
    return sprite_ ? &sprite_.value() : nullptr;
}

void PieceSprite::rebuild() {
    // if we need to destroy sprite
    if (type() == PieceType::None) {
        sprite_.reset();
        return;
    }

    // we need sprite
    // either reuse existing texture or create new one if it does not exist
    if(sprite_) {
        sprite_.value().setTexture(TextureCache::get(type(), color()), true);
    } else {
        sprite_ = sf::Sprite{TextureCache::get(type(), color())};
    }
}

void PieceSprite::fitToSquare(const float squareSize) {
    if (!sprite_) {
        return;
    }

    // size of the texture region the sprite uses
    const sf::FloatRect bounds = sprite_->getLocalBounds(); // {left, top, width, height}

    const float xSize = squareSize / bounds.size.x;
    const float ySize = squareSize / bounds.size.y;

    // preserve aspect ratio
    const float finalSize = std::min(xSize, ySize);

    sprite_->setScale({finalSize, finalSize});
}

void PieceSprite::centerOrigin() {
    if (!sprite_) {
        return;
    }
    const sf::FloatRect bounds = sprite_->getLocalBounds(); // {left, top, width, height}
    sprite_->setOrigin({bounds.position.x + (bounds.size.x / 2.F), bounds.position.y + (bounds.size.y / 2.F)});
}

void PieceSprite::updateSpritePosition(const float xPos, const float yPos) {
    if (sprite_) {
        sprite_.value().setPosition({xPos, yPos});
    }
}

Highlight Square::highlight() const {
    if(highlight_) {
        return highlight_.value();
    }
    // no highlight, throw
    throw std::runtime_error("Attempted to retrieve highlight from Square with no highlight.");
}

void Square::toggleHighlight(Highlight highlight) {
    if(highlight_) {
        highlight_.reset();
    } else {
        highlight_ = highlight;
    }
}

void Square::clearHighlight(Highlight highlight) {
    if(highlight_ && highlight_.value() == highlight) {
        highlight_.reset();
    }
}

int Board::getSquareIndexFromCoordinates(int xPos, int yPos) {
    const int row = static_cast<int>(yPos / SQUARE_WIDTH);
    const int col = static_cast<int>(xPos / SQUARE_HEIGHT);
    return Utils::getSquareIndex(col, row);
}

Square& Board::at(int squareIndex) {
    return board_.at(squareIndex);
}

void Board::draw(sf::RenderWindow& window) const {
    // other overload skips over nothing given std::nullopt
    draw(window, std::nullopt);
}

void Board::draw(sf::RenderWindow& window, const std::optional<int> heldSquare) const {
    // draw row by row
    for(int squareIndex = 0; squareIndex < 64; squareIndex++) {
        // get row and col from index
        const int row = Utils::getRow(squareIndex);
        const int col = Utils::getCol(squareIndex);
        const bool isLight = row%2 == col%2;
        Square squareObject = board_.at(squareIndex);

        sf::RectangleShape squareShape{{SQUARE_WIDTH, SQUARE_HEIGHT}};

        // determine square color
        sf::Color color;
        if(squareObject.hasHighlight()) {
            color = isLight ? squareObject.highlight().lightHighlight() : squareObject.highlight().darkHighlight();
        } else {
            color = isLight ? LIGHT_SQUARE_COLOR : DARK_SQUARE_COLOR;
        }
        squareShape.setFillColor(color);

        // set position based on row/col
        const float xpos = SQUARE_WIDTH * col;
        const float ypos = SQUARE_HEIGHT * row;
        squareShape.setPosition({xpos, ypos});
        window.draw(squareShape);

        // skip empty squares or square that is currently held
        if(squareObject.isEmpty() || (heldSquare && heldSquare.value() == squareIndex)) {
            continue;
        }

        if (const sf::Sprite* sprite = squareObject.pieceSprite().sprite()) {
            window.draw(*sprite);
        }
    }
}

void Board::clearAllHighlights() {
    for(Square& square : board_) {
        square.clearHighlight();
    }
}

void Board::clearAllHighlights(const Highlight highlightToClear) {
    for(Square& square : board_) {
        square.clearHighlight(highlightToClear);
    }
}

void Board::clearAllHighlightsExcept(const Highlight highlightToSkip) {
    for(Square& square : board_) {
        if(square.hasHighlight() && square.highlight() == highlightToSkip) {
            // skip if input matches
            continue;
        }

        // else, clear it
        square.clearHighlight();
    }
}

void Board::updateBoardFromGame(const Game& game) {
    // TODO: consider making more performant by checking equality before updating for each piece
    for(int squareIndex = 0; squareIndex < Game::NUM_SQUARES; squareIndex++) {
        const int row = Utils::getRow(squareIndex);
        const int col = Utils::getCol(squareIndex);

        const float xPos = SQUARE_WIDTH * col;
        const float yPos = SQUARE_WIDTH * row;

        Square& square = board_.at(squareIndex);
        const Piece piece = game.pieceAtSquareForGui(squareIndex);
        // const Piece piece = game.board()[squareIndex];
        square.pieceSprite() = PieceSprite{piece};
        // fit to center of square
        square.pieceSprite().centerOrigin();
        square.pieceSprite().fitToSquare(SQUARE_WIDTH * 0.97F);
        square.pieceSprite().updateSpritePosition(xPos + (SQUARE_WIDTH / 2.F), yPos + (SQUARE_HEIGHT / 2.F));
    }
}
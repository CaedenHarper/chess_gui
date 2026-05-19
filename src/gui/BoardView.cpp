#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>

#include <SFML/Graphics.hpp>

#include "BoardView.hpp"
#include "../game/Utils.hpp"

#include "Constants.hpp"


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

    const int BOUNDS_CENTER_X_OFFSET = bounds.size.x / 2.F;
    const int BOUNDS_CENTER_Y_OFFSET = bounds.size.y / 2.F;

    sprite_->setOrigin({bounds.position.x + BOUNDS_CENTER_X_OFFSET, bounds.position.y + BOUNDS_CENTER_Y_OFFSET});
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

int BoardView::getSquareIndexFromCoordinates(int xPos, int yPos) {
    const int row = static_cast<int>(yPos / Constants::SQUARE_WIDTH_PX);
    const int col = static_cast<int>(xPos / Constants::SQUARE_HEIGHT_PX);
    return Utils::getSquareIndex(col, row);
}

Square& BoardView::at(int squareIndex) {
    return board_.at(squareIndex);
}

Square BoardView::at(int squareIndex) const {
    return board_.at(squareIndex);
}

void BoardView::draw(sf::RenderWindow& window) const {
    // other overload skips over nothing given std::nullopt
    draw(window, std::nullopt);
}

void BoardView::draw(sf::RenderWindow& window, const std::optional<int> heldSquare) const {
    // draw row by row
    for(int squareIndex = 0; squareIndex < Utils::NUM_SQUARES; squareIndex++) {
        // get row and col from index
        const int row = Utils::getRow(squareIndex);
        const int col = Utils::getCol(squareIndex);
        const bool isLight = row%2 == col%2;
        Square squareObject = board_.at(squareIndex);

        sf::RectangleShape squareShape{{Constants::SQUARE_WIDTH_PX, Constants::SQUARE_HEIGHT_PX}};

        // determine square color
        sf::Color color;
        if(squareObject.hasHighlight()) {
            color = isLight ? squareObject.highlight().lightHighlight() : squareObject.highlight().darkHighlight();
        } else {
            color = isLight ? LIGHT_SQUARE_COLOR : DARK_SQUARE_COLOR;
        }
        squareShape.setFillColor(color);

        // set position based on row/col
        const float xpos = Constants::SQUARE_WIDTH_PX * col;
        const float ypos = Constants::SQUARE_HEIGHT_PX * row;
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

void BoardView::clearAllHighlights() {
    for(Square& square : board_) {
        square.clearHighlight();
    }
}

void BoardView::clearAllHighlights(const Highlight highlightToClear) {
    for(Square& square : board_) {
        square.clearHighlight(highlightToClear);
    }
}

void BoardView::clearAllHighlightsExcept(const Highlight highlightToSkip) {
    for(Square& square : board_) {
        if(square.hasHighlight() && square.highlight() == highlightToSkip) {
            // skip if input matches
            continue;
        }

        // else, clear it
        square.clearHighlight();
    }
}

void BoardView::updateBoardFromGame(const Game& game) {
    // Square takes up 97% of the square
    constexpr float piecePercentageInSquare = 0.97F;

    // TODO: consider making more performant by checking equality before updating for each piece
    for(int squareIndex = 0; squareIndex < Utils::NUM_SQUARES; squareIndex++) {
        const int row = Utils::getRow(squareIndex);
        const int col = Utils::getCol(squareIndex);

        const float xPos = Constants::SQUARE_WIDTH_PX * col;
        const float yPos = Constants::SQUARE_WIDTH_PX * row;

        constexpr float SPRITE_CENTER_X_OFFSET = Constants::SQUARE_WIDTH_PX * 0.5F;
        constexpr float SPRITE_CENTER_Y_OFFSET = Constants::SQUARE_HEIGHT_PX * 0.5F;

        Square& square = board_.at(squareIndex);
        const Piece piece = game.pieceAtSquareForGui(squareIndex);
        // const Piece piece = game.board()[squareIndex];
        square.pieceSprite() = PieceSprite{piece};
        // fit to center of square
        square.pieceSprite().centerOrigin();
        square.pieceSprite().fitToSquare(Constants::SQUARE_WIDTH_PX * piecePercentageInSquare);

        square.pieceSprite().updateSpritePosition(xPos + SPRITE_CENTER_X_OFFSET, yPos + SPRITE_CENTER_Y_OFFSET);
    }
}
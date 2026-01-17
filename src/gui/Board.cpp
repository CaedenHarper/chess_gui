#include <SFML/Graphics/Sprite.hpp>
#include <map>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include <SFML/Graphics.hpp>

#include "Board.hpp"

const sf::Texture& TextureCache::get(const PieceType type, const Color color) {
    static std::map<std::string, sf::Texture> cache;

    std::string path = makePath_(type, color);
    // see if we can find it early from cache
    auto it = cache.find(path);
    if (it != cache.end()) return it->second;

    // error if we can't load texture
    sf::Texture tex;
    if (!tex.loadFromFile(path)) {
        throw std::runtime_error("Failed to load " + path);
    }

    tex.setSmooth(true);

    // Generate mipmap to increase texture quality when downscaled
    if (!tex.generateMipmap()) {
        std::cerr << "Warning: mipmap generation failed for " << path << "\n";
    }

    // insert into cache and return
    auto [insertedIt, _] = cache.emplace(path, std::move(tex));
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
    if (type_ == PieceType::None) {
        sprite_.reset();
        return;
    }

    // we need sprite
    // either reuse existing texture or create new one if it does not exist
    if(sprite_) {
        sprite_.value().setTexture(TextureCache::get(type_, color_), true);
    } else {
        sprite_ = sf::Sprite{TextureCache::get(type_, color_)};
    }
}

void PieceSprite::fitToSquare(const float squareSize) {
    if (!sprite_) return;

    // size of the texture region the sprite uses
    const sf::FloatRect bounds = sprite_->getLocalBounds(); // {left, top, width, height}

    const float sx = squareSize / bounds.size.x;
    const float sy = squareSize / bounds.size.y;

    // preserve aspect ratio
    const float s = std::min(sx, sy);

    sprite_->setScale({s, s});
}

void PieceSprite::centerOrigin() {
    if (!sprite_) return;
    sf::FloatRect b = sprite_->getLocalBounds(); // {left, top, width, height}
    sprite_->setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
}

void PieceSprite::updateSpritePosition(const float x, const float y) {
    if (sprite_) sprite_.value().setPosition({x, y});
}

PieceSprite& Square::pieceSprite() {
    return pieceSprite_;
}

bool Square::isEmpty() const {
    return !pieceSprite_.exists();
}

Highlight Square::highlight() const {
    if(highlight_) return highlight_.value();
    // no highlight, throw
    throw std::runtime_error("Attempted to retrieve highlight from Square with no highlight.");
}

bool Square::hasHighlight() const {
    return highlight_.has_value();
}

void Square::setHighlight(Highlight highlight) {
    highlight_ = highlight;
}

void Square::toggleHighlight(Highlight highlight) {
    if(highlight_) {
        highlight_.reset();
    } else {
        highlight_ = highlight;
    }
}

void Square::clearHighlight() {
    highlight_.reset();
}

void Square::clearHighlight(Highlight highlight) {
    if(highlight_ && highlight_.value() == highlight) {
        highlight_.reset();
    }
}

int Board::getSquareIndexFromCoordinates(int x, int y) {
    int row = static_cast<int>(y / SQUARE_WIDTH);
    int col = static_cast<int>(x / SQUARE_HEIGHT);
    return 8 * row + col;
}

Square& Board::at(int i) {
    return board_.at(i);
}

void Board::draw(sf::RenderWindow& window) const {
    // other overload skips over nothing given std::nullopt
    draw(window, std::nullopt);
}

void Board::draw(sf::RenderWindow& window, const std::optional<int> heldSquare) const {
    // draw row by row
    for(int squareIndex = 0; squareIndex < 64; squareIndex++) {
        // get row and col from index
        const int row = squareIndex/8;
        const int col = squareIndex%8;
        const bool isLight = row%2 == col%2;
        Square squareObject = board_.at(squareIndex);

        sf::RectangleShape squareShape{{SQUARE_WIDTH, SQUARE_HEIGHT}};

        // determine square color
        sf::Color color;
        if(squareObject.hasHighlight()) {
            color = isLight ? squareObject.highlight().lightHighlight : squareObject.highlight().darkHighlight;
        } else {
            color = isLight ? LIGHT_SQUARE_COLOR : DARK_SQUARE_COLOR;
        }
        squareShape.setFillColor(color);

        // set position based on row/col
        float xpos = SQUARE_WIDTH * col;
        float ypos = SQUARE_HEIGHT * row;
        squareShape.setPosition({xpos, ypos});
        window.draw(squareShape);

        // skip empty squares or square that is currently held
        if(squareObject.isEmpty() || (heldSquare && heldSquare.value() == squareIndex)) continue;

        if (const sf::Sprite* sp = squareObject.pieceSprite().sprite()) {
            window.draw(*sp);
        }
    }
}

void Board::clearAllHighlights() {
    for(Square& square : board_) square.clearHighlight();
}

void Board::clearAllHighlights(const Highlight highlight) {
    for(Square& square : board_) square.clearHighlight(highlight);
}

void Board::updateBoardFromGame(const Game& game) {
    // TODO: consider making more performant by checking equality before updating for each piece
    int squareIndex = 0;
    for(const Piece& piece : game.board()) {
        int row = squareIndex / 8;
        int col = squareIndex % 8;

        float xpos = SQUARE_WIDTH * col;
        float ypos = SQUARE_WIDTH * row;

        Square& square = board_.at(squareIndex);
        square.pieceSprite() = PieceSprite{piece};
        // fit to center of square
        square.pieceSprite().centerOrigin();
        square.pieceSprite().fitToSquare(SQUARE_WIDTH * 0.97f);
        square.pieceSprite().updateSpritePosition(xpos + SQUARE_WIDTH/2.f, ypos + SQUARE_HEIGHT/2.f);

        squareIndex++;
    }
}
#pragma once

#include <SFML/Graphics.hpp>

#include "../game/Game.hpp"
#include "Highlight.hpp"

// Cache which maintains and retrieves textures for Pieces.
class TextureCache {
public:
    // Retrieve texture from piece type and color.
    static const sf::Texture& get(PieceType type, Color color);

private:
    // Retrieve path to texture from piece type and color.
    static std::string makePath_(PieceType type, Color color);
};


class PieceSprite : public Piece {
public:
    // Construct empty PieceSprite with empty square Piece.
    PieceSprite() = default;
    // Construct PieceSprite from Piece.
    explicit PieceSprite(Piece piece);

    // Return pointer to sprite, or nullptr if no sprite exists.
    const sf::Sprite* sprite() const;
    
    // Rebuild sprite based on current Piece.
    void rebuild();
    // Fit sprite to given squareSize.
    void fitToSquare(float squareSize);
    // Center sprite origin.
    void centerOrigin();
    // Update sprite position to x and y.
    void updateSpritePosition(float xPos, float yPos);

private:
    // Corresponding sprite for PieceSprite. Does not exist if Piece is the empty square.
    std::optional<sf::Sprite> sprite_;
};

class Square {
public:
    // Construct a square, which always has a PieceSprite, and sometimes a Highlight.
    Square() = default;

    // Retrieve PieceSprite.
    PieceSprite& pieceSprite() { return pieceSprite_; }
    // Retrieve highlight. Assumes highlight exists, and throws if it does not.
    Highlight highlight() const;

    // If the square is empty. I.e., it has no piece on it.
    bool isEmpty() const { return !pieceSprite_.exists(); }

    // If the square has a highlight.
    bool hasHighlight() const { return highlight_.has_value(); }
    // Set the square's highlight.
    void setHighlight(Highlight highlight) { highlight_ = highlight; }
    // Clear the square's highlight.
    void clearHighlight() { highlight_.reset(); }
    // Clear the square's highlight if it matches the input highlight.
    void clearHighlight(Highlight highlight);
    // Toggle the square's highlight if it matches the input highlight.
    void toggleHighlight(Highlight highlight);

private:
    // The square's PieceSprite.
    PieceSprite pieceSprite_;
    // The square's highlight. Does not exist if the square does not have a highlight.
    std::optional<Highlight> highlight_;
};

// Manages graphical representation of the board.
class BoardView{
public:
    // Construct an empty board. I.e., a board with all empty squares.
    BoardView() = default;
    // Retrieve square at a given board square. Throws if out of range.
    Square& at(int squareIndex);
    // Read-only version of at(). Throws if out of range.
    Square at(int squareIndex) const;

    // Update board representation given game.
    void updateBoardFromGame(const Game& game);

    // Clear all highlights.
    void clearAllHighlights();
    // Clear all highlights that match inputted highlight.
    void clearAllHighlights(Highlight highlightToClear);
    // Clear all highlights, except the given one.
    void clearAllHighlightsExcept(Highlight highlightToSkip);

    // Get the index of the square given the x, y coordinates.
    static int getSquareIndexFromCoordinates(int xPos, int yPos);

private:
    // Board representation: an array of squares.
    std::array<Square, Utils::NUM_SQUARES> board_;
};
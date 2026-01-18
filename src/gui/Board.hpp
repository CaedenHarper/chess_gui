#pragma once

#include <SFML/Graphics.hpp>

#include "../game/Game.hpp"

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

class Highlight {
public:
    // Construct a Highlight.
    constexpr Highlight(sf::Color light, sf::Color dark): lightHighlight_{light}, darkHighlight_{dark} {};
    // Compare a Highlight to another. Two highlights are equal if both of their colors are the same.
    bool operator==(Highlight other) const { return lightHighlight_ == other.lightHighlight_ && darkHighlight_ == other.darkHighlight_; };
    sf::Color lightHighlight() const { return lightHighlight_; };
    sf::Color darkHighlight() const { return darkHighlight_; };

private:
    // Color for the light squares.
    sf::Color lightHighlight_;
    // Color for the dark squares.
    sf::Color darkHighlight_;
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
class Board{
public:
    // Width of the board in pixels.
    static constexpr int BOARD_WIDTH = 800;
    // Height of the board in pixels.
    static constexpr int BOARD_HEIGHT = 800;
    // Width of a square in pixels.
    static constexpr float SQUARE_WIDTH = BOARD_WIDTH/8.F;
    // Height of a square in pixels.
    static constexpr float SQUARE_HEIGHT = BOARD_HEIGHT/8.F;

    // Normal dark square color. Dark brown, rgb(179, 136, 98).
    static constexpr sf::Color DARK_SQUARE_COLOR{179, 136, 98};
    // Normal light square color. Beige, rgb(238, 217, 179).
    static constexpr sf::Color LIGHT_SQUARE_COLOR{238, 217, 179};
    // Right click dark square color. Darker red, rgb(211, 107, 80).
    static constexpr sf::Color DARK_HIGHLIGHT_SQUARE_COLOR{211, 107, 80};
    // Right click light square color. Lighter red, rgb(236, 125, 106).
    static constexpr sf::Color LIGHT_HIGHLIGHT_SQUARE_COLOR{236, 125, 106};
    static constexpr Highlight RIGHT_CLICK_HIGHLIGHT{LIGHT_HIGHLIGHT_SQUARE_COLOR, DARK_HIGHLIGHT_SQUARE_COLOR};
    // Legal dark square color. Darker green, rgb(68, 151, 48).
    static constexpr sf::Color DARK_LEGAL_SQUARE_COLOR{68, 151, 48};
    // Legal light square color. Lighter green, rgb(91, 177, 70).
    static constexpr sf::Color LIGHT_LEGAL_SQUARE_COLOR{91, 177, 70};
    static constexpr Highlight LEGAL_HIGHLIGHT{LIGHT_LEGAL_SQUARE_COLOR, DARK_LEGAL_SQUARE_COLOR};
    // Selected move dark square color. Darker yellow, rgb(200, 221, 80).
    static constexpr sf::Color DARK_SELECTED_SQUARE_COLOR{200, 221, 80};
    // Selected move light square color. Lighter yellow, rgb(237, 253, 142).
    static constexpr sf::Color LIGHT_SELECTED_SQUARE_COLOR{237, 253, 142};
    static constexpr Highlight SELECTED_HIGHLIGHT{LIGHT_SELECTED_SQUARE_COLOR, DARK_SELECTED_SQUARE_COLOR};

    // Construct an empty board. I.e., a board with all empty squares.
    Board() = default;
    // Retrieve square at a given board square. Throws if out of range.
    Square& at(int squareIndex);

    // Draw board to window.
    void draw(sf::RenderWindow& window) const;
    // Draw board to window, skipping over heldSquare. If heldSquare does not exist, draws all squares.
    void draw(sf::RenderWindow& window, std::optional<int> heldSquare) const;

    // Update board representation given game.
    void updateBoardFromGame(const Game& game);

    // Clear all highlights.
    void clearAllHighlights();
    // Clear all highlights that match inputted highlight.
    void clearAllHighlights(Highlight highlight);

    // Get the index of the square given the x, y coordinates.
    static int getSquareIndexFromCoordinates(int xPos, int yPos);

private:
    // Board representation: an array of squares.
    std::array<Square, Game::NUM_SQUARES> board_;
};
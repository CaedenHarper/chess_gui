#pragma once

#include <SFML/Graphics.hpp>
#include <sys/stat.h>

#include "../game/Game.hpp"

class TextureCache {
public:
    static const sf::Texture& get(PieceType type, Color color);

private:
    static std::string makePath(PieceType type, Color color);
};


class PieceSprite : public Piece {
public:
    PieceSprite() {};
    PieceSprite(Piece piece);

    void rebuild();

    void fitToSquare(float squareSize);

    void centerOrigin();

    const sf::Sprite* sprite() const;

    void updateSpritePosition(float x, float y);

private:
    std::optional<sf::Sprite> sprite_;
};

class Highlight {
public:
    constexpr Highlight(sf::Color light, sf::Color dark): lightHighlight{light}, darkHighlight{dark} {};
    sf::Color lightHighlight;
    sf::Color darkHighlight;
};

class Square {
public:
    Square() {};

    PieceSprite& pieceSprite();

    bool isEmpty() const;

    Highlight highlight() const;
    bool hasHighlight() const;
    void setHighlight(Highlight highlight);
    void clearHighlight();
    void toggleHighlight(Highlight highlight);

private:
    PieceSprite pieceSprite_;
    std::optional<Highlight> highlight_;
};

/*
    Manages graphical representation of the board.
*/
class Board{
public:
    static constexpr int BOARD_WIDTH = 800;
    static constexpr int BOARD_HEIGHT = 800;
    static constexpr float squareWidth = BOARD_WIDTH/8.f;
    static constexpr float squareHeight = BOARD_HEIGHT/8.f;

    // Dark brown
    // rgb(179, 136, 98)
    static constexpr sf::Color DARK_SQUARE_COLOR{179, 136, 98};
    // Beige
    // rgb(238, 217, 179)
    static constexpr sf::Color LIGHT_SQUARE_COLOR{238, 217, 179};
    // Darker red, right-click dark highlight color
    // rgb(211, 107, 80)
    static constexpr sf::Color DARK_HIGHLIGHT_SQUARE_COLOR{211, 107, 80};
    // Lighter red, right-click light highlight color
    // rgb(236, 125, 106)
    static constexpr sf::Color LIGHT_HIGHLIGHT_SQUARE_COLOR{236, 125, 106};
    static constexpr Highlight RIGHT_CLICK_HIGHLIGHT{LIGHT_HIGHLIGHT_SQUARE_COLOR, DARK_HIGHLIGHT_SQUARE_COLOR};
    // Darker green, legal move dark highlight color
    // rgb(68, 151, 48)
    static constexpr sf::Color DARK_LEGAL_SQUARE_COLOR{68, 151, 48};
    // Lighter red, legal move light highlight color
    // rgb(91, 177, 70)
    static constexpr sf::Color LIGHT_LEGAL_SQUARE_COLOR{91, 177, 70};
    static constexpr Highlight LEGAL_HIGHLIGHT{LIGHT_LEGAL_SQUARE_COLOR, DARK_LEGAL_SQUARE_COLOR};

    Board() {};
    Square& at(int i);
    void draw(sf::RenderWindow& window) const;
    void updateBoardFromGame(const Game& game);
    void resetAllHighlights();
    static int getSquareIndexFromCoordinates(int x, int y);

private:
    std::array<Square, 64> board_;
};
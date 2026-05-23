#pragma once

#include <SFML/Graphics.hpp>

#include "Highlight.hpp"
#include "../game/Piece.hpp"

#include "../game/Utils.hpp"


namespace RenderUtils {
    // -- CONSTANTS --
    inline constexpr int BOARD_WIDTH_PX = 800;
    inline constexpr int BOARD_HEIGHT_PX = 800;
    inline constexpr float SQUARE_WIDTH_PX = BOARD_WIDTH_PX/8.F;
    inline constexpr float SQUARE_HEIGHT_PX = BOARD_HEIGHT_PX/8.F;
    inline constexpr float SPRITE_SCALE_FACTOR = 0.97F;

    // Normal dark square color. Dark brown, rgb(179, 136, 98).
    inline constexpr sf::Color DARK_SQUARE_COLOR{179, 136, 98};
    // Normal light square color. Beige, rgb(238, 217, 179).
    inline constexpr sf::Color LIGHT_SQUARE_COLOR{238, 217, 179};
    // Right click dark square color. Darker red, rgb(211, 107, 80).
    inline constexpr sf::Color DARK_HIGHLIGHT_SQUARE_COLOR{211, 107, 80};
    // Right click light square color. Lighter red, rgb(236, 125, 106).
    inline constexpr sf::Color LIGHT_HIGHLIGHT_SQUARE_COLOR{236, 125, 106};
    inline constexpr Highlight RIGHT_CLICK_HIGHLIGHT{LIGHT_HIGHLIGHT_SQUARE_COLOR, DARK_HIGHLIGHT_SQUARE_COLOR};
    // Legal dark square color. Darker green, rgb(68, 151, 48).
    inline constexpr sf::Color DARK_LEGAL_SQUARE_COLOR{68, 151, 48};
    // Legal light square color. Lighter green, rgb(91, 177, 70).
    inline constexpr sf::Color LIGHT_LEGAL_SQUARE_COLOR{91, 177, 70};
    inline constexpr Highlight LEGAL_HIGHLIGHT{LIGHT_LEGAL_SQUARE_COLOR, DARK_LEGAL_SQUARE_COLOR};
    // Selected move dark square color. Darker yellow, rgb(200, 221, 80).
    inline constexpr sf::Color DARK_SELECTED_SQUARE_COLOR{200, 221, 80};
    // Selected move light square color. Lighter yellow, rgb(237, 253, 142).
    inline constexpr sf::Color LIGHT_SELECTED_SQUARE_COLOR{237, 253, 142};
    inline constexpr Highlight SELECTED_HIGHLIGHT{LIGHT_SELECTED_SQUARE_COLOR, DARK_SELECTED_SQUARE_COLOR};
    // Check move dark square color. Darker orange, rgb(233, 114, 60).
    inline constexpr sf::Color DARK_CHECK_SQUARE_COLOR{233, 114, 60};
    // Check move light square color. Lighter orange, rgb(224, 139, 100).
    inline constexpr sf::Color LIGHT_CHECK_SQUARE_COLOR{224, 139, 100};
    inline constexpr Highlight CHECK_HIGHLIGHT{LIGHT_CHECK_SQUARE_COLOR, DARK_CHECK_SQUARE_COLOR};
    // Cyan dark square color. Darker cyan, rgb(33, 180, 206).
    inline constexpr sf::Color DARK_CYAN_SQUARE_COLOR{33, 180, 206};
    // Cyan light square color. Lighter cyan, rgb(99, 208, 228).
    inline constexpr sf::Color LIGHT_CYAN_SQUARE_COLOR{99, 208, 228};
    inline constexpr Highlight CYAN_HIGHLIGHT{LIGHT_CYAN_SQUARE_COLOR, DARK_CYAN_SQUARE_COLOR};

    
    // -- HELPER FUNCTIONS --
    constexpr int getSquareFromDisplayPerspective(int square, Color displayColor) {
        if(displayColor == Color::White) {
            return square;
        }

        // displayColor == Black; flip 180 degrees
        return (Utils::NUM_SQUARES - 1) - square;
    }

    constexpr int getSquareIndexFromCoordinates(int xPos, int yPos, Color displayColor) {
        const int row = static_cast<int>(yPos / SQUARE_HEIGHT_PX);
        const int col = static_cast<int>(xPos / SQUARE_WIDTH_PX);
        return getSquareFromDisplayPerspective(Utils::getSquareIndex(col, row), displayColor);
    }

    constexpr sf::Vector2f squareCenterPx(const int squareIndex) {
        const int row = Utils::getRow(squareIndex);
        const int col = Utils::getCol(squareIndex);

        const float xPos = (SQUARE_WIDTH_PX * col) + (SQUARE_WIDTH_PX * 0.5F);
        const float yPos = (SQUARE_HEIGHT_PX * row) + (SQUARE_HEIGHT_PX * 0.5F);

        return {xPos, yPos};
    }

    constexpr bool isSquareLight(int row, int col) {
        return row%2 == col%2;
    }

    constexpr bool isSquareLight(int squareIndex) {
        const int row = Utils::getRow(squareIndex);
        const int col = Utils::getCol(squareIndex);

        return isSquareLight(row, col);
    }
} // namespace RenderUtils
#pragma once

#include <SFML/System/Vector2.hpp>

#include <SFML/Graphics.hpp>

#include "../game/Game.hpp"
#include "BoardView.hpp"

#include <optional>

enum class InputResult : std::int8_t {
    None,
    MoveMade
};

struct HeldPieceState {
    int heldSquare;
    sf::Vector2f mousePos{0.F, 0.F};
    bool isDragging; // is the piece being dragged or being click-click moved?
} __attribute__((aligned(16))); // NOLINT[magic numbers] align to 16 bytes

class InputHandler {
public:
    explicit InputHandler(BoardView* board) : board_{board} {

    }

    InputResult handleEvent(const sf::Event& event, Game& game);

    const std::optional<HeldPieceState>& heldPiece() const {
        return heldPiece_;
    }

private:
    InputResult mouseClickEvent(const sf::Event::MouseButtonPressed& event, Game& game);
    InputResult mouseMovementEvent(const sf::Event::MouseMoved& event);
    InputResult mouseUnclickEvent(const sf::Event::MouseButtonReleased& event, Game& game);
    InputResult leftClickEvent(const sf::Event::MouseButtonPressed& event, Game& game);
    InputResult rightClickEvent(const sf::Event::MouseButtonPressed& event);
    
    std::optional<HeldPieceState> heldPiece_;
    BoardView* board_;
};
#pragma once

#include <SFML/System/Vector2.hpp>

#include <SFML/Graphics.hpp>

#include "../game/Game.hpp"

#include <optional>

class InputResult {
public:
    enum class Type : std::int8_t {
        None,
        InvalidMove,
        MoveMade,
        RedHighlight,
        SelectedSquare
    };

    static InputResult none() {
        return InputResult{Type::None, std::nullopt};
    }

    static InputResult invalidMove() {
        return InputResult{Type::InvalidMove, std::nullopt};
    }

    static InputResult moveMade() {
        return InputResult{Type::MoveMade, std::nullopt};
    }

    static InputResult redHighlight(int square) {
        return InputResult{Type::RedHighlight, square};
    }

    static InputResult selectedSquare(int square) {
        return InputResult{Type::SelectedSquare, square};
    }

    Type type() const {
        return type_;
    }

    std::optional<int> square() const {
        return square_;
    }

private:
    // Only allow the valid states above
    InputResult(Type type, std::optional<int> square)
        : type_{type}, square_{square} {}

    Type type_ = Type::None;
    std::optional<int> square_ = std::nullopt;
};

struct HeldPieceState {
    int heldSquare;
    sf::Vector2f mousePos{0.F, 0.F};
    bool isDragging; // is the piece being dragged or being click-click moved?
} __attribute__((aligned(16))); // NOLINT[magic numbers] align to 16 bytes

class InputHandler {
public:
    InputHandler() = default;

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
};
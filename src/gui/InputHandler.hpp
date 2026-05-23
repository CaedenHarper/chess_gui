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
        RedHighlight
    };

    static InputResult none(bool cancelRedHighlights = false) {
        return InputResult{Type::None, std::nullopt, cancelRedHighlights};
    }

    static InputResult invalidMove(bool cancelRedHighlights = true) {
        return InputResult{Type::InvalidMove, std::nullopt, cancelRedHighlights};
    }

    static InputResult moveMade(bool cancelRedHighlights = true) {
        return InputResult{Type::MoveMade, std::nullopt, cancelRedHighlights};
    }

    static InputResult redHighlight(int square) {
        return InputResult{Type::RedHighlight, square, false};
    }

    Type type() const {
        return type_;
    }

    std::optional<int> square() const {
        return square_;
    }

    bool clearRedHighlights() const {
        return clearRedHighlights_;
    }

private:
    // Only allow the valid states above
    InputResult(Type type, std::optional<int> square, bool clearRedHighlights)
        : type_{type}, square_{square}, clearRedHighlights_(clearRedHighlights) {}

    Type type_ = Type::None;
    std::optional<int> square_ = std::nullopt;
    bool clearRedHighlights_;
};

enum class InputMode : std::uint8_t {
    FullGameplay,
    BoardAnnotationsOnly,
    Disabled
};

struct HeldPieceState {
    int heldSquare;
    sf::Vector2f mousePos{0.F, 0.F};
    bool isDragging; // is the piece being dragged or being click-click moved?
} __attribute__((aligned(16))); // NOLINT[magic numbers] align to 16 bytes

class InputHandler {
public:
    InputHandler() = default;

    InputResult handleEvent(const sf::Event& event, Game& game, Color playerColor, Color displayColor, InputMode mode);

    const std::optional<HeldPieceState>& heldPiece() const {
        return heldPiece_;
    }

private:
    InputResult mouseClickEvent(const sf::Event::MouseButtonPressed& event, Game& game, Color playerColor, Color displayColor, InputMode mode);
    InputResult mouseMovementEvent(const sf::Event::MouseMoved& event);
    InputResult mouseUnclickEvent(const sf::Event::MouseButtonReleased& event, Game& game, Color displayColor, InputMode mode);
    InputResult leftClickEvent(const sf::Event::MouseButtonPressed& event, Game& game, Color playerColor, Color displayColor, InputMode mode);
    InputResult rightClickEvent(const sf::Event::MouseButtonPressed& event, Color displayColor);
    
    std::optional<HeldPieceState> heldPiece_;
};
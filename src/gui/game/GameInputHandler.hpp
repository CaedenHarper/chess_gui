#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "../../game/Game.hpp"

#include <optional>

class GameInputResult {
public:
    enum class Type : std::int8_t { None, InvalidMove, MoveMade, RedHighlight };

    static GameInputResult none(bool cancelRedHighlights = false) {
        return GameInputResult{Type::None, std::nullopt, cancelRedHighlights};
    }

    static GameInputResult invalidMove(bool cancelRedHighlights = true) {
        return GameInputResult{Type::InvalidMove, std::nullopt, cancelRedHighlights};
    }

    static GameInputResult moveMade(bool cancelRedHighlights = true) {
        return GameInputResult{Type::MoveMade, std::nullopt, cancelRedHighlights};
    }

    static GameInputResult redHighlight(int square) {
        return GameInputResult{Type::RedHighlight, square, false};
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
    GameInputResult(Type type, std::optional<int> square, bool clearRedHighlights)
        : type_{type}, square_{square}, clearRedHighlights_(clearRedHighlights) {
    }

    Type type_ = Type::None;
    std::optional<int> square_ = std::nullopt;
    bool clearRedHighlights_;
};

enum class InputMode : std::uint8_t { FullGameplay, BoardAnnotationsOnly, Disabled };

struct HeldPieceState {
    int heldSquare;
    sf::Vector2f mousePos{0.F, 0.F};
    bool isDragging; // is the piece being dragged or being click-click moved?
} __attribute__((aligned(16))); // NOLINT[magic numbers] align to 16 bytes

class GameInputHandler {
public:
    GameInputHandler() = default;

    GameInputResult handleEvent(
        const sf::Event& event,
        Game& game,
        Color playerColor,
        Color displayColor,
        InputMode mode
    );

    const std::optional<HeldPieceState>& heldPiece() const {
        return heldPiece_;
    }

private:
    GameInputResult mouseClickEvent(
        const sf::Event::MouseButtonPressed& event,
        Game& game,
        Color playerColor,
        Color displayColor,
        InputMode mode
    );
    GameInputResult mouseMovementEvent(const sf::Event::MouseMoved& event);
    GameInputResult mouseUnclickEvent(
        const sf::Event::MouseButtonReleased& event,
        Game& game,
        Color displayColor,
        InputMode mode
    );
    GameInputResult leftClickEvent(
        const sf::Event::MouseButtonPressed& event,
        Game& game,
        Color playerColor,
        Color displayColor,
        InputMode mode
    );
    GameInputResult rightClickEvent(const sf::Event::MouseButtonPressed& event, Color displayColor);

    std::optional<HeldPieceState> heldPiece_;
};
#include "GameInputHandler.hpp"

#include "../resources/RenderUtils.hpp"

GameInputResult GameInputHandler::handleEvent(
    const sf::Event& event,
    Game& game,
    Color playerColor,
    Color displayColor,
    InputMode mode
) {
    if(mode == InputMode::Disabled) {
        return GameInputResult::none();
    }

    if(const auto* mouseClicked = event.getIf<sf::Event::MouseButtonPressed>()) {
        return mouseClickEvent(*mouseClicked, game, playerColor, displayColor, mode);
    }

    if(const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        return mouseMovementEvent(*mouseMoved);
    }

    if(const auto* mouseUnclicked = event.getIf<sf::Event::MouseButtonReleased>()) {
        return mouseUnclickEvent(*mouseUnclicked, game, displayColor, mode);
    }

    return GameInputResult::none();
}

GameInputResult GameInputHandler::mouseClickEvent(
    const sf::Event::MouseButtonPressed& event,
    Game& game,
    Color playerColor,
    Color displayColor,
    InputMode mode
) {
    if(event.button == sf::Mouse::Button::Right) {
        return rightClickEvent(event, displayColor);
    }

    if(event.button == sf::Mouse::Button::Left) {
        return leftClickEvent(event, game, playerColor, displayColor, mode);
    }

    return GameInputResult::none();
}

GameInputResult GameInputHandler::mouseMovementEvent(const sf::Event::MouseMoved& event) {
    if(!heldPiece_) {
        return GameInputResult::none();
    };

    heldPiece_.value().mousePos =
        sf::Vector2f{static_cast<float>(event.position.x), static_cast<float>(event.position.y)};

    return GameInputResult::none();
}

GameInputResult GameInputHandler::mouseUnclickEvent(
    const sf::Event::MouseButtonReleased& event,
    Game& game,
    Color displayColor,
    InputMode mode
) {
    // only allow left click releases on the physical board
    if(event.position.x > RenderUtils::BOARD_WIDTH_PX || event.position.y > RenderUtils::BOARD_HEIGHT_PX) {
        // release piece if we click oob
        heldPiece_.reset();
        return GameInputResult::none();
    }

    const int targetSquare =
        RenderUtils::getSquareIndexFromCoordinates(event.position.x, event.position.y, displayColor);

    // out of bounds
    if(!Utils::onBoard(targetSquare)) {
        return GameInputResult::none();
    }

    // remaining unclick operations require a held piece; exit early if no held piece exists
    if(!heldPiece_) {
        return GameInputResult::none();
    }

    const int sourceSquare = heldPiece_->heldSquare;

    // same square means we should try click-click move instead of dragging move
    // therefore, we do not reset heldSquare
    if(sourceSquare == targetSquare) {
        heldPiece_->isDragging = false;
        return GameInputResult::none();
    }

    // We exit early before actually making the move if it's not our turn
    if(mode != InputMode::FullGameplay) {
        heldPiece_.reset();
        return GameInputResult::none(true);
    }

    // move is on board and different square
    const Move potentialMove =
        Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
    // if move is legal, try it
    if(game.tryMove(potentialMove)) {
        heldPiece_.reset();
        return GameInputResult::moveMade();
    }

    heldPiece_.reset();
    return GameInputResult::invalidMove();
}

GameInputResult GameInputHandler::leftClickEvent(
    const sf::Event::MouseButtonPressed& event,
    Game& game,
    Color playerColor,
    Color displayColor,
    InputMode mode
) {
    const sf::Vector2i mousePos = event.position;

    // only allow left clicks on the physical board
    if(mousePos.x > RenderUtils::BOARD_WIDTH_PX || mousePos.y > RenderUtils::BOARD_HEIGHT_PX) {
        return GameInputResult::none(true);
    }

    const int targetSquare = RenderUtils::getSquareIndexFromCoordinates(mousePos.x, mousePos.y, displayColor);

    // target square does not exist; reset any selected piece
    if(!Utils::onBoard(targetSquare)) {
        heldPiece_.reset();
        return GameInputResult::none(true);
    }

    // target square does exist, but no currently held piece
    if(!heldPiece_) {
        // no need to do additional processing for clicking on empty square, or wrong player's piece
        // TODO: GameInputHandler should not use mailbox(); rewrite with Game function
        if(!game.mailbox().at(targetSquare).exists() || game.mailbox().at(targetSquare).color() != playerColor) {
            return GameInputResult::none(true);
        }

        // hold square
        heldPiece_ =
            HeldPieceState{targetSquare, {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)}, true};

        return GameInputResult::none(true);
    }

    // currently held piece exists; click-click move
    const int sourceSquare = heldPiece_->heldSquare;
    // if same square, we cancel move
    if(sourceSquare == targetSquare) {
        heldPiece_.reset();
        return GameInputResult::none(true);
    }

    // We exit early before actually making the move if it's not our turn
    if(mode != InputMode::FullGameplay) {
        return GameInputResult::none(true);
    }

    // Try to make click-click move
    const Move potentialMove =
        Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
    if(game.tryMove(potentialMove)) {
        heldPiece_.reset();
        return GameInputResult::moveMade();
    }

    heldPiece_.reset();
    return GameInputResult::invalidMove();
}

GameInputResult GameInputHandler::rightClickEvent(const sf::Event::MouseButtonPressed& event, Color displayColor) {
    // right click cancels any held square
    if(heldPiece_) {
        heldPiece_.reset();
        return GameInputResult::none();
    }

    // only allow right clicks on the physical board
    const sf::Vector2i mousePos = event.position;
    if(mousePos.x > RenderUtils::BOARD_WIDTH_PX || mousePos.y > RenderUtils::BOARD_HEIGHT_PX) {
        return GameInputResult::none();
    }

    // swap highlight status of square
    const int targetSquare = RenderUtils::getSquareIndexFromCoordinates(mousePos.x, mousePos.y, displayColor);
    if(!Utils::onBoard(targetSquare)) {
        return GameInputResult::none();
    }

    return GameInputResult::redHighlight(targetSquare);
}

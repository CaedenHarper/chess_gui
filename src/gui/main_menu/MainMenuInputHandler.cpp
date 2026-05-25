#include "MainMenuInputHandler.hpp"

MainMenuInputResult MainMenuInputHandler::handleEvent(const sf::Event& event) {
}

MainMenuInputResult MainMenuInputHandler::mouseClickEvent(const sf::Event::MouseButtonPressed& event) {
}

MainMenuInputResult MainMenuInputHandler::leftClickEvent(
    const sf::Event::MouseButtonPressed& event,
    Game& game,
    Color playerColor,
    Color displayColor,
    InputMode mode
) {
    const sf::Vector2i mousePos = event.position;

    // only allow left clicks on the physical board
    if(mousePos.x > RenderUtils::BOARD_WIDTH_PX || mousePos.y > RenderUtils::BOARD_HEIGHT_PX) {
        return MainMenuInputResult::none(true);
    }

    const int targetSquare = RenderUtils::getSquareIndexFromCoordinates(mousePos.x, mousePos.y, displayColor);

    // target square does not exist; reset any selected piece
    if(!Utils::onBoard(targetSquare)) {
        heldPiece_.reset();
        return MainMenuInputResult::none(true);
    }

    // target square does exist, but no currently held piece
    if(!heldPiece_) {
        // no need to do additional processing for clicking on empty square, or wrong player's piece
        // TODO: MainMenuInputHandler should not use mailbox(); rewrite with Game function
        if(!game.mailbox().at(targetSquare).exists() || game.mailbox().at(targetSquare).color() != playerColor) {
            return MainMenuInputResult::none(true);
        }

        // hold square
        heldPiece_ =
            HeldPieceState{targetSquare, {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)}, true};

        return MainMenuInputResult::none(true);
    }

    // currently held piece exists; click-click move
    const int sourceSquare = heldPiece_->heldSquare;
    // if same square, we cancel move
    if(sourceSquare == targetSquare) {
        heldPiece_.reset();
        return MainMenuInputResult::none(true);
    }

    // We exit early before actually making the move if it's not our turn
    if(mode != InputMode::FullGameplay) {
        return MainMenuInputResult::none(true);
    }

    // Try to make click-click move
    const Move potentialMove =
        Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
    if(game.tryMove(potentialMove)) {
        heldPiece_.reset();
        return MainMenuInputResult::moveMade();
    }

    heldPiece_.reset();
    return MainMenuInputResult::invalidMove();
}
#include "RenderUtils.hpp"
#include "InputHandler.hpp"

InputResult InputHandler::handleEvent(const sf::Event& event, Game& game) {
    if(const auto* mouseClicked = event.getIf<sf::Event::MouseButtonPressed>()) {
        return mouseClickEvent(*mouseClicked, game);
    }

    if(const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        return mouseMovementEvent(*mouseMoved);
    }

    if(const auto* mouseUnclicked = event.getIf<sf::Event::MouseButtonReleased>()) {
        return mouseUnclickEvent(*mouseUnclicked, game);
    }

    return InputResult::none();
}

InputResult InputHandler::mouseClickEvent(const sf::Event::MouseButtonPressed& event, Game& game) {
        if(event.button == sf::Mouse::Button::Left) {
            return leftClickEvent(event, game);
        }

        if(event.button == sf::Mouse::Button::Right) {
            return rightClickEvent(event);
        }

        return InputResult::none();
}

InputResult InputHandler::mouseMovementEvent(const sf::Event::MouseMoved& event) {
    if(!heldPiece_) {
        return InputResult::none();
    };

    heldPiece_.value().mousePos = sf::Vector2f{static_cast<float>(event.position.x), static_cast<float>(event.position.y)};

    return InputResult::none();
}

InputResult InputHandler::mouseUnclickEvent(const sf::Event::MouseButtonReleased& event, Game& game) {    
    // only allow left click releases on the physical board
    if(event.position.x > RenderUtils::BOARD_WIDTH_PX || event.position.y > RenderUtils::BOARD_HEIGHT_PX) {
        // release piece if we click oob
        heldPiece_.reset();
        return InputResult::none();
    }

    const int targetSquare = RenderUtils::getSquareIndexFromCoordinates(event.position.x, event.position.y);
    
    // out of bounds
    if(!Utils::onBoard(targetSquare)) {
        return InputResult::none();
    }

    // remaining unclick operations require a held piece; exit early if no held piece exists
    if(!heldPiece_) {
        return InputResult::none();
    }

    const int sourceSquare = heldPiece_.value().heldSquare;

    // same square means we should try click-click move instead of dragging move
    // therefore, we do not reset heldSquare
    if(sourceSquare == targetSquare) {
        heldPiece_->isDragging = false;
        return InputResult::none();
    }

    // move is on board and different square
    const Move potentialMove = Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
    // if move is legal, try it
    if (game.tryMove(potentialMove)) {
        heldPiece_.reset();
        return InputResult::moveMade();
    }

    heldPiece_.reset();
    return InputResult::invalidMove();
}

InputResult InputHandler::leftClickEvent(const sf::Event::MouseButtonPressed& event, Game& game) {
    const sf::Vector2i mousePos = event.position;

    // only allow left clicks on the physical board
    if(mousePos.x > RenderUtils::BOARD_WIDTH_PX || mousePos.y > RenderUtils::BOARD_HEIGHT_PX) {
        return InputResult::none();
    }

    const int targetSquare = RenderUtils::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
    
    // target square does not exist; reset any selected piece
    if(!Utils::onBoard(targetSquare)) {
        heldPiece_.reset();
        return InputResult::none();
    }

    // target square does exist, but no currently held piece
    if(!heldPiece_) {
        // no need to do additional processing for clicking on empty square, or wrong player's piece
        if(!game.mailbox().at(targetSquare).exists() || game.mailbox().at(targetSquare).color() != game.sideToMove()) {
            return InputResult::none();
        }

        // hold square
        heldPiece_ = HeldPieceState{targetSquare, {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)}, true};

        return InputResult::none();
    }

    // currently held piece exists; click-click move
    const int sourceSquare = heldPiece_.value().heldSquare;
    // if same square, we cancel move
    if(sourceSquare == targetSquare) {
        heldPiece_.reset();
        return InputResult::none();
    }
    
    // Try to make click-click move
    const Move potentialMove = Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
    if(game.tryMove(potentialMove)) {
        heldPiece_.reset();
        return InputResult::moveMade();
    }
    
    heldPiece_.reset();
    return InputResult::invalidMove();
}

InputResult InputHandler::rightClickEvent(const sf::Event::MouseButtonPressed& event) {
    // right click cancels any held square
    heldPiece_.reset();

    // only allow right clicks on the physical board
    const sf::Vector2i mousePos = event.position;
    if(mousePos.x > RenderUtils::BOARD_WIDTH_PX || mousePos.y > RenderUtils::BOARD_HEIGHT_PX) {
        return InputResult::none();
    }

    // swap highlight status of square
    const int targetSquare = RenderUtils::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
    if(Utils::onBoard(targetSquare)) {
        return InputResult::redHighlight(targetSquare);
    }

    return InputResult::none();
}

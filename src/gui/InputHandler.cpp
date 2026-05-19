#include "Constants.hpp"
#include "BoardView.hpp"
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

    return InputResult::None;
}

InputResult InputHandler::mouseClickEvent(const sf::Event::MouseButtonPressed& event, Game& game) {
        if(event.button == sf::Mouse::Button::Left) {
            return leftClickEvent(event, game);
        }

        if(event.button == sf::Mouse::Button::Right) {
            return rightClickEvent(event);
        }

        return InputResult::None;
}

InputResult InputHandler::mouseMovementEvent(const sf::Event::MouseMoved& event) {
    if(!heldPiece_) {
        return InputResult::None;
    };

    heldPiece_.value().mousePos = sf::Vector2f{static_cast<float>(event.position.x), static_cast<float>(event.position.y)};

    return InputResult::None;
}

InputResult InputHandler::mouseUnclickEvent(const sf::Event::MouseButtonReleased& event, Game& game) {    
    // only allow left click releases on the physical board
    if(event.position.x > Constants::BOARD_WIDTH_PX || event.position.y > Constants::BOARD_HEIGHT_PX) {
        // release piece if we click oob
        heldPiece_.reset();
        return InputResult::None;
    }

    const int targetSquare = BoardView::getSquareIndexFromCoordinates(event.position.x, event.position.y);
    
    // out of bounds
    if(!Utils::onBoard(targetSquare)) {
        return InputResult::None;
    }

    // remaining unclick operations require a held piece; exit early if no held piece exists
    if(!heldPiece_) {
        return InputResult::None;
    }

    const int sourceSquare = heldPiece_.value().heldSquare;

    // same square means we should try click-click move instead of dragging move
    // therefore, we do not reset heldSquare
    if(sourceSquare == targetSquare) {
        heldPiece_->isDragging = false;
        return InputResult::None;
    }

    // move is on board and different square
    const Move potentialMove = Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
    // if move is legal, try it
    if (game.tryMove(potentialMove)) {
        board_->updateBoardFromGame(game);
        heldPiece_.reset();
        board_->clearAllHighlightsExcept(BoardView::RIGHT_CLICK_HIGHLIGHT);
        return InputResult::MoveMade;
    }

    heldPiece_.reset();
    board_->clearAllHighlightsExcept(BoardView::RIGHT_CLICK_HIGHLIGHT);
    return InputResult::None;
}

InputResult InputHandler::leftClickEvent(const sf::Event::MouseButtonPressed& event, Game& game) {
    // first clear all highlights
    board_->clearAllHighlights();

    const sf::Vector2i mousePos = event.position;

    // only allow left clicks on the physical board
    if(mousePos.x > Constants::BOARD_WIDTH_PX || mousePos.y > Constants::BOARD_HEIGHT_PX) {
        return InputResult::None;
    }

    const int targetSquare = BoardView::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
    
    // target square does not exist; reset any selected piece
    if(!Utils::onBoard(targetSquare)) {
        heldPiece_.reset();
        return InputResult::None;
    }

    // target square does exist, but no currently held piece
    if(!heldPiece_) {
        // no need to do additional processing for clicking on empty square, or wrong player's piece
        if(!game.mailbox().at(targetSquare).exists() || game.mailbox().at(targetSquare).color() != game.sideToMove()) {
            return InputResult::None;
        }

        // hold square
        heldPiece_ = HeldPieceState{targetSquare, {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)}, true};

        // highlight selected square if there's a piece there
        board_->at(targetSquare).setHighlight(BoardView::SELECTED_HIGHLIGHT);

        // highlight legal moves
        MoveList legalMoves;
        game.generateLegalMovesFromSquare(targetSquare, legalMoves);
        for(int i = 0; i < legalMoves.size; i++) {
            const Move move = legalMoves.data[i];
            board_->at(move.targetSquare()).setHighlight(BoardView::LEGAL_HIGHLIGHT);
        }

        return InputResult::None;
    }

    // currently held piece exists; click-click move
    const int sourceSquare = heldPiece_.value().heldSquare;
    // if same square, we remove highlight and cancel move
    if(sourceSquare == targetSquare) {
        heldPiece_.reset();
        board_->clearAllHighlights(BoardView::SELECTED_HIGHLIGHT);
        return InputResult::None;
    }
    
    // Try to make click-click move; if successful, update visual board
    const Move potentialMove = Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
    if(game.tryMove(potentialMove)) {
        board_->updateBoardFromGame(game);
        heldPiece_.reset();
        board_->clearAllHighlightsExcept(BoardView::RIGHT_CLICK_HIGHLIGHT);
        return InputResult::None;
    }
    
    heldPiece_.reset();
    board_->clearAllHighlightsExcept(BoardView::RIGHT_CLICK_HIGHLIGHT);
    return InputResult::MoveMade;
}

InputResult InputHandler::rightClickEvent(const sf::Event::MouseButtonPressed& event) {
    // clear all left click highlights
    board_->clearAllHighlights(BoardView::LEGAL_HIGHLIGHT);
    board_->clearAllHighlights(BoardView::SELECTED_HIGHLIGHT);

    // right click cancels any held square
    heldPiece_.reset();

    const sf::Vector2i mousePos = event.position;

    // only allow right clicks on the physical board
    if(mousePos.x > Constants::BOARD_WIDTH_PX || mousePos.y > Constants::BOARD_HEIGHT_PX) {
        return InputResult::None;
    }

    const int targetSquare = BoardView::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
    
    // swap highlight status of square
    if(Utils::onBoard(targetSquare)) {
        board_->at(targetSquare).toggleHighlight(BoardView::RIGHT_CLICK_HIGHLIGHT);
    }

    return InputResult::None;
}

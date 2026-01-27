#include <optional>
#include <string>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "game/Game.hpp"
#include "gui/Board.hpp"

// TODO: implement CLI class and move Game.to_string()
// void runCLIGame() {
//     // init game
//     Game game;
//     game.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

//     while(!game.isFinished()) {
//         // Print info to player
//         std::cout << game.to_string() << "\n" << (game.currentTurn() == Color::White ? "White" : "Black") << "'s turn\nMove: ";
//         // read move from input
//         std::string moveS;
//         std::getline(std::cin, moveS);

//         std::optional<Move> possibleMove = game.parseMove(moveS);
//         if(!possibleMove) {
//             // invalid move
//             std::cout << "Invalid move. Try again.\n";
//             continue;
//         }
//         const Move move = possibleMove.value();
//         std::cout << "Parsed move: " << move.to_string(game) << "\n";

//         if(!game.tryMove(move)) {
//             std::cout << "Move is not legal. Try again.\n";
//             continue;
//         }
//         std::cout << "\n";
//     }
// }

void runGUIBitboardTest() {
    // helper function for this bitboard test; using the arbitrary bitboard button number, get a piece that matches so we can further extract information from Game
    const auto bitboardNumberToPiece = [] (int n) -> Piece {
        switch(n) {
            case 0: return Piece{PieceType::Pawn, Color::White};
            case 1: return Piece{PieceType::Knight, Color::White};
            case 2: return Piece{PieceType::Bishop, Color::White};
            case 3: return Piece{PieceType::Rook, Color::White};
            case 4: return Piece{PieceType::Queen, Color::White};
            case 5: return Piece{PieceType::King, Color::White};
            case 6: return Piece{PieceType::Pawn, Color::Black};
            case 7: return Piece{PieceType::Knight, Color::Black};;
            case 8: return Piece{PieceType::Bishop, Color::Black};;
            case 9: return Piece{PieceType::Rook, Color::Black};;
            case 10: return Piece{PieceType::Queen, Color::Black};;
            case 11: return Piece{PieceType::King, Color::Black};;
            default: return Piece{}; // shouldn't happen
        }
    };

    constexpr std::array<std::string_view, 12> bitboardButtonText{
        "White Pawn", "White Knight", "White Bishop", "White Rook", "White Queen", "White King",
        "Black Pawn", "Black Knight", "Black Bishop", "Black Rook", "Black Queen", "Black King"
    };

    constexpr int STARTING_WINDOW_WIDTH = 1000;
    constexpr int STARTING_WINDOW_HEIGHT = 1000;
    const std::string WINDOW_TITLE = "Chess";

    // init game
    Game game;
    game.loadFEN(std::string{Game::STARTING_FEN});

    // init board
    Board board;
    board.updateBoardFromGame(game);

    // init window
    sf::RenderWindow window{sf::VideoMode{{STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT}}, WINDOW_TITLE};
    // enable vsync
    window.setVerticalSyncEnabled(true);

    // init sounds
    // TODO: potentially throw / recover from file missing
    const sf::SoundBuffer PIECE_MOVEMENT_BUFFER{"assets/sounds/piece_movement.wav"};
    sf::Sound PIECE_MOVEMENT_SOUND{PIECE_MOVEMENT_BUFFER};
    PIECE_MOVEMENT_SOUND.setVolume(75.F);

    // init font
    const sf::Font font{"assets/fonts/LiberationSans-Regular.ttf"};

    // init current held square for making moves
    std::optional<int> heldSquare;
    // init helpers for dragging logic
    bool isDragging = false;
    sf::Vector2f dragPosPx{0.F, 0.F};

    // init current bitboard button state for drawing helper logic
    std::optional<int> chosenBitboard;

    // main window loop
    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            // close the window
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // check mouse clicks
            if (const auto* mouseObject = event->getIf<sf::Event::MouseButtonPressed>()) {
                // LEFT CLICK
                if(mouseObject->button == sf::Mouse::Button::Left) {
                    // first clear all highlights
                    board.clearAllHighlights();

                    const sf::Vector2i mousePos = mouseObject->position;

                    // check if we hit a button before doing other bounds checking
                    // x range -> 820 - 980
                    if(mousePos.x >= 820 && mousePos.x <= 980) {
                        // y range 20 -> 70, 100 -> 170, ..., 900 -> 950
                        for (int i = 0; i < 12; i++) {
                            const int top = (80 * i) + 20;
                            const int bottom = top + 50;

                            if (mousePos.y >= top && mousePos.y <= bottom) {
                                // we've hit button i

                                if(chosenBitboard.value_or(-1) == i) {
                                    // clicked twice, we can unset it
                                    chosenBitboard.reset();
                                } else{
                                    chosenBitboard = i;
                                }
                            }
                        }
                    }

                    // only allow other left clicks on the physical board
                    if(mousePos.x > 800 || mousePos.y > 800) {
                        continue;
                    }

                    int targetSquare = Board::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
                    
                    // square does not exist; reset any selected piece and continue
                    if(!Game::onBoard(targetSquare)) {
                        heldSquare.reset();
                        continue;
                    }

                    // square does exist
                    // no currently held piece
                    if(!heldSquare) {
                        // no need to do additional processing for clicking on empty square, or wrong player's piece
                        if(!game.mailbox().at(targetSquare).exists() || game.mailbox().at(targetSquare).color() != game.currentTurn()) {
                            continue;
                        }

                        // hold square
                        heldSquare = targetSquare;
                        isDragging = true;

                        // highlight selected square if there's a piece there
                        board.at(targetSquare).setHighlight(Board::SELECTED_HIGHLIGHT);

                        // highlight legal moves
                        MoveList legalMoves;
                        game.generateLegalMovesFromSquare(targetSquare, legalMoves);
                        for(int i = 0; i < legalMoves.size; i++) {
                            const Move move = legalMoves.data[i];
                            board.at(move.targetSquare()).setHighlight(Board::LEGAL_HIGHLIGHT);
                        }

                        dragPosPx = {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)};

                        continue;
                    }

                    // currently held piece exists; click-click move
                    const int sourceSquare = heldSquare.value();
                    // if same square, we remove highlight and cancel move
                    if(sourceSquare == targetSquare) {
                        heldSquare.reset();
                        board.clearAllHighlights(Board::SELECTED_HIGHLIGHT);
                        continue;
                    }
                    
                    // Try to make click-click move; if successful, update visual board
                    const Move potentialMove = Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
                    if(game.tryMove(potentialMove)) {
                        board.updateBoardFromGame(game);
                        PIECE_MOVEMENT_SOUND.play();
                    }

                    heldSquare.reset();
                    board.clearAllHighlightsExcept(Board::RIGHT_CLICK_HIGHLIGHT);
                }
                
                // RIGHT CLICK
                if(mouseObject->button == sf::Mouse::Button::Right) {
                    // clear all left click highlights
                    board.clearAllHighlights(Board::LEGAL_HIGHLIGHT);
                    board.clearAllHighlights(Board::SELECTED_HIGHLIGHT);

                    // right click cancels any held square
                    heldSquare.reset();

                    const sf::Vector2i mousePos = mouseObject->position;

                    // only allow right clicks on the physical board
                    if(mousePos.x > 800 || mousePos.y > 800) {
                        continue;
                    }

                    const int targetSquare = Board::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
                    
                    // swap highlight status of square
                    if(Game::onBoard(targetSquare)) {
                        board.at(targetSquare).toggleHighlight(Board::RIGHT_CLICK_HIGHLIGHT);
                    }
                }
            }

            // check mouse movement
            if (const auto* mouseObject = event->getIf<sf::Event::MouseMoved>()) {
                dragPosPx = sf::Vector2f{static_cast<float>(mouseObject->position.x), static_cast<float>(mouseObject->position.y)};
            }

            // check mouse unclicks
            if (const auto* mouseObject = event->getIf<sf::Event::MouseButtonReleased>()) {
                // try to make dragged move
                if (mouseObject->button == sf::Mouse::Button::Left && heldSquare) {
                    const int sourceSquare = heldSquare.value();

                    // only allow left click releases on the physical board
                    if(mouseObject->position.x > 800 || mouseObject->position.y > 800) {
                        // release piece if we click oob
                        heldSquare.reset();
                        continue;
                    }

                    const int targetSquare = Board::getSquareIndexFromCoordinates(mouseObject->position.x, mouseObject->position.y);
                    
                    // out of bounds
                    if(!Game::onBoard(targetSquare)) {
                        continue;
                    }

                    // same square means we should try click-click move instead of dragging move
                    // therefore, we do not reset heldSquare
                    if(sourceSquare == targetSquare) {
                        isDragging = false;
                        continue;
                    }

                    // move is on board and different square
                    const Move potentialMove = Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
                    // if move is legal, try it
                    if (game.tryMove(potentialMove)) {
                        board.updateBoardFromGame(game);
                        PIECE_MOVEMENT_SOUND.play();
                    }

                    heldSquare.reset();
                    // only dont undo right click highlights
                    board.clearAllHighlightsExcept(Board::RIGHT_CLICK_HIGHLIGHT);
                }
            }
        }
        
        // clear the window
        window.clear(sf::Color::Black);

        // TODO: replace check highlight with sprite
        if(game.isInCheck(game.currentTurn())) {
            // add check highlight after main loop to override other highlights
            board.at(game.findKingSquare(game.currentTurn())).setHighlight(Board::CHECK_HIGHLIGHT);
        }

        // highlight squares based on the chosen bitboard
        board.clearAllHighlights(Board::RIGHT_CLICK_HIGHLIGHT);
        board.clearAllHighlights(Board::CYAN_HIGHLIGHT);
        if(chosenBitboard.has_value()) {
            const Piece bitboardPiece = bitboardNumberToPiece(chosenBitboard.value());
            const Bitboard bitboard = game.pieceToBitboard(bitboardPiece);
            // const std::array<Bitboard, 64> attackBitboards = game.pieceToAttackBitboard(bitboardPiece);

            // highlight allPieces_ based on color of chosenBitboard
            Bitboard allPieces = game.colorToOccupancyBitboard(bitboardPiece.color());
            // paint board based on allPieces
            while(!allPieces.empty()) {
                const int pieceLocation = allPieces.popLsb();
                board.at(pieceLocation).setHighlight(Board::CYAN_HIGHLIGHT);
            }

            // highlight attack bitboards by building all attacks from bitboard
            // Bitboard allAttacks{0};
            // Bitboard temp = bitboard;
            // while(!temp.empty()) {
            //     const int pieceLocation = temp.popLsb();
            //     allAttacks.mergeIn(attackBitboards[pieceLocation]);
            // }

            // // then, paint board based on allAttacks
            // while(!allAttacks.empty()) {
            //     const int attackLocation = allAttacks.popLsb();
            //     board.at(attackLocation).setHighlight(Board::CYAN_HIGHLIGHT);
            // }

            for(int square = 0; square < Game::NUM_SQUARES; square++) {
                // highlight chosen bitboard after so red is on top of cyan
                if(bitboard.containsSquare(square)) {
                    board.at(square).setHighlight(Board::RIGHT_CLICK_HIGHLIGHT);
                }
            }
        }

        // draw board without heldSqaure iff we are dragging
        board.draw(window, isDragging ? heldSquare : std::nullopt);

        // if heldSquare and we are dragging, we copy sprite to mouse
        if(heldSquare && isDragging) {
            if (const sf::Sprite* sprite = board.at(*heldSquare).pieceSprite().sprite()) {
                sf::Sprite dragSprite = *sprite;
                dragSprite.setPosition(dragPosPx);
                window.draw(dragSprite);
            }
        }

        // draw the bitboard test buttons
        for(int i = 0; i < 12; i++) {
            const sf::Vector2f bitboardButtonSize{160.F, 50.F};
            sf::RectangleShape bitboardButton{bitboardButtonSize};
            if(chosenBitboard == i) {
                // red, rgb(230, 87, 87)
                bitboardButton.setFillColor(sf::Color{230, 87, 87});
            } else {
                // white, rgb(255, 255, 255)
                bitboardButton.setFillColor(sf::Color{255, 255, 255});
            }
            bitboardButton.setOutlineThickness(2.F);
            // black, rgb(0, 0, 0)
            bitboardButton.setOutlineColor(sf::Color{0, 0, 0});
            // space out from y=20 to y=900
            const float bitboardXPos = 820;
            const float bitboardYPos = (80 * i) + 20;
            bitboardButton.setPosition({bitboardXPos, bitboardYPos});
            window.draw(bitboardButton);

            // draw text
            sf::Text text{font};
            text.setString(std::string{bitboardButtonText[i]});
            text.setCharacterSize(22);
            // black, rgb(0, 0, 0)
            text.setFillColor(sf::Color{0, 0, 0});

            text.setPosition({bitboardXPos + 20, bitboardYPos + 10});

            window.draw(text);
        }

        // end the current frame
        window.display();
    }
}

void runGUIgame() {
    constexpr int STARTING_WINDOW_WIDTH = 1000;
    constexpr int STARTING_WINDOW_HEIGHT = 1000;
    const std::string WINDOW_TITLE = "Chess";

    // init game
    Game game;
    game.loadFEN(std::string{Game::STARTING_FEN});

    // init board
    Board board;
    board.updateBoardFromGame(game);

    // init window
    sf::RenderWindow window{sf::VideoMode{{STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT}}, WINDOW_TITLE};
    // enable vsync
    window.setVerticalSyncEnabled(true);

    // init sounds
    // TODO: potentially throw / recover from file missing
    const sf::SoundBuffer PIECE_MOVEMENT_BUFFER{"assets/sounds/piece_movement.wav"};
    sf::Sound PIECE_MOVEMENT_SOUND{PIECE_MOVEMENT_BUFFER};
    PIECE_MOVEMENT_SOUND.setVolume(75.F);

    // init current held square for making moves
    std::optional<int> heldSquare;
    // init helpers for dragging logic
    bool isDragging = false;
    sf::Vector2f dragPosPx{0.F, 0.F};

    // main window loop
    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            // close the window
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // check mouse clicks
            if (const auto* mouseObject = event->getIf<sf::Event::MouseButtonPressed>()) {
                // LEFT CLICK
                if(mouseObject->button == sf::Mouse::Button::Left) {
                    // first clear all highlights
                    board.clearAllHighlights();

                    const sf::Vector2i mousePos = mouseObject->position;

                    // only allow left clicks on the physical board
                    if(mousePos.x > 800 || mousePos.y > 800) {
                        continue;
                    }

                    int targetSquare = Board::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
                    
                    // square does not exist; reset any selected piece and continue
                    if(!Game::onBoard(targetSquare)) {
                        heldSquare.reset();
                        continue;
                    }

                    // square does exist
                    // no currently held piece
                    if(!heldSquare) {
                        // no need to do additional processing for clicking on empty square, or wrong player's piece
                        if(!game.mailbox().at(targetSquare).exists() || game.mailbox().at(targetSquare).color() != game.currentTurn()) {
                            continue;
                        }

                        // hold square
                        heldSquare = targetSquare;
                        isDragging = true;

                        // highlight selected square if there's a piece there
                        board.at(targetSquare).setHighlight(Board::SELECTED_HIGHLIGHT);

                        // highlight legal moves
                        MoveList legalMoves;
                        game.generateLegalMovesFromSquare(targetSquare, legalMoves);
                        for(int i = 0; i < legalMoves.size; i++) {
                            const Move move = legalMoves.data[i];
                            board.at(move.targetSquare()).setHighlight(Board::LEGAL_HIGHLIGHT);
                        }

                        dragPosPx = {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)};

                        continue;
                    }

                    // currently held piece exists; click-click move
                    const int sourceSquare = heldSquare.value();
                    // if same square, we remove highlight and cancel move
                    if(sourceSquare == targetSquare) {
                        heldSquare.reset();
                        board.clearAllHighlights(Board::SELECTED_HIGHLIGHT);
                        continue;
                    }
                    
                    // Try to make click-click move; if successful, update visual board
                    const Move potentialMove = Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
                    if(game.tryMove(potentialMove)) {
                        board.updateBoardFromGame(game);
                        PIECE_MOVEMENT_SOUND.play();
                    }
                    
                    heldSquare.reset();
                    board.clearAllHighlightsExcept(Board::RIGHT_CLICK_HIGHLIGHT);
                }
                
                // RIGHT CLICK
                if(mouseObject->button == sf::Mouse::Button::Right) {
                    // clear all left click highlights
                    board.clearAllHighlights(Board::LEGAL_HIGHLIGHT);
                    board.clearAllHighlights(Board::SELECTED_HIGHLIGHT);

                    // right click cancels any held square
                    heldSquare.reset();

                    const sf::Vector2i mousePos = mouseObject->position;

                    // only allow right clicks on the physical board
                    if(mousePos.x > 800 || mousePos.y > 800) {
                        continue;
                    }

                    const int targetSquare = Board::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
                    
                    // swap highlight status of square
                    if(Game::onBoard(targetSquare)) {
                        board.at(targetSquare).toggleHighlight(Board::RIGHT_CLICK_HIGHLIGHT);
                    }
                }
            }

            // check mouse movement
            if (const auto* mouseObject = event->getIf<sf::Event::MouseMoved>()) {
                dragPosPx = sf::Vector2f{static_cast<float>(mouseObject->position.x), static_cast<float>(mouseObject->position.y)};
            }

            // check mouse unclicks
            if (const auto* mouseObject = event->getIf<sf::Event::MouseButtonReleased>()) {
                // try to make dragged move
                if (mouseObject->button == sf::Mouse::Button::Left && heldSquare) {
                    const int sourceSquare = heldSquare.value();
                    
                    // only allow left click releases on the physical board
                    if(mouseObject->position.x > 800 || mouseObject->position.y > 800) {
                        // release piece if we click oob
                        heldSquare.reset();
                        continue;
                    }

                    const int targetSquare = Board::getSquareIndexFromCoordinates(mouseObject->position.x, mouseObject->position.y);
                    
                    // out of bounds
                    if(!Game::onBoard(targetSquare)) {
                        continue;
                    }

                    // same square means we should try click-click move instead of dragging move
                    // therefore, we do not reset heldSquare
                    if(sourceSquare == targetSquare) {
                        isDragging = false;
                        continue;
                    }

                    // move is on board and different square
                    const Move potentialMove = Move::fromPieces(sourceSquare, targetSquare, game.mailbox().at(sourceSquare), game.mailbox().at(targetSquare));
                    // if move is legal, try it
                    if (game.tryMove(potentialMove)) {
                        board.updateBoardFromGame(game);
                        PIECE_MOVEMENT_SOUND.play();
                    }

                    heldSquare.reset();
                    board.clearAllHighlightsExcept(Board::RIGHT_CLICK_HIGHLIGHT);
                }
            }
        }

        // highlight attacked squares
        // for(int i = 0; i < Game::NUM_SQUARES; i++) {
        //     if(game.isSquareAttacked(i, Color::White)) {
        //         board.at(i).setHighlight(Board::CYAN_HIGHLIGHT);
        //     }
        //     if(game.isSquareAttacked(i, Color::Black)) {
        //         board.at(i).setHighlight(Board::CYAN_HIGHLIGHT);
        //     }
        // }

        // TODO: replace check highlight with sprite
        if(game.isInCheck(game.currentTurn())) {
            // add check highlight after main loop to override other highlights
            board.at(game.findKingSquare(game.currentTurn())).setHighlight(Board::CHECK_HIGHLIGHT);
        }

        // clear the window
        window.clear(sf::Color::Black);

        // draw board without heldSqaure iff we are dragging
        board.draw(window, isDragging ? heldSquare : std::nullopt);

        // if heldSquare and we are dragging, we copy sprite to mouse
        if(heldSquare && isDragging) {
            if (const sf::Sprite* sprite = board.at(*heldSquare).pieceSprite().sprite()) {
                sf::Sprite dragSprite = *sprite;
                dragSprite.setPosition(dragPosPx);
                window.draw(dragSprite);
            }
        }

        // end the current frame
        window.display();
    }
}

int main() {
    runGUIBitboardTest();
}

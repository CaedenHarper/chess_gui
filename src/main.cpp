#include <iostream>
#include <optional>
#include <string>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "game/Game.hpp"
#include "gui/Board.hpp"

void runCLIGame() {
    // init game
    Game game;
    game.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    while(!game.isFinished()) {
        // Print info to player
        std::cout << game.to_string() << "\n" << (game.currentTurn() == Color::White ? "White" : "Black") << "'s turn\nMove: ";
        // read move from input
        std::string moveS;
        std::getline(std::cin, moveS);

        std::optional<Move> possibleMove = game.parseMove(moveS);
        if(!possibleMove) {
            // invalid move
            std::cout << "Invalid move. Try again.\n";
            continue;
        }
        const Move move = possibleMove.value();
        std::cout << "Parsed move: " << move.to_string() << "\n";

        if(!game.tryMove(move)) {
            std::cout << "Move is not legal. Try again.\n";
            continue;
        }
        std::cout << "\n";
    }
}

void runGUIGame() {
    constexpr int STARTING_WINDOW_WIDTH = 800;
    constexpr int STARTING_WINDOW_HEIGHT = 800;
    const std::string WINDOW_TITLE = "Chess";

    // init game
    Game game;
    game.loadFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");

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
                        if(!game.board().at(targetSquare).exists() || game.board().at(targetSquare).color() != game.currentTurn()) {
                            continue;
                        }

                        // hold square
                        heldSquare = targetSquare;
                        isDragging = true;

                        // highlight selected square if there's a piece there
                        board.at(targetSquare).setHighlight(Board::SELECTED_HIGHLIGHT);

                        // highlight legal moves
                        const std::vector<Move> legalMoves = game.generateLegalMoves(targetSquare);
                        for(const Move move : legalMoves) {
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
                    const Move potentialMove{sourceSquare, targetSquare, game.board().at(sourceSquare), game.board().at(targetSquare)};
                    if(game.tryMove(potentialMove)) {
                        board.updateBoardFromGame(game);
                        PIECE_MOVEMENT_SOUND.play();
                    }

                    heldSquare.reset();
                    board.clearAllHighlights(Board::SELECTED_HIGHLIGHT);
                    board.clearAllHighlights(Board::LEGAL_HIGHLIGHT);
                }
                
                // RIGHT CLICK
                if(mouseObject->button == sf::Mouse::Button::Right) {
                    // clear all left click highlights
                    board.clearAllHighlights(Board::LEGAL_HIGHLIGHT);
                    board.clearAllHighlights(Board::SELECTED_HIGHLIGHT);

                    // right click cancels any held square
                    heldSquare.reset();

                    const sf::Vector2i mousePos = mouseObject->position;
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
                    const Move potentialMove{sourceSquare, targetSquare, game.board().at(sourceSquare), game.board().at(targetSquare)};
                    // if move is legal, try it
                    if (game.tryMove(potentialMove)) {
                        board.updateBoardFromGame(game);
                        PIECE_MOVEMENT_SOUND.play();
                        // successfully making a move resets any legal move highlights
                        board.clearAllHighlights(Board::LEGAL_HIGHLIGHT);
                    }

                    heldSquare.reset();
                    board.clearAllHighlights(Board::SELECTED_HIGHLIGHT);
                    board.clearAllHighlights(Board::LEGAL_HIGHLIGHT);
                }
            }
        }

        // highlight attacked squares
        for(int i = 0; i < Game::NUM_SQUARES; i++) {
            // if(game.isSquareAttacked(i, Color::White)) board.at(i).setHighlight(Board::RIGHT_CLICK_HIGHLIGHT);
            // if(game.isSquareAttacked(i, Color::Black)) board.at(i).setHighlight(Board::RIGHT_CLICK_HIGHLIGHT);
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
    runGUIGame();
}

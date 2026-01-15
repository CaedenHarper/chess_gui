#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

#include "game/Game.hpp"
#include "gui/Board.hpp"

const int STARTING_WINDOW_WIDTH = 800;
const int STARTING_WINDOW_HEIGHT = 800;
const char* WINDOW_TITLE = "Chess";

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
        if(!possibleMove.has_value()) {
            // invalid move
            std::cout << "Invalid move. Try again.\n";
            continue;
        }
        Move move = possibleMove.value();
        std::cout << "Parsed move: " << move.to_string() << "\n";

        if(!game.tryMove(move)) {
            std::cout << "Move is not legal. Try again.\n";
            continue;
        }
        std::cout << "\n";
    }
}

void runGUIGame() {
    // init game
    Game game;
    game.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // init board
    Board board;
    board.updateBoardFromGame(game);

    // init window
    sf::RenderWindow window{sf::VideoMode{{STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT}}, WINDOW_TITLE};
    // enable vsync
    window.setVerticalSyncEnabled(true);

    // main window loop
    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            // close the window
            if (event->is<sf::Event::Closed>()) window.close();
            // check mouse clicks
            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                // LEFT CLICK
                if(mouseButtonPressed->button == sf::Mouse::Button::Left) {
                    // first reset all right click highlights
                    board.resetAllHighlights();

                    sf::Vector2i mousePos = mouseButtonPressed->position;
                    int selectedSquare = Board::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
                    
                    // square exists
                    if(Game::onBoard(selectedSquare)) {
                        Square square = board.at(selectedSquare);
                        std::vector<Move> legalMoves = game.generatePseudoLegalMoves(selectedSquare);
                        for(Move move : legalMoves) board.at(move.targetSquare()).setHighlight(Board::LEGAL_HIGHLIGHT);
                    }
                }
                
                // RIGHT CLICK
                if(mouseButtonPressed->button == sf::Mouse::Button::Right) {
                    sf::Vector2i mousePos = mouseButtonPressed->position;
                    int selectedSquare = Board::getSquareIndexFromCoordinates(mousePos.x, mousePos.y);
                    
                    // swap highlight status of square
                    if(Game::onBoard(selectedSquare)) board.at(selectedSquare).toggleHighlight(Board::RIGHT_CLICK_HIGHLIGHT);
                }
            }
            // check mouse unclicks
            // if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonReleased>()) {
            // }
        }

        // clear the window
        window.clear(sf::Color::Black);

        board.draw(window);

        // end the current frame
        window.display();
    }
}

int main() {
    runGUIGame();
}

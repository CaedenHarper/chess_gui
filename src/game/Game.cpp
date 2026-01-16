#include "Game.hpp"
#include <string>
#include <iostream>
#include <optional>
#include <vector>

/*
    Constructor with no arguments returns an empty square.
*/
Piece::Piece() : type_{PieceType::None}, color_{Color::None} {
}

/*
    Construct a Piece.
*/
Piece::Piece(PieceType type, Color color) : type_{type}, color_{color} {
}

PieceType Piece::type() const {
    return type_;
}

Color Piece::color() const {
    return color_;
}

bool Piece::exists() const {
    return type_ != PieceType::None;
}

PieceType Piece::charToPieceType(char c) {
    if(c == 'P' || c == 'p') {
        return PieceType::Pawn;
    } else if(c == 'N' || c == 'n') {
        return PieceType::Knight;
    } else if(c == 'B' || c == 'b') {
        return PieceType::Bishop;
    } else if(c == 'R' || c == 'r') {
        return PieceType::Rook;
    } else if(c == 'Q' || c == 'q') {
        return PieceType::Queen;
    } else if(c == 'K' || c == 'k') {
        return PieceType::King;
    } else {
        // TODO: debug msg here?
        return PieceType::None;
    }
}

Piece Piece::charToPiece(char c) {
    switch (c) {
        case 'P': return Piece{PieceType::Pawn, Color::White};
        case 'p': return Piece{PieceType::Pawn, Color::Black};
        case 'N': return Piece{PieceType::Knight, Color::White};
        case 'n': return Piece{PieceType::Knight, Color::Black};
        case 'B': return Piece{PieceType::Bishop, Color::White};
        case 'b': return Piece{PieceType::Bishop, Color::Black};
        case 'R': return Piece{PieceType::Rook, Color::White};
        case 'r': return Piece{PieceType::Rook, Color::Black};
        case 'Q': return Piece{PieceType::Queen, Color::White};
        case 'q': return Piece{PieceType::Queen, Color::Black};
        case 'K': return Piece{PieceType::King, Color::White};
        case 'k': return Piece{PieceType::King, Color::Black};
        default: return Piece{PieceType::None, Color::White};
    }
}

std::string Piece::to_string_short() const {
    bool isWhite = color_ == Color::White;

    switch (type_) {
        case PieceType::None: return "?";
        case PieceType::Pawn: return isWhite ? "P": "p";
        case PieceType::Knight: return isWhite ? "N": "n";
        case PieceType::Bishop: return isWhite ? "B": "b";
        case PieceType::Rook: return isWhite ? "R": "r";
        case PieceType::Queen: return isWhite ? "Q": "q";
        case PieceType::King: return isWhite ? "K" : "k";
    }
}

std::string Piece::to_string_long() const {
    std::string color = color_ == Color::White ? "White " : "Black ";

    switch (type_) {
        case PieceType::None: return "Empty Square";
        case PieceType::Pawn: return color + "Pawn";
        case PieceType::Knight: return color + "Knight";
        case PieceType::Bishop: return color + "Bishop";
        case PieceType::Rook: return color + "Rook";
        case PieceType::Queen: return color + "Queen";
        case PieceType::King: return color + "King";
    }
}

Move::Move(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece) 
 : sourceSquare_{sourceSquare}, targetSquare_{targetSquare}, sourcePiece_{sourcePiece}, targetPiece_{targetPiece} {
}

int Move::sourceSquare() const {
    return sourceSquare_;
}

int Move::targetSquare() const {
    return targetSquare_;
}

Piece Move::sourcePiece() const {
    return sourcePiece_;
}

Piece Move::targetPiece() const {
    return targetPiece_;
}

std::string Move::to_string() const {
    return ( 
        sourcePiece_.to_string_long() + " on " + Game::intToAlgebraicNotation(sourceSquare_) +
        " (" + std::to_string(sourceSquare_) + ") to " + targetPiece_.to_string_long() + " on " + Game::intToAlgebraicNotation(targetSquare_) +
        " (" + std::to_string(targetSquare_) +")"
    );
}

Game::Game() : currentTurn_{Color::White} {
}

Color Game::currentTurn() const {
    return currentTurn_;
}

std::array<Piece, 64> Game::board() const {
    return board_;
}

/*
Print in format:
  +---------------+
8 |r|n|b|k|q|b|n|r|
7 |p|p|p|p|p|p|p|p|
6 | |#| |#| |#| |#|
5 |#| |#| |#| |#| |
4 | |#| |#| |#| |#|
3 |#| |#| |#| |#| |
2 |P|P|P|P|P|P|P|P|
1 |R|N|B|K|Q|B|N|R|
  +---------------+
   a b c d e f g h
*/
std::string Game::to_string() const {
    // starting border
    std::string o = "  +---------------+\n";

    int i = 0;
    for (const auto& piece : board_) {
        // print line start if at start of row
        if(i%8 == 0) {
            o += std::to_string(8-(i/8)) + " |";
        }
    
        if(piece.exists()) {
            o += piece.to_string_short() + "|";
        } else {
            // if piece is none, print square color
            // square color is white if the square polarity matches the row polarity
            o += i%2 == (i/8)%2 ? " |" : "#|";
        }

        // print line ending if at end of row
        if(i%8 == 7) {
            o += "\n";
        }

        i++;
    }

    // print ending border
    return o + "  +---------------+\n   a b c d e f g h";
}

/*
    Load FEN into game board.
*/
void Game::loadFEN(const std::string FEN) {
    /*
    Fen has 6 fields:
    0 - piece placement
    1 - current color
    2 - castling
    3 - en passent
    4 - halfmove clock
    5 - current move number
    */
    int field = 0;
    // used for piece placement field to find out where we are putting each piece
    int piecePlacementIndex = 0;
    for(char c : FEN) {
        // space indicates we are ready for the next field
        if(c == ' ') {
            field++;
            continue;
        }

        // piece placement
        if(field == 0) {
            if(c >= '1' && c <= '8') {
                // Numbers indicate n empty squares
                int numberEmptySquares = c-'0';
                for(int i = 0; i < numberEmptySquares; i++) {
                    Piece emptySquare = Piece{};
                    board_.at(piecePlacementIndex) = emptySquare;
                    piecePlacementIndex++;
                }
                continue;
            }
            
            if(c == '/') {
                // '/' represents a new rank and can be ignored
                continue;
            }
            
            // We either have a proper piece or an invalid FEN
            Piece newPiece = Piece::charToPiece(c);
            if(!newPiece.exists()) {
                // TODO: parsing error
                std::cerr << "loadFEN parsing error!\n";
                continue;
            }

            // we have a valid piece, add it and update index
            board_.at(piecePlacementIndex) = newPiece;
            piecePlacementIndex++;
        }

        // current color
        if(field == 1) {
            // this field only has one character, either w or b
            if(c == 'w') {
                currentTurn_ = Color::White;
            } else if(c == 'b') {
                currentTurn_ = Color::Black;
            } else {
                // TODO: parsing error
                std::cerr << "loadFEN parsing error!\n";
            }
            continue;
        }

        // TODO: castling
        if(field == 2) {
            continue;
        }

        // TODO: en passent
        if(field == 3) {
            continue;
        }

        // TODO: halfmove clock
        if(field == 4) {
            continue;
        }

        // TODO: current move number
        if(field == 5) {
            continue;
        }
    }
}

bool Game::isFinished() const {
    return false;
}

std::optional<Move> Game::parseLongNotation_(const std::string sourceS, const std::string targetS) const {
    // TODO: implement pawn promotion eventually
    // We assume each string has at least two characters
    if(sourceS.length() < 2 || targetS.length() < 2) {
        // TODO: debug statement
        std::cerr << "parseLongNotation: sourceS or targetS not long enough!\n";
        return std::nullopt;
    }

    char sourceFileC = sourceS.at(0);
    char sourceRankC = sourceS.at(1);
    char targetFileC = targetS.at(0);
    char targetRankC = targetS.at(1);

    int sourceFile = sourceFileC - 'a';
    int sourceRank = sourceRankC - '0';
    int targetFile = targetFileC - 'a';
    int targetRank = targetRankC - '0';

    // all files and ranks should be in range 0 - 7
    if(!(sourceFile >= 0 && sourceFile <= 7) 
    || !(sourceRank >= 0 && sourceRank <= 7)
    || !(targetFile >= 0 && targetFile <= 7) 
    || !(targetRank >= 0 && targetRank <= 7)) {
        // TODO: debug statement
        std::cerr << "parseLongNotation: one file or rank is out of bounds\n";
        return std::nullopt;
    }

    // rank 1 starts at at index 0, whereas Game handles the board starting from the top left (rank 8 being 0)
    // therefore, we need to reflect it across the middle of the board with (8 - rank)
    int sourceSquare = 8 * (8 - sourceRank) + sourceFile; 
    int targetSquare = 8 * (8 - targetRank) + targetFile; 

    return Move{sourceSquare, targetSquare, board_.at(sourceSquare), board_.at(targetSquare)};
}

// (-Wunused-parameter)
std::optional<Move> Game::parseAlgebraicNotation_(const std::string s) const {   // NOLINT
    std::cerr << "parseAlgebraicNotation: Not yet implemented!";
    return std::nullopt;
}

/*
    We parse two types of input types: "sourceSquare targetSquare" (long notation) and algebraic notation.

    E.g., "e2 e4" vs. "e4" (from starting position).
*/
std::optional<Move> Game::parseMove(const std::string s) const {
    int currentPart = 0;
    std::string firstPart = "";
    std::string secondPart = "";
    // iterate through and split based on first part before space vs. second part. any unknown characters are ignored. any spaces after the first space are ignored.
    for(char c : s) {
        if(c == ' ' && currentPart == 0) {
            currentPart = 1;
            continue;
        }

        // ignore any more spaces
        if(c == ' ' && currentPart > 0) {
            continue;
        }

        if(currentPart == 0) {
            firstPart += c;
            continue;
        }

        secondPart += c;
    }

    if(secondPart.empty()) {
        return parseAlgebraicNotation_(firstPart);
    }
    return parseLongNotation_(firstPart, secondPart);
}

// TODO: en passent
std::vector<Move> Game::generatePseudoLegalPawnMoves(int sourceSquare) {
    /* 
        Four pawn capture types:
        1. One move forward (TODO: promotion)
        2. Two moves forward, has to start on 2nd rank
        3. Capture left, has to be opposite color piece (or en passent)
        4. Capture right, has to be opposite color piece (or en passent)
    */
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = sourceSquare / 8;
    const int col = sourceSquare % 8;

    const int dir = (sourceColor == Color::White) ? -1 : 1; // this function is from black's pov
    const int startRow = (sourceColor == Color::White) ? 6 : 1;  // rank 2 / rank 7

    // Case 1 and 2
    const int one = sourceSquare + dir * 8;
    if (onBoard(one) && !board_.at(one).exists()) {
        out.push_back(Move{sourceSquare, one, sourcePiece, board_.at(one)});

        // Case 2
        const int two = sourceSquare + dir * 16;
        if (row == startRow && onBoard(two) && !board_.at(two).exists()) {
            out.push_back(Move{sourceSquare, two, sourcePiece, board_.at(two)});
        }
    }

    // captures
    // left/right depends on color only through dir:
    // white captures: -7 (left), -9 (right)
    // black captures: +9 (left), +7 (right)
    const int capLeft  = sourceSquare + dir * 8 - 1;
    const int capRight = sourceSquare + dir * 8 + 1;

    // Case 3
    if (col > 0 && onBoard(capLeft)) {
        const Piece p = board_.at(capLeft);
        if (p.exists() && p.color() != sourceColor) out.push_back(Move{sourceSquare, capLeft, sourcePiece, p});
    }

    // Case 4
    if (col < 7 && onBoard(capRight)) {
        const Piece p = board_.at(capRight);
        if (p.exists() && p.color() != sourceColor) out.push_back(Move{sourceSquare, capRight, sourcePiece, p});
    }

    return out;
}


std::vector<Move> Game::generatePseudoLegalKnightMoves(int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = sourceSquare / 8;
    const int col = sourceSquare % 8;

    // 8 possible moves
    // comments are from whites POV, but they are symmetrical for black
    constexpr int knightDeltas[8][2] {
        // col (x), row (y)
        {-2, -1}, // left up
        {-1, -2}, // up left
        {1, -2}, // up right
        {2, -1}, // right up
        {2, 1}, // right down
        {1, 2}, // down right
        {-1, 2}, // down left
        {-2, 1}, // left down
    };

    for(int i = 0; i < 8; i++) {
        const int col2 = col + knightDeltas[i][0];
        const int row2 = row + knightDeltas[i][1];
        if(!onBoard(col2, row2)) continue;
        
        const int square2 = row2 * 8 + col2;
        const Piece piece2 = board_.at(square2);

        // Can move if target square is empty or has enemy
        if (!piece2.exists() || piece2.color() != sourceColor) {
            out.push_back(Move{sourceSquare, square2, sourcePiece, piece2});
        }
    }

    return out;
}

std::vector<Move> Game::generatePseudoLegalBishopMoves(int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = sourceSquare / 8;
    const int col = sourceSquare % 8;

    // four possible moves (which are repeated)
    constexpr int deltas[4][2] {
        // col (x), row (y)
        {-1, -1}, // up left
        {1, -1}, // up right
        {-1, 1}, // down left
        {1, 1}, // down right
    };

    for(int i = 0; i < 4; i++) {
        const int dcol = deltas[i][0];
        const int drow = deltas[i][1];

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dcol;
        int curRow = row + drow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = curRow * 8 + curCol;
            const Piece curPiece = board_.at(curSquare);

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.push_back(Move{sourceSquare, curSquare, sourcePiece, curPiece});
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.push_back(Move{sourceSquare, curSquare, sourcePiece, curPiece});
                }

                // since square is occupied we stop
                break;
            }

            // check next square
            curCol += dcol;
            curRow += drow;
        }
    }

    return out;
}

std::vector<Move> Game::generatePseudoLegalRookMoves(int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = sourceSquare / 8;
    const int col = sourceSquare % 8;

    // four possible moves (which are repeated)
    constexpr int deltas[4][2] {
        // col (x), row (y)
        {0, 1}, // up
        {0, -1}, // down
        {1, 0}, // right
        {-1, 0}, // left
    };

    for(int i = 0; i < 4; i++) {
        const int dcol = deltas[i][0];
        const int drow = deltas[i][1];

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dcol;
        int curRow = row + drow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = curRow * 8 + curCol;
            const Piece curPiece = board_.at(curSquare);

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.push_back(Move{sourceSquare, curSquare, sourcePiece, curPiece});
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.push_back(Move{sourceSquare, curSquare, sourcePiece, curPiece});
                }

                // since square is occupied we stop
                break;
            }

            // check next square
            curCol += dcol;
            curRow += drow;
        }
    }

    return out;
}

std::vector<Move> Game::generatePseudoLegalQueenMoves(int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = sourceSquare / 8;
    const int col = sourceSquare % 8;

    // eight possible moves (which are repeated)
    constexpr int deltas[9][2] {
        // col (x), row (y)
        // rook moves
        {0, 1}, // up
        {0, -1}, // down
        {1, 0}, // right
        {-1, 0}, // left
        // bishop moves
        {-1, -1}, // up left
        {1, -1}, // up right
        {-1, 1}, // down left
        {1, 1}, // down right
    };

    for(int i = 0; i < 9; i++) {
        const int dcol = deltas[i][0];
        const int drow = deltas[i][1];

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dcol;
        int curRow = row + drow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = curRow * 8 + curCol;
            const Piece curPiece = board_.at(curSquare);

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.push_back(Move{sourceSquare, curSquare, sourcePiece, curPiece});
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.push_back(Move{sourceSquare, curSquare, sourcePiece, curPiece});
                }

                // since square is occupied we stop
                break;
            }

            // check next square
            curCol += dcol;
            curRow += drow;
        }
    }

    return out;
}

// TODO: castling
std::vector<Move> Game::generatePseudoLegalKingMoves(int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = sourceSquare / 8;
    const int col = sourceSquare % 8;

    // 8 possible moves
    // comments are from whites POV, but they are symmetrical for black
    constexpr int kingDeltas[8][2] {
        // col (x), row (y)
        {-1, -1}, // up left
        {0, -1}, // up
        {1, -1}, // up right
        {-1, 0}, // left
        {1, 0}, // right
        {-1, 1}, // down left
        {0, 1}, // down
        {1, 1}, // down right
    };

    for(int i = 0; i < 8; i++) {
        const int col2 = col + kingDeltas[i][0];
        const int row2 = row + kingDeltas[i][1];
        if(!onBoard(col2, row2)) continue;
        
        const int square2 = row2 * 8 + col2;
        const Piece piece2 = board_.at(square2);

        // Can move if target square is empty or has enemy
        if (!piece2.exists() || piece2.color() != sourceColor) {
            out.push_back(Move{sourceSquare, square2, sourcePiece, piece2});
        }
    }

    return out;
}

// TODO: eventually make generateLegalMoves const by finding a workaround other than simply undoing moves
std::vector<Move> Game::generateLegalMoves(int sourceSquare) {
    std::vector<Move> pseudoMoves = generatePseudoLegalMoves(sourceSquare);
    std::vector<Move> legalMoves;

    // only allow moves that do not leave king in check
    for(Move move : pseudoMoves) {
        makeMove(move);
        if(!isInCheck(move.sourcePiece().color())) legalMoves.push_back(move);
        undoMove(move);
    }

    return legalMoves;
}

std::vector<Move> Game::generatePseudoLegalMoves(int sourceSquare) {
    switch(board_.at(sourceSquare).type()) {
        case PieceType::None: return std::vector<Move>{};
        case PieceType::Pawn: return generatePseudoLegalPawnMoves(sourceSquare);
        case PieceType::Knight: return generatePseudoLegalKnightMoves(sourceSquare);
        case PieceType::Bishop: return generatePseudoLegalBishopMoves(sourceSquare);
        case PieceType::Rook: return generatePseudoLegalRookMoves(sourceSquare);
        case PieceType::Queen: return generatePseudoLegalQueenMoves(sourceSquare);
        case PieceType::King: return generatePseudoLegalKingMoves(sourceSquare);
    }
}

// TODO: eventually make isMoveLegal const, see above^
bool Game::isMoveLegal(const Move move) {
    std::vector<Move> legalMoves = generateLegalMoves(move.sourceSquare());
    return std::find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end();
}

bool Game::tryMove(const Move move) {
    // only allow current turn's player to make moves
    if(move.sourcePiece().color() != currentTurn_) {
        return false;
    }

    if(!isMoveLegal(move)) {
        return false;
    }

    // move is legal, so make it
    this->makeMove(move);
    return true;
}

/*
    Make a move regardless of if it is legal or not.
*/
void Game::makeMove(const Move move) {
    board_.at(move.targetSquare()) = move.sourcePiece();
    board_.at(move.sourceSquare()) = Piece{};
    currentTurn_ = oppositeColor(currentTurn_);
}

/*
    Undo a move. This does not verify that the previous move matches the move we are undoing to.
*/
void Game::undoMove(const Move move) {
    board_.at(move.sourceSquare()) = move.sourcePiece();
    board_.at(move.targetSquare()) = move.targetPiece();
    currentTurn_ = oppositeColor(currentTurn_);
}

// TODO: consider extracting common deltas (e.g., knightDeltas) into Game class
/*
    Find if square is attacked. Greedily exits as soon as we find an attacking piece.
*/
bool Game::isSquareAttacked(int targetSquare, Color attackingColor) const {
    const int targetRow = targetSquare / 8;
    const int targetColumn = targetSquare % 8;

    // 1. Pawn attacks (only diagonals)
    // White attacks row - 1, Black attacks row + 1
    const int pawnDir = (attackingColor == Color::White) ? -1 : +1;
    const int attackingPawnRow = targetRow - pawnDir; // square where an attacking pawn would sit
    if (attackingPawnRow >= 0 && attackingPawnRow <= 7) {
        for (const int deltaColumn : {-1, +1}) {
            const int curCol = targetColumn + deltaColumn;
            // out of range
            if (!onBoard(attackingPawnRow, curCol)) continue;

            const int sq = attackingPawnRow * 8 + curCol;
            const Piece p = board_.at(sq);
            if (p.type() == PieceType::Pawn && p.color() == attackingColor) return true;
        }
    }

    // 2. Knight attacks
    constexpr int knightDeltas[8][2] {
        // col (x), row (y)
        {-2, -1}, // left up
        {-1, -2}, // up left
        {1, -2}, // up right
        {2, -1}, // right up
        {2, 1}, // right down
        {1, 2}, // down right
        {-1, 2}, // down left
        {-2, 1}, // left down
    };
    for (int i = 0; i < 8; i++) {
        const int curCol = targetColumn + knightDeltas[i][0];
        const int curRow = targetRow + knightDeltas[i][1];
        // out of bounds
        if (!onBoard(curRow, curCol)) continue;

        const int curSquare = curRow * 8 + curCol;
        const Piece p = board_.at(curSquare);
        if (p.type() == PieceType::Knight && p.color() == attackingColor) return true;
    }

    // 3. King attacks
    constexpr int kingDeltas[8][2] {
        // col (x), row (y)
        {-1, -1}, // up left
        {0, -1}, // up
        {1, -1}, // up right
        {-1, 0}, // left
        {1, 0}, // right
        {-1, 1}, // down left
        {0, 1}, // down
        {1, 1}, // down right
    };
    for (int i = 0; i < 8; i++) {
        const int curCol = targetColumn + kingDeltas[i][0];
        const int curRow = targetColumn + kingDeltas[i][1];
        // out of bounds
        if (!onBoard(curRow, curCol)) continue;

        const int curSquare = curRow * 8 + curCol;
        const Piece p = board_.at(curSquare);
        if (p.type() == PieceType::King && p.color() == attackingColor) return true;
    }

    // Sliding attack helper function
    auto rayHits = [&](int dRow, int dCol, PieceType a, PieceType b) -> bool {
        int curRow = targetRow + dRow;
        int curCol = targetColumn + dCol;
        while (onBoard(curRow, curCol)) {
            int sq = curRow * 8 + curCol;
            const Piece p = board_.at(sq);
            if (p.exists()) {
                if (p.color() == attackingColor && (p.type() == a || p.type() == b)) return true;
                // this direction is blocked
                return false;
            }
            // this direction is not blocked, continue
            curRow += dRow;
            curCol += dCol;
        }
        // didn't find anything
        return false;
    };

    // 4. orthogonal rays: rook or queen
    if (rayHits(+1, 0, PieceType::Rook, PieceType::Queen)) return true;
    if (rayHits(-1, 0, PieceType::Rook, PieceType::Queen)) return true;
    if (rayHits(0, +1, PieceType::Rook, PieceType::Queen)) return true;
    if (rayHits(0, -1, PieceType::Rook, PieceType::Queen)) return true;

    // 5. diagonal rays: bishop or queen
    if (rayHits(+1, +1, PieceType::Bishop, PieceType::Queen)) return true;
    if (rayHits(+1, -1, PieceType::Bishop, PieceType::Queen)) return true;
    if (rayHits(-1, +1, PieceType::Bishop, PieceType::Queen)) return true;
    if (rayHits(-1, -1, PieceType::Bishop, PieceType::Queen)) return true;

    // didn't find anything
    return false;
}


// TODO: optimize
bool Game::isInCheck(Color colorToFind) const {
    // TODO: this throws if game is inactive
    int kingSquare = findKingSquare(colorToFind).value();
    return isSquareAttacked(kingSquare, oppositeColor(colorToFind));
}

/*
    Find a given color's king. Should never return nullopt if game is active.
*/
std::optional<int> Game::findKingSquare(Color colorToFind) const {
    int squareIndex = 0;
    for(const Piece& piece : board_) {
        if(piece.type() == PieceType::King && piece.color() == colorToFind) return squareIndex;
        squareIndex++;
    }
    return std::nullopt;
}

std::string Game::intToAlgebraicNotation(int n) {
    int col = n%8;
    std::string file = std::string{(char)('a' + col)};
    
    // first reflect n over middle of chessboard
    int row = 7-(n/8);
    std::string rank = std::string{ (char)('1' + row)};

    return file + rank;
}

/*
    Check if square is on board.
*/
bool Game::onBoard(int square) {
    return 0 <= square && square <= 63;
}

/*
    Check if column and row combination is on board.
*/
bool Game::onBoard(int col, int row) {
    return 0 <= col && col <= 7 && 0 <= row && row <= 7;
}

Color Game::oppositeColor(Color c) {
    return (c == Color::White) ? Color::Black : Color::White;
}
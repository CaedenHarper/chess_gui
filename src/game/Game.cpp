#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "Game.hpp"

Piece::Piece() : type_{PieceType::None}, color_{Color::None} {
}

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

PieceType Piece::charToPieceType(const char piece) {
    if(piece == 'P' || piece == 'p') {
        return PieceType::Pawn;
    } 
    
    if(piece == 'N' || piece == 'n') {
        return PieceType::Knight;
    } 
    
    if(piece == 'B' || piece == 'b') {
        return PieceType::Bishop;
    } 
    
    if(piece == 'R' || piece == 'r') {
        return PieceType::Rook;
    } 
    
    if(piece == 'Q' || piece == 'q') {
        return PieceType::Queen;
    } 
    
    if(piece == 'K' || piece == 'k') {
        return PieceType::King;
    } 
    
    // TODO: error / debug msg here?
    return PieceType::None;
}

Piece Piece::charToPiece(const char piece) {
    switch (piece) {
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
    const bool isWhite = color_ == Color::White;

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
    const std::string color = color_ == Color::White ? "White " : "Black ";

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

Move::Move(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece) :
 sourceSquare_{sourceSquare},
 targetSquare_{targetSquare},
 sourcePiece_{sourcePiece},
 targetPiece_{targetPiece},
 isPromotion_{isPotentialPawnPromotion_(targetSquare, sourcePiece)},
 promotionPiece_{PieceType::Queen, sourcePiece.color()},
 isKingSideCastle_{isPotentialKingSideCastle_(sourceSquare, targetSquare, sourcePiece, targetPiece)},
 isQueenSideCastle_{isPotentialQueenSideCastle_(sourceSquare, targetSquare, sourcePiece, targetPiece)},
 isDoublePawn_{isPotentialDoublePawn_(sourceSquare, targetSquare, sourcePiece)},
 isEnPassant_{isPotentialEnPassant_(sourceSquare, targetSquare, sourcePiece, targetPiece)} {
}

bool Move::isPotentialPawnPromotion_(int targetSquare, Piece sourcePiece) {
    const int targetRow = Game::getRow(targetSquare);
    const int finalRow = (sourcePiece.color() == Color::White) ? 0 : 7; // rank 7 for white, rank 0 for black
    return targetRow == finalRow && sourcePiece.type() == PieceType::Pawn;
}

bool Move::isPotentialKingSideCastle_(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece) {
    if (sourcePiece.color() == Color::White) {
        return (
            sourceSquare == Game::WHITE_KING_STARTING_SQUARE &&
            targetSquare == Game::WHITE_KINGSIDE_TARGET_SQUARE &&
            sourcePiece.type() == PieceType::King &&
            !targetPiece.exists()
        );
    }

    if (sourcePiece.color() == Color::Black) {
        return (
            sourceSquare == Game::BLACK_KING_STARTING_SQUARE &&
            targetSquare == Game::BLACK_KINGSIDE_TARGET_SQUARE &&
            sourcePiece.type() == PieceType::King &&
            !targetPiece.exists()
        );
    }

    // fallback; should only hit for empty sourcePiece
    return false;
}

bool Move::isPotentialQueenSideCastle_(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece) {
    if (sourcePiece.color() == Color::White) {
        return (
            sourceSquare == Game::WHITE_KING_STARTING_SQUARE &&
            targetSquare == Game::WHITE_QUEENSIDE_TARGET_SQUARE &&
            sourcePiece.type() == PieceType::King &&
            !targetPiece.exists()
        );
    }

    if (sourcePiece.color() == Color::Black) {
        return (
            sourceSquare == Game::BLACK_KING_STARTING_SQUARE &&
            targetSquare == Game::BLACK_QUEENSIDE_TARGET_SQUARE &&
            sourcePiece.type() == PieceType::King &&
            !targetPiece.exists()
        );
    }

    // fallback; should only hit for empty sourcePiece
    return false;
}

bool Move::isPotentialDoublePawn_(int sourceSquare, int targetSquare, Piece sourcePiece) {
    // Only double pawn if pawn moves a distance of two rows
    return sourcePiece.type() == PieceType::Pawn && abs(Game::getRow(targetSquare) - Game::getRow(sourceSquare)) == 2;
}

bool Move::isPotentialEnPassant_(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece) {
    // Only en passant if pawn lands on empty square with a capture (i.e., not on same column)
    return sourcePiece.type() == PieceType::Pawn && !targetPiece.exists() && Game::getCol(sourceSquare) != Game::getCol(targetSquare);
}

std::string Move::to_string() const {
    return ( 
        sourcePiece_.to_string_long() + " on " + Game::intToAlgebraicNotation(sourceSquare_) + " to " +
        targetPiece_.to_string_long() + " on " + Game::intToAlgebraicNotation(targetSquare_)
    );
}

Game::Game()
    : currentTurn_{Color::White},
    canWhiteKingSideCastle_{true},
    canBlackKingSideCastle_{true},
    canWhiteQueenSideCastle_{true},
    canBlackQueenSideCastle_{true},
    currentEnPassantSquare{std::nullopt} {
}

Color Game::currentTurn() const {
    return currentTurn_;
}

std::array<Piece, 64> Game::board() const {
    return board_;
}

std::string Game::to_string() const {
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
    // starting border
    std::string out = "  +---------------+\n";

    int squareIndex = 0;
    for (const auto& piece : board_) {
        // print line start if at start of row
        if(getCol(squareIndex) == 0) {
            out += std::to_string(8 - getRow(squareIndex)) + " |";
        }
    
        if(piece.exists()) {
            out += piece.to_string_short() + "|";
        } else {
            // if piece is none, print square color
            // square color is white if the col polarity matches row polarity
            out += getCol(squareIndex) % 2 == getRow(squareIndex) % 2 ? " |" : "#|";
        }

        // print line ending if at end of row
        if(getCol(squareIndex) == 7) {
            out += "\n";
        }

        squareIndex++;
    }

    // print ending border
    return out + "  +---------------+\n   a b c d e f g h";
}

void Game::loadFEN(const std::string& FEN) {
    /*
    Fen has 6 fields:
    0 - piece placement
    1 - current color
    2 - castling
    3 - en passant
    4 - halfmove clock
    5 - current move number
    */
    int field = 0;
    // used for piece placement field to find out where we are putting each piece
    int piecePlacementIndex = 0;
    for(const char c : FEN) {  // NOLINT(readability-identifier-length)
        // space indicates we are ready for the next field
        if(c == ' ') {
            field++;
            continue;
        }

        // piece placement
        if(field == 0) {
            if(c >= '1' && c <= '8') {
                // Numbers indicate n empty squares
                const int numberEmptySquares = c-'0';
                for(int i = 0; i < numberEmptySquares; i++) {
                    const Piece emptySquare = Piece{};
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
            const Piece newPiece = Piece::charToPiece(c);
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

        // TODO: en passant
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

bool Game::isFinished() {
    std::optional<int> currentTurnKingSquare = findKingSquare(currentTurn_);
    // king doesn't exist, so game is over
    if(!currentTurnKingSquare) {
        return true;
    }
    // if no legal moves for current turn then the game is over
    return generateLegalMoves(currentTurnKingSquare.value()).empty();
}

void Game::setWhiteKingSideCastle(bool canCastle) {
    canWhiteKingSideCastle_ = canCastle;
}

void Game::setBlackKingSideCastle(bool canCastle) {
    canBlackKingSideCastle_ = canCastle;
}

void Game::setWhiteQueenSideCastle(bool canCastle) {
    canWhiteQueenSideCastle_ = canCastle;
}

void Game::setBlackQueenSideCastle(bool canCastle) {
    canBlackQueenSideCastle_ = canCastle;
}

std::optional<Move> Game::parseLongNotation_(const std::string& sourceMove, const std::string& targetMove) const {
    // TODO: implement pawn promotion eventually
    // We assume each string has at least two characters
    if(sourceMove.length() < 2 || targetMove.length() < 2) {
        // TODO: debug statement
        std::cerr << "parseLongNotation: sourceS or targetS not long enough!\n";
        return std::nullopt;
    }

    const char sourceColC = sourceMove.at(0);
    const char sourceRowC = sourceMove.at(1);
    const char targetColC = targetMove.at(0);
    const char targetRowC = targetMove.at(1);

    const int sourceCol = sourceColC - 'a';
    const int sourceRow = sourceRowC - '0';
    const int targetCol = targetColC - 'a';
    const int targetRow = targetRowC - '0';

    // all files and ranks should be in range 0 - 7
    if(sourceCol < 0 || sourceCol > 7 
    || sourceRow < 0 || sourceRow > 7
    || targetCol < 0 || targetCol > 7 
    || targetRow < 0 || targetRow > 7) {
        // TODO: debug statement
        std::cerr << "parseLongNotation: one file or rank is out of bounds\n";
        return std::nullopt;
    }

    // rank 1 starts at at index 0, whereas Game handles the board starting from the top left (rank 8 being 0)
    // therefore, we need to reflect it across the middle of the board with (8 - rank)
    const int sourceSquare = getSquareIndex(sourceCol, 8 - sourceRow);
    const int targetSquare = getSquareIndex(targetCol, 8 - targetRow); 

    return Move{sourceSquare, targetSquare, board_.at(sourceSquare), board_.at(targetSquare)};
}

std::optional<Move> Game::parseAlgebraicNotation_(const std::string& move) const { // NOLINT(readability-convert-member-functions-to-static) undo when
    std::cerr << "parseAlgebraicNotation: Not yet implemented! " << move;
    return std::nullopt;
}

/*
    We parse two types of input types: "sourceSquare targetSquare" (long notation) and algebraic notation.

    E.g., "e2 e4" vs. "e4" (from starting position).
*/
std::optional<Move> Game::parseMove(const std::string& move) const {
    int currentPart = 0;
    std::string firstPart;
    std::string secondPart;
    // iterate through and split based on first part before space vs. second part. any unknown characters are ignored. any spaces after the first space are ignored.
    for(const char c : move) { // NOLINT(readability-identifier-length)
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

std::vector<Move> Game::generatePseudoLegalPawnMoves_(const int sourceSquare) {
    /* 
        Four pawn capture types:
        1. One move forward
        2. Two moves forward, has to start on 2nd rank
        3. Capture left, has to be opposite color piece (or en passant)
        4. Capture right, has to be opposite color piece (or en passant)
    */
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    const int dir = (sourceColor == Color::White) ? -1 : 1; // this function is from black's pov
    const int startRow = (sourceColor == Color::White) ? 6 : 1;  // rank 7 / rank 2

    // Case 1 and 2
    const int one = sourceSquare + (dir * 8);
    if (onBoard(one) && !board_.at(one).exists()) {
        out.emplace_back(sourceSquare, one, sourcePiece, board_.at(one));

        // Case 2
        const int two = sourceSquare + (dir * 16);
        if (row == startRow && onBoard(two) && !board_.at(two).exists()) {
            out.emplace_back(sourceSquare, two, sourcePiece, board_.at(two));
        }
    }

    // captures
    // left/right depends on color only through dir:
    // white captures: -7 (left), -9 (right)
    // black captures: +9 (left), +7 (right)
    const int capLeft  = sourceSquare + (dir * 8) - 1;
    const int capRight = sourceSquare + (dir * 8) + 1;

    // Case 3
    if (col > 0 && onBoard(capLeft)) {
        const Piece capPiece = board_.at(capLeft);
        if (capPiece.exists() && capPiece.color() != sourceColor) {
            out.emplace_back(sourceSquare, capLeft, sourcePiece, capPiece);
        }

        // en passant
        if (currentEnPassantSquare.has_value() && capLeft == currentEnPassantSquare.value()) {
            out.emplace_back(sourceSquare, capLeft, sourcePiece, capPiece);
        }
    }

    // Case 4
    if (col < 7 && onBoard(capRight)) {
        const Piece capPiece = board_.at(capRight);
        if (capPiece.exists() && capPiece.color() != sourceColor) { 
            out.emplace_back(sourceSquare, capRight, sourcePiece, capPiece);
        }

        // en passant
        if (currentEnPassantSquare.has_value() && capRight == currentEnPassantSquare.value()) {
            out.emplace_back(sourceSquare, capRight, sourcePiece, capPiece);
        }
    }

    return out;
}


std::vector<Move> Game::generatePseudoLegalKnightMoves_(const int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 8; i++) {
        const int col2 = col + knightDeltas.at(i).at(0);
        const int row2 = row + knightDeltas.at(i).at(1);
        if(!onBoard(col2, row2)) {
            continue;
        }
        
        const int square2 = getSquareIndex(col2, row2);
        const Piece piece2 = board_.at(square2);

        // Can move if target square is empty or has enemy
        if (!piece2.exists() || piece2.color() != sourceColor) {
            out.emplace_back(sourceSquare, square2, sourcePiece, piece2);
        }
    }

    return out;
}

std::vector<Move> Game::generatePseudoLegalBishopMoves_(const int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 4; i++) {
        const int dcol = bishopDeltas.at(i).at(0);
        const int drow = bishopDeltas.at(i).at(1);

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dcol;
        int curRow = row + drow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = getSquareIndex(curCol, curRow);
            const Piece curPiece = board_.at(curSquare);

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.emplace_back(sourceSquare, curSquare, sourcePiece, curPiece);
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.emplace_back(sourceSquare, curSquare, sourcePiece, curPiece);
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

std::vector<Move> Game::generatePseudoLegalRookMoves_(const int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 4; i++) {
        const int dcol = rookDeltas.at(i).at(0);
        const int drow = rookDeltas.at(i).at(1);

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dcol;
        int curRow = row + drow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = getSquareIndex(curCol, curRow);
            const Piece curPiece = board_.at(curSquare);

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.emplace_back(sourceSquare, curSquare, sourcePiece, curPiece);
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.emplace_back(sourceSquare, curSquare, sourcePiece, curPiece);
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

std::vector<Move> Game::generatePseudoLegalQueenMoves_(const int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 8; i++) {
        const int dCol = queenDeltas.at(i).at(0);
        const int dRow = queenDeltas.at(i).at(1);

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dCol;
        int curRow = row + dRow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = getSquareIndex(curCol, curRow);
            const Piece curPiece = board_.at(curSquare);

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.emplace_back(sourceSquare, curSquare, sourcePiece, curPiece);
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.emplace_back(sourceSquare, curSquare, sourcePiece, curPiece);
                }

                // since square is occupied we stop
                break;
            }

            // check next square
            curCol += dCol;
            curRow += dRow;
        }
    }

    return out;
}

std::vector<Move> Game::generatePseudoLegalKingMoves_(const int sourceSquare) {
    std::vector<Move> out;

    const Piece sourcePiece = board_.at(sourceSquare);
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 8; i++) {
        const int col2 = col + kingDeltas.at(i).at(0);
        const int row2 = row + kingDeltas.at(i).at(1);
        if(!onBoard(col2, row2)) {
            continue;
        }
        
        const int square2 = getSquareIndex(col2, row2);
        const Piece piece2 = board_.at(square2);

        // Can move if target square is empty or has enemy
        if (!piece2.exists() || piece2.color() != sourceColor) {
            out.emplace_back(sourceSquare, square2, sourcePiece, piece2);
        }
    }

    // --- CASTLING ---

    if(sourceColor == Color::White) {
        // King side castling
        if(
            canWhiteKingSideCastle_ &&
            sourceSquare == Game::WHITE_KING_STARTING_SQUARE &&
            !board_.at(WHITE_KINGSIDE_PASSING_SQUARE).exists() &&
            !board_.at(WHITE_KINGSIDE_TARGET_SQUARE).exists()
        ) {
            out.emplace_back(sourceSquare, Game::WHITE_KINGSIDE_TARGET_SQUARE, sourcePiece, board_.at(Game::WHITE_KINGSIDE_TARGET_SQUARE));
        }

        // Queen side castling
        if(
            canWhiteQueenSideCastle_ &&
            sourceSquare == Game::WHITE_KING_STARTING_SQUARE &&
            !board_.at(WHITE_QUEENSIDE_PASSING_SQUARE).exists() &&
            !board_.at(WHITE_QUEENSIDE_TARGET_SQUARE).exists()
        ) {
            out.emplace_back(sourceSquare, Game::WHITE_QUEENSIDE_TARGET_SQUARE, sourcePiece, board_.at(Game::WHITE_QUEENSIDE_TARGET_SQUARE));
        }
    }

    if(sourceColor == Color::Black) {
        // King side castling
        if(
            canBlackKingSideCastle_ &&
            sourceSquare == Game::BLACK_KING_STARTING_SQUARE &&
            !board_.at(BLACK_KINGSIDE_PASSING_SQUARE).exists() &&
            !board_.at(BLACK_KINGSIDE_TARGET_SQUARE).exists()
        ) {
            out.emplace_back(sourceSquare, Game::BLACK_KINGSIDE_TARGET_SQUARE, sourcePiece, board_.at(Game::BLACK_KINGSIDE_TARGET_SQUARE));
        }

        // Queen side castling
        if(
            canBlackQueenSideCastle_ &&
            sourceSquare == Game::BLACK_KING_STARTING_SQUARE &&
            !board_.at(BLACK_QUEENSIDE_PASSING_SQUARE).exists() &&
            !board_.at(BLACK_QUEENSIDE_TARGET_SQUARE).exists()
        ) {
            out.emplace_back(sourceSquare, Game::BLACK_QUEENSIDE_TARGET_SQUARE, sourcePiece, board_.at(Game::BLACK_QUEENSIDE_TARGET_SQUARE));
        }
    }

    return out;
}

std::vector<Move> Game::generateLegalMoves(const int sourceSquare) {
    // TODO: eventually make generateLegalMoves const by finding a workaround other than simply undoing moves
    const std::vector<Move> pseudoMoves = generatePseudoLegalMoves_(sourceSquare);
    std::vector<Move> legalMoves;

    // only allow moves that do not leave king in check
    for(const Move move : pseudoMoves) {
        const Color moveColor = move.sourcePiece().color();
        const Color enemyColor = oppositeColor(moveColor);

        makeMove(move);
        // assume move is legal
        bool legal = true;

        // can't cause king to be in check
        if(isInCheck(moveColor)) {
            legal = false;
        }
        
        // castling legality rules
        const int KING_STARTING_SQUARE = (moveColor == Color::White) ? Game::WHITE_KING_STARTING_SQUARE : Game::BLACK_KING_STARTING_SQUARE;

        const int KINGSIDE_PASSING_SQUARE = (moveColor == Color::White) ? Game::WHITE_KINGSIDE_PASSING_SQUARE : Game::BLACK_KINGSIDE_PASSING_SQUARE;
        const int KINGSIDE_TARGET_SQUARE = (moveColor == Color::White) ? Game::WHITE_KINGSIDE_TARGET_SQUARE : Game::BLACK_KINGSIDE_TARGET_SQUARE;

        if(move.isKingSideCastle()) {
            if(
                isSquareAttacked(KING_STARTING_SQUARE, enemyColor) ||   // king cant start in check
                isSquareAttacked(KINGSIDE_PASSING_SQUARE, enemyColor) ||   // king cant pass through check 
                isSquareAttacked(KINGSIDE_TARGET_SQUARE, enemyColor)      // king cant end in check
            ) {
                legal = false;
            }
        }

        const int QUEENSIDE_PASSING_SQUARE = (moveColor == Color::White) ? Game::WHITE_QUEENSIDE_PASSING_SQUARE : Game::BLACK_QUEENSIDE_PASSING_SQUARE;
        const int QUEENSIDE_TARGET_SQUARE = (moveColor == Color::White) ? Game::WHITE_QUEENSIDE_TARGET_SQUARE : Game::BLACK_QUEENSIDE_TARGET_SQUARE;

        if(move.isQueenSideCastle()) {
            if(
                isSquareAttacked(KING_STARTING_SQUARE, enemyColor) ||   // king cant start in check
                isSquareAttacked(QUEENSIDE_PASSING_SQUARE, enemyColor) ||   // king cant pass through check 
                isSquareAttacked(QUEENSIDE_TARGET_SQUARE, enemyColor)      // king cant end in check
            ) {
                legal = false;
            }
        }


        if(legal) {
            legalMoves.emplace_back(move);
        }

        undoMove(move);
    }

    return legalMoves;
}

std::vector<Move> Game::generatePseudoLegalMoves_(const int sourceSquare) {
    switch(board_.at(sourceSquare).type()) {
        case PieceType::None: return std::vector<Move>{};
        case PieceType::Pawn: return generatePseudoLegalPawnMoves_(sourceSquare);
        case PieceType::Knight: return generatePseudoLegalKnightMoves_(sourceSquare);
        case PieceType::Bishop: return generatePseudoLegalBishopMoves_(sourceSquare);
        case PieceType::Rook: return generatePseudoLegalRookMoves_(sourceSquare);
        case PieceType::Queen: return generatePseudoLegalQueenMoves_(sourceSquare);
        case PieceType::King: return generatePseudoLegalKingMoves_(sourceSquare);
    }
}

bool Game::isMoveLegal(const Move move) {
    // TODO: eventually make isMoveLegal const, see above^
    std::vector<Move> legalMoves = generateLegalMoves(move.sourceSquare());
    // we didn't find anything!
    return std::any_of(legalMoves.begin(), legalMoves.end(), [move](Move legalMove) { return legalMove == move; });
}

bool Game::tryMove(const Move move) {
    // only allow current turn's player to make moves
    if(move.sourcePiece().color() != currentTurn_) {
        std::cerr << "[DEBUG] Attempted move: " + move.to_string() + " is not legal because it is not the correct player's turn\n";
        return false;
    }

    if(!isMoveLegal(move)) {
        std::cerr << "[DEBUG] Attempted move: " + move.to_string() + " is not legal per isMoveLegal\n";
        return false;
    }

    // move is legal, so make it
    makeMove(move);
    return true;
}

void Game::makeMove(const Move move) {
    const bool isSourcePieceWhite = move.sourcePiece().color() == Color::White;
    // flip current turn
    currentTurn_ = oppositeColor(currentTurn_);
    // remove en passant (we may set it again later in this function)
    currentEnPassantSquare.reset(); 

    // update castling flags
    if(
        move.sourceSquare() == Game::WHITE_KING_STARTING_SQUARE ||
        move.sourceSquare() == Game::WHITE_KINGSIDE_ROOK_STARTING_SQUARE
    ) {
        setWhiteKingSideCastle(false);
    }
    if(
        move.sourceSquare() == Game::WHITE_KING_STARTING_SQUARE ||
        move.sourceSquare() == Game::WHITE_QUEENSIDE_ROOK_STARTING_SQUARE
    ) {
        setWhiteQueenSideCastle(false);
    }
    if(
        move.sourceSquare() == Game::BLACK_KING_STARTING_SQUARE ||
        move.sourceSquare() == Game::BLACK_KINGSIDE_ROOK_STARTING_SQUARE
    ) {
        setBlackKingSideCastle(false);
    }
    if(
        move.sourceSquare() == Game::BLACK_KING_STARTING_SQUARE ||
        move.sourceSquare() == Game::BLACK_QUEENSIDE_ROOK_STARTING_SQUARE
    ) {
        setBlackQueenSideCastle(false);
    }

    // update en passant flag
    if(move.isDoublePawn()) {
        const int sourceRow = getRow(move.sourceSquare());
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int passedRow = sourceRow + towardsCenter; // row that was passed in the double move
        currentEnPassantSquare = getSquareIndex(getCol(move.sourceSquare()), passedRow); 
    }

    // If en passant capture, remove the captured pawn
    if (move.isEnPassant()) {
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int capturedIndex = move.targetSquare() - (towardsCenter * 8);
        board_.at(capturedIndex) = Piece{};
    }

    // TODO: fully implement promotion; for now Queen is assumed in all cases
    // handle pawn promotion
    if(move.isPromotion()) {
        board_.at(move.targetSquare()) = move.promotionPiece();
        board_.at(move.sourceSquare()) = Piece{};
        return;
    }

    // If king side castle, also move the rook
    if(move.isKingSideCastle()) {
        const int KINGSIDE_PASSING_SQUARE = isSourcePieceWhite ? WHITE_KINGSIDE_PASSING_SQUARE : BLACK_KINGSIDE_PASSING_SQUARE;
        const int KINGSIDE_ROOK_SQUARE = isSourcePieceWhite ? WHITE_KINGSIDE_ROOK_STARTING_SQUARE : BLACK_KINGSIDE_ROOK_STARTING_SQUARE;
        // also move rook
        board_.at(KINGSIDE_PASSING_SQUARE) = Piece{PieceType::Rook, move.sourcePiece().color()};
        board_.at(KINGSIDE_ROOK_SQUARE) = Piece{};
    }

    // If queen side castle, also move the queen
    if(move.isQueenSideCastle()) {
        const int QUEENSIDE_PASSING_SQUARE = isSourcePieceWhite ? WHITE_QUEENSIDE_PASSING_SQUARE : BLACK_QUEENSIDE_PASSING_SQUARE;
        const int QUEENSIDE_ROOK_SQUARE = isSourcePieceWhite ? WHITE_QUEENSIDE_ROOK_STARTING_SQUARE : BLACK_QUEENSIDE_ROOK_STARTING_SQUARE;
        // also move rook
        board_.at(QUEENSIDE_PASSING_SQUARE) = Piece{PieceType::Rook, move.sourcePiece().color()};
        board_.at(QUEENSIDE_ROOK_SQUARE) = Piece{};
    }

    board_.at(move.targetSquare()) = move.sourcePiece();
    board_.at(move.sourceSquare()) = Piece{};
}

void Game::undoMove(const Move move) {
    const bool isSourcePieceWhite = move.sourcePiece().color() == Color::White;
    // flip current turn
    currentTurn_ = oppositeColor(currentTurn_);

    // handle king side castle
    if(move.isKingSideCastle()) {
        const int KINGSIDE_PASSING_SQUARE = isSourcePieceWhite ? WHITE_KINGSIDE_PASSING_SQUARE : BLACK_KINGSIDE_PASSING_SQUARE;
        const int KINGSIDE_ROOK_SQUARE = isSourcePieceWhite ? WHITE_KINGSIDE_ROOK_STARTING_SQUARE : BLACK_KINGSIDE_ROOK_STARTING_SQUARE;
        // also move rook
        board_.at(KINGSIDE_PASSING_SQUARE) = Piece{};
        board_.at(KINGSIDE_ROOK_SQUARE) = Piece{PieceType::Rook, move.sourcePiece().color()};
        // undo castling flag
        isSourcePieceWhite ? setWhiteKingSideCastle(true) : setBlackKingSideCastle(true);
    }

    // handle queen side castle
    if(move.isQueenSideCastle()) {
        const int QUEENSIDE_PASSING_SQUARE = isSourcePieceWhite ? WHITE_QUEENSIDE_PASSING_SQUARE : BLACK_QUEENSIDE_PASSING_SQUARE;
        const int QUEENSIDE_ROOK_SQUARE = isSourcePieceWhite ? WHITE_QUEENSIDE_ROOK_STARTING_SQUARE : BLACK_QUEENSIDE_ROOK_STARTING_SQUARE;
        // also move rook
        board_.at(QUEENSIDE_PASSING_SQUARE) = Piece{};
        board_.at(QUEENSIDE_ROOK_SQUARE) = Piece{PieceType::Rook, move.sourcePiece().color()};
        // undo castling flag
        isSourcePieceWhite ? setWhiteQueenSideCastle(true) : setBlackQueenSideCastle(true);
    }

    // handle en passant
    if(move.isEnPassant()) {
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int capturedIndex = move.targetSquare() - (towardsCenter * 8);
        // replace captured pawn
        board_.at(capturedIndex) = Piece{PieceType::Pawn, oppositeColor(move.sourcePiece().color())};
        // undo en passant flag
        currentEnPassantSquare = move.targetSquare();
    }

    // If the last move was a double pawn we undo the en passant square that we set
    if(move.isDoublePawn()) {
        currentEnPassantSquare.reset();
    }

    board_.at(move.sourceSquare()) = move.sourcePiece();
    board_.at(move.targetSquare()) = move.targetPiece();
}

bool Game::isSquareAttacked(const int targetSquare, const Color attackingColor) const {
    // TODO: consider extracting common deltas (e.g., knightDeltas) into Game class
    const int targetRow = getRow(targetSquare);
    const int targetColumn = getCol(targetSquare);

    // 1. Pawn attacks (only diagonals)
    // White attacks row - 1, Black attacks row + 1
    const int pawnDir = (attackingColor == Color::White) ? -1 : +1;
    const int attackingPawnRow = targetRow - pawnDir; // square where an attacking pawn would sit
    if (attackingPawnRow >= 0 && attackingPawnRow <= 7) {
        for (const int deltaColumn : {-1, +1}) {
            const int curCol = targetColumn + deltaColumn;
            // out of range
            if (!onBoard(curCol, attackingPawnRow)) {
                continue;
            }

            const int attackingPawnSquare = getSquareIndex(curCol, attackingPawnRow);
            const Piece possibleAttackingPawn = board_.at(attackingPawnSquare);
            if (possibleAttackingPawn.type() == PieceType::Pawn && possibleAttackingPawn.color() == attackingColor) {
                return true;
            }
        }
    }

    // 2. Knight attacks
    for (int i = 0; i < 8; i++) {
        const int curCol = targetColumn + knightDeltas.at(i).at(0);
        const int curRow = targetRow + knightDeltas.at(i).at(1);
        // out of bounds
        if (!onBoard(curCol, curRow)) {
            continue;
        }

        const int curSquare = getSquareIndex(curCol, curRow);
        const Piece possibleKnight = board_.at(curSquare);
        if (possibleKnight.type() == PieceType::Knight && possibleKnight.color() == attackingColor) {
            return true;
        }
    }

    // 3. King attacks
    for (int i = 0; i < 8; i++) {
        const int curCol = targetColumn + kingDeltas.at(i).at(0);
        const int curRow = targetRow + kingDeltas.at(i).at(1);
        // out of bounds
        if (!onBoard(curCol, curRow)) {
            continue;
        }

        const int curSquare = getSquareIndex(curCol, curRow);
        const Piece possibleKing = board_.at(curSquare);
        if (possibleKing.type() == PieceType::King && possibleKing.color() == attackingColor) {
            return true;
        }
    }

    // Sliding attack helper function
    auto rayHits = [&](int dRow, int dCol, PieceType attackingPiece1, PieceType attackingPiece2) -> bool {
        int curRow = targetRow + dRow;
        int curCol = targetColumn + dCol;
        while (onBoard(curCol, curRow)) {
            const int curSquare = getSquareIndex(curCol, curRow);
            const Piece attackingPiece = board_.at(curSquare);
            if (attackingPiece.exists()) {
                // if not right piece, we are blocked and can return
                return attackingPiece.color() == attackingColor && (attackingPiece.type() == attackingPiece1 || attackingPiece.type() == attackingPiece2);
            }
            // this direction is not blocked, continue
            curRow += dRow;
            curCol += dCol;
        }
        // didn't find anything
        return false;
    };

    // 4. orthogonal rays: rook or queen
    if (rayHits(+1, 0, PieceType::Rook, PieceType::Queen)) {
        return true;
    }
    if (rayHits(-1, 0, PieceType::Rook, PieceType::Queen)) {
        return true;
    }
    if (rayHits(0, +1, PieceType::Rook, PieceType::Queen)) {
        return true;
    }
    if (rayHits(0, -1, PieceType::Rook, PieceType::Queen)) {
        return true;
    }

    // 5. diagonal rays: bishop or queen
    if (rayHits(+1, +1, PieceType::Bishop, PieceType::Queen)) {
        return true;
    }
    if (rayHits(+1, -1, PieceType::Bishop, PieceType::Queen)) {
        return true;
    }
    if (rayHits(-1, +1, PieceType::Bishop, PieceType::Queen)) {
        return true;
    }
    if (rayHits(-1, -1, PieceType::Bishop, PieceType::Queen)) {
        return true;
    }

    // didn't find anything
    return false;
}

bool Game::isInCheck(const Color colorToFind) const {
    // TODO: optimize
    // TODO: this throws if game is inactive
    const int kingSquare = findKingSquare(colorToFind).value();
    return isSquareAttacked(kingSquare, oppositeColor(colorToFind));
}

std::optional<int> Game::findKingSquare(const Color colorToFind) const {
    int squareIndex = 0;
    for(const Piece& piece : board_) {
        if(piece.type() == PieceType::King && piece.color() == colorToFind) {
            return squareIndex;
        }
        squareIndex++;
    }
    return std::nullopt;
}

std::string Game::intToAlgebraicNotation(const int square) {
    const int col = getCol(square);
    const std::string file = std::string{static_cast<char>('a' + col)};
    
    // reflect n over middle of chessboard
    const int row = 7 - getRow(square);
    const std::string rank = std::string{ static_cast<char>('1' + row)};

    return file + rank;
}

bool Game::onBoard(const int square) {
    return 0 <= square && square <= 63;
}

bool Game::onBoard(const int col, int row) {
    return 0 <= col && col <= 7 && 0 <= row && row <= 7;
}

int Game::getCol(int square) {
    return square % 8;
}

int Game::getRow(int square) {
    return square / 8;
}

int Game::getSquareIndex(int col, int row) {
    return (8 * row) + col;
}

Color Game::oppositeColor(const Color color) {
    return (color == Color::White) ? Color::Black : Color::White;
}
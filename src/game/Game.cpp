#include <iostream>
#include <optional>
#include <string>

#include "Game.hpp"

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
    const bool isWhite = color() == Color::White;

    switch (type()) {
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
    const std::string out = color() == Color::White ? "White " : "Black ";

    switch (type()) {
        case PieceType::None: return "Empty Square";
        case PieceType::Pawn: return out + "Pawn";
        case PieceType::Knight: return out + "Knight";
        case PieceType::Bishop: return out + "Bishop";
        case PieceType::Rook: return out + "Rook";
        case PieceType::Queen: return out + "Queen";
        case PieceType::King: return out + "King";
    }
}

Move Move::fromPieces(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece) {
    const int sourceCol = Game::getCol(sourceSquare);
    const int sourceRow = Game::getRow(sourceSquare);
    const int targetCol = Game::getCol(targetSquare);
    const int targetRow = Game::getRow(targetSquare);
    const bool isCapture = targetPiece.exists();
    const bool isSourcePiecePawn = sourcePiece.type() == PieceType::Pawn;
    const bool isSourcePieceWhite = sourcePiece.color() == Color::White;
    const int promotionRow = isSourcePieceWhite ? 0 : 7; // rank 7 for white, rank 0 for black
    
    // --- Special pawn moves ---
    // Pawn promotion iff a pawn is ending on the promotion row
    if(isSourcePiecePawn && targetRow == promotionRow) {
        const MoveFlag flag = isCapture ? MoveFlag::PromotionCapture : MoveFlag::Promotion;
        // TODO: queen is assumed in fromPieces (e.g., when user makes a move); add a way to specify
        return Move{sourceSquare, targetSquare, flag, Promotion::Queen};
    }

    // Double pawn move iff a pawn moves a distance of two rows
    if(isSourcePiecePawn && abs(targetRow - sourceRow) == 2) {
        return Move{sourceSquare, targetSquare, MoveFlag::DoublePawnPush, Promotion::None};
    }

    // En passant iff a pawn lands on an empty square, but its not a normal pawn push (e.g., ends on a different column)
    if(isSourcePiecePawn && !targetPiece.exists() && sourceCol != targetCol) {
        return Move{sourceSquare, targetSquare, MoveFlag::EnPassant, Promotion::None};
    }

    // Update castling related constants based on source piece color
    const bool isSourcePieceKing = sourcePiece.type() == PieceType::King;
    const int kingStartingSquare = isSourcePieceWhite ? Game::WHITE_KING_STARTING_SQUARE : Game::BLACK_KING_STARTING_SQUARE;
    const int kingsideTargetSquare = isSourcePieceWhite ? Game::WHITE_KINGSIDE_TARGET_SQUARE : Game::BLACK_KINGSIDE_TARGET_SQUARE;
    const int queensideTargetSquare = isSourcePieceWhite ? Game::WHITE_QUEENSIDE_TARGET_SQUARE : Game::BLACK_QUEENSIDE_TARGET_SQUARE;

    // --- Special castling moves ---
    // (note we are just generating a move here, we don't validate it. so its okay if we are castling through a piece for example)

    // TODO: do we have to check if the targetpiece doesn't exist here?
    // Kingside castle iff a king moves from its starting square to the castling target square, and the piece does not exist.
    if(isSourcePieceKing && sourceSquare == kingStartingSquare && targetSquare == kingsideTargetSquare && !targetPiece.exists()) {
        return Move{sourceSquare, targetSquare, MoveFlag::KingCastle, Promotion::None};
    }

    // Queenside castle iff a king moves from its starting square to the castling target square, and the piece does not exist.
    if(isSourcePieceKing && sourceSquare == kingStartingSquare && targetSquare == queensideTargetSquare && !targetPiece.exists()) {
        return Move{sourceSquare, targetSquare, MoveFlag::QueenCastle, Promotion::None};
    }

    // No special move, return normal move
    const MoveFlag flag = isCapture ? MoveFlag::Capture : MoveFlag::Normal;
    return Move{sourceSquare, targetSquare, flag, Promotion::None};
}

std::string Move::to_string(const Game& game) const {
    return ( 
        game.board()[sourceSquare()].to_string_long() + " on " + Game::intToAlgebraicNotation(sourceSquare()) + " to " +
        game.board()[targetSquare()].to_string_long() + " on " + Game::intToAlgebraicNotation(targetSquare())
    );
}

std::string Move::toLongAlgebraic() const {
    if(isPromotion()) {
        // promotion requires an additional character representing which piece we promoted to
        char promotionRepresentation = '?';

        switch(promotion()) {
            case Promotion::None: break; // should never happen
            case Promotion::Knight: promotionRepresentation = 'n'; break;
            case Promotion::Bishop: promotionRepresentation = 'b'; break;
            case Promotion::Rook: promotionRepresentation = 'r'; break;
            case Promotion::Queen: promotionRepresentation = 'q'; break;
        }

        return Game::intToAlgebraicNotation(sourceSquare()) + Game::intToAlgebraicNotation(targetSquare()) + promotionRepresentation;
    }

    // normal move
    return Game::intToAlgebraicNotation(sourceSquare()) + Game::intToAlgebraicNotation(targetSquare());
}

constexpr PieceType Move::promotionToPieceType(Promotion promotion) noexcept {
    switch (promotion) {
        case Promotion::Knight: return PieceType::Knight;
        case Promotion::Bishop: return PieceType::Bishop;
        case Promotion::Rook: return PieceType::Rook;
        case Promotion::Queen: return PieceType::Queen;
        default: return PieceType::None;
    }
}

Game::Game()
    : currentTurn_{Color::White},
    castlingRights_{0},
    whiteKingSquare_{0},
    blackKingSquare_{0},
    currentEnPassantSquare_{UndoInfo::noEnPassant} {
}

Color Game::currentTurn() const {
    return currentTurn_;
}

std::array<Piece, Game::NUM_SQUARES> Game::board() const {
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
    5 - fullmove clock
    */
    int field = 0;
    // used for piece placement field to find out where we are putting each piece
    int piecePlacementIndex = 0;
    // used for building the en passant square
    std::string enPassantSquare;
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
                    board_.at(piecePlacementIndex) = Piece{};
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
                throw std::runtime_error("Unable to parse FEN: " + FEN + "\nInvalid char piece: " + "'" + c + "'");
            }

            // we have a valid piece, add it and update index
            board_.at(piecePlacementIndex) = newPiece;
            setBitboardForPiece(piecePlacementIndex, newPiece);

            // if its a king, we update our knowledge of the king's location
            if(newPiece.type() == PieceType::King) {
                if(newPiece.color() == Color::White) {
                    whiteKingSquare_ = piecePlacementIndex;
                } else {
                    blackKingSquare_ = piecePlacementIndex;
                }
            }

            piecePlacementIndex++;
            continue;
        }

        // current color
        if(field == 1) {
            // this field only has one character, either w or b
            switch(c) {
                // white's turn
                case 'w': currentTurn_ = Color::White; break;
                // black's turn
                case 'b': currentTurn_ = Color::Black; break;
                default: throw std::runtime_error("Unable to parse FEN: " + FEN + "\nInvalid current color: " + "'" + c + "'");
            }
            continue;
        }

        // castling
        if(field == 2) {
            switch (c) {
                // White may castle kingside
                case 'K': castlingRights_.setWhiteKingside(); break;
                // White may castle queenside
                case 'Q': castlingRights_.setWhiteQueenside(); break;
                // Black may castle kingside
                case 'k': castlingRights_.setBlackKingside(); break;
                // Black may castle queenside
                case 'q': castlingRights_.setBlackQueenside(); break;
                // No one can castle
                case '-': break;
                default: throw std::runtime_error("Unable to parse FEN: " + FEN + "\nInvalid castling char: " + "'" + c + "'");
            }
            continue;
        }

        // en passant
        if(field == 3) {
            // This field has '-' for no en passant, or algebraic notation of the current en passant square
            if(c == '-') {
                continue;
            }

            // first char of en passant square
            if(enPassantSquare.empty()) {
                enPassantSquare += c;
            } else {
                // second square, we can set it and we are done
                enPassantSquare += c;
                currentEnPassantSquare_ = algebraicNotationToInt(enPassantSquare);
            }

            continue;
        }

        // TODO: halfmove clock
        if(field == 4) {
            continue;
        }

        // TODO: fullmove clock
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
    MoveList legalMoves;
    generateLegalMoves(currentTurnKingSquare.value(), legalMoves);
    return legalMoves.size == 0;
}

UndoInfo Game::getUndoInfo(const Piece capturedPiece) const {
    // pack bools into castling rights uint8_t for speedy lookup
    return UndoInfo{
        castlingRights_,
        static_cast<uint8_t>(currentEnPassantSquare_),
        capturedPiece,
        static_cast<uint8_t>(whiteKingSquare_),
        static_cast<uint8_t>(blackKingSquare_),
    };
}

Piece Game::pieceAtSquareForGui(int square) const noexcept {
    const uint64_t squareBit = Bitboard::bit(square);

    // Uses syntax which omits Piece{},
    if (bbWhitePawns_.hasBit(squareBit))   {
        return {PieceType::Pawn, Color::White};
    }
    if (bbWhiteKnights_.hasBit(squareBit)) {
        return {PieceType::Knight, Color::White};
    }
    if (bbWhiteBishops_.hasBit(squareBit)) {
        return {PieceType::Bishop, Color::White};
    }
    if (bbWhiteRooks_.hasBit(squareBit))   {
        return {PieceType::Rook, Color::White};
    }
    if (bbWhiteQueens_.hasBit(squareBit))  {
        return {PieceType::Queen, Color::White};
    }
    if (bbWhiteKing_.hasBit(squareBit))    {
        return {PieceType::King, Color::White};
    }

    if (bbBlackPawns_.hasBit(squareBit))   {
        return {PieceType::Pawn, Color::Black};
    }
    if (bbBlackKnights_.hasBit(squareBit)) {
        return {PieceType::Knight, Color::Black};
    }
    if (bbBlackBishops_.hasBit(squareBit)) {
        return {PieceType::Bishop, Color::Black};
    }
    if (bbBlackRooks_.hasBit(squareBit))   {
        return {PieceType::Rook, Color::Black};
    }
    if (bbBlackQueens_.hasBit(squareBit))  {
        return {PieceType::Queen, Color::Black};
    }
    if (bbBlackKing_.hasBit(squareBit))    {
        return {PieceType::King, Color::Black};
    }

    return {}; // empty
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

    return Move::fromPieces(sourceSquare, targetSquare, board_.at(sourceSquare), board_.at(targetSquare));
}

std::optional<Move> Game::parseAlgebraicNotation_(const std::string& move) const { // NOLINT(readability-convert-member-functions-to-static) undo when finished
    std::cerr << "parseAlgebraicNotation: Not yet implemented! " << move;
    return std::nullopt;
}

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

void Game::addAllPawnPromotionsToMoves_(MoveList& moves, const int sourceSquare, const int targetSquare, const Piece sourcePiece, const bool isCapture) {
    const Color pawnColor = sourcePiece.color();
    const int promotionRow = pawnColor == Color::White ? 0 : 7; 
    if(getRow(targetSquare) == promotionRow) {
        const MoveFlag flag = isCapture ? MoveFlag::PromotionCapture : MoveFlag::Promotion;
        // add promotions
        moves.push_back(Move{sourceSquare, targetSquare, flag, Promotion::Knight});
        moves.push_back(Move{sourceSquare, targetSquare, flag, Promotion::Bishop});
        moves.push_back(Move{sourceSquare, targetSquare, flag, Promotion::Rook});
        moves.push_back(Move{sourceSquare, targetSquare, flag, Promotion::Queen});
    } else {
        const MoveFlag flag = isCapture ? MoveFlag::Capture : MoveFlag::Normal;
        // just add normal move
        moves.push_back(Move{sourceSquare, targetSquare, flag, Promotion::None});
    }
}

void Game::generatePseudoLegalPawnMoves_(const int sourceSquare, MoveList& out) {
    /* 
        Four pawn capture types:
        1. One move forward
        2. Two moves forward, has to start on 2nd rank
        3. Capture left, has to be opposite color piece (or en passant)
        4. Capture right, has to be opposite color piece (or en passant)
    */
    const Piece sourcePiece = board_[sourceSquare];
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    const int dir = (sourceColor == Color::White) ? -1 : 1; // this function is from black's pov
    const int startRow = (sourceColor == Color::White) ? 6 : 1;  // rank 7 / rank 2

    // Case 1 and 2
    const int one = sourceSquare + (dir * 8);
    if (onBoard(one) && !board_[one].exists()) {
        addAllPawnPromotionsToMoves_(out, sourceSquare, one, sourcePiece, false);

        // Case 2
        const int two = sourceSquare + (dir * 16);
        if (row == startRow && onBoard(two) && !board_[two].exists()) {
            out.push_back(Move{sourceSquare, two, MoveFlag::DoublePawnPush, Promotion::None});
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
        const Piece capPiece = board_[capLeft];
        if (capPiece.exists() && capPiece.color() != sourceColor) {
            addAllPawnPromotionsToMoves_(out, sourceSquare, capLeft, sourcePiece, true);
        }

        // en passant
        if (capLeft == currentEnPassantSquare_) {
            out.push_back(Move{sourceSquare, capLeft, MoveFlag::EnPassant, Promotion::None});
        }
    }

    // Case 4
    if (col < 7 && onBoard(capRight)) {
        const Piece capPiece = board_[capRight];
        if (capPiece.exists() && capPiece.color() != sourceColor) { 
            addAllPawnPromotionsToMoves_(out, sourceSquare, capRight, sourcePiece, true);
        }

        // en passant
        if (capRight == currentEnPassantSquare_) {
            // en passant cant be a promotion
            out.push_back(Move{sourceSquare, capRight, MoveFlag::EnPassant, Promotion::None});
        }
    }
}


void Game::generatePseudoLegalKnightMoves_(const int sourceSquare, MoveList& out) {
    const Piece sourcePiece = board_[sourceSquare];
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 8; i++) {
        const int col2 = col + knightDeltas[i][0];
        const int row2 = row + knightDeltas[i][1];
        if(!onBoard(col2, row2)) {
            continue;
        }
        
        const int square2 = getSquareIndex(col2, row2);
        const Piece piece2 = board_[square2];

        // Can move if target square is empty or has enemy
        if (!piece2.exists() || piece2.color() != sourceColor) {
            const MoveFlag flag = piece2.exists() ? MoveFlag::Capture : MoveFlag::Normal;
            out.push_back(Move{sourceSquare, square2, flag, Promotion::None});
        }
    }
}

void Game::generatePseudoLegalBishopMoves_(const int sourceSquare, MoveList& out) {
    const Piece sourcePiece = board_[sourceSquare];
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 4; i++) {
        const int dcol = bishopDeltas[i][0];
        const int drow = bishopDeltas[i][1];

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dcol;
        int curRow = row + drow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = getSquareIndex(curCol, curRow);
            const Piece curPiece = board_[curSquare];

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.push_back(Move{sourceSquare, curSquare, MoveFlag::Normal, Promotion::None});
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.push_back(Move{sourceSquare, curSquare, MoveFlag::Capture, Promotion::None});
                }

                // since square is occupied we stop
                break;
            }

            // check next square
            curCol += dcol;
            curRow += drow;
        }
    }
}

void Game::generatePseudoLegalRookMoves_(const int sourceSquare, MoveList& out) {
    const Piece sourcePiece = board_[sourceSquare];
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 4; i++) {
        const int dcol = rookDeltas[i][0];
        const int drow = rookDeltas[i][1];

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dcol;
        int curRow = row + drow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = getSquareIndex(curCol, curRow);
            const Piece curPiece = board_[curSquare];

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.push_back(Move{sourceSquare, curSquare, MoveFlag::Normal, Promotion::None});
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.push_back(Move{sourceSquare, curSquare, MoveFlag::Capture, Promotion::None});
                }

                // since square is occupied we stop
                break;
            }

            // check next square
            curCol += dcol;
            curRow += drow;
        }
    }
}

void Game::generatePseudoLegalQueenMoves_(const int sourceSquare, MoveList& out) {
    const Piece sourcePiece = board_[sourceSquare];
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 8; i++) {
        const int dCol = queenDeltas[i][0];
        const int dRow = queenDeltas[i][1];

        // start with one delta and continue until off board, or until a piece is hit
        int curCol = col + dCol;
        int curRow = row + dRow;
        while(onBoard(curCol, curRow)) {
            const int curSquare = getSquareIndex(curCol, curRow);
            const Piece curPiece = board_[curSquare];

            if(!curPiece.exists()) {
                // empty square; we can continue
                out.push_back(Move{sourceSquare, curSquare, MoveFlag::Normal, Promotion::None});
            } else {
                // theres a piece here
                
                if(curPiece.color() != sourceColor) {
                    // enemy piece, we can capture
                    out.push_back(Move{sourceSquare, curSquare, MoveFlag::Capture, Promotion::None});
                }

                // since square is occupied we stop
                break;
            }

            // check next square
            curCol += dCol;
            curRow += dRow;
        }
    }
}

void Game::generatePseudoLegalKingMoves_(const int sourceSquare, MoveList& out) {
    const Piece sourcePiece = board_[sourceSquare];
    const Color sourceColor = sourcePiece.color();

    const int row = getRow(sourceSquare);
    const int col = getCol(sourceSquare);

    for(int i = 0; i < 8; i++) {
        const int col2 = col + kingDeltas[i][0];
        const int row2 = row + kingDeltas[i][1];
        if(!onBoard(col2, row2)) {
            continue;
        }
        
        const int square2 = getSquareIndex(col2, row2);
        const Piece piece2 = board_[square2];

        // Can move if target square is empty or has enemy
        if (!piece2.exists() || piece2.color() != sourceColor) {
            const MoveFlag flag = piece2.exists() ? MoveFlag::Capture : MoveFlag::Normal;
            out.push_back(Move{sourceSquare, square2, flag, Promotion::None});
        }
    }

    // TODO: this logic can be cleaned up, remove if(SourceColor)... check
    // --- CASTLING ---

    if(sourceColor == Color::White) {
        // King side castling
        if(
            castlingRights_.canWhiteKingside() &&
            sourceSquare == WHITE_KING_STARTING_SQUARE &&
            !board_[WHITE_KINGSIDE_PASSING_SQUARE].exists() &&
            !board_[WHITE_KINGSIDE_TARGET_SQUARE].exists()
        ) {
            out.push_back(Move{sourceSquare, WHITE_KINGSIDE_TARGET_SQUARE, MoveFlag::KingCastle, Promotion::None});
        }

        // Queen side castling
        if(
            castlingRights_.canWhiteQueenside() &&
            sourceSquare == WHITE_KING_STARTING_SQUARE &&
            !board_[WHITE_QUEENSIDE_PASSING_SQUARE].exists() &&
            !board_[WHITE_QUEENSIDE_PASSING_SQUARE - 2].exists() &&  // queenside has two passing squares
            !board_[WHITE_QUEENSIDE_TARGET_SQUARE].exists()
        ) {
            out.push_back(Move{sourceSquare, WHITE_QUEENSIDE_TARGET_SQUARE, MoveFlag::QueenCastle, Promotion::None});
        }
    }

    if(sourceColor == Color::Black) {
        // King side castling
        if(
            castlingRights_.canBlackKingside() &&
            sourceSquare == BLACK_KING_STARTING_SQUARE &&
            !board_[BLACK_KINGSIDE_PASSING_SQUARE].exists() &&
            !board_[BLACK_KINGSIDE_TARGET_SQUARE].exists()
        ) {
            out.push_back(Move{sourceSquare, BLACK_KINGSIDE_TARGET_SQUARE, MoveFlag::KingCastle, Promotion::None});
        }

        // Queen side castling
        if(
            castlingRights_.canBlackQueenside() &&
            sourceSquare == BLACK_KING_STARTING_SQUARE &&
            !board_[BLACK_QUEENSIDE_PASSING_SQUARE].exists() &&
            !board_[BLACK_QUEENSIDE_PASSING_SQUARE - 2].exists() &&  // queenside has two passing squares
            !board_[BLACK_QUEENSIDE_TARGET_SQUARE].exists()
        ) {
            out.push_back(Move{sourceSquare, BLACK_QUEENSIDE_TARGET_SQUARE, MoveFlag::QueenCastle, Promotion::None});
        }
    }
}

void Game::generateLegalMoves(const int sourceSquare, MoveList& out) {
    // TODO: eventually make generateLegalMoves const by finding a workaround other than simply undoing moves
    MoveList pseudoMoves;
    generatePseudoLegalMoves_(sourceSquare, pseudoMoves);

    // only allow moves that do not leave king in check
    for(int i = 0; i < pseudoMoves.size; i++) {
        const Move move = pseudoMoves.data[i];
        const Piece& sourcePiece = board_[move.sourceSquare()];
        const Color moveColor = sourcePiece.color();
        const Color enemyColor = oppositeColor(moveColor);

        // save info to pass to undo move
        const UndoInfo undoInfo = getUndoInfo(board_[move.targetSquare()]);

        makeMove(move);

        // can't cause king to be in check
        if(isInCheck(moveColor)) {
            undoMove(move, undoInfo);
            continue;
        }
        
        // castling legality rules
        const int kingStartingSquare = (moveColor == Color::White) ? Game::WHITE_KING_STARTING_SQUARE : Game::BLACK_KING_STARTING_SQUARE;

        if(move.isKingSideCastle()) {
            const int kingsidePassingSquare = (moveColor == Color::White) ? Game::WHITE_KINGSIDE_PASSING_SQUARE : Game::BLACK_KINGSIDE_PASSING_SQUARE;
            const int kingsideTargetSquare = (moveColor == Color::White) ? Game::WHITE_KINGSIDE_TARGET_SQUARE : Game::BLACK_KINGSIDE_TARGET_SQUARE;
    
            if(
                isSquareAttacked(kingStartingSquare, enemyColor) ||   // king cant start in check
                isSquareAttacked(kingsidePassingSquare, enemyColor) ||   // king cant pass through check 
                isSquareAttacked(kingsideTargetSquare, enemyColor)      // king cant end in check
            ) {
                undoMove(move, undoInfo);
                continue;
            }
        }

        if(move.isQueenSideCastle()) {
            const int queensidePassingSquare = (moveColor == Color::White) ? Game::WHITE_QUEENSIDE_PASSING_SQUARE : Game::BLACK_QUEENSIDE_PASSING_SQUARE;
            const int queensideTargetSquare = (moveColor == Color::White) ? Game::WHITE_QUEENSIDE_TARGET_SQUARE : Game::BLACK_QUEENSIDE_TARGET_SQUARE;
            if(
                isSquareAttacked(kingStartingSquare, enemyColor) ||   // king cant start in check
                isSquareAttacked(queensidePassingSquare, enemyColor) ||   // king cant pass through check 
                isSquareAttacked(queensideTargetSquare, enemyColor)      // king cant end in check
            ) {
                undoMove(move, undoInfo);
                continue;
            }
        }

        // move is legal, allow it
        out.push_back(move);
        undoMove(move, undoInfo);
    }
}

void Game::generateAllPseudoLegalMoves(MoveList& out) {
    out.clear();

    for(int squareIndex = 0; squareIndex < Game::NUM_SQUARES; squareIndex++) {
        if(board_[squareIndex].color() != currentTurn()) { // NOLINT allow [] in hot loop
            continue;
        }

        generatePseudoLegalMoves_(squareIndex, out);
    }
}

void Game::generateAllLegalMoves(MoveList& out) {
    for(int squareIndex = 0; squareIndex < Game::NUM_SQUARES; squareIndex++) {
        if(board_[squareIndex].color() != currentTurn()) { // NOLINT allow [] in hot loop
            continue;
        }

        generateLegalMoves(squareIndex, out);
    }
}

void Game::generatePseudoLegalMoves_(const int sourceSquare, MoveList& out) {
    switch(board_[sourceSquare].type()) {
        case PieceType::None: return;
        case PieceType::Pawn: generatePseudoLegalPawnMoves_(sourceSquare, out); return;
        case PieceType::Knight: generatePseudoLegalKnightMoves_(sourceSquare, out); return;
        case PieceType::Bishop: generatePseudoLegalBishopMoves_(sourceSquare, out); return;
        case PieceType::Rook: generatePseudoLegalRookMoves_(sourceSquare, out); return;
        case PieceType::Queen: generatePseudoLegalQueenMoves_(sourceSquare, out); return;
        case PieceType::King: generatePseudoLegalKingMoves_(sourceSquare, out); return;
    }
}

bool Game::isMoveLegal(const Move& move) {
    MoveList legalMoves;
    generateLegalMoves(move.sourceSquare(), legalMoves);

    for (int i = 0; i < legalMoves.size; i++) {
        if (legalMoves.data[i] == move) {
            return true;
        }
    }
    return false;
}


bool Game::tryMove(const Move& move) {
    const Piece sourcePiece = board_[move.sourceSquare()];
    // only allow current turn's player to make moves
    if(sourcePiece.color() != currentTurn_) {
        std::cerr << "[DEBUG] Attempted move: " + move.to_string(*this) + " is not legal because it is not the correct player's turn\n";
        return false;
    }

    if(!isMoveLegal(move)) {
        std::cerr << "[DEBUG] Attempted move: " + move.to_string(*this) + " is not legal per isMoveLegal\n";
        return false;
    }

    // move is legal, so make it
    makeMove(move);
    return true;
}

void Game::makeMove(const Move& move) {
    const Piece sourcePiece = board_[move.sourceSquare()];
    const bool isSourcePieceWhite = sourcePiece.color() == Color::White;
    // flip current turn
    currentTurn_ = oppositeColor(currentTurn_);
    // remove en passant (we may set it again later in this function)
    currentEnPassantSquare_ = UndoInfo::noEnPassant;

    // source piece's bitboard
    Bitboard& sourceBitboard = pieceToBitboard(sourcePiece);

    // update castling flags
    if(
        move.sourceSquare() == WHITE_KING_STARTING_SQUARE || move.sourceSquare() == WHITE_KINGSIDE_ROOK_STARTING_SQUARE || // moving white kingside pieces
        move.targetSquare() == WHITE_KINGSIDE_ROOK_STARTING_SQUARE // white kingside rook captured
    ) {
        castlingRights_.clearWhiteKingside();
    }
    if(
        move.sourceSquare() == WHITE_KING_STARTING_SQUARE || move.sourceSquare() == WHITE_QUEENSIDE_ROOK_STARTING_SQUARE || // moving white queenside pieces
        move.targetSquare() == WHITE_QUEENSIDE_ROOK_STARTING_SQUARE // white queenside rook captured
    ) {
        castlingRights_.clearWhiteQueenside();
    }
    if(
        move.sourceSquare() == BLACK_KING_STARTING_SQUARE || move.sourceSquare() == BLACK_KINGSIDE_ROOK_STARTING_SQUARE || // moving black kingside pieces
        move.targetSquare() == BLACK_KINGSIDE_ROOK_STARTING_SQUARE // black kingside rook captured
    ) {
        castlingRights_.clearBlackKingside();
    }
    if(
        move.sourceSquare() == BLACK_KING_STARTING_SQUARE || move.sourceSquare() == BLACK_QUEENSIDE_ROOK_STARTING_SQUARE || // moving black queenside pieces
        move.targetSquare() == BLACK_QUEENSIDE_ROOK_STARTING_SQUARE // black queenside rook captured
    ) {
        castlingRights_.clearBlackQueenside();
    }

    // update en passant flag
    if(move.isDoublePawn()) {
        const int sourceRow = getRow(move.sourceSquare());
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int passedRow = sourceRow + towardsCenter; // row that was passed in the double move  
        currentEnPassantSquare_ = getSquareIndex(getCol(move.sourceSquare()), passedRow); 
    }

    // If en passant capture, remove the captured pawn
    if (move.isEnPassant()) {
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int capturedIndex = move.targetSquare() - (towardsCenter * 8);

        // clear captured pawn's bitboard placement
        Bitboard& bbTargetPawns = pieceToBitboard(PieceType::Pawn, oppositeColor(sourcePiece.color()));
        bbTargetPawns.clearSquare(capturedIndex);

        // clear captured pawn from board
        board_[capturedIndex] = Piece{};
    }

    // If king side castle, also move the rook
    if(move.isKingSideCastle()) {
        const int kingsidePassingSquare = isSourcePieceWhite ? WHITE_KINGSIDE_PASSING_SQUARE : BLACK_KINGSIDE_PASSING_SQUARE;
        const int kingsideRookSquare = isSourcePieceWhite ? WHITE_KINGSIDE_ROOK_STARTING_SQUARE : BLACK_KINGSIDE_ROOK_STARTING_SQUARE;

        // set kingside rook's new position in bitboard and clear old position
        Bitboard& bbSourceRooks = pieceToBitboard(PieceType::Rook, sourcePiece.color());
        bbSourceRooks.setSquare(kingsidePassingSquare);
        bbSourceRooks.clearSquare(kingsideRookSquare);

        // also move rook
        board_[kingsidePassingSquare] = Piece{PieceType::Rook, sourcePiece.color()};
        board_[kingsideRookSquare] = Piece{};
    }

    // If queen side castle, also move the queen
    if(move.isQueenSideCastle()) {
        const int queensidePassingSquare = isSourcePieceWhite ? WHITE_QUEENSIDE_PASSING_SQUARE : BLACK_QUEENSIDE_PASSING_SQUARE;
        const int queensideRookSquare = isSourcePieceWhite ? WHITE_QUEENSIDE_ROOK_STARTING_SQUARE : BLACK_QUEENSIDE_ROOK_STARTING_SQUARE;

        // set kingside rook's new position in bitboard and clear old position
        Bitboard& bbSourceRooks = pieceToBitboard(PieceType::Rook, sourcePiece.color());
        bbSourceRooks.setSquare(queensidePassingSquare);
        bbSourceRooks.clearSquare(queensideRookSquare);

        // also move rook
        board_[queensidePassingSquare] = Piece{PieceType::Rook, sourcePiece.color()};
        board_[queensideRookSquare] = Piece{};;
    }

    // handle pawn promotion; different enough we need to return early
    if(move.isPromotion()) {
        const PieceType promotionType = Move::promotionToPieceType(move.promotion());

        // update promotion bitboard
        // remove pawn from pawn bitboard
        sourceBitboard.clearSquare(move.sourceSquare());
        // add promoted piece to promoted piece bitboard
        pieceToBitboard(promotionType, sourcePiece.color()).setSquare(move.targetSquare());

        if(move.isCapture()) {
            // remove captured piece from board
            Bitboard& targetBitboard = pieceToBitboard(board_[move.targetSquare()]);
            targetBitboard.clearSquare(move.targetSquare());
        }

        board_[move.targetSquare()] = Piece{promotionType, sourcePiece.color()};
        board_[move.sourceSquare()] = Piece{};
        return;
    }

    // update bitboard
    sourceBitboard.clearSquare(move.sourceSquare());
    sourceBitboard.setSquare(move.targetSquare());

    // handle capture
    if(move.isCapture() && !move.isEnPassant()) {
        Bitboard& targetBitboard = pieceToBitboard(board_[move.targetSquare()]);
        targetBitboard.clearSquare(move.targetSquare());
    }

    board_[move.targetSquare()] = sourcePiece;
    board_[move.sourceSquare()] = Piece{};

    // update king square cache(s)
    if(sourcePiece.type() == PieceType::King) {
        if(isSourcePieceWhite) {
            whiteKingSquare_ = move.targetSquare();
        } else {
            blackKingSquare_ = move.targetSquare();
        }
    }
}

void Game::undoMove(const Move& move, const UndoInfo& undoInfo) {
    // sourcePiece is now sitting at targetSquare
    const Piece sourcePiece = board_[move.targetSquare()];
    const bool isSourcePieceWhite = sourcePiece.color() == Color::White;
    // flip current turn
    currentTurn_ = oppositeColor(currentTurn_);

    // source piece's bitboard
    Bitboard& sourceBitboard = pieceToBitboard(sourcePiece);

    // handle king side castle
    if(move.isKingSideCastle()) {
        const int kingsidePassingSquare = isSourcePieceWhite ? WHITE_KINGSIDE_PASSING_SQUARE : BLACK_KINGSIDE_PASSING_SQUARE;
        const int kingsideRookSquare = isSourcePieceWhite ? WHITE_KINGSIDE_ROOK_STARTING_SQUARE : BLACK_KINGSIDE_ROOK_STARTING_SQUARE;

        Bitboard& bbSourceRooks = pieceToBitboard(PieceType::Rook, sourcePiece.color());
        bbSourceRooks.clearSquare(kingsidePassingSquare);
        bbSourceRooks.setSquare(kingsideRookSquare);

        // undo rook move
        board_[kingsidePassingSquare] = Piece{};
        board_[kingsideRookSquare] = Piece{PieceType::Rook, sourcePiece.color()};
    }

    // handle queen side castle
    if(move.isQueenSideCastle()) {
        const int queensidePassingSquare = isSourcePieceWhite ? WHITE_QUEENSIDE_PASSING_SQUARE : BLACK_QUEENSIDE_PASSING_SQUARE;
        const int queensideRookSquare = isSourcePieceWhite ? WHITE_QUEENSIDE_ROOK_STARTING_SQUARE : BLACK_QUEENSIDE_ROOK_STARTING_SQUARE;

        Bitboard& bbSourceRooks = pieceToBitboard(PieceType::Rook, sourcePiece.color());
        bbSourceRooks.clearSquare(queensidePassingSquare);
        bbSourceRooks.setSquare(queensideRookSquare);

        // undo rook move
        board_[queensidePassingSquare] = Piece{};
        board_[queensideRookSquare] = Piece{PieceType::Rook, sourcePiece.color()};
    }

    // handle en passant
    if(move.isEnPassant()) {
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int capturedIndex = move.targetSquare() - (towardsCenter * 8);

        // reset captured pawn's bitboard placement
        Bitboard& bbEnemyPawns = pieceToBitboard(PieceType::Pawn, oppositeColor(sourcePiece.color()));
        bbEnemyPawns.setSquare(capturedIndex);

        // replace captured pawn
        board_[capturedIndex] = Piece{PieceType::Pawn, oppositeColor(sourcePiece.color())};
    }

    // handle promotion; different enough that we need to return early
    if(move.isPromotion()) {
        // update promotion bitboard
        // re-add pawn to pawn bitboard
        pieceToBitboard(PieceType::Pawn, sourcePiece.color()).clearSquare(move.sourceSquare());
        // remove promoted piece from promoted piece bitboard
        sourceBitboard.clearSquare(move.targetSquare());

        if(move.isCapture()) {
            // re add captured piece to board
            Bitboard& capturedBitboard = pieceToBitboard(undoInfo.capturedPiece);
            capturedBitboard.setSquare(move.targetSquare());
        }

        // 'sourcePiece' is now a promoted piece instead of a pawn; we correct that
        board_[move.sourceSquare()] = Piece{PieceType::Pawn, sourcePiece.color()};
        board_[move.targetSquare()] = undoInfo.capturedPiece;
        return;
    }

    // update bitboard
    sourceBitboard.setSquare(move.sourceSquare());
    sourceBitboard.clearSquare(move.targetSquare());

    // handle capture
    if(move.isCapture() && !move.isEnPassant()) {
        Bitboard& capturedBitboard = pieceToBitboard(undoInfo.capturedPiece);
        capturedBitboard.setSquare(move.targetSquare());
    }

    // undo the general move
    board_[move.sourceSquare()] = sourcePiece;
    board_[move.targetSquare()] = undoInfo.capturedPiece;

    // restore all flags
    castlingRights_ = undoInfo.prevCastlingRights;
    currentEnPassantSquare_ = undoInfo.prevEnPassantSquare;
    whiteKingSquare_ = undoInfo.prevWhiteKingSquare;
    blackKingSquare_ = undoInfo.prevBlackKingSquare;
}

bool Game::isSquareAttacked(const int targetSquare, const Color attackingColor) const {
    const int targetRow = getRow(targetSquare);
    const int targetCol = getCol(targetSquare);

    // 1. Pawn attacks (only diagonals)
    // White attacks row - 1, Black attacks row + 1
    const int pawnDir = (attackingColor == Color::White) ? -1 : +1;
    const int attackingPawnRow = targetRow - pawnDir; // square where an attacking pawn would sit
    if (attackingPawnRow >= 0 && attackingPawnRow <= 7) {
        for (const int deltaColumn : {-1, +1}) {
            const int curCol = targetCol + deltaColumn;
            // out of range
            if (!onBoard(curCol, attackingPawnRow)) {
                continue;
            }

            const int attackingPawnSquare = getSquareIndex(curCol, attackingPawnRow);
            const Piece possibleAttackingPawn = board_[attackingPawnSquare];
            if (possibleAttackingPawn.type() == PieceType::Pawn && possibleAttackingPawn.color() == attackingColor) {
                return true;
            }
        }
    }

    // 2. Knight attacks
    for (int i = 0; i < 8; i++) {
        const int curCol = targetCol + knightDeltas[i][0];
        const int curRow = targetRow + knightDeltas[i][1];
        // out of bounds
        if (!onBoard(curCol, curRow)) {
            continue;
        }

        const int curSquare = getSquareIndex(curCol, curRow);
        const Piece possibleKnight = board_[curSquare];
        if (possibleKnight.type() == PieceType::Knight && possibleKnight.color() == attackingColor) {
            return true;
        }
    }

    // 3. King attacks
    for (int i = 0; i < 8; i++) {
        const int curCol = targetCol + kingDeltas[i][0];
        const int curRow = targetRow + kingDeltas[i][1];
        // out of bounds
        if (!onBoard(curCol, curRow)) {
            continue;
        }

        const int curSquare = getSquareIndex(curCol, curRow);
        const Piece possibleKing = board_[curSquare];
        if (possibleKing.type() == PieceType::King && possibleKing.color() == attackingColor) {
            return true;
        }
    }

    // The following loops are unrolled for speed
    // 4. Orthogonal (rook/queen)
    // North
    for (int curRow = targetRow + 1; curRow < 8; curRow++) {
        const Piece curPiece = board_[getSquareIndex(targetCol, curRow)];
        if (curPiece.exists()) {
            if (curPiece.color() == attackingColor &&
                (curPiece.type() == PieceType::Rook || curPiece.type() == PieceType::Queen)) {
                return true;
            }
            break; // blocked by some piece
        }
    }
    // South
    for (int curRow = targetRow - 1; curRow >= 0; curRow--) {
        const Piece curPiece = board_[getSquareIndex(targetCol, curRow)];
        if (curPiece.exists()) {
            if (curPiece.color() == attackingColor &&
                (curPiece.type() == PieceType::Rook || curPiece.type() == PieceType::Queen)) {
                return true;
            }
            break; // blocked by some piece
        }
    }
    // East
    for (int curCol = targetCol + 1; curCol < 8; curCol++) {
        const Piece curPiece = board_[getSquareIndex(curCol, targetRow)];
        if (curPiece.exists()) {
            if (curPiece.color() == attackingColor &&
                (curPiece.type() == PieceType::Rook || curPiece.type() == PieceType::Queen)) {
                return true;
            }
            break; // blocked by some piece
        }
    }
    // West
    for (int curCol = targetCol - 1; curCol >= 0; curCol--) {
        const Piece curPiece = board_[getSquareIndex(curCol, targetRow)];
        if (curPiece.exists()) {
            if (curPiece.color() == attackingColor &&
                (curPiece.type() == PieceType::Rook || curPiece.type() == PieceType::Queen)) {
                return true;
            }
            break; // blocked by some piece
        }
    }

    // 5. Diagonals (bishop/queen)
    // NE
    for (int curCol = targetCol + 1, curRow = targetRow + 1; curCol < 8 && curRow < 8; curCol++, curRow++) {
        const Piece curPiece = board_[getSquareIndex(curCol, curRow)];
        if (curPiece.exists()) {
            if (curPiece.color() == attackingColor &&
                (curPiece.type() == PieceType::Bishop || curPiece.type() == PieceType::Queen)) {
                return true;
            }
            break; // blocked by some piece
        }
    }
    // NW
    for (int curCol = targetCol - 1, curRow = targetRow + 1; curCol >= 0 && curRow < 8; curCol--, curRow++) {
        const Piece curPiece = board_[getSquareIndex(curCol, curRow)];
        if (curPiece.exists()) {
            if (curPiece.color() == attackingColor &&
                (curPiece.type() == PieceType::Bishop || curPiece.type() == PieceType::Queen)) {
                return true;
            }
            break; // blocked by some piece
        }
    }
    // SE
    for (int curCol = targetCol + 1, curRow = targetRow - 1; curCol < 8 && curRow >= 0; curCol++, curRow--) {
        const Piece curPiece = board_[getSquareIndex(curCol, curRow)];
        if (curPiece.exists()) {
            if (curPiece.color() == attackingColor &&
                (curPiece.type() == PieceType::Bishop || curPiece.type() == PieceType::Queen)) {
                return true;
            }
            break; // blocked by some piece
        }
    }
    // SW
    for (int curCol = targetCol - 1, curRow = targetRow - 1; curCol >= 0 && curRow >= 0; curCol--, curRow--) {
        const Piece curPiece = board_[getSquareIndex(curCol, curRow)];
        if (curPiece.exists()) {
            if (curPiece.color() == attackingColor &&
                (curPiece.type() == PieceType::Bishop || curPiece.type() == PieceType::Queen)) {
                return true;
            }
            break; // blocked by some piece
        }
    }

    // didn't find anything
    return false;
}

bool Game::isInCheck(const Color& colorToFind) const {
    // TODO: this throws no king on both sides
    const int kingSquare = findKingSquare(colorToFind).value();
    return isSquareAttacked(kingSquare, oppositeColor(colorToFind));
}

std::optional<int> Game::findKingSquare(const Color& colorToFind) const {
    return colorToFind == Color::White ? whiteKingSquare_ : blackKingSquare_;
}

std::string Game::intToAlgebraicNotation(const int square) {
    const int col = getCol(square);
    const std::string file = std::string{static_cast<char>('a' + col)};
    
    // reflect square over middle of chessboard
    const int row = 7 - getRow(square);
    const std::string rank = std::string{ static_cast<char>('1' + row)};

    return file + rank;
}

int Game::algebraicNotationToInt(const std::string& square) {
    // Assume length of exactly 2
    const char colC = square.at(0);
    const char rowC = square.at(1);
    
    const int col = colC - 'a';
    // reflect row over middle of chessboard
    const int row = 7 - (rowC - '1');

    return getSquareIndex(col, row);
}
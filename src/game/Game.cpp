#include <corecrt_wstring.h>
#include <iostream>
#include <string>

#include "Game.hpp"
#include "Bitboard.hpp"

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
        game.mailbox()[sourceSquare()].to_string_long() + " on " + Game::intToAlgebraicNotation(sourceSquare()) + " to " +
        game.mailbox()[targetSquare()].to_string_long() + " on " + Game::intToAlgebraicNotation(targetSquare())
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
    currentEnPassantSquare_{UndoInfo::noEnPassant} {
    initAttackBitboards_(); // any new game should init the attack bitboards
}

Color Game::currentTurn() const {
    return currentTurn_;
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
    for (const auto& piece : mailbox_) {
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
                // Numbers indicate n empty squares; we don't have to do anything except increase the piecePlacementIndex because bitboards have empty space by default
                const int numberEmptySquares = c-'0';
                piecePlacementIndex += numberEmptySquares;
                continue;
            }
            
            if(c == '/') {
                // '/' represents a new rank and can be ignored
                continue;
            }
            
            // We either have a proper piece or an invalid FEN
            const Piece newPiece = Piece::charToPiece(c);
            if(!newPiece.exists()) {
                std::cerr << "Unable to parse FEN: " << FEN << "\nInvalid char piece: " << "'" << c << "'";
            }

            // we have a valid piece, add it and update index
            mailbox_.at(piecePlacementIndex) = newPiece;
            Bitboard& pieceBitboard = pieceToBitboard(newPiece);
            Bitboard& colorBitboard = colorToOccupancyBitboard(newPiece.color());
            
            pieceBitboard.setSquare(piecePlacementIndex);
            colorBitboard.setSquare(piecePlacementIndex);

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
                default: std::cerr << "Unable to parse FEN: " << FEN << "\nInvalid current color: " << "'" << c << "'";
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
                default: std::cerr << "Unable to parse FEN: " << FEN << "\nInvalid castling char: " << "'" << c << "'";
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
    // if no legal moves for current turn then the game is over
    MoveList legalMoves;
    generateLegalMoves(legalMoves);
    return legalMoves.size == 0;
}

UndoInfo Game::getUndoInfo(const Piece capturedPiece) const {
    // pack bools into castling rights uint8_t for speedy lookup
    return UndoInfo{
        castlingRights_,
        static_cast<uint8_t>(currentEnPassantSquare_),
        capturedPiece
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

// This is slow, but that's okay because its only ran once per Game instance.
void Game::initAttackBitboards_() {
    for (int square = 0; square < Game::NUM_SQUARES; square++) {
        const int col = Game::getCol(square);
        const int row = Game::getRow(square);

        // Knight attacks
        Bitboard knightMoves{0};
        for (int i = 0; i < 8; ++i) {
            const int curCol = col + Game::knightDeltas[i][0];
            const int curRow = row + Game::knightDeltas[i][1];
            if (Game::onBoard(curCol, curRow)) {
                knightMoves.setSquare(Game::getSquareIndex(curCol, curRow));
            }
        }
        attackBitboards_.knightAttacks[square] = knightMoves;

        // King attacks
        Bitboard kingMoves{0};
        for (int i = 0; i < 8; ++i) {
            const int curCol = col + Game::kingDeltas[i][0];
            const int curRow = row + Game::kingDeltas[i][1];
            if (Game::onBoard(curCol, curRow)) {
                kingMoves.setSquare(Game::getSquareIndex(curCol, curRow));
            }
        }
        attackBitboards_.kingAttacks[square] = kingMoves;

        // Pawn FROM
        Bitboard whitePawnMoves{0};
        if (Game::onBoard(col - 1, row - 1)) {
            whitePawnMoves.setBit(Bitboard::bit(Game::getSquareIndex(col - 1, row - 1)));
        }
        if (Game::onBoard(col + 1, row - 1)) {
            whitePawnMoves.setBit(Bitboard::bit(Game::getSquareIndex(col + 1, row - 1)));
        }
        attackBitboards_.whitePawnAttacks[square] = whitePawnMoves;

        Bitboard blackPawnMoves{0};
        if (Game::onBoard(col - 1, row + 1)) {
            blackPawnMoves.setBit(Bitboard::bit(Game::getSquareIndex(col - 1, row + 1)));
        }
        if (Game::onBoard(col + 1, row + 1)) {
            blackPawnMoves.setBit(Bitboard::bit(Game::getSquareIndex(col + 1, row + 1)));
        }
        attackBitboards_.blackPawnAttacks[square] = blackPawnMoves;

    // Sliding attack helper function; this is slower, but it's only called once per game so it's okay
    // Note also this does not take blocking pieces into account
    const auto rayHits = [&](int dRow, int dCol) -> Bitboard {
        Bitboard attacks{0};

        int curRow = row + dRow;
        int curCol = col + dCol;
        while (onBoard(curCol, curRow)) {
            const int curSquare = getSquareIndex(curCol, curRow);

            attacks.setSquare(curSquare);
            
            // this direction is not blocked, continue
            curRow += dRow;
            curCol += dCol;
        }

        return attacks;
    };

    // 4. orthogonal rays: rook and queen
    const Bitboard south = rayHits(+1, 0);
    attackBitboards_.rookAttacks[square].mergeIn(south);
    attackBitboards_.queenAttacks[square].mergeIn(south);
    const Bitboard north = rayHits(-1, 0);
    attackBitboards_.rookAttacks[square].mergeIn(north);
    attackBitboards_.queenAttacks[square].mergeIn(north);
    const Bitboard east = rayHits(0, +1);
    attackBitboards_.rookAttacks[square].mergeIn(east);
    attackBitboards_.queenAttacks[square].mergeIn(east);
    const Bitboard west = rayHits(0, -1);
    attackBitboards_.rookAttacks[square].mergeIn(west);
    attackBitboards_.queenAttacks[square].mergeIn(west);

    // 5. diagonal rays: bishop and queen
    const Bitboard southEast = rayHits(+1, +1);
    attackBitboards_.bishopAttacks[square].mergeIn(southEast);
    attackBitboards_.queenAttacks[square].mergeIn(southEast);
    const Bitboard southWest = rayHits(+1, -1);
    attackBitboards_.bishopAttacks[square].mergeIn(southWest);
    attackBitboards_.queenAttacks[square].mergeIn(southWest);
    const Bitboard northEast = rayHits(-1, +1);
    attackBitboards_.bishopAttacks[square].mergeIn(northEast);
    attackBitboards_.queenAttacks[square].mergeIn(northEast);
    const Bitboard northWest = rayHits(-1, -1);
    attackBitboards_.bishopAttacks[square].mergeIn(northWest);
    attackBitboards_.queenAttacks[square].mergeIn(northWest);
    }
}

// TODO: this may be a slow point for move gen
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

// TODO: consider + profile: all const Bitboard -> const Bitboard&; uint64_t may be fast enough to copy that it is not worth it

void Game::generatePseudoLegalPawnMoves_(MoveList& out) {
    const bool isWhite = currentTurn_ == Color::White;

    Bitboard sourcePawns = isWhite ? bbWhitePawns_ : bbBlackPawns_;
    const Bitboard sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    // TODO: consider incrementally updated 'bbAllPieces_' here; its used rarely enough constructing from white + black pieces is probably fine
    const Bitboard emptySquares = bbWhitePieces_.merge(bbBlackPieces_).flip();
    if(isWhite) { // black and white pawns move differently
        // White

        // Normal moves
        // for white we shift down one row (8 squares) if it lands on an empty square
        const Bitboard oneRowPush = sourcePawns.rightShift(8).mask(emptySquares);
    
        Bitboard normal = oneRowPush;
        while(!normal.empty()) {
            const int targetSquare = normal.popLsb();
            addAllPawnPromotionsToMoves_(out, targetSquare+8, targetSquare, Piece{PieceType::Pawn, Color::White}, false);
        }

        // Double push
        // for white we shift down two rows (16 squares) if it lands on an empty square on the fourth rank
        // we shift from 'oneRowPush' to ensure both squares are empty
        Bitboard doublePush = oneRowPush.rightShift(8).mask(emptySquares).mask(Bitboard{Bitboard::Rank4});
        while(!doublePush.empty()) {
            const int targetSquare = doublePush.popLsb();
            // double push can never be a promotion, so we don't need to call addAllPawnPromotionsToMoves_ here
            out.push_back(Move{targetSquare+16, targetSquare, MoveFlag::DoublePawnPush, Promotion::None});
        }

        // En Passant
        if (currentEnPassantSquare_ != UndoInfo::noEnPassant) {
            // we check black pawn attack pattern because pawn moves are not symmetrical
            Bitboard attackers = bbWhitePawns_.mask(attackBitboards_.blackPawnAttacks[currentEnPassantSquare_]);

            while (!attackers.empty()) {
                const int from = attackers.popLsb();
                out.push_back(Move{from, currentEnPassantSquare_, MoveFlag::EnPassant, Promotion::None});
            }
        }

        // we can now mutate sourcePawns because we're done with the constant operations
        while(!sourcePawns.empty()) {
            const int sourceSquare = sourcePawns.popLsb();

            // Normal capture
            const Bitboard captureAttacks = attackBitboards_.whitePawnAttacks[sourceSquare].mask(sourcePieces.flip());
            Bitboard captures = captureAttacks.mask(targetPieces); // attacks that land on target pieces
            while(!captures.empty()) {
                const int targetSquare = captures.popLsb();
                addAllPawnPromotionsToMoves_(out, sourceSquare, targetSquare, Piece{PieceType::Pawn, Color::White}, true);
            }
        }
    } else {
        // Black

        // Normal moves
        // for black we shift up one row (8 squares) if it lands on an empty square
        const Bitboard oneRowPush = sourcePawns.leftShift(8).mask(emptySquares);
    
        Bitboard normal = oneRowPush;
        while(!normal.empty()) {
            const int targetSquare = normal.popLsb();
            addAllPawnPromotionsToMoves_(out, targetSquare-8, targetSquare, Piece{PieceType::Pawn, Color::Black}, false);
        }

        // Double push
        // for black we shift up two rows (16 squares) if it lands on an empty square on the fifth rank
        // we shift from 'oneRowPush' to ensure both squares are empty
        Bitboard doublePush = oneRowPush.leftShift(8).mask(emptySquares).mask(Bitboard{Bitboard::Rank5});
        while(!doublePush.empty()) {
            const int targetSquare = doublePush.popLsb();
            // double push can never be a promotion, so we don't need to call addAllPawnPromotionsToMoves_ here
            out.push_back(Move{targetSquare-16, targetSquare, MoveFlag::DoublePawnPush, Promotion::None});
        }

        // En Passant
        if (currentEnPassantSquare_ != UndoInfo::noEnPassant) {
            // we check white pawn attack pattern because pawn moves are not symmetrical
            Bitboard attackers = bbBlackPawns_.mask(attackBitboards_.whitePawnAttacks[currentEnPassantSquare_]);

            while (!attackers.empty()) {
                const int from = attackers.popLsb();
                out.push_back(Move{from, currentEnPassantSquare_, MoveFlag::EnPassant, Promotion::None});
            }
        }

        // we can now mutate sourcePawns because we're done with the constant operations
        while(!sourcePawns.empty()) {
            const int sourceSquare = sourcePawns.popLsb();

            // Normal capture
            const Bitboard captureAttacks = attackBitboards_.blackPawnAttacks[sourceSquare].mask(sourcePieces.flip());
            Bitboard captures = captureAttacks.mask(targetPieces); // attacks that land on target pieces
            while(!captures.empty()) {
                const int targetSquare = captures.popLsb();
                addAllPawnPromotionsToMoves_(out, sourceSquare, targetSquare, Piece{PieceType::Pawn, Color::Black}, true);
            }
        }
    }
}


void Game::generatePseudoLegalKnightMoves_(MoveList& out) {
    const bool isWhite = currentTurn_ == Color::White;

    Bitboard sourceKnights = isWhite ? bbWhiteKnights_ : bbBlackKnights_;
    const Bitboard sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    while(!sourceKnights.empty()) {
        const int sourceSquare = sourceKnights.popLsb();
        const Bitboard attacks = attackBitboards_.knightAttacks[sourceSquare].mask(sourcePieces.flip()); // can not attack own pieces

        // Normal moves (non-captures)
        Bitboard normal = attacks.mask(targetPieces.flip()); // attacks that do not land on target pieces
        while(!normal.empty()) {
            const int targetSquare = normal.popLsb();
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // Capture moves
        Bitboard captures = attacks.mask(targetPieces); // attacks that land on target pieces
        while(!captures.empty()) {
            const int targetSquare = captures.popLsb();
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
        }
    }
}

// TODO: implement magic bitboards

void Game::generatePseudoLegalBishopMoves_(MoveList& out) {
    const bool isWhite = currentTurn_ == Color::White;

    Bitboard sourceBishops = isWhite ? bbWhiteBishops_ : bbBlackBishops_;
    const Bitboard sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    while (!sourceBishops.empty()) {
        const int sourceSquare = sourceBishops.popLsb();

        // ---- NE (+9): stop when we hit H-file (col becomes 0 after wrap)
        for (int targetSquare = sourceSquare + 9; targetSquare <= 63 && getCol(targetSquare) != 0; targetSquare += 9) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- NW (+7): stop when we hit A-file (col becomes 7 after wrap)
        for (int targetSquare = sourceSquare + 7; targetSquare <= 63 && getCol(targetSquare) != 7; targetSquare += 7) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- SE (-7): stop when we hit H-file (col becomes 0 after wrap)
        for (int targetSquare = sourceSquare - 7; targetSquare >= 0 && getCol(targetSquare) != 0; targetSquare -= 7) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- SW (-9): stop when we hit A-file (col becomes 7 after wrap)
        for (int targetSquare = sourceSquare - 9; targetSquare >= 0 && getCol(targetSquare) != 7; targetSquare -= 9) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }
    }
}


void Game::generatePseudoLegalRookMoves_(MoveList& out) {
    const bool isWhite = currentTurn_ == Color::White;

    Bitboard sourceRooks = isWhite ? bbWhiteRooks_ : bbBlackRooks_;
    const Bitboard sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    while (!sourceRooks.empty()) {
        const int sourceSquare = sourceRooks.popLsb();

        // ---- North (+8)
        for (int targetSquare = sourceSquare + 8; targetSquare <= 63; targetSquare += 8) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- South (-8)
        for (int targetSquare = sourceSquare - 8; targetSquare >= 0; targetSquare -= 8) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- East (+1) stop at H-file
        for (int targetSquare = sourceSquare + 1; targetSquare <= 63 && getCol(targetSquare) != 0; ++targetSquare) {
            // getCol(targetSquare) != 0 prevents wrap H->A because when you go from 7 to 8, file becomes 0
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- West (-1) stop at A-file
        for (int targetSquare = sourceSquare - 1; targetSquare >= 0 && getCol(targetSquare) != 7; targetSquare--) {
            // getCol(targetSquare) != 7 prevents wrap A->H because when you go from 8 to 7, file becomes 7
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) {
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }
    }
}


void Game::generatePseudoLegalQueenMoves_(MoveList& out) {
    const bool isWhite = currentTurn_ == Color::White;

    Bitboard sourceQueens = isWhite ? bbWhiteQueens_ : bbBlackQueens_;
    const Bitboard sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    while (!sourceQueens.empty()) {
        const int sourceSquare = sourceQueens.popLsb();

        // ---- Rook Moves ----
        // ---- North (+8)
        for (int targetSquare = sourceSquare + 8; targetSquare < NUM_SQUARES; targetSquare += 8) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- South (-8)
        for (int targetSquare = sourceSquare - 8; targetSquare >= 0; targetSquare -= 8) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- East (+1) stop at H-file
        for (int targetSquare = sourceSquare + 1; targetSquare <= 63 && getCol(targetSquare) != 0; ++targetSquare) {
            // getCol(targetSquare) != 0 prevents wrap H->A because when you go from 7 to 8, file becomes 0
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- West (-1) stop at A-file
        for (int targetSquare = sourceSquare - 1; targetSquare >= 0 && getCol(targetSquare) != 7; --targetSquare) {
            // getCol(targetSquare) != 7 prevents wrap A->H because when you go from 8 to 7, file becomes 7
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- Bishop Moves ----
        // ---- NE (+9): stop when we hit H-file (col becomes 0 after wrap)
        for (int targetSquare = sourceSquare + 9; targetSquare <= 63 && getCol(targetSquare) != 0; targetSquare += 9) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- NW (+7): stop when we hit A-file (col becomes 7 after wrap)
        for (int targetSquare = sourceSquare + 7; targetSquare <= 63 && getCol(targetSquare) != 7; targetSquare += 7) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- SE (-7): stop when we hit H-file (col becomes 0 after wrap)
        for (int targetSquare = sourceSquare - 7; targetSquare >= 0 && getCol(targetSquare) != 0; targetSquare -= 7) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // ---- SW (-9): stop when we hit A-file (col becomes 7 after wrap)
        for (int targetSquare = sourceSquare - 9; targetSquare >= 0 && getCol(targetSquare) != 7; targetSquare -= 9) {
            // hit our own piece, stop
            if (sourcePieces.containsSquare(targetSquare)) {
                break;
            }

            // hit an enemy piece, add it and stop
            if (targetPieces.containsSquare(targetSquare)) { 
                out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
                break;
            }

            // add move and continue
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }
    }
}

void Game::generatePseudoLegalKingMoves_(MoveList& out) {
    const bool isWhite = (currentTurn_ == Color::White);

    Bitboard sourceKings = isWhite ? bbWhiteKing_ : bbBlackKing_;
    const Bitboard sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;
    const Bitboard allPieces = bbWhitePieces_.merge(bbBlackPieces_);

    // TODO: we assume just one king anyway so we can probably remove the loop, but it's probably fine for readability
    while(!sourceKings.empty()) {
        const int sourceSquare = sourceKings.popLsb();
        const Bitboard attacks = attackBitboards_.kingAttacks[sourceSquare].mask(sourcePieces.flip()); // can not attack own pieces

        // Normal moves (non-captures)
        Bitboard normal = attacks.mask(targetPieces.flip()); // attacks that do not land on target pieces
        while(!normal.empty()) {
            const int targetSquare = normal.popLsb();
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Normal, Promotion::None});
        }

        // Capture moves
        Bitboard captures = attacks.mask(targetPieces); // attacks that land on target pieces
        while(!captures.empty()) {
            const int targetSquare = captures.popLsb();
            out.push_back(Move{sourceSquare, targetSquare, MoveFlag::Capture, Promotion::None});
        }

        // Castling
        const int kingStartingSquare = isWhite ? WHITE_KING_STARTING_SQUARE : BLACK_KING_STARTING_SQUARE;

        const int kingsidePassingSquare = isWhite ? WHITE_KINGSIDE_PASSING_SQUARE : BLACK_KINGSIDE_PASSING_SQUARE;
        const int kingsideTargetSquare = isWhite ? WHITE_KINGSIDE_TARGET_SQUARE : BLACK_KINGSIDE_TARGET_SQUARE;

        const int queensidePassingSquare = isWhite ? WHITE_QUEENSIDE_PASSING_SQUARE : BLACK_QUEENSIDE_PASSING_SQUARE;
        const int queensideTargetSquare = isWhite ? WHITE_QUEENSIDE_TARGET_SQUARE : BLACK_QUEENSIDE_TARGET_SQUARE;

        const bool canKingside = isWhite ? castlingRights_.canWhiteKingside() : castlingRights_.canBlackKingside();
        const bool canQueenside = isWhite ? castlingRights_.canWhiteQueenside() : castlingRights_.canBlackQueenside();

        // TODO: bitwise masks instead of allPieces.containsSquare(...)
        // King side castling
        if(
            canKingside &&
            sourceSquare == kingStartingSquare &&
            !allPieces.containsSquare(kingsidePassingSquare) &&  // passing square does not contain a piece 
            !allPieces.containsSquare(kingsideTargetSquare)      // target square does not contain a piece
        ) {
            out.push_back(Move{sourceSquare, kingsideTargetSquare, MoveFlag::KingCastle, Promotion::None});
        }

        // Queen side castling
        if(
            canQueenside &&
            sourceSquare == kingStartingSquare &&
            !allPieces.containsSquare(queensidePassingSquare) &&      // passing square does not contain a piece
            !allPieces.containsSquare(queensidePassingSquare - 2) &&  // queenside has two passing squares
            !allPieces.containsSquare(queensideTargetSquare)          // target square does not contain a piece
        ) {
            out.push_back(Move{sourceSquare, queensideTargetSquare, MoveFlag::QueenCastle, Promotion::None});
        }
    }
}

void Game::generateLegalMoves(MoveList& out) {
    // TODO: eventually make generateLegalMoves const by finding a workaround other than simply undoing moves
    MoveList pseudoMoves;
    generatePseudoLegalMoves(pseudoMoves);

    // only allow moves that do not leave king in check
    for(int i = 0; i < pseudoMoves.size; i++) {
        const Move move = pseudoMoves.data[i];

        const Piece sourcePiece = mailbox_[move.sourceSquare()];
        const Color moveColor = sourcePiece.color();
        const Color enemyColor = oppositeColor(moveColor);
        const bool isSourceWhite = moveColor == Color::White;

        // save info to pass to undo move
        const UndoInfo undoInfo = getUndoInfo(mailbox_[move.targetSquare()]);

        makeMove(move);

        // can't cause king to be in check
        if(isInCheck(moveColor)) {
            undoMove(move, undoInfo);
            continue;
        }
        
        // castling legality rules
        const int kingStartingSquare = isSourceWhite ? Game::WHITE_KING_STARTING_SQUARE : Game::BLACK_KING_STARTING_SQUARE;

        if(move.isKingSideCastle()) {
            const int kingsidePassingSquare = isSourceWhite ? Game::WHITE_KINGSIDE_PASSING_SQUARE : Game::BLACK_KINGSIDE_PASSING_SQUARE;
            const int kingsideTargetSquare = isSourceWhite ? Game::WHITE_KINGSIDE_TARGET_SQUARE : Game::BLACK_KINGSIDE_TARGET_SQUARE;
    
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
            const int queensidePassingSquare = isSourceWhite ? Game::WHITE_QUEENSIDE_PASSING_SQUARE : Game::BLACK_QUEENSIDE_PASSING_SQUARE;
            const int queensideTargetSquare = isSourceWhite ? Game::WHITE_QUEENSIDE_TARGET_SQUARE : Game::BLACK_QUEENSIDE_TARGET_SQUARE;
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

void Game::generateLegalMovesFromSquare(int sourceSquare, MoveList& out) {
    MoveList legalMoves;
    generateLegalMoves(legalMoves);

    for(int i = 0; i < legalMoves.size; i++) {
        const Move move = legalMoves.data[i];
        if(move.sourceSquare() == sourceSquare) {
            out.push_back(move);
        }
    }
}

void Game::generatePseudoLegalMoves(MoveList& out) {
    generatePseudoLegalPawnMoves_(out);
    generatePseudoLegalKnightMoves_(out);
    generatePseudoLegalBishopMoves_(out);
    generatePseudoLegalRookMoves_(out);
    generatePseudoLegalQueenMoves_(out);
    generatePseudoLegalKingMoves_(out);
}

bool Game::isMoveLegal(const Move& move) {
    MoveList legalMoves;
    generateLegalMoves(legalMoves);

    for (int i = 0; i < legalMoves.size; i++) {
        if (legalMoves.data[i] == move) {
            return true;
        }
    }
    return false;
}


bool Game::tryMove(const Move& move) {
    // TODO: consider making isWhite() function in Game; this logic is repeated quite often
    const bool isWhite = currentTurn_ == Color::White;
    const bool sourceSquareHasWhitePiece = bbWhitePieces_.containsSquare(move.sourceSquare());
    const bool sourceSquareHasBlackPiece = bbBlackPieces_.containsSquare(move.sourceSquare());
    // only allow current turn's player to make moves
    if((isWhite && !sourceSquareHasWhitePiece) && (!isWhite && !sourceSquareHasBlackPiece)) {
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
    const Piece sourcePiece = mailbox_[move.sourceSquare()];
    const bool isSourcePieceWhite = sourcePiece.color() == Color::White;
    // flip current turn
    currentTurn_ = oppositeColor(currentTurn_);
    // remove en passant (we may set it again later in this function)
    currentEnPassantSquare_ = UndoInfo::noEnPassant;

    Bitboard& sourceBitboard = pieceToBitboard(sourcePiece);
    Bitboard& sourceColorBitboard = colorToOccupancyBitboard(sourcePiece.color());
    Bitboard& targetColorBitboard = colorToOccupancyBitboard(oppositeColor(sourcePiece.color()));

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

        // update occupancy board
        targetColorBitboard.clearSquare(capturedIndex);

        // clear captured pawn from mailbox
        mailbox_[capturedIndex] = Piece{};
    }

    // If king side castle, also move the rook
    if(move.isKingSideCastle()) {
        const int kingsidePassingSquare = isSourcePieceWhite ? WHITE_KINGSIDE_PASSING_SQUARE : BLACK_KINGSIDE_PASSING_SQUARE;
        const int kingsideRookSquare = isSourcePieceWhite ? WHITE_KINGSIDE_ROOK_STARTING_SQUARE : BLACK_KINGSIDE_ROOK_STARTING_SQUARE;

        // set kingside rook's new position in bitboard and clear old position
        Bitboard& bbSourceRooks = pieceToBitboard(PieceType::Rook, sourcePiece.color());
        bbSourceRooks.setSquare(kingsidePassingSquare);
        bbSourceRooks.clearSquare(kingsideRookSquare);

        // update occupancy board
        sourceColorBitboard.setSquare(kingsidePassingSquare);
        sourceColorBitboard.clearSquare(kingsideRookSquare);

        // also move rook in mailbox
        mailbox_[kingsidePassingSquare] = Piece{PieceType::Rook, sourcePiece.color()};
        mailbox_[kingsideRookSquare] = Piece{};
    }

    // If queen side castle, also move the queen
    if(move.isQueenSideCastle()) {
        const int queensidePassingSquare = isSourcePieceWhite ? WHITE_QUEENSIDE_PASSING_SQUARE : BLACK_QUEENSIDE_PASSING_SQUARE;
        const int queensideRookSquare = isSourcePieceWhite ? WHITE_QUEENSIDE_ROOK_STARTING_SQUARE : BLACK_QUEENSIDE_ROOK_STARTING_SQUARE;

        // set kingside rook's new position in bitboard and clear old position
        Bitboard& bbSourceRooks = pieceToBitboard(PieceType::Rook, sourcePiece.color());
        bbSourceRooks.setSquare(queensidePassingSquare);
        bbSourceRooks.clearSquare(queensideRookSquare);

        sourceColorBitboard.setSquare(queensidePassingSquare);
        sourceColorBitboard.clearSquare(queensideRookSquare);

        // also move rook in mailbox
        mailbox_[queensidePassingSquare] = Piece{PieceType::Rook, sourcePiece.color()};
        mailbox_[queensideRookSquare] = Piece{};;
    }

    // handle pawn promotion; different enough we need to return early
    if(move.isPromotion()) {
        const PieceType promotionType = Move::promotionToPieceType(move.promotion());

        // update promotion bitboard
        // remove pawn from pawn bitboard
        sourceBitboard.clearSquare(move.sourceSquare());
        // add promoted piece to promoted piece bitboard
        pieceToBitboard(promotionType, sourcePiece.color()).setSquare(move.targetSquare());

        // update occupancy board
        sourceColorBitboard.clearSquare(move.sourceSquare());
        sourceColorBitboard.setSquare(move.targetSquare());

        if(move.isCapture()) {
            Bitboard& targetBitboard = pieceToBitboard(mailbox_[move.targetSquare()]);
            targetBitboard.clearSquare(move.targetSquare());

            // update target occupancy board
            targetColorBitboard.clearSquare(move.targetSquare());
        }

        // update mailbox
        mailbox_[move.targetSquare()] = Piece{promotionType, sourcePiece.color()};
        mailbox_[move.sourceSquare()] = Piece{};
        return;
    }

    // update bitboards
    sourceBitboard.clearSquare(move.sourceSquare());
    sourceBitboard.setSquare(move.targetSquare());

    // update occupancy bitboard
    sourceColorBitboard.clearSquare(move.sourceSquare());
    sourceColorBitboard.setSquare(move.targetSquare());

    // handle capture
    if(move.isCapture() && !move.isEnPassant()) { // en passant already handles capture separately
        Bitboard& targetBitboard = pieceToBitboard(mailbox_[move.targetSquare()]);
        targetBitboard.clearSquare(move.targetSquare());

        // update occupancy bitboard
        targetColorBitboard.clearSquare(move.targetSquare());
    }

    // update mailbox
    mailbox_[move.targetSquare()] = sourcePiece;
    mailbox_[move.sourceSquare()] = Piece{};
}

void Game::undoMove(const Move& move, const UndoInfo& undoInfo) {
    // sourcePiece is now sitting at targetSquare
    const Piece sourcePiece = mailbox_[move.targetSquare()];
    const bool isSourcePieceWhite = sourcePiece.color() == Color::White;
    // flip current turn
    currentTurn_ = oppositeColor(currentTurn_);

    // source piece's bitboard
    Bitboard& sourceBitboard = pieceToBitboard(sourcePiece);
    Bitboard& sourceColorBitboard = colorToOccupancyBitboard(sourcePiece.color());
    Bitboard& targetColorBitboard = colorToOccupancyBitboard(oppositeColor(sourcePiece.color()));

    // handle king side castle
    if(move.isKingSideCastle()) {
        const int kingsidePassingSquare = isSourcePieceWhite ? WHITE_KINGSIDE_PASSING_SQUARE : BLACK_KINGSIDE_PASSING_SQUARE;
        const int kingsideRookSquare = isSourcePieceWhite ? WHITE_KINGSIDE_ROOK_STARTING_SQUARE : BLACK_KINGSIDE_ROOK_STARTING_SQUARE;

        // undo rook move in bitboard
        Bitboard& bbSourceRooks = pieceToBitboard(PieceType::Rook, sourcePiece.color());
        bbSourceRooks.clearSquare(kingsidePassingSquare);
        bbSourceRooks.setSquare(kingsideRookSquare);

        // update occupancy bitboard
        sourceColorBitboard.clearSquare(kingsidePassingSquare);
        sourceColorBitboard.setSquare(kingsideRookSquare);

        // undo rook move in mailbox
        mailbox_[kingsidePassingSquare] = Piece{};
        mailbox_[kingsideRookSquare] = Piece{PieceType::Rook, sourcePiece.color()};
    }

    // handle queen side castle
    if(move.isQueenSideCastle()) {
        const int queensidePassingSquare = isSourcePieceWhite ? WHITE_QUEENSIDE_PASSING_SQUARE : BLACK_QUEENSIDE_PASSING_SQUARE;
        const int queensideRookSquare = isSourcePieceWhite ? WHITE_QUEENSIDE_ROOK_STARTING_SQUARE : BLACK_QUEENSIDE_ROOK_STARTING_SQUARE;

        Bitboard& bbSourceRooks = pieceToBitboard(PieceType::Rook, sourcePiece.color());
        bbSourceRooks.clearSquare(queensidePassingSquare);
        bbSourceRooks.setSquare(queensideRookSquare);

        // update occupancy bitboard
        sourceColorBitboard.clearSquare(queensidePassingSquare);
        sourceColorBitboard.setSquare(queensideRookSquare);

        // undo rook move
        mailbox_[queensidePassingSquare] = Piece{};
        mailbox_[queensideRookSquare] = Piece{PieceType::Rook, sourcePiece.color()};
    }

    // handle en passant
    if(move.isEnPassant()) {
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int capturedIndex = move.targetSquare() - (towardsCenter * 8);

        // reset captured pawn's bitboard placement
        Bitboard& bbEnemyPawns = pieceToBitboard(PieceType::Pawn, oppositeColor(sourcePiece.color()));
        bbEnemyPawns.setSquare(capturedIndex);

        // update occupancy bitboard
        targetColorBitboard.setSquare(capturedIndex);

        // replace captured pawn
        mailbox_[capturedIndex] = Piece{PieceType::Pawn, oppositeColor(sourcePiece.color())};
    }

    // handle promotion; different enough that we need to return early
    if(move.isPromotion()) {
        // update promotion bitboard
        // re-add pawn to pawn bitboard
        pieceToBitboard(PieceType::Pawn, sourcePiece.color()).setSquare(move.sourceSquare());
        // remove promoted piece from promoted piece bitboard
        sourceBitboard.clearSquare(move.targetSquare());

        // update occupancy bitboard
        // re-add pawn
        sourceColorBitboard.setSquare(move.sourceSquare());
        // remove promoted piece
        sourceColorBitboard.clearSquare(move.targetSquare());

        if(move.isCapture()) {
            // re add captured piece to board
            Bitboard& capturedBitboard = pieceToBitboard(undoInfo.capturedPiece);
            capturedBitboard.setSquare(move.targetSquare());

            // update occupancy bitboard
            targetColorBitboard.setSquare(move.targetSquare());
        }

        // 'sourcePiece' is now a promoted piece instead of a pawn; we correct that
        mailbox_[move.sourceSquare()] = Piece{PieceType::Pawn, sourcePiece.color()};
        mailbox_[move.targetSquare()] = undoInfo.capturedPiece;
        return;
    }

    // update bitboard
    sourceBitboard.setSquare(move.sourceSquare());
    sourceBitboard.clearSquare(move.targetSquare());

    // update occupancy bitboard
    sourceColorBitboard.setSquare(move.sourceSquare());
    sourceColorBitboard.clearSquare(move.targetSquare());

    // handle capture
    if(move.isCapture() && !move.isEnPassant()) {
        Bitboard& capturedBitboard = pieceToBitboard(undoInfo.capturedPiece);
        capturedBitboard.setSquare(move.targetSquare());

        targetColorBitboard.setSquare(move.targetSquare());
    }

    // undo the general move
    mailbox_[move.sourceSquare()] = sourcePiece;
    mailbox_[move.targetSquare()] = undoInfo.capturedPiece;

    // restore all flags
    castlingRights_ = undoInfo.prevCastlingRights;
    currentEnPassantSquare_ = undoInfo.prevEnPassantSquare;
}

bool Game::isSquareAttacked(const int targetSquare, const Color attackingColor) const {
    const bool isWhiteAttacking = attackingColor == Color::White;
    const Bitboard allPieces = bbWhitePieces_.merge(bbBlackPieces_);
    // we compute "is attackingColor attacking targetSquare"
    // Pawns -- since pawn moves are not symmetric we use the opposite color's attacking bitboard
    const Bitboard attackingPawns = isWhiteAttacking ? bbWhitePawns_ : bbBlackPawns_;
    const std::array<Bitboard, 64> attackingPawnsMap = isWhiteAttacking ? attackBitboards_.blackPawnAttacks : attackBitboards_.whitePawnAttacks;
    if(!attackingPawns.mask(attackingPawnsMap[targetSquare]).empty()) {
        return true;
    }

    // Knights -- is there an attacking knight sitting a knights move away from targetSquare
    const Bitboard attackingKnights = isWhiteAttacking ? bbWhiteKnights_ : bbBlackKnights_;
    if(!attackingKnights.mask(attackBitboards_.knightAttacks[targetSquare]).empty()) {
        return true;
    }

    // Kings -- is there an attacking king sitting a kings move away from targetSquare
    const Bitboard attackingKings = isWhiteAttacking ? bbWhiteKing_ : bbBlackKing_;
    if(!attackingKings.mask(attackBitboards_.kingAttacks[targetSquare]).empty()) {
        return true;
    }

    // Sliding pieces -- TODO: implement magic bitboards
    const Bitboard attackingRooks = isWhiteAttacking ? bbWhiteRooks_ : bbBlackRooks_;
    const Bitboard attackingBishops = isWhiteAttacking ? bbWhiteBishops_ : bbBlackBishops_;
    const Bitboard attackingQueens = isWhiteAttacking ? bbWhiteQueens_ : bbBlackQueens_;
    // Orthogonal, rook / queen
    const Bitboard rookLike = attackingRooks.merge(attackingQueens);
    // North (+8)
    for (int curSquare = targetSquare + 8; curSquare <= 63; curSquare += 8) {
        // if we hit the correct piece, we are done, the square is attacked
        if(rookLike.containsSquare(curSquare)) {
            return true;
        }

        // if we hit the wrong piece after, we are done for this direction
        if(allPieces.containsSquare(curSquare)) {
            break;
        }
    }

    // South (-8)
    for (int curSquare = targetSquare - 8; curSquare >= 0; curSquare -= 8) {
        // if we hit the correct piece, we are done, the square is attacked
        if(rookLike.containsSquare(curSquare)) {
            return true;
        }

        // if we hit the wrong piece after, we are done for this direction
        if(allPieces.containsSquare(curSquare)) {
            break;
        }
    }

    // East (+1) until file wraps (when file becomes 0 after H->A wrap)
    for (int curSquare = targetSquare + 1; curSquare <= 63 && getCol(curSquare) != 0; curSquare++) {
        // if we hit the correct piece, we are done, the square is attacked
        if(rookLike.containsSquare(curSquare)) {
            return true;
        }

        // if we hit the wrong piece after, we are done for this direction
        if(allPieces.containsSquare(curSquare)) {
            break;
        }
    }

    // West (-1) until file wraps (when file becomes 7 after A->H wrap)
    for (int curSquare = targetSquare - 1; curSquare >= 0 && getCol(curSquare) != 7; curSquare--) {
        // if we hit the correct piece, we are done, the square is attacked
        if(rookLike.containsSquare(curSquare)) {
            return true;
        }

        // if we hit the wrong piece after, we are done for this direction
        if(allPieces.containsSquare(curSquare)) {
            break;
        }
    }

    // Diagonal, bishop / queen
    const Bitboard bishopLike = attackingBishops.merge(attackingQueens);
    // NE (+9) stop at H-file wrap (file becomes 0)
    for (int curSquare = targetSquare + 9; curSquare <= 63 && getCol(curSquare) != 0; curSquare += 9) {
        // if we hit the correct piece, we are done, the square is attacked
        if(bishopLike.containsSquare(curSquare)) {
            return true;
        }

        // if we hit the wrong piece after, we are done for this direction
        if(allPieces.containsSquare(curSquare)) {
            break;
        }
    }

    // NW (+7) stop at A-file wrap (file becomes 7)
    for (int curSquare = targetSquare + 7; curSquare <= 63 && getCol(curSquare) != 7; curSquare += 7) {
        // if we hit the correct piece, we are done, the square is attacked
        if(bishopLike.containsSquare(curSquare)) {
            return true;
        }

        // if we hit the wrong piece after, we are done for this direction
        if(allPieces.containsSquare(curSquare)) {
            break;
        }
    }

    // SE (-7) stop at H-file wrap (file becomes 0)
    for (int curSquare = targetSquare - 7; curSquare >= 0 && getCol(curSquare) != 0; curSquare -= 7) {
        // if we hit the correct piece, we are done, the square is attacked
        if(bishopLike.containsSquare(curSquare)) {
            return true;
        }

        // if we hit the wrong piece after, we are done for this direction
        if(allPieces.containsSquare(curSquare)) {
            break;
        }
    }

    // SW (-9) stop at A-file wrap (file becomes 7)
    for (int curSquare = targetSquare - 9; curSquare >= 0 && getCol(curSquare) != 7; curSquare -= 9) {
        // if we hit the correct piece, we are done, the square is attacked
        if(bishopLike.containsSquare(curSquare)) {
            return true;
        }

        // if we hit the wrong piece after, we are done for this direction
        if(allPieces.containsSquare(curSquare)) {
            break;
        }
    }

    return false;
}

bool Game::isInCheck(const Color& colorToFind) const {
    // TODO: this has undefined behavior if no kings on both sides
    const int kingSquare = findKingSquare(colorToFind);
    return isSquareAttacked(kingSquare, oppositeColor(colorToFind));
}

int Game::findKingSquare(const Color& colorToFind) const {
    Bitboard bbKing = colorToFind == Color::White ? bbWhiteKing_ : bbBlackKing_;
    return bbKing.popLsb();
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
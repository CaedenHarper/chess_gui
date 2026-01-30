#include <iostream>
#include <string>

#include "Game.hpp"
#include "Utils.hpp"

Game::Game()
    : sideToMove_{Color::White},
    castlingRights_{0},
    enPassantSquare_{UndoInfo::noEnPassant} {
    // Init lookup tables
    initAttackBitboards_();
    initPieceToBBTable_();
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
        if(Utils::getCol(squareIndex) == 0) {
            out += std::to_string(Utils::BOARD_HEIGHT - Utils::getRow(squareIndex)) + " |";
        }
    
        if(piece.exists()) {
            out += piece.to_string_short() + "|";
        } else {
            // if piece is none, print square color
            // square color is white if the col polarity matches row polarity
            out += Utils::getCol(squareIndex) % 2 == Utils::getRow(squareIndex) % 2 ? " |" : "#|";
        }

        // print line ending if at end of row
        if(Utils::getCol(squareIndex) == Utils::BOARD_WIDTH-1) {
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
    constexpr int PIECE_PLACEMENT = 0;
    constexpr int SIDE_TO_MOVE = 1;
    constexpr int CASTLING = 2;
    constexpr int EN_PASSANT = 3;
    constexpr int HALFMOVE_CLOCK = 4;
    constexpr int FULLMOVE_CLOCK = 5;

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

        if(field == PIECE_PLACEMENT) {
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
                throw std::runtime_error("Invalid FEN.");
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

        if(field == SIDE_TO_MOVE) {
            // this field only has one character, either w or b
            switch(c) {
                // white's turn
                case 'w': sideToMove_ = Color::White; break;
                // black's turn
                case 'b': sideToMove_ = Color::Black; break;
                default: std::cerr << "Unable to parse FEN: " << FEN << "\nInvalid current color: " << "'" << c << "'"; throw std::runtime_error("Invalid FEN.");
            }
            continue;
        }

        if(field == CASTLING) {
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
                default: std::cerr << "Invalid FEN: " << FEN << "\nInvalid castling char: " << "'" << c << "'"; throw std::runtime_error("Invalid FEN.");
            }
            continue;
        }

        if(field == EN_PASSANT) {
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
                enPassantSquare_ = Utils::algebraicNotationToInt(enPassantSquare);
            }

            continue;
        }

        // TODO: halfmove clock
        if(field == HALFMOVE_CLOCK) {
            continue;
        }

        // TODO: fullmove clock
        if(field == FULLMOVE_CLOCK) {
            continue;
        }
    }

    // throw if either king is missing
    if(bbWhiteKing_.empty() || bbBlackKing_.empty()) {
        std::cerr << "Unable to parse FEN: " << FEN << "\nFEN must include both kings.";
        throw std::runtime_error("Invalid FEN.");
    }
}

bool Game::isFinished() {
    // if no legal moves for current turn then the game is over
    MoveList legalMoves;
    generateLegalMoves(legalMoves);
    return legalMoves.size == 0;
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
    for (int square = 0; square < Utils::NUM_SQUARES; square++) {
        const int col = Utils::getCol(square);
        const int row = Utils::getRow(square);

        // Knight attacks
        Bitboard knightMoves{0};
        for (const auto knightDelta : Utils::knightDeltas) {
            const int curCol = col + knightDelta[0];
            const int curRow = row + knightDelta[1];
            if (Utils::onBoard(curCol, curRow)) {
                knightMoves.setSquare(Utils::getSquareIndex(curCol, curRow));
            }
        }
        attackBitboards_.knightAttacks[square] = knightMoves;

        // King attacks
        Bitboard kingMoves{0};
        for (const auto kingDelta: Utils::kingDeltas) {
            const int curCol = col + kingDelta[0];
            const int curRow = row + kingDelta[1];
            if (Utils::onBoard(curCol, curRow)) {
                kingMoves.setSquare(Utils::getSquareIndex(curCol, curRow));
            }
        }
        attackBitboards_.kingAttacks[square] = kingMoves;

        // Pawn FROM
        Bitboard whitePawnMoves{0};
        if (Utils::onBoard(col - 1, row - 1)) {
            whitePawnMoves.setBit(Bitboard::bit(Utils::getSquareIndex(col - 1, row - 1)));
        }
        if (Utils::onBoard(col + 1, row - 1)) {
            whitePawnMoves.setBit(Bitboard::bit(Utils::getSquareIndex(col + 1, row - 1)));
        }
        attackBitboards_.whitePawnAttacks[square] = whitePawnMoves;

        Bitboard blackPawnMoves{0};
        if (Utils::onBoard(col - 1, row + 1)) {
            blackPawnMoves.setBit(Bitboard::bit(Utils::getSquareIndex(col - 1, row + 1)));
        }
        if (Utils::onBoard(col + 1, row + 1)) {
            blackPawnMoves.setBit(Bitboard::bit(Utils::getSquareIndex(col + 1, row + 1)));
        }
        attackBitboards_.blackPawnAttacks[square] = blackPawnMoves;

        // Slider rays
        for (int curSquare = square + Utils::NORTH; curSquare < Utils::NUM_SQUARES; curSquare += Utils::NORTH) {
            attackBitboards_.northRay[square].setSquare(curSquare);
        }

        for (int curSquare = square + Utils::SOUTH; curSquare >= 0; curSquare += Utils::SOUTH) {
            attackBitboards_.southRay[square].setSquare(curSquare);
        }

        // Stop at H-file
        for (int curSquare = square + Utils::EAST; curSquare < Utils::NUM_SQUARES && Utils::getCol(curSquare) != 0; curSquare += Utils::EAST) {
            attackBitboards_.eastRay[square].setSquare(curSquare);
        }

        // Stop at A-file
        for (int curSquare = square + Utils::WEST; curSquare >= 0 && Utils::getCol(curSquare) != Utils::BOARD_WIDTH-1; curSquare += Utils::WEST) {
            attackBitboards_.westRay[square].setSquare(curSquare);
        }

        // Stop at H-file
        for (int curSquare = square + Utils::NORTH_EAST; curSquare < Utils::NUM_SQUARES && Utils::getCol(curSquare) != 0; curSquare += Utils::NORTH_EAST) {
            attackBitboards_.neRay[square].setSquare(curSquare);
        }

        // Stop at A-file
        for (int curSquare = square + Utils::NORTH_WEST; curSquare < Utils::NUM_SQUARES && Utils::getCol(curSquare) != Utils::BOARD_WIDTH-1; curSquare += Utils::NORTH_WEST) {
            attackBitboards_.nwRay[square].setSquare(curSquare);
        }

        // Stop at H-file
        for (int curSquare = square + Utils::SOUTH_EAST; curSquare >= 0 && Utils::getCol(curSquare) != 0; curSquare += Utils::SOUTH_EAST) {
            attackBitboards_.seRay[square].setSquare(curSquare);
        }

        // Stop at A-file
        for (int curSquare = square + Utils::SOUTH_WEST; curSquare >= 0 && Utils::getCol(curSquare) != Utils::BOARD_WIDTH-1; curSquare += Utils::SOUTH_WEST) {
            attackBitboards_.swRay[square].setSquare(curSquare);
        }
    }
}

// This is slow, but that's okay because its only ran once per Game instance.
void Game::initPieceToBBTable_() {
    piecePackedToBB_.fill(nullptr);
    // helper to set table value based on piecetype, color, and bitboard
    auto set = [&](PieceType type, Color color, Bitboard* bitboard){
        const Piece piece{type, color};
        piecePackedToBB_[piece.raw()] = bitboard;
    };
    // manually set each piece type
    set(PieceType::Pawn, Color::White, &bbWhitePawns_);
    set(PieceType::Knight, Color::White, &bbWhiteKnights_);
    set(PieceType::Bishop, Color::White, &bbWhiteBishops_);
    set(PieceType::Rook, Color::White, &bbWhiteRooks_);
    set(PieceType::Queen, Color::White, &bbWhiteQueens_);
    set(PieceType::King, Color::White, &bbWhiteKing_);

    set(PieceType::Pawn, Color::Black, &bbBlackPawns_);
    set(PieceType::Knight, Color::Black, &bbBlackKnights_);
    set(PieceType::Bishop, Color::Black, &bbBlackBishops_);
    set(PieceType::Rook, Color::Black, &bbBlackRooks_);
    set(PieceType::Queen, Color::Black, &bbBlackQueens_);
    set(PieceType::King, Color::Black, &bbBlackKing_);
}

void Game::generatePseudoLegalPawnMoves_(MoveList& out) {
    const bool isWhite = sideToMove_ == Color::White;

    Bitboard sourcePawns = isWhite ? bbWhitePawns_ : bbBlackPawns_;
    const Bitboard& sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard& targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    // TODO: once bbAllPieces_ is implemented, replace this with bbAllPieces_.flip()
    const Bitboard& emptySquares = bbWhitePieces_.merge(bbBlackPieces_).flip();
    if(isWhite) { // black and white pawns move differently
        // White

        // Normal moves
        // for white we shift up one row (8 squares) if it lands on an empty square
        const Bitboard& oneRowPush = sourcePawns.rightShift(Utils::NORTH).mask(emptySquares);
    
        Bitboard normal = oneRowPush;
        while(!normal.empty()) {
            const int targetSquare = normal.popLsb();
            addAllPawnPromotionsToMoves_(out, targetSquare+Utils::NORTH, targetSquare, Piece{PieceType::Pawn, Color::White}, false);
        }

        // Double push
        // for white we shift up two rows (16 squares) if it lands on an empty square on the fourth rank
        // we shift from 'oneRowPush' to ensure both squares are empty
        Bitboard doublePush = oneRowPush.rightShift(Utils::NORTH).mask(emptySquares).mask(Bitboard{Bitboard::Rank4});
        while(!doublePush.empty()) {
            const int targetSquare = doublePush.popLsb();
            // double push can never be a promotion, so we don't need to call addAllPawnPromotionsToMoves_ here
            out.push_back(Move{targetSquare+(2*Utils::NORTH), targetSquare, MoveFlag::DoublePawnPush, Promotion::None});
        }

        // En Passant
        if (enPassantSquare_ != UndoInfo::noEnPassant) {
            // we check black pawn attack pattern because pawn moves are not symmetrical
            Bitboard attackers = bbWhitePawns_.mask(attackBitboards_.blackPawnAttacks[enPassantSquare_]);

            while (!attackers.empty()) {
                const int from = attackers.popLsb();
                out.push_back(Move{from, enPassantSquare_, MoveFlag::EnPassant, Promotion::None});
            }
        }

        // we can now mutate sourcePawns because we're done with the constant operations
        while(!sourcePawns.empty()) {
            const int sourceSquare = sourcePawns.popLsb();

            // Normal capture
            const Bitboard& captureAttacks = attackBitboards_.whitePawnAttacks[sourceSquare].mask(sourcePieces.flip());
            Bitboard captures = captureAttacks.mask(targetPieces); // attacks that land on target pieces
            while(!captures.empty()) {
                const int targetSquare = captures.popLsb();
                addAllPawnPromotionsToMoves_(out, sourceSquare, targetSquare, Piece{PieceType::Pawn, Color::White}, true);
            }
        }
    } else {
        // Black

        // Normal moves
        // for black we shift down one row (8 squares) if it lands on an empty square
        constexpr int ONE_ROW = 8;
        const Bitboard& oneRowPush = sourcePawns.leftShift(ONE_ROW).mask(emptySquares);
    
        Bitboard normal = oneRowPush;
        while(!normal.empty()) {
            const int targetSquare = normal.popLsb();
            addAllPawnPromotionsToMoves_(out, targetSquare+Utils::SOUTH, targetSquare, Piece{PieceType::Pawn, Color::Black}, false);
        }

        // Double push
        // for black we shift down two rows (16 squares) if it lands on an empty square on the fifth rank
        // we shift from 'oneRowPush' to ensure both squares are empty
        Bitboard doublePush = oneRowPush.leftShift(ONE_ROW).mask(emptySquares).mask(Bitboard{Bitboard::Rank5});
        while(!doublePush.empty()) {
            const int targetSquare = doublePush.popLsb();
            // double push can never be a promotion, so we don't need to call addAllPawnPromotionsToMoves_ here
            out.push_back(Move{targetSquare+(2*Utils::SOUTH), targetSquare, MoveFlag::DoublePawnPush, Promotion::None});
        }

        // En Passant
        if (enPassantSquare_ != UndoInfo::noEnPassant) {
            // we check white pawn attack pattern because pawn moves are not symmetrical
            Bitboard attackers = bbBlackPawns_.mask(attackBitboards_.whitePawnAttacks[enPassantSquare_]);

            while (!attackers.empty()) {
                const int from = attackers.popLsb();
                out.push_back(Move{from, enPassantSquare_, MoveFlag::EnPassant, Promotion::None});
            }
        }

        // we can now mutate sourcePawns because we're done with the constant operations
        while(!sourcePawns.empty()) {
            const int sourceSquare = sourcePawns.popLsb();

            // Normal capture
            const Bitboard& captureAttacks = attackBitboards_.blackPawnAttacks[sourceSquare].mask(sourcePieces.flip());
            Bitboard captures = captureAttacks.mask(targetPieces); // attacks that land on target pieces
            while(!captures.empty()) {
                const int targetSquare = captures.popLsb();
                addAllPawnPromotionsToMoves_(out, sourceSquare, targetSquare, Piece{PieceType::Pawn, Color::Black}, true);
            }
        }
    }
}


void Game::generatePseudoLegalKnightMoves_(MoveList& out) {
    const bool isWhite = sideToMove_ == Color::White;

    Bitboard sourceKnights = isWhite ? bbWhiteKnights_ : bbBlackKnights_;
    const Bitboard& sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard& targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    while(!sourceKnights.empty()) {
        const int sourceSquare = sourceKnights.popLsb();
        const Bitboard& attacks = attackBitboards_.knightAttacks[sourceSquare].mask(sourcePieces.flip()); // can not attack own pieces

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

void Game::generatePseudoLegalBishopMoves_(MoveList& out) {
    const bool isWhite = sideToMove_ == Color::White;

    Bitboard sourceBishops = isWhite ? bbWhiteBishops_ : bbBlackBishops_;
    const Bitboard& sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard& targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    while (!sourceBishops.empty()) {
        const int sourceSquare = sourceBishops.popLsb();

        // NOTE: loop unrolled here; less readable, but faster
        // Stop when we hit H-file
        for (int targetSquare = sourceSquare + Utils::NORTH_EAST; targetSquare < Utils::NUM_SQUARES && Utils::getCol(targetSquare) != 0; targetSquare += Utils::NORTH_EAST) {
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

        // Stop when we hit A-file
        for (int targetSquare = sourceSquare + Utils::NORTH_WEST; targetSquare < Utils::NUM_SQUARES && Utils::getCol(targetSquare) != Utils::BOARD_WIDTH-1; targetSquare += Utils::NORTH_WEST) {
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

        // Stop when we hit H-file
        for (int targetSquare = sourceSquare + Utils::SOUTH_EAST; targetSquare >= 0 && Utils::getCol(targetSquare) != 0; targetSquare += Utils::SOUTH_EAST) {
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

        // Stop when we hit A-file
        for (int targetSquare = sourceSquare + Utils::SOUTH_WEST; targetSquare >= 0 && Utils::getCol(targetSquare) != Utils::BOARD_WIDTH-1; targetSquare += Utils::SOUTH_WEST) {
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
    const bool isWhite = sideToMove_ == Color::White;

    Bitboard sourceRooks = isWhite ? bbWhiteRooks_ : bbBlackRooks_;
    const Bitboard& sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard& targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    while (!sourceRooks.empty()) {
        const int sourceSquare = sourceRooks.popLsb();

        for (int targetSquare = sourceSquare + Utils::NORTH; targetSquare < Utils::NUM_SQUARES; targetSquare += Utils::NORTH) {
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

        for (int targetSquare = sourceSquare + Utils::SOUTH; targetSquare >= 0; targetSquare += Utils::SOUTH) {
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

        // Stop at H-file
        for (int targetSquare = sourceSquare + Utils::EAST; targetSquare < Utils::NUM_SQUARES && Utils::getCol(targetSquare) != 0; targetSquare += Utils::EAST) {
            // Utils::getCol(targetSquare) != 0 prevents wrap H->A because when you go from 7 to 8, file becomes 0
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

        // Stop at A-file
        for (int targetSquare = sourceSquare + Utils::WEST; targetSquare >= 0 && Utils::getCol(targetSquare) != Utils::BOARD_WIDTH-1; targetSquare += Utils::WEST) {
            // Utils::getCol(targetSquare) != 7 prevents wrap A->H because when you go from 8 to 7, file becomes 7
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
    const bool isWhite = sideToMove_ == Color::White;

    Bitboard sourceQueens = isWhite ? bbWhiteQueens_ : bbBlackQueens_;
    const Bitboard& sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard& targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;

    while (!sourceQueens.empty()) {
        const int sourceSquare = sourceQueens.popLsb();

        // ---- Rook Moves ----
        for (int targetSquare = sourceSquare + Utils::NORTH; targetSquare < Utils::NUM_SQUARES; targetSquare += Utils::NORTH) {
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

        for (int targetSquare = sourceSquare + Utils::SOUTH; targetSquare >= 0; targetSquare += Utils::SOUTH) {
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

        // Stop at H-file
        for (int targetSquare = sourceSquare + Utils::EAST; targetSquare < Utils::NUM_SQUARES && Utils::getCol(targetSquare) != 0; targetSquare += Utils::EAST) {
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

        // Stop at A-file
        for (int targetSquare = sourceSquare + Utils::WEST; targetSquare >= 0 && Utils::getCol(targetSquare) != Utils::BOARD_WIDTH-1; targetSquare += Utils::WEST) {
            // Utils::getCol(targetSquare) != 7 prevents wrap A->H because when you go from 8 to 7, file becomes 7
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
        // Stop when we hit H-file (col becomes 0 after wrap)
        for (int targetSquare = sourceSquare + Utils::NORTH_EAST; targetSquare < Utils::NUM_SQUARES && Utils::getCol(targetSquare) != 0; targetSquare += Utils::NORTH_EAST) {
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

        // Stop when we hit A-file
        for (int targetSquare = sourceSquare + Utils::NORTH_WEST; targetSquare < Utils::NUM_SQUARES && Utils::getCol(targetSquare) != Utils::BOARD_WIDTH-1; targetSquare += Utils::NORTH_WEST) {
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

        // Stop when we hit H-file
        for (int targetSquare = sourceSquare + Utils::SOUTH_EAST; targetSquare >= 0 && Utils::getCol(targetSquare) != 0; targetSquare += Utils::SOUTH_EAST) {
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

        // Stop when we hit A-file
        for (int targetSquare = sourceSquare + Utils::SOUTH_WEST; targetSquare >= 0 && Utils::getCol(targetSquare) != Utils::BOARD_WIDTH-1; targetSquare += Utils::SOUTH_WEST) {
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
    const bool isWhite = sideToMove_ == Color::White;

    Bitboard sourceKing = isWhite ? bbWhiteKing_ : bbBlackKing_;
    const Bitboard& sourcePieces = isWhite ? bbWhitePieces_ : bbBlackPieces_;
    const Bitboard& targetPieces = isWhite ? bbBlackPieces_ : bbWhitePieces_;
    const Bitboard& allPieces = bbWhitePieces_.merge(bbBlackPieces_);

    // NOTE: because we assume just one king per side, we do not need to loop over all king squares.
    // for the same reason, we do not need to check if a king exists before using .popLsb();
    while(!sourceKing.empty()) {
        const int sourceSquare = sourceKing.popLsb();
        const Bitboard& attacks = attackBitboards_.kingAttacks[sourceSquare].mask(sourcePieces.flip()); // can not attack own pieces

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
        // TODO: move "getKingStartingSquare(Color color)", etc. into Utils
        const int kingStartingSquare = isWhite ? Utils::WHITE_KING_STARTING_SQUARE : Utils::BLACK_KING_STARTING_SQUARE;

        const int kingsidePassingSquare = isWhite ? Utils::WHITE_KINGSIDE_PASSING_SQUARE : Utils::BLACK_KINGSIDE_PASSING_SQUARE;
        const int kingsideTargetSquare = isWhite ? Utils::WHITE_KINGSIDE_TARGET_SQUARE : Utils::BLACK_KINGSIDE_TARGET_SQUARE;

        const int queensidePassingSquare = isWhite ? Utils::WHITE_QUEENSIDE_PASSING_SQUARE : Utils::BLACK_QUEENSIDE_PASSING_SQUARE;
        const int queensideTargetSquare = isWhite ? Utils::WHITE_QUEENSIDE_TARGET_SQUARE : Utils::BLACK_QUEENSIDE_TARGET_SQUARE;

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
        const int kingStartingSquare = isSourceWhite ? Utils::WHITE_KING_STARTING_SQUARE : Utils::BLACK_KING_STARTING_SQUARE;

        if(move.isKingSideCastle()) {
            const int kingsidePassingSquare = isSourceWhite ? Utils::WHITE_KINGSIDE_PASSING_SQUARE : Utils::BLACK_KINGSIDE_PASSING_SQUARE;
            const int kingsideTargetSquare = isSourceWhite ? Utils::WHITE_KINGSIDE_TARGET_SQUARE : Utils::BLACK_KINGSIDE_TARGET_SQUARE;
    
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
            const int queensidePassingSquare = isSourceWhite ? Utils::WHITE_QUEENSIDE_PASSING_SQUARE : Utils::BLACK_QUEENSIDE_PASSING_SQUARE;
            const int queensideTargetSquare = isSourceWhite ? Utils::WHITE_QUEENSIDE_TARGET_SQUARE : Utils::BLACK_QUEENSIDE_TARGET_SQUARE;
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

bool Game::tryMove(const Move& move) {
    // TODO: consider making isWhite() function in Game; this logic is repeated quite often
    const bool isWhite = sideToMove_ == Color::White;
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
    const Color sourceColor = sourcePiece.color();
    const Color targetColor = oppositeColor(sourceColor);

    const bool isSourcePieceWhite = sourceColor == Color::White;

    // flip current turn
    sideToMove_ = oppositeColor(sideToMove_);
    // remove en passant (we may set it again later in this function)
    enPassantSquare_ = UndoInfo::noEnPassant;

    Bitboard& sourceBitboard = pieceToBitboard(sourcePiece);
    Bitboard& sourceColorBitboard = colorToOccupancyBitboard(sourceColor);
    Bitboard& targetColorBitboard = colorToOccupancyBitboard(targetColor);

    // update castling flags
    if(
        move.sourceSquare() == Utils::WHITE_KING_STARTING_SQUARE ||
        move.sourceSquare() == Utils::WHITE_KINGSIDE_ROOK_STARTING_SQUARE ||
        move.targetSquare() == Utils::WHITE_KINGSIDE_ROOK_STARTING_SQUARE // white kingside rook captured
    ) {
        castlingRights_.clearWhiteKingside();
    }
    if(
        move.sourceSquare() == Utils::WHITE_KING_STARTING_SQUARE ||
        move.sourceSquare() == Utils::WHITE_QUEENSIDE_ROOK_STARTING_SQUARE || // moving white queenside pieces
        move.targetSquare() == Utils::WHITE_QUEENSIDE_ROOK_STARTING_SQUARE // white queenside rook captured
    ) {
        castlingRights_.clearWhiteQueenside();
    }
    if(
        move.sourceSquare() == Utils::BLACK_KING_STARTING_SQUARE ||
        move.sourceSquare() == Utils::BLACK_KINGSIDE_ROOK_STARTING_SQUARE || // moving black kingside pieces
        move.targetSquare() == Utils::BLACK_KINGSIDE_ROOK_STARTING_SQUARE // black kingside rook captured
    ) {
        castlingRights_.clearBlackKingside();
    }
    if(
        move.sourceSquare() == Utils::BLACK_KING_STARTING_SQUARE ||
        move.sourceSquare() == Utils::BLACK_QUEENSIDE_ROOK_STARTING_SQUARE || // moving black queenside pieces
        move.targetSquare() == Utils::BLACK_QUEENSIDE_ROOK_STARTING_SQUARE // black queenside rook captured
    ) {
        castlingRights_.clearBlackQueenside();
    }

    // update en passant flag
    if(move.isDoublePawn()) {
        const int sourceRow = Utils::getRow(move.sourceSquare());
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int passedRow = sourceRow + towardsCenter; // row that was passed in the double move  
        enPassantSquare_ = Utils::getSquareIndex(Utils::getCol(move.sourceSquare()), passedRow); 
    }

    // If en passant capture, remove the captured pawn
    if (move.isEnPassant()) {
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int capturedIndex = move.targetSquare() - (towardsCenter * 8);

        // clear captured pawn's bitboard placement
        Bitboard& bbTargetPawns = targetColor == Color::White ? bbWhitePawns_ : bbBlackPawns_;
        bbTargetPawns.clearSquare(capturedIndex);

        // update occupancy board
        targetColorBitboard.clearSquare(capturedIndex);

        // clear captured pawn from mailbox
        mailbox_[capturedIndex] = Piece{};
    }

    // If king side castle, also move the rook
    if(move.isKingSideCastle()) {
        const int kingsidePassingSquare = isSourcePieceWhite ? Utils::WHITE_KINGSIDE_PASSING_SQUARE : Utils::BLACK_KINGSIDE_PASSING_SQUARE;
        const int kingsideRookSquare = isSourcePieceWhite ? Utils::WHITE_KINGSIDE_ROOK_STARTING_SQUARE : Utils::BLACK_KINGSIDE_ROOK_STARTING_SQUARE;

        // set kingside rook's new position in bitboard and clear old position
        Bitboard& bbSourceRooks = isSourcePieceWhite ? bbWhiteRooks_ : bbBlackRooks_;
        bbSourceRooks.setSquare(kingsidePassingSquare);
        bbSourceRooks.clearSquare(kingsideRookSquare);

        // update occupancy board
        sourceColorBitboard.setSquare(kingsidePassingSquare);
        sourceColorBitboard.clearSquare(kingsideRookSquare);

        // also move rook in mailbox
        mailbox_[kingsidePassingSquare] = Piece{PieceType::Rook, sourceColor};
        mailbox_[kingsideRookSquare] = Piece{};
    }

    // If queen side castle, also move the queen
    if(move.isQueenSideCastle()) {
        const int queensidePassingSquare = isSourcePieceWhite ? Utils::WHITE_QUEENSIDE_PASSING_SQUARE : Utils::BLACK_QUEENSIDE_PASSING_SQUARE;
        const int queensideRookSquare = isSourcePieceWhite ? Utils::WHITE_QUEENSIDE_ROOK_STARTING_SQUARE : Utils::BLACK_QUEENSIDE_ROOK_STARTING_SQUARE;

        // set kingside rook's new position in bitboard and clear old position
        Bitboard& bbSourceRooks = isSourcePieceWhite ? bbWhiteRooks_ : bbBlackRooks_;;
        bbSourceRooks.setSquare(queensidePassingSquare);
        bbSourceRooks.clearSquare(queensideRookSquare);

        sourceColorBitboard.setSquare(queensidePassingSquare);
        sourceColorBitboard.clearSquare(queensideRookSquare);

        // also move rook in mailbox
        mailbox_[queensidePassingSquare] = Piece{PieceType::Rook, sourceColor};
        mailbox_[queensideRookSquare] = Piece{};;
    }

    // handle pawn promotion; different enough we need to return early
    if(move.isPromotion()) {
        const PieceType promotionType = Move::promotionToPieceType(move.promotion());

        // update promotion bitboard
        // remove pawn from pawn bitboard
        sourceBitboard.clearSquare(move.sourceSquare());
        // add promoted piece to promoted piece bitboard
        pieceToBitboard(Piece{promotionType, sourceColor}).setSquare(move.targetSquare());

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
        mailbox_[move.targetSquare()] = Piece{promotionType, sourceColor};
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

UndoInfo Game::makeMoveWithUndoInfo(const Move& move) {
    // save info to return
    const UndoInfo undoInfo = getUndoInfo(mailbox_[move.targetSquare()]);

    makeMove(move);

    return undoInfo;
}

void Game::undoMove(const Move& move, const UndoInfo& undoInfo) {
    // sourcePiece is now sitting at targetSquare
    const Piece sourcePiece = mailbox_[move.targetSquare()];
    const Color sourceColor = sourcePiece.color();

    const bool isSourcePieceWhite = sourceColor == Color::White;

    // flip current turn
    sideToMove_ = oppositeColor(sideToMove_);

    // source piece's bitboard
    Bitboard& sourceBitboard = pieceToBitboard(sourcePiece);
    Bitboard& sourceColorBitboard = colorToOccupancyBitboard(sourceColor);
    Bitboard& targetColorBitboard = colorToOccupancyBitboard(oppositeColor(sourceColor));

    // handle king side castle
    if(move.isKingSideCastle()) {
        const int kingsidePassingSquare = isSourcePieceWhite ? Utils::WHITE_KINGSIDE_PASSING_SQUARE : Utils::BLACK_KINGSIDE_PASSING_SQUARE;
        const int kingsideRookSquare = isSourcePieceWhite ? Utils::WHITE_KINGSIDE_ROOK_STARTING_SQUARE : Utils::BLACK_KINGSIDE_ROOK_STARTING_SQUARE;

        // undo rook move in bitboard
        Bitboard& bbSourceRooks = isSourcePieceWhite ? bbWhiteRooks_ : bbBlackRooks_;
        bbSourceRooks.clearSquare(kingsidePassingSquare);
        bbSourceRooks.setSquare(kingsideRookSquare);

        // update occupancy bitboard
        sourceColorBitboard.clearSquare(kingsidePassingSquare);
        sourceColorBitboard.setSquare(kingsideRookSquare);

        // undo rook move in mailbox
        mailbox_[kingsidePassingSquare] = Piece{};
        mailbox_[kingsideRookSquare] = Piece{PieceType::Rook, sourceColor};
    }

    // handle queen side castle
    if(move.isQueenSideCastle()) {
        const int queensidePassingSquare = isSourcePieceWhite ? Utils::WHITE_QUEENSIDE_PASSING_SQUARE : Utils::BLACK_QUEENSIDE_PASSING_SQUARE;
        const int queensideRookSquare = isSourcePieceWhite ? Utils::WHITE_QUEENSIDE_ROOK_STARTING_SQUARE : Utils::BLACK_QUEENSIDE_ROOK_STARTING_SQUARE;

        Bitboard& bbSourceRooks = isSourcePieceWhite ? bbWhiteRooks_ : bbBlackRooks_;
        bbSourceRooks.clearSquare(queensidePassingSquare);
        bbSourceRooks.setSquare(queensideRookSquare);

        // update occupancy bitboard
        sourceColorBitboard.clearSquare(queensidePassingSquare);
        sourceColorBitboard.setSquare(queensideRookSquare);

        // undo rook move
        mailbox_[queensidePassingSquare] = Piece{};
        mailbox_[queensideRookSquare] = Piece{PieceType::Rook, sourceColor};
    }

    // handle en passant
    if(move.isEnPassant()) {
        const int towardsCenter = isSourcePieceWhite ? -1 : +1;
        const int capturedIndex = move.targetSquare() - (towardsCenter * 8);

        // reset captured pawn's bitboard placement
        Bitboard& bbEnemyPawns = isSourcePieceWhite ? bbBlackPawns_ : bbWhitePawns_;
        bbEnemyPawns.setSquare(capturedIndex);

        // update occupancy bitboard
        targetColorBitboard.setSquare(capturedIndex);

        // replace captured pawn
        mailbox_[capturedIndex] = Piece{PieceType::Pawn, oppositeColor(sourceColor)};
    }

    // handle promotion; different enough that we need to return early
    if(move.isPromotion()) {
        // update promotion bitboard
        // re-add pawn to pawn bitboard
        pieceToBitboard(Piece{PieceType::Pawn, sourceColor}).setSquare(move.sourceSquare());
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
        mailbox_[move.sourceSquare()] = Piece{PieceType::Pawn, sourceColor};
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
    enPassantSquare_ = undoInfo.prevEnPassantSquare;
}

bool Game::isSquareAttacked(const int targetSquare, const Color attackingColor) const {
    const bool isWhiteAttacking = attackingColor == Color::White;
    const Bitboard& allPieces = bbWhitePieces_.merge(bbBlackPieces_);
    // we compute "is attackingColor attacking targetSquare"
    // Pawns -- since pawn moves are not symmetric we use the opposite color's attacking bitboard
    const Bitboard& attackingPawns = isWhiteAttacking ? bbWhitePawns_ : bbBlackPawns_;
    const std::array<Bitboard, Utils::NUM_SQUARES>& attackingPawnsMap = isWhiteAttacking ? attackBitboards_.blackPawnAttacks : attackBitboards_.whitePawnAttacks;
    if(!attackingPawns.mask(attackingPawnsMap[targetSquare]).empty()) {
        return true;
    }

    // Knights -- is there an attacking knight sitting a knights move away from targetSquare
    const Bitboard& attackingKnights = isWhiteAttacking ? bbWhiteKnights_ : bbBlackKnights_;
    if(!attackingKnights.mask(attackBitboards_.knightAttacks[targetSquare]).empty()) {
        return true;
    }

    // Kings -- is there an attacking king sitting a kings move away from targetSquare
    const Bitboard& attackingKings = isWhiteAttacking ? bbWhiteKing_ : bbBlackKing_;
    if(!attackingKings.mask(attackBitboards_.kingAttacks[targetSquare]).empty()) {
        return true;
    }

    // Sliding pieces -- Find nearest blocker to determine if square is attacked
    const Bitboard& attackingRooks = isWhiteAttacking ? bbWhiteRooks_ : bbBlackRooks_;
    const Bitboard& attackingBishops = isWhiteAttacking ? bbWhiteBishops_ : bbBlackBishops_;
    const Bitboard& attackingQueens = isWhiteAttacking ? bbWhiteQueens_ : bbBlackQueens_;
    // Orthogonal, rook / queen
    const Bitboard& rookLike = attackingRooks.merge(attackingQueens);
    // North (increasing squares) -> nearest blocker = LSB of blockers on that ray
    const Bitboard northBlockers = attackBitboards_.northRay[targetSquare].mask(allPieces);
    if(!northBlockers.empty()) {
        const int blocker = northBlockers.lsbIndex();
        if(rookLike.containsSquare(blocker)) {
            return true;
        }
    }

    // South (decreasing squares) -> nearest blocker = MSB
    const Bitboard southBlockers = attackBitboards_.southRay[targetSquare].mask(allPieces);
    if(!southBlockers.empty()) {
        const int blocker = southBlockers.msbIndex();
        if(rookLike.containsSquare(blocker)) {
            return true;
        }
    }

    // East (increasing) -> LSB
    const Bitboard eastBlockers = attackBitboards_.eastRay[targetSquare].mask(allPieces);
    if(!eastBlockers.empty()) {
        const int blocker = eastBlockers.lsbIndex();
        if(rookLike.containsSquare(blocker)) {
            return true;
        }
    }

    // West (decreasing) -> MSB
    const Bitboard westBlockers = attackBitboards_.westRay[targetSquare].mask(allPieces);
    if(!westBlockers.empty()) {
        const int blocker = westBlockers.msbIndex();
        if(rookLike.containsSquare(blocker)) {
            return true;
        }
    }

    // Diagonal, bishop / queen
    const Bitboard& bishopLike = attackingBishops.merge(attackingQueens);
    // NorthEast (increasing squares) -> LSB
    const Bitboard northEastBlockers = attackBitboards_.neRay[targetSquare].mask(allPieces);
    if(!northEastBlockers.empty()) {
        const int blocker = northEastBlockers.lsbIndex();
        if(bishopLike.containsSquare(blocker)) {
            return true;
        }
    }

    // NorthWest (increasing squares) -> nearest blocker = LSB
    const Bitboard northWestBlockers = attackBitboards_.nwRay[targetSquare].mask(allPieces);
    if(!northWestBlockers.empty()) {
        const int blocker = northWestBlockers.lsbIndex();
        if(bishopLike.containsSquare(blocker)) {
            return true;
        }
    }

    // SouthEast (decreasing) -> MSB
    const Bitboard southEastBlockers = attackBitboards_.seRay[targetSquare].mask(allPieces);
    if(!southEastBlockers.empty()) {
        const int blocker = southEastBlockers.msbIndex();
        if(bishopLike.containsSquare(blocker)) {
            return true;
        }
    }

    // SouthWest (decreasing) -> MSB
    const Bitboard southWestBlockers = attackBitboards_.swRay[targetSquare].mask(allPieces);
    if(!southWestBlockers.empty()) {
        const int blocker = southWestBlockers.msbIndex();
        if(bishopLike.containsSquare(blocker)) {
            return true;
        }
    }

    return false;
}

std::string Move::to_string(const Game& game) const {
    return ( 
        game.mailbox()[sourceSquare()].to_string_long() + " on " + Utils::intToAlgebraicNotation(sourceSquare()) + " to " +
        game.mailbox()[targetSquare()].to_string_long() + " on " + Utils::intToAlgebraicNotation(targetSquare())
    );
}
#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <string>

#include "Bitboard.hpp"
#include "Move.hpp"
#include "Piece.hpp"
#include "Utils.hpp"

// TODO: fixup magic numbers
// Representation of the castling rights of a position, stored in uint8_t for maximum speed.
struct CastlingRights {
    uint8_t castlingRights;

    // Pack the four castling bools into one uint8_t.
    static constexpr uint8_t pack(bool whiteKingside, bool whiteQueenside, bool blackKingside, bool blackQueenside) noexcept {
        return (static_cast<uint8_t>(whiteKingside) << 0) |
                (static_cast<uint8_t>(whiteQueenside) << 1) |
                (static_cast<uint8_t>(blackKingside) << 2) |
                (static_cast<uint8_t>(blackQueenside) << 3);
    }

    // Getters extract bits from packed representation.
    constexpr bool canWhiteKingside() const noexcept { return static_cast<bool>(castlingRights & (1 << 0)); }
    constexpr bool canWhiteQueenside() const noexcept { return static_cast<bool>(castlingRights & (1 << 1)); }
    constexpr bool canBlackKingside() const noexcept { return static_cast<bool>(castlingRights & (1 << 2)); }
    constexpr bool canBlackQueenside() const noexcept { return static_cast<bool>(castlingRights & (1 << 3)); }

    constexpr void setWhiteKingside() noexcept { castlingRights |= (1 << 0); }
    constexpr void setWhiteQueenside() noexcept { castlingRights |= (1 << 1); }
    constexpr void setBlackKingside() noexcept { castlingRights |= (1 << 2); }
    constexpr void setBlackQueenside() noexcept { castlingRights |= (1 << 3); }

    constexpr void clearWhiteKingside() noexcept { castlingRights &= ~(1 << 0); }
    constexpr void clearWhiteQueenside() noexcept { castlingRights &= ~(1 << 1); }
    constexpr void clearBlackKingside() noexcept { castlingRights &= ~(1 << 2); }
    constexpr void clearBlackQueenside() noexcept { castlingRights &= ~(1 << 3); }
};

// Info used to fully undo a move.
struct UndoInfo {
    // Store previous state
    CastlingRights prevCastlingRights;
    uint8_t prevEnPassantSquare;
    Piece capturedPiece;

    static constexpr uint8_t noEnPassant = 255;

    constexpr UndoInfo(CastlingRights castlingRights,
                       uint8_t enPassantSquare,
                       Piece capturedPiece_) noexcept
        : prevCastlingRights{castlingRights},
          prevEnPassantSquare{enPassantSquare},
          capturedPiece{capturedPiece_} {}
} __attribute__((aligned(4))); // align to 4 bytes

// Create holder for all AttackBitboards
struct AttackBitboards {
    std::array<Bitboard, 64> whitePawnAttacks{};
    std::array<Bitboard, 64> blackPawnAttacks{};
    std::array<Bitboard, 64> knightAttacks{};
    std::array<Bitboard, 64> kingAttacks{};

    std::array<Bitboard, 64> northRay, southRay, eastRay, westRay;
    std::array<Bitboard, 64> neRay, nwRay, seRay, swRay;
} __attribute__((aligned(128))); // align to 128 bytes

// A chess game. Contains information for the game and helpers to generate and validate moves.
class Game {
public:
    // Construct a new game with an empty board. Current turn defaults to white.
    Game();
    // Update the board given a FEN.
    void loadFEN(const std::string& FEN);

    // Retrieve mailbox.
    constexpr std::array<Piece, Utils::NUM_SQUARES> mailbox() const noexcept { return mailbox_; }
    // Retrieve the color of the current player's turn.
    constexpr Color sideToMove() const noexcept { return sideToMove_; }
    // Retrieve a string representation of the current state of the board.
    std::string to_string() const;
    // If the game is finished.
    bool isFinished();
    // Get flags in the form of UndoInfo.
    constexpr UndoInfo getUndoInfo(Piece capturedPiece) const noexcept {
        // pack bools into castling rights uint8_t for speedy lookup
        return UndoInfo{
            castlingRights_,
            static_cast<uint8_t>(currentEnPassantSquare_),
            capturedPiece
        };
    }
    // Get piece at a square for the GUI. Note this method is relatively slow and should not be used in hot loops.
    Piece pieceAtSquareForGui(int square) const noexcept;
    // Try a move and return if the move was made. The move is only made if it is legal.
    bool tryMove(const Move& move);
    // Make a move, even if it is not legal.
    void makeMove(const Move& move);
    // Undo a move. Does not check if the move we are undoing happened before.
    void undoMove(const Move& move, const UndoInfo& undoInfo);
    // If a move is legal.
    constexpr bool isMoveLegal(const Move& move) noexcept {
        MoveList legalMoves;
        generateLegalMoves(legalMoves);

        for (int i = 0; i < legalMoves.size; i++) {
            if (legalMoves.data[i] == move) {
                return true;
            }
        }
        return false;
    }
    // Generate all legal moves.
    void generateLegalMoves(MoveList& out);
    // Generate all legal moves from a sourceSquare. This is slow and should only be used sparingly (e.g., in GUI).
    void generateLegalMovesFromSquare(int sourceSquare, MoveList& out);
    // Generate all pseudo legal moves. Pseudo legal moves only take piece movement into account, no king check status.
    constexpr void generatePseudoLegalMoves(MoveList& out) noexcept {
        generatePseudoLegalPawnMoves_(out);
        generatePseudoLegalKnightMoves_(out);
        generatePseudoLegalBishopMoves_(out);
        generatePseudoLegalRookMoves_(out);
        generatePseudoLegalQueenMoves_(out);
        generatePseudoLegalKingMoves_(out);
    }
    // If the given color is in check.
    constexpr bool isInCheck(const Color& colorToFind) const noexcept {
        // NOTE: this has undefined behavior if no kings on both sides
        const int kingSquare = findKingSquare(colorToFind);
        return isSquareAttacked(kingSquare, oppositeColor(colorToFind));
    }
    // If a given square is attacked by the attacking color.
    bool isSquareAttacked(int targetSquare, Color attackingColor) const;
    // Retrieve king square for a given color. Does not exist if king is not on board.
    constexpr int findKingSquare(const Color& colorToFind) const noexcept {
        Bitboard bbKing = colorToFind == Color::White ? bbWhiteKing_ : bbBlackKing_;
        // NOTE: this has undefined behavior if bbKing is empty
        return bbKing.popLsb();
    }

    // Get the opposite color of a given color.
    static constexpr Color oppositeColor(Color color) noexcept {
        return (color == Color::White) ? Color::Black : Color::White;
    }

    constexpr AttackBitboards attackBitboards() const noexcept {
        return attackBitboards_;
    }

    constexpr Bitboard& colorToOccupancyBitboard(Color color) const noexcept {
        // use very fast lookup table
        Bitboard* bitboard = colorToOccupancyBitboard_[static_cast<uint8_t>(color)];
        return *bitboard;
    }

    // Get a given piece's bitboard.
    constexpr Bitboard& pieceToBitboard(Piece piece) const noexcept {
        // Use lookup table for quick access
        Bitboard* const bitboard = piecePackedToBB_[piece.raw()];
        assert(bitboard != nullptr);
        return *bitboard;
    }

    // Bitboard getters
    // White
    constexpr Bitboard& bbWhitePawns() {
        return bbWhitePawns_;
    }
    constexpr Bitboard& bbWhiteKnights() {
        return bbWhiteKnights_;
    }
    constexpr Bitboard& bbWhiteBishops() {
        return bbWhiteBishops_;
    }
    constexpr Bitboard& bbWhiteRooks() {
        return bbWhiteRooks_;
    }
    constexpr Bitboard& bbWhiteQueens() {
        return bbWhiteQueens_;
    }
    constexpr Bitboard& bbWhiteKing() {
        return bbWhiteKing_;
    }

    // Black
    constexpr Bitboard& bbBlackPawns() {
        return bbBlackPawns_;
    }
    constexpr Bitboard& bbBlackKnights() {
        return bbBlackKnights_;
    }
    constexpr Bitboard& bbBlackBishops() {
        return bbBlackBishops_;
    }
    constexpr Bitboard& bbBlackRooks() {
        return bbBlackRooks_;
    }
    constexpr Bitboard& bbBlackQueens() {
        return bbBlackQueens_;
    }
    constexpr Bitboard& bbBlackKing() {
        return bbBlackKing_;
    }

    // Occupancy
    constexpr Bitboard& bbWhitePieces() {
        return bbWhitePieces_;
    }
    constexpr Bitboard& bbBlackPieces() {
        return bbBlackPieces_;
    }

private:
    // mailbox used to quickly find piece from square; not used for generating pieces
    std::array<Piece, Utils::NUM_SQUARES> mailbox_;

    // The game's current turn.
    Color sideToMove_;

    // Castling flags.
    CastlingRights castlingRights_;
    
    // Current en passant square. Is UndoInfo sentinal if no en passant.
    int currentEnPassantSquare_;

    // Bitboards to keep state
    // White
    Bitboard bbWhitePawns_;
    Bitboard bbWhiteKnights_;
    Bitboard bbWhiteBishops_;
    Bitboard bbWhiteRooks_;
    Bitboard bbWhiteQueens_;
    Bitboard bbWhiteKing_;

    // Black
    Bitboard bbBlackPawns_;
    Bitboard bbBlackKnights_;
    Bitboard bbBlackBishops_;
    Bitboard bbBlackRooks_;
    Bitboard bbBlackQueens_;
    Bitboard bbBlackKing_;

    // Occupancy
    Bitboard bbWhitePieces_;
    Bitboard bbBlackPieces_;

    // Attack bitboards
    AttackBitboards attackBitboards_;

    // Lookup table to bb by piece's packed uint8_t representation for quick access
    std::array<Bitboard*, 256> piecePackedToBB_{};

    // Lookup table to occupancy board by color's uint8_t representation
    std::array<Bitboard*, 4> colorToOccupancyBitboard_{nullptr, &bbWhitePieces_, &bbBlackPieces_};

    // init lookup tables
    void initAttackBitboards_();
    void initPieceToBBTable_();
    
    // Add move and all pawn promotion variants to moves. If move is not a pawn promotion, just add move by itself.
    static constexpr void addAllPawnPromotionsToMoves_(MoveList& moves, int sourceSquare, int targetSquare, Piece sourcePiece, bool isCapture) {
        const Color pawnColor = sourcePiece.color();
        const int promotionRow = pawnColor == Color::White ? 0 : 7; 
        if(Utils::getRow(targetSquare) == promotionRow) {
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

    // Generate all pseudo legal pawn moves.
    void generatePseudoLegalPawnMoves_(MoveList& out);
    // Generate all pseudo legal knight moves.
    void generatePseudoLegalKnightMoves_(MoveList& out);
    // Generate all pseudo legal bishop moves.
    void generatePseudoLegalBishopMoves_(MoveList& out);
    // Generate all pseudo legal rook moves.
    void generatePseudoLegalRookMoves_(MoveList& out);
    // Generate all pseudo legal queen moves.
    void generatePseudoLegalQueenMoves_(MoveList& out);
    // Generate all pseudo legal king moves.
    void generatePseudoLegalKingMoves_(MoveList& out);
};
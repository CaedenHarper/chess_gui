#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <string>

#include "Bitboard.hpp"

// Colors a piece can have. None represents an empty square.
enum class Color : uint8_t  {
    None,
    White,
    Black
};

// Types a piece can have. None represents an empty square.
enum class PieceType: uint8_t  {
    None,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

// A chess piece, with a piece type and color. Stored in uint8_t as (PieceType | Color).
class Piece {
public:
    // Construct an empty piece, which represents an empty square.
    constexpr Piece() noexcept : packed_{0} {}
    // Construct a piece given a type and color.
    constexpr Piece(PieceType type, Color color) noexcept : packed_{pack_(type, color)} {}

    // If a piece is equal to another piece. A piece is equal if the type and color matches.
    constexpr bool operator==(Piece other) const noexcept { return packed_ == other.packed_; }

    // Setters use bitwise ops to quickly extract info from packed_.
    constexpr PieceType type() const noexcept { return static_cast<PieceType>(packed_ & TYPE_MASK); }
    constexpr Color color() const noexcept { return static_cast<Color>((packed_ >> 3) & COLOR_MASK); }
    // Retrieve if the piece exists. i.e., if the piece is not an empty square.
    constexpr bool exists() const noexcept { return (packed_ & 0x7) != 0; } // 0 -> PieceType::None
    constexpr uint8_t raw() const noexcept { return packed_; }

    // Retrieve a string of length one which represents the piece. Uppercase for white, lowercase for black. E.g., white pawn -> "P"
    std::string to_string_short() const;
    // Retrieve a string which represents the piece. E.g., white pawn -> "White Pawn"
    std::string to_string_long() const;
    
    // Determine piece type from char. E.g., 'P' -> Pawn
    static PieceType charToPieceType(char piece);
    // Determine piece from char. Uppercase for white, lowercase for black. E.g., 'P' -> White Pawn
    static Piece charToPiece(char piece);

protected:
    // packed representation of PieceType | Color. 
    uint8_t packed_;

    // 3 bits for type, 2 for color, and corresponding bitmasks
    static constexpr uint8_t TYPE_BITS = 3;
    static constexpr uint8_t COLOR_BITS = 2;
    static constexpr uint8_t TYPE_MASK = (1 << TYPE_BITS) - 1;
    static constexpr uint8_t COLOR_MASK = (1 << COLOR_BITS) - 1;

    // Pack PieceType and Color into uint8_t
    static constexpr uint8_t pack_(PieceType type, Color color) noexcept {
        return (static_cast<uint8_t>(type) & TYPE_MASK) | ((static_cast<uint8_t>(color) & COLOR_MASK) << 3);
    }
};

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

enum class MoveFlag : uint8_t {
    Normal,  // no special flags
    Capture,
    DoublePawnPush,
    KingCastle,
    QueenCastle,
    EnPassant,
    Promotion,  // non-capture promotion
    PromotionCapture
};

enum class Promotion : uint8_t { 
    None,
    Knight,
    Bishop,
    Rook,
    Queen
};

class Game; // forward declare for Move

// TODO: rewrite without magic numbers here
// A chess move, with information for squares, pieces, and special flags like promotion and castling.
class Move {
public:
    // Create a default move that should not be used; only needed for MoveList to quickly initialize many default Moves
    constexpr Move() noexcept : packed_{0} {}
    // Create a move given a source square, target square, flag, and promotion.
    constexpr Move(uint8_t sourceSquare, uint8_t targetSquare, MoveFlag flag, Promotion promotion) noexcept
                : packed_{pack_(sourceSquare, targetSquare, flag, promotion)} {}
    constexpr Move(int sourceSquare, int targetSquare, MoveFlag flag, Promotion promotion) noexcept
                : packed_{pack_(static_cast<uint8_t>(sourceSquare), static_cast<uint8_t>(targetSquare), flag, promotion)} {}

    // Create a move using Pieces for context for flag + promotion. This is slower, only use if necessary / not in hot loop.
    static Move fromPieces(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece);

    // Has to have the same starting and target squares, pieces, and any special flags.
    constexpr bool operator==(Move other) const noexcept { return packed_ == other.packed_; }

    // Setters use bitwise ops to quickly extract info from packed_.
    constexpr uint8_t sourceSquare() const noexcept { return (packed_ >> 0) & 0x3F; }
    constexpr uint8_t targetSquare() const noexcept { return (packed_ >> 6) & 0x3F; }
    constexpr MoveFlag flag() const noexcept { return static_cast<MoveFlag>((packed_ >> 12) & 0xF); }
    constexpr Promotion promotion() const noexcept { return static_cast<Promotion>((packed_ >> 16) & 0x7); }
    constexpr bool isPromotion() const noexcept { return flag() == MoveFlag::Promotion || flag() == MoveFlag::PromotionCapture; }
    constexpr bool isEnPassant() const noexcept { return flag() == MoveFlag::EnPassant; }
    constexpr bool isDoublePawn() const noexcept { return flag() == MoveFlag::DoublePawnPush; }
    constexpr bool isKingSideCastle() const noexcept { return flag() == MoveFlag::KingCastle; }
    constexpr bool isQueenSideCastle() const noexcept { return flag() == MoveFlag::QueenCastle; }
    constexpr bool isCapture() const noexcept { return flag() == MoveFlag::Capture || flag() == MoveFlag::PromotionCapture || flag() == MoveFlag::EnPassant; }

    // Retrieve a string representation of the move. E.g., "White Pawn on e2 to Empty Square on e4".
    std::string to_string(const Game& game) const;
    // Retrieve a long algebraic representation of the move. E.g., "e2e4"
    std::string toLongAlgebraic() const;

    // Get piece type from promotion class.
    static constexpr PieceType promotionToPieceType(Promotion promotion) noexcept;

private:
    uint32_t packed_;

    static constexpr uint32_t pack_(uint8_t sourceSquare, uint8_t targetSquare, MoveFlag flag, Promotion promotion) noexcept {
        return (static_cast<uint32_t>(sourceSquare) & 0x3F)
             | ((static_cast<uint32_t>(targetSquare) & 0x3F) << 6)
             | ((static_cast<uint32_t>(flag) & 0xF) << 12)
             | ((static_cast<uint32_t>(promotion) & 0x7) << 16);
    }
};

// A list of moves. Wrapper for std::array<> for quick lookups. 
struct MoveList {
    // Max amount of moves; somewhat arbitrary, but should be enough for any pseudo-legal move count
    static constexpr int kMaxMoves = 256;
    std::array<Move, kMaxMoves> data;
    // Only moves between [0, MoveList.size) are valid; the rest are placeholder Moves which have undefined behavior
    int size = 0;

    constexpr void clear() noexcept { size = 0; }

    constexpr void push_back(const Move& move) noexcept {
        data[size++] = move;
    }
} __attribute__((aligned(128))); // align to 128 bytes

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
    static constexpr int NUM_SQUARES = 64;

    // stringview for constexpr
    // Starting game's FEN string.
    static constexpr std::string_view STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Constants for castling.
    static constexpr int WHITE_KING_STARTING_SQUARE = 60;
    static constexpr int BLACK_KING_STARTING_SQUARE = 4;

    static constexpr int WHITE_KINGSIDE_TARGET_SQUARE = 62;
    static constexpr int BLACK_KINGSIDE_TARGET_SQUARE = 6;

    static constexpr int WHITE_QUEENSIDE_TARGET_SQUARE = 58;
    static constexpr int BLACK_QUEENSIDE_TARGET_SQUARE = 2;

    static constexpr int WHITE_KINGSIDE_PASSING_SQUARE = 61;
    static constexpr int BLACK_KINGSIDE_PASSING_SQUARE = 5;

    static constexpr int WHITE_QUEENSIDE_PASSING_SQUARE = 59;
    static constexpr int BLACK_QUEENSIDE_PASSING_SQUARE = 3;

    static constexpr int WHITE_KINGSIDE_ROOK_STARTING_SQUARE = 63;
    static constexpr int BLACK_KINGSIDE_ROOK_STARTING_SQUARE = 7;

    static constexpr int WHITE_QUEENSIDE_ROOK_STARTING_SQUARE = 56;
    static constexpr int BLACK_QUEENSIDE_ROOK_STARTING_SQUARE = 0;

    // Constants for some piece's movements
    static constexpr std::array<std::array<int, 2>, 8> knightDeltas {{
        // col (x), row (y)
        {-2, -1}, // left up
        {-1, -2}, // up left
        {1, -2}, // up right
        {2, -1}, // right up
        {2, 1}, // right down
        {1, 2}, // down right
        {-1, 2}, // down left
        {-2, 1}, // left down
    }};

    static constexpr std::array<std::array<int, 2>, 4> bishopDeltas {{
        // col (x), row (y)
        {-1, -1}, // up left
        {1, -1}, // up right
        {-1, 1}, // down left
        {1, 1}, // down right
    }};

    static constexpr std::array<std::array<int, 2>, 4> rookDeltas {{
        // col (x), row (y)
        {0, 1}, // up
        {0, -1}, // down
        {1, 0}, // right
        {-1, 0}, // left
    }};

    static constexpr std::array<std::array<int, 2>, 8> queenDeltas {{
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
    }};

    static constexpr std::array<std::array<int, 2>, 8> kingDeltas {{
        // col (x), row (y)
        {-1, -1}, // up left
        {0, -1}, // up
        {1, -1}, // up right
        {-1, 0}, // left
        {1, 0}, // right
        {-1, 1}, // down left
        {0, 1}, // down
        {1, 1}, // down right
    }};

    // Construct a new game with an empty board. Current turn defaults to white.
    Game();
    // Update the board given a FEN.
    void loadFEN(const std::string& FEN);

    // Retrieve mailbox.
    constexpr std::array<Piece, NUM_SQUARES> mailbox() const noexcept { return mailbox_; }
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
    
    // Retrieve algebraic notation from a given square. E.g., 0 -> "a8".
    static std::string intToAlgebraicNotation(int square);
    // Retrieve square int from a given algebraic notation. E.g., "a8" -> 0.
    static int algebraicNotationToInt(const std::string& square);

    // If the square is on the board, in bounds.
    static constexpr bool onBoard(int square) noexcept {
        return 0 <= square && square <= 63;
    }
    // If a column and row is on the board, in bounds.
    static constexpr bool onBoard(int col, int row) noexcept {
        return 0 <= col && col <= 7 && 0 <= row && row <= 7;
    }

    // Get column of square.
    static constexpr int getCol(int square) noexcept {
        // equivalent to square % 8
        return square & 7;
    }
    // Get row of square.
    static constexpr int getRow(int square) noexcept {
        // equivalent to square / 8
        return square >> 3;
    }
    // Get square index from column and row.
    static constexpr int getSquareIndex(int col, int row) noexcept {
        // equivalent to (8 * row) + col
        return (row << 3) | col;
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

private:
    // mailbox used to quickly find piece from square; not used for generating pieces
    std::array<Piece, NUM_SQUARES> mailbox_;

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
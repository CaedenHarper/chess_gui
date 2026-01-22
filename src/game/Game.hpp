#pragma once

#include <array>
#include <cassert>
#include <optional>
#include <stdexcept>
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
    uint8_t prevWhiteKingSquare;
    uint8_t prevBlackKingSquare;

    static constexpr uint8_t noEnPassant = 255;

    constexpr UndoInfo(CastlingRights castlingRights,
                       uint8_t enPassantSquare,
                       Piece capturedPiece_,
                       uint8_t whiteKingSquare,
                       uint8_t blackKingSquare) noexcept
        : prevCastlingRights{castlingRights},
          prevEnPassantSquare{enPassantSquare},
          capturedPiece{capturedPiece_},
          prevWhiteKingSquare{whiteKingSquare},
          prevBlackKingSquare{blackKingSquare} {}
} __attribute__((aligned(8)));

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

    void clear() { size = 0; }

    void push_back(const Move& move) {
        data[size++] = move;
    }
} __attribute__((aligned(128)));


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

    // Retrieve board.
    std::array<Piece, NUM_SQUARES> board() const;
    // Retrieve the color of the current player's turn.
    Color currentTurn() const;
    // Retrieve a string representation of the current state of the board.
    std::string to_string() const;
    // If the game is finished.
    bool isFinished();
    // Get flags in the form of UndoInfo.
    UndoInfo getUndoInfo(Piece capturedPiece) const;
    // Get piece at a square for the GUI. Note this method is relatively slow and should not be used in hot loops.
    Piece pieceAtSquareForGui(int square) const noexcept;
    // Try a move and return if the move was made. The move is only made if it is legal.
    bool tryMove(const Move& move);
    // Make a move, even if it is not legal.
    void makeMove(const Move& move);
    // Undo a move. Does not check if the move we are undoing happened before.
    void undoMove(const Move& move, const UndoInfo& undoInfo);
    // If a move is legal.
    bool isMoveLegal(const Move& move);
    // Generate all legal moves from a given square.
    void generateLegalMoves(int sourceSquare, MoveList& out);
    // Generate all pseudo legal moves using the given current color.
    void generateAllPseudoLegalMoves(MoveList& out);
    // Generate all legal moves using the given current color.
    void generateAllLegalMoves(MoveList& out);
    // Attempt to parse arbitrary notation (e.g., "g1 f3" or "Nf3") to a move.
    std::optional<Move> parseMove(const std::string& move) const;

    // If the given color is in check.
    bool isInCheck(const Color& colorToFind) const;
    // If a given square is attacked by the attacking color.
    bool isSquareAttacked(int targetSquare, Color attackingColor) const;
    // Retrieve king square for a given color. Does not exist if king is not on board.
    std::optional<int> findKingSquare(const Color& colorToFind) const;
    
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

    // Get the nth bitboard, 0-5 -> white, 6-11 -> black
    constexpr Bitboard getBitboard(int n) noexcept {
        switch(n) {
            case 0: return bbWhitePawns_;
            case 1: return bbWhiteKnights_;
            case 2: return bbWhiteBishops_;
            case 3: return bbWhiteRooks_;
            case 4: return bbWhiteQueens_;
            case 5: return bbWhiteKing_;
            case 6: return bbBlackPawns_;
            case 7: return bbBlackKnights_;
            case 8: return bbBlackBishops_;
            case 9: return bbBlackRooks_;
            case 10: return bbBlackQueens_;
            case 11: return bbBlackKing_;
            default: return Bitboard{0};
        }
    }

private:
    // The game's board of pieces.
    std::array<Piece, NUM_SQUARES> board_;
    // The game's current turn.
    Color currentTurn_;

    // Castling flags.
    CastlingRights castlingRights_;

    // Keep track of current king positions. We assume they're on the board at all times (unknown behavior / crash if assumption is broken).
    int whiteKingSquare_;
    int blackKingSquare_;
    
    // TODO: extract en passant to its own class
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
    Bitboard bbAllPieces_;

    // recompute the occupancy bbWhitePieces, bbBlackPieces, bbAllPieces
    constexpr void recomputeOccupancy_() noexcept {
        // Recompute white pieces
        bbWhitePieces_.clear();
        bbWhitePieces_.mergeIn(bbWhitePawns_);
        bbWhitePieces_.mergeIn(bbWhiteKnights_);
        bbWhitePieces_.mergeIn(bbWhiteBishops_);
        bbWhitePieces_.mergeIn(bbWhiteRooks_);
        bbWhitePieces_.mergeIn(bbWhiteQueens_);
        bbWhitePieces_.mergeIn(bbWhiteKing_);

        // Recompute black pieces
        bbBlackPieces_.clear();
        bbBlackPieces_.mergeIn(bbBlackPawns_);
        bbBlackPieces_.mergeIn(bbBlackKnights_);
        bbBlackPieces_.mergeIn(bbBlackBishops_);
        bbBlackPieces_.mergeIn(bbBlackRooks_);
        bbBlackPieces_.mergeIn(bbBlackQueens_);
        bbBlackPieces_.mergeIn(bbBlackKing_);

        // Recompute all pieces
        bbAllPieces_.clear();
        bbAllPieces_.mergeIn(bbWhitePieces_);
        bbAllPieces_.mergeIn(bbBlackPieces_);
    }

    // Get a given piece type's bitboard.
    constexpr Bitboard& pieceToBitboard(PieceType type, Color color) {
        const bool isWhite = color == Color::White;
        switch(type) {
            case PieceType::Pawn: return isWhite ? bbWhitePawns_ : bbBlackPawns_;
            case PieceType::Knight: return isWhite ? bbWhiteKnights_ : bbBlackKnights_;
            case PieceType::Bishop: return isWhite ? bbWhiteBishops_ : bbBlackBishops_;
            case PieceType::Rook: return isWhite ? bbWhiteRooks_ : bbBlackRooks_;
            case PieceType::Queen: return isWhite ? bbWhiteQueens_ : bbBlackQueens_;
            case PieceType::King: return isWhite ? bbWhiteKing_ : bbBlackKing_;
            case PieceType::None: throw std::runtime_error("Bitboard does not exist.");
        }
    }

    // Get a given piece's bitboard.
    constexpr Bitboard& pieceToBitboard(Piece piece) {
        const bool isWhite = piece.color() == Color::White;
        const PieceType pieceType = piece.type();
        switch(pieceType) {
            case PieceType::Pawn: return isWhite ? bbWhitePawns_ : bbBlackPawns_;
            case PieceType::Knight: return isWhite ? bbWhiteKnights_ : bbBlackKnights_;
            case PieceType::Bishop: return isWhite ? bbWhiteBishops_ : bbBlackBishops_;
            case PieceType::Rook: return isWhite ? bbWhiteRooks_ : bbBlackRooks_;
            case PieceType::Queen: return isWhite ? bbWhiteQueens_ : bbBlackQueens_;
            case PieceType::King: return isWhite ? bbWhiteKing_ : bbBlackKing_;
            case PieceType::None: throw std::runtime_error("Bitboard does not exist for piece: " + piece.to_string_long());
        }
    }

    // Set bitboard based on given piece.
    void setBitboardForPiece(int square, Piece piece) {
        Bitboard& bitboard = pieceToBitboard(piece);
        bitboard.setSquare(square);
    }

    // Clear bitboard based on given piece.
    constexpr void clearBitboardForPiece(int square, Piece piece) {
        Bitboard& bitboard = pieceToBitboard(piece);
        bitboard.clearSquare(square);
    }

    // Attempt to parse long notation (e.g., "g1 f3") to a move.
    std::optional<Move> parseLongNotation_(const std::string& sourceMove, const std::string& targetMove) const;
    // Attempt to parse algebraic notation (e.g., "Nf3") to a move.
    std::optional<Move> parseAlgebraicNotation_(const std::string& move) const;

    // Add move and all pawn promotion variants to moves. If move is not a pawn promotion, just add move by itself.
    static void addAllPawnPromotionsToMoves_(MoveList& moves, int sourceSquare, int targetSquare, Piece sourcePiece, bool isCapture);

    // Generate all pseudo legal moves from a given square. Pseudo legal moves only take piece movement into account, no king check status.
    void generatePseudoLegalMoves_(int sourceSquare, MoveList& out);
    // Generate all pseudo legal pawn moves from a given square.
    void generatePseudoLegalPawnMoves_(int sourceSquare, MoveList& out);
    // Generate all pseudo legal knight moves from a given square.
    void generatePseudoLegalKnightMoves_(int sourceSquare, MoveList& out);
    // Generate all pseudo legal bishop moves from a given square.
    void generatePseudoLegalBishopMoves_(int sourceSquare, MoveList& out);
    // Generate all pseudo legal rook moves from a given square.
    void generatePseudoLegalRookMoves_(int sourceSquare, MoveList& out);
    // Generate all pseudo legal queen moves from a given square.
    void generatePseudoLegalQueenMoves_(int sourceSquare, MoveList& out);
    // Generate all pseudo legal king moves from a given square.
    void generatePseudoLegalKingMoves_(int sourceSquare, MoveList& out);
};
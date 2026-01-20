#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

// Colors a piece can have. None represents an empty square.
enum class Color {
    None,
    White,
    Black
};

// Types a piece can have. None represents an empty square.
enum class PieceType {
    None,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

// A chess piece, with a piece type and color.
class Piece {
public:
    // Construct an empty piece, which represents an empty square.
    Piece();
    // Construct a piece given a type and color.
    Piece(PieceType type, Color color);

    // If a piece is equal to another piece. A piece is equal if the type and color matches.
    bool operator==(Piece other) const { return type_ == other.type_ && color_ == other.color_; }

    // Retrieve the piece type.
    PieceType type() const;
    // Retrieve the piece color.
    Color color() const;
    // Retrieve if the piece exists. I.e., if the piece is not an empty square.
    bool exists() const;

    // Retrieve a string of length one which represents the piece. Uppercase for white, lowercase for black. E.g., white pawn -> "P"
    std::string to_string_short() const;
    // Retrieve a string which represents the piece. E.g., white pawn -> "White Pawn"
    std::string to_string_long() const;
    
    // Determine piece type from char. E.g., 'P' -> Pawn
    static PieceType charToPieceType(char piece);
    // Determine piece from char. Uppercase for white, lowercase for black. E.g., 'P' -> White Pawn
    static Piece charToPiece(char piece);

protected:
    // A piece's type.
    PieceType type_;
    // A piece's color.
    Color color_;
};

// Info used to fully undo a move.
struct UndoInfo {
    bool whiteKingSideCastle, whiteQueenSideCastle, blackKingSideCastle, blackQueenSideCastle;
    std::optional<int> enPassantSquare;

    UndoInfo(bool whiteKingSideCastle_, bool whiteQueenSideCastle_, bool blackKingSideCastle_, bool blackQueenSideCastle_, std::optional<int> enPassantSquare_) :
    whiteKingSideCastle{whiteKingSideCastle_},
    whiteQueenSideCastle{whiteQueenSideCastle_},
    blackKingSideCastle{blackKingSideCastle_},
    blackQueenSideCastle{blackQueenSideCastle_},
    enPassantSquare{enPassantSquare_} {}
} __attribute__((aligned(16)));

// A chess move, with information for squares, pieces, and special flags like promotion and castling.
class Move {
public:
    // Create a move given a source square, target square, source piece, and target piece. Determines if move is special, e.g., promotion or castling.
    Move(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece);

    // If a move is equal to another move. Has to have the same starting and target squares, pieces, and any special flags.
    bool operator==(Move other) const { return sourceSquare_ == other.sourceSquare_ &&
                                               targetSquare_ == other.targetSquare_ &&
                                            //    it should be fine if these are omitted, but im not actually sure...?
                                            //    sourcePiece_ == other.sourcePiece_ &&
                                            //    targetPiece_ == other.targetPiece_ &&
                                            //    isPawnPromotion_ == other.isPawnPromotion_ &&
                                               promotionPiece_ == other.promotionPiece_ &&
                                               isKingSideCastle_ == other.isKingSideCastle_ &&
                                               isQueenSideCastle_ == other.isQueenSideCastle_ &&
                                               isEnPassant_ == other.isEnPassant_; }
    // Retrieve source square.
    int sourceSquare() const { return sourceSquare_; }
    // Retrieve target square.
    int targetSquare() const { return targetSquare_; }
    // Retrieve source piece.
    Piece sourcePiece() const { return sourcePiece_; }
    // Retrieve target square.
    Piece targetPiece() const { return targetPiece_; }

    // Retrieve piece to promote to.
    Piece promotionPiece() const { return promotionPiece_; }
    // Set piece to promote to.
    void setPromotionPiece(Piece piece) { promotionPiece_ = piece; }

    // If a move is a pawn promotion.
    bool isPawnPromotion() const;
    // If a move is a king side castle.
    bool isKingSideCastle() const;
    // If a move is a queen side castle.
    bool isQueenSideCastle() const;
    // If a move is a double pawn move.
    bool isDoublePawn() const;
    // If a move is a en passant pawn move.
    bool isEnPassant() const;

    // Retrieve a string representation of the move. E.g., "White Pawn on e2 to Empty Square on e4".
    std::string to_string() const;
    // Retrieve a long algebraic representation of the move. E.g., "e2e4"
    std::string toLongAlgebraic() const;

private:
    // A move's source square.
    int sourceSquare_;
    // A move's target square.
    int targetSquare_;
    // A move's source piece.
    Piece sourcePiece_;
    // A move's target square.
    Piece targetPiece_;

    // TODO: add logic for promotion pieces other than queen
    // Piece to promote to. Undefined behavior if move is not a pawn promotion.
    Piece promotionPiece_;

    // all of these flags are evaluated once on move creation and saved for speedup and to allow more constness
    bool isPawnPromotion_;
    bool isKingSideCastle_;
    bool isQueenSideCastle_;
    bool isDoublePawn_;
    bool isEnPassant_;

};

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
    UndoInfo getUndoInfo() const;

    void setWhiteKingSideCastle(bool canCastle);
    void setBlackKingSideCastle(bool canCastle);
    void setWhiteQueenSideCastle(bool canCastle);
    void setBlackQueenSideCastle(bool canCastle);

    // Try a move and return if the move was made. The move is only made if it is legal.
    bool tryMove(const Move& move);
    // Make a move, even if it is not legal.
    void makeMove(const Move& move);
    // Undo a move. Does not check if the move we are undoing happened before.
    void undoMove(const Move& move, const UndoInfo& flags);
    // If a move is legal.
    bool isMoveLegal(const Move& move);
    // Generate all legal moves from a given square.
    std::vector<Move> generateLegalMoves(int sourceSquare);
    // Generate all pseudo legal moves using the given current color.
    std::vector<Move> generateAllPseudoLegalMoves();
    // Generate all legal moves using the given current color.
    std::vector<Move> generateAllLegalMoves();
    // Attempt to parse arbitrary notation (e.g., "g1 f3" or "Nf3") to a move.
    std::optional<Move> parseMove(const std::string& move) const;

    // If the given color is in check.
    bool isInCheck(const Color& colorToFind) const;
    // If a given square is attacked by the attacking color.
    bool isSquareAttacked(int targetSquare, const Color& attackingColor) const;
    // Retrieve king square for a given color. Does not exist if king is not on board.
    std::optional<int> findKingSquare(const Color& colorToFind) const;
    
    // Retrieve algebraic notation from a given square. E.g., 0 -> "a8".
    static std::string intToAlgebraicNotation(int square);
    // Retrieve square int from a given algebraic notation. E.g., "a8" -> 0.
    static int algebraicNotationToInt(const std::string& square);

    // If the square is on the board, in bounds.
    static bool onBoard(int square);
    // If a column and row is on the board, in bounds.
    static bool onBoard(int col, int row);

    // Get column of square.
    static int getCol(int square);
    // Get row of square.
    static int getRow(int square);
    // Get square index from column and row.
    static int getSquareIndex(int col, int row);

    // Get the opposite color of a given color.
    static Color oppositeColor(Color color);

private:
    // The game's board of pieces.
    std::array<Piece, NUM_SQUARES> board_;
    // The game's current turn.
    Color currentTurn_;
    // Castling flags.
    bool canWhiteKingSideCastle_;
    bool canBlackKingSideCastle_;
    bool canWhiteQueenSideCastle_;
    bool canBlackQueenSideCastle_;
    // Current en passant square. Does not exist if no en passant is possible on the board.
    std::optional<int> currentEnPassantSquare_;
    // Attempt to parse long notation (e.g., "g1 f3") to a move.
    std::optional<Move> parseLongNotation_(const std::string& sourceMove, const std::string& targetMove) const;
    // Attempt to parse algebraic notation (e.g., "Nf3") to a move.
    std::optional<Move> parseAlgebraicNotation_(const std::string& move) const;

    // Add move and all pawn promotion variants to moves. If move is not a pawn promotion, just add move by itself.
    static void addAllPawnPromotionsToMoves_(std::vector<Move>& moves, const Move& move);

    // Generate all pseudo legal moves from a given square. Pseudo legal moves only take piece movement into account, no king check status.
    std::vector<Move> generatePseudoLegalMoves_(int sourceSquare);
    // Generate all pseudo legal pawn moves from a given square.
    std::vector<Move> generatePseudoLegalPawnMoves_(int sourceSquare);
    // Generate all pseudo legal knight moves from a given square.
    std::vector<Move> generatePseudoLegalKnightMoves_(int sourceSquare);
    // Generate all pseudo legal bishop moves from a given square.
    std::vector<Move> generatePseudoLegalBishopMoves_(int sourceSquare);
    // Generate all pseudo legal rook moves from a given square.
    std::vector<Move> generatePseudoLegalRookMoves_(int sourceSquare);
    // Generate all pseudo legal queen moves from a given square.
    std::vector<Move> generatePseudoLegalQueenMoves_(int sourceSquare);
    // Generate all pseudo legal king moves from a given square.
    std::vector<Move> generatePseudoLegalKingMoves_(int sourceSquare);
};
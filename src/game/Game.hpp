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
    static PieceType charToPieceType(char c);
    // Determine piece from char. Uppercase for white, lowercase for black. E.g., 'P' -> White Pawn
    static Piece charToPiece(char c);

protected:
    // A piece's type.
    PieceType type_;
    // A piece's color.
    Color color_;
};

// A chess move, with information for squares, pieces, and special flags like promotion and castling.
class Move {
public:
    // Create a move given a source square, target square, source piece, and target piece. Determines if move is special, e.g., promotion or castling.
    Move(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece);

    // If a move is equal to another move. Has to have the same starting and target squares, pieces, and any special flags.
    bool operator==(Move other) const { return sourceSquare_ == other.sourceSquare_ &&
                                               targetSquare_ == other.targetSquare_ &&
                                               sourcePiece_ == other.sourcePiece_ &&
                                               targetPiece_ == other.targetPiece_ &&
                                               isPromotion_ == other.isPromotion_ &&
                                               promotionPiece_ == other.promotionPiece_; }
    // Retrieve source square.
    int sourceSquare() const;
    // Retrieve target square.
    int targetSquare() const;
    // Retrieve source piece.
    Piece sourcePiece() const;
    // Retrieve target square.
    Piece targetPiece() const;

    // Retrieve if this move is a pawn promotion.
    bool isPromotion() const;
    // Retrieve piece to promote to.
    Piece promotionPiece() const;

    // Retrieve if this move is a king side castle.
    bool isKingSideCastle() const;
    // Retrieve if this move is a king side castle.
    bool isQueenSideCastle() const;
    
    // Retrieve a string representation of the move. E.g., "White Pawn on e2 to Empty Square on e4".
    std::string to_string() const;

private:
    // A move's source square.
    const int sourceSquare_;
    // A move's target square.
    const int targetSquare_;
    // A move's source piece.
    const Piece sourcePiece_;
    // A move's target square.
    const Piece targetPiece_;

    // If a move is a pawn promotion.
    const bool isPromotion_;
    // TODO: add logic for promotion pieces other than queen
    // Piece to promote to. Undefined behavior if move is not a pawn promotion.
    const Piece promotionPiece_;
    
    // If a move is a king side castle.
    const bool isKingSideCastle_;
    // If a move is a queen side castle.
    const bool isQueenSideCastle_;

    // If a potential move is a pawn promotion.
    static bool isPotentialPawnPromotion_(int targetSquare, Piece sourcePiece);
    // If a potential move is a king side castle.
    static bool isPotentialKingSideCastle_(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece);
    // If a potential move is a queen side castle.
    static bool isPotentialQueenSideCastle_(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece);
};

// A chess game. Contains information for the game and helpers to generate and validate moves.
class Game {
public:
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

    // Construct a new game with an empty board. Current turn defaults to white.
    Game();
    // Update the board given a FEN.
    void loadFEN(std::string FEN);

    // Retrieve board.
    std::array<Piece, 64> board() const;
    // Retrieve the color of the current player's turn.
    Color currentTurn() const;
    // Retrieve a string representation of the current state of the board.
    std::string to_string() const;
    // If the game is finished.
    bool isFinished() const;

    void setWhiteKingSideCastle(bool b);
    void setBlackKingSideCastle(bool b);
    void setWhiteQueenSideCastle(bool b);
    void setBlackQueenSideCastle(bool b);

    // Try a move and return if the move was made. The move is only made if it is legal.
    bool tryMove(Move move);
    // Make a move, even if it is not legal.
    void makeMove(Move move);
    // Undo a move. Does not check if the move we are undoing happened before.
    void undoMove(Move move);
    // If a move is legal.
    bool isMoveLegal(Move move);
    // Generate all legal moves from a given square.
    std::vector<Move> generateLegalMoves(int sourceSquare);
    // Attempt to parse arbitrary notation (e.g., "g1 f3" or "Nf3") to a move.
    std::optional<Move> parseMove(std::string s) const;

    // If the given color is in check.
    bool isInCheck(Color colorToFind) const;
    // If a given square is attacked by the attacking color.
    bool isSquareAttacked(int targetSquare, Color attackingColor) const;
    // Retrieve king square for a given color. Does not exist if king is not on board.
    std::optional<int> findKingSquare(Color colorToFind) const;
    
    // Retrieve algebraic notation from a given square. E.g., "e4".
    static std::string intToAlgebraicNotation(int square);

    // If the square is on the board, in bounds.
    static bool onBoard(int square);
    // If a column and row is on the board, in bounds.
    static bool onBoard(int col, int row);

    // Get the opposite color of a given color.
    static Color oppositeColor(Color c);

private:
    // The game's board of pieces.
    std::array<Piece, 64> board_;
    // The game's current turn.
    Color currentTurn_;
    bool canWhiteKingSideCastle_;
    bool canBlackKingSideCastle_;
    bool canWhiteQueenSideCastle_;
    bool canBlackQueenSideCastle_;
    // Attempt to parse long notation (e.g., "g1 f3") to a move.
    std::optional<Move> parseLongNotation_(std::string sourceS, std::string targetS) const;
    // Attempt to parse algebraic notation (e.g., "Nf3") to a move.
    std::optional<Move> parseAlgebraicNotation_(std::string s) const;

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
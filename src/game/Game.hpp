#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

enum class Color {
    None,
    White,
    Black
};

enum class PieceType {
    None,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

class Piece {
public:
    Piece();
    Piece(PieceType type, Color color);
    bool operator==(Piece other) const { return type_ == other.type_ && color_ == other.color_; }
    PieceType type() const;
    Color color() const;
    bool exists() const;
    std::string to_string_short() const;
    std::string to_string_long() const;
    static PieceType charToPieceType(char c);
    static Piece charToPiece(char c);

protected:
    PieceType type_;
    Color color_;
};

class Move {
public:
    Move(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece);
    bool operator==(Move other) const { return sourceSquare_ == other.sourceSquare_ &&
                                               targetSquare_ == other.targetSquare_ &&
                                               sourcePiece_ == other.sourcePiece_ &&
                                               targetPiece_ == other.targetPiece_; }
    int sourceSquare() const;
    int targetSquare() const;
    Piece sourcePiece() const;
    Piece targetPiece() const;
    std::string to_string() const;

private:
    const int sourceSquare_;
    const int targetSquare_;
    const Piece sourcePiece_;
    const Piece targetPiece_;
};

class Game {
public:
    Game();
    Color currentTurn() const;
    std::string to_string() const;
    void loadFEN(const std::string FEN);
    bool isFinished() const;
    std::optional<Move> parseMove(const std::string s) const;
    bool tryMove(const Move move);
    void makeMove(const Move move);
    void undoMove(const Move move);
    bool isMoveLegal(const Move move);
    std::vector<Move> generateLegalMoves(int sourceSquare);
    std::vector<Move> generatePseudoLegalMoves(int sourceSquare);
    std::vector<Move> generatePseudoLegalPawnMoves(int sourceSquare);
    std::vector<Move> generatePseudoLegalKnightMoves(int sourceSquare);
    std::vector<Move> generatePseudoLegalBishopMoves(int sourceSquare);
    std::vector<Move> generatePseudoLegalRookMoves(int sourceSquare);
    std::vector<Move> generatePseudoLegalQueenMoves(int sourceSquare);
    std::vector<Move> generatePseudoLegalKingMoves(int sourceSquare);
    bool isSquareAttacked(int targetSquare, Color attackingColor) const;
    bool isInCheck(Color colorToFind) const;
    std::optional<int> findKingSquare(Color colorToFind) const;
    std::array<Piece, 64> board() const;
    static std::string intToAlgebraicNotation(int square);
    static bool onBoard(int square);
    static bool onBoard(int col, int row);
    static Color oppositeColor(Color c);

private:
    Color currentTurn_;
    std::array<Piece, 64> board_;
    std::optional<Move> parseLongNotation_(const std::string sourceS, const std::string targetS) const;
    std::optional<Move> parseAlgebraicNotation_(const std::string s) const;
};
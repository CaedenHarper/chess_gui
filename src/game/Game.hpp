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
    bool isMoveLegal(const Move move) const;
    std::vector<Move> generatePseudoLegalMoves(int sourceSquare) const;
    std::vector<Move> generatePseudoLegalPawnMoves(int sourceSquare) const;
    std::vector<Move> generatePseudoLegalKnightMoves(int sourceSquare) const;
    std::vector<Move> generatePseudoLegalBishopMoves(int sourceSquare) const;
    std::vector<Move> generatePseudoLegalRookMoves(int sourceSquare) const;
    std::vector<Move> generatePseudoLegalQueenMoves(int sourceSquare) const;
    std::vector<Move> generatePseudoLegalKingMoves(int sourceSquare) const;
    std::array<Piece, 64> board() const;
    static std::string intToAlgebraicNotation(int square);
    static bool onBoard(int square);

private:
    Color currentTurn_;
    bool canWhiteCastle_;
    bool canBlackCastle_;
    std::array<Piece, 64> board_;
    std::optional<Move> parseLongNotation_(const std::string sourceS, const std::string targetS) const;
    std::optional<Move> parseAlgebraicNotation_(const std::string s) const;
};
#include "Piece.hpp"

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
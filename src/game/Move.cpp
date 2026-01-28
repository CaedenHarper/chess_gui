#include <cassert>

#include "Move.hpp"
#include "Utils.hpp"

Move Move::fromPieces(int sourceSquare, int targetSquare, Piece sourcePiece, Piece targetPiece) {
    const int sourceCol = Utils::getCol(sourceSquare);
    const int sourceRow = Utils::getRow(sourceSquare);
    const int targetCol = Utils::getCol(targetSquare);
    const int targetRow = Utils::getRow(targetSquare);
    const bool isCapture = targetPiece.exists();
    const bool isSourcePiecePawn = sourcePiece.type() == PieceType::Pawn;
    const bool isSourcePieceWhite = sourcePiece.color() == Color::White;
    const int promotionRow = isSourcePieceWhite ? 0 : 7; // rank 7 for white, rank 0 for black
    
    // --- Special pawn moves ---
    // Pawn promotion iff a pawn is ending on the promotion row
    if(isSourcePiecePawn && targetRow == promotionRow) {
        const MoveFlag flag = isCapture ? MoveFlag::PromotionCapture : MoveFlag::Promotion;
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
    const int kingStartingSquare = isSourcePieceWhite ? Utils::WHITE_KING_STARTING_SQUARE : Utils::BLACK_KING_STARTING_SQUARE;
    const int kingsideTargetSquare = isSourcePieceWhite ? Utils::WHITE_KINGSIDE_TARGET_SQUARE : Utils::BLACK_KINGSIDE_TARGET_SQUARE;
    const int queensideTargetSquare = isSourcePieceWhite ? Utils::WHITE_QUEENSIDE_TARGET_SQUARE : Utils::BLACK_QUEENSIDE_TARGET_SQUARE;

    // --- Special castling moves ---
    // (note we are just generating a move here, we don't validate it. so its okay if we are castling through a piece for example)

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

std::string Move::toLongAlgebraic() const {
    if(isPromotion()) {
        // promotion requires an additional character representing which piece we promoted to
        char promotionRepresentation = '?';

        switch(promotion()) {
            case Promotion::None: assert(false); break; // should never happen
            case Promotion::Knight: promotionRepresentation = 'n'; break;
            case Promotion::Bishop: promotionRepresentation = 'b'; break;
            case Promotion::Rook: promotionRepresentation = 'r'; break;
            case Promotion::Queen: promotionRepresentation = 'q'; break;
        }

        return Utils::intToAlgebraicNotation(sourceSquare()) + Utils::intToAlgebraicNotation(targetSquare()) + promotionRepresentation;
    }

    // normal move
    return Utils::intToAlgebraicNotation(sourceSquare()) + Utils::intToAlgebraicNotation(targetSquare());
}
// TODO: move to CLI class
// std::optional<Move> Game::parseLongNotation_(const std::string& sourceMove, const std::string& targetMove) const {
//     // TODO: implement pawn promotion eventually
//     // We assume each string has at least two characters
//     if(sourceMove.length() < 2 || targetMove.length() < 2) {
//         // TODO: debug statement
//         std::cerr << "parseLongNotation: sourceS or targetS not long enough!\n";
//         return std::nullopt;
//     }

//     const char sourceColC = sourceMove.at(0);
//     const char sourceRowC = sourceMove.at(1);
//     const char targetColC = targetMove.at(0);
//     const char targetRowC = targetMove.at(1);

//     const int sourceCol = sourceColC - 'a';
//     const int sourceRow = sourceRowC - '0';
//     const int targetCol = targetColC - 'a';
//     const int targetRow = targetRowC - '0';

//     // all files and ranks should be in range 0 - 7
//     if(sourceCol < 0 || sourceCol > 7 
//     || sourceRow < 0 || sourceRow > 7
//     || targetCol < 0 || targetCol > 7 
//     || targetRow < 0 || targetRow > 7) {
//         // TODO: debug statement
//         std::cerr << "parseLongNotation: one file or rank is out of bounds\n";
//         return std::nullopt;
//     }

//     // rank 1 starts at at index 0, whereas Game handles the board starting from the top left (rank 8 being 0)
//     // therefore, we need to reflect it across the middle of the board with (8 - rank)
//     const int sourceSquare = getSquareIndex(sourceCol, 8 - sourceRow);
//     const int targetSquare = getSquareIndex(targetCol, 8 - targetRow); 

//     return Move::fromPieces(sourceSquare, targetSquare, board_.at(sourceSquare), board_.at(targetSquare));
// }

// std::optional<Move> Game::parseAlgebraicNotation_(const std::string& move) const { // NOLINT(readability-convert-member-functions-to-static) undo when finished
//     std::cerr << "parseAlgebraicNotation: Not yet implemented! " << move;
//     return std::nullopt;
// }

// std::optional<Move> Game::parseMove(const std::string& move) const {
//     int currentPart = 0;
//     std::string firstPart;
//     std::string secondPart;
//     // iterate through and split based on first part before space vs. second part. any unknown characters are ignored. any spaces after the first space are ignored.
//     for(const char c : move) { // NOLINT(readability-identifier-length)
//         if(c == ' ' && currentPart == 0) {
//             currentPart = 1;
//             continue;
//         }

//         // ignore any more spaces
//         if(c == ' ' && currentPart > 0) {
//             continue;
//         }

//         if(currentPart == 0) {
//             firstPart += c;
//             continue;
//         }

//         secondPart += c;
//     }

//     if(secondPart.empty()) {
//         return parseAlgebraicNotation_(firstPart);
//     }
//     return parseLongNotation_(firstPart, secondPart);
// }
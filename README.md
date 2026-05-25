# Chess (C++)

A C++ chess project with a working GUI, CLI, and engine.

## Features
- Move generation and validation.
- CLI and GUI for playing and debugging.
- Supports two-player games and one-player games against an engine.

## Requirements
- C++17 compiled with clang.
- CMake >= 4.2.1.
- Written in windows, should also compile with Unix (may have to change some commands below).

## Build
1. Create the build directory:
    ```
    cmake -S . -B build-release -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang -DCMAKE_BUILD_TYPE=Release  # Release build, -O3
    cmake -S . -B build-prof -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_FLAGS_RELWITHDEBINFO="-O3 -g -fno-omit-frame-pointer" -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -g -fno-omit-frame-pointer"  # Profiling build, -O3 without omitting frame pointer for profiling
    cmake -S . -B build-debug -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug  # Debug build, -O0
    ```
2. Build:
    ```
    cmake --build build-release  # or build-prof / build-debug
    ```
3. Executable is placed in `build-release\`.

## Run
From project root:
```
.\build-release\chess.exe  # release
.\build-prof\chess.exe     # profiling
.\build-debug\chess.exe    # debug
```

## Tests
- Tests are in `tests\`
- Run with:
```
ctest --test-dir build-release --verbose  # release
ctest --test-dir build-prof --verbose     # profling
ctest --test-dir build-debug --verbose    # debug
```

## TODOs (general)
- Look into migrating as many int types to their smallest representation as possible (e.g., uint8_t), and reducing static_cast<>'s
- Investigate struct byte alignments for performance; are they really needed?
- Investigate noexcept for performance

## TODOs (game)
- Incrementally update full board occupancy
- Create 'en passant square' class
- Split makeMove and undoMove into dispatch functions based on move type (e.g., makeMoveCastle_); they are a bit complex and hard to debug as of right now
- Implement magic bitboards
- PGN support
- Halfmove / fullmove support, in loadFEN and otherwise
- Drawing logic -- zobrist hashing

## TODOs (gui)
- Add proper finish / checkmate screen
- Fix bug where starting to drag when piece is already selected does not work properly
- Extract shared logic between renderers
- Extract shared logic between InputHandlers
- Speed up main menu -> game loading

## TODOs (testing)
- Unit tests for Piece, Move, Game
- Verify all incremental building is correct in Game (e.g., maybe perft with material status?)
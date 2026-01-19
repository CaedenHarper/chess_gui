# Chess (C++)

A C++ chess project with a working GUI, CLI, and engine.

## Features
- Move generation and validation.
- CLI and GUI for playing and debugging.
- Supports two-player games and one-player games against an engine.

## Requirements
- C++17 compiled with clang.
- CMake >= 4.2.1.
- Written in windows, should compile with Unix (may have to change some commands below).

## Build
1. Create the build directory:
    ```
    cmake -S . -B build-release -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang -DCMAKE_BUILD_TYPE=Release
    ```
2. Build:
    ```
    cmake --build build-release
    ```
3. Executable is placed in `build-release\`.

## Run
From project root:
```
.\build-release\chess.exe   # Windows
```

## Tests
- Tests are in tests\
- Run with:
```
ctest --test-dir build-release --verbose
```
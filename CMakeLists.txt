cmake_minimum_required(VERSION 4.2.1)

# Create config file for VSCode
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

project(
    chess
    VERSION 0.1.0
    LANGUAGES CXX
)

# Include SFML from github
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_MakeAvailable(SFML)

# Clang only
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "This project is intended to be built with Clang only.")
endif()

add_executable(chess
    src/main.cpp
    src/game/Game.cpp
    src/gui/Board.cpp
    src/engine/Engine.cpp
)

# link sfml
target_link_libraries(chess PRIVATE SFML::Audio SFML::Graphics)

# Built with c++17
target_compile_features(chess PRIVATE cxx_std_17)
target_compile_options(chess PRIVATE -Wall -Wextra -Wpedantic)

# Enable testing by exporting chess library
add_library(chess_lib
    src/game/Game.cpp
    src/gui/Board.cpp
    src/engine/Engine.cpp
)
target_include_directories(chess_lib PUBLIC include)

target_link_libraries(chess_lib PUBLIC SFML::Graphics SFML::Window)

enable_testing()
add_subdirectory(tests)
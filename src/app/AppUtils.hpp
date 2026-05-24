#pragma once

#include <SFML/Graphics.hpp>

#include <string_view>

namespace AppUtils {
// -- CONSTANTS --
constexpr int STARTING_WINDOW_WIDTH = 1000;
constexpr int STARTING_WINDOW_HEIGHT = 1000;
constexpr std::string_view WINDOW_TITLE = "Chess";
constexpr int MAX_FPS = 144;
} // namespace AppUtils
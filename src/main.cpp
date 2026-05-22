#include "app/Application.hpp"

int main() {
    const Color playerColor = Color::White;
    Application app{playerColor};
    app.run();
}
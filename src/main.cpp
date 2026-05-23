#include "app/Application.hpp"

int main() {
    const Color playerColor = Color::Black;
    Application app{playerColor};
    app.run();
}
#include <raylib.h>
#include "game.hpp"

int main() {
    // Enable MSAA 4X for smooth anti-aliased isometric rails and lines
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    SetTraceLogLevel(LOG_WARNING);

    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "COASTER GRID - 2.5D Isometric Coaster Tycoon");
    SetTargetFPS(60);

    Game game;

    while (!game.ShouldClose()) {
        game.HandleInput();
        game.Update(GetFrameTime());
        game.Draw();
    }

    CloseWindow();
    return 0;
}

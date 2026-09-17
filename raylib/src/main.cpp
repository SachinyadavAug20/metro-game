#include <raylib.h>
#include "game.hpp"
#include "font_system.hpp"

#include <algorithm>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    static Game* g_game = nullptr;
    static void UpdateDrawFrame() {
        float dt = std::min(GetFrameTime(), 0.1f);
        g_game->HandleInput();
        g_game->Update(dt);
        g_game->Draw();
    }
#endif

int main() {
#if defined(PLATFORM_WEB)
    SetConfigFlags(FLAG_MSAA_4X_HINT);
#else
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
#endif
    SetTraceLogLevel(LOG_WARNING);

    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "METRO GRID - 2.5D Urban Transit Simulator");
    SetTargetFPS(60);

    InitGameFont();

#if defined(PLATFORM_WEB)
    g_game = new Game();
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    Game game;
    while (!game.ShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.1f);
        game.HandleInput();
        game.Update(dt);
        game.Draw();
    }
    CleanupGameFont();
    CloseWindow();
#endif
    return 0;
}

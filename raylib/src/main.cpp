#include <raylib.h>
#include "game.hpp"
#include "font_system.hpp"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    static Game* g_game = nullptr;
    static void UpdateDrawFrame() {
        g_game->HandleInput();
        g_game->Update(GetFrameTime());
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

    Game game;

#if defined(PLATFORM_WEB)
    g_game = &game;
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!game.ShouldClose()) {
        game.HandleInput();
        game.Update(GetFrameTime());
        game.Draw();
    }
#endif

    CleanupGameFont();
    CloseWindow();
    return 0;
}

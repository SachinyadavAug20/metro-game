#include <raylib.h>
#include "game.hpp"
#include <iostream>

class TestGame : public Game {
public:
    void SetCamera(Vector2 pos, float z) {
        cameraPos = pos;
        zoom = z;
    }
    void OpenStaffWindow(bool open) {
        staffWindowOpen = open;
    }
    void OpenStatsWindow(bool open) {
        statsWindowOpen = open;
    }
    void SelectPeep(int idx) {
        selectedPeepIdx = idx;
    }
    void SetWeekTimer(float t) {
        weekTimer = t;
    }
    void RepaintCoaster(Color c, const std::string& name) {
        cachedStats.themeColor = c;
        cachedStats.coasterName = name;
        tracks.SetTrackColor(c);
        train.SetTrainTheme(c);
        train.SetCoasterName(name);
    }
    void AddMesses() {
        ParkMess v1;
        v1.pos = {4.5f, 12.0f};
        v1.isVomit = true;
        messes.push_back(v1);

        ParkMess c1;
        c1.pos = {5.2f, 12.2f};
        c1.isVomit = false;
        messes.push_back(c1);
    }
    void PositionStaff() {
        if (!staff.empty()) {
            staff[0].pos = {4.2f, 12.0f};
            staff[0].targetPos = {4.5f, 12.0f};
            staff[0].isWorking = true;
            staff[0].workTimer = 0.8f;
        }
        if (staff.size() > 1) {
            staff[1].pos = {7.5f, 12.5f};
            staff[1].isWorking = true;
            staff[1].workTimer = 1.2f;
        }
    }
    void Step(float dt) {
        Update(dt);
    }
    void Render() {
        Draw();
    }
};

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIDDEN);
    SetTraceLogLevel(LOG_WARNING);

    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "COASTER GRID - Iteration Capture");
    SetTargetFPS(60);

    // 1. Twilight / Night lighting with glowing lampposts and train headlamp
    {
        TestGame game;
        game.AddMesses();
        game.PositionStaff();
        for (int i = 0; i < 150; ++i) game.Step(0.033f);
        game.SetWeekTimer(52.0f); // 52s = Night twilight phase
        game.Render();
        TakeScreenshot("test_night_lighting.png");
        std::cout << "Captured test_night_lighting.png\n";
    }

    // 2. Coaster Repainted to Emerald Viper with Palette window open
    {
        TestGame game;
        game.RepaintCoaster(Color{16, 185, 129, 255}, "The Emerald Viper");
        game.OpenStatsWindow(true);
        for (int i = 0; i < 100; ++i) game.Step(0.033f);
        game.Render();
        TakeScreenshot("test_repainted_coaster_emerald.png");
        std::cout << "Captured test_repainted_coaster_emerald.png\n";
    }

    // 3. Peep Inspector with Balloon and Handyman at work
    {
        TestGame game;
        game.AddMesses();
        game.PositionStaff();
        game.SetCamera({600.0f, 20.0f}, 1.25f);
        for (int i = 0; i < 120; ++i) game.Step(0.033f);
        game.SelectPeep(0);
        game.PositionStaff();
        game.Render();
        TakeScreenshot("test_peep_balloon_inspector.png");
        std::cout << "Captured test_peep_balloon_inspector.png\n";
    }

    CloseWindow();
    return 0;
}

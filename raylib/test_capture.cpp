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
    void OpenManualOverlay(bool open) {
        helpOverlayOpen = open;
    }
    void StartGame() {
        state = STATE_PLAYING;
    }
    void SelectCommuter(int idx) {
        selectedPeepIdx = idx;
    }
    void SetWeekTimer(float t) {
        weekTimer = t;
    }
    void RepaintRoute(Color c, const std::string& name) {
        cachedStats.themeColor = c;
        cachedStats.lineName = name;
        tracks.SetTrackColor(c);
        train.SetTrainTheme(c);
        train.SetLineName(name);
    }
    void AddStationMesses() {
        StationMess m1;
        m1.pos = {4.5f, 12.0f};
        m1.isSpill = true;
        messes.push_back(m1);

        StationMess m2;
        m2.pos = {5.2f, 12.2f};
        m2.isSpill = false;
        messes.push_back(m2);
    }
    void PositionCrew() {
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
    void SetHoveredGrid(int gx, int gy) {
        hoveredGx = gx;
        hoveredGy = gy;
    }
    void SetBuildTool(TrackType t, Direction heading) {
        activeTab = CAT_TRACK;
        currentTrack = t;
        buildHeading = heading;
        isBulldozing = false;
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
    InitWindow(screenWidth, screenHeight, "METRO GRID - Inspection Capture");
    SetTargetFPS(60);

    TestGame game;
    game.AddStationMesses();
    game.PositionCrew();

    // 0. Start Menu & Instructions Screen (STATE_TITLE)
    game.Render();
    TakeScreenshot("test_metro_start_menu.png");
    std::cout << "Captured test_metro_start_menu.png\n";

    // Start playing
    game.StartGame();

    // 1. Initial State at Central Hub: Entrance, Plaza, Fountain, Central Hub Platform
    game.SetCamera({660.0f, -30.0f}, 1.0f);
    game.Render();
    TakeScreenshot("test_metro_hub_island_platform.png");
    std::cout << "Captured test_metro_hub_island_platform.png\n";

    // 2. Simulate 120 steps: Train boards commuters at Central Hub, collects fares with floating text
    for (int i = 0; i < 120; ++i) game.Step(0.033f);
    game.Render();
    TakeScreenshot("test_metro_boarding_simulation.png");
    std::cout << "Captured test_metro_boarding_simulation.png\n";

    // 3. Elevated SkyTrain Viaduct over Canal
    game.SetCamera({540.0f, -120.0f}, 1.15f);
    for (int i = 0; i < 150; ++i) game.Step(0.033f);
    game.Render();
    TakeScreenshot("test_metro_viaduct_signaling.png");
    std::cout << "Captured test_metro_viaduct_signaling.png\n";

    // 4. Line Operations Window: RCT-Style Mode (Open/Test/Closed), Fleet Formation, Fare Pricing
    game.SetCamera({660.0f, -30.0f}, 1.0f);
    game.OpenStatsWindow(true);
    game.SelectCommuter(0);
    game.Render();
    TakeScreenshot("test_metro_occ_dashboard.png");
    std::cout << "Captured test_metro_occ_dashboard.png\n";

    // 5. Transit Crew & Station Maintenance Window
    game.OpenStatsWindow(false);
    game.SelectCommuter(-1);
    game.OpenStaffWindow(true);
    game.Render();
    TakeScreenshot("test_metro_crew_management.png");
    std::cout << "Captured test_metro_crew_management.png\n";

    // 6. OCC Operations Manual
    game.OpenStaffWindow(false);
    game.OpenManualOverlay(true);
    game.Render();
    TakeScreenshot("test_metro_operations_manual.png");
    std::cout << "Captured test_metro_operations_manual.png\n";

    // 7. Long Simulation Loop Test: Run 600 steps to verify infinite loop without stall
    game.OpenManualOverlay(false);
    for (int i = 0; i < 600; ++i) game.Step(0.033f);
    game.Render();
    TakeScreenshot("test_metro_infinite_loop_proof.png");
    std::cout << "Captured test_metro_infinite_loop_proof.png\n";

    CloseWindow();
    return 0;
}

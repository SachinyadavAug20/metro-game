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

    // 1. Central Hub Island Platform with EMU Train, PSDs, PIDS, Commuters with shape badges
    game.SetCamera({680.0f, -80.0f}, 1.0f);
    for (int i = 0; i < 40; ++i) game.Step(0.033f);
    game.Render();
    TakeScreenshot("test_metro_hub_island_platform.png");
    std::cout << "Captured test_metro_hub_island_platform.png\n";

    // 2. Elevated SkyTrain Concrete Viaduct & Wayside 3-Aspect Signaling Mast over Canal
    for (int i = 0; i < 180; ++i) game.Step(0.033f);
    game.SetCamera({560.0f, -140.0f}, 1.25f);
    game.Render();
    TakeScreenshot("test_metro_viaduct_signaling.png");
    std::cout << "Captured test_metro_viaduct_signaling.png\n";

    // 3. Operations Control Center (OCC) Dashboard with Line Operations & Commuter Inspector
    game.OpenStatsWindow(true);
    game.SelectCommuter(0);
    game.SetCamera({680.0f, -80.0f}, 1.0f);
    game.Render();
    TakeScreenshot("test_metro_occ_dashboard.png");
    std::cout << "Captured test_metro_occ_dashboard.png\n";

    // 4. Passenger Concourse, Subway Entrance Stairwell, Smartcard Gates & Newsstand
    game.OpenStatsWindow(false);
    game.SelectCommuter(-1);
    game.SetCamera({720.0f, -80.0f}, 1.35f);
    for (int i = 0; i < 30; ++i) game.Step(0.033f);
    game.Render();
    TakeScreenshot("test_metro_concourse_turnstiles.png");
    std::cout << "Captured test_metro_concourse_turnstiles.png\n";

    // 5. Transit Crew & Station Maintenance Window
    game.OpenStaffWindow(true);
    game.Render();
    TakeScreenshot("test_metro_crew_management.png");
    std::cout << "Captured test_metro_crew_management.png\n";

    // 6. OCC Operations Manual (Overhauled 3-Step Clear Guide)
    game.OpenStaffWindow(false);
    game.OpenManualOverlay(true);
    game.Render();
    TakeScreenshot("test_metro_operations_manual.png");
    std::cout << "Captured test_metro_operations_manual.png\n";

    // 7. L-Turn Rail Construction Ghost with Directional Arrow
    game.OpenManualOverlay(false);
    game.SetCamera({660.0f, -80.0f}, 1.35f);
    game.SetBuildTool(TRACK_CURVE_LEFT, DIR_EAST);
    game.SetHoveredGrid(10, 11);
    game.Render();
    TakeScreenshot("test_metro_lturn_placement.png");
    std::cout << "Captured test_metro_lturn_placement.png\n";

    // 8. In-Game HUD Quick Tip Banner & Streamlined Toolbar Labels
    game.SetBuildTool(TRACK_STRAIGHT, DIR_EAST);
    game.SetHoveredGrid(5, 12);
    game.Render();
    TakeScreenshot("test_metro_simple_instructions.png");
    std::cout << "Captured test_metro_simple_instructions.png\n";

    CloseWindow();
    return 0;
}

#include <raylib.h>
#include "game.hpp"
#include "font_system.hpp"
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
    void BuyExtraTrainNow() {
        float cost = ExtraTrainCostP(GetExtraTrainCount());
        if (GetExtraTrainCount() < MaxExtraTrains() && economy.balance >= cost) {
            economy.balance -= cost;
            MetroTrain nt;
            nt.SetCarriageCount(3);
            nt.SetTrainTheme(train.GetTrainTheme());
            int index = GetExtraTrainCount();
            nt.Reset(tracks, 0.5f + tracks.GetTotalCircuitLength() * (0.5f + 0.18f * index));
            extraTrains.push_back(nt);
        }
    }
    void GiveCash(double amt) { economy.balance = (float)amt; }
    void SetBriefing(float t) { briefingTimer = t; }
    void SetBest(int b) { bestSessionRiders = b; }
    void ForceRush(bool on) { rushHourActive = on; }
    void ForcePause(bool on) { isPaused = on; }
    void ForceResultsScreen() { economy.totalDelivered = 512; week = 3; state = STATE_VICTORY; }
    void ForceGameOverScreen() { economy.totalDelivered = 343; state = STATE_GAME_OVER; }
    void ForceSpotlightLesson() {
        piecesPlaced = 5;
        stationsPlaced = 1;
        sceneryPlaced = 3;
        viaductPiecesPlaced = 1;
    }
    void ResetTutorialTracking() {
        piecesPlaced = 0;
        stationsPlaced = 0;
        sceneryPlaced = 0;
        viaductPiecesPlaced = 0;
        bulldozeUses = 0;
        rideCamUsed = false;
    }
    float GetDistance() const { return train.GetTrainDistance(); }
    float GetDistance2() const { return extraTrains.empty() ? 0.0f : extraTrains[0].GetTrainDistance(); }
    int GetDelivered() const { return economy.totalDelivered; }
    int FleetCount() const { return GetExtraTrainCount(); }
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

    InitGameFont();

    TestGame game;
    game.AddStationMesses();
    game.PositionCrew();

    // 0. Start Menu & Instructions Screen (STATE_TITLE) with session high-score
    game.SetBest(250);
    game.Render();
    TakeScreenshot("test_metro_start_menu.png");
    std::cout << "Captured test_metro_start_menu.png\n";

    // Start playing
    game.StartGame();

    // 0c. Arcade PAUSE overlay + RUSH HOUR banner (flickering juice)
    game.ForceRush(true);
    game.Render();
    TakeScreenshot("test_metro_rush_banner.png");
    std::cout << "Captured test_metro_rush_banner.png\n";
    game.ForceRush(false);
    game.ForcePause(true);
    game.Render();
    TakeScreenshot("test_metro_paused.png");
    std::cout << "Captured test_metro_paused.png\n";
    game.ForcePause(false);

    // 0b. Arcade MISSION BRIEFING overlay + spotlight shop lesson (tutorial wayfinding)
    game.SetBriefing(4.5f);
    game.Render();
    TakeScreenshot("test_metro_briefing.png");
    std::cout << "Captured test_metro_briefing.png\n";
    game.SetBriefing(0.0f);
    game.ForceSpotlightLesson();
    game.Render();
    TakeScreenshot("test_metro_spotlight_lesson.png");
    std::cout << "Captured test_metro_spotlight_lesson.png\n";
    game.ResetTutorialTracking();
    game.Render();

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

    // 7. Buy a Fleet (2 extra trains) and verify they all run & deliver
    int dBefore = game.GetDelivered();
    game.GiveCash(10000.0);
    game.BuyExtraTrainNow();
    game.BuyExtraTrainNow();
    for (int i = 0; i < 400; ++i) game.Step(0.033f);
    int dAfter = game.GetDelivered();
    std::cout << "Extra fleet delivered +" << (dAfter - dBefore)
              << " commuters (fleet=" << game.FleetCount()
              << " d1=" << game.GetDistance() << " d2=" << game.GetDistance2() << ")\n";
    game.Render();
    TakeScreenshot("test_metro_extra_train.png");
    std::cout << "Captured test_metro_extra_train.png\n";
    if (dAfter - dBefore < 5 || game.FleetCount() != 2) {
        std::cout << "TEST FAIL: extra fleet not contributing\n";
        return 1;
    }

    // 8. Long Simulation Loop Test: Run 600 steps to verify infinite loop without stall
    game.OpenManualOverlay(false);
    for (int i = 0; i < 600; ++i) game.Step(0.033f);
    game.Render();
    TakeScreenshot("test_metro_infinite_loop_proof.png");
    std::cout << "Captured test_metro_infinite_loop_proof.png\n";

    // 9. Arcade result screens (star rating + flicker + RESTART)
    game.ForceResultsScreen();
    game.Render();
    TakeScreenshot("test_metro_victory.png");
    std::cout << "Captured test_metro_victory.png\n";
    game.ForceGameOverScreen();
    game.Render();
    TakeScreenshot("test_metro_gameover.png");
    std::cout << "Captured test_metro_gameover.png\n";

    CleanupGameFont();
    CloseWindow();
    return 0;
}

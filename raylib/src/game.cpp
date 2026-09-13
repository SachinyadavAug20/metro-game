#include "game.hpp"
#include "font_system.hpp"
#include <algorithm>

Game::Game() {
    Init();
}

Game::~Game() {
    AudioManager::Cleanup();
}

void Game::Init() {
    AudioManager::Init();

    int screenW = GetScreenWidth();
    if (screenW <= 0) screenW = 1280;
    int screenH = GetScreenHeight();
    if (screenH <= 0) screenH = 720;

    zoom = 1.2f;
    // Center camera on the metropolitan transit circuit and Central Hub
    cameraPos = {(float)screenW / 2.0f + 20.0f * zoom, ((float)screenH / 2.0f) - 390.0f * zoom};

    SetupInitialPark();
    tracks.InitDefaultCircuit();
    train.Reset(tracks);
    extraTrains.clear();
    peeps.Init({1.0f, 9.0f}, {7.0f, 8.0f}, {6.0f, 10.0f});
    particles.Clear();

    economy.balance = 2500.0f;
    economy.baseFare = 2.50f;
    economy.totalDelivered = 0;
    cachedStats.ticketFare = 2.50f;
    train.SetTicketFare(2.50f);

    parkRating = 92.0f;
    angryLeaves = 0;
    week = 1;
    weekTimer = 0.0f;
    gameSpeed = 1;
    state = STATE_TITLE;
    lastMilestoneAwarded = 0;
    rushCombo = 1.0f;
    comboTimer = 0.0f;
    comboStreak = 0;
    endlessMode = false;

    statsWindowOpen = false;
    staffWindowOpen = false;
    rideCamActive = false;
    helpOverlayOpen = false;
    selectedPeepIdx = -1;
    panMoves = 0;
    lastArmedLesson = -1;

    // Staff & Cleanliness
    staff.clear();
    messes.clear();
    parkCleanliness = 100.0f;
    staffWageTimer = 0.0f;

    StaffMember custodian;
    custodian.name = "Kenji (Custodian)";
    custodian.type = STAFF_CUSTODIAN;
    custodian.pos = {3.0f, 9.0f};
    custodian.targetPos = {3.0f, 9.0f};
    staff.push_back(custodian);

    StaffMember engineer;
    engineer.name = "Sato (Signal Engineer)";
    engineer.type = STAFF_ENGINEER;
    engineer.pos = {7.0f, 9.0f};
    engineer.targetPos = {7.0f, 9.0f};
    staff.push_back(engineer);

    ShowToast("[METRO] Welcome to METRO GRID! Line 1 Service Active | Press [H] for Manual", Color{56, 189, 248, 255}, 4.5f);
}

void Game::ShowToast(const std::string& text, Color color, float duration) {
    activeToast.text = text;
    activeToast.color = color;
    activeToast.timer = duration;
}

bool Game::IsBuildable(int gx, int gy) const {
    if (gx < 0 || gx >= GRID_SIZE || gy < 0 || gy >= GRID_SIZE) return false;
    int d = std::abs(gx - LAND_CENTER_X) + std::abs(gy - LAND_CENTER_Y);
    return d <= buildRadius;
}

void Game::ReclaimLand() {
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            if (IsBuildable(x, y) && terrain[x][y] == GROUND_WATER) {
                terrain[x][y] = GROUND_GRASS;
            }
        }
    }
    particles.SpawnConfetti(Vector3{(float)LAND_CENTER_X + 2.0f, (float)LAND_CENTER_Y - 2.0f, 0.5f}, 24);
    particles.SpawnFloatingText(Vector3{(float)LAND_CENTER_X + 2.0f, (float)LAND_CENTER_Y - 2.0f, 1.2f}, "NEW ISLAND!", Color{74, 222, 128, 255});
    ShowToast(TextFormat("LAND EXPANDED! Buildable island ring is now %d tiles wide.", buildRadius), Color{34, 197, 94, 255}, 4.0f);
    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
}

static const TutorialStageInfo kTutorialStages[] = {
    {"YOUR TRAIN IS RUNNING!",
     "Watch the red EMU circle the island on the pre-built loop.",
     "Just watch - riders will board at stations.",
     -1, -1, true, 0},
    {"DELIVER 10 COMMUTERS",
     "Riders board, ride, and leave at their station. Deliver 10.",
     "Watch the RIDERS counter at the top.",
     7, 9, false, 0},
    {"BUILD 5 TRACK PIECES",
     "Straight Track is armed for you. Click the glowing grass 5 times.",
     "Each piece costs $40.",
     9, 13, false, 0},
    {"ADD A STATION TO THE LINE",
     "Station tool is armed. Click a track tile to add boarding.",
     "Stations pay a big fare bonus.",
     14, 9, false, 0},
    {"PLACE 3 PIECES OF SCENERY",
     "Tree tool is armed. Click grass 3 times to plant.",
     "Scenery keeps the city happy.",
     7, 13, false, 0},
    {"LEARN HEIGHT: CROSS THE CANAL",
     "Viaduct is armed and raised to Z=1. Click the water to stride across.",
     "Elevated rail dodges the canal.",
     12, 8, false, 0},
    {"BUY A BIGGER TRAIN  (CAR+1)",
     "Click the blue CAR+1 button for $800 - more seats, faster boarding.",
     "Low on cash? Deliver riders first.",
     0, 0, false, 1},
    {"EXPAND THE ISLAND  (LAND)",
     "Click the green LAND button ($600) to reclaim sea as buildable ground.",
     "You cannot build on water.",
     0, 0, false, 2},
    {"BUY A SECOND TRAIN  (EXTRA $1,400)",
     "Click the blue EXTRA TRAIN button - a second EMU doubles your throughput.",
     "Each extra train costs more than the last.",
     0, 0, false, 3},
    {"ROTATE + DEMOLISH",
     "Bulldozer is armed. Click any tile to remove it.",
     "Bulldoze is your undo button.",
     -1, -1, false, 0},
    {"NAVIGATE THE MAP",
     "Explore the city: WASD/arrows pan, drag with the right or middle mouse button, or slide the mouse to the screen edge.",
     "Then press HOME to snap back to the train.",
     -1, -1, false, 0},
    {"RIDE THE DRIVER'S CAB  (F)",
     "Press F to ride along in the cab. Press F again to return to map view.",
     "The cab follows the rails by itself.",
     -1, -1, false, 0},
    {"FINAL LESSON: SERVE 75 COMMUTERS",
     "Keep the loop closed and clear crowded platforms. Serve 75 to finish onboarding!",
     "Then: 500 riders wins. Keep growing in Endless Mode!",
     -1, -1, false, 0}
};
static const int kTutorialCount = 13;

bool Game::GetTutorialDone(int idx) const {
    switch (idx) {
        case 0: return train.GetSpeedKmh() > 0.5f;      // train running
        case 1: return economy.totalDelivered >= 10;
        case 2: return piecesPlaced >= 5;
        case 3: return stationsPlaced >= 1;
        case 4: return sceneryPlaced >= 3;
        case 5: return viaductPiecesPlaced >= 1;        // height: built one elevated piece
        case 6: return train.GetCarriageCount() >= 4;   // bought a train car
        case 7: return buildRadius > 14;                // bought land / expanded island
        case 8: return !extraTrains.empty();            // bought an extra train
        case 9: return bulldozeUses >= 1;               // tried the bulldozer
        case 10: return panMoves >= 3;                  // roamed the camera
        case 11: return rideCamUsed;                    // tried the cab camera
        case 12: return economy.totalDelivered >= 75;
        default: return true;
    }
}

bool Game::GetTutorialStageInfo(int idx, TutorialStageInfo& out) const {
    if (idx < 0 || idx >= kTutorialCount) return false;
    out = kTutorialStages[idx];
    return true;
}

int Game::GetTutorialDoneCount() const {
    int done = 0;
    for (int i = 0; i < kTutorialCount; ++i) {
        if (GetTutorialDone(i)) done++;
    }
    return done;
}

int Game::GetCurrentTutorialIdx() const {
    for (int i = 0; i < kTutorialCount; ++i) {
        if (!GetTutorialDone(i)) return i;
    }
    return kTutorialCount;
}

void Game::SetupInitialPark() {
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            terrain[x][y] = GROUND_GRASS;
            groundZ[x][y] = 0;
            scenery[x][y] = SCENERY_NONE;
        }
    }

    // Urban River Canal under elevated Marina Viaduct
    for (int y = 6; y <= 21; ++y) {
        terrain[12][y] = GROUND_WATER;
        terrain[13][y] = GROUND_WATER;
    }

    // Pedestrian sidewalks connecting entrance plaza to Central Hub Station
    for (int x = 0; x <= 5; ++x) {
        terrain[x][9] = GROUND_PATH;
    }

    // Granite Transit Plaza at Central Hub
    for (int x = 5; x <= 8; ++x) {
        for (int y = 7; y <= 11; ++y) {
            terrain[x][y] = GROUND_PLAZA;
        }
    }

    // Tactile safety platform edge queue zones
    terrain[7][8] = GROUND_QUEUE;
    terrain[7][10] = GROUND_PATH;

    // Concourse paths connecting to Suburban Terminal
    for (int y = 10; y <= 15; ++y) {
        terrain[5][y] = GROUND_PATH;
    }

    // Concourse paths to University Med Center
    for (int x = 5; x <= 10; ++x) {
        terrain[x][17] = GROUND_PATH;
    }

    // Urban Transit Station Amenities & Scenery
    scenery[0][9] = SCENERY_METRO_ENTRANCE; // Grand subway portal entrance
    scenery[2][9] = SCENERY_TURNSTILE_GATE; // Contactless fare gates & TVM ticket machine
    scenery[3][8] = SCENERY_MAP_KIOSK;      // Harry Beck style schematic transit map board
    scenery[5][7] = SCENERY_NEWSSTAND;      // Platform Metro Cafe & refreshments
    scenery[1][10] = SCENERY_BIKE_RACK;     // Metro bike share docking rack
    scenery[4][7] = SCENERY_FOUNTAIN;       // Splashing park water fountain

    // Platform Benches
    scenery[6][8] = SCENERY_BENCH;
    scenery[8][8] = SCENERY_BENCH;

    // High-Efficiency Municipal LED Streetlamps
    scenery[3][8] = SCENERY_LAMP_POST;
    scenery[8][10] = SCENERY_LAMP_POST;
    scenery[5][12] = SCENERY_LAMP_POST;

    // Manicured Trees & Botanical Flower Beds
    scenery[1][8] = SCENERY_STREET_TREE;
    scenery[3][10] = SCENERY_STREET_TREE;
    scenery[5][10] = SCENERY_STREET_TREE;
    scenery[4][10] = SCENERY_FLOWER_BED;
    scenery[6][7]  = SCENERY_FLOWER_BED;

    // Metropolitan Park Pine Trees across the canal
    scenery[17][8]  = SCENERY_PINE_TREE;
    scenery[17][12] = SCENERY_PINE_TREE;
    scenery[17][16] = SCENERY_PINE_TREE;
    scenery[15][18] = SCENERY_PINE_TREE;

    // Island ocean border: land further out than the bought ring is open sea.
    // Buy LAND (toolbar) to reclaim more island ring.
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            if (terrain[x][y] == GROUND_GRASS && !IsBuildable(x, y)) {
                terrain[x][y] = GROUND_WATER;
            }
        }
    }
}

void Game::ResetPark() {
    Init();
}

void Game::AutoArmTutorialTool() {
    TutorialStageInfo st;
    if (!GetTutorialStageInfo(GetCurrentTutorialIdx(), st)) return;
    switch (GetCurrentTutorialIdx()) {
        case 2:  activeTab = CAT_TRACK;   currentTrack = TRACK_STRAIGHT;      isBulldozing = false; break; // lay straight rail
        case 3:  activeTab = CAT_TRACK;   currentTrack = TRACK_STATION;       isBulldozing = false; break; // drop a station
        case 4:  activeTab = CAT_SCENERY; currentScenery = SCENERY_STREET_TREE; isBulldozing = false; break; // plant trees
        case 5:  activeTab = CAT_TRACK;   currentTrack = TRACK_VIADUCT_ELEVATED; currentZ = 1;       isBulldozing = false; break; // cross the canal
        case 6:
        case 7:
        case 8:  activeTab = CAT_TRACK;   currentTrack = TRACK_STRAIGHT;      currentZ = 0;          isBulldozing = false; break; // shop lessons: neutral
        case 9:  isBulldozing = true;     currentZ = 0; break;                                                          // undo button
        case 10: activeTab = CAT_TRACK;   currentTrack = TRACK_STRAIGHT;      currentZ = 0;          isBulldozing = false; break; // navigation: tidy up
        default: break;
    }
}

void Game::DrawTutorialTarget() const {
    if (state != STATE_PLAYING || tutorialDone) return;
    if (!showObjectivePanel) return; // player hid the guided lesson (G)

    TutorialStageInfo st;
    if (!GetTutorialStageInfo(GetCurrentTutorialIdx(), st)) return;

    float pulse = 0.5f + 0.5f * sinf(GetTime() * 4.0f);
    Color ringC = Color{255, 214, 0, (unsigned char)(150 + 105 * pulse)};
    Color ringFill = Color{255, 214, 0, (unsigned char)(26 + 22 * pulse)};

    Vector3 fp;
    if (st.focusTrain) {
        fp = train.GetLocomotivePos();
        fp.z += 1.2f; // float above the roof
    } else if (st.focusGx >= 0) {
        fp = Vector3{(float)st.focusGx + 0.5f, (float)st.focusGy + 0.5f, 0.0f};
    } else {
        return; // pure toolbar spotlight lessons - no world marker needed
    }

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    Vector2 p = Iso::GridToScreen(fp.x, fp.y, fp.z, cameraPos, zoom);
    float ts = TILE_WIDTH * zoom;         // on-screen tile width
    float th = TILE_HEIGHT * zoom;        // on-screen tile height

    bool onScreen = (p.x > 30 && p.x < screenW - 30 && p.y > 66 && p.y < screenH - 168);
    if (onScreen) {
        // Soft translucent tile fill + pulsing diamond ring snapped to the tile
        float bx = p.x - ts / 2.0f, by = p.y - th / 2.0f;
        DrawRectangle((int)bx, (int)by, (int)ts, (int)th, ringFill);
        DrawRectangleLinesEx(Rectangle{bx, by, ts, th}, 2.0f, ringC);
        // Corner ticks make it read as a precise tile
        float c = 6.0f * (0.6f + 0.4f * pulse);
        DrawLine((int)bx, (int)by, (int)(bx + c), (int)by, ringC);
        DrawLine((int)bx, (int)by, (int)bx, (int)(by + c), ringC);
        DrawLine((int)(bx + ts), (int)by, (int)(bx + ts - c), (int)by, ringC);
        DrawLine((int)(bx + ts), (int)by, (int)(bx + ts), (int)(by + c), ringC);
        DrawLine((int)bx, (int)(by + th), (int)(bx + c), (int)(by + th), ringC);
        DrawLine((int)bx, (int)(by + th), (int)bx, (int)(by + th - c), ringC);
        DrawLine((int)(bx + ts), (int)(by + th), (int)(bx + ts - c), (int)(by + th), ringC);
        DrawLine((int)(bx + ts), (int)(by + th), (int)(bx + ts), (int)(by + th - c), ringC);
        if (st.focusTrain) {
            // Small bouncing arrow above the train
            float bounce = 6.0f * pulse;
            DrawLine((int)p.x, (int)(p.y - th - 8 - bounce), (int)p.x, (int)(p.y - th + 10 - bounce), Color{255, 214, 0, 255});
            DrawTriangle({p.x, p.y - th - 16 - bounce}, {p.x - 7, p.y - th - 2 - bounce}, {p.x + 7, p.y - th - 2 - bounce}, Color{255, 214, 0, 255});
        }
    } else {
        // Off-screen compass arrow glued to the window edge, pointing at the target
        Vector2 center{(float)screenW / 2.0f, (float)screenH / 2.0f};
        Vector2 d = {p.x - center.x, p.y - center.y};
        float len = std::max(1.0f, sqrtf(d.x * d.x + d.y * d.y));
        d = {d.x / len, d.y / len};
        float ang = atan2f(d.y, d.x);
        Vector2 base = {
            std::max(38.0f, std::min((float)screenW - 38.0f, p.x)),
            std::max(76.0f, std::min((float)screenH - 170.0f, p.y))
        };
        base.x -= d.x * 16.0f;
        base.y -= d.y * 16.0f;
        Vector2 tip = {base.x + d.x * 26.0f, base.y + d.y * 26.0f};
        float ca = cosf(ang), sa = sinf(ang);
        auto rot = [ca, sa](Vector2 v) {
            return Vector2{v.x * ca - v.y * sa, v.x * sa + v.y * ca};
        };
        Vector2 left = rot({-11.0f, 8.0f});
        Vector2 right = rot({-11.0f, -8.0f});
        DrawTriangle(tip, {base.x + left.x, base.y + left.y}, {base.x + right.x, base.y + right.y},
                     Color{255, 214, 0, (unsigned char)(180 + 75 * pulse)});
        DrawCircle((int)base.x, (int)base.y, 5, Color{255, 214, 0, (unsigned char)(150 + 105 * pulse)});
    }
}

void Game::GenerateWeeklyUpgrades() {
    activeUpgrades.clear();

    UpgradeChoice c1;
    c1.title = "4-Car EMU Trainset";
    c1.description = "Extends rolling stock formation to 4 cars, adding +4 commuter capacity.";
    c1.perkTag = "+4 SEATS / TRAIN";
    c1.accentColor = Color{56, 189, 248, 255}; // Sky Blue
    activeUpgrades.push_back(c1);

    UpgradeChoice c2;
    c2.title = "CBTC Signaling & Boost";
    c2.description = "Upgrades track signaling to Communications-Based Train Control, raising cruising speed to 75 km/h.";
    c2.perkTag = "HIGH-SPEED CBTC";
    c2.accentColor = Color{220, 38, 38, 255}; // Red
    activeUpgrades.push_back(c2);

    UpgradeChoice c3;
    c3.title = "Transit Subsidy & Pass";
    c3.description = "Receives $1,200 municipal transit subsidy and boosts commuter satisfaction by +15%.";
    c3.perkTag = "+$1,200 CASH & 15% SAT";
    c3.accentColor = Color{16, 185, 129, 255}; // Green
    activeUpgrades.push_back(c3);
}

void Game::ApplyUpgrade(int choiceIdx) {
    if (choiceIdx == 0) {
        train.SetCarriageCount(4);
        ShowToast("Upgrade Applied: 4-Car EMU Trainset deployed!", Color{56, 189, 248, 255});
    } else if (choiceIdx == 1) {
        peeps.SetSpawnInterval(1.6f);
        ShowToast("Upgrade Applied: CBTC Signaling & rapid dispatches active!", Color{220, 38, 38, 255});
    } else if (choiceIdx == 2) {
        economy.balance += 1200.0f;
        parkRating = std::min(100.0f, parkRating + 15.0f);
        ShowToast("Upgrade Applied: +$1,200 Subsidy & Commuter Satisfaction boosted!", Color{16, 185, 129, 255});
    }
    state = STATE_PLAYING;
    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
}

void Game::HandleInput() {
    Vector2 mousePos = GetMousePosition();

    // 0. Title & Start Menu Screen Input Handler
    if (state == STATE_TITLE) {
        if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui.CheckTitleStartClick(mousePos)) ||
            IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            state = STATE_PLAYING;
            briefingTimer = 4.5f;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.8f);
            ShowToast("[METRO] Service Commenced! Train will board passengers at Station [7]", Color{56, 189, 248, 255}, 5.0f);
        }
        return;
    }

    // 0b. Arcade PAUSE (Esc) - freeze the world, show clickable PAUSED menu
    if (state == STATE_PLAYING && isPaused) {
        if (IsKeyPressed(KEY_ESCAPE) ||
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui.CheckPauseClick(mousePos) == 0)) {
            isPaused = false;
            gameSpeed = 1;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
            ShowToast("SERVICE RESUMED!", Color{56, 189, 248, 255}, 2.0f);
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                   ui.CheckPauseClick(mousePos) == 1) {
            // Restart run from a clean slate (fresh title), keeping session best
            int keepBest = bestSessionRiders;
            ResetPark();
            bestSessionRiders = keepBest;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                   ui.CheckPauseClick(mousePos) == 2) {
            int keepBest = bestSessionRiders;
            ResetPark();
            bestSessionRiders = keepBest;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
        }
        if (IsKeyPressed(KEY_M)) {
            AudioManager::SetMute(!AudioManager::IsMuted());
        }
        return;
    }

    // 1. Camera Panning with WASD / Arrow Keys (speed scales with zoom so it
    //    feels identical whether zoomed in on the island or viewing the sea)
    float panSpeed = 340.0f * zoom * GetFrameTime();
    bool manualPan = false;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { cameraPos.y += panSpeed; manualPan = true; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  { cameraPos.y -= panSpeed; manualPan = true; }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  { cameraPos.x += panSpeed; manualPan = true; }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { cameraPos.x -= panSpeed; manualPan = true; }

    // Mouse Edge-of-screen Scrolling (RTS style) - skip when over the toolbar/HUD
    if (!isDragging && !statsWindowOpen && !staffWindowOpen && !helpOverlayOpen) {
        Vector2 mEdge = GetMousePosition();
        int edgeMargin = 26;
        int scrW = GetScreenWidth();
        int scrH = GetScreenHeight();
        if (mEdge.x < edgeMargin) { cameraPos.x += panSpeed; manualPan = true; }
        else if (mEdge.x > scrW - edgeMargin) { cameraPos.x -= panSpeed; manualPan = true; }
        if (mEdge.y < edgeMargin && mEdge.y > 60) { cameraPos.y += panSpeed; manualPan = true; }
        else if (mEdge.y > scrH - 150 && mEdge.y < scrH - 1) { cameraPos.y -= panSpeed; manualPan = true; }
        // (bottom -140 keeps the toolbar free for clicks)
    }

    if (manualPan) { rideCamActive = false; lastManualPanTime = 7.0f; panMoves++; }

    // Mouse Drag Pan with RIGHT or MIDDLE button (grab the map and pull)
    bool dragKey = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
        dragStart = mousePos;
        isDragging = true;
        rideCamActive = false;
    }
    if (isDragging) {
        if (dragKey) {
            cameraPos.x += (mousePos.x - dragStart.x);
            cameraPos.y += (mousePos.y - dragStart.y);
            dragStart = mousePos;
            lastManualPanTime = 7.0f;
        } else {
            isDragging = false;
        }
    }

    // Zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        zoom = std::max(0.65f, std::min(2.4f, zoom + wheel * 0.1f));
    }

    // Recenter the camera on the lead train (and exit cab view)
    if (IsKeyPressed(KEY_HOME)) {
        rideCamActive = false;
        Vector3 lp = train.GetLocomotivePos();
        Vector2 delta = Iso::GridToScreen(lp.x, lp.y, lp.z, Vector2{0.0f, 0.0f}, zoom);
        cameraPos.x = (float)GetScreenWidth() / 2.0f - delta.x;
        cameraPos.y = (float)GetScreenHeight() / 2.0f - delta.y;
        lastManualPanTime = 0.0f;
        ShowToast("CAMERA RECENTERED on the lead train (HOME)", Color{56, 189, 248, 255}, 2.0f);
        AudioManager::Play(SFX_BUTTON_CLICK, 0.5f);
    }

    // Hotkeys
if (IsKeyPressed(KEY_F)) {
        rideCamActive = !rideCamActive;
        rideCamUsed = true;
        ShowToast(rideCamActive ? "[CAB CAM] Driver's Cab Cam Active (Tracking Lead EMU)" : "Free OCC Camera Mode", Color{56, 189, 248, 255}, 2.0f);
    }
    if (IsKeyPressed(KEY_T)) {
        statsWindowOpen = !statsWindowOpen;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_P)) {
        staffWindowOpen = !staffWindowOpen;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_SLASH)) {
        helpOverlayOpen = !helpOverlayOpen;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SPACE)) {
        if (!isPaused) {
            gameSpeed = (gameSpeed == 0) ? 1 : 0;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        isPaused = true;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
    }
    if (IsKeyPressed(KEY_M)) {
        AudioManager::SetMute(!AudioManager::IsMuted());
    }
    if (IsKeyPressed(KEY_G)) {
        showObjectivePanel = !showObjectivePanel;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_C)) {
        train.Reset(tracks);
        ShowToast("Train Calibrated to Central Hub Station", Color{255, 214, 0, 255}, 2.5f);
        AudioManager::Play(SFX_BUTTON_CLICK, 0.8f);
    }
    if (IsKeyPressed(KEY_TAB)) {
        activeTab = (ToolCategory)((activeTab + 1) % 3);
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }

    // Elevation controls
    if (IsKeyPressed(KEY_E)) {
        currentZ = std::min(5, currentZ + 1);
        AudioManager::Play(SFX_BUTTON_CLICK, 0.5f);
    }
    if (IsKeyPressed(KEY_Q)) {
        currentZ = std::max(0, currentZ - 1);
        AudioManager::Play(SFX_BUTTON_CLICK, 0.5f);
    }

    // Rotation / Heading control
    if (IsKeyPressed(KEY_R)) {
        buildHeading = (Direction)((buildHeading + 1) % 4);
        AudioManager::Play(SFX_BUTTON_CLICK, 0.5f);
    }

    // Quick Tool Selection Keys (1-8 & X)
    if (IsKeyPressed(KEY_X)) {
        isBulldozing = !isBulldozing;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_ONE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_STRAIGHT;
        else if (activeTab == CAT_INFRA) currentGround = GROUND_PATH;
        else currentScenery = SCENERY_METRO_ENTRANCE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_TWO)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_CURVE_LEFT;
        else if (activeTab == CAT_INFRA) currentGround = GROUND_QUEUE;
        else currentScenery = SCENERY_TURNSTILE_GATE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_THREE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_CURVE_RIGHT;
        else if (activeTab == CAT_INFRA) currentGround = GROUND_PLAZA;
        else currentScenery = SCENERY_STREET_TREE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FOUR)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_VIADUCT_ELEVATED;
        else currentScenery = SCENERY_PINE_TREE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FIVE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_VIADUCT_SLOPE;
        else currentScenery = SCENERY_BENCH;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SIX)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_TUNNEL_PORTAL;
        else currentScenery = SCENERY_LAMP_POST;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SEVEN)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_STATION;
        else currentScenery = SCENERY_FOUNTAIN;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_EIGHT)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_SIGNAL;
        else currentScenery = SCENERY_NEWSSTAND;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }

    // Modal Interaction Handlers
    if (state == STATE_WEEKLY_UPGRADE) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int ch = ui.CheckUpgradeModalClick(mousePos);
            if (ch != -1) {
                ApplyUpgrade(ch);
            }
        }
        return;
    }
    if (state == STATE_VICTORY) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (ui.CheckVictoryContinueClick(mousePos)) {
                endlessMode = true;
                state = STATE_PLAYING;
                ShowToast("[ENDLESS METROPOLIS] Service Continued! Grants every 250 commuters!", Color{34, 197, 94, 255}, 5.0f);
                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
                return;
            }
            if (ui.CheckRestartClick(mousePos)) {
                ResetPark();
                return;
            }
        }
        return;
    }
    if (state == STATE_GAME_OVER) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui.CheckRestartClick(mousePos)) {
            ResetPark();
        }
        return;
    }

    // Handle Help Overlay Click
    if (helpOverlayOpen) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui.CheckHelpOverlayClick(mousePos)) {
            helpOverlayOpen = false;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
        }
        return;
    }

    // Handle UI Top HUD & Toolbar Clicks
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Top HUD Click
        int newSpeed = -1;
        bool toggleMute = false;
        bool toggleStats = false;
        bool toggleStaff = false;
        bool toggleCam = false;
        bool toggleHelp = false;
        if (ui.CheckHUDClick(mousePos, newSpeed, toggleMute, toggleStats, toggleStaff, toggleCam, toggleHelp)) {
            if (newSpeed != -1) gameSpeed = newSpeed;
            if (toggleMute) AudioManager::SetMute(!AudioManager::IsMuted());
            if (toggleStats) statsWindowOpen = !statsWindowOpen;
            if (toggleStaff) staffWindowOpen = !staffWindowOpen;
            if (toggleCam) rideCamActive = !rideCamActive;
            if (toggleHelp) helpOverlayOpen = !helpOverlayOpen;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
            return;
        }

        // Line Operations Window Click
        if (statsWindowOpen) {
            float deltaFare = 0.0f;
            int colorChoice = -1;
            int modeChange = -1;
            int carDelta = 0;
            bool closeStats = false;
            if (ui.CheckStatsWindowClick(mousePos, deltaFare, colorChoice, modeChange, carDelta, closeStats)) {
                if (closeStats) statsWindowOpen = false;
                if (modeChange != -1) {
                    train.SetOperatingMode((LineOperatingMode)modeChange);
                    const char* mNames[] = {"OPEN (Boarding Active)", "TEST RUN (No Boarding)", "CLOSED (Service Suspended)"};
                    Color mCols[] = {Color{34, 197, 94, 255}, Color{234, 179, 8, 255}, Color{239, 68, 68, 255}};
                    ShowToast(TextFormat("Line Status: %s", mNames[modeChange]), mCols[modeChange], 3.0f);
                    AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
                }
                if (carDelta != 0) {
                    int newCars = train.GetCarriageCount() + carDelta;
                    train.SetCarriageCount(newCars);
                    ShowToast(TextFormat("Rolling Stock: Formation updated to %d Cars", train.GetCarriageCount()), Color{56, 189, 248, 255}, 2.5f);
                    AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
                }
                if (deltaFare != 0.0f) {
                    float newFare = std::max(0.50f, std::min(10.0f, train.GetTicketFare() + deltaFare));
                    train.SetTicketFare(newFare);
                    economy.baseFare = newFare;
                    cachedStats.ticketFare = newFare;
                    ShowToast(TextFormat("Ticket Tariff Set to $%.2f", newFare), Color{34, 197, 94, 255}, 2.0f);
                    AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
                }
                if (colorChoice != -1) {
                    Color liveries[] = {
                        Color{229, 57, 53, 255},  // Tokyo Red (Marunouchi)
                        Color{37, 99, 235, 255},  // London Blue (Piccadilly)
                        Color{16, 185, 129, 255}, // Paris Green (Line 6)
                        Color{147, 51, 234, 255}, // MTR Purple (Tseung Kwan O)
                        Color{245, 158, 11, 255}, // Chicago Amber (Brown Line)
                        Color{234, 88, 12, 255}   // Tokyo Ginza Orange
                    };
                    const char* names[] = {
                        "Line 1 - Marunouchi Red",
                        "Line 2 - Piccadilly Blue",
                        "Line 3 - Paris Emerald",
                        "Line 4 - Victoria Purple",
                        "Line 5 - Chicago Amber",
                        "Line 6 - Ginza Orange"
                    };
                    cachedStats.themeColor = liveries[colorChoice];
                    cachedStats.lineName = names[colorChoice];
                    tracks.SetTrackColor(liveries[colorChoice]);
                    train.SetTrainTheme(liveries[colorChoice]);
                    train.SetLineName(names[colorChoice]);
                    ShowToast(TextFormat("Route Livery Updated: %s!", names[colorChoice]), liveries[colorChoice], 3.0f);
                    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.7f);
                }
                AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
                return;
            }
        }

        // Transit Crew Window Click
        if (staffWindowOpen) {
            bool hireCustodian = false;
            bool hireTechnician = false;
            bool closeCrew = false;
            if (ui.CheckStaffWindowClick(mousePos, hireCustodian, hireTechnician, closeCrew)) {
                if (closeCrew) staffWindowOpen = false;
                if (hireCustodian) {
                    if (economy.balance >= 80.0f) {
                        economy.balance -= 80.0f;
                        StaffMember c;
                        c.name = TextFormat("Custodian #%d", (int)staff.size() + 1);
                        c.type = STAFF_CUSTODIAN;
                        c.pos = {2.0f, 12.0f};
                        c.targetPos = {2.0f, 12.0f};
                        staff.push_back(c);
                        ShowToast("Hired Station Custodian! Platforms kept sparkling clean.", Color{59, 130, 246, 255}, 3.0f);
                        AudioManager::Play(SFX_SMARTCARD_BEEP, 0.7f);
                    } else {
                        ShowToast("Insufficient funds to hire Custodian ($80 required)", Color{239, 68, 68, 255}, 2.5f);
                    }
                }
                if (hireTechnician) {
                    if (economy.balance >= 100.0f) {
                        economy.balance -= 100.0f;
                        StaffMember t;
                        t.name = TextFormat("Technician #%d", (int)staff.size() + 1);
                        t.type = STAFF_ENGINEER;
                        t.pos = {7.0f, 13.0f};
                        t.targetPos = {7.0f, 13.0f};
                        staff.push_back(t);
                        ShowToast("Hired Signal Technician! Track switches and signals inspected.", Color{245, 158, 11, 255}, 3.0f);
                        AudioManager::Play(SFX_SMARTCARD_BEEP, 0.7f);
                    } else {
                        ShowToast("Insufficient funds to hire Technician ($100 required)", Color{239, 68, 68, 255}, 2.5f);
                    }
                }
                AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
                return;
            }
        }

        // Peep Inspector Close Click
        if (selectedPeepIdx != -1 && ui.CheckPeepInspectorCloseClick(mousePos)) {
            selectedPeepIdx = -1;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
            return;
        }

        // Toolbar Category Tab Click
        int tabIdx = ui.CheckToolbarTabClick(mousePos);
        if (tabIdx != -1) {
            activeTab = (ToolCategory)tabIdx;
            isBulldozing = false;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
            return;
        }

        // Toolbar Item Click
        int itemIdx = ui.CheckToolbarItemClick(mousePos, activeTab);
        if (itemIdx != -1) {
            if (activeTab == CAT_TRACK) {
                TrackType tTypes[] = {
                    TRACK_STRAIGHT, TRACK_CURVE_LEFT, TRACK_CURVE_RIGHT,
                    TRACK_VIADUCT_ELEVATED, TRACK_VIADUCT_SLOPE, TRACK_TUNNEL_PORTAL,
                    TRACK_STATION, TRACK_SIGNAL, TRACK_NONE
                };
                if (itemIdx == 8) isBulldozing = true;
                else { isBulldozing = false; currentTrack = tTypes[itemIdx]; }
            } else if (activeTab == CAT_INFRA) {
                GroundType gTypes[] = { GROUND_PATH, GROUND_QUEUE, GROUND_PLAZA, GROUND_GRASS };
                if (itemIdx == 3) isBulldozing = true;
                else { isBulldozing = false; currentGround = gTypes[itemIdx]; }
            } else if (activeTab == CAT_SCENERY) {
                SceneryType sTypes[] = {
                    SCENERY_METRO_ENTRANCE, SCENERY_TURNSTILE_GATE, SCENERY_STREET_TREE,
                    SCENERY_PINE_TREE, SCENERY_BENCH, SCENERY_LAMP_POST,
                    SCENERY_FOUNTAIN, SCENERY_NEWSSTAND, SCENERY_NONE
                };
                if (itemIdx == 8) isBulldozing = true;
                else { isBulldozing = false; currentScenery = sTypes[itemIdx]; }
            }
            AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
            return;
        }

        // Toolbar Aux Click (Height & Rotate & Buy)
        int zDelta = 0;
        bool doRotate = false;
        int buyAction = 0;
        if (ui.CheckToolbarAuxClick(mousePos, zDelta, doRotate, buyAction)) {
            if (buyAction == 1) {
                if (economy.balance >= TRAIN_CAR_COST) {
                    economy.balance -= TRAIN_CAR_COST;
                    train.AddCarriage();
                    ShowToast("Bought a train car! +" + std::to_string(train.GetCarriageCount()) + " car formation.", Color{56, 189, 248, 255});
                    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
                } else {
                    ShowToast("Need $" + std::to_string((int)TRAIN_CAR_COST) + " for a train car!", Color{239, 68, 68, 255});
                }
                return;
            }
            if (buyAction == 2) {
                if (economy.balance >= LAND_EXPAND_COST) {
                    economy.balance -= LAND_EXPAND_COST;
                    buildRadius = std::min(26, buildRadius + 3);
                    ReclaimLand();
                    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
                } else {
                    ShowToast("Need $" + std::to_string((int)LAND_EXPAND_COST) + " to expand the island!", Color{239, 68, 68, 255});
                }
                return;
            }
            if (buyAction == 3) {
                if (GetExtraTrainCount() >= MAX_EXTRA_TRAINS) {
                    ShowToast("Fleet at max capacity! More cars or more LAND instead.", Color{56, 189, 248, 255});
                } else {
                    float cost = ExtraTrainCostP(GetExtraTrainCount());
                    if (economy.balance >= cost) {
                        if (!tracks.IsCircuitClosed()) {
                            ShowToast("Connect the loop (press C) so another train can run!", Color{239, 68, 68, 255});
                        } else {
                            economy.balance -= cost;
                            MetroTrain nt;
                            nt.SetCarriageCount(3);
                            nt.SetTrainTheme(train.GetTrainTheme());
                            int index = GetExtraTrainCount();
                            nt.Reset(tracks, 0.5f + tracks.GetTotalCircuitLength() * (0.5f + 0.18f * index));
                            extraTrains.push_back(nt);
                            ShowToast("EXTRA TRAIN bought! Fleet now has " + std::to_string(GetExtraTrainCount() + 1) + " trains.", Color{56, 189, 248, 255});
                            particles.SpawnConfetti(nt.GetLocomotivePos(), 30);
                            AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
                        }
                    } else {
                        ShowToast("Need $" + std::to_string((int)cost) + " for an extra train!", Color{239, 68, 68, 255});
                    }
                }
                return;
            }
            if (zDelta != 0) currentZ = std::max(0, std::min(5, currentZ + zDelta));
            if (doRotate) {
                buildHeading = (Direction)((buildHeading + 1) % 4);
            }
            AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
            return;
        }
    }

    // 2. Grid Raycast Hover
    Vector2 gridPos = Iso::ScreenToGrid(mousePos, cameraPos, zoom, (float)currentZ);
    hoveredGx = (int)floorf(gridPos.x);
    hoveredGy = (int)floorf(gridPos.y);
    bool insideGrid = (hoveredGx >= 0 && hoveredGx < GRID_SIZE && hoveredGy >= 0 && hoveredGy < GRID_SIZE);

    // Smart Snap Assist: If placing track, auto-align heading to continue adjacent track pieces
    if (activeTab == CAT_TRACK && !isBulldozing && insideGrid && !IsKeyPressed(KEY_R)) {
        if (!tracks.HasPiece(hoveredGx, hoveredGy)) {
            Direction suggested;
            if (tracks.SuggestConnectingHeading(hoveredGx, hoveredGy, currentZ, suggested)) {
                buildHeading = suggested;
            }
        }
    }

    // 3. Commuter Inspection or Placement
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.y > 60 && mousePos.y < GetScreenHeight() - 150) {
        // Check if clicking on a commuter
        int peepUnderMouse = peeps.FindCommuterAtScreenPos(mousePos, cameraPos, zoom);
        if (peepUnderMouse != -1) {
            selectedPeepIdx = peepUnderMouse;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
            return;
        }

        if (insideGrid) {
            if (isBulldozing) {
                // Bulldoze priority: Scenery -> Track -> Path
                if (scenery[hoveredGx][hoveredGy] != SCENERY_NONE) {
                    scenery[hoveredGx][hoveredGy] = SCENERY_NONE;
                    economy.balance += 15.0f;
                    bulldozeUses++;
                    AudioManager::Play(SFX_BULLDOZE, 0.7f);
                    particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 10);
                    particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ + 0.5f}, "+$15", Color{34, 197, 94, 255});
                } else if (tracks.HasPiece(hoveredGx, hoveredGy)) {
                    bool wasClosed = tracks.IsCircuitClosed();
                    tracks.RemovePiece(hoveredGx, hoveredGy);
                    economy.balance += 25.0f;
                    bulldozeUses++;
                    AudioManager::Play(SFX_BULLDOZE, 0.8f);
                    particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 12);
                    particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ + 0.5f}, "+$25", Color{34, 197, 94, 255});
                    if (wasClosed && !tracks.IsCircuitClosed()) {
                        ShowToast("[CIRCUIT] Loop broken! Reconnect the track to make a closed loop.", Color{239, 68, 68, 255}, 4.0f);
                    }
                } else if (terrain[hoveredGx][hoveredGy] != GROUND_GRASS && terrain[hoveredGx][hoveredGy] != GROUND_WATER) {
                    terrain[hoveredGx][hoveredGy] = GROUND_GRASS;
                    economy.balance += 5.0f;
                    AudioManager::Play(SFX_BULLDOZE, 0.6f);
                    particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.5f}, "+$5", Color{34, 197, 94, 255});
                }
            } else {
                // Active placement based on Tab
                if (terrain[hoveredGx][hoveredGy] == GROUND_WATER) {
                    bool elevatedTrack = (activeTab == CAT_TRACK && currentZ > 0);
                    if (!elevatedTrack) {
                        if (IsBuildable(hoveredGx, hoveredGy)) {
                            ShowToast("That's the river! Press E to raise elevation and build a viaduct track over it.", Color{239, 68, 68, 255}, 3.0f);
                        } else {
                            ShowToast("That's open ocean! Buy LAND on the toolbar to expand the island.", Color{239, 68, 68, 255}, 3.0f);
                        }
                        return;
                    }
                }
                if (activeTab == CAT_TRACK) {
                    if (economy.balance >= 40.0f) {
                        bool wasClosed = tracks.IsCircuitClosed();
                        Direction inDir, outDir;
                        GetTrackPieceDirs(currentTrack, buildHeading, inDir, outDir);
                        if (tracks.AddPiece(hoveredGx, hoveredGy, currentZ, currentTrack, inDir, outDir)) {
                            economy.balance -= 40.0f;
                            if (currentTrack == TRACK_VIADUCT_ELEVATED || currentTrack == TRACK_VIADUCT_SLOPE || currentZ >= 1) {
                                viaductPiecesPlaced++;
                            }
                            AudioManager::Play(SFX_CONSTRUCTION, 0.8f);
                            particles.SpawnSparks(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 8);
                            particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ + 0.6f}, "-$40", Color{239, 68, 68, 255});

                            if (!wasClosed && tracks.IsCircuitClosed()) {
                                ShowToast("[CIRCUIT] Transit Loop Closed! Regular EMU Schedule Active!", Color{34, 197, 94, 255}, 4.0f);
                                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
                            } else if (wasClosed && !tracks.IsCircuitClosed()) {
                                ShowToast("[CIRCUIT] Loop broken! Reconnect the track to make a closed loop.", Color{239, 68, 68, 255}, 4.0f);
                            }

                            // Auto-advance cursor forward along track exit direction and sync build heading!
                            Vector2 fwd = GetDirectionOffset(outDir);
                            hoveredGx += (int)fwd.x;
                            hoveredGy += (int)fwd.y;
                            buildHeading = outDir; // Auto-align next track piece
                            if (currentTrack == TRACK_STATION) stationsPlaced++;
                            else piecesPlaced++;
                            if (currentTrack == TRACK_VIADUCT_ELEVATED) currentZ++;
                            if (currentTrack == TRACK_VIADUCT_SLOPE) currentZ = std::max(0, currentZ - 1);
                        }
                    } else {
                        ShowToast("Insufficient funds to lay track ($40 required)", Color{239, 68, 68, 255}, 2.0f);
                    }
                } else if (activeTab == CAT_INFRA) {
                    if (economy.balance >= 15.0f) {
                        terrain[hoveredGx][hoveredGy] = currentGround;
                        economy.balance -= 15.0f;
                        AudioManager::Play(SFX_CONSTRUCTION, 0.7f);
                        particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.6f}, "-$15", Color{239, 68, 68, 255});
                    } else {
                        ShowToast("Insufficient funds for concourse paving ($15 required)", Color{239, 68, 68, 255}, 2.0f);
                    }
                } else if (activeTab == CAT_SCENERY) {
                    int scnCost = 35;
                    float boost = 1.5f;
                    if (currentScenery == SCENERY_METRO_ENTRANCE) { scnCost = 120; boost = 4.0f; }
                    else if (currentScenery == SCENERY_TURNSTILE_GATE) { scnCost = 90; boost = 3.0f; }
                    else if (currentScenery == SCENERY_NEWSSTAND) { scnCost = 150; boost = 4.0f; }
                    else if (currentScenery == SCENERY_MAP_KIOSK) { scnCost = 40; boost = 1.5f; }
                    else if (currentScenery == SCENERY_STREET_TREE) { scnCost = 35; boost = 1.2f; }
                    else if (currentScenery == SCENERY_PINE_TREE) { scnCost = 30; boost = 1.0f; }
                    else if (currentScenery == SCENERY_LAMP_POST) { scnCost = 25; boost = 1.0f; }
                    else if (currentScenery == SCENERY_BENCH) { scnCost = 20; boost = 0.8f; }
                    else if (currentScenery == SCENERY_FOUNTAIN) { scnCost = 120; boost = 5.0f; }
                    else if (currentScenery == SCENERY_FLOWER_BED) { scnCost = 25; boost = 1.2f; }

                    if (economy.balance >= (float)scnCost) {
                        scenery[hoveredGx][hoveredGy] = currentScenery;
                        economy.balance -= (float)scnCost;
                        parkRating = std::min(100.0f, parkRating + boost);
                        sceneryPlaced++;
                        AudioManager::Play(SFX_CONSTRUCTION, 0.8f);
                        particles.SpawnConfetti(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.5f}, 10);
                        particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.8f}, TextFormat("-$%d", scnCost), Color{239, 68, 68, 255});
                    } else {
                        ShowToast(TextFormat("Insufficient funds for scenery ($%d required)", scnCost), Color{239, 68, 68, 255}, 2.0f);
                    }
                }
            }
        }
    }
}

void Game::Update(float dt) {
    ui.Update(GetMousePosition(), IsMouseButtonPressed(MOUSE_BUTTON_LEFT));

    if (activeToast.timer > 0.0f) {
        activeToast.timer -= dt;
    }

    if (state != STATE_PLAYING) return;
    if (gameSpeed == 0 || isPaused) return;

    // Arcade mission briefing timer (real-time countdown)
    if (briefingTimer > 0.0f) briefingTimer -= dt;

    // Tutorial objective progression
    if (!tutorialDone) {
        int nowDone = GetTutorialDoneCount();
        int wasDone = tutorialDoneCount;
        tutorialDoneCount = nowDone;
        if (nowDone > wasDone) {
            int lessons = nowDone - wasDone;
            if (nowDone >= kTutorialCount) {
                tutorialDone = true;
                economy.balance += 500.0f;
                ShowToast("TUTORIAL COMPLETE! +$500 grant. Final goal: 500 riders to win.", Color{34, 197, 94, 255}, 5.5f);
                AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
            } else {
                TutorialStageInfo next;
                if (GetTutorialStageInfo(nowDone, next)) {
                    ShowToast(TextFormat("LESSON PASSED (+$%d)! Next: %s", lessons * 75, next.title), Color{255, 214, 0, 255}, 4.0f);
                } else {
                    ShowToast("LESSON PASSED!", Color{255, 214, 0, 255}, 3.0f);
                }
                economy.balance += 75.0f * lessons;
                particles.SpawnConfetti(Vector3{9.5f, 11.5f, 0.4f}, 18);
                particles.SpawnFloatingText(Vector3{9.5f, 11.5f, 0.6f}, TextFormat("+$%d LESSON BONUS", lessons * 75), Color{255, 214, 0, 255});
                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.7f);
            }
        }
        // Arm the exact tool for the newly-active lesson so the player
        // only has to click the glowing ring - no key-number hunting.
        int curIdx = GetCurrentTutorialIdx();
        if (curIdx != lastArmedLesson) {
            lastArmedLesson = curIdx;
            AutoArmTutorialTool();
        }
    }

    float simDt = dt * (float)gameSpeed;

    // 1. Cab Cam Smooth Tracking (Driver's point of view)
    if (rideCamActive) {
        Vector3 locoPos = train.GetLocomotivePos();
        Vector2 targetScreen = Iso::GridToScreen(locoPos.x, locoPos.y, locoPos.z, {0, 0}, zoom);
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        Vector2 desiredCam = { (float)screenW / 2.0f - targetScreen.x, (float)screenH / 2.0f - targetScreen.y };

        cameraPos = Vector2Lerp(cameraPos, desiredCam, simDt * 5.0f);
    } else if (!tutorialDone) {
        // Deliberate tutorial: gently steer the camera at the lesson's focus
        lastManualPanTime = std::max(0.0f, lastManualPanTime - simDt);
        TutorialStageInfo st;
        if (GetTutorialStageInfo(GetCurrentTutorialIdx(), st) && lastManualPanTime <= 0.0f) {
            Vector3 fp;
            bool haveFocus = st.focusTrain;
            if (st.focusTrain) {
                fp = train.GetLocomotivePos();
            } else if (st.focusGx >= 0) {
                fp = Vector3{(float)st.focusGx + 0.5f, (float)st.focusGy + 0.5f, 0.0f};
            }
            if (haveFocus) {
                Vector2 targetScreen = Iso::GridToScreen(fp.x, fp.y, fp.z, {0, 0}, zoom);
                int screenW = GetScreenWidth();
                int screenH = GetScreenHeight();
                // Make the target sit slightly BELOW center so the island's
                // surroundings stay visible - never a lonely empty void.
                float anchorH = st.focusTrain ? 0.5f : 0.62f;
                Vector2 desiredCam = { (float)screenW / 2.0f - targetScreen.x, (float)screenH * anchorH - targetScreen.y };
                cameraPos = Vector2Lerp(cameraPos, desiredCam, std::min(1.0f, simDt * 2.2f));
            }
        }
    }

    // 2. Weekly Calendar Progression & Mini Metro Rhythm
    weekTimer += simDt;
    if (weekTimer >= WEEK_DURATION) {
        weekTimer = 0.0f;
        week++;
        GenerateWeeklyUpgrades();
        state = STATE_WEEKLY_UPGRADE;
        AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
        return;
    }
    if (weekTimer >= 12.0f && weekTimer < 40.0f) {
        rushHourActive = (fmodf(weekTimer, 14.0f) < 7.0f);
    } else {
        rushHourActive = false;
    }
    if (rushHourActive) {
        rushHourTimer += simDt;
        peeps.SetSpawnInterval(0.85f);
        if (rushHourTimer > 0.45f) {
            rushHourTimer = 0.0f;
            particles.SpawnFloatingText(Vector3{(float)LAND_CENTER_X, (float)LAND_CENTER_Y, 0.6f}, "RUSH HOUR!", Color{248, 113, 113, 255});
        }
    } else {
        peeps.SetSpawnInterval(1.9f);
        rushHourTimer = 0.0f;
    }

    // 3. Update Particles
    particles.Update(simDt);

    // 4. Update Metro Train Operations & Physics
    int delivered = 0;
    float fareRevenue = 0.0f;
    train.Update(simDt, tracks, particles, delivered, fareRevenue);

    // Extra fleet EMUs (purchasable); rush-hour surges spawn them in demand
    for (auto& extra : extraTrains) {
        int deliveredN = 0;
        float fareN = 0.0f;
        extra.Update(simDt, tracks, particles, deliveredN, fareN);
        delivered += deliveredN;
        fareRevenue += fareN;
    }

    if (delivered > 0) {
        economy.totalDelivered += delivered;
        bestSessionRiders = std::max(bestSessionRiders, economy.totalDelivered);

        // Ridership Rush Combo multiplier
        comboStreak += delivered;
        comboTimer = 18.0f;
        rushCombo = 1.0f + std::min(1.0f, (float)comboStreak * 0.05f); // up to 2.0x
        float rushMult = rushHourActive ? 1.25f : 1.0f;                  // rush-hour fare surge
        float comboBonus = fareRevenue * rushMult * (rushCombo - 1.0f);
        float totalEarned = fareRevenue * rushMult + comboBonus;
        economy.balance += totalEarned;
        parkRating = std::min(100.0f, parkRating + (float)delivered * 0.6f);

        // Alight passengers onto platform and concourse
        Vector3 locoP = train.GetLocomotivePos();
        peeps.AlightPassengers(delivered, Vector2{locoP.x, locoP.y});

        if (rushCombo > 1.05f) {
            particles.SpawnFloatingText(Vector3{locoP.x, locoP.y, locoP.z + 1.4f}, TextFormat("RUSH x%.1f!", rushCombo), Color{255, 215, 0, 255});
        }

        // Milestone grants
        int milestones[] = {25, 50, 100, 250, 500};
        for (int m : milestones) {
            if (economy.totalDelivered >= m && lastMilestoneAwarded < m) {
                lastMilestoneAwarded = m;
                float bonus = (float)m * 10.0f;
                economy.balance += bonus;
                particles.SpawnConfetti(locoP, 30);
                particles.SpawnFloatingText(Vector3{locoP.x, locoP.y, locoP.z + 1.2f}, TextFormat("+$%.0f BONUS", bonus), Color{255, 215, 0, 255});
                ShowToast(TextFormat("[MILESTONE] %d Commuters Served! Grant: +$%.0f", m, bonus), Color{34, 197, 94, 255}, 4.0f);
                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
                break;
            }
        }

        if (economy.totalDelivered >= 500 && !endlessMode) {
            state = STATE_VICTORY;
            AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
        } else if (endlessMode) {
            int nextEndlessMilestone = ((lastMilestoneAwarded / 250) + 1) * 250;
            if (economy.totalDelivered >= nextEndlessMilestone && lastMilestoneAwarded < nextEndlessMilestone) {
                lastMilestoneAwarded = nextEndlessMilestone;
                float bonus = 2500.0f;
                economy.balance += bonus;
                particles.SpawnConfetti(locoP, 40);
                particles.SpawnFloatingText(Vector3{locoP.x, locoP.y, locoP.z + 1.2f}, TextFormat("+$%.0f ENDLESS SUBSIDY", bonus), Color{255, 215, 0, 255});
                ShowToast(TextFormat("[ENDLESS METROPOLIS] %d Commuters! Subsidy: +$%.0f", nextEndlessMilestone, bonus), Color{34, 197, 94, 255}, 4.0f);
                AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
            }
        }
    }

    // Dynamic Wayside Signaling Update
    tracks.UpdateSignals(train.GetTrainDistance());

    // 5. Update Commuter Flow & Overcrowding Triage
    int angry = 0;
    peeps.Update(simDt, train, extraTrains.empty() ? nullptr : &extraTrains, particles, parkRating, angry, messes);
    peeps.CheckSceneryInteractions(scenery, particles, economy.balance, messes);

    // Platform Cleanliness calculation
    parkCleanliness = std::max(0.0f, 100.0f - (float)messes.size() * 3.5f);
    if (parkCleanliness < 50.0f) {
        parkRating = std::max(0.0f, parkRating - simDt * 0.3f);
    }

    // 6. Update Transit Crew (Custodians clean spills, Technicians inspect signals)
    for (auto& s : staff) {
        s.walkTimer += simDt;
        if (s.type == STAFF_CUSTODIAN) {
            if (s.isWorking) {
                s.workTimer += simDt;
                if (fmodf(s.workTimer, 0.4f) < simDt) {
                    particles.SpawnSmoke(Vector3{s.pos.x, s.pos.y, 0.05f}, 2);
                }
                if (s.workTimer >= 1.5f) {
                    s.isWorking = false;
                    s.workTimer = 0.0f;
                    // Remove closest mess within cleaning range
                    for (auto it = messes.begin(); it != messes.end();) {
                        if (Vector2Distance(s.pos, it->pos) < 0.7f) {
                            particles.SpawnSparks(Vector3{it->pos.x, it->pos.y, 0.1f}, 5);
                            it = messes.erase(it);
                            parkRating = std::min(100.0f, parkRating + 1.5f);
                            break;
                        } else {
                            ++it;
                        }
                    }
                }
            } else if (!messes.empty()) {
                // Seek nearest litter / spill
                float minDist = 999.0f;
                int bestIdx = -1;
                for (size_t mi = 0; mi < messes.size(); ++mi) {
                    float d = Vector2Distance(s.pos, messes[mi].pos);
                    if (d < minDist) {
                        minDist = d;
                        bestIdx = (int)mi;
                    }
                }
                if (bestIdx != -1) {
                    s.targetPos = messes[bestIdx].pos;
                    Vector2 dir = Vector2Subtract(s.targetPos, s.pos);
                    float dist = Vector2Length(dir);
                    if (dist > 0.25f) {
                        dir = Vector2Normalize(dir);
                        s.pos.x += dir.x * 1.5f * simDt;
                        s.pos.y += dir.y * 1.5f * simDt;
                    } else {
                        s.isWorking = true;
                        s.workTimer = 0.0f;
                    }
                }
            } else {
                // Patrol concourse
                Vector2 dir = Vector2Subtract(s.targetPos, s.pos);
                if (Vector2Length(dir) < 0.2f) {
                    float rx = 1.0f + (float)(rand() % 8);
                    s.targetPos = {rx, 12.0f};
                } else {
                    dir = Vector2Normalize(dir);
                    s.pos.x += dir.x * 1.0f * simDt;
                    s.pos.y += dir.y * 1.0f * simDt;
                }
            }
        } else if (s.type == STAFF_ENGINEER) {
            // Signal Engineer patrols tracks & signals
            Vector2 dir = Vector2Subtract(s.targetPos, s.pos);
            if (Vector2Length(dir) < 0.25f) {
                if (s.isWorking) {
                    s.workTimer += simDt;
                    if (s.workTimer >= 2.5f) {
                        s.isWorking = false;
                        s.workTimer = 0.0f;
                        s.targetPos = {6.0f + (float)(rand() % 4), 12.0f + (float)(rand() % 3)};
                    }
                } else {
                    s.isWorking = true;
                    s.workTimer = 0.0f;
                }
            } else {
                dir = Vector2Normalize(dir);
                s.pos.x += dir.x * 1.1f * simDt;
                s.pos.y += dir.y * 1.1f * simDt;
            }
        }
    }

    // Weekly staff payroll
    staffWageTimer += simDt;
    if (staffWageTimer >= WEEK_DURATION) {
        staffWageTimer = 0.0f;
        float wages = 0.0f;
        for (const auto& s : staff) wages += (s.type == STAFF_CUSTODIAN ? 10.0f : 15.0f);
        if (wages > 0.0f) {
            economy.balance = std::max(0.0f, economy.balance - wages);
            ShowToast(TextFormat("Weekly transit crew payroll disbursed: $%.0f", wages), Color{148, 163, 184, 255}, 2.5f);
        }
    }

    // Rush Combo timer decay
    if (comboTimer > 0.0f) {
        comboTimer -= simDt;
        if (comboTimer <= 0.0f) {
            comboTimer = 0.0f;
            comboStreak = 0;
            rushCombo = 1.0f;
        }
    }

    if (angry > 0) {
        angryLeaves += angry;
        comboStreak = 0;
        rushCombo = 1.0f;
        comboTimer = 0.0f;
        ShowToast("[ALERT] Platform Overcrowding! Commuters left in frustration!", Color{239, 68, 68, 255}, 3.0f);
        if (parkRating <= 0.0f) {
            state = STATE_GAME_OVER;
            AudioManager::Play(SFX_QUEUE_ALARM, 0.9f);
        }
    }

    // 7. Refresh Telemetry Statistics
    cachedStats = train.GetStats(tracks);
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(Color{241, 245, 249, 255}); // Slate 100 soft architectural canvas

    if (state == STATE_TITLE) {
        ui.DrawTitleScreen(bestSessionRiders);
        EndDrawing();
        return;
    }

    // 1. Draw Ground Tiles
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            bool isHovered = (x == hoveredGx && y == hoveredGy);
            Iso::DrawTile(x, y, groundZ[x][y], terrain[x][y], cameraPos, zoom, isHovered);
        }
    }

    // 2. Draw Transit Portal Arch Marquee at entrance (0, 9)
    Iso::DrawTransitPortalArch({0.0f, 9.0f}, cameraPos, zoom);

    // 3. Draw Scenery Items (Layered with correct isometric depth)
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (scenery[x][y] != SCENERY_NONE) {
                Iso::DrawScenery(x, y, groundZ[x][y], scenery[x][y], cameraPos, zoom);
            }
        }
    }

    // 4. Draw Station Litter / Messes
    for (const auto& m : messes) {
        Iso::DrawMess(m, cameraPos, zoom);
    }

    // 5. Draw Commuters with Shape Badges
    peeps.Draw(cameraPos, zoom);

    // 6. Draw Transit Crew (Custodians & Signal Technicians)
    for (const auto& s : staff) {
        Iso::DrawStaff(s, cameraPos, zoom);
    }

    // 7. Draw Track Ballast, Concrete Sleepers, 3rd Rail, Island Platforms, Signals
    tracks.DrawAllTracks(cameraPos, zoom);

    // 8. Draw Metro EMU Rolling Stock & Commuter Passengers
    train.Draw(cameraPos, zoom);
    for (auto& extra : extraTrains) extra.Draw(cameraPos, zoom);

    // 9. Draw Particles (Sparks, Smoke, Confetti, Door Chime rings)
    particles.Draw(cameraPos, zoom);

    // 10. Draw Ghost Placement Preview
    Vector2 mousePos = GetMousePosition();
    bool overUI = (mousePos.y <= 56 || mousePos.y >= GetScreenHeight() - 150);
    if (!overUI && hoveredGx >= 0 && hoveredGx < GRID_SIZE && hoveredGy >= 0 && hoveredGy < GRID_SIZE) {
        if (isBulldozing) {
            Iso::DrawCursor(hoveredGx, hoveredGy, currentZ, cameraPos, zoom, Color{239, 68, 68, 220});
        } else {
            if (activeTab == CAT_TRACK) {
                Direction inDir, outDir;
                GetTrackPieceDirs(currentTrack, buildHeading, inDir, outDir);
                tracks.DrawGhostPiece(hoveredGx, hoveredGy, currentZ, currentTrack, inDir, outDir, cameraPos, zoom, true);
            } else if (activeTab == CAT_INFRA) {
                Iso::DrawCursor(hoveredGx, hoveredGy, 0, cameraPos, zoom, Color{56, 189, 248, 200});
            } else if (activeTab == CAT_SCENERY) {
                Iso::DrawCursor(hoveredGx, hoveredGy, 0, cameraPos, zoom, Color{74, 222, 128, 200});
                Iso::DrawScenery(hoveredGx, hoveredGy, 0, currentScenery, cameraPos, zoom);
            }
        }
    }

    // 10b. Open-circuit gap markers: two pulsing red rings + dashed connector
    //      so the player sees exactly which two tiles to bridge.
    if (state == STATE_PLAYING && !tracks.IsCircuitClosed()) {
        int ax, ay, bx, by;
        tracks.GetOpenEndpoints(ax, ay, bx, by);
        if (ax >= 0 && bx >= 0) {
            float pulse = 0.5f + 0.5f * sinf(GetTime() * 5.0f);
            Color ringC = Color{239, 68, 68, (unsigned char)(160 + 95 * pulse)};
            auto drawGapRing = [&](int gx, int gy) {
                Vector2 p = Iso::GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, 0.0f, cameraPos, zoom);
                float rw = TILE_WIDTH * zoom * 0.55f;
                float rh = TILE_HEIGHT * zoom * 0.55f;
                DrawEllipse((int)p.x, (int)p.y, rw, rh, Color{239, 68, 68, (unsigned char)(30 + 30 * pulse)});
                DrawEllipseLines((int)p.x, (int)p.y, rw, rh, ringC);
            };
            drawGapRing(ax, ay);
            drawGapRing(bx, by);
            // Dashed connector between the two endpoints
            Vector2 pa = Iso::GridToScreen((float)ax + 0.5f, (float)ay + 0.5f, 0.0f, cameraPos, zoom);
            Vector2 pb = Iso::GridToScreen((float)bx + 0.5f, (float)by + 0.5f, 0.0f, cameraPos, zoom);
            float dist = Vector2Distance(pa, pb);
            int segments = std::max(1, (int)(dist / 8.0f));
            for (int s = 0; s < segments; s += 2) {
                float t0 = (float)s / (float)segments;
                float t1 = std::min(1.0f, (float)(s + 1) / (float)segments);
                Vector2 l0 = Vector2Lerp(pa, pb, t0);
                Vector2 l1 = Vector2Lerp(pa, pb, t1);
                DrawLineEx(l0, l1, 2.0f, Color{239, 68, 68, (unsigned char)(100 + 80 * pulse)});
            }
        }
    }

    // 11. Tutorial "GO HERE" target marker (pulsing ring on the exact tile +
    //     bouncing arrow; never leaves you staring at empty void)
    DrawTutorialTarget();

    // 12. Atmospheric Day / Sunset / Night Rush Hour Lighting
    float weekPhase = fmodf(weekTimer, WEEK_DURATION);
    if (weekPhase >= 36.0f && weekPhase < 48.0f) {
        // Sunset golden hour
        float alpha = (weekPhase - 36.0f) / 12.0f;
        DrawRectangle(0, 54, GetScreenWidth(), GetScreenHeight() - 54, Color{255, 140, 0, (unsigned char)(alpha * 35.0f)});
    } else if (weekPhase >= 48.0f) {
        // Twilight evening rush hour
        float alpha = (weekPhase - 48.0f) / 12.0f;
        DrawRectangle(0, 54, GetScreenWidth(), GetScreenHeight() - 54, Color{15, 23, 42, (unsigned char)(45.0f + alpha * 65.0f)});

        // Streetlamp glowing illumination pools on platforms
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                if (scenery[x][y] == SCENERY_LAMP_POST) {
                    Vector2 sPos = Iso::GridToScreen((float)x, (float)y, 0.0f, cameraPos, zoom);
                    DrawCircleGradient(sPos, 40.0f * zoom, Color{255, 238, 88, 120}, Color{255, 238, 88, 0});
                }
            }
        }
    }

    // 12. Draw Operations Control Center (OCC) Main HUD
    ui.DrawHUD(
        economy.totalDelivered,
        parkRating,
        week,
        weekTimer,
        train.GetSpeedKmh(),
        tracks.GetActiveSignalAspect(),
        tracks.IsCircuitClosed(),
        gameSpeed,
        AudioManager::IsMuted(),
        economy.balance,
        statsWindowOpen,
        staffWindowOpen,
        rideCamActive,
        helpOverlayOpen,
        rushCombo
    );

    // 12a. Guided Tutorial Objectives Panel (hidden during modals)
    if (showObjectivePanel && !tutorialDone && state == STATE_PLAYING) {
        bool doneFlags[kTutorialCount];
        for (int i = 0; i < kTutorialCount; ++i) doneFlags[i] = GetTutorialDone(i);
        TutorialStageInfo stage;
        int idx = GetCurrentTutorialIdx();
        if (GetTutorialStageInfo(idx, stage)) {
            ui.DrawTutorialPanel(true, stage, idx, kTutorialCount, doneFlags);
        }
    }

    // 12aa. Persistent arcade OBJECTIVE chip - visible whenever the big card is
    //       hidden (G) or the tutorial is done. Always answers "what now?"
    if (state == STATE_PLAYING && (!showObjectivePanel || tutorialDone)) {
        if (!tutorialDone) {
            TutorialStageInfo st;
            if (GetTutorialStageInfo(GetCurrentTutorialIdx(), st) && st.title[0]) {
                ui.DrawObjectiveChip("NEXT TASK", st.title, -1.0f);
            }
        } else if (endlessMode) {
            int m = ((lastMilestoneAwarded / 250) + 1) * 250;
            float pct = (float)(economy.totalDelivered - (m - 250)) / 250.0f;
            ui.DrawObjectiveChip("ENDLESS METROPOLIS", TextFormat("Next subsidy grant at %d riders", m), pct);
        } else {
            int left = std::max(0, 500 - economy.totalDelivered);
            ui.DrawObjectiveChip("VICTORY GOAL", TextFormat("Deliver %d more riders (%d / 500)", left, economy.totalDelivered), (float)economy.totalDelivered / 500.0f);
        }
    }

    // 12b. Arcade MISSION BRIEFING overlay (0.5s fade-out at the end)
    if (briefingTimer > 0.0f) {
        float fade = std::min(1.0f, briefingTimer / 0.5f);
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        DrawRectangle(0, 0, screenW, screenH, Color{2, 6, 23, (unsigned char)(238.0f * fade)});
        DrawGameBoldTextCentered("MISSION BRIEFING", (float)screenW / 2.0f, (float)(screenH / 2 - 130), 40, Color{255, 214, 0, (unsigned char)(255 * fade)});
        DrawText("Your EMU is already running its loop.", screenW / 2 - 150, screenH / 2 - 72, 14, Color{248, 250, 252, (unsigned char)(255 * fade)});
        DrawText("Follow the gold LESSON card - a glowing ring points at the spot.", screenW / 2 - 215, screenH / 2 - 44, 14, Color{203, 213, 225, (unsigned char)(255 * fade)});
        DrawText("Each lesson passed pays a cash BONUS.  (G hides lessons)", screenW / 2 - 180, screenH / 2 - 16, 13, Color{148, 163, 184, (unsigned char)(255 * fade)});
    }

    // 12c. Arcade RUSH HOUR banner (pulsing, dropped during peak windows)
    if (rushHourActive && briefingTimer <= 0.0f) {
        float rp = 0.5f + 0.5f * sinf(GetTime() * 6.0f);
        int rw = 300;
        int rh = 32;
        int rx = ((int)GetScreenWidth() - rw) / 2;
        int ry = 106;
        DrawRectangleRounded(Rectangle{(float)rx, (float)ry, (float)rw, (float)rh}, 0.5f, 4, Color{185, 28, 28, (unsigned char)(215 + 40 * rp)});
        DrawRectangleRoundedLines(Rectangle{(float)rx, (float)ry, (float)rw, (float)rh}, 0.5f, 4, Color{254, 202, 202, (unsigned char)(220 * rp)});
        DrawGameBoldTextCentered("RUSH HOUR!  FARES x1.25", (float)GetScreenWidth() / 2.0f, (float)ry + 7, 15, Color{255, 255, 255, (unsigned char)(220 + 35 * rp)});
    }

    // 12b. Draw Contextual Quick Tip Banner (hidden while the lesson card is
    //      up - one piece of guidance at a time, never a wall of text)
    if (!(showObjectivePanel && !tutorialDone)) {
        std::string quickTip;
        if (isBulldozing) {
            quickTip = "DEMOLISH MODE: click a tile to remove it (X to exit)";
        } else if (train.CanBoard() || train.GetState() == TRAIN_BOARDING || train.GetState() == TRAIN_STOPPED_IN_STATION) {
            quickTip = "Train is at a station - commuters are boarding!";
        } else if (activeTab == CAT_TRACK) {
            if (tracks.IsCircuitClosed()) {
                quickTip = "Train running! Tab 2/3 or keys 1-8 to build more";
            } else {
                quickTip = "Connect the track into a closed loop so the train can run";
            }
        } else if (activeTab == CAT_INFRA) {
            quickTip = "Concourse: 1 Sidewalk  2 Queue  3 Plaza (X demolish)";
        } else if (activeTab == CAT_SCENERY) {
            quickTip = "Scenery: 1 Entrance  2 Gates  3 Tree  5 Bench  8 Cafe";
        }
        ui.DrawQuickTipBanner(quickTip);
    }

    // 13. Draw Categorized Toolbar
    ui.DrawToolbar(
        activeTab,
        currentTrack,
        currentScenery,
        currentGround,
        currentZ,
        buildHeading,
        isBulldozing,
        economy.balance,
        train.GetCarriageCount(),
        buildRadius,
        GetExtraTrainCount()
    );

    // 14. Draw Line Operations Window
    if (statsWindowOpen) {
        ui.DrawLineOperations(cachedStats);
    }

    // 15. Draw Transit Crew Window
    if (staffWindowOpen) {
        ui.DrawTransitCrewWindow(staff, parkCleanliness, economy.balance);
    }

    // 16. Draw Commuter Inspector
    if (selectedPeepIdx != -1) {
        const Commuter* p = peeps.GetCommuter(selectedPeepIdx);
        if (p) {
            ui.DrawCommuterInspector(p);
        } else {
            selectedPeepIdx = -1;
        }
    }

    // 17. Draw Toast Notifications (skip while the lesson card is visible)
    if (!(showObjectivePanel && !tutorialDone)) {
        ui.DrawToast(activeToast);
    }

    // 18. Draw Operations Manual
    if (helpOverlayOpen) {
        ui.DrawTransitOperationsManual();
    }

    // 19. Draw Modals (Weekly Grant, Game Over, Victory) + Arcade Pause overlay
    if (state == STATE_WEEKLY_UPGRADE) {
        int ch = ui.CheckUpgradeModalClick(GetMousePosition());
        ui.DrawWeeklyModal(activeUpgrades, ch);
    } else if (state == STATE_GAME_OVER) {
        int stars = 1 + ((economy.totalDelivered >= 500) ? 1 : 0) + ((week <= 4) ? 1 : 0);
        ui.DrawGameOver(economy.totalDelivered, stars, bestSessionRiders);
    } else if (state == STATE_VICTORY) {
        int stars = 1 + ((economy.totalDelivered >= 500) ? 1 : 0) + ((week <= 4) ? 1 : 0);
        ui.DrawVictory(economy.totalDelivered, week, stars, economy.balance, bestSessionRiders);
    } else if (state == STATE_PLAYING && (gameSpeed == 0 || isPaused)) {
        ui.DrawPauseOverlay();
    }

    EndDrawing();
}

bool Game::ShouldClose() const {
    return WindowShouldClose();
}

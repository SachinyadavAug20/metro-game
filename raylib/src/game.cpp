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

    zoom = 1.15f;
    SetupInitialPark();
    tracks.InitDefaultCircuit();
    train.Reset(tracks);
    extraTrains.clear();
    peeps.Init({1.0f, 9.0f}, {7.0f, 8.0f}, {6.0f, 10.0f});
    particles.Clear();

    // Center camera on the metropolitan transit circuit and Central Hub
    RecenterCamera();

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
    stateEntryTime = GetTime();
    lastMilestoneAwarded = 0;
    rushCombo = 1.0f;
    comboTimer = 0.0f;
    comboStreak = 0;
    endlessMode = false;
    buildRadius = 38;

    statsWindowOpen = false;
    staffWindowOpen = false;
    rideCamActive = false;
    helpOverlayOpen = false;
    selectedPeepIdx = -1;
    selectedStationGx = -1;
    selectedStationGy = -1;

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
    buildRadius = std::min(100, buildRadius + 16);

    // Physically reclaim coastal water cells in the newly expanded territory into buildable land!
    int reclaimedCount = 0;
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            int d = std::abs(x - LAND_CENTER_X) + std::abs(y - LAND_CENTER_Y);
            if (d <= buildRadius && terrain[x][y] == GROUND_WATER) {
                // Determine if this cell is on the outer boundary (create sand beach) or inner (emerald grass)
                bool outerShore = (d >= buildRadius - 2);
                terrain[x][y] = outerShore ? GROUND_SAND : GROUND_GRASS;
                reclaimedCount++;
                if ((reclaimedCount % 4) == 0) {
                    particles.SpawnConfetti(Vector3{(float)x + 0.5f, (float)y + 0.5f, 0.5f}, 6);
                }
            }
        }
    }

    const char* districtNames[] = {
        "Suburban Green Heights",
        "Highland Mountain Vista",
        "Marina Bay & Waterfront",
        "Emerald Valley & Lake",
        "Northgate Pine Plateau",
        "Grand Metropolis Megalopolis"
    };
    int distIdx = std::min(5, (buildRadius - 36) / 12);
    const char* distName = districtNames[std::max(0, distIdx)];

    particles.SpawnConfetti(Vector3{(float)LAND_CENTER_X, (float)LAND_CENTER_Y, 1.0f}, 40);
    particles.SpawnFloatingText(Vector3{(float)LAND_CENTER_X, (float)LAND_CENTER_Y, 2.0f}, TextFormat("+%d TILES RECLAIMED!", reclaimedCount), Color{56, 189, 248, 255});
    ShowToast(TextFormat("TERRITORY EXPANDED: %s unlocked! (+%d water cells converted to land)", distName, reclaimedCount), Color{34, 197, 94, 255}, 5.0f);
    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
}



void Game::SetupInitialPark() {
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            terrain[x][y] = GROUND_GRASS;
            groundZ[x][y] = 0;
            scenery[x][y] = SCENERY_NONE;
        }
    }

    // 1. Natural Scenic Winding River Canal
    // Passes under the Marina Viaduct elevated bridge at (12, 9) & (13, 9)
    // and over the University subway tunnel at (12, 16) & (13, 16)
    for (int y = 0; y <= 25; ++y) {
        terrain[12][y] = GROUND_WATER;
        terrain[13][y] = GROUND_WATER;
        // Natural golden sand beaches along both banks of the river
        if (terrain[11][y] == GROUND_GRASS) terrain[11][y] = GROUND_SAND;
        if (terrain[14][y] == GROUND_GRASS) terrain[14][y] = GROUND_SAND;
    }

    // 2. Grand Southern Coastal Ocean & Sandy Bay (Scalloped natural shoreline)
    for (int x = 12; x < GRID_SIZE; ++x) {
        float coastY = 28.0f - 1.8f * sinf((float)x * 0.42f);
        for (int y = (int)coastY - 2; y < GRID_SIZE; ++y) {
            bool isIsland = (x >= 31 && x <= 37 && y >= 33 && y <= 39);
            if (isIsland) {
                // Offshore Island: lush green center surrounded by sandy beach
                bool shore = (x == 31 || x == 37 || y == 33 || y == 39);
                terrain[x][y] = shore ? GROUND_SAND : GROUND_GRASS;
            } else if ((float)y >= coastY) {
                terrain[x][y] = GROUND_WATER;
            } else if ((float)y >= coastY - 1.8f) {
                terrain[x][y] = GROUND_SAND; // Coastal sand beach
            }
        }
    }

    // 3. Western Suburbs Freshwater Lake (Natural rounded oval with sand perimeter)
    for (int x = 2; x <= 10; ++x) {
        for (int y = 26; y <= 34; ++y) {
            float dx = (float)(x - 6);
            float dy = (float)(y - 30);
            float d = sqrtf(dx * dx + dy * dy);
            if (d <= 3.2f) {
                terrain[x][y] = GROUND_WATER;
            } else if (d <= 4.2f && terrain[x][y] == GROUND_GRASS) {
                terrain[x][y] = GROUND_SAND; // Lake shoreline
            }
        }
    }

    // 4. Highland Mountains & Rolling Hills (Elevation Z=1 and Z=2) in the East
    // Natural organic rounded massif with rolling green foothills and rocky slate summits
    for (int x = 24; x < GRID_SIZE; ++x) {
        for (int y = 2; y <= 26; ++y) {
            float dx = (float)(x - 36) / 10.0f;
            float dy = (float)(y - 14) / 10.0f;
            float distSq = dx * dx + dy * dy;
            if (distSq < 0.38f && x >= 32 && y >= 6 && y <= 20) {
                groundZ[x][y] = 2;
                terrain[x][y] = GROUND_STONE; // High alpine slate summit
            } else if (distSq < 1.05f && x >= 26) {
                groundZ[x][y] = 1; // Rolling foothills
            }
        }
    }

    // 5. Pedestrian sidewalks connecting entrance plaza to Central Hub Station
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

    // Landscaped Central Park Plaza inside the transit loop (x=8..10, y=11..14)
    for (int x = 8; x <= 10; ++x) {
        for (int y = 11; y <= 14; ++y) {
            if (terrain[x][y] == GROUND_GRASS) terrain[x][y] = GROUND_PLAZA;
        }
    }

    // 6. Urban Transit Station Amenities & Scenery
    scenery[0][9] = SCENERY_METRO_ENTRANCE; // Grand subway portal entrance
    scenery[2][9] = SCENERY_TURNSTILE_GATE; // Contactless fare gates & TVM ticket machine
    scenery[3][8] = SCENERY_MAP_KIOSK;      // Harry Beck style schematic transit map board
    scenery[5][7] = SCENERY_NEWSSTAND;      // Platform Metro Cafe & refreshments
    scenery[1][10] = SCENERY_BIKE_RACK;     // Metro bike share docking rack
    scenery[4][7] = SCENERY_FOUNTAIN;       // Splashing park water fountain
    scenery[9][12] = SCENERY_FOUNTAIN;      // Central park ornamental fountain

    // Platform Benches
    scenery[6][8] = SCENERY_BENCH;
    scenery[8][8] = SCENERY_BENCH;
    scenery[8][11] = SCENERY_BENCH;
    scenery[10][11] = SCENERY_BENCH;

    // High-Efficiency Municipal LED Streetlamps
    scenery[3][8] = SCENERY_LAMP_POST;
    scenery[8][10] = SCENERY_LAMP_POST;
    scenery[5][12] = SCENERY_LAMP_POST;
    scenery[8][14] = SCENERY_LAMP_POST;
    scenery[10][14] = SCENERY_LAMP_POST;

    // Manicured Trees & Botanical Flower Beds
    scenery[1][8] = SCENERY_STREET_TREE;
    scenery[3][10] = SCENERY_STREET_TREE;
    scenery[5][10] = SCENERY_STREET_TREE;
    scenery[4][10] = SCENERY_FLOWER_BED;
    scenery[6][7]  = SCENERY_FLOWER_BED;
    scenery[9][11] = SCENERY_FLOWER_BED;
    scenery[9][14] = SCENERY_FLOWER_BED;

    // Metropolitan Park Pine Trees across the canal
    scenery[17][8]  = SCENERY_PINE_TREE;
    scenery[17][12] = SCENERY_PINE_TREE;
    scenery[17][16] = SCENERY_PINE_TREE;
    scenery[15][18] = SCENERY_PINE_TREE;

    // Natural Organic Pine Forests on the Highland Mountains (Clustered)
    for (int px = 26; px < GRID_SIZE - 2; ++px) {
        for (int py = 3; py < 25; ++py) {
            if (terrain[px][py] != GROUND_WATER && groundZ[px][py] >= 1 && scenery[px][py] == SCENERY_NONE) {
                unsigned int th = ((unsigned int)px * 1597334677u) ^ ((unsigned int)py * 3812015801u);
                if ((th % 4) == 0) {
                    scenery[px][py] = SCENERY_PINE_TREE;
                }
            }
        }
    }

    // Lakeside Amenities & Trees
    scenery[2][30] = SCENERY_STREET_TREE;
    scenery[2][32] = SCENERY_STREET_TREE;
    scenery[9][30] = SCENERY_STREET_TREE;
    scenery[9][33] = SCENERY_STREET_TREE;
    scenery[6][26] = SCENERY_BENCH;
    scenery[6][34] = SCENERY_BENCH;
    scenery[6][30] = SCENERY_FOUNTAIN; // Splashing lake fountain

    // Offshore Island Scenery
    scenery[34][36] = SCENERY_FOUNTAIN;
    scenery[33][35] = SCENERY_STREET_TREE;
    scenery[35][37] = SCENERY_PINE_TREE;
}

void Game::ResetPark() {
    Init();
}

void Game::ClampCamera() {
    int screenW = GetScreenWidth();
    if (screenW <= 0) screenW = 1280;
    int screenH = GetScreenHeight();
    if (screenH <= 0) screenH = 720;

    Vector2 centerGrid = Iso::ScreenToGrid(Vector2{(float)screenW * 0.5f, (float)screenH * 0.5f}, cameraPos, zoom, 0.0f);
    
    // Bounds for where the center of the player's viewport is allowed to point:
    // Grid is 48x48. Clamping the center between [3.0f, 45.0f] guarantees
    // the metropolitan island is ALWAYS firmly in view, and the screen can never wander into empty void.
    const float kMinGrid = 3.0f;
    const float kMaxGrid = 45.0f;
    bool clamped = false;
    if (centerGrid.x < kMinGrid) { centerGrid.x = kMinGrid; clamped = true; }
    if (centerGrid.x > kMaxGrid) { centerGrid.x = kMaxGrid; clamped = true; }
    if (centerGrid.y < kMinGrid) { centerGrid.y = kMinGrid; clamped = true; }
    if (centerGrid.y > kMaxGrid) { centerGrid.y = kMaxGrid; clamped = true; }

    if (clamped) {
        float halfW = (TILE_WIDTH / 2.0f) * zoom;
        float halfH = (TILE_HEIGHT / 2.0f) * zoom;
        cameraPos.x = (float)screenW * 0.5f - (centerGrid.x - centerGrid.y) * halfW;
        cameraPos.y = (float)screenH * 0.5f - (centerGrid.x + centerGrid.y) * halfH;
    }
}

void Game::RecenterCamera() {
    int screenW = GetScreenWidth();
    if (screenW <= 0) screenW = 1280;
    int screenH = GetScreenHeight();
    if (screenH <= 0) screenH = 720;

    // Center on the lead train, or fallback to Central Hub station platform (7, 9)
    float targetGx = 7.0f;
    float targetGy = 9.0f;
    Vector3 lp = train.GetLocomotivePos();
    if (lp.x >= 2.0f && lp.y >= 2.0f && lp.x < (float)GRID_SIZE - 2.0f && lp.y < (float)GRID_SIZE - 2.0f) {
        targetGx = lp.x;
        targetGy = lp.y;
    }

    float halfW = (TILE_WIDTH / 2.0f) * zoom;
    float halfH = (TILE_HEIGHT / 2.0f) * zoom;
    cameraPos.x = (float)screenW * 0.5f - (targetGx - targetGy) * halfW;
    cameraPos.y = (float)screenH * 0.5f - (targetGx + targetGy) * halfH;
    ClampCamera();
    ShowToast("CAMERA RECENTERED (C / HOME)", Color{56, 189, 248, 255}, 2.0f);
    AudioManager::Play(SFX_BUTTON_CLICK, 0.5f);
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
            RecenterCamera();
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

    // 1. Camera Panning with WASD / Arrow Keys (100% manual player control)
    float panSpeed = 380.0f * zoom * GetFrameTime();
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { cameraPos.y += panSpeed; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  { cameraPos.y -= panSpeed; }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  { cameraPos.x += panSpeed; }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { cameraPos.x -= panSpeed; }

    // Mouse Drag Pan with RIGHT or MIDDLE button (grab the map and pull)
    bool dragKey = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
        dragStart = mousePos;
        isDragging = true;
    }
    if (isDragging) {
        if (dragKey) {
            cameraPos.x += (mousePos.x - dragStart.x);
            cameraPos.y += (mousePos.y - dragStart.y);
            dragStart = mousePos;
        } else {
            isDragging = false;
        }
    }

    // Zoom with mouse scroll wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        zoom = std::max(0.65f, std::min(2.4f, zoom + wheel * 0.1f));
    }

    // Strict Camera Clamping: island is ALWAYS in view, never travels to void
    ClampCamera();

    // Hotkeys
    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_HOME)) {
        RecenterCamera();
    }
    if (IsKeyPressed(KEY_N)) {
        nightMode = !nightMode;
        ShowToast(nightMode ? "NIGHT VISTA: City lights active [Press N to toggle]" : "DAYLIGHT: Standard view", nightMode ? Color{147, 197, 253, 255} : Color{250, 204, 21, 255}, 2.0f);
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_B)) {
        if (!tracks.IsCircuitClosed()) {
            int piecesPlaced = 0;
            if (tracks.AutoBridgeCircuitGap(piecesPlaced)) {
                float cost = piecesPlaced * 40.0f;
                if (economy.balance >= cost) economy.balance -= cost; else economy.balance = 0.0f;
                train.Reset(tracks);
                AudioManager::Play(SFX_CONSTRUCTION, 1.0f);
                ShowToast(TextFormat("Loop Connected! Auto-placed %d rails (-$%.0f)", piecesPlaced, cost), Color{34, 197, 94, 255}, 4.0f);
            } else {
                ShowToast("Could not auto-bridge track gap (align endpoints or remove obstacles)", Color{239, 68, 68, 255}, 3.0f);
            }
        }
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
    if (IsKeyPressed(KEY_L)) {
        currentLineId = (currentLineId % 6) + 1;
        Color liveries[] = {
            Color{229, 57, 53, 255},  // Tokyo Red (Marunouchi)
            Color{37, 99, 235, 255},  // London Blue (Piccadilly)
            Color{16, 185, 129, 255}, // Paris Green (Line 6)
            Color{147, 51, 234, 255}, // Victoria Purple
            Color{245, 158, 11, 255}, // Chicago Amber
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
        int idx = currentLineId - 1;
        cachedStats.themeColor = liveries[idx];
        cachedStats.lineName = names[idx];
        tracks.SetTrackColor(liveries[idx]);
        train.SetTrainTheme(liveries[idx]);
        train.SetLineName(names[idx]);
        for (auto& ex : extraTrains) {
            ex.SetTrainTheme(liveries[idx]);
            ex.SetLineName(names[idx]);
        }
        ShowToast(TextFormat("Route Theme: %s [Press L to cycle]", names[idx]), liveries[idx], 2.5f);
        AudioManager::Play(SFX_SMARTCARD_BEEP, 0.7f);
    }
    if (IsKeyPressed(KEY_TAB)) {
        activeTab = (ToolCategory)((activeTab + 1) % 3);
        isBulldozing = false;
        isTerraformingRaise = false;
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
    if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_NINE)) {
        isBulldozing = !isBulldozing;
        isTerraformingRaise = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_ONE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_STRAIGHT;
        else if (activeTab == CAT_INFRA) { currentGround = GROUND_PATH; isTerraformingRaise = false; }
        else currentScenery = SCENERY_METRO_ENTRANCE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_TWO)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_CURVE_LEFT;
        else if (activeTab == CAT_INFRA) { currentGround = GROUND_QUEUE; isTerraformingRaise = false; }
        else currentScenery = SCENERY_TURNSTILE_GATE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_THREE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_CURVE_RIGHT;
        else if (activeTab == CAT_INFRA) { currentGround = GROUND_PLAZA; isTerraformingRaise = false; }
        else currentScenery = SCENERY_STREET_TREE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FOUR)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_VIADUCT_ELEVATED;
        else if (activeTab == CAT_INFRA) { currentGround = GROUND_GRASS; isTerraformingRaise = false; }
        else currentScenery = SCENERY_PINE_TREE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FIVE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_VIADUCT_SLOPE;
        else if (activeTab == CAT_INFRA) { currentGround = GROUND_WATER; isTerraformingRaise = false; }
        else currentScenery = SCENERY_BENCH;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SIX)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_TUNNEL_PORTAL;
        else if (activeTab == CAT_INFRA) { currentGround = GROUND_SAND; isTerraformingRaise = false; }
        else currentScenery = SCENERY_LAMP_POST;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SEVEN)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_STATION;
        else if (activeTab == CAT_INFRA) { currentGround = GROUND_STONE; isTerraformingRaise = false; }
        else currentScenery = SCENERY_FOUNTAIN;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_EIGHT)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_SIGNAL;
        else if (activeTab == CAT_INFRA) { isTerraformingRaise = true; }
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
        bool autoBridge = false;
        if (ui.CheckHUDClick(mousePos, newSpeed, toggleMute, toggleStats, toggleStaff, toggleCam, toggleHelp, &autoBridge)) {
            if (autoBridge) {
                if (tracks.IsCircuitClosed()) {
                    RecenterCamera();
                } else {
                    int piecesPlaced = 0;
                    if (tracks.AutoBridgeCircuitGap(piecesPlaced)) {
                        float cost = piecesPlaced * 40.0f;
                        if (economy.balance >= cost) economy.balance -= cost; else economy.balance = 0.0f;
                        train.Reset(tracks);
                        AudioManager::Play(SFX_CONSTRUCTION, 1.0f);
                        ShowToast(TextFormat("Loop Connected! Auto-placed %d rails (-$%.0f)", piecesPlaced, cost), Color{34, 197, 94, 255}, 4.0f);
                    } else {
                        ShowToast("Could not auto-bridge: align endpoints closer or remove obstacles", Color{239, 68, 68, 255}, 3.5f);
                    }
                }
                return;
            }
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

        // Peep Inspector Close Click or Body Click
        if (selectedPeepIdx != -1) {
            if (ui.CheckPeepInspectorCloseClick(mousePos)) {
                selectedPeepIdx = -1;
                AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
                return;
            }
            if (ui.IsMouseInPeepInspector(mousePos)) {
                return; // consume click on commuter card
            }
        }

        // Station Inspector Close Click or Upgrade Click
        if (selectedStationGx != -1 && selectedStationGy != -1) {
            const TrackNode* stNode = tracks.GetStationAt(selectedStationGx, selectedStationGy);
            if (stNode) {
                bool doUpgrade = false;
                bool doClose = false;
                if (ui.CheckStationInspectorClick(mousePos, stNode, doUpgrade, doClose)) {
                    if (doClose) {
                        selectedStationGx = -1;
                        selectedStationGy = -1;
                        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
                        return;
                    }
                    if (doUpgrade) {
                        float cost = (stNode->stationLevel == 1) ? 250.0f : 500.0f;
                        if (economy.balance >= cost) {
                            int newLvl = 1;
                            if (tracks.UpgradeStation(selectedStationGx, selectedStationGy, newLvl)) {
                                economy.balance -= cost;
                                AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
                                Vector3 stPos = {(float)selectedStationGx + 0.5f, (float)selectedStationGy + 0.5f, (float)stNode->gz + 1.0f};
                                particles.SpawnConfetti(stPos, 36);
                                particles.SpawnFloatingText(stPos, (newLvl == 2 ? "+LV2 MODERN CONCOURSE!" : "+LV3 GRAND TERMINAL!"), Color{250, 204, 21, 255});
                                ShowToast(TextFormat("Station %s upgraded to Level %d Concourse! (+%d%% fare bonus)", stNode->stationName.c_str(), newLvl, newLvl == 2 ? 25 : 50), Color{34, 197, 94, 255}, 4.0f);
                            }
                        } else {
                            ShowToast(TextFormat("Need $%.0f to upgrade station concourse!", cost), Color{239, 68, 68, 255}, 2.5f);
                        }
                        return;
                    }
                    return; // consume click on station inspector card
                }
            } else {
                selectedStationGx = -1;
                selectedStationGy = -1;
            }
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
                isTerraformingRaise = false;
            } else if (activeTab == CAT_INFRA) {
                GroundType gTypes[] = {
                    GROUND_PATH, GROUND_QUEUE, GROUND_PLAZA,
                    GROUND_GRASS, GROUND_WATER, GROUND_SAND,
                    GROUND_STONE, GROUND_GRASS, GROUND_GRASS
                };
                if (itemIdx == 8) {
                    isBulldozing = true;
                    isTerraformingRaise = false;
                } else if (itemIdx == 7) {
                    isTerraformingRaise = true;
                    isBulldozing = false;
                } else {
                    isBulldozing = false;
                    isTerraformingRaise = false;
                    currentGround = gTypes[itemIdx];
                }
            } else if (activeTab == CAT_SCENERY) {
                SceneryType sTypes[] = {
                    SCENERY_METRO_ENTRANCE, SCENERY_TURNSTILE_GATE, SCENERY_STREET_TREE,
                    SCENERY_PINE_TREE, SCENERY_BENCH, SCENERY_LAMP_POST,
                    SCENERY_FOUNTAIN, SCENERY_NEWSSTAND, SCENERY_NONE
                };
                if (itemIdx == 8) isBulldozing = true;
                else { isBulldozing = false; currentScenery = sTypes[itemIdx]; }
                isTerraformingRaise = false;
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
                    ReclaimLand();
                } else {
                    ShowToast("Need $" + std::to_string((int)LAND_EXPAND_COST) + " to expand district!", Color{239, 68, 68, 255});
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
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    bool clickBlockedByUI = (mousePos.y <= 56 || mousePos.y >= screenH - 150);
    if (statsWindowOpen && mousePos.x >= screenW - 360 && mousePos.y >= 58 && mousePos.y <= 540) clickBlockedByUI = true;
    if (staffWindowOpen && mousePos.x >= screenW - 360 && mousePos.y >= 58 && mousePos.y <= 420) clickBlockedByUI = true;
    if (selectedPeepIdx != -1 && ui.IsMouseInPeepInspector(mousePos)) clickBlockedByUI = true;
    if (selectedStationGx != -1 && ui.IsMouseInStationInspector(mousePos)) clickBlockedByUI = true;
    if (mousePos.x <= 270 && mousePos.y >= 58 && mousePos.y <= 120) clickBlockedByUI = true;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !clickBlockedByUI) {
        // Check if clicking on a commuter
        int peepUnderMouse = peeps.FindCommuterAtScreenPos(mousePos, cameraPos, zoom);
        if (peepUnderMouse != -1) {
            selectedPeepIdx = peepUnderMouse;
            selectedStationGx = -1;
            selectedStationGy = -1;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
            return;
        }

        // Check if clicking on an existing station platform to inspect concourse
        if (insideGrid && !isBulldozing && currentTrack != TRACK_STATION) {
            const TrackNode* stClicked = tracks.GetStationAt(hoveredGx, hoveredGy);
            if (stClicked) {
                selectedStationGx = hoveredGx;
                selectedStationGy = hoveredGy;
                selectedPeepIdx = -1;
                AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
                return;
            }
        }

        if (insideGrid) {
            if (isBulldozing) {
                // Bulldoze priority: Scenery -> Track -> Path
                if (scenery[hoveredGx][hoveredGy] != SCENERY_NONE) {
                    scenery[hoveredGx][hoveredGy] = SCENERY_NONE;
                    economy.balance += 15.0f;
                    shakeTimer = 0.1f;
                    AudioManager::Play(SFX_BULLDOZE, 0.7f);
                    particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 10);
                    particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ + 0.5f}, "+$15", Color{34, 197, 94, 255});
                } else if (tracks.HasPiece(hoveredGx, hoveredGy)) {
                    bool wasClosed = tracks.IsCircuitClosed();
                    tracks.RemovePiece(hoveredGx, hoveredGy);
                    economy.balance += 25.0f;
                    shakeTimer = 0.12f;
                    AudioManager::Play(SFX_BULLDOZE, 0.8f);
                    particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 12);
                    particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ + 0.5f}, "+$25", Color{34, 197, 94, 255});
                    if (wasClosed && !tracks.IsCircuitClosed()) {
                        ShowToast("[CIRCUIT] Loop broken! Reconnect the track to make a closed loop.", Color{239, 68, 68, 255}, 4.0f);
                    }
                } else if (groundZ[hoveredGx][hoveredGy] > 0) {
                    groundZ[hoveredGx][hoveredGy]--;
                    AudioManager::Play(SFX_BULLDOZE, 0.65f);
                    particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)groundZ[hoveredGx][hoveredGy] + 0.5f}, 10);
                    particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.8f}, "Z-1 (Flattened)", Color{250, 204, 21, 255});
                } else if (terrain[hoveredGx][hoveredGy] != GROUND_GRASS && terrain[hoveredGx][hoveredGy] != GROUND_WATER) {
                    terrain[hoveredGx][hoveredGy] = GROUND_GRASS;
                    economy.balance += 5.0f;
                    AudioManager::Play(SFX_BULLDOZE, 0.6f);
                    particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.5f}, "+$5", Color{34, 197, 94, 255});
                }
            } else {
                // Active placement based on Tab
                // Territory expansion check: auto-expand if outside current bounds and funds permit
                if (!IsBuildable(hoveredGx, hoveredGy)) {
                    if (economy.balance >= LAND_EXPAND_COST) {
                        economy.balance -= LAND_EXPAND_COST;
                        ReclaimLand();
                        ShowToast("Boundary expanded to encompass new territory! (-$500)", Color{34, 197, 94, 255}, 3.0f);
                    } else {
                        ShowToast("Outside territory! Click EXPAND ($500) on toolbar to expand bounds.", Color{239, 68, 68, 255}, 3.0f);
                        return;
                    }
                }

                // Water cell auto-removal: Building tracks or scenery over water converts water to solid land!
                if (terrain[hoveredGx][hoveredGy] == GROUND_WATER && activeTab != CAT_INFRA) {
                    bool elevatedTrack = (activeTab == CAT_TRACK && (currentZ > 0 || currentTrack == TRACK_VIADUCT_ELEVATED || currentTrack == TRACK_VIADUCT_SLOPE));
                    if (!elevatedTrack) {
                        float reclaimCost = 25.0f;
                        if (economy.balance >= reclaimCost + 20.0f) {
                            terrain[hoveredGx][hoveredGy] = GROUND_GRASS;
                            economy.balance -= reclaimCost;
                            particles.SpawnConfetti(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.5f}, 10);
                            particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.7f}, "-$25 (Land Reclaimed)", Color{34, 197, 94, 255});
                            ShowToast("Water cell reclaimed to solid land foundation!", Color{34, 197, 94, 255}, 2.0f);
                        } else {
                            ShowToast("Need $25 land reclamation fee to build over water canal!", Color{239, 68, 68, 255}, 2.5f);
                            return;
                        }
                    }
                }
                if (activeTab == CAT_TRACK) {
                    if (economy.balance >= 40.0f) {
                        bool wasClosed = tracks.IsCircuitClosed();
                        Direction inDir, outDir;
                        GetTrackPieceDirs(currentTrack, buildHeading, inDir, outDir);
                        if (tracks.AddPiece(hoveredGx, hoveredGy, currentZ, currentTrack, inDir, outDir, cachedStats.themeColor)) {
                            economy.balance -= 40.0f;
                            shakeTimer = 0.15f; // subtle screen shake
                            AudioManager::Play(SFX_CONSTRUCTION, 0.8f);
                            particles.SpawnSparks(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 14);
                            particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ + 0.6f}, "-$40", Color{239, 68, 68, 255});

                            if (!wasClosed && tracks.IsCircuitClosed()) {
                                ShowToast("[CIRCUIT] Transit Loop Closed! Regular EMU Schedule Active!", Color{34, 197, 94, 255}, 4.0f);
                                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
                                circuitFlashTimer = 0.8f;
                            } else if (wasClosed && !tracks.IsCircuitClosed()) {
                                ShowToast("[CIRCUIT] Loop broken! Reconnect the track to make a closed loop.", Color{239, 68, 68, 255}, 4.0f);
                            }

                            // Landmark discovery checks for exploring distant biomes (Minecraft / Terraria style)
                            if (currentTrack == TRACK_STATION) {
                                if (hoveredGx >= 26 && groundZ[hoveredGx][hoveredGy] >= 1) {
                                    ShowToast("LANDMARK DISCOVERED: Alpine Highland Summit Station (+30% Scenic Fare!)", Color{56, 189, 248, 255}, 5.0f);
                                    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
                                    particles.SpawnConfetti(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 2.0f}, 26);
                                } else if (hoveredGy >= 27) {
                                    ShowToast("LANDMARK DISCOVERED: Marina Bay Coastal Terminal (+30% Tourist Flow!)", Color{52, 211, 153, 255}, 5.0f);
                                    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
                                    particles.SpawnConfetti(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 1.0f}, 26);
                                } else if (hoveredGx <= 9 && hoveredGy >= 22) {
                                    ShowToast("LANDMARK DISCOVERED: Emerald Lake Sanctuary Station (+25% Green Transit!)", Color{250, 204, 21, 255}, 5.0f);
                                    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
                                    particles.SpawnConfetti(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 1.0f}, 26);
                                }
                            }

                            // Auto-advance cursor forward along track exit direction and sync build heading!
                            Vector2 fwd = GetDirectionOffset(outDir);
                            hoveredGx += (int)fwd.x;
                            hoveredGy += (int)fwd.y;
                            buildHeading = outDir; // Auto-align next track piece
                            if (currentTrack == TRACK_VIADUCT_ELEVATED) currentZ++;
                            if (currentTrack == TRACK_VIADUCT_SLOPE) currentZ = std::max(0, currentZ - 1);
                        }
                    } else {
                        ShowToast("Insufficient funds to lay track ($40 required)", Color{239, 68, 68, 255}, 2.0f);
                    }
                } else if (activeTab == CAT_INFRA) {
                    if (isTerraformingRaise) {
                        if (economy.balance >= 40.0f) {
                            economy.balance -= 40.0f;
                            groundZ[hoveredGx][hoveredGy] = std::min(4, groundZ[hoveredGx][hoveredGy] + 1);
                            if (terrain[hoveredGx][hoveredGy] == GROUND_WATER) {
                                terrain[hoveredGx][hoveredGy] = GROUND_GRASS;
                            }
                            shakeTimer = 0.12f;
                            AudioManager::Play(SFX_CONSTRUCTION, 0.85f);
                            particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)groundZ[hoveredGx][hoveredGy]}, 14);
                            particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)groundZ[hoveredGx][hoveredGy] + 0.5f}, "Z+1 (-$40)", Color{56, 189, 248, 255});
                            ShowToast("Raised terrain hill elevation +1 Z! ($40)", Color{56, 189, 248, 255}, 2.0f);
                        } else {
                            ShowToast("Insufficient funds to raise hill ($40 required)", Color{239, 68, 68, 255}, 2.0f);
                        }
                    } else if (currentGround == GROUND_WATER) {
                        // Excavate water canal!
                        if (economy.balance >= 30.0f) {
                            economy.balance -= 30.0f;
                            terrain[hoveredGx][hoveredGy] = GROUND_WATER;
                            scenery[hoveredGx][hoveredGy] = SCENERY_NONE;
                            groundZ[hoveredGx][hoveredGy] = 0;
                            AudioManager::Play(SFX_BULLDOZE, 0.75f);
                            particles.SpawnSparks(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.4f}, 10);
                            particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.6f}, "-$30", Color{56, 189, 248, 255});
                            ShowToast("Excavated water canal waterway! ($30)", Color{56, 189, 248, 255}, 2.0f);
                        } else {
                            ShowToast("Insufficient funds to excavate canal ($30 required)", Color{239, 68, 68, 255}, 2.0f);
                        }
                    } else if (terrain[hoveredGx][hoveredGy] == GROUND_WATER) {
                        // Land reclamation: Fill water into solid land!
                        float reclaimCost = 25.0f;
                        if (economy.balance >= reclaimCost) {
                            economy.balance -= reclaimCost;
                            terrain[hoveredGx][hoveredGy] = currentGround;
                            AudioManager::Play(SFX_CONSTRUCTION, 0.9f);
                            particles.SpawnConfetti(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.5f}, 12);
                            particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.6f}, "-$25", Color{34, 197, 94, 255});
                            ShowToast("Reclaimed water cell into solid land! ($25)", Color{34, 197, 94, 255}, 2.5f);
                        } else {
                            ShowToast("Insufficient funds for land reclamation ($25 required)", Color{239, 68, 68, 255}, 2.0f);
                        }
                    } else {
                        // Regular concourse & biome paving
                        float cost = (currentGround == GROUND_PLAZA || currentGround == GROUND_STONE) ? 20.0f : 15.0f;
                        if (economy.balance >= cost) {
                            economy.balance -= cost;
                            terrain[hoveredGx][hoveredGy] = currentGround;
                            AudioManager::Play(SFX_CONSTRUCTION, 0.7f);
                            particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.6f}, TextFormat("-$%d", (int)cost), Color{239, 68, 68, 255});
                        } else {
                            ShowToast(TextFormat("Insufficient funds for paving ($%d required)", (int)cost), Color{239, 68, 68, 255}, 2.0f);
                        }
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
    // Screen shake decay
    if (shakeTimer > 0.0f) {
        shakeTimer -= dt;
        shakeIntensity = shakeTimer > 0.0f ? sinf(GetTime() * 80.0f) * 3.0f * (shakeTimer / 0.15f) : 0.0f;
    } else {
        shakeIntensity = 0.0f;
    }
    if (circuitFlashTimer > 0.0f) circuitFlashTimer -= dt;

    ui.Update(GetMousePosition(), IsMouseButtonPressed(MOUSE_BUTTON_LEFT));

    if (activeToast.timer > 0.0f) {
        activeToast.timer -= dt;
    }

    if (state != STATE_PLAYING) return;
    if (gameSpeed == 0 || isPaused) return;

    // Arcade mission briefing timer (real-time countdown)
    float simDt = dt * (float)gameSpeed;

    // Strict camera safety: guarantee camera center is always firmly within playable map
    ClampCamera();

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
    bool wasRush = rushHourActive;
    if (weekTimer >= 12.0f && weekTimer < 40.0f) {
        rushHourActive = (fmodf(weekTimer, 14.0f) < 7.0f);
    } else {
        rushHourActive = false;
    }
    if (rushHourActive && !wasRush) {
        rushHourFlashTimer = 0.6f; // 0.6s red screen-edge flash
    }
    if (rushHourActive) {
        rushHourTimer += simDt;
        if (rushHourFlashTimer > 0.0f) rushHourFlashTimer -= simDt;
        peeps.SetSpawnInterval(0.85f);
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
            stateEntryTime = GetTime();
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
            stateEntryTime = GetTime();
            AudioManager::Play(SFX_QUEUE_ALARM, 0.9f);
        }
    }

    // 7. Refresh Telemetry Statistics
    cachedStats = train.GetStats(tracks);
    float investedTracks = (float)tracks.GetTrackCount() * 40.0f;
    float investedStations = (float)tracks.GetStationCount() * 150.0f;
    float investedCars = (float)train.GetCarriageCount() * 600.0f;
    float investedTrains = (float)GetExtraTrainCount() * 1200.0f;
    cachedStats.parkValue = economy.balance + investedTracks + investedStations + investedCars + investedTrains + (float)economy.totalDelivered * 5.0f;
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(nightMode ? Color{15, 23, 42, 255} : Color{241, 245, 249, 255}); // Slate 900 night vs Slate 100 soft architectural canvas

    if (state == STATE_TITLE) {
        ui.DrawTitleScreen(bestSessionRiders);
        EndDrawing();
        return;
    }

    // 1. Draw Ground Tiles (with screen shake offset)
    Vector2 drawCam = {cameraPos.x + shakeIntensity, cameraPos.y + shakeIntensity * 0.6f};
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            bool isHovered = (x == hoveredGx && y == hoveredGy);
            Iso::DrawTile(x, y, groundZ[x][y], terrain[x][y], drawCam, zoom, isHovered);
        }
    }

    // 2. Draw Transit Portal Arch Marquee at entrance (0, 9)
    Iso::DrawTransitPortalArch({0.0f, 9.0f}, drawCam, zoom);

    // 3. Draw Scenery Items (Layered with correct isometric depth)
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (scenery[x][y] != SCENERY_NONE) {
                Iso::DrawScenery(x, y, groundZ[x][y], scenery[x][y], drawCam, zoom);
            }
        }
    }

    // 4. Draw Station Litter / Messes
    for (const auto& m : messes) {
        Iso::DrawMess(m, drawCam, zoom);
    }

    // 5. Draw Commuters with Shape Badges
    peeps.Draw(drawCam, zoom);

    // 6. Draw Transit Crew (Custodians & Signal Technicians)
    for (const auto& s : staff) {
        Iso::DrawStaff(s, drawCam, zoom);
    }

    // 7. Draw Track Ballast, Concrete Sleepers, 3rd Rail, Island Platforms, Signals
    tracks.DrawAllTracks(drawCam, zoom);

    // 8. Draw Metro EMU Rolling Stock & Commuter Passengers
    train.Draw(drawCam, zoom);
    for (auto& extra : extraTrains) extra.Draw(drawCam, zoom);

    // 9. Draw Particles (Sparks, Smoke, Confetti, Door Chime rings)
    particles.Draw(drawCam, zoom);

    // 10. Draw Ghost Placement Preview
    Vector2 mousePos = GetMousePosition();
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    bool overUI = (mousePos.y <= 56 || mousePos.y >= screenH - 150);
    if (statsWindowOpen && mousePos.x >= screenW - 360 && mousePos.y >= 58 && mousePos.y <= 540) overUI = true;
    if (staffWindowOpen && mousePos.x >= screenW - 360 && mousePos.y >= 58 && mousePos.y <= 420) overUI = true;
    if (selectedPeepIdx != -1 && ui.IsMouseInPeepInspector(mousePos)) overUI = true;
    if (selectedStationGx != -1 && ui.IsMouseInStationInspector(mousePos)) overUI = true;
    if (mousePos.x <= 270 && mousePos.y >= 58 && mousePos.y <= 120) overUI = true;
    if (!overUI && hoveredGx >= 0 && hoveredGx < GRID_SIZE && hoveredGy >= 0 && hoveredGy < GRID_SIZE) {
        if (isBulldozing) {
            Iso::DrawCursor(hoveredGx, hoveredGy, currentZ, drawCam, zoom, Color{239, 68, 68, 220});
        } else {
            if (activeTab == CAT_TRACK) {
                Direction inDir, outDir;
                GetTrackPieceDirs(currentTrack, buildHeading, inDir, outDir);
                bool canPlace = (hoveredGx >= 0 && hoveredGx < GRID_SIZE && hoveredGy >= 0 && hoveredGy < GRID_SIZE);
                tracks.DrawGhostPiece(hoveredGx, hoveredGy, currentZ, currentTrack, inDir, outDir, drawCam, zoom, canPlace);
            } else if (activeTab == CAT_INFRA) {
                if (isTerraformingRaise) {
                    Iso::DrawCursor(hoveredGx, hoveredGy, groundZ[hoveredGx][hoveredGy] + 1, drawCam, zoom, Color{250, 204, 21, 220});
                } else if (currentGround == GROUND_WATER) {
                    Iso::DrawCursor(hoveredGx, hoveredGy, 0, drawCam, zoom, Color{37, 99, 235, 220});
                } else if (terrain[hoveredGx][hoveredGy] == GROUND_WATER) {
                    Iso::DrawCursor(hoveredGx, hoveredGy, 0, drawCam, zoom, Color{34, 197, 94, 220});
                } else {
                    Iso::DrawCursor(hoveredGx, hoveredGy, groundZ[hoveredGx][hoveredGy], drawCam, zoom, Color{56, 189, 248, 200});
                }
            } else if (activeTab == CAT_SCENERY) {
                Iso::DrawCursor(hoveredGx, hoveredGy, 0, drawCam, zoom, Color{74, 222, 128, 200});
                Iso::DrawScenery(hoveredGx, hoveredGy, 0, currentScenery, drawCam, zoom);
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
                Vector2 p = Iso::GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, 0.0f, drawCam, zoom);
                float rw = TILE_WIDTH * zoom * 0.55f;
                float rh = TILE_HEIGHT * zoom * 0.55f;
                DrawEllipse((int)p.x, (int)p.y, rw, rh, Color{239, 68, 68, (unsigned char)(30 + 30 * pulse)});
                DrawEllipseLines((int)p.x, (int)p.y, rw, rh, ringC);
            };
            drawGapRing(ax, ay);
            drawGapRing(bx, by);
            // Dashed connector between the two endpoints
            Vector2 pa = Iso::GridToScreen((float)ax + 0.5f, (float)ay + 0.5f, 0.0f, drawCam, zoom);
            Vector2 pb = Iso::GridToScreen((float)bx + 0.5f, (float)by + 0.5f, 0.0f, drawCam, zoom);
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



    // 12. Atmospheric Day / Sunset / Night Rush Hour Lighting
    if (nightMode) {
        // Deep nocturnal vista
        DrawRectangle(0, 54, GetScreenWidth(), GetScreenHeight() - 54, Color{15, 23, 42, 90});
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                if (scenery[x][y] == SCENERY_LAMP_POST) {
                    Vector2 sPos = Iso::GridToScreen((float)x + 0.5f, (float)y + 0.5f, (float)groundZ[x][y] + 0.85f, drawCam, zoom);
                    DrawCircleGradient(sPos, 48.0f * zoom, Color{254, 240, 138, 140}, Color{254, 240, 138, 0});
                    DrawCircle((int)sPos.x, (int)sPos.y, 4.0f * zoom, Color{255, 255, 255, 240});
                }
            }
        }
    } else {
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
                        Vector2 sPos = Iso::GridToScreen((float)x + 0.5f, (float)y + 0.5f, (float)groundZ[x][y] + 0.85f, drawCam, zoom);
                        DrawCircleGradient(sPos, 40.0f * zoom, Color{255, 238, 88, 120}, Color{255, 238, 88, 0});
                    }
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

    // 12a. Persistent arcade OBJECTIVE chip - compact goal tracking
    if (state == STATE_PLAYING) {
        if (endlessMode) {
            int m = ((lastMilestoneAwarded / 250) + 1) * 250;
            float pct = (float)(economy.totalDelivered - (m - 250)) / 250.0f;
            ui.DrawObjectiveChip("ENDLESS METROPOLIS", TextFormat("Next subsidy grant at %d riders", m), pct);
        } else {
            int left = std::max(0, 500 - economy.totalDelivered);
            ui.DrawObjectiveChip("VICTORY GOAL", TextFormat("Deliver %d more riders (%d / 500)", left, economy.totalDelivered), (float)economy.totalDelivered / 500.0f);
        }
    }

    // 12c. Arcade RUSH HOUR banner (dramatic pulsing)
    if (rushHourActive) {
        float rp = 0.5f + 0.5f * sinf(GetTime() * 6.0f);
        int rw = 380;
        int rh = 40;
        int rx = ((int)GetScreenWidth() - rw) / 2;
        int ry = 106;
        // Drop shadow
        DrawRectangleRounded(Rectangle{(float)rx + 2, (float)ry + 3, (float)rw, (float)rh}, 0.4f, 4, Color{0, 0, 0, 60});
        // Outer glow
        DrawRectangleRounded(Rectangle{(float)rx - 3, (float)ry - 3, (float)rw + 6, (float)rh + 6}, 0.4f, 4, Color{239, 68, 68, (unsigned char)(40 + 40 * rp)});
        // Body
        DrawRectangleRounded(Rectangle{(float)rx, (float)ry, (float)rw, (float)rh}, 0.4f, 4, Color{185, 28, 28, (unsigned char)(220 + 35 * rp)});
        DrawRectangleRoundedLines(Rectangle{(float)rx, (float)ry, (float)rw, (float)rh}, 0.4f, 4, Color{254, 202, 202, (unsigned char)(200 + 55 * rp)});
        // Lightning bolt icon
        DrawTriangle({(float)(rx + 20), (float)(ry + 6)}, {(float)(rx + 14), (float)(ry + 20)}, {(float)(rx + 22), (float)(ry + 18)}, Color{255, 214, 0, 255});
        DrawTriangle({(float)(rx + 22), (float)(ry + 18)}, {(float)(rx + 16), (float)(ry + 34)}, {(float)(rx + 24), (float)(ry + 22)}, Color{255, 214, 0, 255});
        DrawGameBoldTextCentered("RUSH HOUR!  FARES x1.25", (float)GetScreenWidth() / 2.0f, (float)ry + 10, 18, Color{255, 255, 255, (unsigned char)(230 + 25 * rp)});
        // Remaining time bar
        float rushRemaining = 7.0f - fmodf(weekTimer, 14.0f);
        float rushPct = std::max(0.0f, rushRemaining / 7.0f);
        DrawRectangle(rx + 8, ry + rh - 5, rw - 16, 3, Color{127, 29, 29, 255});
        DrawRectangle(rx + 8, ry + rh - 5, (int)((rw - 16) * rushPct), 3, Color{255, 214, 0, 200});
        // Screen-edge red flash on activation
        if (rushHourFlashTimer > 0.0f) {
            unsigned char flashAlpha = (unsigned char)(80 * (rushHourFlashTimer / 0.6f));
            int sw = GetScreenWidth(), sh = GetScreenHeight();
            DrawRectangle(0, 0, sw, 8, Color{239, 68, 68, flashAlpha});
            DrawRectangle(0, sh - 8, sw, 8, Color{239, 68, 68, flashAlpha});
            DrawRectangle(0, 0, 8, sh, Color{239, 68, 68, flashAlpha});
            DrawRectangle(sw - 8, 0, 8, sh, Color{239, 68, 68, flashAlpha});
        }
    }

    // 12a+. Circuit closed green celebration flash
    if (circuitFlashTimer > 0.0f) {
        unsigned char cfAlpha = (unsigned char)(60 * (circuitFlashTimer / 0.8f));
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        DrawRectangle(0, 0, sw, 10, Color{34, 197, 94, cfAlpha});
        DrawRectangle(0, sh - 10, sw, 10, Color{34, 197, 94, cfAlpha});
        DrawRectangle(0, 0, 10, sh, Color{34, 197, 94, cfAlpha});
        DrawRectangle(sw - 10, 0, 10, sh, Color{34, 197, 94, cfAlpha});
    }

    // 12b. Draw Contextual Quick Tip Banner
    {
        std::string quickTip;
        if (isBulldozing) {
            quickTip = "DEMOLISH MODE: click a tile to remove it (X to exit)";
        } else if (train.CanBoard() || train.GetState() == TRAIN_BOARDING || train.GetState() == TRAIN_STOPPED_IN_STATION) {
            quickTip = "Train is at a station - commuters are boarding!";
        } else if (activeTab == CAT_TRACK) {
            if (tracks.IsCircuitClosed()) {
                quickTip = "Tracks: 1 Rail  4 Viaduct  7 Station  [L] Route Livery  [C] Recenter";
            } else {
                quickTip = "Connect the track into a closed loop ([B] to auto-bridge gap, [C] recenter)";
            }
        } else if (activeTab == CAT_INFRA) {
            quickTip = "Concourse: 1 Walkway  3 Plaza  4 Reclaim Water  5 Canal  6 Sand  7 Stone  8 Hill";
        } else if (activeTab == CAT_SCENERY) {
            quickTip = "Scenery: 1 Entrance  2 Gates  3 Tree  5 Bench  7 Fountain  8 Cafe";
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
        GetExtraTrainCount(),
        isTerraformingRaise
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

    // 16b. Draw Station Concourse Inspector
    if (selectedStationGx != -1 && selectedStationGy != -1) {
        const TrackNode* stNode = tracks.GetStationAt(selectedStationGx, selectedStationGy);
        if (stNode) {
            int waiting = peeps.GetCommutersWantingShape(stNode->stationShape);
            ui.DrawStationInspector(stNode, waiting, economy.balance);
        } else {
            selectedStationGx = -1;
            selectedStationGy = -1;
        }
    }

    // 17. Draw Toast Notifications
    ui.DrawToast(activeToast);

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
        ui.DrawGameOver(economy.totalDelivered, stars, bestSessionRiders, stateEntryTime);
    } else if (state == STATE_VICTORY) {
        int stars = 1 + ((economy.totalDelivered >= 500) ? 1 : 0) + ((week <= 4) ? 1 : 0);
        ui.DrawVictory(economy.totalDelivered, week, stars, economy.balance, bestSessionRiders, stateEntryTime);
    } else if (state == STATE_PLAYING && (gameSpeed == 0 || isPaused)) {
        ui.DrawPauseOverlay();
    }

    EndDrawing();
}

bool Game::ShouldClose() const {
    return WindowShouldClose();
}

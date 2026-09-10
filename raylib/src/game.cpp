#include "game.hpp"
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

    zoom = 1.0f;
    // Center camera on the metropolitan transit circuit and Central Hub
    cameraPos = {(float)screenW / 2.0f + 40.0f * zoom, ((float)screenH / 2.0f - 20.0f) - 420.0f * zoom};

    SetupInitialPark();
    tracks.InitDefaultCircuit();
    train.Reset(tracks);
    peeps.Init({2.0f, 12.0f}, {7.0f, 13.0f}, {6.0f, 14.0f});
    particles.Clear();

    economy.balance = 2500.0f;
    economy.baseFare = 2.50f;
    economy.totalDelivered = 0;
    cachedStats.ticketFare = 2.50f;

    parkRating = 90.0f;
    angryLeaves = 0;
    week = 1;
    weekTimer = 0.0f;
    gameSpeed = 1;
    state = STATE_PLAYING;

    statsWindowOpen = false;
    staffWindowOpen = false;
    rideCamActive = false;
    helpOverlayOpen = false;
    selectedPeepIdx = -1;

    // Staff & Cleanliness
    staff.clear();
    messes.clear();
    parkCleanliness = 100.0f;
    staffWageTimer = 0.0f;

    StaffMember custodian;
    custodian.name = "Kenji (Custodian)";
    custodian.type = STAFF_CUSTODIAN;
    custodian.pos = {3.0f, 12.0f};
    custodian.targetPos = {3.0f, 12.0f};
    staff.push_back(custodian);

    StaffMember engineer;
    engineer.name = "Sato (Signal Engineer)";
    engineer.type = STAFF_ENGINEER;
    engineer.pos = {7.0f, 13.0f};
    engineer.targetPos = {7.0f, 13.0f};
    staff.push_back(engineer);

    ShowToast("🚇 Welcome to METRO GRID! Line 1 Service Active | Press [H] for Manual", Color{56, 189, 248, 255}, 4.5f);
}

void Game::ShowToast(const std::string& text, Color color, float duration) {
    activeToast.text = text;
    activeToast.color = color;
    activeToast.timer = duration;
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
    for (int y = 9; y <= 18; ++y) {
        terrain[11][y] = GROUND_WATER;
        terrain[12][y] = GROUND_WATER;
    }

    // Pedestrian sidewalks connecting subway entrance to Central Hub
    for (int x = 0; x <= 5; ++x) {
        terrain[x][12] = GROUND_PATH;
    }

    // Granite Transit Plaza at Central Hub
    for (int x = 5; x <= 8; ++x) {
        for (int y = 11; y <= 13; ++y) {
            terrain[x][y] = GROUND_PLAZA;
        }
    }

    // Tactile safety platform edge queue zones
    terrain[7][13] = GROUND_QUEUE;
    terrain[8][13] = GROUND_QUEUE;

    // Concourse path connecting to transfer station
    terrain[8][14] = GROUND_PATH;
    terrain[7][14] = GROUND_PATH;
    terrain[6][14] = GROUND_PATH;
    terrain[6][13] = GROUND_PATH;

    // Concourse towards University Med Center
    terrain[8][15] = GROUND_PATH;
    terrain[9][15] = GROUND_PATH;

    // Urban Transit Station Amenities & Scenery
    scenery[2][12] = SCENERY_METRO_ENTRANCE; // Subway stairs with illuminated "M" roundel totem
    scenery[4][12] = SCENERY_TURNSTILE_GATE; // Contactless fare gates & TVM ticket machine
    scenery[3][11] = SCENERY_MAP_KIOSK;      // Harry Beck style schematic transit map board
    scenery[6][11] = SCENERY_NEWSSTAND;      // Platform coffee & newspaper kiosk
    scenery[1][13] = SCENERY_BIKE_RACK;      // Metro bike share docking rack

    // Platform Benches
    scenery[5][11] = SCENERY_BENCH;
    scenery[7][11] = SCENERY_BENCH;

    // High-Efficiency Municipal LED Streetlamps
    scenery[3][13] = SCENERY_LAMP_POST;
    scenery[8][11] = SCENERY_LAMP_POST;
    scenery[6][15] = SCENERY_LAMP_POST;

    // Manicured Urban Ginkgo / Street Trees with sidewalk iron grates
    scenery[2][10] = SCENERY_STREET_TREE;
    scenery[4][10] = SCENERY_STREET_TREE;
    scenery[1][15] = SCENERY_STREET_TREE;
    scenery[5][14] = SCENERY_STREET_TREE;

    // Metropolitan Park Pine Trees across the canal
    scenery[15][8]  = SCENERY_PINE_TREE;
    scenery[16][12] = SCENERY_PINE_TREE;
    scenery[17][15] = SCENERY_PINE_TREE;
    scenery[15][18] = SCENERY_PINE_TREE;
}

void Game::ResetPark() {
    Init();
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

    // 1. Camera Panning with WASD / Arrow Keys
    float panSpeed = 480.0f * GetFrameTime();
    bool manualPan = false;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { cameraPos.y += panSpeed; manualPan = true; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  { cameraPos.y -= panSpeed; manualPan = true; }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  { cameraPos.x += panSpeed; manualPan = true; }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { cameraPos.x -= panSpeed; manualPan = true; }
    if (manualPan) rideCamActive = false;

    // Mouse Right-Click Drag
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        dragStart = mousePos;
        isDragging = true;
        rideCamActive = false;
    }
    if (isDragging) {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            cameraPos.x += (mousePos.x - dragStart.x);
            cameraPos.y += (mousePos.y - dragStart.y);
            dragStart = mousePos;
        } else {
            isDragging = false;
        }
    }

    // Zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        zoom = std::max(0.65f, std::min(2.0f, zoom + wheel * 0.1f));
    }

    // Hotkeys
    if (IsKeyPressed(KEY_F)) {
        rideCamActive = !rideCamActive;
        ShowToast(rideCamActive ? "🎥 Driver's Cab Cam Active (Tracking Lead EMU)" : "Free OCC Camera Mode", Color{56, 189, 248, 255}, 2.0f);
        AudioManager::Play(SFX_BUTTON_CLICK, 0.7f);
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
        gameSpeed = (gameSpeed == 0) ? 1 : 0;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_M)) {
        AudioManager::SetMute(!AudioManager::IsMuted());
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

    // Rotation control
    if (IsKeyPressed(KEY_R)) {
        currentInDir = (Direction)((currentInDir + 1) % 4);
        currentOutDir = (Direction)((currentOutDir + 1) % 4);
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
        else currentScenery = SCENERY_MAP_KIOSK;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FOUR)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_VIADUCT_ELEVATED;
        else currentScenery = SCENERY_STREET_TREE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FIVE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_VIADUCT_SLOPE;
        else currentScenery = SCENERY_PINE_TREE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SIX)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_TUNNEL_PORTAL;
        else currentScenery = SCENERY_BENCH;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SEVEN)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_STATION;
        else currentScenery = SCENERY_LAMP_POST;
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
    if (state == STATE_GAME_OVER || state == STATE_VICTORY) {
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
            bool closeStats = false;
            if (ui.CheckStatsWindowClick(mousePos, deltaFare, colorChoice, closeStats)) {
                if (closeStats) statsWindowOpen = false;
                if (deltaFare != 0.0f) {
                    cachedStats.ticketFare = std::max(0.50f, std::min(10.0f, cachedStats.ticketFare + deltaFare));
                }
                if (colorChoice != -1) {
                    Color liveries[] = {
                        Color{229, 57, 53, 255},  // Tokyo Red (Marunouchi)
                        Color{37, 99, 235, 255},  // London Blue (Piccadilly)
                        Color{16, 185, 129, 255}, // Paris Green (Line 6)
                        Color{147, 51, 234, 255}, // MTR Purple (Tseung Kwan O)
                        Color{245, 158, 11, 255}  // Chicago Amber (Brown Line)
                    };
                    const char* names[] = {
                        "Line 1 - Marunouchi Red",
                        "Line 2 - Piccadilly Blue",
                        "Line 3 - Paris Emerald",
                        "Line 4 - Victoria Purple",
                        "Line 5 - Chicago Amber"
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
                    SCENERY_METRO_ENTRANCE, SCENERY_TURNSTILE_GATE, SCENERY_MAP_KIOSK,
                    SCENERY_STREET_TREE, SCENERY_PINE_TREE,
                    SCENERY_BENCH, SCENERY_LAMP_POST, SCENERY_NEWSSTAND, SCENERY_NONE
                };
                if (itemIdx == 8) isBulldozing = true;
                else { isBulldozing = false; currentScenery = sTypes[itemIdx]; }
            }
            AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
            return;
        }

        // Toolbar Aux Click (Height & Rotate)
        int zDelta = 0;
        bool doRotate = false;
        if (ui.CheckToolbarAuxClick(mousePos, zDelta, doRotate)) {
            if (zDelta != 0) currentZ = std::max(0, std::min(5, currentZ + zDelta));
            if (doRotate) {
                currentInDir = (Direction)((currentInDir + 1) % 4);
                currentOutDir = (Direction)((currentOutDir + 1) % 4);
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

    // 3. Commuter Inspection or Placement
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.y > 60 && mousePos.y < GetScreenHeight() - 90) {
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
                    AudioManager::Play(SFX_BULLDOZE, 0.7f);
                    particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 10);
                } else if (tracks.HasPiece(hoveredGx, hoveredGy)) {
                    tracks.RemovePiece(hoveredGx, hoveredGy);
                    economy.balance += 25.0f;
                    AudioManager::Play(SFX_BULLDOZE, 0.8f);
                    particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 12);
                } else if (terrain[hoveredGx][hoveredGy] != GROUND_GRASS && terrain[hoveredGx][hoveredGy] != GROUND_WATER) {
                    terrain[hoveredGx][hoveredGy] = GROUND_GRASS;
                    economy.balance += 5.0f;
                    AudioManager::Play(SFX_BULLDOZE, 0.6f);
                }
            } else {
                // Active placement based on Tab
                if (activeTab == CAT_TRACK) {
                    if (economy.balance >= 40.0f) {
                        bool wasClosed = tracks.IsCircuitClosed();
                        if (tracks.AddPiece(hoveredGx, hoveredGy, currentZ, currentTrack, currentInDir, currentOutDir)) {
                            economy.balance -= 40.0f;
                            AudioManager::Play(SFX_CONSTRUCTION, 0.8f);
                            particles.SpawnSparks(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 8);

                            if (!wasClosed && tracks.IsCircuitClosed()) {
                                ShowToast("🚇 Transit Loop Closed! Regular EMU Schedule Active!", Color{34, 197, 94, 255}, 4.0f);
                                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
                            }

                            // Auto-advance cursor forward along track direction
                            Vector2 fwd = GetDirectionOffset(currentOutDir);
                            hoveredGx += (int)fwd.x;
                            hoveredGy += (int)fwd.y;
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

                    if (economy.balance >= (float)scnCost) {
                        scenery[hoveredGx][hoveredGy] = currentScenery;
                        economy.balance -= (float)scnCost;
                        parkRating = std::min(100.0f, parkRating + boost);
                        AudioManager::Play(SFX_CONSTRUCTION, 0.8f);
                        particles.SpawnConfetti(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.5f}, 10);
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
    if (gameSpeed == 0) return;

    float simDt = dt * (float)gameSpeed;

    // 1. Cab Cam Smooth Tracking (Driver's point of view)
    if (rideCamActive) {
        Vector3 locoPos = train.GetLocomotivePos();
        Vector2 targetScreen = Iso::GridToScreen(locoPos.x, locoPos.y, locoPos.z, {0, 0}, zoom);
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        Vector2 desiredCam = { (float)screenW / 2.0f - targetScreen.x, (float)screenH / 2.0f - targetScreen.y };

        cameraPos = Vector2Lerp(cameraPos, desiredCam, simDt * 5.0f);
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

    // 3. Update Particles
    particles.Update(simDt);

    // 4. Update Metro Train Operations & Physics
    int delivered = 0;
    float fareRevenue = 0.0f;
    train.Update(simDt, tracks, particles, delivered, fareRevenue);

    if (delivered > 0) {
        economy.totalDelivered += delivered;
        economy.balance += fareRevenue;
        parkRating = std::min(100.0f, parkRating + (float)delivered * 0.6f);

        if (economy.totalDelivered >= 500) {
            state = STATE_VICTORY;
            AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
        }
    }

    // Dynamic Wayside Signaling Update
    tracks.UpdateSignals(train.GetTrainDistance());

    // 5. Update Commuter Flow & Overcrowding Triage
    int angry = 0;
    peeps.Update(simDt, train, particles, parkRating, angry, messes);
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

    if (angry > 0) {
        angryLeaves += angry;
        ShowToast("⚠️ Platform Overcrowding! Commuters left in frustration!", Color{239, 68, 68, 255}, 3.0f);
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
        ui.DrawTitleScreen();
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

    // 2. Draw Transit Portal Arch Marquee at entrance (0, 12)
    Iso::DrawTransitPortalArch({0.0f, 12.0f}, cameraPos, zoom);

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

    // 9. Draw Particles (Sparks, Smoke, Confetti, Door Chime rings)
    particles.Draw(cameraPos, zoom);

    // 10. Draw Ghost Placement Preview
    Vector2 mousePos = GetMousePosition();
    bool overUI = (mousePos.y <= 56 || mousePos.y >= GetScreenHeight() - 86);
    if (!overUI && hoveredGx >= 0 && hoveredGx < GRID_SIZE && hoveredGy >= 0 && hoveredGy < GRID_SIZE) {
        if (isBulldozing) {
            Iso::DrawCursor(hoveredGx, hoveredGy, currentZ, cameraPos, zoom, Color{239, 68, 68, 220});
        } else {
            if (activeTab == CAT_TRACK) {
                tracks.DrawGhostPiece(hoveredGx, hoveredGy, currentZ, currentTrack, currentInDir, currentOutDir, cameraPos, zoom, true);
            } else if (activeTab == CAT_INFRA) {
                Iso::DrawCursor(hoveredGx, hoveredGy, 0, cameraPos, zoom, Color{56, 189, 248, 200});
            } else if (activeTab == CAT_SCENERY) {
                Iso::DrawCursor(hoveredGx, hoveredGy, 0, cameraPos, zoom, Color{74, 222, 128, 200});
                Iso::DrawScenery(hoveredGx, hoveredGy, 0, currentScenery, cameraPos, zoom);
            }
        }
    }

    // 11. Atmospheric Day / Sunset / Night Rush Hour Lighting
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
        helpOverlayOpen
    );

    // 13. Draw Categorized Toolbar
    ui.DrawToolbar(
        activeTab,
        currentTrack,
        currentScenery,
        currentGround,
        currentZ,
        currentOutDir,
        isBulldozing
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

    // 17. Draw Toast Notifications
    ui.DrawToast(activeToast);

    // 18. Draw Operations Manual
    if (helpOverlayOpen) {
        ui.DrawTransitOperationsManual();
    }

    // 19. Draw Modals (Weekly Grant, Game Over, Victory)
    if (state == STATE_WEEKLY_UPGRADE) {
        int ch = ui.CheckUpgradeModalClick(GetMousePosition());
        ui.DrawWeeklyModal(activeUpgrades, ch);
    } else if (state == STATE_GAME_OVER) {
        ui.DrawGameOver(economy.totalDelivered);
    } else if (state == STATE_VICTORY) {
        ui.DrawVictory(economy.totalDelivered);
    }

    EndDrawing();
}

bool Game::ShouldClose() const {
    return WindowShouldClose();
}

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
    // Center camera on the coaster circuit and station
    cameraPos = {(float)screenW / 2.0f + 96.0f * zoom, ((float)screenH / 2.0f - 20.0f) - 400.0f * zoom};

    SetupInitialPark();
    tracks.InitDefaultCircuit();
    train.Reset(tracks);
    peeps.Init({2.0f, 12.0f}, {7.0f, 13.0f}, {8.0f, 14.0f});
    particles.Clear();

    economy.balance = 1500.0f;
    economy.admissionPrice = 5.0f;
    economy.totalDelivered = 0;
    cachedStats.ticketPrice = 5.0f;

    parkRating = 85.0f;
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

    StaffMember handyman;
    handyman.name = "Bob (Handyman)";
    handyman.type = STAFF_HANDYMAN;
    handyman.pos = {3.0f, 12.0f};
    handyman.targetPos = {3.0f, 12.0f};
    staff.push_back(handyman);

    StaffMember mechanic;
    mechanic.name = "Hank (Mechanic)";
    mechanic.type = STAFF_MECHANIC;
    mechanic.pos = {8.0f, 13.0f};
    mechanic.targetPos = {8.0f, 13.0f};
    staff.push_back(mechanic);

    ShowToast("Welcome to COASTER GRID! Press [H] for Help | [P] Staff", Color{56, 189, 248, 255}, 4.5f);
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

    // East river canal
    for (int y = 0; y < GRID_SIZE; ++y) {
        terrain[20][y] = GROUND_WATER;
        terrain[21][y] = GROUND_WATER;
    }

    // Main entrance path to Station at (8, 12)
    for (int x = 0; x <= 8; ++x) {
        terrain[x][12] = GROUND_PATH;
    }
    terrain[7][13] = GROUND_QUEUE;
    terrain[8][13] = GROUND_QUEUE;
    terrain[8][14] = GROUND_PATH;
    terrain[7][14] = GROUND_PATH;
    terrain[6][14] = GROUND_PATH;
    terrain[6][13] = GROUND_PATH;

    // Scenic Landscaping
    scenery[3][5] = SCENERY_PINE_TREE;
    scenery[6][4] = SCENERY_PINE_TREE;
    scenery[18][8] = SCENERY_PINE_TREE;
    scenery[18][14] = SCENERY_PINE_TREE;
    scenery[19][18] = SCENERY_PINE_TREE;
    scenery[4][18] = SCENERY_PINE_TREE;
    scenery[11][21] = SCENERY_PINE_TREE;
    scenery[16][20] = SCENERY_PINE_TREE;

    scenery[2][10] = SCENERY_OAK_TREE;
    scenery[4][14] = SCENERY_OAK_TREE;
    scenery[1][14] = SCENERY_OAK_TREE;
    scenery[11][18] = SCENERY_OAK_TREE;

    // Park Amenities & Landscaping
    scenery[5][11] = SCENERY_BENCH;
    scenery[7][11] = SCENERY_BENCH;
    scenery[4][10] = SCENERY_FOUNTAIN;
    scenery[3][11] = SCENERY_FLOWER_BED;
    scenery[4][11] = SCENERY_FLOWER_BED;
    scenery[6][15] = SCENERY_DRINK_STALL;
    scenery[1][11] = SCENERY_BALLOON_STALL;
    scenery[3][13] = SCENERY_LAMP_POST;
    scenery[8][15] = SCENERY_LAMP_POST;
}

void Game::ResetPark() {
    Init();
}

void Game::GenerateWeeklyUpgrades() {
    activeUpgrades.clear();
    UpgradeChoice c1;
    c1.title = "Add Extra Carriage";
    c1.description = "Increases train capacity by 2 seats to clear queues faster.";
    c1.perkTag = "+2 SEATS / DISPATCH";
    c1.accentColor = Color{59, 130, 246, 255}; // Blue
    activeUpgrades.push_back(c1);

    UpgradeChoice c2;
    c2.title = "Magnetic Boosters";
    c2.description = "Boosts lift hill chain speed by 30% for faster circuit turnaround.";
    c2.perkTag = "FAST DISPATCH";
    c2.accentColor = Color{234, 88, 12, 255}; // Orange
    activeUpgrades.push_back(c2);

    UpgradeChoice c3;
    c3.title = "Express Queue Pass";
    c3.description = "Doubles queue tolerance time before overcrowding clocks trigger.";
    c3.perkTag = "ANTI-OVERCROWD";
    c3.accentColor = Color{16, 185, 129, 255}; // Green
    activeUpgrades.push_back(c3);
}

void Game::ApplyUpgrade(int choiceIdx) {
    if (choiceIdx == 0) {
        train.SetCarriageCount(train.GetMaxCapacity() / 2 + 1);
        ShowToast("Upgrade Applied: Extra Carriage added!", Color{59, 130, 246, 255});
    } else if (choiceIdx == 1) {
        peeps.SetSpawnInterval(1.8f);
        ShowToast("Upgrade Applied: Fast Dispatch Boost activated!", Color{234, 88, 12, 255});
    } else if (choiceIdx == 2) {
        parkRating = std::min(100.0f, parkRating + 15.0f);
        ShowToast("Upgrade Applied: Express Pass & +15% Rating!", Color{16, 185, 129, 255});
    }
    state = STATE_PLAYING;
    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
}

void Game::HandleInput() {
    Vector2 mousePos = GetMousePosition();

    // 1. Camera Panning with WASD / Keys
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
        ShowToast(rideCamActive ? "🎥 Ride Cam Active (Following Coaster)" : "Free Camera Mode", Color{56, 189, 248, 255}, 2.0f);
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
        train.RecoverFromCrash(tracks);
        ShowToast("Coaster Train Recovered to Station", Color{255, 214, 0, 255}, 2.5f);
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

    // Quick Tool Selection Keys
    if (IsKeyPressed(KEY_X)) {
        isBulldozing = !isBulldozing;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_ONE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_STRAIGHT;
        else if (activeTab == CAT_INFRA) currentGround = GROUND_PATH;
        else currentScenery = SCENERY_PINE_TREE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_TWO)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_CURVE_LEFT;
        else if (activeTab == CAT_INFRA) currentGround = GROUND_QUEUE;
        else currentScenery = SCENERY_OAK_TREE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_THREE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_CURVE_RIGHT;
        else currentScenery = SCENERY_BENCH;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FOUR)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_LIFT_HILL;
        else currentScenery = SCENERY_FOUNTAIN;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FIVE)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_DROP;
        else currentScenery = SCENERY_FLOWER_BED;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SIX)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_LOOP;
        else currentScenery = SCENERY_DRINK_STALL;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_SEVEN)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_BRAKES;
        else currentScenery = SCENERY_BALLOON_STALL;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_EIGHT)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_STATION;
        else currentScenery = SCENERY_LAMP_POST;
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

        // Stats Window Click
        if (statsWindowOpen) {
            float deltaPrice = 0.0f;
            int colorChoice = -1;
            bool closeStats = false;
            if (ui.CheckStatsWindowClick(mousePos, deltaPrice, colorChoice, closeStats)) {
                if (closeStats) statsWindowOpen = false;
                if (deltaPrice != 0.0f) {
                    cachedStats.ticketPrice = std::max(1.0f, std::min(20.0f, cachedStats.ticketPrice + deltaPrice));
                }
                if (colorChoice != -1) {
                    Color themes[] = {
                        Color{229, 57, 53, 255}, // Red
                        Color{37, 99, 235, 255}, // Blue
                        Color{16, 185, 129, 255}, // Green
                        Color{147, 51, 234, 255}, // Purple
                        Color{245, 158, 11, 255}  // Amber
                    };
                    const char* names[] = {
                        "The Red Falcon", "The Blue Comet", "The Emerald Viper", "The Mystic Phantom", "The Golden Dragon"
                    };
                    cachedStats.themeColor = themes[colorChoice];
                    cachedStats.coasterName = names[colorChoice];
                    tracks.SetTrackColor(themes[colorChoice]);
                    train.SetTrainTheme(themes[colorChoice]);
                    train.SetCoasterName(names[colorChoice]);
                    ShowToast(TextFormat("Coaster repainted: %s!", names[colorChoice]), themes[colorChoice], 3.0f);
                    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.7f);
                }
                AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
                return;
            }
        }

        // Staff Window Click
        if (staffWindowOpen) {
            bool hireHandyman = false;
            bool hireMechanic = false;
            bool closeStaff = false;
            if (ui.CheckStaffWindowClick(mousePos, hireHandyman, hireMechanic, closeStaff)) {
                if (closeStaff) staffWindowOpen = false;
                if (hireHandyman) {
                    if (economy.balance >= 80.0f) {
                        economy.balance -= 80.0f;
                        StaffMember h;
                        h.name = TextFormat("Handyman #%d", (int)staff.size() + 1);
                        h.type = STAFF_HANDYMAN;
                        h.pos = {2.0f, 12.0f};
                        h.targetPos = {2.0f, 12.0f};
                        staff.push_back(h);
                        ShowToast("Hired Handyman! Park paths will be kept sparkling.", Color{59, 130, 246, 255}, 3.0f);
                        AudioManager::Play(SFX_CASH_REGISTER, 0.7f);
                    } else {
                        ShowToast("Insufficient funds to hire Handyman ($80 required)", Color{239, 68, 68, 255}, 2.5f);
                    }
                }
                if (hireMechanic) {
                    if (economy.balance >= 100.0f) {
                        economy.balance -= 100.0f;
                        StaffMember m;
                        m.name = TextFormat("Mechanic #%d", (int)staff.size() + 1);
                        m.type = STAFF_MECHANIC;
                        m.pos = {8.0f, 13.0f};
                        m.targetPos = {8.0f, 13.0f};
                        staff.push_back(m);
                        ShowToast("Hired Mechanic! Coaster safety and speed ensured.", Color{245, 158, 11, 255}, 3.0f);
                        AudioManager::Play(SFX_CASH_REGISTER, 0.7f);
                    } else {
                        ShowToast("Insufficient funds to hire Mechanic ($100 required)", Color{239, 68, 68, 255}, 2.5f);
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
                    TRACK_LIFT_HILL, TRACK_DROP, TRACK_LOOP, TRACK_BRAKES,
                    TRACK_STATION, TRACK_NONE
                };
                if (itemIdx == 8) isBulldozing = true;
                else { isBulldozing = false; currentTrack = tTypes[itemIdx]; }
            } else if (activeTab == CAT_INFRA) {
                if (itemIdx == 0) { isBulldozing = false; currentGround = GROUND_PATH; }
                else if (itemIdx == 1) { isBulldozing = false; currentGround = GROUND_QUEUE; }
                else isBulldozing = true;
            } else if (activeTab == CAT_SCENERY) {
                SceneryType sTypes[] = {
                    SCENERY_PINE_TREE, SCENERY_OAK_TREE, SCENERY_BENCH,
                    SCENERY_FOUNTAIN, SCENERY_FLOWER_BED,
                    SCENERY_DRINK_STALL, SCENERY_BALLOON_STALL, SCENERY_LAMP_POST, SCENERY_NONE
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

    // 3. Peep Inspection or Placement
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.y > 60 && mousePos.y < GetScreenHeight() - 90) {
        // First check if clicking on a peep
        int peepUnderMouse = peeps.FindPeepAtScreenPos(mousePos, cameraPos, zoom);
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
                    economy.balance += 20.0f;
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
                    if (economy.balance >= 35.0f) {
                        bool wasClosed = tracks.IsCircuitClosed();
                        if (tracks.AddPiece(hoveredGx, hoveredGy, currentZ, currentTrack, currentInDir, currentOutDir)) {
                            economy.balance -= 35.0f;
                            AudioManager::Play(SFX_CONSTRUCTION, 0.8f);
                            particles.SpawnSparks(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 8);

                            if (!wasClosed && tracks.IsCircuitClosed()) {
                                ShowToast("🎢 Circuit Completed! Coaster is now Operational!", Color{34, 197, 94, 255}, 4.0f);
                                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
                            }

                            // Auto-advance cursor
                            Vector2 fwd = GetDirectionOffset(currentOutDir);
                            hoveredGx += (int)fwd.x;
                            hoveredGy += (int)fwd.y;
                            if (currentTrack == TRACK_LIFT_HILL) currentZ++;
                            if (currentTrack == TRACK_DROP) currentZ = std::max(0, currentZ - 1);
                        }
                    } else {
                        ShowToast("Insufficient funds to build track ($35 required)", Color{239, 68, 68, 255}, 2.0f);
                    }
                } else if (activeTab == CAT_INFRA) {
                    if (economy.balance >= 10.0f) {
                        terrain[hoveredGx][hoveredGy] = currentGround;
                        economy.balance -= 10.0f;
                        AudioManager::Play(SFX_CONSTRUCTION, 0.7f);
                    } else {
                        ShowToast("Insufficient funds for path ($10 required)", Color{239, 68, 68, 255}, 2.0f);
                    }
                } else if (activeTab == CAT_SCENERY) {
                    int scnCost = 30;
                    float boost = 1.5f;
                    if (currentScenery == SCENERY_FOUNTAIN) { scnCost = 180; boost = 5.0f; }
                    else if (currentScenery == SCENERY_DRINK_STALL) { scnCost = 150; boost = 3.0f; }
                    else if (currentScenery == SCENERY_BALLOON_STALL) { scnCost = 120; boost = 3.0f; }
                    else if (currentScenery == SCENERY_OAK_TREE) { scnCost = 45; boost = 1.5f; }
                    else if (currentScenery == SCENERY_PINE_TREE) { scnCost = 30; boost = 1.0f; }
                    else if (currentScenery == SCENERY_LAMP_POST) { scnCost = 25; boost = 1.0f; }
                    else if (currentScenery == SCENERY_BENCH) { scnCost = 20; boost = 0.8f; }
                    else if (currentScenery == SCENERY_FLOWER_BED) { scnCost = 15; boost = 1.0f; }

                    if (economy.balance >= (float)scnCost) {
                        scenery[hoveredGx][hoveredGy] = currentScenery;
                        economy.balance -= (float)scnCost;
                        parkRating = std::min(100.0f, parkRating + boost); // Scenery boosts park satisfaction!
                        AudioManager::Play(SFX_CONSTRUCTION, 0.8f);
                        particles.SpawnConfetti(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, 0.5f}, 12);
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

    // 1. Ride Cam Smooth Tracking
    if (rideCamActive) {
        Vector3 locoPos = train.GetLocomotivePos();
        Vector2 targetScreen = Iso::GridToScreen(locoPos.x, locoPos.y, locoPos.z, {0, 0}, zoom);
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        Vector2 desiredCam = { (float)screenW / 2.0f - targetScreen.x, (float)screenH / 2.0f - targetScreen.y };

        cameraPos = Vector2Lerp(cameraPos, desiredCam, simDt * 5.0f);

        // Dynamic camera shake on high-speed plunges
        float spd = train.GetSpeedKmh();
        if (spd > 35.0f) {
            float rumble = (spd - 35.0f) * 0.05f;
            cameraPos.x += (((float)rand() / RAND_MAX) - 0.5f) * rumble;
            cameraPos.y += (((float)rand() / RAND_MAX) - 0.5f) * rumble;
        }
    }

    // 2. Weekly Calendar Progression
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

    // 4. Update Coaster Train Physics
    int delivered = 0;
    bool wasDerailed = train.IsDerailed();
    train.Update(simDt, tracks, particles, delivered);

    if (!wasDerailed && train.IsDerailed()) {
        ShowToast("💥 DERAILMENT! Sharp turn taken too fast! Press 'C' to reset.", Color{239, 68, 68, 255}, 4.5f);
    }

    if (delivered > 0) {
        economy.totalDelivered += delivered;
        float ticketRev = (float)delivered * cachedStats.ticketPrice;
        economy.balance += ticketRev;
        parkRating = std::min(100.0f, parkRating + (float)delivered * 0.8f);
        AudioManager::Play(SFX_CASH_REGISTER, 0.7f);

        if (rand() % 2 == 0) {
            AudioManager::Play(SFX_PEEP_CHEER, 0.6f);
        }

        if (economy.totalDelivered >= 500) {
            state = STATE_VICTORY;
            AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
        }
    }

    // 5. Update Peeps & Scenery Interactions
    int angry = 0;
    peeps.Update(simDt, train, particles, parkRating, angry, messes);
    peeps.CheckSceneryInteractions(scenery, particles, economy.balance, messes);

    // Park Cleanliness calculation
    parkCleanliness = std::max(0.0f, 100.0f - (float)messes.size() * 3.5f);
    if (parkCleanliness < 50.0f) {
        parkRating = std::max(0.0f, parkRating - simDt * 0.4f);
    }

    // 6. Update Staff (Handymen sweep vomit & litter; Mechanics inspect station)
    for (auto& s : staff) {
        s.walkTimer += simDt;
        if (s.type == STAFF_HANDYMAN) {
            if (s.isWorking) {
                s.workTimer += simDt;
                if (fmodf(s.workTimer, 0.4f) < simDt) {
                    particles.SpawnSmoke(Vector3{s.pos.x, s.pos.y, 0.05f}, 2);
                }
                if (s.workTimer >= 1.6f) {
                    s.isWorking = false;
                    s.workTimer = 0.0f;
                    // Remove closest mess within range
                    for (auto it = messes.begin(); it != messes.end();) {
                        if (Vector2Distance(s.pos, it->pos) < 0.7f) {
                            particles.SpawnSparks(Vector3{it->pos.x, it->pos.y, 0.1f}, 6);
                            it = messes.erase(it);
                            parkRating = std::min(100.0f, parkRating + 1.2f);
                            break;
                        } else {
                            ++it;
                        }
                    }
                }
            } else if (!messes.empty()) {
                // Find closest mess
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
                // Patrol main path
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
        } else if (s.type == STAFF_MECHANIC) {
            // Mechanic patrols station area & inspects ride mechanics
            Vector2 dir = Vector2Subtract(s.targetPos, s.pos);
            if (Vector2Length(dir) < 0.25f) {
                if (s.isWorking) {
                    s.workTimer += simDt;
                    if (s.workTimer >= 3.0f) {
                        s.isWorking = false;
                        s.workTimer = 0.0f;
                        s.targetPos = {7.0f + (float)(rand() % 3), 12.0f + (float)(rand() % 3)};
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

    // Weekly staff wages
    staffWageTimer += simDt;
    if (staffWageTimer >= WEEK_DURATION) {
        staffWageTimer = 0.0f;
        float wages = 0.0f;
        for (const auto& s : staff) wages += (s.type == STAFF_HANDYMAN ? 10.0f : 15.0f);
        if (wages > 0.0f) {
            economy.balance = std::max(0.0f, economy.balance - wages);
            ShowToast(TextFormat("Weekly staff payroll paid: $%.0f", wages), Color{148, 163, 184, 255}, 2.5f);
        }
    }

    if (angry > 0) {
        angryLeaves += angry;
        ShowToast("⚠️ Queue riot! Angry guests walked out!", Color{239, 68, 68, 255}, 3.0f);
        if (parkRating <= 0.0f) {
            state = STATE_GAME_OVER;
            AudioManager::Play(SFX_CRASH, 0.9f);
        }
    }

    // 7. Refresh Coaster Statistics
    cachedStats = train.GetStats(tracks);
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(Color{241, 245, 249, 255}); // Slate 100 soft canvas

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

    // 2. Draw Park Entrance Grand Arch
    Iso::DrawEntranceArch({0.0f, 12.0f}, cameraPos, zoom);

    // 3. Draw Scenery Items (Layered with correct depth)
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (scenery[x][y] != SCENERY_NONE) {
                Iso::DrawScenery(x, y, groundZ[x][y], scenery[x][y], cameraPos, zoom);
            }
        }
    }

    // 4. Draw Park Messes (Vomit puddles & discarded soda cups)
    for (const auto& m : messes) {
        Iso::DrawMess(m, cameraPos, zoom);
    }

    // 5. Draw Peeps on Walkways
    peeps.Draw(cameraPos, zoom);

    // 6. Draw Park Staff (Handymen with brooms, Mechanics with wrenches)
    for (const auto& s : staff) {
        Iso::DrawStaff(s, cameraPos, zoom);
    }

    // 7. Draw Track Pillars and Rails
    tracks.DrawAllTracks(cameraPos, zoom);

    // 8. Draw Coaster Train & Passengers
    train.Draw(cameraPos, zoom);

    // 9. Draw Particles (Sparks, Smoke, Confetti, Vomit)
    particles.Draw(cameraPos, zoom);

    // 10. Draw Building Ghost Preview
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

    // 11. Atmospheric Day / Sunset / Night Lighting (Mini Metro Weekly Rhythm)
    float weekPhase = fmodf(weekTimer, WEEK_DURATION);
    if (weekPhase >= 36.0f && weekPhase < 48.0f) {
        // Sunset golden hour
        float alpha = (weekPhase - 36.0f) / 12.0f;
        DrawRectangle(0, 52, GetScreenWidth(), GetScreenHeight() - 52, Color{255, 140, 0, (unsigned char)(alpha * 38.0f)});
    } else if (weekPhase >= 48.0f) {
        // Twilight night rush hour
        float alpha = (weekPhase - 48.0f) / 12.0f;
        DrawRectangle(0, 52, GetScreenWidth(), GetScreenHeight() - 52, Color{15, 23, 42, (unsigned char)(50.0f + alpha * 65.0f)});

        // Lamppost glowing light pools on walkways
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                if (scenery[x][y] == SCENERY_LAMP_POST) {
                    Vector2 sPos = Iso::GridToScreen((float)x, (float)y, 0.0f, cameraPos, zoom);
                    DrawCircleGradient(sPos, 38.0f * zoom, Color{255, 238, 88, 120}, Color{255, 238, 88, 0});
                }
            }
        }
    }

    // 12. Draw Main HUD and Categorized Toolbar
    ui.DrawHUD(
        economy.totalDelivered,
        parkRating,
        week,
        weekTimer,
        train.GetSpeedKmh(),
        tracks.IsCircuitClosed(),
        gameSpeed,
        AudioManager::IsMuted(),
        economy.balance,
        statsWindowOpen,
        staffWindowOpen,
        rideCamActive,
        helpOverlayOpen
    );

    ui.DrawToolbar(
        activeTab,
        currentTrack,
        currentScenery,
        currentGround,
        currentZ,
        currentOutDir,
        isBulldozing
    );

    // 12. Draw Coaster Information Window
    if (statsWindowOpen) {
        ui.DrawCoasterStats(cachedStats);
    }

    // 13. Draw Staff Management Window
    if (staffWindowOpen) {
        ui.DrawStaffWindow(staff, parkCleanliness, economy.balance);
    }

    // 14. Draw Peep Inspector Card
    if (selectedPeepIdx != -1) {
        const Peep* p = peeps.GetPeep(selectedPeepIdx);
        if (p) {
            ui.DrawPeepInspector(p);
        } else {
            selectedPeepIdx = -1;
        }
    }

    // 15. Draw Toast Notifications
    ui.DrawToast(activeToast);

    // 16. Draw Help & Controls Overlay
    if (helpOverlayOpen) {
        ui.DrawHelpOverlay();
    }

    // 17. Draw Modals (Upgrades, Game Over, Victory)
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

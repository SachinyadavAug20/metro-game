#include "game.hpp"
#include "font_system.hpp"
#include <algorithm>

Game::Game() {
    Init();
}

Game::~Game() {
    AudioManager::Cleanup();
}

// Natural events definitions
const Game::NaturalEvent Game::EVENTS[] = {
    {"Rush Hour Surge",  "Massive commuter spike! x1.5 riders, x1.5 fares", 20.0f, 1.5f, 1.5f, {255, 140, 0, 255}},
    {"Calm Sunday",      "Peaceful day. Half the riders, but +x2 fare bonus", 25.0f, 0.5f, 2.0f, {100, 200, 255, 255}},
    {"City Festival",    "Festival downtown! x2 riders rush to the party!", 18.0f, 2.0f, 1.2f, {255, 50, 150, 255}},
    {"Train Strike",     "Workers protest! Half riders, half fares for 15s", 15.0f, 0.5f, 0.5f, {200, 50, 50, 255}},
    {"Tourist Season",     "Tourists flood in! x1.8 riders, x1.3 fares", 22.0f, 1.8f, 1.3f, {50, 200, 100, 255}},
    {"Late Night Quiet",   "After midnight calm. Few riders, but x3 fare bonus", 20.0f, 0.3f, 3.0f, {80, 80, 160, 255}},
    {"Tropical Rainstorm", "Sudden downpour! Commuters pop umbrellas, +25% transit ridership", 22.0f, 1.7f, 1.3f, {56, 189, 248, 255}},
};

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

    // Smart Advisor init
    advisorShowTime = 0.0f;
    advisorDismissCount = 0;
    lastAdvisorTier = -1;

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
    scenery[1][9] = SCENERY_VENT_GRATE;     // Subway sidewalk ventilation grate with rising steam
    scenery[2][9] = SCENERY_TURNSTILE_GATE; // Contactless fare gates & TVM ticket machine
    scenery[4][9] = SCENERY_VENT_GRATE;     // Secondary sidewalk steam vent
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
    scenery[2][8] = SCENERY_LAMP_POST;
    scenery[8][10] = SCENERY_LAMP_POST;
    scenery[5][12] = SCENERY_LAMP_POST;
    scenery[8][14] = SCENERY_LAMP_POST;
    scenery[10][14] = SCENERY_LAMP_POST;

    // Manicured Trees, Coastal Palms & Botanical Flower Beds
    scenery[1][8] = SCENERY_STREET_TREE;
    scenery[3][10] = SCENERY_STREET_TREE;
    scenery[5][10] = SCENERY_STREET_TREE;
    scenery[11][7]  = SCENERY_PALM_TREE;    // Coastal canal tropical palm
    scenery[11][15] = SCENERY_PALM_TREE;    // Coastal canal tropical palm
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

    // Expanded upgrade pool: 8 possible choices, pick 3 random unique ones
    std::vector<UpgradeChoice> pool;

    UpgradeChoice c1;
    c1.title = "4-Car EMU Trainset";
    c1.description = "Extends rolling stock formation to 4 cars, adding +4 commuter capacity.";
    c1.perkTag = "+4 SEATS / TRAIN";
    c1.accentColor = Color{56, 189, 248, 255};
    c1.id = UPGRADE_4CAR_EMU;
    pool.push_back(c1);

    UpgradeChoice c2;
    c2.title = "CBTC Signaling & Boost";
    c2.description = "Upgrades track signaling to Communications-Based Train Control, raising cruising speed to 75 km/h.";
    c2.perkTag = "HIGH-SPEED CBTC";
    c2.accentColor = Color{220, 38, 38, 255};
    c2.id = UPGRADE_CBTC_SIGNALING;
    pool.push_back(c2);

    UpgradeChoice c3;
    c3.title = "Transit Subsidy & Pass";
    c3.description = "Receives $1,200 municipal transit subsidy and boosts commuter satisfaction by +15%.";
    c3.perkTag = "+$1,200 CASH & 15% SAT";
    c3.accentColor = Color{16, 185, 129, 255};
    c3.id = UPGRADE_TRANSIT_SUBSIDY;
    pool.push_back(c3);

    UpgradeChoice c4;
    c4.title = "Express Line Service";
    c4.description = "Skips intermediate stations on odd-numbered loops, +50% fare for express commuters.";
    c4.perkTag = "+50% EXPRESS FARE";
    c4.accentColor = Color{168, 85, 247, 255}; // Purple
    c4.id = UPGRADE_EXPRESS_LINE;
    pool.push_back(c4);

    UpgradeChoice c5;
    c5.title = "Fleet Expansion Grant";
    c5.description = "Receives a free extra EMU trainset and reduces future train purchase cost by 20%.";
    c5.perkTag = "FREE TRAIN -20% COST";
    c5.accentColor = Color{245, 158, 11, 255}; // Amber
    c5.id = UPGRADE_FLEET_GRANT;
    pool.push_back(c5);

    UpgradeChoice c6;
    c6.title = "Tourist Marketing Campaign";
    c6.description = "Attracts more tourists (+25% tourist spawn) who pay double fare at scenic stations.";
    c6.perkTag = "+25% TOURIST 2x FARE";
    c6.accentColor = Color{236, 72, 153, 255}; // Pink
    c6.id = UPGRADE_TOURIST_MARKETING;
    pool.push_back(c6);

    UpgradeChoice c7;
    c7.title = "Platform Expansion";
    c7.description = "All stations gain +2 platform length, reducing boarding time by 30%.";
    c7.perkTag = "-30% BOARDING TIME";
    c7.accentColor = Color{34, 211, 238, 255}; // Cyan
    c7.id = UPGRADE_PLATFORM_EXPANSION;
    pool.push_back(c7);

    UpgradeChoice c8;
    c8.title = "Rush Hour Bonus";
    c8.description = "During rush hours, fare multiplier increased to 3x (from 2x) and +20% commuter demand.";
    c8.perkTag = "3x RUSH FARE +20%";
    c8.accentColor = Color{239, 68, 68, 255}; // Red
    c8.id = UPGRADE_RUSH_HOUR_BONUS;
    pool.push_back(c8);

    UpgradeChoice c9;
    c9.title = "Train Tier Upgrade";
    c9.description = "Upgrades fleet to next tech tier: more cars, higher capacity, faster acceleration.";
    c9.perkTag = "TIER UP: SPEED+CAP";
    c9.accentColor = Color{255, 214, 0, 255}; // Gold
    c9.id = UPGRADE_TRAIN_TIER_UP;
    pool.push_back(c9);

    UpgradeChoice c10;
    c10.title = "Organic Growth Initiative";
    c10.description = "City zones develop naturally around stations. +50% population growth rate over time.";
    c10.perkTag = "+50% POP GROWTH";
    c10.accentColor = Color{52, 211, 153, 255}; // Green
    c10.id = UPGRADE_ORGANIC_GROWTH;
    pool.push_back(c10);

    UpgradeChoice c11;
    c11.title = "Infrastructure Budget";
    c11.description = "Unlocks buildings, roads, and bridges that spawn organically near your stations.";
    c11.perkTag = "UNLOCK BUILDINGS";
    c11.accentColor = Color{249, 115, 22, 255};
    c11.id = UPGRADE_INFRA_BUDGET;
    pool.push_back(c11);

    UpgradeChoice c12;
    c12.title = "Premium First-Class Cars";
    c12.description = "Adds premium seating with +30% fare revenue from all passengers.";
    c12.perkTag = "+30% ALL FARES";
    c12.accentColor = Color{255, 214, 0, 255};
    c12.id = UPGRADE_PREMIUM_CARS;
    pool.push_back(c12);

    UpgradeChoice c13;
    c13.title = "Station Free WiFi";
    c13.description = "Installs WiFi at all stations. +20% rider satisfaction, slower rating decay.";
    c13.perkTag = "+20% SATISFACTION";
    c13.accentColor = Color{56, 189, 248, 255};
    c13.id = UPGRADE_STATION_WIFI;
    pool.push_back(c13);

    UpgradeChoice c14;
    c14.title = "Night Operations";
    c14.description = "Trains continue running at night. +50% revenue during night cycle.";
    c14.perkTag = "+50% NIGHT REVENUE";
    c14.accentColor = Color{80, 80, 160, 255};
    c14.id = UPGRADE_NIGHT_OPS;
    pool.push_back(c14);

    UpgradeChoice c15;
    c15.title = "Mega Hub Upgrade";
    c15.description = "One station becomes a mega-hub. +100% throughput, -50% boarding time.";
    c15.perkTag = "MEGA HUB +100%";
    c15.accentColor = Color{168, 85, 247, 255};
    c15.id = UPGRADE_MEGA_HUB;
    pool.push_back(c15);

    // Shuffle and pick 3 unique upgrades
    std::vector<int> indices;
    for (int i = 0; i < (int)pool.size(); ++i) indices.push_back(i);
    for (int i = (int)indices.size() - 1; i > 0; --i) {
        int j = GetRandomValue(0, i);
        std::swap(indices[i], indices[j]);
    }
    for (int i = 0; i < 3 && i < (int)indices.size(); ++i) {
        activeUpgrades.push_back(pool[indices[i]]);
    }
}

void Game::ApplyUpgrade(int choiceIdx) {
    if (choiceIdx < 0 || choiceIdx >= (int)activeUpgrades.size()) {
        state = STATE_PLAYING;
        return;
    }

    UpgradeID uid = activeUpgrades[choiceIdx].id;
    Vector3 trainPos = train.GetLocomotivePos();

    switch (uid) {
        case UPGRADE_4CAR_EMU:
            train.SetCarriageCount(4);
            particles.SpawnConfetti(trainPos, 25);
            ShowToast("Upgrade Applied: 4-Car EMU Trainset deployed!", Color{56, 189, 248, 255});
            break;
        case UPGRADE_CBTC_SIGNALING:
            peeps.SetSpawnInterval(1.6f);
            particles.SpawnSparks(trainPos, 20);
            ShowToast("Upgrade Applied: CBTC Signaling & rapid dispatches active!", Color{220, 38, 38, 255});
            break;
        case UPGRADE_TRANSIT_SUBSIDY:
            economy.balance += 1200.0f;
            parkRating = std::min(100.0f, parkRating + 15.0f);
            particles.SpawnConfetti(trainPos, 30);
            ShowToast("Upgrade Applied: +$1,200 Subsidy & Commuter Satisfaction boosted!", Color{16, 185, 129, 255});
            break;
        case UPGRADE_EXPRESS_LINE:
            economy.baseFare *= 1.5f;
            train.SetTicketFare(economy.baseFare);
            cachedStats.ticketFare = economy.baseFare;
            particles.SpawnConfetti(trainPos, 20);
            ShowToast("Upgrade Applied: Express Line Service! +50% fare revenue!", Color{168, 85, 247, 255});
            break;
        case UPGRADE_FLEET_GRANT:
            if ((int)extraTrains.size() < MAX_EXTRA_TRAINS) {
                MetroTrain newTrain;
                newTrain.SetCarriageCount(GetTrainTier(trainTier).carCount);
                newTrain.Reset(tracks);
                extraTrains.push_back(newTrain);
                ShowToast("Upgrade Applied: Free EMU trainset received!", Color{245, 158, 11, 255});
            } else {
                economy.balance += 800.0f;
                ShowToast("Upgrade Applied: Fleet full! Received $800 cash equivalent.", Color{245, 158, 11, 255});
            }
            particles.SpawnConfetti(trainPos, 25);
            break;
        case UPGRADE_TOURIST_MARKETING:
            peeps.SetSpawnInterval(peeps.GetSpawnInterval() * 0.8f);
            particles.SpawnConfetti(trainPos, 20);
            ShowToast("Upgrade Applied: Tourist Marketing! +25% tourists, double fares!", Color{236, 72, 153, 255});
            break;
        case UPGRADE_PLATFORM_EXPANSION:
            parkRating = std::min(100.0f, parkRating + 10.0f);
            particles.SpawnConfetti(trainPos, 15);
            ShowToast("Upgrade Applied: Platform Expansion! -30% boarding time!", Color{34, 211, 238, 255});
            break;
        case UPGRADE_RUSH_HOUR_BONUS:
            rushCombo = std::max(rushCombo, 1.5f);
            particles.SpawnConfetti(trainPos, 25);
            ShowToast("Upgrade Applied: Rush Hour Bonus! 3x fare during peaks!", Color{239, 68, 68, 255});
            break;
        case UPGRADE_TRAIN_TIER_UP: {
            if (trainTier < 4) {
                trainTier++;
                const TrainTier& tier = GetTrainTier(trainTier);
                train.SetCarriageCount(tier.carCount);
                train.SetSpeedMultiplier(tier.speedMult);
                train.SetCapacityMultiplier(tier.capacityPerCar);
                for (auto& et : extraTrains) {
                    et.SetCarriageCount(tier.carCount);
                    et.SetSpeedMultiplier(tier.speedMult);
                    et.SetCapacityMultiplier(tier.capacityPerCar);
                }
                particles.SpawnConfetti(trainPos, 40);
                char buf[128];
                snprintf(buf, sizeof(buf), "Train upgraded to Tier %d: %s!", trainTier + 1, tier.name);
                ShowToast(buf, tier.color);
                tierUpFlashTimer = 2.0f;
                // Achieve "TRAIN EVOLUTION" on first upgrade
                if (trainTier == 1) {
                    snprintf(achievementTitle, sizeof(achievementTitle), "TRAIN EVOLUTION");
                    snprintf(achievementSub, sizeof(achievementSub), "Your fleet advances to %s", tier.name);
                    achievementColor = tier.color;
                    achievementShowTime = GetTime();
                }
            } else {
                economy.balance += 600.0f;
                ShowToast("Train already max tier! Received $600 bonus.", Color{255, 214, 0, 255});
            }
            break;
        }
        case UPGRADE_ORGANIC_GROWTH:
            popGrowthRate *= 1.5f;
            particles.SpawnConfetti(trainPos, 30);
            ShowToast("Upgrade Applied: Organic Growth! +50% population growth rate!", Color{52, 211, 153, 255});
            break;
        case UPGRADE_INFRA_BUDGET:
            infraUnlocked = true;
            particles.SpawnConfetti(trainPos, 25);
            ShowToast("Upgrade Applied: Infrastructure Budget! Buildings spawn near stations!", Color{249, 115, 22, 255});
            break;
        case UPGRADE_PREMIUM_CARS:
            economy.baseFare *= 1.3f;
            train.SetTicketFare(economy.baseFare);
            cachedStats.ticketFare = economy.baseFare;
            particles.SpawnConfetti(trainPos, 25);
            ShowToast("Upgrade Applied: Premium First-Class Cars! +30% all fares!", Color{255, 214, 0, 255});
            break;
        case UPGRADE_STATION_WIFI:
            parkRating = std::min(100.0f, parkRating + 20.0f);
            particles.SpawnConfetti(trainPos, 20);
            ShowToast("Upgrade Applied: Station Free WiFi! +20% rider satisfaction!", Color{56, 189, 248, 255});
            break;
        case UPGRADE_NIGHT_OPS:
            daySpeed *= 0.5f; // slower day/night cycle = more night time
            economy.baseFare *= 1.5f;
            train.SetTicketFare(economy.baseFare);
            cachedStats.ticketFare = economy.baseFare;
            particles.SpawnConfetti(trainPos, 30);
            ShowToast("Upgrade Applied: Night Operations! Trains run at night! +50% night revenue!", Color{80, 80, 160, 255});
            break;
        case UPGRADE_MEGA_HUB:
            popGrowthRate *= 2.0f;
            particles.SpawnConfetti(trainPos, 50);
            ShowToast("Upgrade Applied: MEGA HUB! +100% throughput, -50% boarding time!", Color{168, 85, 247, 255});
            shakeTimer = 0.5f;
            shakeIntensity = 5.0f;
            break;
        case UPGRADE_COUNT:
            break;
    }

    state = STATE_PLAYING;
    AudioManager::Play(SFX_UPGRADE_FANFARE, 0.9f);
}

void Game::UpdateAdvisor() {
    if (state != STATE_PLAYING || isPaused) return;
    if (advisorShowTime > 0.0f && (GetTime() - advisorShowTime) < 8.0f) return;

    float t = GetTime();
    int priority = -1;
    advisorTargetGx = -1;
    advisorTargetGy = -1;

    // Priority 10: No circuit closed - MUST build a loop
    if (!tracks.IsCircuitClosed()) {
        priority = 10;
        // Point to the open gap endpoint
        int ax, ay, bx, by;
        tracks.GetOpenEndpoints(ax, ay, bx, by);
        if (ax >= 0 && bx >= 0) {
            advisorTargetGx = ax;
            advisorTargetGy = ay;
        }
    }

    // Priority 9: Need more stations
    int stationCount = tracks.GetStationCount();
    if (stationCount < 2 && economy.totalDelivered < 25 && priority < 9) {
        priority = 9;
        // Suggest placing near the existing station but offset
        auto stations = tracks.GetAllStations();
        if (!stations.empty()) {
            int sgx = stations[0].gx;
            int sgy = stations[0].gy;
            // Suggest placing 3-4 tiles away along an open direction
            if (terrain[sgx + 4][sgy] == GROUND_GRASS || terrain[sgx + 4][sgy] == GROUND_PLAZA) {
                advisorTargetGx = sgx + 4;
                advisorTargetGy = sgy;
            } else if (terrain[sgx][sgy + 4] == GROUND_GRASS || terrain[sgx][sgy + 4] == GROUND_PLAZA) {
                advisorTargetGx = sgx;
                advisorTargetGy = sgy + 4;
            } else {
                advisorTargetGx = sgx + 3;
                advisorTargetGy = sgy;
            }
        }
    }

    // Priority 8: Cash very low
    if (economy.balance < 100.0f && economy.totalDelivered > 10 && priority < 8) {
        priority = 8;
        // Point to nearest station to suggest adding more
        auto stations = tracks.GetAllStations();
        if (!stations.empty()) {
            advisorTargetGx = stations[0].gx;
            advisorTargetGy = stations[0].gy;
        }
    }

    // Priority 7: Queue overcrowding
    if (peeps.IsOvercrowded() && priority < 7) {
        priority = 7;
        // Point to the overcrowded station
        auto stations = tracks.GetAllStations();
        for (auto& s : stations) {
            if (s.isOvercrowded) {
                advisorTargetGx = s.gx;
                advisorTargetGy = s.gy;
                break;
            }
        }
    }

    // Priority 6: Buy extra train
    if (economy.balance > 2000.0f && extraTrains.empty() && economy.totalDelivered > 50 && priority < 6) {
        priority = 6;
        // No map target needed - this is a toolbar action
    }

    // Priority 5: Expand territory
    if (economy.balance > 800.0f && economy.totalDelivered > 100 && buildRadius < 42 && priority < 5) {
        priority = 5;
        // Point to the outer edge of current territory
        advisorTargetGx = LAND_CENTER_X + buildRadius - 2;
        advisorTargetGy = LAND_CENTER_Y;
    }

    // Priority 4: Rating dropping - add scenery
    if (parkRating < 60.0f && economy.totalDelivered > 30 && priority < 4) {
        priority = 4;
        // Point to an empty tile near a station
        auto stations = tracks.GetAllStations();
        if (!stations.empty()) {
            int sgx = stations[0].gx + 2;
            int sgy = stations[0].gy;
            if (sgx >= 0 && sgx < GRID_SIZE && scenery[sgx][sgy] == SCENERY_NONE) {
                advisorTargetGx = sgx;
                advisorTargetGy = sgy;
            } else {
                advisorTargetGx = stations[0].gx;
                advisorTargetGy = stations[0].gy + 2;
            }
        }
    }

    // Priority 3: Upgrade stations
    if (economy.balance > 500.0f && economy.totalDelivered > 200 && priority < 3) {
        priority = 3;
        // Point to the lowest-level station
        auto stations = tracks.GetAllStations();
        int minLevel = 99;
        for (auto& s : stations) {
            if (s.stationLevel < minLevel) {
                minLevel = s.stationLevel;
                advisorTargetGx = s.gx;
                advisorTargetGy = s.gy;
            }
        }
    }

    // Priority 3b: Add scenery near stations (when rating is mediocre but > 60)
    if (parkRating >= 60.0f && parkRating < 75.0f && economy.totalDelivered > 150 &&
        economy.balance > 300.0f && priority < 3) {
        priority = 3;
        auto stations = tracks.GetAllStations();
        if (!stations.empty()) {
            int sgx = stations[0].gx + 2;
            int sgy = stations[0].gy;
            if (sgx >= 0 && sgx < GRID_SIZE && scenery[sgx][sgy] == SCENERY_NONE &&
                !tracks.HasPiece(sgx, sgy) && terrain[sgx][sgy] != GROUND_WATER) {
                advisorTargetGx = sgx;
                advisorTargetGy = sgy;
            }
        }
    }

    // Priority 2: Grow network
    if (economy.totalDelivered > 300 && tracks.GetStationCount() < 4 && priority < 2) {
        priority = 2;
        // Suggest placing in an open area away from existing stations
        auto stations = tracks.GetAllStations();
        if (!stations.empty()) {
            // Find a good open area
            int bestGx = -1, bestGy = -1;
            for (int r = 4; r <= 8; r += 2) {
                for (auto& s : stations) {
                    int tryX = s.gx + r;
                    int tryY = s.gy;
                    if (tryX >= 0 && tryX < GRID_SIZE && tryY >= 0 && tryY < GRID_SIZE &&
                        terrain[tryX][tryY] != GROUND_WATER && scenery[tryX][tryY] == SCENERY_NONE &&
                        !tracks.HasPiece(tryX, tryY)) {
                        bestGx = tryX;
                        bestGy = tryY;
                        break;
                    }
                }
                if (bestGx >= 0) break;
            }
            advisorTargetGx = bestGx;
            advisorTargetGy = bestGy;
        }
    }

    // Priority 1b: Signal discipline (when circuit is closed and signals are all green)
    if (tracks.IsCircuitClosed() && economy.totalDelivered > 100 && priority < 1) {
        priority = 1;
        // No map target needed - informational
    }

    // Priority 0: Network optimization (late game)
    if (economy.totalDelivered > 500 && tracks.GetStationCount() >= 4 && priority < 0) {
        priority = 0;
        // No map target needed - informational
    }

    // Show if we have a higher-priority suggestion than last shown
    if (priority > lastAdvisorTier) {
        lastAdvisorTier = priority;
        advisorShowTime = t;
        advisorPulseTimer = 0.0f;
    }

    // Animate pulse timer
    advisorPulseTimer += GetFrameTime();
}

void Game::HandleInput() {
    Vector2 mousePos = GetMousePosition();

    // 0. Title & Start Menu Screen Input Handler
    if (state == STATE_TITLE) {
        if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui.CheckTitleStartClick(mousePos)) ||
            IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            state = STATE_PLAYING;
            endlessMode = false;
            RecenterCamera();
            AudioManager::Play(SFX_BUTTON_CLICK, 0.8f);
            ShowToast("[METRO] Service Commenced! Train will board passengers at Station [7]", Color{56, 189, 248, 255}, 5.0f);
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui.CheckTitleEndlessClick(mousePos)) {
            state = STATE_PLAYING;
            endlessMode = true;
            RecenterCamera();
            AudioManager::Play(SFX_BUTTON_CLICK, 0.8f);
            ShowToast("[SANDBOX] Endless Mode — build freely, no win condition!", Color{56, 189, 248, 255}, 5.0f);
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
    if (IsKeyPressed(KEY_L)) {
        lineColorIdx = (lineColorIdx + 1) % LINE_COLOR_COUNT;
        lineColorFlash = 0.5f;
        train.SetTrainTheme(LINE_COLORS[lineColorIdx].primary);
        tracks.SetTrackColor(LINE_COLORS[lineColorIdx].primary);
        cachedStats.themeColor = LINE_COLORS[lineColorIdx].primary;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
        ShowToast(TextFormat("Line Color: %s", LINE_COLORS[lineColorIdx].name),
                  LINE_COLORS[lineColorIdx].primary, 2.0f);
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
        else currentScenery = SCENERY_VENT_GRATE;
        isBulldozing = false;
        AudioManager::Play(SFX_BUTTON_CLICK, 0.6f);
    }
    if (IsKeyPressed(KEY_FOUR)) {
        if (activeTab == CAT_TRACK) currentTrack = TRACK_VIADUCT_ELEVATED;
        else if (activeTab == CAT_INFRA) { currentGround = GROUND_GRASS; isTerraformingRaise = false; }
        else currentScenery = SCENERY_PALM_TREE;
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

    // Handle Advisor dismiss click
    if (advisorShowTime > 0.0f && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (ui.CheckAdvisorDismissClick(mousePos, advisorShowTime)) {
            advisorShowTime = 0.0f;
            advisorDismissCount++;
            lastAdvisorTier = -1; // reset to allow re-showing
            AudioManager::Play(SFX_BUTTON_CLICK, 0.5f);
        }
        // Also dismiss if clicking on the highlighted tile
        else if (advisorTargetGx >= 0 && advisorTargetGy >= 0 &&
                 hoveredGx == advisorTargetGx && hoveredGy == advisorTargetGy) {
            advisorShowTime = 0.0f;
            advisorDismissCount++;
            lastAdvisorTier = -1;
            AudioManager::Play(SFX_BUTTON_CLICK, 0.4f);
        }
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
                    SCENERY_METRO_ENTRANCE, SCENERY_TURNSTILE_GATE, SCENERY_VENT_GRATE,
                    SCENERY_PALM_TREE, SCENERY_BENCH, SCENERY_LAMP_POST,
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
                            nt.SetCarriageCount(GetTrainTier(trainTier).carCount);
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
                            particles.SpawnSmoke(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ}, 5);
                            particles.SpawnFloatingText(Vector3{(float)hoveredGx + 0.5f, (float)hoveredGy + 0.5f, (float)currentZ + 0.6f}, "-$40", Color{239, 68, 68, 255});

                            // Track placement rhythmic streak
                            static float lastTrackPlaceTime = 0.0f;
                            static int trackCombo = 0;
                            float now = (float)GetTime();
                            if (now - lastTrackPlaceTime < 1.4f) trackCombo++;
                            else trackCombo = 1;
                            lastTrackPlaceTime = now;
                            if (trackCombo == 5) {
                                ShowToast("★ 5-Track Streak! Rapid transit expansion!", Color{250, 204, 21, 255}, 1.8f);
                            } else if (trackCombo == 10) {
                                ShowToast("★ 10-Track Mega Streak! Master civil engineer!", Color{56, 189, 248, 255}, 2.2f);
                                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.7f);
                            }

                            if (!wasClosed && tracks.IsCircuitClosed()) {
                                ShowToast("[CIRCUIT] Transit Loop Closed! Regular EMU Schedule Active!", Color{34, 197, 94, 255}, 4.0f);
                                AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
                                circuitFlashTimer = 0.8f;
                                // Achievement popup
                                achievementShowTime = GetTime();
                                snprintf(achievementTitle, sizeof(achievementTitle), "FIRST LOOP CLOSED!");
                                snprintf(achievementSub, sizeof(achievementSub), "Your transit network is now operational!");
                                achievementColor = Color{34, 197, 94, 255};
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
                    else if (currentScenery == SCENERY_VENT_GRATE) { scnCost = 45; boost = 2.0f; }
                    else if (currentScenery == SCENERY_PALM_TREE) { scnCost = 40; boost = 2.5f; }
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
    // Toolbar auto-hide: fade toolbar when not interacting
    if (state == STATE_PLAYING) {
        bool mouseOverToolbar = GetMousePosition().y > GetScreenHeight() - 180;
        bool anyKeyPressed = IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_THREE) ||
                             IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_FIVE) || IsKeyPressed(KEY_SIX) ||
                             IsKeyPressed(KEY_SEVEN) || IsKeyPressed(KEY_EIGHT) || IsKeyPressed(KEY_TAB) ||
                             IsKeyPressed(KEY_L) || IsKeyPressed(KEY_R) || IsKeyPressed(KEY_X);
        if (mouseOverToolbar || anyKeyPressed || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            toolbarIdleTimer = 0.0f;
        }
        toolbarIdleTimer += dt;
        float targetAlpha = (toolbarIdleTimer > 4.0f) ? 0.4f : 1.0f;
        toolbarAlpha += (targetAlpha - toolbarAlpha) * 3.0f * dt;
    } else {
        toolbarAlpha = 1.0f;
    }

    // Screen shake decay
    if (shakeTimer > 0.0f) {
        shakeTimer -= dt;
        shakeIntensity = shakeTimer > 0.0f ? sinf(GetTime() * 80.0f) * 3.0f * (shakeTimer / 0.15f) : 0.0f;
    } else {
        shakeIntensity = 0.0f;
    }
    if (circuitFlashTimer > 0.0f) circuitFlashTimer -= dt;
    if (tierUpFlashTimer > 0.0f) tierUpFlashTimer -= dt;

    // Day/night cycle (visual only)
    if (state == STATE_PLAYING && !isPaused) {
        timeOfDay += daySpeed * dt;
        if (timeOfDay >= 1.0f) timeOfDay -= 1.0f;
        // Compute ambient tint from time of day
        float t = timeOfDay;
        float brightness = 1.0f;
        float warmth = 0.0f;
        if (t < 0.2f) {        // Night
            brightness = 0.35f;
            warmth = -0.1f;
        } else if (t < 0.3f) { // Dawn
            float p = (t - 0.2f) / 0.1f;
            brightness = 0.35f + 0.65f * p;
            warmth = -0.1f + 0.2f * p;
        } else if (t < 0.5f) { // Morning
            brightness = 1.0f;
            warmth = 0.1f;
        } else if (t < 0.7f) { // Afternoon
            brightness = 1.0f;
            warmth = 0.0f;
        } else if (t < 0.8f) { // Dusk
            float p = (t - 0.7f) / 0.1f;
            brightness = 1.0f - 0.3f * p;
            warmth = 0.15f * (1.0f - p);
        } else {                // Night
            brightness = 0.7f - 0.35f * ((t - 0.8f) / 0.2f);
            warmth = 0.0f;
        }
        int r = (int)(255.0f * (brightness + warmth * 0.3f));
        int g = (int)(255.0f * brightness);
        int b = (int)(255.0f * (brightness - warmth * 0.1f));
        ambientTint = {(unsigned char)std::min(255, std::max(0, r)),
                       (unsigned char)std::min(255, std::max(0, g)),
                       (unsigned char)std::min(255, std::max(0, b)), 255};
    }

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
        ui.SetModalEntryTime(GetTime());
        AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
        return;
    }
    bool wasRush = rushHourActive;
    // Dynamic rush hour: more frequent and longer as player progresses
    float progressPct = std::min(1.0f, (float)economy.totalDelivered / (float)WIN_GOAL);
    float rushWindowStart = 12.0f - progressPct * 4.0f;  // starts earlier (8s at max)
    float rushWindowEnd = 40.0f + progressPct * 10.0f;    // ends later (50s at max)
    float rushCycleLen = 14.0f - progressPct * 4.0f;       // cycles faster (10s at max)
    float rushDutyLen = 7.0f + progressPct * 2.0f;         // rush periods longer (9s at max)
    if (weekTimer >= rushWindowStart && weekTimer < rushWindowEnd) {
        rushHourActive = (fmodf(weekTimer, rushCycleLen) < rushDutyLen);
    } else {
        rushHourActive = false;
    }
    if (rushHourActive && !wasRush) {
        rushHourFlashTimer = 0.6f; // 0.6s red screen-edge flash
        rushHourCountdown = 0.0f;
    }
    // Rush hour countdown telegraph (5s warning before rush hour)
    if (!rushHourActive && state == STATE_PLAYING) {
        float nextRushStart = rushWindowStart;
        float timeToRush = nextRushStart - weekTimer;
        if (timeToRush > 0.0f && timeToRush < 5.0f && weekTimer > rushWindowEnd) {
            // Approaching next rush cycle
            rushHourCountdown = timeToRush;
        } else {
            rushHourCountdown = 0.0f;
        }
    }
    if (rushHourActive) {
        rushHourTimer += simDt;
        if (rushHourFlashTimer > 0.0f) rushHourFlashTimer -= simDt;
        // Dynamic difficulty: faster spawning during rush hour, scales with progress
        float diffScale = std::max(0.5f, 1.0f - (float)economy.totalDelivered * 0.0003f);
        peeps.SetSpawnInterval(0.85f * diffScale);
    } else {
        // Base spawn also scales with progress
        float diffScale = std::max(0.7f, 1.0f - (float)economy.totalDelivered * 0.0002f);
        peeps.SetSpawnInterval(1.9f * diffScale);
        rushHourTimer = 0.0f;
    }

    // 3. Update Particles
    particles.Update(simDt);

    // 4. Update Metro Train Operations & Physics (CBTC Moving-Block Headway Control)
    int delivered = 0;
    float fareRevenue = 0.0f;
    float leadAheadDist = extraTrains.empty() ? -1.0f : extraTrains.back().GetTrainDistance();
    train.Update(simDt, tracks, particles, delivered, fareRevenue, leadAheadDist);

    // Extra fleet EMUs (purchasable); headway spacing keeps trains evenly distributed
    for (size_t i = 0; i < extraTrains.size(); ++i) {
        float aheadDist = (i == 0) ? train.GetTrainDistance() : extraTrains[i - 1].GetTrainDistance();
        int deliveredN = 0;
        float fareN = 0.0f;
        extraTrains[i].Update(simDt, tracks, particles, deliveredN, fareN, aheadDist);
        delivered += deliveredN;
        fareRevenue += fareN;
    }

    // Subway tunnel resonant echo audio
    static float tunnelSoundTimer = 0.0f;
    tunnelSoundTimer += dt;
    Vector3 locPos = train.GetLocomotivePos();
    int tgx = (int)roundf(locPos.x);
    int tgy = (int)roundf(locPos.y);
    const TrackNode* tNode = tracks.GetPiece(tgx, tgy);
    if (tNode && (tNode->type == TRACK_TUNNEL || tNode->type == TRACK_TUNNEL_PORTAL) && train.GetSpeedKmh() > 10.0f) {
        if (tunnelSoundTimer > 1.2f) {
            AudioManager::Play(SFX_TUNNEL_REVERB, 0.48f);
            tunnelSoundTimer = 0.0f;
        }
    }

    // Dynamic Station PIDS Arrival Displays (Next train live ETA)
    std::vector<float> extraDists;
    for (const auto& et : extraTrains) extraDists.push_back(et.GetTrainDistance());
    tracks.UpdateStationPIDS(train.GetTrainDistance(), extraDists, train.GetSpeedKmh());

    if (delivered > 0) {
        economy.totalDelivered += delivered;
        bestSessionRiders = std::max(bestSessionRiders, economy.totalDelivered);

        // Ridership Rush Combo multiplier
        comboStreak += delivered;
        comboTimer = 18.0f;
        rushCombo = 1.0f + std::min(1.0f, (float)comboStreak * 0.05f); // up to 2.0x
        float rushMult = rushHourActive ? 1.25f : 1.0f;                  // rush-hour fare surge
        float eventFundMult = (activeEvent >= 0) ? EVENTS[activeEvent].fundBoost : 1.0f;
        float comboBonus = fareRevenue * rushMult * (rushCombo - 1.0f);
        float totalEarned = fareRevenue * rushMult * eventFundMult + comboBonus;
        economy.balance += totalEarned;
        parkRating = std::min(100.0f, parkRating + (float)delivered * 0.6f);

        // Floating text for rider delivery
        Vector3 locoP = train.GetLocomotivePos();
        particles.SpawnFloatingText(Vector3{locoP.x, locoP.y, locoP.z + 1.0f},
                                    TextFormat("+%d Rider%s!", delivered, delivered > 1 ? "s" : ""),
                                    Color{52, 211, 153, 255});
        peeps.AlightPassengers(delivered, Vector2{locoP.x, locoP.y});

        if (rushCombo > 1.05f) {
            particles.SpawnFloatingText(Vector3{locoP.x, locoP.y, locoP.z + 1.4f}, TextFormat("RUSH x%.1f!", rushCombo), Color{255, 215, 0, 255});
        }

        // Milestone grants (expanded progression: 12 tiers up to 1500)
        int milestones[] = {25, 50, 100, 250, 500, 1000, 2500, 5000, 7500, WIN_GOAL};
        for (int m : milestones) {
            if (economy.totalDelivered >= m && lastMilestoneAwarded < m) {
                lastMilestoneAwarded = m;
                float bonus = (float)m * 10.0f;
                economy.balance += bonus;
                // Big celebration: confetti burst + screen shake + golden flash
                particles.SpawnConfetti(locoP, 50);
                particles.SpawnFloatingText(Vector3{locoP.x, locoP.y, locoP.z + 1.2f}, TextFormat("+$%.0f BONUS!", bonus), Color{255, 215, 0, 255});
                particles.SpawnFloatingText(Vector3{locoP.x, locoP.y, locoP.z + 2.0f}, TextFormat("%d COMMUTERS!", m), Color{255, 255, 255, 255});
                shakeTimer = 0.3f;
                shakeIntensity = 4.0f;
                circuitFlashTimer = 0.6f;
                ShowToast(TextFormat("[MILESTONE] %d Commuters Served! Grant: +$%.0f", m, bonus), Color{34, 197, 94, 255}, 5.0f);
                AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
                // Achievement popup
                achievementShowTime = GetTime();
                snprintf(achievementTitle, sizeof(achievementTitle), "%d COMMUTERS!", m);
                snprintf(achievementSub, sizeof(achievementSub), "Bonus: +$%.0f earned. Keep building!", bonus);
                achievementColor = Color{255, 214, 0, 255};
                break;
            }
        }

        // Rank-up detection
        int currentRank = GetTransitRankTier();
        if (currentRank > lastRankTier) {
            lastRankTier = currentRank;
            particles.SpawnConfetti(locoP, 80);
            particles.SpawnFloatingText(Vector3{locoP.x, locoP.y, locoP.z + 2.0f}, TextFormat("RANK UP: %s!", GetTransitRank()), GetRankColor());
            shakeTimer = 0.4f;
            shakeIntensity = 5.0f;
            ShowToast(TextFormat("[RANK UP] Promoted to %s!", GetTransitRank()), GetRankColor(), 6.0f);
            AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
            // Achievement popup
            achievementShowTime = GetTime();
            snprintf(achievementTitle, sizeof(achievementTitle), "RANK UP: %s!", GetTransitRank());
            snprintf(achievementSub, sizeof(achievementSub), "Your transit empire grows stronger!");
            achievementColor = GetRankColor();
        }

        if (economy.totalDelivered >= WIN_GOAL && !endlessMode) {
            state = STATE_VICTORY;
            stateEntryTime = GetTime();
            AudioManager::Play(SFX_UPGRADE_FANFARE, 1.0f);
        } else if (endlessMode) {
            int nextEndlessMilestone = ((lastMilestoneAwarded / 500) + 1) * 500;
            if (economy.totalDelivered >= nextEndlessMilestone && lastMilestoneAwarded < nextEndlessMilestone) {
                lastMilestoneAwarded = nextEndlessMilestone;
                float bonus = 5000.0f;
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

    // 7b. Weather effects: rain during rush hour / storm, umbrellas, cherry blossoms during calm
    if (state == STATE_PLAYING && !isPaused) {
        bool isRaining = rushHourActive || (activeEvent == 6);
        peeps.SetRaining(isRaining);

        static float weatherTimer = 0.0f;
        weatherTimer += dt;
        if (isRaining && weatherTimer > 0.25f) {
            // Rain particle downpour
            particles.SpawnRain(GetScreenWidth(), GetScreenHeight(), 10);
            weatherTimer = 0.0f;

            // Wet rail spray mist when train runs at speed
            if (train.GetSpeedKmh() > 15.0f) {
                Vector3 tPos = train.GetLocomotivePos();
                particles.SpawnSmoke(Vector3{tPos.x, tPos.y, tPos.z * 0.5f}, 1);
            }
        } else if (!isRaining && weatherTimer > 1.5f && parkRating > 70.0f) {
            // Cherry blossom petals when rating is good
            particles.SpawnPetals(GetScreenWidth(), GetScreenHeight(), 3);
            weatherTimer = 0.0f;
        }

        // Ambient Soundscape (nature, coastal, crickets & thunder audio immersion)
        static float ambientTimer = 0.0f;
        ambientTimer += dt;
        if (ambientTimer >= 5.5f) {
            ambientTimer = 0.0f;
            if (isRaining) {
                AudioManager::Play(SFX_THUNDER_ROLL, 0.45f);
            } else if (nightMode) {
                AudioManager::Play(SFX_NIGHT_CRICKET, 0.28f);
            } else {
                Vector2 cGrid = Iso::ScreenToGrid(Vector2{(float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f}, cameraPos, zoom, 0.0f);
                if (cGrid.y >= 22.0f || (cGrid.x >= 11.0f && cGrid.x <= 15.0f)) {
                    AudioManager::Play(SFX_OCEAN_AMBIENT, 0.32f);
                } else {
                    AudioManager::Play(SFX_MORNING_BIRD, 0.26f);
                }
            }
        }

        // Natural events system
        if (activeEvent >= 0) {
            eventTimer -= dt;
            if (eventTimer <= 0.0f) {
                activeEvent = -1;
                eventCooldown = 15.0f; // cooldown before next event
                ShowToast("Event ended. Calm returns...", Color{180, 180, 180, 255}, 2.0f);
            }
        } else {
            eventCooldown -= dt;
            if (eventCooldown <= 0.0f && economy.totalDelivered > 10) {
                // Random chance to trigger event (roughly every 30-60s)
                if (GetRandomValue(0, 3000) < 2) {
                    activeEvent = GetRandomValue(0, EVENT_COUNT - 1);
                    eventTimer = EVENTS[activeEvent].duration;
                    ShowToast(TextFormat("[EVENT] %s: %s", EVENTS[activeEvent].name, EVENTS[activeEvent].desc),
                              EVENTS[activeEvent].color, 4.0f);
                    if (activeEvent == 6) {
                        AudioManager::Play(SFX_THUNDER_ROLL, 0.70f);
                    } else {
                        AudioManager::Play(SFX_UPGRADE_FANFARE, 0.8f);
                        particles.SpawnConfetti(train.GetLocomotivePos(), 25);
                    }
                }
            }
        }
    }

    // 7c. Organic population growth - people appear as time passes
    if (state == STATE_PLAYING && !isPaused) {
        popGrowthTimer += dt;
        float riderMult = (activeEvent >= 0) ? EVENTS[activeEvent].riderBoost : 1.0f;
        float popInterval = 2.0f / (popGrowthRate * riderMult); // adjusted by event
        if (popGrowthTimer >= popInterval) {
            popGrowthTimer -= popInterval;
            auto stations = tracks.GetAllStations();
            int stationCount = (int)stations.size();
            if (stationCount >= 2) {
                totalWorldPopulation++;
                if (totalWorldPopulation >= 500) populationTier = 4;
                else if (totalWorldPopulation >= 200) populationTier = 3;
                else if (totalWorldPopulation >= 80) populationTier = 2;
                else if (totalWorldPopulation >= 30) populationTier = 1;
                else populationTier = 0;

                // Population tier milestone achievements
                static int lastPopTier = 0;
                if (populationTier > lastPopTier) {
                    lastPopTier = populationTier;
                    static const char* tierNames[] = {"Village", "Town", "City", "Metropolis", "MEGACITY"};
                    static Color tierColors[] = {
                        {148,163,184,255}, {52,211,153,255}, {56,189,248,255},
                        {168,85,247,255}, {255,214,0,255}
                    };
                    snprintf(achievementTitle, sizeof(achievementTitle), "%s REACHED!", tierNames[populationTier]);
                    snprintf(achievementSub, sizeof(achievementSub), "Population: %d citizens", totalWorldPopulation);
                    achievementColor = tierColors[populationTier];
                    achievementShowTime = GetTime();
                    particles.SpawnConfetti(train.GetLocomotivePos(), 30 + populationTier * 10);
                }

                peeps.SpawnCommuter();
            }
        }
    }

    // 7e. Research tree progression (Factorio-inspired)
    if (state == STATE_PLAYING && !isPaused) {
        int researchThresholds[] = {0, 100, 300, 750, 1500, 3000, 5000, 8000};
        int nextTier = researchTier + 1;
        if (nextTier < RESEARCH_COUNT && economy.totalDelivered >= researchThresholds[nextTier]) {
            researchTier = nextTier;
            researchUnlocked[nextTier] = true;
            // Unlock features at key tiers
            if (nextTier == RESEARCH_CIVIC) infraUnlocked = true;
            // Research completion notification
            Color tierColors[] = {
                Color{56, 189, 248, 255},   // Civic - blue
                Color{52, 211, 153, 255},   // Commercial - green
                Color{249, 115, 22, 255},   // Industrial - orange
                Color{168, 85, 247, 255},   // HighTech - purple
                Color{236, 72, 153, 255},   // Urban - pink
                Color{34, 197, 94, 255},    // Biotech - emerald
                Color{255, 214, 0, 255},    // MegaProject - gold
            };
            int colorIdx = std::min(nextTier - 1, 6);
            snprintf(achievementTitle, sizeof(achievementTitle), "RESEARCH: %s", GetResearchName((ResearchTier)nextTier));
            snprintf(achievementSub, sizeof(achievementSub), "New capabilities unlocked! (%d/%d)", nextTier, RESEARCH_COUNT - 1);
            achievementColor = (colorIdx >= 0 && colorIdx < 7) ? tierColors[colorIdx] : Color{255, 255, 255, 255};
            achievementShowTime = GetTime();
            particles.SpawnConfetti(train.GetLocomotivePos(), 40);
            shakeTimer = 0.2f;
            shakeIntensity = 3.0f;
        }
        // Calculate research progress bar
        if (nextTier < RESEARCH_COUNT) {
            int prev = researchThresholds[researchTier];
            int next = researchThresholds[nextTier];
            researchProgress = (float)(economy.totalDelivered - prev) / (float)(next - prev);
            researchProgress = std::max(0.0f, std::min(1.0f, researchProgress));
        } else {
            researchProgress = 1.0f;
        }
    }

    // 7f. World district generation (auto-expanding zones)
    if (state == STATE_PLAYING && !isPaused) {
        districtSpawnTimer += dt;
        if (districtSpawnTimer >= 15.0f && (int)districts.size() < maxDistricts) {
            districtSpawnTimer = 0.0f;
            // Spawn a new district in an unoccupied area
            int cx = GetRandomValue(8, GRID_SIZE - 8);
            int cy = GetRandomValue(8, GRID_SIZE - 8);
            int radius = GetRandomValue(4, 7);
            // Check if area is free
            bool overlaps = false;
            for (const auto& d : districts) {
                int dx = cx - d.centerX;
                int dy = cy - d.centerY;
                if (dx * dx + dy * dy < (d.radius + radius + 2) * (d.radius + radius + 2)) {
                    overlaps = true;
                    break;
                }
            }
            if (!overlaps) {
                WorldDistrict nd;
                nd.centerX = cx;
                nd.centerY = cy;
                nd.radius = radius;
                nd.biome = (BiomeType)GetRandomValue(0, BIOME_COUNT - 1);
                nd.population = 0;
                nd.unlocked = (int)districts.size() <= researchTier + 1;
                nd.spawnTimer = 0.0f;
                districts.push_back(nd);
                // Terrain the district
                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        int gx = cx + dx;
                        int gy = cy + dy;
                        if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
                            if (dx * dx + dy * dy <= radius * radius) {
                                if (terrain[gx][gy] == GROUND_GRASS && !tracks.HasPiece(gx, gy)) {
                                    if (nd.biome == BIOME_WATERFRONT) terrain[gx][gy] = GROUND_SAND;
                                    else if (nd.biome == BIOME_PARKLAND) terrain[gx][gy] = GROUND_GRASS;
                                    else if (nd.biome == BIOME_INDUSTRIAL) terrain[gx][gy] = GROUND_STONE;
                                    else if (nd.biome == BIOME_DOWNTOWN) terrain[gx][gy] = GROUND_PLAZA;
                                    else terrain[gx][gy] = GROUND_PATH;
                                    groundZ[gx][gy] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 7d. Infrastructure spawning - buildings appear near stations over time
    if (state == STATE_PLAYING && !isPaused && infraUnlocked) {
        infraSpawnTimer += dt;
        if (infraSpawnTimer >= 8.0f) { // every 8 seconds
            infraSpawnTimer = 0.0f;
            auto stations = tracks.GetAllStations();

            // Auto-build paths between nearby stations (road network)
            for (size_t i = 0; i < stations.size(); ++i) {
                for (size_t j = i + 1; j < stations.size(); ++j) {
                    int dx = stations[j].gx - stations[i].gx;
                    int dy = stations[j].gy - stations[i].gy;
                    int dist = abs(dx) + abs(dy);
                    if (dist <= 8) {
                        // Place path tiles between stations
                        int mx = (stations[i].gx + stations[j].gx) / 2;
                        int my = (stations[i].gy + stations[j].gy) / 2;
                        if (mx >= 0 && mx < GRID_SIZE && my >= 0 && my < GRID_SIZE) {
                            if (terrain[mx][my] == GROUND_GRASS && !tracks.HasPiece(mx, my)) {
                                terrain[mx][my] = GROUND_PATH;
                                // Also extend path one step toward each station
                                int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
                                int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
                                int nx = mx + stepX, ny = my + stepY;
                                if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE &&
                                    terrain[nx][ny] == GROUND_GRASS && !tracks.HasPiece(nx, ny)) {
                                    terrain[nx][ny] = GROUND_PATH;
                                }
                            }
                        }
                    }
                }
            }

            // Spawn buildings near stations
            for (const auto& st : stations) {
                int tries = 0;
                while (tries < 5) {
                    int rx = st.gx + GetRandomValue(-3, 3);
                    int ry = st.gy + GetRandomValue(-3, 3);
                    if (rx >= 0 && rx < GRID_SIZE && ry >= 0 && ry < GRID_SIZE) {
                        if (terrain[rx][ry] == GROUND_GRASS && scenery[rx][ry] == SCENERY_NONE &&
                            tracks.HasPiece(rx, ry) == false) {
                            int bldg = GetRandomValue(0, 5);
                            if (bldg == 0) scenery[rx][ry] = SCENERY_LAMP_POST;
                            else if (bldg == 1) scenery[rx][ry] = SCENERY_BENCH;
                            else if (bldg == 2) scenery[rx][ry] = SCENERY_NEWSSTAND;
                            else if (bldg == 3) scenery[rx][ry] = SCENERY_STREET_TREE;
                            else if (bldg == 4) scenery[rx][ry] = SCENERY_BIKE_RACK;
                            else scenery[rx][ry] = SCENERY_FLOWER_BED;
                            // Buildings grow taller with population
                            int maxHeight = 1 + populationTier;
                            groundZ[rx][ry] = std::min(groundZ[rx][ry] + 1, maxHeight + 1);
                            break;
                        }
                    }
                    tries++;
                }
            }

            // Higher population = upgrade existing grass to plaza near stations
            if (populationTier >= 2) {
                for (const auto& st : stations) {
                    int rx = st.gx + GetRandomValue(-1, 1);
                    int ry = st.gy + GetRandomValue(-1, 1);
                    if (rx >= 0 && rx < GRID_SIZE && ry >= 0 && ry < GRID_SIZE) {
                        if (terrain[rx][ry] == GROUND_GRASS && !tracks.HasPiece(rx, ry)) {
                            terrain[rx][ry] = GROUND_PLAZA;
                        }
                    }
                }
            }
        }
    }

    // 8. Smart Advisor - contextual guidance
    UpdateAdvisor();
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(nightMode ? Color{15, 23, 42, 255} : Color{241, 245, 249, 255}); // Slate 900 night vs Slate 100 soft architectural canvas

    if (state == STATE_TITLE) {
        ui.DrawTitleScreen(bestSessionRiders);
        EndDrawing();
        return;
    }

    // 1. Draw Ground Tiles (with screen shake offset) - frustum culled
    Vector2 drawCam = {cameraPos.x + shakeIntensity, cameraPos.y + shakeIntensity * 0.6f};
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (!Iso::IsVisible(x, y, groundZ[x][y], drawCam, zoom)) continue;
            bool isHovered = (x == hoveredGx && y == hoveredGy);
            Iso::DrawTile(x, y, groundZ[x][y], terrain[x][y], drawCam, zoom, isHovered);
        }
    }

    // 2. Draw Transit Portal Arch Marquee at entrance (0, 9)
    Iso::DrawTransitPortalArch({0.0f, 9.0f}, drawCam, zoom);

    // 3. Draw Scenery Items (frustum culled)
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (scenery[x][y] != SCENERY_NONE) {
                if (!Iso::IsVisible(x, y, groundZ[x][y], drawCam, zoom)) continue;
                Iso::DrawScenery(x, y, groundZ[x][y], scenery[x][y], drawCam, zoom);
            }
        }
    }

    // 4. Draw Station Litter / Messes (culled)
    int scrW = GetScreenWidth(), scrH = GetScreenHeight();
    for (const auto& m : messes) {
        Vector2 sp = Iso::GridToScreen(m.pos.x, m.pos.y, 0.0f, drawCam, zoom);
        if (sp.x < -80 || sp.x > scrW + 80 || sp.y < -80 || sp.y > scrH + 80) continue;
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

    // 8b. Line color flash effect
    if (lineColorFlash > 0.0f) {
        float a = lineColorFlash * 0.4f;
        Color lc = LINE_COLORS[lineColorIdx].primary;
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      Color{lc.r, lc.g, lc.b, (unsigned char)(a * 255)});
        lineColorFlash -= GetFrameTime();
    }

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

    // 10c. Smart Advisor map indicators: glowing pulse on target tile + directional arrow
    if (state == STATE_PLAYING && !isPaused && advisorShowTime > 0.0f &&
        advisorTargetGx >= 0 && advisorTargetGx < GRID_SIZE &&
        advisorTargetGy >= 0 && advisorTargetGy < GRID_SIZE) {
        float age = GetTime() - advisorShowTime;
        if (age < 8.0f) {
            float pulse = 0.5f + 0.5f * sinf(advisorPulseTimer * 4.0f);
            int gx = advisorTargetGx;
            int gy = advisorTargetGy;

            // Determine color based on current advice
            Color glowCol = Color{56, 189, 248, 255}; // default cyan
            if (!tracks.IsCircuitClosed()) glowCol = Color{239, 68, 68, 255}; // red
            else if (peeps.IsOvercrowded()) glowCol = Color{239, 68, 68, 255}; // red
            else if (parkRating < 60.0f) glowCol = Color{168, 85, 247, 255}; // purple
            else if (parkRating >= 60.0f && parkRating < 75.0f && economy.totalDelivered > 150) glowCol = Color{168, 85, 247, 255}; // purple (scenery)
            else if (economy.balance > 800.0f && economy.totalDelivered > 100 && buildRadius < 42) glowCol = Color{52, 211, 153, 255}; // green

            // Glowing ring on target tile
            Vector2 tPos = Iso::GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, 0.0f, drawCam, zoom);
            float rw = TILE_WIDTH * zoom * 0.6f;
            float rh = TILE_HEIGHT * zoom * 0.6f;
            float glowAlpha = 40.0f + 50.0f * pulse;
            DrawEllipse((int)tPos.x, (int)tPos.y, rw * (1.0f + 0.15f * pulse), rh * (1.0f + 0.15f * pulse),
                        Color{glowCol.r, glowCol.g, glowCol.b, (unsigned char)glowAlpha});
            DrawEllipseLines((int)tPos.x, (int)tPos.y, rw, rh,
                            Color{glowCol.r, glowCol.g, glowCol.b, (unsigned char)(180 + 75 * pulse)});

            // Pulsing arrow pointing down at the target
            float arrowY = tPos.y - 40.0f - 8.0f * pulse;
            float arrowX = tPos.x;
            float arrowSize = 10.0f + 3.0f * pulse;
            DrawTriangle(
                Vector2{arrowX, arrowY + arrowSize},
                Vector2{arrowX - arrowSize, arrowY},
                Vector2{arrowX + arrowSize, arrowY},
                Color{glowCol.r, glowCol.g, glowCol.b, (unsigned char)(200 + 55 * pulse)}
            );
            // Arrow stem
            DrawRectangle((int)(arrowX - 2.0f), (int)(arrowY - 12.0f), 4, 14,
                         Color{glowCol.r, glowCol.g, glowCol.b, (unsigned char)(160 + 60 * pulse)});
        }
    }

    // 10d. Progress tracker sidebar (right side, above toolbar) - Modern Glassmorphism
    if (state == STATE_PLAYING && !isPaused) {
        int screenW = GetScreenWidth();
        int sidebarW = 175;
        int sidebarX = screenW - sidebarW - 14;
        int sidebarY = 60;
        int rowH = 26;
        int panelH = rowH * 5 + 60;

        // Drop shadow
        ui.DrawShadowRect(sidebarX - 8, sidebarY - 6, sidebarW + 16, panelH, 0.1f, Color{0, 0, 0, 80});
        // Glass panel
        ui.DrawGlassPanelBorder(sidebarX - 8, sidebarY - 6, sidebarW + 16, panelH,
                                Color{8, 14, 28, 210}, Color{56, 189, 248, 60}, 0.1f);

        // Title with icon + animated accent line
        DrawGameBoldText("OBJECTIVES", sidebarX + 2, sidebarY - 2, 12, Color{56, 189, 248, 255});
        float titlePulse = 0.4f + 0.6f * sinf(GetTime() * 2.0f);
        DrawCircle(sidebarX + sidebarW - 4, sidebarY + 4, 3, Color{56, 189, 248, (unsigned char)(100 + 80 * titlePulse)});
        // Accent underline
        DrawRectangleGradientH(sidebarX + 2, sidebarY + 12, sidebarW - 12, 2,
                               Color{56, 189, 248, 120}, Color{56, 189, 248, 20});
        sidebarY += 20;

        // Milestones: show next 5 unachieved with colored bars
        int milestones[] = {25, 50, 100, 250, 500, 1000, 2500, 5000, 7500, 10000};
        Color barColors[] = {
            Color{56, 189, 248, 255},   // cyan
            Color{34, 197, 94, 255},    // green
            Color{250, 204, 21, 255},   // yellow
            Color{249, 115, 22, 255},   // orange
            Color{239, 68, 68, 255},    // red
        };
        int shown = 0;
        for (int i = 0; i < 9 && shown < 5; i++) {
            if (economy.totalDelivered >= milestones[i]) continue;
            float pct = (float)economy.totalDelivered / (float)milestones[i] * 100.0f;
            pct = std::min(100.0f, pct);

            // Milestone label
            const char* icon = (shown == 0) ? ">" : " ";
            Color labelCol = (shown == 0) ? Color{230, 235, 245, 255} : Color{160, 170, 190, 200};
            DrawText(TextFormat("%s %d commuters", icon, milestones[i]), sidebarX + 2, sidebarY, 10, labelCol);

            // Progress bar (glass background + gradient fill)
            int barX = sidebarX + 2;
            int barY = sidebarY + 13;
            int barW = sidebarW - 14;
            int barH = 7;
            // Glass bar background
            DrawRectangleRounded(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.4f, 3,
                                 Color{20, 30, 48, 200});
            int fillW = (int)((pct / 100.0f) * barW);
            Color fillCol = barColors[std::min(4, shown)];
            if (fillW > 0) {
                // Gradient fill bar
                DrawRectangleGradientH(barX, barY, fillW, barH,
                                       Color{(unsigned char)(fillCol.r * 7 / 10), (unsigned char)(fillCol.g * 7 / 10), (unsigned char)(fillCol.b * 7 / 10), fillCol.a},
                                       fillCol);
                // Top shine
                DrawRectangleGradientH(barX, barY, fillW, 2,
                                       Color{255, 255, 255, 45}, Color{255, 255, 255, 15});
                // Glow at fill end
                if (fillW > 4) {
                    DrawCircleGradient(Vector2{(float)(barX + fillW), (float)(barY + barH / 2)}, 5,
                                       Color{fillCol.r, fillCol.g, fillCol.b, 60}, Color{fillCol.r, fillCol.g, fillCol.b, 0});
                }
            }
            // Percentage text
            DrawText(TextFormat("%.0f%%", pct), barX + barW + 2, barY - 1, 8, Color{160, 170, 190, 160});

            sidebarY += rowH;
            shown++;
        }

        // Rank progress to next tier
        int currentRank = GetTransitRankTier();
        int nextThresholds[] = {25, 50, 100, 250, 500, 1000, 2500, 5000, 7500, 99999};
        if (currentRank < 9) {
            int nextReq = nextThresholds[currentRank + 1];
            float rankPct = (float)economy.totalDelivered / (float)nextReq * 100.0f;
            rankPct = std::min(100.0f, rankPct);
            sidebarY += 6;
            // Rank label with accent
            DrawGameBoldText("NEXT RANK", sidebarX + 2, sidebarY, 10, Color{234, 179, 8, 255});
            sidebarY += 16;
            int barX = sidebarX + 2;
            int barY = sidebarY;
            int barW = sidebarW - 14;
            int barH = 8;
            DrawRectangleRounded(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.4f, 3,
                                 Color{20, 30, 48, 200});
            int fillW = (int)((rankPct / 100.0f) * barW);
            if (fillW > 0) {
                DrawRectangleGradientH(barX, barY, fillW, barH,
                                       Color{180, 130, 10, 255}, Color{234, 179, 8, 255});
                DrawRectangleGradientH(barX, barY, fillW, 2,
                                       Color{255, 255, 255, 50}, Color{255, 255, 255, 15});
            }
            DrawText(TextFormat("%d / %d", economy.totalDelivered, nextReq), sidebarX + 2, barY + 11, 9, Color{160, 170, 190, 180});
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

    // 11b. Day/night ambient tint overlay
    if (state == STATE_PLAYING) {
        int tintR = 255 - ambientTint.r;
        int tintG = 255 - ambientTint.g;
        int tintB = 255 - ambientTint.b;
        int tintA = (tintR + tintG + tintB) / 3;
        if (tintA > 5) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          Color{(unsigned char)tintR, (unsigned char)tintG, (unsigned char)tintB, (unsigned char)std::min(tintA, 180)});
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
        rushCombo,
        GetTransitRank(),
        GetRankColor()
    );

    // 12a. Persistent arcade OBJECTIVE chip - compact goal tracking
    if (state == STATE_PLAYING) {
        if (endlessMode) {
            int m = ((lastMilestoneAwarded / 500) + 1) * 500;
            float pct = (float)(economy.totalDelivered - (m - 250)) / 250.0f;
            ui.DrawObjectiveChip("ENDLESS METROPOLIS", TextFormat("Next subsidy grant at %d riders", m), pct);
        } else {
            int left = std::max(0, WIN_GOAL - economy.totalDelivered);
            ui.DrawObjectiveChip("VICTORY GOAL", TextFormat("Deliver %d more riders (%d / %d)", left, economy.totalDelivered, WIN_GOAL), (float)economy.totalDelivered / (float)WIN_GOAL);
        }

        // Fleet utilization mini-bar below objective chip
        int totalCap = train.GetMaxCapacity();
        int totalOn = train.GetTotalPassengers();
        for (const auto& et : extraTrains) {
            totalCap += et.GetMaxCapacity();
            totalOn += et.GetTotalPassengers();
        }
        float utilPct = totalCap > 0 ? (float)totalOn / (float)totalCap : 0.0f;
        Color utilCol = (utilPct >= 0.8f) ? Color{239, 68, 68, 255} : (utilPct >= 0.5f) ? Color{234, 179, 8, 255} : Color{56, 189, 248, 255};
        int fBarX = 12, fBarY = 124, fBarW = 260;
        // Glass fleet bar background
        ui.DrawGlassPanel(fBarX - 2, fBarY - 2, fBarW + 4, 42, Color{8, 14, 28, 210}, 0.15f);
        DrawRectangleRounded(Rectangle{(float)(fBarX + 4), (float)(fBarY + 5), (float)(fBarW - 8), 4.0f}, 0.4f, 3, Color{20, 30, 48, 200});
        DrawRectangleGradientH(fBarX + 4, fBarY + 5, (int)((fBarW - 8) * utilPct), 4,
                               Color{(unsigned char)(utilCol.r * 7 / 10), (unsigned char)(utilCol.g * 7 / 10), (unsigned char)(utilCol.b * 7 / 10), utilCol.a}, utilCol);
        DrawText(TextFormat("FLEET: %d/%d seats (%.0f%%)", totalOn, totalCap, utilPct * 100.0f), fBarX + 6, fBarY + 12, 11, Color{203, 213, 225, 255});

        // 12b. Population, Train Tier, Research, Districts display
        static const char* popTierNames[] = {"Village", "Town", "City", "Metropolis", "MEGACITY"};
        Color popCol = (populationTier >= 4) ? Color{255, 214, 0, 255} :
                       (populationTier >= 3) ? Color{168, 85, 247, 255} :
                       (populationTier >= 2) ? Color{56, 189, 248, 255} :
                       (populationTier >= 1) ? Color{52, 211, 153, 255} :
                       Color{148, 163, 184, 255};
        // Time of day icon
        const char* timeIcon = (timeOfDay < 0.2f || timeOfDay >= 0.8f) ? "[NIGHT]" :
                               (timeOfDay < 0.3f) ? "[DAWN]" :
                               (timeOfDay < 0.7f) ? "[DAY]" : "[DUSK]";
        DrawText(TextFormat("POP %d (%s) | T%d %s | %s",
                            totalWorldPopulation, popTierNames[populationTier],
                            trainTier + 1, GetTrainTier(trainTier).name,
                            timeIcon), fBarX + 6, fBarY + 26, 10, popCol);
    }

    // 12c. Natural event banner (top-right corner)
    if (activeEvent >= 0) {
        const auto& ev = EVENTS[activeEvent];
        float ep = 0.5f + 0.5f * sinf(GetTime() * 4.0f);
        int ew = 300, eh = 36;
        int ex = (int)GetScreenWidth() - ew - 16;
        int ey = 66;
        DrawRectangleRounded(Rectangle{(float)ex + 2, (float)ey + 2, (float)ew, (float)eh}, 0.4f, 4, Color{0, 0, 0, 50});
        DrawRectangleRounded(Rectangle{(float)ex, (float)ey, (float)ew, (float)eh}, 0.4f, 4,
                            Color{ev.color.r, ev.color.g, ev.color.b, (unsigned char)(180 + 40 * ep)});
        DrawRectangleRoundedLines(Rectangle{(float)ex, (float)ey, (float)ew, (float)eh}, 0.4f, 4,
                                  Color{255, 255, 255, (unsigned char)(120 + 80 * ep)});
        DrawGameBoldText(TextFormat("%s (%.0fs)", ev.name, eventTimer),
                         (float)ex + 12, (float)ey + 8, 13, Color{255, 255, 255, 255});
    }

    // 12d. Arcade RUSH HOUR banner (dramatic pulsing)
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
        // Remaining time bar (uses same dynamic scaling as rush hour logic)
        float progressPctUI = std::min(1.0f, (float)economy.totalDelivered / (float)WIN_GOAL);
        float rushCycleLenUI = 14.0f - progressPctUI * 4.0f;
        float rushDutyLenUI = 7.0f + progressPctUI * 2.0f;
        float rushRemaining = rushDutyLenUI - fmodf(weekTimer, rushCycleLenUI);
        float rushPct = std::max(0.0f, rushRemaining / rushDutyLenUI);
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

    // 12a+. Rush hour countdown telegraph (5s warning)
    if (rushHourCountdown > 0.0f && !rushHourActive) {
        float cp = 0.5f + 0.5f * sinf(GetTime() * 5.0f);
        int cw = 340, ch = 32;
        int cx = ((int)GetScreenWidth() - cw) / 2;
        int cy = 98;
        DrawRectangleRounded(Rectangle{(float)cx + 1, (float)cy + 2, (float)cw, (float)ch}, 0.4f, 4, Color{0, 0, 0, 40});
        DrawRectangleRounded(Rectangle{(float)cx, (float)cy, (float)cw, (float)ch}, 0.4f, 4,
                            Color{180, 50, 50, (unsigned char)(140 + 60 * cp)});
        DrawRectangleRoundedLines(Rectangle{(float)cx, (float)cy, (float)cw, (float)ch}, 0.4f, 4,
                                  Color{255, 100, 100, (unsigned char)(150 + 100 * cp)});
        DrawGameBoldTextCentered(TextFormat("INCOMING: RUSH HOUR in %.0fs", rushHourCountdown),
                                 (float)GetScreenWidth() / 2.0f, (float)cy + 6, 13,
                                 Color{255, 200, 200, (unsigned char)(200 + 55 * cp)});
    }

    // 12a++. Circuit closed green celebration flash
    if (circuitFlashTimer > 0.0f) {
        unsigned char cfAlpha = (unsigned char)(60 * (circuitFlashTimer / 0.8f));
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        DrawRectangle(0, 0, sw, 10, Color{34, 197, 94, cfAlpha});
        DrawRectangle(0, sh - 10, sw, 10, Color{34, 197, 94, cfAlpha});
        DrawRectangle(0, 0, 10, sh, Color{34, 197, 94, cfAlpha});
        DrawRectangle(sw - 10, 0, 10, sh, Color{34, 197, 94, cfAlpha});
    }

    // 12a++. Train tier-up golden flash
    if (tierUpFlashTimer > 0.0f) {
        unsigned char tfAlpha = (unsigned char)(80 * (tierUpFlashTimer / 2.0f));
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        Color tierCol = GetTrainTier(trainTier).color;
        tierCol.a = tfAlpha;
        DrawRectangle(0, 0, sw, 6, tierCol);
        DrawRectangle(0, sh - 6, sw, 6, tierCol);
        DrawRectangle(0, 0, 6, sh, tierCol);
        DrawRectangle(sw - 6, 0, 6, sh, tierCol);
    }

    // 12b. Draw Contextual Quick Tip Banner
    {
        std::string quickTip;
        if (activeEvent >= 0) {
            quickTip = TextFormat("[EVENT] %s: %s", EVENTS[activeEvent].name, EVENTS[activeEvent].desc);
        } else if (isBulldozing) {
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

    // 13. Draw Categorized Toolbar (with auto-hide fade)
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
    // Toolbar fade when idle
    if (toolbarAlpha < 0.95f) {
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        unsigned char fadeA = (unsigned char)((1.0f - toolbarAlpha) * 200);
        DrawRectangle(0, sh - 180, sw, 180, Color{15, 23, 42, fadeA});
        // "Hover to activate" hint
        if (toolbarAlpha < 0.6f) {
            DrawGameBoldTextCentered("[ Hover toolbar or press 1-8 to build ]",
                                     (float)sw / 2.0f, (float)sh - 100, 11,
                                     Color{148, 163, 184, (unsigned char)(100 * toolbarAlpha)});
        }
    }

    // 13b. First-action spotlight: pulsing glow on Straight track button when no circuit
    if (state == STATE_PLAYING && !isPaused && !tracks.IsCircuitClosed() && economy.totalDelivered == 0) {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        int barX = ToolbarMetrics::BarX(screenW);
        int barY = ToolbarMetrics::BarY(screenH);
        // Straight button is the first item (position 0)
        int itemX = barX + 12;
        int itemY = barY + ToolbarMetrics::ITEM_Y;
        int itemW = ToolbarMetrics::ITEM_W;
        int itemH = ToolbarMetrics::ITEM_H;
        float spotlightPulse = 0.4f + 0.6f * sinf(GetTime() * 3.0f);
        // Glow ring around the Straight button
        DrawRectangleRounded(Rectangle{(float)itemX - 4, (float)itemY - 4, (float)itemW + 8, (float)itemH + 8},
                           0.15f, 6, Color{56, 189, 248, (unsigned char)(40 * spotlightPulse)});
        DrawRectangleRoundedLines(Rectangle{(float)itemX - 2, (float)itemY - 2, (float)itemW + 4, (float)itemH + 4},
                                0.15f, 6, Color{56, 189, 248, (unsigned char)(150 * spotlightPulse)});
        // "START HERE" text above
        const char* startHint = "START HERE";
        int hintW = MeasureText(startHint, 10);
        int hintX = itemX + (itemW - hintW) / 2;
        DrawText(startHint, hintX, itemY - 16, 10, Color{56, 189, 248, (unsigned char)(200 * spotlightPulse)});
    }

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
        int stars = 1 + ((economy.totalDelivered >= WIN_GOAL) ? 1 : 0) + ((week <= 12) ? 1 : 0);
        ui.DrawGameOver(economy.totalDelivered, stars, bestSessionRiders, stateEntryTime);
    } else if (state == STATE_VICTORY) {
        int stars = 1 + ((economy.totalDelivered >= WIN_GOAL) ? 1 : 0) && ((week <= 12) ? 1 : 0);
        ui.DrawVictory(economy.totalDelivered, week, stars, economy.balance, bestSessionRiders, stateEntryTime);
    } else if (state == STATE_PLAYING && (gameSpeed == 0 || isPaused)) {
        ui.DrawPauseOverlay(researchTier);
    }

    // 20. Smart Advisor suggestion card
    if (state == STATE_PLAYING && !isPaused && advisorShowTime > 0.0f) {
        // Re-derive the current suggestion for display
        float t = GetTime();
        float age = t - advisorShowTime;
        if (age < 8.5f) {
            // Determine the current advisor message based on priority
            const char* aIcon = "!";
            const char* aTitle = "BUILD A LOOP";
            const char* aHint = "Place track tiles to connect stations into a closed circuit.";
            Color aCol = Color{239, 68, 68, 255};

            if (!tracks.IsCircuitClosed()) {
                aIcon = "!"; aTitle = "BUILD A LOOP";
                aHint = "Place track tiles to connect stations into a closed circuit.";
                aCol = Color{239, 68, 68, 255};
            } else if (tracks.GetStationCount() < 2 && economy.totalDelivered < 25) {
                aIcon = "7"; aTitle = "ADD MORE STATIONS";
                aHint = "Press [7] to place stations. Commuters need places to board!";
                aCol = Color{249, 115, 22, 255};
            } else if (economy.balance < 100.0f && economy.totalDelivered > 10) {
                aIcon = "$"; aTitle = "LOW ON CASH";
                aHint = "Build more stations to earn fare revenue. Each ride earns money!";
                aCol = Color{234, 179, 8, 255};
            } else if (peeps.IsOvercrowded()) {
                aIcon = "~"; aTitle = "PLATFORM OVERCROWDED";
                aHint = "Add more stations or buy extra trains to handle demand!";
                aCol = Color{239, 68, 68, 255};
            } else if (economy.balance > 2000.0f && extraTrains.empty() && economy.totalDelivered > 50) {
                aIcon = "T"; aTitle = "BUY AN EXTRA TRAIN";
                aHint = "Click EXTRA TRAIN ($1200) on the right to boost capacity!";
                aCol = Color{56, 189, 248, 255};
            } else if (economy.balance > 800.0f && economy.totalDelivered > 100 && buildRadius < 42) {
                aIcon = "+"; aTitle = "EXPAND TERRITORY";
                aHint = "Click EXPAND ($500) to unlock new buildable land!";
                aCol = Color{52, 211, 153, 255};
            } else if (parkRating < 60.0f && economy.totalDelivered > 30) {
                aIcon = "*"; aTitle = "RATING IS DROPPING";
                aHint = "Add trees, benches, or fountains to boost commuter satisfaction!";
                aCol = Color{168, 85, 247, 255};
            } else if (economy.balance > 500.0f && economy.totalDelivered > 200) {
                aIcon = "^"; aTitle = "UPGRADE STATIONS";
                aHint = "Click a station to upgrade it. Higher level = more fare revenue!";
                aCol = Color{16, 185, 129, 255};
            } else if (economy.totalDelivered > 300 && tracks.GetStationCount() < 4) {
                aIcon = "#"; aTitle = "GROW YOUR NETWORK";
                aHint = "Add more stations and connect them for a bigger transit system!";
                aCol = Color{56, 189, 248, 255};
            } else if (parkRating >= 60.0f && parkRating < 75.0f && economy.totalDelivered > 150 && economy.balance > 300.0f) {
                aIcon = "*"; aTitle = "ADD SCENERY";
                aHint = "Place trees, benches, or fountains near stations to boost satisfaction!";
                aCol = Color{168, 85, 247, 255};
            } else if (economy.totalDelivered > 500 && tracks.GetStationCount() >= 4) {
                aIcon = "~"; aTitle = "OPTIMIZE YOUR NETWORK";
                aHint = "Upgrade stations and add scenery to maximize your transit empire!";
                aCol = Color{34, 197, 94, 255};
            }

            ui.DrawAdvisorSuggestion(aIcon, aTitle, aHint, aCol, advisorShowTime);
        }
    }

    // 21. Achievement popup (big animated badge for milestones)
    if (state == STATE_PLAYING && achievementShowTime > 0.0f) {
        ui.DrawAchievementPopup(achievementTitle, achievementSub, achievementColor, achievementShowTime);
    }

    EndDrawing();
}

bool Game::ShouldClose() const {
    return WindowShouldClose();
}

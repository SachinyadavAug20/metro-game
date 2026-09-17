#pragma once

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <memory>
#include <iostream>

// Isometric Grid Parameters (2:1 Ratio - Classic Dimetric Sawyer Style)
constexpr int TILE_WIDTH = 64;
constexpr int TILE_HEIGHT = 32;
constexpr int HEIGHT_STEP = 24; // Pixel rise per elevation level (Z)
constexpr int GRID_SIZE = 64;   // 64x64 expanded metropolitan grid
constexpr int WIN_GOAL = 10000;  // Deliver 10,000 commuters - true sandbox endgame

// Biome Types (Terraria/Minecraft-inspired world variety)
enum BiomeType {
    BIOME_URBAN = 0,      // Default city - grass tiles
    BIOME_WATERFRONT,     // Coastal - sand + water edges
    BIOME_PARKLAND,       // Green belt - dense trees, flowers
    BIOME_INDUSTRIAL,     // Factory zone - stone, darker tones
    BIOME_DOWNTOWN,       // High density - plaza, tall buildings
    BIOME_SUBURBAN,       // Residential - houses, quiet
    BIOME_COUNT
};

inline const char* GetBiomeName(BiomeType b) {
    switch (b) {
        case BIOME_URBAN:      return "Urban Core";
        case BIOME_WATERFRONT: return "Waterfront";
        case BIOME_PARKLAND:   return "Green Belt";
        case BIOME_INDUSTRIAL: return "Industrial";
        case BIOME_DOWNTOWN:   return "Downtown";
        case BIOME_SUBURBAN:   return "Suburbs";
        default:               return "Unknown";
    }
}

// Research Tree Tiers (Factorio-inspired progression)
enum ResearchTier {
    RESEARCH_BASIC = 0,       // Unlocked at start
    RESEARCH_CIVIC,           // 100 riders - civic buildings
    RESEARCH_COMMERCIAL,      // 300 riders - shops, offices
    RESEARCH_INDUSTRIAL,      // 750 riders - factories, warehouses
    RESEARCH_HIGHTECH,        // 1500 riders - tech campus, labs
    RESEARCH_URBAN,           // 3000 riders - urban planning, density
    RESEARCH_BIOTECH,         // 5000 riders - biotech, green energy
    RESEARCH_MEGAPROJECT,     // 8000 riders - mega structures
    RESEARCH_COUNT
};

inline const char* GetResearchName(ResearchTier t) {
    switch (t) {
        case RESEARCH_BASIC:       return "Basic Transit";
        case RESEARCH_CIVIC:       return "Civic Infrastructure";
        case RESEARCH_COMMERCIAL:  return "Commercial District";
        case RESEARCH_INDUSTRIAL:  return "Industrial Zone";
        case RESEARCH_HIGHTECH:    return "High-Tech Campus";
        case RESEARCH_URBAN:       return "Urban Planning";
        case RESEARCH_BIOTECH:     return "Biotech & Green";
        case RESEARCH_MEGAPROJECT: return "Mega Project";
        default:                   return "Unknown";
    }
}

// World Districts (auto-generated zones that unlock with population)
struct WorldDistrict {
    int centerX, centerY;
    int radius;
    BiomeType biome;
    int population;        // grows over time
    bool unlocked;
    float spawnTimer;
};

// Directions
enum Direction {
    DIR_NORTH = 0, // -Y (Upper Right in Iso)
    DIR_EAST  = 1, // +X (Lower Right in Iso)
    DIR_SOUTH = 2, // +Y (Lower Left in Iso)
    DIR_WEST  = 3  // -X (Upper Left in Iso)
};

inline Direction GetOppositeDir(Direction dir) {
    return static_cast<Direction>((dir + 2) % 4);
}

inline Vector2 GetDirectionOffset(Direction dir) {
    switch (dir) {
        case DIR_NORTH: return {0.0f, -1.0f};
        case DIR_EAST:  return {1.0f, 0.0f};
        case DIR_SOUTH: return {0.0f, 1.0f};
        case DIR_WEST:  return {-1.0f, 0.0f};
    }
    return {0.0f, 0.0f};
}

// Mini Metro Inspired Destination Station Shapes
enum StationShape {
    SHAPE_NONE = 0,
    SHAPE_CIRCLE = 1,   // Residential Suburbs / Bedroom Community (Blue)
    SHAPE_TRIANGLE = 2, // Commercial Hub / Retail & Entertainment (Orange)
    SHAPE_SQUARE = 3,   // Financial District / Central Business District (Teal)
    SHAPE_CROSS = 4     // University / Medical Center / Hospital (Red)
};

inline const char* GetShapeName(StationShape shape) {
    switch (shape) {
        case SHAPE_CIRCLE:   return "Residential (Circle)";
        case SHAPE_TRIANGLE: return "Commercial (Triangle)";
        case SHAPE_SQUARE:   return "Financial CBD (Square)";
        case SHAPE_CROSS:    return "Medical/University (Cross)";
        default:             return "Standard Station";
    }
}

inline Color GetShapeColor(StationShape shape) {
    switch (shape) {
        case SHAPE_CIRCLE:   return Color{59, 130, 246, 255};  // Vibrant Blue
        case SHAPE_TRIANGLE: return Color{249, 115, 22, 255};  // Vibrant Orange
        case SHAPE_SQUARE:   return Color{16, 185, 129, 255};  // Emerald Teal
        case SHAPE_CROSS:    return Color{239, 68, 68, 255};   // Crimson Red
        default:             return Color{148, 163, 184, 255};
    }
}

// Wayside Signaling System (3-Aspect ABS)
enum SignalAspect {
    SIGNAL_GREEN = 0,  // Clear - Proceed at track speed (60-80 km/h)
    SIGNAL_AMBER = 1,  // Caution - Next block occupied / decelerate to 35 km/h
    SIGNAL_RED   = 2   // Danger - Stop immediately at signal mast
};

// Transit Track & Infrastructure Types
enum TrackType {
    TRACK_NONE = 0,
    TRACK_STRAIGHT,        // Surface rail: Dual steel rails on concrete ties + 3rd rail
    TRACK_CURVE_RIGHT,     // 90 degree clockwise turn
    TRACK_CURVE_LEFT,      // 90 degree counter-clockwise turn
    TRACK_VIADUCT_ELEVATED,// Elevated concrete SkyTrain viaduct (+1 Z)
    TRACK_VIADUCT_SLOPE,   // Viaduct ramp descending from elevated to surface (-1 Z)
    TRACK_TUNNEL_PORTAL,   // Arched concrete subway tunnel portal into underground
    TRACK_TUNNEL,          // Subterranean subway tunnel: arched concrete tube with utility lighting
    TRACK_STATION,         // Station Platform: Island platform with glass canopy, LED PIDS, PSDs
    TRACK_SIGNAL           // Track segment equipped with 3-aspect wayside signaling mast
};

// Ground / Pavement / Terraforming Types (Minecraft / Terraria / RCT Style)
enum GroundType {
    GROUND_GRASS = 0,      // Lush emerald lawn / parkland
    GROUND_DIRT,           // Fertile loam
    GROUND_WATER,          // Urban canal / river / ocean
    GROUND_PATH,           // Urban pedestrian sidewalk
    GROUND_QUEUE,          // Platform queuing tactile safety zone
    GROUND_PLAZA,          // Modern granite transit plaza
    GROUND_SAND,           // Tropical coastal beach sand & dunes
    GROUND_STONE           // Mountain granite rock / chiseled cobblestone
};

// Urban & Transit Scenery Types
enum SceneryType {
    SCENERY_NONE = 0,
    SCENERY_METRO_ENTRANCE,// Subway stairs descending underground with glowing roundel totem
    SCENERY_TURNSTILE_GATE,// TVM ticket vending machine & contactless fare gates
    SCENERY_MAP_KIOSK,     // Harry Beck style transit system map board
    SCENERY_STREET_TREE,   // Manicured urban maple/ginkgo shade tree
    SCENERY_PINE_TREE,     // Conifer park tree
    SCENERY_BENCH,         // Modern stainless steel & wood subway bench
    SCENERY_LAMP_POST,     // High-efficiency LED urban streetlamp
    SCENERY_NEWSSTAND,     // Platform coffee / newspaper kiosk
    SCENERY_BIKE_RACK,     // Metro bike share docking station
    SCENERY_FOUNTAIN,      // Splashing park water fountain with animated sprays
    SCENERY_FLOWER_BED,    // Vibrant multi-color botanical blossoms
    SCENERY_VENT_GRATE,    // Subway ventilation sidewalk iron grate with rising steam wisps
    SCENERY_PALM_TREE      // Coastal tropical palm tree with swaying fronds
};

// Tool Categories
enum ToolCategory {
    CAT_TRACK = 0,
    CAT_INFRA,
    CAT_SCENERY
};

// Commuter Archetypes
enum CommuterType {
    COMMUTER_WORKER = 0,   // Daily CBD commuter with briefcase
    COMMUTER_STUDENT,      // College student with backpack
    COMMUTER_TOURIST,      // Sightseer with camera and map
    COMMUTER_EXECUTIVE     // High-profile traveler heading to financial core
};

// Commuter States
enum CommuterState {
    COMMUTER_ENTERING = 0,
    COMMUTER_SWIPING_GATE,
    COMMUTER_WALKING_TO_PLATFORM,
    COMMUTER_ON_PLATFORM,
    COMMUTER_RIDING,
    COMMUTER_ALIGHTING,
    COMMUTER_EXITING_STATION,
    COMMUTER_LEAVING_ANGRY
};

// Metro EMU Train States
enum TrainState {
    TRAIN_STOPPED_IN_STATION = 0,
    TRAIN_BOARDING,
    TRAIN_DOOR_CLOSING,
    TRAIN_ACCELERATING,
    TRAIN_CRUISING,
    TRAIN_APPROACHING_STATION,
    TRAIN_BRAKING,
    TRAIN_SIGNAL_STOP
};

// Game Mode / State
enum GameState {
    STATE_TITLE = 0,
    STATE_PLAYING,
    STATE_WEEKLY_UPGRADE,
    STATE_GAME_OVER,
    STATE_VICTORY
};

// Toast Notifications
struct ToastMessage {
    std::string text;
    Color color;
    float timer = 0.0f;
};

// Metro Line Operating Status (RCT 3-Aspect: Open, Testing, Closed)
enum LineOperatingMode {
    LINE_OPEN = 0,   // Full passenger service: boarding and revenue active
    LINE_TEST = 1,   // Test run: train runs empty to test loops & track
    LINE_CLOSED = 2  // Service suspended: train stopped at station
};

// Metro Line Telemetry & Efficiency Ratings (replacing CoasterStats)
struct MetroLineStats {
    std::string lineName = "Line 1 - Central Loop";
    Color themeColor = Color{229, 57, 53, 255}; // Tokyo Red Line
    LineOperatingMode mode = LINE_OPEN;
    float maxSpeedKmh = 72.0f;
    float currentSpeedKmh = 0.0f;
    float trackLengthM = 0.0f;
    int stationCount = 1;
    int fleetCars = 3;
    float onTimeRate = 98.4f;        // Punctuality %
    float commuterSatisfaction = 94.0f; // Commuter happiness %
    int totalRiders = 0;
    float totalRevenue = 0.0f;
    float ticketFare = 2.50f;        // Standard metro fare ($2.50)
    SignalAspect currentSignal = SIGNAL_GREEN;
    float excitementRating = 7.8f;   // RCT Excitement (0.0 to 10.0)
    float intensityRating = 5.2f;    // RCT Intensity (0.0 to 10.0)
    float parkValue = 18500.0f;      // RCT Total Transit Park Value ($)
};

// Transit Authority Economy
struct TransitEconomy {
    float balance = 2500.0f;
    float hourlyIncome = 0.0f;
    float baseFare = 2.50f;
    int totalDelivered = 0;
    int dailyPeakRidership = 0;
};

// Transit Authority Staff
enum StaffType {
    STAFF_CUSTODIAN = 0,   // Station attendant sweeping platforms (Blue transit uniform)
    STAFF_ENGINEER         // Signal & track engineer inspecting switches (Orange high-vis)
};

struct StaffMember {
    std::string name;
    StaffType type;
    Vector2 pos;
    Vector2 targetPos;
    float walkTimer = 0.0f;
    float workTimer = 0.0f;
    bool isWorking = false;
};

// Platform Litter / Maintenance
struct StationMess {
    Vector2 pos;
    bool isSpill; // true = spilled coffee, false = dropped newspaper/receipt
    float timer = 0.0f;
};

// Train Upgrade Tiers (Metro EMU progression)
struct TrainTier {
    const char* name;
    const char* desc;
    int carCount;
    int capacityPerCar;
    float speedMult;   // multiplied by base speed
    Color color;
    int deliveryBonus; // extra commuters per delivery
};

inline const TrainTier& GetTrainTier(int tier) {
    static const TrainTier tiers[] = {
        {"Commuter Rail",  "3-car starter set",          3, 6,  1.0f, {148,163,184,255}, 0},  // tier 0
        {"Express EMU",    "Faster accel, more seats",  3, 8,  1.2f, {56,189,248,255},  0},  // tier 1
        {"Rapid Transit",  "4-car high-density set",    4, 10, 1.4f, {52,211,153,255},  1},  // tier 2
        {"High-Speed Rail","5-car intercity express",   5, 12, 1.7f, {168,85,247,255},  2},  // tier 3
        {"Maglev Express", "6-car levitation marvel",   6, 15, 2.0f, {255,214,0,255},   3},  // tier 4
    };
    return tiers[std::clamp(tier, 0, 4)];
}

// Weekly Upgrade IDs (deterministic matching)
enum UpgradeID {
    UPGRADE_4CAR_EMU = 0,
    UPGRADE_CBTC_SIGNALING,
    UPGRADE_TRANSIT_SUBSIDY,
    UPGRADE_EXPRESS_LINE,
    UPGRADE_FLEET_GRANT,
    UPGRADE_TOURIST_MARKETING,
    UPGRADE_PLATFORM_EXPANSION,
    UPGRADE_RUSH_HOUR_BONUS,
    UPGRADE_TRAIN_TIER_UP,
    UPGRADE_ORGANIC_GROWTH,
    UPGRADE_INFRA_BUDGET,
    UPGRADE_PREMIUM_CARS,      // +30% fare from all passengers
    UPGRADE_STATION_WIFI,      // +20% rider satisfaction, slower decay
    UPGRADE_NIGHT_OPS,         // trains run at night too, +50% night revenue
    UPGRADE_MEGA_HUB,          // one station becomes a mega-hub, +100% throughput
    UPGRADE_COUNT
};

// Line color palette (metro map colors for visual route distinction)
struct LineColor {
    Color primary;
    Color dark;
    const char* name;
};
static const LineColor LINE_COLORS[] = {
    {{30, 64, 175, 255},  {15, 32, 88, 255}, "Blue Line"},
    {{220, 38, 38, 255},  {120, 20, 20, 255}, "Red Line"},
    {{22, 163, 74, 255},  {11, 82, 37, 255}, "Green Line"},
    {{217, 119, 6, 255},  {109, 60, 3, 255}, "Orange Line"},
    {{147, 51, 234, 255}, {74, 26, 117, 255}, "Purple Line"},
    {{236, 72, 153, 255}, {118, 36, 77, 255}, "Pink Line"},
    {{14, 165, 233, 255}, {7, 83, 117, 255}, "Sky Line"},
    {{234, 179, 8, 255},  {117, 90, 4, 255}, "Gold Line"},
};
static const int LINE_COLOR_COUNT = sizeof(LINE_COLORS) / sizeof(LINE_COLORS[0]);


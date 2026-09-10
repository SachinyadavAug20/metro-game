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
constexpr int GRID_SIZE = 26;   // 26x26 metropolitan isometric grid

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
    TRACK_STATION,         // Station Platform: Island platform with glass canopy, LED PIDS, PSDs
    TRACK_SIGNAL           // Track segment equipped with 3-aspect wayside signaling mast
};

// Ground / Pavement Types
enum GroundType {
    GROUND_GRASS = 0,
    GROUND_DIRT,
    GROUND_WATER,          // Urban canal / river
    GROUND_PATH,           // Urban pedestrian sidewalk
    GROUND_QUEUE,          // Platform queuing tactile safety zone
    GROUND_PLAZA           // Modern granite transit plaza
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
    SCENERY_BIKE_RACK      // Metro bike share docking station
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

// Metro Line Telemetry & Efficiency Ratings (replacing CoasterStats)
struct MetroLineStats {
    std::string lineName = "Line 1 - Central Loop";
    Color themeColor = Color{229, 57, 53, 255}; // Tokyo Red Line
    float maxSpeedKmh = 72.0f;
    float currentSpeedKmh = 0.0f;
    float trackLengthM = 0.0f;
    int stationCount = 1;
    int fleetCars = 3;
    float onTimeRate = 98.4f;        // Punctuality %
    float commuterSatisfaction = 94.0f; // Commuter happiness %
    int totalRiders = 0;
    float ticketFare = 2.50f;        // Standard metro fare ($2.50)
    SignalAspect currentSignal = SIGNAL_GREEN;
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


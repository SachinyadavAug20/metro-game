#pragma once

#include "common.hpp"
#include "isometric.hpp"
#include "audio.hpp"
#include "particles.hpp"
#include "track.hpp"
#include "train.hpp"
#include "peep.hpp"
#include "ui.hpp"

class Game {
public:
    Game();
    ~Game();

    void Init();
    void HandleInput();
    void Update(float dt);
    void Draw();

    bool ShouldClose() const;
    void ShowToast(const std::string& text, Color color = Color{34, 197, 94, 255}, float duration = 3.5f);

    // Transit rank system (progression prestige)
    int GetTransitRankTier() const {
        int d = economy.totalDelivered;
        if (d >= WIN_GOAL) return 9;
        if (d >= 7500) return 8;
        if (d >= 5000) return 7;
        if (d >= 2500) return 6;
        if (d >= 1000) return 5;
        if (d >= 500) return 4;
        if (d >= 250) return 3;
        if (d >= 100) return 2;
        if (d >= 25) return 1;
        return 0;
    }
    const char* GetTransitRank() const {
        int d = economy.totalDelivered;
        if (d >= WIN_GOAL) return "TRANSIT LEGEND";
        if (d >= 7500) return "Metro Director";
        if (d >= 5000) return "Chief Engineer";
        if (d >= 2500) return "Line Supervisor";
        if (d >= 1000) return "Station Manager";
        if (d >= 500) return "Senior Operator";
        if (d >= 250) return "Train Driver";
        if (d >= 100) return "Conductor";
        if (d >= 25) return "Platform Guard";
        return "Apprentice";
    }
    Color GetRankColor() const {
        int d = economy.totalDelivered;
        if (d >= WIN_GOAL) return Color{255, 214, 0, 255};  // Gold
        if (d >= 5000) return Color{168, 85, 247, 255};     // Purple
        if (d >= 2500) return Color{56, 189, 248, 255};     // Blue
        if (d >= 1000) return Color{52, 211, 153, 255};     // Green
        if (d >= 100)  return Color{234, 179, 8, 255};      // Yellow
        return Color{148, 163, 184, 255};                    // Gray
    }

    // Public fleet pricing (used by the toolbar shop)
    static int MaxExtraTrains() { return MAX_EXTRA_TRAINS; }
    static float ExtraTrainCostP(int owned) { return EXTRA_TRAIN_BASE_COST + 900.0f * owned; }

protected:
    GameState state = STATE_PLAYING;
    int gameSpeed = 1; // 0 = Pause, 1 = Normal, 2 = Fast

    // Isometric Camera
    Vector2 cameraPos = {0.0f, 0.0f};
    float zoom = 1.0f;
    Vector2 dragStart = {0.0f, 0.0f};
    bool isDragging = false;
    bool rideCamActive = false;

    // Terrain & Scenery
    GroundType terrain[GRID_SIZE][GRID_SIZE];
    int groundZ[GRID_SIZE][GRID_SIZE];
    SceneryType scenery[GRID_SIZE][GRID_SIZE];

    // Systems
    TrackSystem tracks;
    CoasterTrain train;
    PeepManager peeps;
    ParticleSystem particles;
    UserInterface ui;

    // Building tool state
    ToolCategory activeTab = CAT_TRACK;
    TrackType currentTrack = TRACK_STRAIGHT;
    SceneryType currentScenery = SCENERY_PINE_TREE;
    GroundType currentGround = GROUND_PATH;
    Direction buildHeading = DIR_EAST;
    int currentZ = 0;
    bool isBulldozing = false;
    bool isTerraformingRaise = false;
    bool nightMode = false;
    int currentLineId = 1; // Line 1 to 5 (Tokyo Red, London Blue, etc.)
    int hoveredGx = -1;
    int hoveredGy = -1;

    void GetTrackPieceDirs(TrackType type, Direction heading, Direction& outInDir, Direction& outOutDir) const {
        outInDir = GetOppositeDir(heading);
        if (type == TRACK_CURVE_LEFT) {
            outOutDir = (Direction)((heading + 3) % 4); // 90 deg counter-clockwise (L-turn)
        } else if (type == TRACK_CURVE_RIGHT) {
            outOutDir = (Direction)((heading + 1) % 4); // 90 deg clockwise (R-turn)
        } else {
            outOutDir = heading; // Straight ahead
        }
    }

    // UI Panels & Inspection
    bool statsWindowOpen = false;
    bool staffWindowOpen = false;
    bool helpOverlayOpen = false;
    int selectedPeepIdx = -1;
    int selectedStationGx = -1;
    int selectedStationGy = -1;
    ToastMessage activeToast;

    // Staff & Station Maintenance
    std::vector<StaffMember> staff;
    std::vector<StationMess> messes;
    float parkCleanliness = 100.0f;
    float staffWageTimer = 0.0f;

    // Transit Stats, Economy & Progression
    TransitEconomy economy;
    MetroLineStats cachedStats;
    float parkRating = 85.0f;
    int angryLeaves = 0;
    int week = 1;
    float weekTimer = 0.0f;
    const float WEEK_DURATION = 60.0f;
    int lastMilestoneAwarded = 0;
    int lastRankTier = 0;       // 0=Apprentice..9=Legend, for rank-up detection
    float rushCombo = 1.0f;
    float comboTimer = 0.0f;
    int comboStreak = 0;
    bool endlessMode = false;

    // Island land expansion (RCT-style purchasable land)
    static const int LAND_CENTER_X = 14;
    static const int LAND_CENTER_Y = 14;
    static constexpr float TRAIN_CAR_COST = 600.0f;
    static constexpr float LAND_EXPAND_COST = 500.0f;
    static constexpr float EXTRA_TRAIN_BASE_COST = 1200.0f;
    static constexpr int MAX_EXTRA_TRAINS = 6;
    int buildRadius = 36;  // Manhattan ring of developable land
    bool IsBuildable(int gx, int gy) const;
    void ReclaimLand();

    // Purchasable extra EMUs running the same loop (each more expensive)
    std::vector<MetroTrain> extraTrains;
    int GetExtraTrainCount() const { return (int)extraTrains.size(); }

    // Rush-hour logic (weekly peak: doubled spawn demand)
    float rushHourTimer = 0.0f;
    bool rushHourActive = false;
    float rushHourFlashTimer = 0.0f;  // screen-edge red flash when rush hour starts
    float circuitFlashTimer = 0.0f;   // green flash when circuit is first closed

    // Smart Advisor system (contextual guidance)
    float advisorShowTime = 0.0f;     // GetTime() when current suggestion appeared
    int advisorDismissCount = 0;      // number of times player dismissed an advisor
    int lastAdvisorTier = -1;         // last advice shown (avoid repeats)
    int advisorTargetGx = -1;         // grid X of suggested build location
    int advisorTargetGy = -1;         // grid Y of suggested build location
    float advisorPulseTimer = 0.0f;   // pulsing glow timer for target highlight

    // Achievement popup system
    float achievementShowTime = 0.0f; // GetTime() when achievement appeared
    char achievementTitle[64] = {};    // e.g. "FIRST LOOP CLOSED"
    char achievementSub[128] = {};     // e.g. "Your transit network is operational!"
    Color achievementColor = {};       // accent color

    // Train tier system (0-4)
    int trainTier = 0;
    float tierUpFlashTimer = 0.0f;

    // Organic population growth
    float popGrowthTimer = 0.0f;
    float popGrowthRate = 1.0f;    // multiplier from upgrades
    int totalWorldPopulation = 0;   // grows over time
    int populationTier = 0;         // 0=village, 1=town, 2=city, 3=metropolis, 4=megacity

    // Research tree (Factorio-inspired progression)
    int researchTier = 0;           // 0-7, unlocks buildings/features
    float researchProgress = 0.0f;  // progress toward next tier
    bool researchUnlocked[8] = {true, false, false, false, false, false, false, false};

    // Rush hour countdown telegraph
    float rushHourCountdown = 0.0f;  // seconds until rush hour starts

    // Toolbar auto-hide (simplify UI when idle)
    float toolbarIdleTimer = 0.0f;   // seconds since last toolbar interaction
    float toolbarAlpha = 1.0f;       // 0-1, fades toolbar when idle

    // World districts (auto-generated zones)
    std::vector<WorldDistrict> districts;
    float districtSpawnTimer = 0.0f;
    int maxDistricts = 8;

    // Infrastructure spawning
    float infraSpawnTimer = 0.0f;
    bool infraUnlocked = false;

    // Day/night cycle (visual only)
    float timeOfDay = 0.35f;   // 0-1: 0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk
    float daySpeed = 0.008f;   // speed multiplier (1 full cycle per ~2 min)
    Color ambientTint = {255, 255, 255, 255}; // applied as tint to all world rendering

    // Line color coding
    int lineColorIdx = 0;       // index into LINE_COLORS[] for current line
    float lineColorFlash = 0.0f; // flash timer when color changes

    // Natural events system
    struct NaturalEvent {
        const char* name;
        const char* desc;
        float duration;     // seconds
        float riderBoost;   // multiplier on spawn rate (1.0 = normal)
        float fundBoost;    // multiplier on fare income (1.0 = normal)
        Color color;
    };
    static const NaturalEvent EVENTS[];
    static const int EVENT_COUNT = 6;
    int activeEvent = -1;       // index into EVENTS[], -1 = none
    float eventTimer = 0.0f;
    float eventCooldown = 0.0f; // minimum time between events

    // Audio roar / chain click timers
    float chainSoundTimer = 0.0f;

    // Weekly upgrade choices
    std::vector<UpgradeChoice> activeUpgrades;
    int hoveredUpgrade = -1;

    bool isPaused = false;           // arcade ESC hard-pause overlay
    int bestSessionRiders = 0;       // high-score table (this session) for replay loop
    float stateEntryTime = 0.0f;   // GetTime() when current state was entered (for fade-in effects)
    float shakeTimer = 0.0f;       // screen shake decay timer (seconds remaining)
    float shakeIntensity = 0.0f;   // current shake pixel offset

    // Helper functions
    void SetupInitialPark();
    void GenerateWeeklyUpgrades();
    void ApplyUpgrade(int choiceIdx);
    void ResetPark();
    void RecenterCamera();
    void ClampCamera();
    void UpdateAdvisor();
};

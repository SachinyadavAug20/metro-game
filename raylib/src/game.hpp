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
    float rushCombo = 1.0f;
    float comboTimer = 0.0f;
    int comboStreak = 0;
    bool endlessMode = false;

    // Island land expansion (RCT-style purchasable land)
    static const int LAND_CENTER_X = 9;
    static const int LAND_CENTER_Y = 11;
    static constexpr float TRAIN_CAR_COST = 800.0f;
    static constexpr float LAND_EXPAND_COST = 600.0f;
    static constexpr float EXTRA_TRAIN_BASE_COST = 1400.0f;
    static constexpr int MAX_EXTRA_TRAINS = 3;
    int buildRadius = 14;  // Manhattan ring of developable land
    bool IsBuildable(int gx, int gy) const;
    void ReclaimLand();

    // Purchasable extra EMUs running the same loop (each more expensive)
    std::vector<MetroTrain> extraTrains;
    int GetExtraTrainCount() const { return (int)extraTrains.size(); }

    // Rush-hour logic (weekly peak: doubled spawn demand)
    float rushHourTimer = 0.0f;
    bool rushHourActive = false;

    // Audio roar / chain click timers
    float chainSoundTimer = 0.0f;

    // Weekly upgrade choices
    std::vector<UpgradeChoice> activeUpgrades;
    int hoveredUpgrade = -1;

    // Guided tutorial objectives
    bool showObjectivePanel = true;
    bool tutorialDone = false;
    int tutorialDoneCount = 0;
    int piecesPlaced = 0;
    int stationsPlaced = 0;
    int sceneryPlaced = 0;
    bool GetTutorialDone(int idx) const;
    int GetTutorialDoneCount() const;
    int GetCurrentTutorialIdx() const;
    bool GetTutorialStageInfo(int idx, TutorialStageInfo& out) const;

    // Deliberate tutorial tracking counters
    int viaductPiecesPlaced = 0;
    int bulldozeUses = 0;
    bool rideCamUsed = false;
    float lastManualPanTime = 0.0f;  // player panning suspends tutorial camera focus

    // Helper functions
    void SetupInitialPark();
    void GenerateWeeklyUpgrades();
    void ApplyUpgrade(int choiceIdx);
    void ResetPark();
};

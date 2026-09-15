#pragma once

#include "common.hpp"
#include "peep.hpp"

struct TrackNode;

struct UpgradeChoice {
    std::string title;
    std::string description;
    std::string perkTag;
    Color accentColor;
    UpgradeID id;
};

// Shared toolbar geometry - ONE source of truth so the draw code
// and click hit-tests always agree (single place to rescale).
struct ToolbarMetrics {
    static constexpr int BAR_W = 840;
    static constexpr int BAR_H = 134;
    static constexpr int AUX_W = 152;
    static constexpr int GAP = 12;          // gap between main bar and aux pillar
    static constexpr int ITEM_W = 84;
    static constexpr int ITEM_H = 86;       // tall tool buttons for clean large fonts
    static constexpr int TAB_H = 30;
    static constexpr int BUY_H = 24;        // CAR/LAND/EXTRA TRAIN purchase buttons
    static constexpr int BUY_Y = 74;        // purchase row offset from bar top
    static constexpr int ITEM_Y = 14;       // tool button row offset from bar top
    static constexpr int DOCK_H = 14;       // clearances (barY = screenH - BAR_H - DOCK_H)
    static int BarX(int screenW) { return std::max(10, (screenW - (BAR_W + GAP + AUX_W)) / 2); }
    static int BarY(int screenH) { return screenH - BAR_H - DOCK_H; }
    static int AuxX(int screenW)  { return BarX(screenW) + BAR_W + GAP; }
};

class UserInterface {
public:
    UserInterface();

    void Update(Vector2 mousePos, bool mouseClicked);

    // Operations Control Center Main HUD
    void DrawHUD(
        int ridership,
        float satisfaction,
        int week,
        float weekTimer,
        float speedKmh,
        SignalAspect signalAspect,
        bool circuitClosed,
        int gameSpeed,
        bool muted,
        float balance,
        bool lineOpsOpen,
        bool crewOpen,
        bool cabCamActive,
        bool helpOpen,
        float rushCombo = 1.0f,
        const char* rank = "Apprentice",
        Color rankColor = Color{148, 163, 184, 255}
    );

    // Categorized Toolbar (Track, Concourse, Scenery)
    void DrawToolbar(
        ToolCategory activeTab,
        TrackType currentTrack,
        SceneryType currentScenery,
        GroundType currentGround,
        int currentZ,
        Direction currentDir,
        bool isBulldozing,
        float balance,
        int carCount,
        int buildRadius,
        int extraTrainCount,
        bool isTerraformingRaise = false
    );

    // OCC Panels & Modals
    void DrawLineOperations(const MetroLineStats& stats);
    void DrawTransitCrewWindow(const std::vector<StaffMember>& staff, float cleanliness, float balance);
    void DrawCommuterInspector(const Commuter* commuter);
    void DrawStationInspector(const TrackNode* station, int waitingCommuters, float balance);
    void DrawToast(const ToastMessage& toast);
    void DrawQuickTipBanner(const std::string& tip);
    void DrawTransitOperationsManual();
    void DrawWeeklyModal(const std::vector<UpgradeChoice>& choices, int hoveredChoice);
    void DrawGameOver(int finalRidership, int stars, int best, float stateEntryTime);
    void DrawVictory(int finalRidership, int weeks, int stars, float balance, int best, float stateEntryTime);
    void DrawTitleScreen(int best);
    void DrawPauseOverlay();
    void DrawObjectiveChip(const char* title, const char* sub, float progressPct);
    void DrawAdvisorSuggestion(const char* icon, const char* title, const char* hint, Color accentCol, float showTime);
    bool CheckAdvisorDismissClick(Vector2 mousePos, float showTime);

    // Arcade pause menu. Returns: 0 = RESUME, 1 = RESTART, 2 = QUIT TO MENU, -1 = none.
    int CheckPauseClick(Vector2 mousePos) const;

    // Compatibility aliases
    void DrawCoasterStats(const MetroLineStats& stats) { DrawLineOperations(stats); }
    void DrawStaffWindow(const std::vector<StaffMember>& staff, float cleanliness, float balance) {
        DrawTransitCrewWindow(staff, cleanliness, balance);
    }
    void DrawPeepInspector(const Commuter* commuter) { DrawCommuterInspector(commuter); }
    void DrawHelpOverlay() { DrawTransitOperationsManual(); }

    // Interaction checks
    int CheckToolbarTabClick(Vector2 mousePos) const;
    int CheckToolbarItemClick(Vector2 mousePos, ToolCategory activeTab) const;
    bool CheckToolbarAuxClick(Vector2 mousePos, int& outZDelta, bool& outRotate, int& outBuy) const;

    bool CheckHUDClick(
        Vector2 mousePos,
        int& outNewSpeed,
        bool& outToggleMute,
        bool& outToggleStats,
        bool& outToggleStaff,
        bool& outToggleRideCam,
        bool& outToggleHelp,
        bool* outAutoBridge = nullptr
    ) const;

    bool CheckStatsWindowClick(Vector2 mousePos, float& outTicketPriceDelta, int& outColorChoice, int& outModeChange, int& outCarDelta, bool& outClose) const;
    bool CheckStaffWindowClick(Vector2 mousePos, bool& outHireHandyman, bool& outHireMechanic, bool& outClose) const;
    bool CheckPeepInspectorCloseClick(Vector2 mousePos) const;
    bool IsMouseInPeepInspector(Vector2 mousePos) const;
    bool CheckStationInspectorClick(Vector2 mousePos, const TrackNode* station, bool& outUpgrade, bool& outClose) const;
    bool IsMouseInStationInspector(Vector2 mousePos) const;
    bool CheckHelpOverlayClick(Vector2 mousePos) const;
    int CheckUpgradeModalClick(Vector2 mousePos) const;
    bool CheckRestartClick(Vector2 mousePos) const;
    bool CheckVictoryContinueClick(Vector2 mousePos) const;
    bool CheckTitleStartClick(Vector2 mousePos) const;
    void SetModalEntryTime(float t) { modalEntryTime = t; }

private:
    float pulseAnim = 0.0f;
    float animTabX = 0.0f;  // smooth-sliding X position for active tab indicator
    float pressScale = 1.0f; // toolbar button press scale (1.0 = normal, 0.92 = pressed)
    float cashFlashTimer = 0.0f; // green flash on cash when money earned
    float prevBalance = 0.0f;    // detect balance increases for flash
    float modalEntryTime = 0.0f; // GetTime() when modal last opened (for slide-in)
    float advisorDismissY = 0.0f; // Y offset for dismiss button hit-test
};


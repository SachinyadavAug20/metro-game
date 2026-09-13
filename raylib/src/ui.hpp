#pragma once

#include "common.hpp"
#include "peep.hpp"

struct UpgradeChoice {
    std::string title;
    std::string description;
    std::string perkTag;
    Color accentColor;
};

// Shared toolbar geometry - ONE source of truth so the draw code, click
// hit-tests and tutorial spotlight always agree (single place to rescale).
struct ToolbarMetrics {
    static constexpr int BAR_W = 840;
    static constexpr int BAR_H = 118;
    static constexpr int AUX_W = 146;
    static constexpr int GAP = 12;          // gap between main bar and aux pillar
    static constexpr int ITEM_W = 84;
    static constexpr int ITEM_H = 86;       // tall tool buttons for clean large fonts
    static constexpr int TAB_H = 30;
    static constexpr int BUY_H = 24;        // CAR/LAND/EXTRA TRAIN purchase buttons
    static constexpr int BUY_Y = 90;        // purchase row offset from bar top
    static constexpr int ITEM_Y = 14;       // tool button row offset from bar top
    static constexpr int DOCK_H = 12;       // clearances (barY = screenH - BAR_H - DOCK_H)
    static int BarX(int screenW) { return std::max(10, (screenW - (BAR_W + GAP + AUX_W)) / 2); }
    static int BarY(int screenH) { return screenH - BAR_H - DOCK_H; }
    static int AuxX(int screenW)  { return BarX(screenW) + BAR_W + GAP; }
};

// A single stage in the guided, deliberate tutorial walkthrough.
struct TutorialStageInfo {
    const char* title = "";
    const char* body = "";
    const char* hint = "";
    int focusGx = -1;        // grid tile the camera should center on (-1 = none)
    int focusGy = -1;
    bool focusTrain = false; // focus the moving lead EMU instead of a tile
    int toolbarTag = 0;      // 0 none, 1 CAR+1 btn, 2 LAND btn, 3 EXTRA TRAIN btn
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
        float rushCombo = 1.0f
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
        int extraTrainCount
    );

    // OCC Panels & Modals
    void DrawLineOperations(const MetroLineStats& stats);
    void DrawTransitCrewWindow(const std::vector<StaffMember>& staff, float cleanliness, float balance);
    void DrawCommuterInspector(const Commuter* commuter);
    void DrawToast(const ToastMessage& toast);
    void DrawQuickTipBanner(const std::string& tip);
    void DrawTutorialPanel(bool visible, const TutorialStageInfo& stage, int currentIdx, int total, const bool* doneFlags);
    void DrawTransitOperationsManual();
    void DrawWeeklyModal(const std::vector<UpgradeChoice>& choices, int hoveredChoice);
    void DrawGameOver(int finalRidership, int stars, int best);
    void DrawVictory(int finalRidership, int weeks, int stars, float balance, int best);
    void DrawTitleScreen(int best);
    void DrawPauseOverlay();

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
        bool& outToggleHelp
    ) const;

    bool CheckStatsWindowClick(Vector2 mousePos, float& outTicketPriceDelta, int& outColorChoice, int& outModeChange, int& outCarDelta, bool& outClose) const;
    bool CheckStaffWindowClick(Vector2 mousePos, bool& outHireHandyman, bool& outHireMechanic, bool& outClose) const;
    bool CheckPeepInspectorCloseClick(Vector2 mousePos) const;
    bool CheckHelpOverlayClick(Vector2 mousePos) const;
    int CheckUpgradeModalClick(Vector2 mousePos) const;
    bool CheckRestartClick(Vector2 mousePos) const;
    bool CheckVictoryContinueClick(Vector2 mousePos) const;
    bool CheckTitleStartClick(Vector2 mousePos) const;

private:
    float pulseAnim = 0.0f;
};


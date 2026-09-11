#pragma once

#include "common.hpp"
#include "peep.hpp"

struct UpgradeChoice {
    std::string title;
    std::string description;
    std::string perkTag;
    Color accentColor;
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
        bool helpOpen
    );

    // Categorized Toolbar (Track, Concourse, Scenery)
    void DrawToolbar(
        ToolCategory activeTab,
        TrackType currentTrack,
        SceneryType currentScenery,
        GroundType currentGround,
        int currentZ,
        Direction currentDir,
        bool isBulldozing
    );

    // OCC Panels & Modals
    void DrawLineOperations(const MetroLineStats& stats);
    void DrawTransitCrewWindow(const std::vector<StaffMember>& staff, float cleanliness, float balance);
    void DrawCommuterInspector(const Commuter* commuter);
    void DrawToast(const ToastMessage& toast);
    void DrawQuickTipBanner(const std::string& tip);
    void DrawTransitOperationsManual();
    void DrawWeeklyModal(const std::vector<UpgradeChoice>& choices, int hoveredChoice);
    void DrawGameOver(int finalRidership);
    void DrawVictory(int finalRidership);
    void DrawTitleScreen();

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
    bool CheckToolbarAuxClick(Vector2 mousePos, int& outZDelta, bool& outRotate) const;

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
    bool CheckTitleStartClick(Vector2 mousePos) const;

private:
    float pulseAnim = 0.0f;
};


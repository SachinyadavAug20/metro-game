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

    // Main HUD
    void DrawHUD(
        int score,
        float parkRating,
        int week,
        float weekTimer,
        float speedKmh,
        bool circuitClosed,
        int gameSpeed,
        bool muted,
        float balance,
        bool statsOpen,
        bool staffOpen,
        bool rideCamActive,
        bool helpOpen
    );

    // Categorized Toolbar
    void DrawToolbar(
        ToolCategory activeTab,
        TrackType currentTrack,
        SceneryType currentScenery,
        GroundType currentGround,
        int currentZ,
        Direction currentDir,
        bool isBulldozing
    );

    // Modals & Panels
    void DrawCoasterStats(const CoasterStats& stats);
    void DrawStaffWindow(const std::vector<StaffMember>& staff, float cleanliness, float balance);
    void DrawPeepInspector(const Peep* peep);
    void DrawToast(const ToastMessage& toast);
    void DrawHelpOverlay();
    void DrawWeeklyModal(const std::vector<UpgradeChoice>& choices, int hoveredChoice);
    void DrawGameOver(int finalScore);
    void DrawVictory(int finalScore);
    void DrawTitleScreen();

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

    bool CheckStatsWindowClick(Vector2 mousePos, float& outTicketPriceDelta, int& outColorChoice, bool& outClose) const;
    bool CheckStaffWindowClick(Vector2 mousePos, bool& outHireHandyman, bool& outHireMechanic, bool& outClose) const;
    bool CheckPeepInspectorCloseClick(Vector2 mousePos) const;
    bool CheckHelpOverlayClick(Vector2 mousePos) const;
    int CheckUpgradeModalClick(Vector2 mousePos) const;
    bool CheckRestartClick(Vector2 mousePos) const;

private:
    float pulseAnim = 0.0f;
};

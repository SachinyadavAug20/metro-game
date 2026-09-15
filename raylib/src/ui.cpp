#include "ui.hpp"
#include "font_system.hpp"
#include "game.hpp"
#include <cmath>
#include <algorithm>

UserInterface::UserInterface() {}

void UserInterface::Update(Vector2 mousePos, bool mouseClicked) {
    (void)mousePos;
    (void)mouseClicked;
    pulseAnim += GetFrameTime() * 3.0f;
}

void UserInterface::DrawHUD(
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
    float rushCombo
) {
    (void)satisfaction; (void)week;
    (void)signalAspect;
    (void)lineOpsOpen; (void)crewOpen; (void)cabCamActive; (void)rushCombo;

    int screenW = GetScreenWidth();

    // Top Bar Background (Operations Control Center Navy, fully opaque to prevent bleed-through)
    DrawRectangle(0, 0, screenW, 52, Color{15, 23, 42, 255});
    DrawLine(0, 52, screenW, 52, Color{51, 65, 85, 200});
    // Week timer progress bar (thin bar at HUD bottom)
    float weekPct = fmodf(weekTimer, 60.0f) / 60.0f;
    DrawRectangle(0, 50, screenW, 2, Color{30, 41, 59, 200});
    DrawRectangle(0, 50, (int)(screenW * weekPct), 2, Color{56, 189, 248, 150});
    // Subtle gradient fade below HUD for depth
    DrawRectangleGradientV(0, 52, screenW, 12, Color{15, 23, 42, 80}, Color{15, 23, 42, 0});

    // 1. Metro Brand Logo & Roundel Badge
    DrawCircle(26, 26, 14, Color{220, 38, 38, 255});
    DrawCircle(26, 26, 10, Color{15, 23, 42, 255});
    DrawRectangle(16, 23, 20, 6, Color{220, 38, 38, 255});
    DrawText("M", 22, 19, 14, WHITE);
    DrawGameBoldText("METRO GRID", 48, 12, 16, Color{248, 250, 252, 255});
    DrawText("TYCOON SIM", 48, 29, 10, Color{148, 163, 184, 255});

    // 2. Transit Authority Treasury (cash pill card)
    int bankX = 145;
    // Cash earned flash detection
    if (balance > prevBalance + 0.5f) cashFlashTimer = 0.4f;
    prevBalance = balance;
    if (cashFlashTimer > 0.0f) cashFlashTimer -= GetFrameTime();
    float cfAlpha = cashFlashTimer > 0.0f ? 40.0f * (cashFlashTimer / 0.4f) : 0.0f;
    if (cfAlpha > 0) DrawRectangleRounded(Rectangle{(float)bankX - 1, 7.0f, 97.0f, 38.0f}, 0.25f, 4, Color{34, 197, 94, (unsigned char)cfAlpha});
    DrawRectangleRounded(Rectangle{(float)bankX, 8.0f, 95.0f, 36.0f}, 0.25f, 4, Color{30, 41, 59, 200});
    DrawRectangleRoundedLines(Rectangle{(float)bankX, 8.0f, 95.0f, 36.0f}, 0.25f, 4, Color{51, 65, 85, 220});
    DrawText("TREASURY", bankX + 8, 11, 9, Color{148, 163, 184, 255});
    const char* cashStr;
    if (balance >= 1000.0f) {
        static char buf[32];
        snprintf(buf, sizeof(buf), "$%.1fK", balance / 1000.0f);
        cashStr = buf;
    } else {
        static char buf[32];
        snprintf(buf, sizeof(buf), "$%.0f", balance);
        cashStr = buf;
    }
    Color cashColor = (balance >= 200.0f) ? Color{52, 211, 153, 255} : (balance >= 100.0f ? Color{234, 179, 8, 255} : Color{239, 68, 68, 255});
    DrawGameBoldText(cashStr, bankX + 8, 22, 16, cashColor);

    // 3. Commuters Transported + Goal progress card
    int scoreX = 248;
    DrawRectangleRounded(Rectangle{(float)scoreX, 8.0f, 155.0f, 36.0f}, 0.25f, 4, Color{30, 41, 59, 200});
    DrawRectangleRoundedLines(Rectangle{(float)scoreX, 8.0f, 155.0f, 36.0f}, 0.25f, 4, Color{51, 65, 85, 220});
    DrawText("RIDERS", scoreX + 8, 11, 9, Color{148, 163, 184, 255});
    DrawGameBoldText(TextFormat("%d", ridership), scoreX + 8, 22, 16, Color{255, 214, 0, 255});
    const int GOAL = 500;
    float goalPct = std::max(0.0f, std::min(1.0f, (float)ridership / (float)GOAL));
    DrawRectangle(scoreX + 56, 24, 48, 6, Color{51, 65, 85, 255});
    DrawRectangle(scoreX + 56, 24, (int)(48.0f * goalPct), 6, Color{255, 214, 0, 255});
    DrawText(TextFormat("%d/%d", ridership, GOAL), scoreX + 108, 22, 10, Color{148, 163, 184, 255});

    // 3b. Tycoon Transit Rating (RollerCoaster Tycoon style rating)
    int rateX = 411;
    int ratingPts = (int)(satisfaction * 9.99f);
    Color rateCol = (ratingPts >= 750) ? Color{52, 211, 153, 255} : ((ratingPts >= 450) ? Color{250, 204, 21, 255} : Color{239, 68, 68, 255});
    // Danger pulse when rating low
    if (ratingPts < 450) {
        float dp = 0.5f + 0.5f * sinf(GetTime() * 5.0f);
        DrawRectangleRounded(Rectangle{(float)rateX - 1, 7.0f, 94.0f, 38.0f}, 0.25f, 4, Color{239, 68, 68, (unsigned char)(30 + 25 * dp)});
    }
    DrawRectangleRounded(Rectangle{(float)rateX, 8.0f, 92.0f, 36.0f}, 0.25f, 4, Color{30, 41, 59, 200});
    DrawRectangleRoundedLines(Rectangle{(float)rateX, 8.0f, 92.0f, 36.0f}, 0.25f, 4, Color{51, 65, 85, 220});
    DrawText("RATING", rateX + 8, 11, 9, Color{148, 163, 184, 255});
    DrawGameBoldText(TextFormat("%d", ratingPts), rateX + 8, 22, 16, rateCol);
    DrawText("PTS", rateX + 48, 25, 9, rateCol);

    // 4. Train speed
    int spdX = 511;
    Color spdColor = (speedKmh > 55.0f) ? Color{56, 189, 248, 255} : Color{203, 213, 225, 255};
    DrawRectangleRounded(Rectangle{(float)spdX, 8.0f, 88.0f, 36.0f}, 0.25f, 4, Color{30, 41, 59, 200});
    DrawRectangleRoundedLines(Rectangle{(float)spdX, 8.0f, 88.0f, 36.0f}, 0.25f, 4, Color{51, 65, 85, 220});
    DrawText("TRAIN SPEED", spdX + 8, 11, 9, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.0f km/h", speedKmh), spdX + 8, 22, 15, spdColor);

    // 4b. Circuit status badge (OPEN TRACK = red pulsing, LOOP CLOSED = green)
    int badgeX = 607;
    float pulse = 0.5f + 0.5f * sinf(GetTime() * 3.0f);
    if (circuitClosed) {
        // LOOP CLOSED: solid green, calm
        DrawRectangleRounded(Rectangle{(float)badgeX, 8.0f, 120.0f, 36.0f}, 0.25f, 4, Color{22, 101, 52, 255});
        DrawRectangleRoundedLines(Rectangle{(float)badgeX, 8.0f, 120.0f, 36.0f}, 0.25f, 4, Color{34, 197, 94, 255});
        DrawText("LOOP CLOSED", badgeX + 10, 12, 12, Color{110, 231, 183, 255});
        DrawText("RECENTER [C]", badgeX + 10, 26, 10, Color{167, 243, 208, 200});
    } else {
        // OPEN TRACK: pulsing red border for urgency - clickable to auto-bridge
        Color urgentBorder = Color{239, 68, 68, (unsigned char)(180 + 75 * pulse)};
        DrawRectangleRounded(Rectangle{(float)badgeX, 8.0f, 120.0f, 36.0f}, 0.25f, 4, Color{185, 28, 28, 255});
        DrawRectangleRoundedLines(Rectangle{(float)badgeX, 8.0f, 120.0f, 36.0f}, 0.25f, 4, urgentBorder);
        DrawText("AUTO-LOOP [B]", badgeX + 8, 14, 13, WHITE);
        DrawText("CLICK TO CLOSE", badgeX + 8, 28, 10, Color{254, 202, 202, (unsigned char)(200 + 55 * pulse)});
    }

    // 5. Simulation controls & mute (far right anchor)
    int btnX = screenW - 140;

    // 5b. Telemetry / Stats button [i] (OCC Ride & Line Operations)
    int infoX = btnX - 74;
    int infoR = 14;
    int infoCY = 26;
    DrawCircle(infoX + infoR, infoCY, infoR, lineOpsOpen ? Color{234, 179, 8, 255} : Color{30, 41, 59, 220});
    DrawCircleLines(infoX + infoR, infoCY, infoR, lineOpsOpen ? WHITE : Color{148, 163, 184, 255});
    DrawText("i", infoX + infoR - 3, infoCY - 8, 16, lineOpsOpen ? Color{15, 23, 42, 255} : WHITE);

    // 6. "How to Play" — clean circular help icon
    int helpX = btnX - 38;
    int helpR = 14;
    int helpCY = 26;
    DrawCircle(helpX + helpR, helpCY, helpR, helpOpen ? Color{220, 38, 38, 255} : Color{14, 116, 144, 240});
    DrawCircleLines(helpX + helpR, helpCY, helpR, Color{56, 189, 248, 255});
    DrawText("?", helpX + helpR - 5, helpCY - 8, 16, helpOpen ? WHITE : Color{255, 214, 0, 255});

    // Pause / Normal / Fast (modern icon style)
    DrawRectangleRounded(Rectangle{(float)btnX, 12.0f, 28.0f, 28.0f}, 0.2f, 4, (gameSpeed == 0) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawRectangle(btnX + 10, 18, 3, 12, WHITE);
    DrawRectangle(btnX + 16, 18, 3, 12, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 32, 12.0f, 28.0f, 28.0f}, 0.2f, 4, (gameSpeed == 1) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawTriangle({(float)(btnX + 40), 18.0f}, {(float)(btnX + 40), 30.0f}, {(float)(btnX + 50), 24.0f}, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 64, 12.0f, 28.0f, 28.0f}, 0.2f, 4, (gameSpeed == 2) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawTriangle({(float)(btnX + 68), 18.0f}, {(float)(btnX + 68), 30.0f}, {(float)(btnX + 78), 24.0f}, WHITE);
    DrawTriangle({(float)(btnX + 76), 18.0f}, {(float)(btnX + 76), 30.0f}, {(float)(btnX + 86), 24.0f}, WHITE);

    // Mute (speaker icon)
    DrawRectangleRounded(Rectangle{(float)btnX + 96, 12.0f, 28.0f, 28.0f}, 0.2f, 4, muted ? Color{239, 68, 68, 220} : Color{30, 41, 59, 200});
    // Speaker body
    DrawRectangle(btnX + 103, 21, 4, 6, WHITE);
    // Speaker cone
    DrawTriangle({(float)(btnX + 107), 18.0f}, {(float)(btnX + 107), 32.0f}, {(float)(btnX + 116), 24.0f}, WHITE);
    if (muted) {
        // Mute X
        DrawLineEx({(float)(btnX + 117), 18.0f}, {(float)(btnX + 125), 32.0f}, 2.0f, Color{239, 68, 68, 255});
        DrawLineEx({(float)(btnX + 125), 18.0f}, {(float)(btnX + 117), 32.0f}, 2.0f, Color{239, 68, 68, 255});
    } else {
        // Sound waves
        DrawCircleLines(btnX + 119, 24, 3, Color{56, 189, 248, 200});
        DrawCircleLines(btnX + 119, 24, 6, Color{56, 189, 248, 120});
    }
}

void UserInterface::DrawToolbar(
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
    bool isTerraformingRaise
) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int barW = ToolbarMetrics::BAR_W;
    int barH = ToolbarMetrics::BAR_H;
    int auxW = ToolbarMetrics::AUX_W;
    int barX = ToolbarMetrics::BarX(screenW);
    int barY = ToolbarMetrics::BarY(screenH);

    // 1. Category Tabs above toolbar (with icons)
    const char* tabNames[] = {"TRACKS", "CONCOURSE", "SCENERY"};
    int tabW = 150;
    int tabH = ToolbarMetrics::TAB_H;
    int tabStartY = barY - tabH + 2;

    for (int t = 0; t < 3; ++t) {
        int tx = barX + 16 + t * (tabW + 10);
        bool isCurrentTab = ((int)activeTab == t);
        Color tabBg = isCurrentTab ? Color{15, 23, 42, 255} : Color{30, 41, 59, 200};
        Color tabText = isCurrentTab ? Color{56, 189, 248, 255} : Color{148, 163, 184, 255};

        DrawRectangleRounded(Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4}, 0.2f, 4, tabBg);
        DrawRectangleRoundedLines(Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4}, 0.2f, 4, isCurrentTab ? Color{56, 189, 248, 255} : Color{51, 65, 85, 255});

        // Tab icons (small geometric shapes)
        int iconX = tx + 14;
        int iconCY = tabStartY + tabH / 2 + 2;
        if (t == 0) {
            // TRACKS: railroad track icon (two parallel lines)
            DrawLineEx({(float)iconX, (float)(iconCY - 4)}, {(float)(iconX + 10), (float)(iconCY + 4)}, 2.0f, tabText);
            DrawLineEx({(float)(iconX + 3), (float)(iconCY - 4)}, {(float)(iconX + 13), (float)(iconCY + 4)}, 2.0f, tabText);
        } else if (t == 1) {
            // CONCOURSE: building/platform icon
            DrawRectangle(iconX, iconCY - 5, 12, 10, tabText);
            DrawRectangle(iconX + 2, iconCY - 3, 3, 4, Color{15, 23, 42, 255});
            DrawRectangle(iconX + 7, iconCY - 3, 3, 4, Color{15, 23, 42, 255});
        } else {
            // SCENERY: tree icon
            DrawRectangle(iconX + 2, iconCY, 2, 5, tabText);
            DrawTriangle({(float)(iconX + 3), (float)(iconCY - 5)}, {(float)iconX, (float)(iconCY + 1)}, {(float)(iconX + 6), (float)(iconCY + 1)}, tabText);
        }

        DrawGameBoldTextCentered(tabNames[t], tx + 75, tabStartY + 8, 15, tabText);
    }

    // Sliding tab indicator bar
    float targetTabX = (float)(barX + 16 + (int)activeTab * (tabW + 10));
    animTabX += (targetTabX - animTabX) * 0.2f;
    DrawRectangle((int)animTabX, tabStartY + tabH + 2, tabW, 3, Color{56, 189, 248, 200});

    // 2. Toolbar Body Card (semi-transparent for modern layered feel)
    // Drop shadow
    DrawRectangleRounded(Rectangle{(float)barX + 2, (float)barY + 3, (float)barW, (float)barH}, 0.2f, 6, Color{0, 0, 0, 40});
    DrawRectangleRounded(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.2f, 6, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.2f, 6, Color{51, 65, 85, 200});
    // Subtle top-edge gradient for depth
    DrawRectangleGradientV(barX + 4, barY + 2, barW - 8, 6, Color{56, 189, 248, 15}, Color{15, 23, 42, 0});

    // 3. Render Items according to Active Tab
    int itemBtnW = ToolbarMetrics::ITEM_W;
    int itemBtnH = ToolbarMetrics::ITEM_H;
    int itemStartY = barY + ToolbarMetrics::ITEM_Y;
    // Press scale animation (0.92 when held, spring back to 1.0)
    float targetScale = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 0.92f : 1.0f;
    pressScale += (targetScale - pressScale) * 0.3f;
    float tbPulse = 0.5f + 0.5f * sinf(GetTime() * 4.0f);

    if (activeTab == CAT_TRACK) {
        struct TrackBtn { const char* name; const char* key; TrackType type; const char* tip; };
        TrackBtn buttons[] = {
            {"Straight", "1", TRACK_STRAIGHT, "Flat rail segment"},
            {"L-Turn", "2", TRACK_CURVE_LEFT, "90° left curve"},
            {"R-Turn", "3", TRACK_CURVE_RIGHT, "90° right curve"},
            {"Viaduct", "4", TRACK_VIADUCT_ELEVATED, "Elevated bridge (Z=1)"},
            {"Slope", "5", TRACK_VIADUCT_SLOPE, "Slope between levels"},
            {"Tunnel", "6", TRACK_TUNNEL_PORTAL, "Underground passage"},
            {"Station", "7", TRACK_STATION, "Platform for boarding"},
            {"Signal", "8", TRACK_SIGNAL, "Block signal (R/Y/G)"},
            {"Demolish", "X", TRACK_NONE, "Remove track/structure"}
        };

        int spacing = 10;
        int startX = barX + (barW - (9 * itemBtnW + 8 * spacing)) / 2;

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + spacing);
            bool isSelected = (i == 8) ? isBulldozing : (!isBulldozing && currentTrack == buttons[i].type);
            bool isHovered = CheckCollisionPointRec(GetMousePosition(), {(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH});
            float sc = (isHovered && pressScale < 0.99f) ? pressScale : 1.0f;
            int bw = (int)(itemBtnW * sc);
            int bh = (int)(itemBtnH * sc);
            int bxo = bx + (itemBtnW - bw) / 2;
            int byo = itemStartY + (itemBtnH - bh) / 2;

            Color btnBg = isSelected ? Color{220, 38, 38, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bxo, (float)byo, (float)bw, (float)bh}, 0.2f, 4, btnBg);
            // Selected tool gets pulsing glow border
            Color borderC = isSelected ? Color{254, 202, 202, (unsigned char)(200 + 55 * tbPulse)} : Color{71, 85, 105, 200};
            DrawRectangleRoundedLines(Rectangle{(float)bxo, (float)byo, (float)bw, (float)bh}, 0.2f, 4, borderC);
            if (isSelected) {
                DrawRectangleRoundedLines(Rectangle{(float)bxo - 1, (float)byo - 1, (float)bw + 2, (float)bh + 2}, 0.2f, 4, Color{254, 202, 202, (unsigned char)(60 + 40 * tbPulse)});
            }

            DrawText(buttons[i].key, bxo + 6, byo + 8, 16, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bxo + 4, byo + 34, 15, textC);

            // Hover highlight + tooltip
            if (!isSelected && isHovered) {
                DrawRectangleRounded(Rectangle{(float)bxo, (float)byo, (float)bw, (float)bh}, 0.2f, 4, Color{255, 255, 255, 18});
                int tipW = MeasureText(buttons[i].tip, 11);
                float tipX = bxo + (bw - tipW) / 2.0f;
                float tipY = byo + bh + 4;
                DrawRectangleRounded(Rectangle{tipX - 4, tipY, (float)(tipW + 8), 18.0f}, 0.4f, 3, Color{15, 23, 42, 240});
                DrawText(buttons[i].tip, (int)tipX, (int)tipY + 3, 11, Color{203, 213, 225, 255});
            }
        }
    } else if (activeTab == CAT_INFRA) {
        struct InfraBtn { const char* name; const char* key; GroundType ground; int action; int cost; const char* tip; };
        InfraBtn buttons[] = {
            {"Sidewalk", "1", GROUND_PATH, 0, 15, "Concrete walkway ($15)"},
            {"Queue", "2", GROUND_QUEUE, 0, 15, "Platform tactile queue ($15)"},
            {"Plaza", "3", GROUND_PLAZA, 0, 20, "Granite concourse ($20)"},
            {"Reclaim", "4", GROUND_GRASS, 0, 25, "Turn water into grass land ($25)"},
            {"Canal", "5", GROUND_WATER, 0, 30, "Excavate water canal ($30)"},
            {"Sand", "6", GROUND_SAND, 0, 15, "Coastal beach & dunes ($15)"},
            {"Stone", "7", GROUND_STONE, 0, 20, "Mountain slate & cobblestone ($20)"},
            {"Raise Hill", "8", GROUND_GRASS, 1, 40, "Raise ground elevation +1 Z ($40)"},
            {"Demolish", "X", GROUND_GRASS, 2, 0, "Remove pavement / restore ($0)"}
        };

        int spacing = 10;
        int startX = barX + (barW - (9 * itemBtnW + 8 * spacing)) / 2;

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + spacing);
            bool isSelected = false;
            if (buttons[i].action == 2) {
                isSelected = isBulldozing;
            } else if (buttons[i].action == 1) {
                isSelected = (!isBulldozing && isTerraformingRaise);
            } else {
                isSelected = (!isBulldozing && !isTerraformingRaise && currentGround == buttons[i].ground);
            }

            Color btnBg = isSelected ? Color{220, 38, 38, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            Color borderC2 = isSelected ? Color{254, 202, 202, (unsigned char)(200 + 55 * tbPulse)} : Color{71, 85, 105, 200};
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, borderC2);
            if (isSelected) {
                DrawRectangleRoundedLines(Rectangle{(float)bx - 1, (float)itemStartY - 1, (float)itemBtnW + 2, (float)itemBtnH + 2}, 0.2f, 4, Color{254, 202, 202, (unsigned char)(60 + 40 * tbPulse)});
            }

            DrawText(buttons[i].key, bx + 6, itemStartY + 8, 16, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 4, itemStartY + 34, 13, textC);

            if (buttons[i].cost > 0) {
                Color costC = (balance >= buttons[i].cost) ? Color{52, 211, 153, 255}
                           : (balance >= buttons[i].cost * 0.5f ? Color{234, 179, 8, 255} : Color{239, 68, 68, 255});
                DrawText(TextFormat("$%d", buttons[i].cost), bx + 6, itemStartY + 62, 13, costC);
            }

            // Hover highlight + tooltip
            if (!isSelected && CheckCollisionPointRec(GetMousePosition(), {(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH})) {
                DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, Color{255, 255, 255, 18});
                int tipW = MeasureText(buttons[i].tip, 11);
                float tipX = bx + (itemBtnW - tipW) / 2.0f;
                float tipY = itemStartY + itemBtnH + 4;
                DrawRectangleRounded(Rectangle{tipX - 4, tipY, (float)(tipW + 8), 18.0f}, 0.4f, 3, Color{15, 23, 42, 240});
                DrawText(buttons[i].tip, (int)tipX, (int)tipY + 3, 11, Color{203, 213, 225, 255});
            }
        }
    } else if (activeTab == CAT_SCENERY) {
        struct SceneryBtn { const char* name; const char* key; SceneryType scn; int cost; };
        SceneryBtn buttons[] = {
            {"Entrance", "1", SCENERY_METRO_ENTRANCE, 120},
            {"Fare Gates", "2", SCENERY_TURNSTILE_GATE, 90},
            {"Oak Tree", "3", SCENERY_STREET_TREE, 35},
            {"Pine Tree", "4", SCENERY_PINE_TREE, 30},
            {"Bench", "5", SCENERY_BENCH, 20},
            {"LED Lamp", "6", SCENERY_LAMP_POST, 25},
            {"Fountain", "7", SCENERY_FOUNTAIN, 120},
            {"Cafe", "8", SCENERY_NEWSSTAND, 150},
            {"Demolish", "X", SCENERY_NONE, 0}
        };

        int spacing = 10;
        int startX = barX + (barW - (9 * itemBtnW + 8 * spacing)) / 2;

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + spacing);
            bool isSelected = (i == 8) ? isBulldozing : (!isBulldozing && currentScenery == buttons[i].scn);

            Color btnBg = isSelected ? Color{220, 38, 38, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            Color borderC3 = isSelected ? Color{254, 202, 202, (unsigned char)(200 + 55 * tbPulse)} : Color{71, 85, 105, 200};
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, borderC3);
            if (isSelected) {
                DrawRectangleRoundedLines(Rectangle{(float)bx - 1, (float)itemStartY - 1, (float)itemBtnW + 2, (float)itemBtnH + 2}, 0.2f, 4, Color{254, 202, 202, (unsigned char)(60 + 40 * tbPulse)});
            }
DrawText(buttons[i].key, bx + 6, itemStartY + 8, 16, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 4, itemStartY + 34, 15, textC);

            if (buttons[i].cost > 0) {
                Color costC = (balance >= buttons[i].cost) ? Color{52, 211, 153, 255}
                           : (balance >= buttons[i].cost * 0.5f ? Color{234, 179, 8, 255} : Color{239, 68, 68, 255});
                DrawText(TextFormat("$%d", buttons[i].cost), bx + 6, itemStartY + 62, 13, costC);
            }

            // Hover highlight + tooltip
            if (!isSelected && CheckCollisionPointRec(GetMousePosition(), {(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH})) {
                DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, Color{255, 255, 255, 18});
                if (buttons[i].cost > 0) {
                    const char* tipText = TextFormat("%s — $%d", buttons[i].name, buttons[i].cost);
                    int tipW = MeasureText(tipText, 11);
                    float tipX = bx + (itemBtnW - tipW) / 2.0f;
                    float tipY = itemStartY + itemBtnH + 4;
                    DrawRectangleRounded(Rectangle{tipX - 4, tipY, (float)(tipW + 8), 18.0f}, 0.4f, 3, Color{15, 23, 42, 240});
                    DrawText(tipText, (int)tipX, (int)tipY + 3, 11, Color{255, 214, 0, 255});
                }
            }
        }
    }

    // 3b. Fleet & Island info strip inside toolbar bottom padding
    int infoY = barY + 110;
    DrawText(TextFormat("Metropolis Fleet: %d Cars   |   Island Territory: Radius %d / 48   |   [TAB] Category   [L] Theme", carCount, buildRadius), barX + 24, infoY, 12, Color{148, 163, 184, 255});

    // 4. Auxiliary Pills on Right (Elevation & Heading)
    int auxX = barX + barW + 12;

    // Height Pill
    DrawRectangleRounded(Rectangle{(float)auxX + 2, (float)barY + 2, (float)auxW, 34.0f}, 0.25f, 4, Color{0, 0, 0, 35});
    DrawRectangleRounded(Rectangle{(float)auxX, (float)barY, (float)auxW, 34.0f}, 0.25f, 4, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)barY, (float)auxW, 34.0f}, 0.25f, 4, Color{51, 65, 85, 255});
    DrawText("HEIGHT (E/Q)", auxX + 8, barY + 3, 12, Color{148, 163, 184, 255});
    DrawText(TextFormat("Z = %d", currentZ), auxX + 9, barY + 16, 18, Color{255, 214, 0, 255});
    DrawRectangle((float)auxX + 96, (float)barY + 6, 20, 22, Color{30, 41, 59, 255});
    DrawText("-", auxX + 103, barY + 9, 17, WHITE);
    DrawRectangle((float)auxX + 120, (float)barY + 6, 20, 22, Color{30, 41, 59, 255});
    DrawText("+", auxX + 126, barY + 9, 17, WHITE);

    // Heading / Orientation Compass (compact circular)
    const char* dirLabels[] = {"N", "E", "S", "W"};
    DrawRectangleRounded(Rectangle{(float)auxX + 2, (float)barY + 40, (float)auxW, 34.0f}, 0.25f, 4, Color{0, 0, 0, 35});
    DrawRectangleRounded(Rectangle{(float)auxX, (float)barY + 38, (float)auxW, 34.0f}, 0.25f, 4, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)barY + 38, (float)auxW, 34.0f}, 0.25f, 4, Color{51, 65, 85, 255});
    // Compass circle
    int cx = auxX + 20;
    int cy = barY + 55;
    DrawCircle(cx, cy, 12, Color{30, 41, 59, 255});
    DrawCircleLines(cx, cy, 12, Color{100, 116, 139, 255});
    // Cardinal dots
    DrawCircle(cx, cy - 10, 2, Color{239, 68, 68, 255});  // N = red
    DrawCircle(cx, cy + 10, 2, Color{148, 163, 184, 180}); // S
    DrawCircle(cx - 10, cy, 2, Color{148, 163, 184, 180}); // W
    DrawCircle(cx + 10, cy, 2, Color{148, 163, 184, 180}); // E
    // Needle (rotates with currentDir)
    float needleAngle = -1.5708f + currentDir * 1.5708f; // -PI/2 + dir*PI/2
    float nx = cx + cosf(needleAngle) * 8;
    float ny = cy + sinf(needleAngle) * 8;
    DrawLineEx({(float)cx, (float)cy}, {nx, ny}, 2.0f, Color{239, 68, 68, 255});
    DrawText("HEADING [R]", auxX + 38, barY + 41, 11, Color{148, 163, 184, 255});
    DrawText(dirLabels[currentDir], auxX + 38, barY + 53, 14, Color{56, 189, 248, 255});
    DrawRectangle((float)auxX + 104, (float)barY + 44, 34, 22, Color{30, 41, 59, 255});
    DrawText("ROT", auxX + 110, barY + 48, 11, WHITE);

    // Big Buy Buttons (RCT-style purchases): Train Car + Land
    int buyY = barY + ToolbarMetrics::BUY_Y;
    int buyH = ToolbarMetrics::BUY_H;
    Color carOk = (balance >= 600.0f) ? Color{56, 189, 248, 255} : Color{71, 85, 105, 255};
    Color landOk = (balance >= 500.0f) ? Color{74, 222, 128, 255} : Color{71, 85, 105, 255};

    float buyPulse = 0.5f + 0.5f * sinf(GetTime() * 3.0f);
    if (balance >= 600.0f) {
        DrawRectangleRounded(Rectangle{(float)auxX, (float)buyY - 2, 74.0f, (float)buyH + 4}, 0.3f, 4, Color{56, 189, 248, (unsigned char)(30 + 25 * buyPulse)});
    }
    DrawRectangleRounded(Rectangle{(float)auxX, (float)buyY, 74.0f, (float)buyH}, 0.3f, 4, carOk);
    DrawText("CAR+1 $600", auxX + 6, buyY + 6, 11, WHITE);

    if (balance >= 500.0f) {
        DrawRectangleRounded(Rectangle{(float)auxX + 78, (float)buyY - 2, 74.0f, (float)buyH + 4}, 0.3f, 4, Color{74, 222, 128, (unsigned char)(30 + 25 * buyPulse)});
    }
    DrawRectangleRounded(Rectangle{(float)auxX + 78, (float)buyY, 74.0f, (float)buyH}, 0.3f, 4, landOk);
    DrawText("EXPAND $500", auxX + 82, buyY + 6, 10, WHITE);

    // Buy Extra Train (RCT-style purchase) positioned cleanly in aux column
    int exY = buyY + buyH + 4;
    int exH = 26;
    if (extraTrainCount >= Game::MaxExtraTrains()) {
        DrawRectangleRounded(Rectangle{(float)auxX, (float)exY, (float)auxW, (float)exH}, 0.3f, 4, Color{74, 222, 128, 255});
        DrawText("FLEET AT MAX (4 EMU)", auxX + 12, exY + 7, 11, WHITE);
    } else {
        int nextCost = (int)Game::ExtraTrainCostP(extraTrainCount);
        bool canAfford = balance >= nextCost;
        Color extraC = canAfford ? Color{56, 189, 248, 255} : Color{71, 85, 105, 255};
        if (canAfford) {
            DrawRectangleRounded(Rectangle{(float)auxX - 2, (float)exY - 2, (float)auxW + 4, (float)exH + 4}, 0.3f, 4, Color{56, 189, 248, (unsigned char)(40 + 30 * buyPulse)});
        }
        DrawRectangleRounded(Rectangle{(float)auxX, (float)exY, (float)auxW, (float)exH}, 0.3f, 4, extraC);
        if (canAfford) {
            DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)exY, (float)auxW, (float)exH}, 0.3f, 4, Color{147, 197, 253, (unsigned char)(200 + 55 * buyPulse)});
        }
        DrawText(extraTrainCount > 0
                     ? TextFormat("EXTRA TRAIN #%d $%d", extraTrainCount + 1, nextCost)
                     : TextFormat("BUY EXTRA TRAIN $%d", nextCost),
                 auxX + 10, exY + 7, 11, WHITE);
    }
}

void UserInterface::DrawLineOperations(const MetroLineStats& stats) {
    int screenW = GetScreenWidth();
    int winW = 330;
    int winH = 500;
    int winX = screenW - winW - 16;
    int winY = 62;

    // Window shadow & body
    DrawRectangleRounded(Rectangle{(float)winX, (float)winY, (float)winW, (float)winH}, 0.1f, 6, Color{15, 23, 42, 250});
    DrawRectangleRoundedLines(Rectangle{(float)winX, (float)winY, (float)winW, (float)winH}, 0.1f, 6, stats.themeColor);

    // Window Header
    DrawRectangleRounded(Rectangle{(float)winX + 2, (float)winY + 2, (float)winW - 4, 34.0f}, 0.2f, 4, stats.themeColor);
    DrawText(TextFormat("METRO: %s", stats.lineName.c_str()), winX + 12, winY + 11, 11, WHITE);
    DrawText("[X]", winX + winW - 28, winY + 10, 14, WHITE);

    // 1. Operating Mode Traffic Light Buttons (RCT Style: Open / Test / Closed)
    int modeY = winY + 44;
    int modeBtnW = 94;

    // OPEN Button (Green)
    bool isOpen = (stats.mode == LINE_OPEN);
    DrawRectangleRounded(Rectangle{(float)winX + 14, (float)modeY, (float)modeBtnW, 26.0f}, 0.25f, 4, isOpen ? Color{34, 197, 94, 255} : Color{20, 45, 30, 220});
    DrawRectangleRoundedLines(Rectangle{(float)winX + 14, (float)modeY, (float)modeBtnW, 26.0f}, 0.25f, 4, isOpen ? WHITE : Color{34, 197, 94, 120});
    DrawCircle(winX + 26, modeY + 13, 5, isOpen ? WHITE : Color{34, 197, 94, 255});
    DrawText("OPEN", winX + 38, modeY + 7, 10, WHITE);

    // TEST Button (Yellow)
    bool isTest = (stats.mode == LINE_TEST);
    DrawRectangleRounded(Rectangle{(float)winX + 118, (float)modeY, (float)modeBtnW, 26.0f}, 0.25f, 4, isTest ? Color{234, 179, 8, 255} : Color{45, 40, 20, 220});
    DrawRectangleRoundedLines(Rectangle{(float)winX + 118, (float)modeY, (float)modeBtnW, 26.0f}, 0.25f, 4, isTest ? WHITE : Color{234, 179, 8, 120});
    DrawCircle(winX + 130, modeY + 13, 5, isTest ? WHITE : Color{234, 179, 8, 255});
    DrawText("TEST RUN", winX + 140, modeY + 7, 10, WHITE);

    // CLOSED Button (Red)
    bool isClosed = (stats.mode == LINE_CLOSED);
    DrawRectangleRounded(Rectangle{(float)winX + 222, (float)modeY, (float)modeBtnW, 26.0f}, 0.25f, 4, isClosed ? Color{239, 68, 68, 255} : Color{45, 20, 20, 220});
    DrawRectangleRoundedLines(Rectangle{(float)winX + 222, (float)modeY, (float)modeBtnW, 26.0f}, 0.25f, 4, isClosed ? WHITE : Color{239, 68, 68, 120});
    DrawCircle(winX + 234, modeY + 13, 5, isClosed ? WHITE : Color{239, 68, 68, 255});
    DrawText("CLOSED", winX + 246, modeY + 7, 10, WHITE);

    // Content rows
    int rowY = winY + 78;
    int rowH = 24;

    // Train Formation
    DrawText("Train Formation:", winX + 15, rowY + 2, 11, Color{203, 213, 225, 255});
    DrawText(TextFormat("%d Cars (%d Seats)", stats.fleetCars, stats.fleetCars * 4), winX + 130, rowY + 2, 11, Color{56, 189, 248, 255});
    DrawRectangleRounded(Rectangle{(float)winX + 250, (float)rowY, 24.0f, 20.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText("-", winX + 258, rowY + 2, 13, WHITE);
    DrawRectangleRounded(Rectangle{(float)winX + 278, (float)rowY, 24.0f, 20.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText("+", winX + 285, rowY + 2, 13, WHITE);
    rowY += rowH;

    // Fare Tariff Setting
    DrawText("Standard Metro Fare:", winX + 15, rowY + 2, 11, Color{203, 213, 225, 255});
    DrawText(TextFormat("$%.2f", stats.ticketFare), winX + 145, rowY + 2, 12, Color{52, 211, 153, 255});
    DrawRectangleRounded(Rectangle{(float)winX + 250, (float)rowY, 24.0f, 20.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText("-", winX + 258, rowY + 2, 13, WHITE);
    DrawRectangleRounded(Rectangle{(float)winX + 278, (float)rowY, 24.0f, 20.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText("+", winX + 285, rowY + 2, 13, WHITE);
    rowY += rowH + 4;

    DrawLine(winX + 15, rowY, winX + winW - 15, rowY, Color{51, 65, 85, 255});
    rowY += 6;

    // Punctuality / On-Time
    DrawText("On-Time Punctuality:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.1f%% (Tokyo Grade)", stats.onTimeRate), winX + 155, rowY, 11, Color{74, 222, 128, 255});
    rowY += rowH;

    // Commuter Satisfaction
    DrawText("Commuter Comfort:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.1f%% (High)", stats.commuterSatisfaction), winX + 155, rowY, 11, Color{255, 214, 0, 255});
    rowY += rowH;

    // Speeds & Track Length
    DrawText("Cruising Velocity:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.1f km/h", stats.maxSpeedKmh), winX + 155, rowY, 11, WHITE);
    rowY += rowH;

    DrawText("Circuit Track Length:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.0f meters", stats.trackLengthM), winX + 155, rowY, 11, WHITE);
    rowY += rowH;

    DrawText("Interchanges / Stations:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%d Stations", stats.stationCount), winX + 155, rowY, 11, WHITE);
    rowY += rowH;

    DrawText("Delivered Commuters:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%d riders", stats.totalRiders), winX + 155, rowY, 11, Color{255, 214, 0, 255});
    rowY += rowH;

    DrawText("Total Fare Revenue:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("$%.2f", stats.totalRevenue), winX + 155, rowY, 11, Color{52, 211, 153, 255});
    rowY += rowH;

    DrawText("RCT Excitement Rating:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.2f / 10.0 (High)", stats.excitementRating), winX + 155, rowY, 11, Color{250, 204, 21, 255});
    rowY += rowH;

    DrawText("RCT Intensity Rating:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.2f / 10.0 (Medium)", stats.intensityRating), winX + 155, rowY, 11, Color{248, 113, 113, 255});
    rowY += rowH;

    DrawText("Transit Park Value:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("$%.0f", stats.parkValue), winX + 155, rowY, 11, Color{74, 222, 128, 255});
    rowY += rowH + 4;

    // Transit Line Color Livery
    DrawLine(winX + 15, rowY, winX + winW - 15, rowY, Color{51, 65, 85, 255});
    rowY += 6;
    DrawText("Line Livery Palette:", winX + 15, rowY + 3, 11, Color{203, 213, 225, 255});

    Color liveries[] = {
        Color{229, 57, 53, 255},  // Tokyo Red (Marunouchi)
        Color{37, 99, 235, 255},  // London Blue (Piccadilly)
        Color{16, 185, 129, 255}, // Paris Green (Line 6)
        Color{147, 51, 234, 255}, // Victoria Purple
        Color{245, 158, 11, 255}, // Chicago Amber
        Color{234, 88, 12, 255}   // Tokyo Ginza Orange
    };

    for (int c = 0; c < 6; ++c) {
        int cx = winX + 145 + c * 27;
        DrawRectangleRounded(Rectangle{(float)cx, (float)rowY, 23.0f, 20.0f}, 0.3f, 4, liveries[c]);
        DrawRectangleRoundedLines(Rectangle{(float)cx, (float)rowY, 23.0f, 20.0f}, 0.3f, 4, WHITE);
    }
}

void UserInterface::DrawTransitCrewWindow(const std::vector<StaffMember>& staff, float cleanliness, float balance) {
    int screenW = GetScreenWidth();
    int winW = 340;
    int winH = 340;
    int winX = screenW - winW - 16;
    int winY = 62;

    // Window shadow & body
    DrawRectangleRounded(Rectangle{(float)winX, (float)winY, (float)winW, (float)winH}, 0.1f, 6, Color{15, 23, 42, 250});
    DrawRectangleRoundedLines(Rectangle{(float)winX, (float)winY, (float)winW, (float)winH}, 0.1f, 6, Color{56, 189, 248, 255});

    // Window Header
    DrawRectangleRounded(Rectangle{(float)winX + 2, (float)winY + 2, (float)winW - 4, 32.0f}, 0.2f, 4, Color{30, 58, 138, 255});
    DrawText("TRANSIT CREW & MAINTENANCE", winX + 12, winY + 10, 11, Color{255, 214, 0, 255});
    DrawText("[X]", winX + winW - 28, winY + 9, 14, WHITE);

    // Platform Cleanliness bar
    DrawText("STATION CLEANLINESS", winX + 16, winY + 44, 9, Color{148, 163, 184, 255});
    Color cleanCol = (cleanliness > 75.0f) ? Color{74, 222, 128, 255} : (cleanliness > 40.0f ? Color{251, 146, 60, 255} : Color{239, 68, 68, 255});
    DrawText(TextFormat("%.0f%%", cleanliness), winX + winW - 55, winY + 42, 12, cleanCol);
    DrawRectangle(winX + 16, winY + 58, winW - 32, 8, Color{30, 41, 59, 255});
    DrawRectangle(winX + 16, winY + 58, (int)((winW - 32) * (cleanliness / 100.0f)), 8, cleanCol);

    // Hiring Section
    DrawText("HIRE TRANSIT WORKERS", winX + 16, winY + 76, 9, Color{148, 163, 184, 255});

    // Hire Custodian Button
    bool canAffordC = (balance >= 80.0f);
    float btnW = (float)(winW - 40) / 2.0f;
    DrawRectangleRounded(Rectangle{(float)winX + 16, (float)winY + 92, btnW, 36.0f}, 0.2f, 4, canAffordC ? Color{29, 78, 216, 220} : Color{51, 65, 85, 180});
    DrawRectangleRoundedLines(Rectangle{(float)winX + 16, (float)winY + 92, btnW, 36.0f}, 0.2f, 4, Color{96, 165, 250, 255});
    DrawText("+ Custodian", winX + 22, winY + 98, 10, WHITE);
    DrawText("$80 (Cleans Spills)", winX + 22, winY + 112, 8, Color{191, 219, 254, 255});

    // Hire Signal Technician Button
    bool canAffordE = (balance >= 100.0f);
    DrawRectangleRounded(Rectangle{(float)winX + 24 + btnW, (float)winY + 92, btnW, 36.0f}, 0.2f, 4, canAffordE ? Color{217, 119, 6, 220} : Color{51, 65, 85, 180});
    DrawRectangleRoundedLines(Rectangle{(float)winX + 24 + btnW, (float)winY + 92, btnW, 36.0f}, 0.2f, 4, Color{251, 191, 36, 255});
    DrawText("+ Technician", winX + 30 + (int)btnW, winY + 98, 10, WHITE);
    DrawText("$100 (Signal & Rails)", winX + 30 + (int)btnW, winY + 112, 8, Color{254, 240, 138, 255});

    // Active Crew Roster
    DrawText(TextFormat("ACTIVE DISPATCH ROSTER (%d)", (int)staff.size()), winX + 16, winY + 138, 9, Color{148, 163, 184, 255});
    int listY = winY + 154;
    int drawn = 0;
    for (const auto& s : staff) {
        if (drawn >= 5) break;
        DrawRectangle(winX + 16, listY, winW - 32, 28, (drawn % 2 == 0) ? Color{30, 41, 59, 180} : Color{20, 30, 45, 180});
        Color badgeC = (s.type == STAFF_CUSTODIAN) ? Color{59, 130, 246, 255} : Color{245, 158, 11, 255};
        DrawCircle(winX + 26, listY + 14, 5, badgeC);
        DrawText(s.name.c_str(), winX + 38, listY + 8, 10, WHITE);
        const char* status = s.isWorking ? (s.type == STAFF_CUSTODIAN ? "Sweeping" : "Inspecting") : "Patrolling";
        DrawText(status, winX + winW - 85, listY + 8, 9, Color{148, 163, 184, 255});
        listY += 32;
        drawn++;
    }
}

void UserInterface::DrawCommuterInspector(const Commuter* commuter) {
    if (!commuter) return;

    int cardW = 300;
    int cardH = 145;
    int cardX = 14;
    int cardY = 120;

    // Card background
    DrawRectangleRounded(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.15f, 6, Color{15, 23, 42, 250});
    DrawRectangleRoundedLines(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.15f, 6, Color{71, 85, 105, 255});

    // Commuter Avatar & Destination Shape Badge
    DrawCircle(cardX + 28, cardY + 28, 14, commuter->shirtColor);
    DrawCircle(cardX + 28, cardY + 26, 8, Color{255, 224, 178, 255});

    // Mini Metro Destination Shape Badge next to Avatar
    Color sc = GetShapeColor(commuter->targetShape);
    DrawRectangleRounded(Rectangle{(float)cardX + 48, (float)cardY + 12, 18.0f, 18.0f}, 0.2f, 2, sc);
    if (commuter->targetShape == SHAPE_CIRCLE) {
        DrawCircle(cardX + 57, cardY + 21, 5, WHITE);
    } else if (commuter->targetShape == SHAPE_TRIANGLE) {
        DrawTriangle(Vector2{(float)cardX + 57, (float)cardY + 14},
                     Vector2{(float)cardX + 51, (float)cardY + 26},
                     Vector2{(float)cardX + 63, (float)cardY + 26}, WHITE);
    } else if (commuter->targetShape == SHAPE_SQUARE) {
        DrawRectangle(cardX + 53, cardY + 17, 8, 8, WHITE);
    } else if (commuter->targetShape == SHAPE_CROSS) {
        DrawRectangle(cardX + 55, cardY + 15, 4, 12, WHITE);
        DrawRectangle(cardX + 51, cardY + 19, 12, 4, WHITE);
    }

    // Name & Destination
    DrawText(commuter->name.c_str(), cardX + 72, cardY + 13, 13, WHITE);
    DrawGameBoldText(TextFormat("Wants: %s station", GetShapeName(commuter->targetShape)), cardX + 72, cardY + 29, 10, sc);
    DrawText("[x]", cardX + cardW - 22, cardY + 10, 12, Color{148, 163, 184, 255});

    // Mood (RCT style: simple word + thin bar, no percentages or wallet noise)
    const char* moodStr;
    Color moodColor;
    if (commuter->isAngry || commuter->happiness < 55.0f) {
        moodStr = "ANNOYED";
        moodColor = Color{239, 68, 68, 255};
    } else if (commuter->happiness < 80.0f) {
        moodStr = "OK";
        moodColor = Color{234, 179, 8, 255};
    } else {
        moodStr = "HAPPY";
        moodColor = Color{74, 222, 128, 255};
    }

    DrawText("Mood", cardX + 15, cardY + 53, 10, Color{148, 163, 184, 255});
    DrawGameBoldText(moodStr, cardX + 55, cardY + 50, 12, moodColor);
    DrawRectangle(cardX + 112, cardY + 55, 118, 7, Color{30, 41, 59, 255});
    DrawRectangle(cardX + 112, cardY + 55, (int)(118.0f * (commuter->happiness / 100.0f)), 7, moodColor);

    // Current status in plain language
    const char* status = "Waiting to board";
    switch (commuter->state) {
        case COMMUTER_RIDING:        status = "Riding the train"; break;
        case COMMUTER_ENTERING:
        case COMMUTER_SWIPING_GATE:  status = "Arriving at the gate"; break;
        case COMMUTER_EXITING_STATION:
        case COMMUTER_ALIGHTING:     status = "Arrived - going home"; break;
        case COMMUTER_LEAVING_ANGRY: status = "Leaving frustrated"; break;
        default: break;
    }
    DrawText("Status", cardX + 15, cardY + 71, 10, Color{148, 163, 184, 255});
    DrawText(status, cardX + 63, cardY + 69, 11, Color{203, 213, 225, 255});

    // Thoughts speech bubble
    DrawRectangleRounded(Rectangle{(float)cardX + 12, (float)cardY + 92, (float)cardW - 24, 40.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText(TextFormat("\"%s\"", commuter->thought.c_str()), cardX + 18, cardY + 103, 10, Color{255, 214, 0, 255});
}

void UserInterface::DrawStationInspector(const TrackNode* station, int waitingCommuters, float balance) {
    if (!station) return;

    int cardW = 300;
    int cardH = 172;
    int cardX = 14;
    int cardY = 120;

    // Card background
    DrawRectangleRounded(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.15f, 6, Color{15, 23, 42, 250});
    Color borderCol = (station->stationLevel >= 3) ? Color{250, 204, 21, 255} : ((station->stationLevel == 2) ? Color{56, 189, 248, 255} : Color{71, 85, 105, 255});
    DrawRectangleRoundedLines(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.15f, 6, borderCol);

    // Station Shape Badge
    Color sc = GetShapeColor(station->stationShape);
    DrawRectangleRounded(Rectangle{(float)cardX + 14, (float)cardY + 12, 22.0f, 22.0f}, 0.2f, 3, sc);
    if (station->stationShape == SHAPE_CIRCLE) {
        DrawCircle(cardX + 25, cardY + 23, 6, WHITE);
    } else if (station->stationShape == SHAPE_TRIANGLE) {
        DrawTriangle(Vector2{(float)cardX + 25, (float)cardY + 15},
                     Vector2{(float)cardX + 18, (float)cardY + 29},
                     Vector2{(float)cardX + 32, (float)cardY + 29}, WHITE);
    } else if (station->stationShape == SHAPE_SQUARE) {
        DrawRectangle(cardX + 20, cardY + 18, 10, 10, WHITE);
    } else if (station->stationShape == SHAPE_CROSS) {
        DrawRectangle(cardX + 23, cardY + 16, 4, 14, WHITE);
        DrawRectangle(cardX + 18, cardY + 21, 14, 4, WHITE);
    }

    // Station Name & Type Tag
    std::string sName = station->stationName.empty() ? GetShapeName(station->stationShape) : station->stationName;
    DrawText(sName.c_str(), cardX + 44, cardY + 12, 14, WHITE);
    DrawText("TRANSIT CONCOURSE INTERCHANGE", cardX + 44, cardY + 28, 9, Color{148, 163, 184, 255});
    DrawText("[x]", cardX + cardW - 22, cardY + 10, 12, Color{148, 163, 184, 255});

    // Level & Tier Badge
    const char* lvlTag = "LV1 LOCAL PLATFORM (1.0x Fare)";
    Color lvlCol = Color{148, 163, 184, 255};
    if (station->stationLevel == 2) {
        lvlTag = "LV2 MODERN CONCOURSE (+25% Fare Bonus)";
        lvlCol = Color{56, 189, 248, 255};
    } else if (station->stationLevel >= 3) {
        lvlTag = "LV3 GRAND TERMINAL (+50% Fare Bonus)";
        lvlCol = Color{250, 204, 21, 255};
    }
    DrawRectangleRounded(Rectangle{(float)cardX + 14, (float)cardY + 48, (float)cardW - 28, 22.0f}, 0.25f, 3, Color{30, 41, 59, 220});
    DrawGameBoldText(lvlTag, cardX + 22, cardY + 53, 10, lvlCol);

    // Ridership Stats
    DrawText(TextFormat("Riders Delivered: %d passengers", station->passengersServed), cardX + 16, cardY + 78, 11, Color{241, 245, 249, 255});
    DrawText(TextFormat("Waiting Demand:  %d commuters heading here", waitingCommuters), cardX + 16, cardY + 96, 11, Color{203, 213, 225, 255});

    // Concourse Upgrade Action Button
    int btnY = cardY + 122;
    int btnW = cardW - 28;
    int btnH = 34;

    if (station->stationLevel < 3) {
        float cost = (station->stationLevel == 1) ? 250.0f : 500.0f;
        const char* upgText = (station->stationLevel == 1) ? "UPGRADE TO MODERN CONCOURSE ($250)" : "UPGRADE TO GRAND TERMINAL ($500)";
        bool canAfford = (balance >= cost);

        Color btnBg = canAfford ? Color{34, 197, 94, 240} : Color{51, 65, 85, 200};
        Color btnBorder = canAfford ? Color{74, 222, 128, 255} : Color{71, 85, 105, 255};
        Color textCol = canAfford ? WHITE : Color{148, 163, 184, 255};

        DrawRectangleRounded(Rectangle{(float)cardX + 14, (float)btnY, (float)btnW, (float)btnH}, 0.28f, 4, btnBg);
        DrawRectangleRoundedLines(Rectangle{(float)cardX + 14, (float)btnY, (float)btnW, (float)btnH}, 0.28f, 4, btnBorder);
        DrawGameBoldText(upgText, cardX + 24, btnY + 10, 10, textCol);
    } else {
        DrawRectangleRounded(Rectangle{(float)cardX + 14, (float)btnY, (float)btnW, (float)btnH}, 0.28f, 4, Color{234, 179, 8, 40});
        DrawRectangleRoundedLines(Rectangle{(float)cardX + 14, (float)btnY, (float)btnW, (float)btnH}, 0.28f, 4, Color{250, 204, 21, 220});
        DrawGameBoldText("* MAXIMUM CONCOURSE LEVEL *", cardX + 28, btnY + 10, 11, Color{250, 204, 21, 255});
    }
}

void UserInterface::DrawToast(const ToastMessage& toast) {
    if (toast.timer <= 0.0f) return;

    int screenW = GetScreenWidth();
    int toastW = 520;
    int toastH = 40;
    int toastX = (screenW - toastW) / 2;
    int toastY = 62;

    DrawRectangleRounded(Rectangle{(float)toastX, (float)toastY, (float)toastW, (float)toastH}, 0.3f, 4, Color{15, 23, 42, 248});
    DrawRectangleRoundedLines(Rectangle{(float)toastX, (float)toastY, (float)toastW, (float)toastH}, 0.3f, 4, toast.color);

    DrawText(toast.text.c_str(), toastX + 16, toastY + 11, 13, WHITE);
}

void UserInterface::DrawQuickTipBanner(const std::string& tip) {
    if (tip.empty()) return;
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int textW = MeasureText(tip.c_str(), 13);
    int bannerW = textW + 36;
    int bannerH = 30;
    int bannerX = (screenW - bannerW) / 2;
    int barY = ToolbarMetrics::BarY(screenH);
    int bannerY = barY - ToolbarMetrics::TAB_H - bannerH - 8;

    DrawRectangleRounded(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{15, 23, 42, 235});
    DrawRectangleRoundedLines(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{56, 189, 248, 200});

    DrawCircle(bannerX + 14, bannerY + 15, 3.5f, Color{255, 214, 0, 255});
    DrawText(tip.c_str(), bannerX + 24, bannerY + 9, 13, Color{241, 245, 249, 255});
}


void UserInterface::DrawTransitOperationsManual() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 225});

    int boxW = 740;
    int boxH = 490;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.08f, 8, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.08f, 8, Color{56, 189, 248, 255});

    // Header banner
    DrawRectangleRounded(Rectangle{(float)bx + 2, (float)by + 2, (float)boxW - 4, 38.0f}, 0.08f, 6, Color{30, 41, 59, 255});
    DrawText("HOW TO PLAY: METRO OPERATOR'S QUICK GUIDE", bx + 20, by + 12, 15, Color{255, 214, 0, 255});
    DrawText("[X] CLOSE", bx + boxW - 85, by + 13, 12, Color{148, 163, 184, 255});

    // 3 Step-by-Step Cards
    int cardW = 224;
    int cardH = 240;
    int cardY = by + 48;
    int card1X = bx + 16;
    int card2X = card1X + cardW + 14;
    int card3X = card2X + cardW + 14;

    // CARD 1: TRACK & BRIDGES
    DrawRectangleRounded(Rectangle{(float)card1X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)card1X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{56, 189, 248, 255});
    DrawRectangleRounded(Rectangle{(float)card1X + 4, (float)cardY + 4, (float)cardW - 8, 24.0f}, 0.2f, 4, Color{14, 116, 144, 255});
    DrawGameBoldText("1: TRACKS, LOOPS & BRIDGES", card1X + 8, cardY + 10, 11, WHITE);

    int c1y = cardY + 34;
    DrawText("* [1] Straight   [2/3] Turns (90 deg)", card1X + 10, c1y, 11, Color{255, 214, 0, 255}); c1y += 16;
    DrawText("* [4] Viaduct: bridges over rivers/bays!", card1X + 10, c1y, 11, Color{56, 189, 248, 255}); c1y += 16;
    DrawText("* [5] Slopes   [6] Mountain Tunnels", card1X + 10, c1y, 11, Color{255, 214, 0, 255}); c1y += 16;
    DrawText("* [R] Rotate Heading (N/E/S/W)", card1X + 10, c1y, 11, Color{203, 213, 225, 255}); c1y += 18;
    DrawText("* [C] AUTO-LOOP GAP BRIDGER:", card1X + 10, c1y, 11, Color{34, 197, 94, 255}); c1y += 15;
    DrawText("Press [C] or click red top badge", card1X + 10, c1y, 10, Color{74, 222, 128, 255}); c1y += 14;
    DrawText("to auto-pathfind and close the loop!", card1X + 10, c1y, 10, Color{74, 222, 128, 255}); c1y += 17;
    DrawText("* [E / Q] Elevation Level (Z=0..5)", card1X + 10, c1y, 10, Color{203, 213, 225, 255}); c1y += 14;
    DrawText("Build multi-level flyover viaducts!", card1X + 10, c1y, 10, Color{203, 213, 225, 255});

    // CARD 2: STATIONS & EXPANSION
    DrawRectangleRounded(Rectangle{(float)card2X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)card2X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{52, 211, 153, 255});
    DrawRectangleRounded(Rectangle{(float)card2X + 4, (float)cardY + 4, (float)cardW - 8, 24.0f}, 0.2f, 4, Color{16, 149, 106, 255});
    DrawGameBoldText("2: STATIONS & TERRAFORMING", card2X + 6, cardY + 10, 11, WHITE);

    int c2y = cardY + 34;
    DrawText("* [7] Station Platforms (PSD doors)", card2X + 10, c2y, 11, Color{255, 214, 0, 255}); c2y += 16;
    DrawText("* [8] 3-Aspect Wayside Block Signals", card2X + 10, c2y, 11, Color{255, 214, 0, 255}); c2y += 16;
    DrawText("Prevents collisions with multiple EMUs.", card2X + 10, c2y, 10, Color{203, 213, 225, 255}); c2y += 18;
    DrawText("* TERRAFORM & RECLAIM LAND:", card2X + 10, c2y, 11, Color{52, 211, 153, 255}); c2y += 15;
    DrawText("Concourse [4] Reclaim fills water to land!", card2X + 10, c2y, 10, Color{74, 222, 128, 255}); c2y += 14;
    DrawText("[5] Canal digs rivers; [8] Raises Hills +1 Z!", card2X + 10, c2y, 10, Color{74, 222, 128, 255}); c2y += 18;
    DrawText("* EXPAND DISTRICT ($500 button):", card2X + 10, c2y, 10, Color{56, 189, 248, 255}); c2y += 14;
    DrawText("Converts outer water into developable land!", card2X + 10, c2y, 10, Color{203, 213, 225, 255});

    // CARD 3: COMMUTERS & FLEET
    DrawRectangleRounded(Rectangle{(float)card3X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)card3X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{251, 146, 60, 255});
    DrawRectangleRounded(Rectangle{(float)card3X + 4, (float)cardY + 4, (float)cardW - 8, 24.0f}, 0.2f, 4, Color{194, 65, 12, 255});
    DrawGameBoldText("3: COMMUTERS & FLEET", card3X + 14, cardY + 10, 11, WHITE);

    int c3y = cardY + 34;
    DrawText("* Commuters have destination badges:", card3X + 10, c3y, 11, Color{255, 214, 0, 255}); c3y += 16;
    DrawText("Square CBD / Circle Suburb / Triangle", card3X + 10, c3y, 10, Color{203, 213, 225, 255}); c3y += 14;
    DrawText("Waterfront / Cross Hospital. Match shapes!", card3X + 10, c3y, 10, Color{203, 213, 225, 255}); c3y += 18;
    DrawText("* CAR+1: Increases train passenger capacity.", card3X + 10, c3y, 10, Color{56, 189, 248, 255}); c3y += 16;
    DrawText("* EXTRA TRAIN: Dispatches extra EMU trains", card3X + 10, c3y, 10, Color{56, 189, 248, 255}); c3y += 14;
    DrawText("on the circuit to handle peak rush hours!", card3X + 10, c3y, 10, Color{56, 189, 248, 255}); c3y += 18;
    DrawText("* Deliver 500 riders to win, then keep", card3X + 10, c3y, 10, Color{74, 222, 128, 255}); c3y += 14;
    DrawText("growing in Endless Sandbox Tycoon mode!", card3X + 10, c3y, 10, Color{74, 222, 128, 255});

    // BOTTOM SHORTCUTS BOX
    int scY = cardY + cardH + 10;
    int scH = 138;
    DrawRectangleRounded(Rectangle{(float)bx + 16, (float)scY, (float)boxW - 32, (float)scH}, 0.08f, 4, Color{23, 32, 51, 240});
    DrawRectangleRoundedLines(Rectangle{(float)bx + 16, (float)scY, (float)boxW - 32, (float)scH}, 0.08f, 4, Color{51, 65, 85, 255});

    DrawGameBoldText("OCC DISPATCHER KEYBOARD & MOUSE CONTROLS", bx + 28, scY + 8, 11, Color{255, 214, 0, 255});

    int kCol1 = bx + 28;
    int kCol2 = bx + 380;
    int ky = scY + 28;

    DrawText("WASD / Arrows: pan map   Right/Middle drag: grab map   Wheel: zoom", kCol1, ky, 10, Color{203, 213, 225, 255});
    DrawText("Tab 1 / 2 / 3 : Switch Track / Concourse / Scenery", kCol2, ky, 10, Color{203, 213, 225, 255});
    ky += 16;
    DrawText("[1] - [8] Tools: Rails / Reclaim Water / Canal / Sand / Stone / Hill", kCol1, ky, 10, Color{203, 213, 225, 255});
    DrawText("[L]           : Cycle Line Livery Theme (Red/Blue/Green/etc.)", kCol2, ky, 10, Color{250, 204, 21, 255});
    ky += 16;
    DrawText("[C] / [HOME]  : Recenter Camera to Train & Central Station", kCol1, ky, 10, Color{34, 197, 94, 255});
    DrawText("[N]           : Toggle Night Vista Mode (City Lamps & Headlights)", kCol2, ky, 10, Color{147, 197, 253, 255});
    ky += 16;
    DrawText("[B]           : Auto-Bridge Track Loop Gap", kCol1, ky, 10, Color{56, 189, 248, 255});
    DrawText("E / Q         : Raise / Lower Track Elevation (Z=0..5)", kCol2, ky, 10, Color{203, 213, 225, 255});
    ky += 16;
    DrawText("EXPAND ($500) : Converts outer water into land & expands territory", kCol1, ky, 10, Color{52, 211, 153, 255});
    DrawText("X / Bulldozer : Demolish track & flatten raised terrain", kCol2, ky, 10, Color{239, 68, 68, 255});

    // CLOSE BUTTON
    int btnW = 240;
    int btnH = 32;
    int btnX = bx + (boxW - btnW) / 2;
    int btnY = by + boxH - 42;

    DrawRectangleRounded(Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH}, 0.3f, 4, Color{220, 38, 38, 255});
    DrawGameBoldText("START DISPATCHING (CLOSE)", btnX + 24, btnY + 10, 11, WHITE);
}

void UserInterface::DrawWeeklyModal(const std::vector<UpgradeChoice>& choices, int hoveredChoice) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 200});

    int modalW = 700;
    int modalH = 340;
    int mx = (screenW - modalW) / 2;
    int my = (screenH - modalH) / 2;

    DrawRectangleRounded(Rectangle{(float)mx, (float)my, (float)modalW, (float)modalH}, 0.1f, 8, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)mx, (float)my, (float)modalW, (float)modalH}, 0.1f, 8, Color{220, 38, 38, 255});

    DrawGameBoldText("TRANSIT AUTHORITY EXPANSION GRANT", mx + 155, my + 24, 20, Color{255, 179, 0, 255});
    DrawText("Select 1 capital upgrade grant to expand urban network capacity:", mx + 150, my + 54, 13, Color{148, 163, 184, 255});

    int cardW = 195;
    int cardH = 200;
    int cardY = my + 90;
    int gap = 25;
    int startX = mx + (modalW - (3 * cardW + 2 * gap)) / 2;

    for (int i = 0; i < 3 && i < (int)choices.size(); ++i) {
        int cx = startX + i * (cardW + gap);
        bool hovered = (hoveredChoice == i);

        Color cardBg = hovered ? Color{30, 41, 59, 255} : Color{23, 32, 51, 240};
        Color borderC = hovered ? choices[i].accentColor : Color{71, 85, 105, 255};

        DrawRectangleRounded(Rectangle{(float)cx, (float)cardY, (float)cardW, (float)cardH}, 0.15f, 6, cardBg);
        DrawRectangleRoundedLines(Rectangle{(float)cx, (float)cardY, (float)cardW, (float)cardH}, 0.15f, 6, borderC);

        DrawRectangleRounded(Rectangle{(float)cx + 10, (float)cardY + 12, (float)cardW - 20, 24.0f}, 0.3f, 4, choices[i].accentColor);
        DrawText(choices[i].perkTag.c_str(), cx + 18, cardY + 17, 11, WHITE);

        DrawText(choices[i].title.c_str(), cx + 14, cardY + 48, 14, WHITE);
        DrawText(choices[i].description.c_str(), cx + 14, cardY + 78, 12, Color{203, 213, 225, 255});

        DrawRectangleRounded(Rectangle{(float)cx + 20, (float)cardY + cardH - 38, (float)cardW - 40, 26.0f}, 0.3f, 4, hovered ? choices[i].accentColor : Color{51, 65, 85, 255});
        DrawText("AUTHORIZE", cx + 64, cardY + cardH - 31, 11, WHITE);
    }
}

void UserInterface::DrawGameOver(int finalRidership, int stars, int best, float stateEntryTime) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    float elapsed = GetTime() - stateEntryTime;
    float fadeIn = std::min(1.0f, elapsed / 0.5f);

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, (unsigned char)(220 * fadeIn)});

    // Warning spark particles
    float t = GetTime();
    for (int i = 0; i < 12; ++i) {
        float px = fmodf(t * (15 + i * 4) + i * 197.3f, (float)screenW);
        float py = fmodf(t * (10 + i * 3) + i * 113.7f, (float)screenH);
        float sz = 1.5f + sinf(t * 4.0f + i) * 1.0f;
        DrawCircle((int)px, (int)py, sz, Color{239, 68, 68, 150});
    }

    int boxW = 500;
    int boxH = 300;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;
    float slideOffset = 40.0f * (1.0f - fadeIn);
    by += (int)slideOffset;

    // Glowing red border
    float glow = 0.5f + 0.5f * sinf(t * 4.0f);
    DrawRectangleRounded(Rectangle{(float)bx - 3, (float)by - 3, (float)boxW + 6, (float)boxH + 6}, 0.15f, 6, Color{239, 68, 68, (unsigned char)(25 + 25 * glow)});
    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{15, 23, 42, 250});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{239, 68, 68, 255});

    // Warning icon
    float fl = 0.6f + 0.4f * sinf(t * 5.0f);
    DrawTriangle({(float)(bx + boxW / 2), (float)(by + 12)}, {(float)(bx + boxW / 2 - 16), (float)(by + 44)}, {(float)(bx + boxW / 2 + 16), (float)(by + 44)}, Color{239, 68, 68, 255});
    DrawText("!", bx + boxW / 2 - 3, by + 20, 18, WHITE);

    DrawGameBoldText("NETWORK GRIDLOCK", bx + 110, by + 56, 24, Color{239, 68, 68, (unsigned char)(255 * fl)});
    DrawText("Platforms overcrowded and service collapsed.", bx + 100, by + 90, 12, Color{203, 213, 225, 255});

    DrawGameBoldText(TextFormat("%d commuters transported", finalRidership), bx + 120, by + 118, 16, Color{255, 214, 0, 255});

    // Star rating
    char starsStr[16];
    for (int i = 0; i < 3; ++i) starsStr[i] = (i < stars) ? '*' : ' ';
    starsStr[3] = '\0';
    DrawGameBoldText(starsStr, bx + boxW / 2 - 30, by + 145, 28, Color{255, 214, 0, 255});

    DrawText(TextFormat("session best: %d riders", best), bx + 155, by + 180, 12, Color{148, 163, 184, 255});

    // Restart button (red glow)
    int btnW = 220;
    int btnH = 44;
    int btnX = bx + (boxW - btnW) / 2;
    int btnY = by + 210;
    DrawRectangleRounded(Rectangle{(float)btnX - 2, (float)btnY - 2, (float)btnW + 4, (float)btnH + 4}, 0.3f, 4, Color{239, 68, 68, (unsigned char)(50 + 30 * glow)});
    DrawRectangleRounded(Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH}, 0.3f, 4, Color{220, 38, 38, 255});
    DrawGameBoldText("RESTART NETWORK", btnX + 28, btnY + 13, 16, WHITE);
}

void UserInterface::DrawVictory(int finalRidership, int weeks, int stars, float balance, int best, float stateEntryTime) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    float elapsed = GetTime() - stateEntryTime;
    float fadeIn = std::min(1.0f, elapsed / 0.5f);

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, (unsigned char)(215 * fadeIn)});

    // Celebration particles (confetti sparkles)
    float t = GetTime();
    for (int i = 0; i < 20; ++i) {
        float px = fmodf(t * (30 + i * 7) + i * 137.5f, (float)screenW);
        float py = fmodf(t * (20 + i * 5) + i * 89.3f, (float)screenH);
        float sz = 2.0f + sinf(t * 3.0f + i) * 1.5f;
        Color c = (i % 3 == 0) ? Color{255, 214, 0, 180} : ((i % 3 == 1) ? Color{34, 197, 94, 180} : Color{56, 189, 248, 180});
        DrawCircle((int)px, (int)py, sz, c);
    }

    int boxW = 620;
    int boxH = 360;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;
    // Slide-up entrance: start 40px below, ease to target
    float slideOffset = 40.0f * (1.0f - fadeIn);
    by += (int)slideOffset;

    // Glowing border effect
    float glow = 0.5f + 0.5f * sinf(t * 3.0f);
    DrawRectangleRounded(Rectangle{(float)bx - 3, (float)by - 3, (float)boxW + 6, (float)boxH + 6}, 0.15f, 6, Color{34, 197, 94, (unsigned char)(30 + 30 * glow)});
    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{15, 23, 42, 250});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{34, 197, 94, 255});

    float fl = 0.6f + 0.4f * sinf(t * 4.0f);
    // Trophy icon
    DrawRectangle(bx + boxW / 2 - 20, by + 10, 40, 6, Color{255, 214, 0, 255});
    DrawRectangle(bx + boxW / 2 - 12, by + 16, 24, 18, Color{255, 214, 0, 255});
    DrawRectangle(bx + boxW / 2 - 4, by + 34, 8, 6, Color{255, 214, 0, 255});
    DrawRectangle(bx + boxW / 2 - 10, by + 40, 20, 4, Color{255, 214, 0, 255});

    DrawGameBoldText("TRANSIT TRIUMPH!", bx + 160, by + 55, 26, Color{34, 197, 94, (unsigned char)(255 * fl)});
    DrawGameBoldText(TextFormat("%d COMMUTERS DELIVERED!", finalRidership), bx + 140, by + 88, 18, Color{255, 214, 0, 255});
    DrawText("Your rapid transit network connected all districts flawlessly!", bx + 70, by + 118, 12, Color{203, 213, 225, 255});

    // Star rating
    char starsStr[16];
    for (int i = 0; i < 3; ++i) starsStr[i] = (i < stars) ? '*' : ' ';
    starsStr[3] = '\0';
    DrawGameBoldText(starsStr, bx + boxW / 2 - 30, by + 140, 30, Color{255, 214, 0, 255});
    // Extra celebration burst for 3-star perfect score
    if (stars >= 3) {
        for (int i = 0; i < 30; ++i) {
            float px = fmodf(t * (40 + i * 9) + i * 157.3f, (float)screenW);
            float py = fmodf(t * (25 + i * 6) + i * 93.7f, (float)screenH);
            float sz = 2.5f + sinf(t * 4.0f + i) * 2.0f;
            Color c = (i % 4 == 0) ? Color{255, 214, 0, 220} : ((i % 4 == 1) ? Color{34, 197, 94, 220} : ((i % 4 == 2) ? Color{56, 189, 248, 220} : Color{239, 68, 68, 180}));
            DrawCircle((int)px, (int)py, sz, c);
        }
    }

    // PERFECT label for 3-star
    if (stars >= 3) {
        float pfl = 0.5f + 0.5f * sinf(t * 4.0f);
        DrawGameBoldText("PERFECT!", bx + boxW / 2 - 35, by + 170, 14, Color{255, 214, 0, (unsigned char)(200 + 55 * pfl)});
    }

    DrawText(TextFormat("%d weeks | $%.0f budget | best: %d", weeks, balance, best), bx + 130, by + 180, 12, Color{148, 163, 184, 255});
    // NEW BEST flash
    if (finalRidership > best && best > 0) {
        float nbPulse = 0.5f + 0.5f * sinf(t * 5.0f);
        DrawGameBoldText("NEW BEST!", bx + boxW / 2 + 40, by + 176, 16, Color{255, 214, 0, (unsigned char)(200 + 55 * nbPulse)});
    }

    // Continue button (green glow)
    int btnW = 360;
    int btnH = 44;
    int btn1X = bx + (boxW - btnW) / 2;
    int btn1Y = by + 210;
    DrawRectangleRounded(Rectangle{(float)btn1X - 2, (float)btn1Y - 2, (float)btnW + 4, (float)btnH + 4}, 0.3f, 4, Color{34, 197, 94, (unsigned char)(60 + 30 * glow)});
    DrawRectangleRounded(Rectangle{(float)btn1X, (float)btn1Y, (float)btnW, (float)btnH}, 0.3f, 4, Color{16, 185, 129, 255});
    DrawGameBoldText("CONTINUE IN ENDLESS MODE", btn1X + 50, btn1Y + 13, 16, WHITE);

    // Restart button
    int btn2W = 200;
    int btn2H = 32;
    int btn2X = bx + (boxW - btn2W) / 2;
    int btn2Y = by + 274;
    DrawRectangleRounded(Rectangle{(float)btn2X, (float)btn2Y, (float)btn2W, (float)btn2H}, 0.3f, 4, Color{51, 65, 85, 255});
    DrawRectangleRoundedLines(Rectangle{(float)btn2X, (float)btn2Y, (float)btn2W, (float)btn2H}, 0.3f, 4, Color{71, 85, 105, 255});
    DrawText("START NEW NETWORK", btn2X + 24, btn2Y + 8, 13, Color{203, 213, 225, 255});
}

void UserInterface::DrawPauseOverlay() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    float t = GetTime();

    // Dark overlay with subtle vignette
    DrawRectangle(0, 0, screenW, screenH, Color{2, 6, 23, 230});
    DrawRectangleGradientV(0, 0, screenW, 80, Color{2, 6, 23, 0}, Color{2, 6, 23, 140});
    DrawRectangleGradientV(0, screenH - 80, screenW, 80, Color{2, 6, 23, 0}, Color{2, 6, 23, 140});

    float fl = 0.5f + 0.5f * sinf(t * 3.0f);
    int cx = screenW / 2;

    // Header
    DrawGameBoldTextCentered("METRO GRID - PAUSED", (float)cx, 40, 34, Color{255, 214, 0, (unsigned char)(200 + 55 * fl)});
    DrawText("DISPATCH CONSOLE SUSPENDED - REVIEW OPERATOR GUIDE BELOW", cx - 215, 80, 13, Color{148, 163, 184, 255});

    // Instructions & Quick Reference Card (2 Columns)
    int cardW = 900;
    int cardH = 345;
    int cardX = (screenW - cardW) / 2;
    int cardY = 105;

    DrawRectangleRounded(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.06f, 6, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.06f, 6, Color{56, 189, 248, 180});

    // Column 1: Core Mechanics & Tips
    int col1X = cardX + 24;
    int col2X = cardX + cardW / 2 + 10;
    int rowY = cardY + 16;

    DrawGameBoldText("HOW TO PLAY & PRO TIPS", col1X, rowY, 15, Color{255, 214, 0, 255});
    rowY += 26;

    struct TipItem { const char* title; const char* desc; Color col; };
    TipItem tips[] = {
        {"1. Complete Closed Loops", "Trains run automatically only when tracks form a closed circuit.", Color{34, 197, 94, 255}},
        {"2. 1-Click Auto-Loop [C]", "Press [C] or click the red top badge to auto-bridge open track gaps!", Color{56, 189, 248, 255}},
        {"3. Stations & Shapes", "Commuters board to reach matching shapes (Square CBD, Circle Suburb, etc.).", Color{251, 146, 60, 255}},
        {"4. Bridges & Viaducts", "Use Viaduct [4] or raise rails [E] to build bridges across rivers and bays.", Color{168, 85, 247, 255}},
        {"5. Expand Territory", "Click EXPAND ($500) to unlock new districts. Natural water is never erased!", Color{52, 211, 153, 255}},
        {"6. Fleet & Endless Mode", "Add cars (CAR+1) and buy extra trains for rush hours. Play infinitely!", Color{245, 158, 11, 255}}
    };

    for (int i = 0; i < 6; ++i) {
        DrawCircle(col1X + 4, rowY + 6, 3, tips[i].col);
        DrawGameBoldText(tips[i].title, col1X + 14, rowY, 13, tips[i].col);
        rowY += 16;
        DrawText(tips[i].desc, col1X + 14, rowY, 11, Color{203, 213, 225, 255});
        rowY += 23;
    }

    // Column 2: Controls
    int rRowY = cardY + 16;
    DrawGameBoldText("CONTROLS & SHORTCUTS", col2X, rRowY, 15, Color{56, 189, 248, 255});
    rRowY += 26;

    struct KeyMap { const char* key; const char* desc; };
    KeyMap keys[] = {
        {"WASD / Arrow Keys", "Pan map across the 48x48 world"},
        {"Right / Middle Drag", "Grab and drag map view"},
        {"Mouse Scroll", "Zoom in / Zoom out (0.65x - 2.4x)"},
        {"[1] - [8]", "Select track piece / station / signal"},
        {"[R]", "Rotate track / scenery heading"},
        {"[C] / [HOME]", "Recenter camera to train & station"},
        {"[B]", "Auto-Bridge open track loop gap"},
        {"[E] / [Q]", "Raise / Lower elevation (Z=0..5)"},
        {"[TAB]", "Switch category (Track / Concourse / Scenery)"},
        {"[X]", "Demolish tool (refunds cash)"},
        {"[?]", "Open Transit Operations Manual"},
        {"[P] / [ESC]", "Pause / Resume simulation"}
    };

    for (int k = 0; k < 12; ++k) {
        DrawText(keys[k].key, col2X, rRowY, 11, Color{255, 214, 0, 255});
        DrawText(keys[k].desc, col2X + 130, rRowY, 11, Color{203, 213, 225, 255});
        rRowY += 21;
    }

    // Action Buttons at bottom
    int by = cardY + cardH + 18;

    // RESUME (primary, green glow)
    float glow = 0.5f + 0.5f * sinf(t * 4.0f);
    DrawRectangleRounded(Rectangle{(float)cx - 260, (float)by, 160.0f, 44.0f}, 0.3f, 4, Color{34, 197, 94, (unsigned char)(40 + 30 * glow)});
    DrawRectangleRounded(Rectangle{(float)cx - 258, (float)by + 2, 156.0f, 40.0f}, 0.3f, 4, Color{16, 185, 129, 255});
    DrawRectangleRoundedLines(Rectangle{(float)cx - 258, (float)by + 2, 156.0f, 40.0f}, 0.3f, 4, Color{209, 250, 229, 255});
    DrawGameBoldTextCentered("RESUME  [ESC]", (float)cx - 180, (float)by + 14, 15, WHITE);

    // RESTART (secondary)
    DrawRectangleRounded(Rectangle{(float)cx - 80, (float)by + 2, 160.0f, 40.0f}, 0.3f, 4, Color{51, 65, 85, 255});
    DrawRectangleRoundedLines(Rectangle{(float)cx - 80, (float)by + 2, 160.0f, 40.0f}, 0.3f, 4, Color{71, 85, 105, 255});
    DrawGameBoldTextCentered("RESTART", (float)cx, (float)by + 14, 15, Color{203, 213, 225, 255});

    // QUIT TO MENU (secondary)
    DrawRectangleRounded(Rectangle{(float)cx + 100, (float)by + 2, 160.0f, 40.0f}, 0.3f, 4, Color{51, 65, 85, 255});
    DrawRectangleRoundedLines(Rectangle{(float)cx + 100, (float)by + 2, 160.0f, 40.0f}, 0.3f, 4, Color{71, 85, 105, 255});
    DrawGameBoldTextCentered("QUIT TO MENU", (float)cx + 180, (float)by + 14, 14, Color{203, 213, 225, 255});
}

int UserInterface::CheckPauseClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int cx = screenW / 2;
    int by = 105 + 345 + 18; // cardY + cardH + 18 = 468
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx - 258, (float)by + 2, 156.0f, 40.0f})) return 0;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx - 80, (float)by + 2, 160.0f, 40.0f})) return 1;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx + 100, (float)by + 2, 160.0f, 40.0f})) return 2;
    return -1;
}

void UserInterface::DrawObjectiveChip(const char* title, const char* sub, float progressPct) {
    int w = 250;
    int h = progressPct >= 0.0f ? 52 : 38;
    int x = 12;
    int y = 62;
    DrawRectangleRounded(Rectangle{(float)x, (float)y, (float)w, (float)h}, 0.22f, 6, Color{15, 23, 42, 235});
    DrawRectangleRoundedLines(Rectangle{(float)x, (float)y, (float)w, (float)h}, 0.22f, 6, Color{255, 214, 0, 160});
    DrawGameBoldText(title, x + 12, y + 7, 15, Color{255, 214, 0, 255});
    DrawText(sub, x + 12, (progressPct >= 0.0f) ? y + 28 : y + 22, 12, Color{203, 213, 225, 255});
    if (progressPct >= 0.0f) {
        float p = std::max(0.0f, std::min(1.0f, progressPct));
        DrawRectangle(x + 12, y + 44, w - 24, 4, Color{51, 65, 85, 255});
        DrawRectangle(x + 12, y + 44, (int)((w - 24) * p), 4, Color{74, 222, 128, 255});
    }
}

void UserInterface::DrawTitleScreen(int best) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int cx = screenW / 2;
    float t = GetTime();

    // 1. Dark background with animated isometric grid
    DrawRectangle(0, 0, screenW, screenH, Color{15, 23, 42, 255});
    // Animated gradient sweep (slow vertical pan)
    float sweepY = fmodf(t * 8.0f, (float)(screenH + 200)) - 100;
    DrawRectangleGradientV(0, (int)sweepY, screenW, 120, Color{56, 189, 248, 12}, Color{15, 23, 42, 0});
    // Animated diagonal grid lines (parallax drift)
    float drift = fmodf(t * 15.0f, 80.0f);
    for (int x = -100; x < screenW + 300; x += 80)
        DrawLine(x - (int)drift, 0, x - 180 - (int)drift, screenH, Color{30, 41, 59, 50});
    for (int y = 0; y < screenH; y += 80)
        DrawLine(0, y, screenW, y + 40, Color{30, 41, 59, 40});

    // Animated floating dots (commuter-like)
    for (int i = 0; i < 12; ++i) {
        float px = fmodf(t * (6 + i * 2) + i * 173.7f, (float)screenW);
        float py = fmodf(t * (3 + i * 1.5f) + i * 127.3f, (float)screenH);
        float sz = 1.0f + sinf(t * 2.0f + i) * 0.5f;
        Color c = (i % 3 == 0) ? Color{34, 197, 94, 50} : ((i % 3 == 1) ? Color{255, 214, 0, 50} : Color{56, 189, 248, 50});
        DrawCircle((int)px, (int)py, sz, c);
    }

    // Moving train silhouette across bottom
    float trainX = fmodf(t * 60.0f, (float)(screenW + 200)) - 100;
    float trainY = screenH - 60;
    // Train body
    DrawRectangle((int)trainX, (int)trainY, 60, 10, Color{220, 38, 38, 80});
    DrawRectangle((int)trainX + 60, (int)trainY, 50, 10, Color{220, 38, 38, 60});
    DrawRectangle((int)trainX + 110, (int)trainY, 50, 10, Color{220, 38, 38, 40});
    // Windows
    for (int w = 0; w < 4; ++w) {
        DrawRectangle((int)trainX + 6 + w * 14, (int)trainY + 2, 8, 5, Color{255, 238, 88, 60});
    }
    // Track line
    DrawLine(0, (int)trainY + 12, screenW, (int)trainY + 12, Color{51, 65, 85, 80});

    // Top accent bar
    DrawRectangle(0, 0, screenW, 4, Color{220, 38, 38, 255});

    // 2. Title
    int headerY = 60;
    DrawCircle(cx - 160, headerY + 14, 18, Color{220, 38, 38, 255});
    DrawCircle(cx - 160, headerY + 14, 12, Color{15, 23, 42, 255});
    DrawRectangle(cx - 182, headerY + 9, 44, 10, Color{220, 38, 38, 255});
    DrawText("M", cx - 170, headerY + 5, 10, WHITE);
    DrawGameBoldText("METRO GRID", cx - 130, headerY - 2, 32, Color{248, 250, 252, 255});
    DrawText("Urban Transit Tycoon", cx - 65, headerY + 30, 13, Color{148, 163, 184, 255});

    // 3. Tagline
    float pulse = 0.5f + 0.5f * sinf(t * 2.0f);
    DrawGameBoldText("Build tracks. Run trains. Grow your city.", cx - 220, headerY + 65, 16,
                     Color{56, 189, 248, (unsigned char)(180 + 75 * pulse)});

    // 4. Three simple one-liner rules
    int ruleY = headerY + 110;
    int ruleGap = 32;
    const char* rules[] = {
        "Build closed loops so your train runs automatically",
        "Riders board at matching shape stations  (square / circle / cross)",
        "Deliver 500 commuters to win, then keep growing"
    };
    Color ruleColors[] = {
        Color{34, 197, 94, 255},
        Color{255, 214, 0, 255},
        Color{56, 189, 248, 255}
    };
    for (int i = 0; i < 3; ++i) {
        DrawCircle(cx - 240, ruleY + i * ruleGap + 6, 4, ruleColors[i]);
        DrawText(rules[i], cx - 228, ruleY + i * ruleGap - 2, 14, Color{226, 232, 240, 255});
    }

    // 5. Big START button (with glow)
    int btnW = 320;
    int btnH = 52;
    int btnX = (screenW - btnW) / 2;
    int btnY = ruleY + 3 * ruleGap + 20;

    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH});
    Color btnBg = isHovered ? Color{239, 68, 68, 255} : Color{220, 38, 38, 255};
    float btnGlow = isHovered ? 0.8f : (0.3f + 0.2f * sinf(t * 3.0f));
    DrawRectangleRounded(Rectangle{(float)btnX - 3, (float)btnY - 3, (float)btnW + 6, (float)btnH + 6}, 0.3f, 6, Color{239, 68, 68, (unsigned char)(btnGlow * 100)});
    DrawRectangleRounded(Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH}, 0.3f, 6, btnBg);
    DrawRectangleRoundedLines(Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH}, 0.3f, 6, isHovered ? WHITE : Color{254, 202, 202, 255});
    DrawGameBoldText("START PLAYING", btnX + 80, btnY + 14, 20, WHITE);
    DrawText("or press SPACE", btnX + 110, btnY + 36, 11, Color{255, 214, 0, 200});

    // 6. High score
    float bf = 0.5f + 0.5f * sinf(GetTime() * 3.0f);
    DrawGameBoldText(TextFormat("BEST: %d riders", best), cx - 60, btnY + btnH + 14, 14,
                     Color{255, 214, 0, (unsigned char)(180 + 75 * bf)});

    // 7. Minimal controls line
    DrawText("[WASD] Pan Map   [1-8] Build   [R] Rotate   [C] Auto-Loop / Recenter   [X] Demolish   [?] Help", cx - 310, screenH - 28, 12, Color{100, 116, 139, 255});
}

bool UserInterface::CheckTitleStartClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int btnW = 320;
    int btnH = 52;
    int btnX = (screenW - btnW) / 2;
    int btnY = 286; // matches DrawTitleScreen layout

    return CheckCollisionPointRec(mousePos, Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH});
}

int UserInterface::CheckToolbarTabClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barX = ToolbarMetrics::BarX(screenW);
    int barY = ToolbarMetrics::BarY(screenH);

    int tabW = 150;
    int tabH = ToolbarMetrics::TAB_H;
    int tabStartY = barY - tabH + 2;

    for (int t = 0; t < 3; ++t) {
        int tx = barX + 16 + t * (tabW + 10);
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4})) {
            return t;
        }
    }
    return -1;
}

int UserInterface::CheckToolbarItemClick(Vector2 mousePos, ToolCategory activeTab) const {
    (void)activeTab;
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = ToolbarMetrics::BAR_W;
    int barX = ToolbarMetrics::BarX(screenW);
    int barY = ToolbarMetrics::BarY(screenH);

    int itemBtnW = ToolbarMetrics::ITEM_W;
    int itemBtnH = ToolbarMetrics::ITEM_H;
    int itemStartY = barY + ToolbarMetrics::ITEM_Y;
    int spacing = 10;
    int startX = barX + (barW - (9 * itemBtnW + 8 * spacing)) / 2;

    for (int i = 0; i < 9; ++i) {
        int bx = startX + i * (itemBtnW + spacing);
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH})) {
            return i;
        }
    }
    return -1;
}

bool UserInterface::CheckToolbarAuxClick(Vector2 mousePos, int& outZDelta, bool& outRotate, int& outBuy) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barY = ToolbarMetrics::BarY(screenH);
    int auxX = ToolbarMetrics::AuxX(screenW);
    int buyY = barY + ToolbarMetrics::BUY_Y;

    outZDelta = 0;
    outRotate = false;
    outBuy = 0;

    // Buy Train Car [CAR +1]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX, (float)buyY, 74.0f, (float)ToolbarMetrics::BUY_H})) {
        outBuy = 1;
        return true;
    }
    // Expand District [EXPAND]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 78, (float)buyY, 74.0f, (float)ToolbarMetrics::BUY_H})) {
        outBuy = 2;
        return true;
    }
    // Buy Extra Train [in aux column below CAR+1 / EXPAND]
    int exY = buyY + ToolbarMetrics::BUY_H + 4;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX, (float)exY, (float)ToolbarMetrics::AUX_W, 26.0f})) {
        outBuy = 3;
        return true;
    }
    // Height down [-]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 96, (float)barY + 6, 20.0f, 22.0f})) {
        outZDelta = -1;
        return true;
    }
    // Height up [+]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 120, (float)barY + 6, 20.0f, 22.0f})) {
        outZDelta = 1;
        return true;
    }
    // Rotate [ROT]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 104, (float)barY + 44, 34.0f, 22.0f})) {
        outRotate = true;
        return true;
    }
    return false;
}

bool UserInterface::CheckHUDClick(
    Vector2 mousePos,
    int& outNewSpeed,
    bool& outToggleMute,
    bool& outToggleStats,
    bool& outToggleStaff,
    bool& outToggleRideCam,
    bool& outToggleHelp,
    bool* outAutoBridge
) const {
    (void)outToggleStaff; (void)outToggleRideCam;

    int screenW = GetScreenWidth();

    int btnX = screenW - 140;
    int infoCX = btnX - 74 + 14;
    int helpCX = btnX - 38 + 14;

    // Circuit status badge (click to auto-close loop when open)
    int badgeX = 607;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)badgeX, 8.0f, 120.0f, 36.0f})) {
        if (outAutoBridge) *outAutoBridge = true;
        return true;
    }

    // Telemetry / Line Operations stats button (circular "i" icon)
    if (CheckCollisionPointCircle(mousePos, {(float)infoCX, 26.0f}, 16.0f)) {
        outToggleStats = true;
        return true;
    }

    // How to Play (circular button)
    if (CheckCollisionPointCircle(mousePos, {(float)helpCX, 26.0f}, 16.0f)) {
        outToggleHelp = true;
        return true;
    }

    // Pause / Normal / Fast / Mute
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX, 12.0f, 28.0f, 28.0f})) {
        outNewSpeed = 0;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 32, 12.0f, 28.0f, 28.0f})) {
        outNewSpeed = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 64, 12.0f, 28.0f, 28.0f})) {
        outNewSpeed = 2;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 96, 12.0f, 42.0f, 28.0f})) {
        outToggleMute = true;
        return true;
    }

    return false;
}

bool UserInterface::CheckStatsWindowClick(Vector2 mousePos, float& outTicketPriceDelta, int& outColorChoice, int& outModeChange, int& outCarDelta, bool& outClose) const {
    int screenW = GetScreenWidth();
    int winW = 330;
    int winX = screenW - winW - 16;
    int winY = 62;

    outColorChoice = -1;
    outModeChange = -1;
    outCarDelta = 0;
    outTicketPriceDelta = 0.0f;

    // Close button [X]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + winW - 32, (float)winY + 4, 28.0f, 28.0f})) {
        outClose = true;
        return true;
    }

    // Operating Mode buttons: Open, Test, Closed
    int modeY = winY + 44;
    int modeBtnW = 94;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 14, (float)modeY, (float)modeBtnW, 26.0f})) {
        outModeChange = (int)LINE_OPEN;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 118, (float)modeY, (float)modeBtnW, 26.0f})) {
        outModeChange = (int)LINE_TEST;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 222, (float)modeY, (float)modeBtnW, 26.0f})) {
        outModeChange = (int)LINE_CLOSED;
        return true;
    }

    // Train Formation [-] and [+]
    int carRowY = winY + 78;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 250, (float)carRowY, 24.0f, 20.0f})) {
        outCarDelta = -1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 278, (float)carRowY, 24.0f, 20.0f})) {
        outCarDelta = 1;
        return true;
    }

    // Fare price [-] and [+]
    int fareRowY = carRowY + 24;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 250, (float)fareRowY, 24.0f, 20.0f})) {
        outTicketPriceDelta = -0.25f;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 278, (float)fareRowY, 24.0f, 20.0f})) {
        outTicketPriceDelta = 0.25f;
        return true;
    }

    // Theme color buttons (6 colors)
    int palY = fareRowY + 28 + 24 * 6 + 10;
    for (int c = 0; c < 6; ++c) {
        int cx = winX + 145 + c * 27;
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx, (float)palY, 23.0f, 20.0f})) {
            outColorChoice = c;
            return true;
        }
    }

    // Consume any other click inside the window rectangle so it does not click through to the world!
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX, (float)winY, (float)winW, 470.0f})) {
        return true;
    }

    return false;
}

bool UserInterface::CheckStaffWindowClick(Vector2 mousePos, bool& outHireHandyman, bool& outHireMechanic, bool& outClose) const {
    int screenW = GetScreenWidth();
    int winW = 340;
    int winX = screenW - winW - 16;
    int winY = 62;

    // Close button [X]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + winW - 28, (float)winY + 6, 22.0f, 22.0f})) {
        outClose = true;
        return true;
    }

    float btnW = (float)(winW - 40) / 2.0f;
    // Hire Custodian
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 16, (float)winY + 92, btnW, 36.0f})) {
        outHireHandyman = true;
        return true;
    }

    // Hire Technician
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 24 + btnW, (float)winY + 92, btnW, 36.0f})) {
        outHireMechanic = true;
        return true;
    }

    // Consume any other click inside the staff window rectangle so it doesn't click through to the world!
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX, (float)winY, (float)winW, 340.0f})) {
        return true;
    }

    return false;
}

bool UserInterface::CheckPeepInspectorCloseClick(Vector2 mousePos) const {
    int cardW = 300;
    int cardX = 14;
    int cardY = 120;

    return CheckCollisionPointRec(mousePos, Rectangle{(float)cardX + cardW - 28, (float)cardY + 8, 24.0f, 24.0f});
}

bool UserInterface::IsMouseInPeepInspector(Vector2 mousePos) const {
    return CheckCollisionPointRec(mousePos, Rectangle{14.0f, 120.0f, 300.0f, 145.0f});
}

bool UserInterface::CheckStationInspectorClick(Vector2 mousePos, const TrackNode* station, bool& outUpgrade, bool& outClose) const {
    if (!station) return false;

    int cardW = 300;
    int cardH = 172;
    int cardX = 14;
    int cardY = 120;

    outUpgrade = false;
    outClose = false;

    // Close button [x]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)cardX + cardW - 28, (float)cardY + 8, 24.0f, 24.0f})) {
        outClose = true;
        return true;
    }

    // Upgrade button
    int btnY = cardY + 122;
    int btnW = cardW - 28;
    int btnH = 34;
    if (station->stationLevel < 3) {
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)cardX + 14, (float)btnY, (float)btnW, (float)btnH})) {
            outUpgrade = true;
            return true;
        }
    }

    // Consume any other click inside card
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH})) {
        return true;
    }

    return false;
}

bool UserInterface::IsMouseInStationInspector(Vector2 mousePos) const {
    return CheckCollisionPointRec(mousePos, Rectangle{14.0f, 120.0f, 300.0f, 172.0f});
}

bool UserInterface::CheckHelpOverlayClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int boxW = 740;
    int boxH = 490;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    // Close [X]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx + boxW - 95, (float)by + 8, 85.0f, 28.0f})) {
        return true;
    }
    // Dismiss button
    int btnW = 240;
    int btnH = 34;
    int btnX = bx + (boxW - btnW) / 2;
    int btnY = by + boxH - 44;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH})) {
        return true;
    }
    return false;
}

int UserInterface::CheckUpgradeModalClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int modalW = 700;
    int modalH = 340;
    int mx = (screenW - modalW) / 2;
    int my = (screenH - modalH) / 2;

    int cardW = 195;
    int cardH = 200;
    int cardY = my + 90;
    int gap = 25;
    int startX = mx + (modalW - (3 * cardW + 2 * gap)) / 2;

    for (int i = 0; i < 3; ++i) {
        int cx = startX + i * (cardW + gap);
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx, (float)cardY, (float)cardW, (float)cardH})) {
            return i;
        }
    }
    return -1;
}

bool UserInterface::CheckRestartClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // Game Over Restart button
    int bx = (screenW - 480) / 2;
    int by = (screenH - 260) / 2;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx + 140, (float)by + 175, 200.0f, 45.0f})) {
        return true;
    }

    // Victory Screen Restart button
    int vBoxW = 560;
    int vBoxH = 310;
    int vbx = (screenW - vBoxW) / 2;
    int vby = (screenH - vBoxH) / 2;
    int btn2W = 200;
    int btn2H = 34;
    int btn2X = vbx + (vBoxW - btn2W) / 2;
    int btn2Y = vby + 218;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btn2X, (float)btn2Y, (float)btn2W, (float)btn2H})) {
        return true;
    }

    return false;
}

bool UserInterface::CheckVictoryContinueClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int boxW = 560;
    int boxH = 310;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;
    int btnW = 340;
    int btnH = 46;
    int btn1X = bx + (boxW - btnW) / 2;
    int btn1Y = by + 150;
    return CheckCollisionPointRec(mousePos, Rectangle{(float)btn1X, (float)btn1Y, (float)btnW, (float)btnH});
}

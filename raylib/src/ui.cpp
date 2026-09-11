#include "ui.hpp"
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
    bool helpOpen
) {
    int screenW = GetScreenWidth();

    // Top Bar Background with drop shadow (Operations Control Center Navy)
    DrawRectangle(0, 0, screenW, 54, Color{15, 23, 42, 248});
    DrawLine(0, 54, screenW, 54, Color{51, 65, 85, 255});

    // 1. Metro Brand Logo & Roundel Badge
    DrawCircle(28, 27, 16, Color{220, 38, 38, 255}); // Crimson roundel
    DrawCircle(28, 27, 12, Color{15, 23, 42, 255});
    DrawRectangle(16, 23, 24, 8, Color{220, 38, 38, 255});
    DrawText("M", 23, 19, 16, WHITE);

    DrawText("METRO GRID", 52, 11, 18, Color{248, 250, 252, 255});
    DrawText("OCC DISPATCH", 52, 31, 10, Color{148, 163, 184, 255});

    // 2. Service Calendar & Weekly Rhythm
    int calX = 170;
    DrawText(TextFormat("WEEK %d", week), calX, 10, 13, WHITE);
    float weekPct = std::max(0.0f, std::min(1.0f, weekTimer / 60.0f));
    DrawRectangle(calX, 28, 54, 7, Color{51, 65, 85, 255});
    DrawRectangle(calX, 28, (int)(54.0f * weekPct), 7, Color{56, 189, 248, 255});

    // 3. Transit Authority Treasury
    int bankX = 238;
    DrawText("TREASURY", bankX, 8, 9, Color{148, 163, 184, 255});
    DrawText(TextFormat("$%.0f", balance), bankX, 20, 18, Color{52, 211, 153, 255});

    // 4. Commuters Transported (Ridership)
    int scoreX = 330;
    DrawText("RIDERSHIP", scoreX, 8, 9, Color{148, 163, 184, 255});
    DrawText(TextFormat("%d", ridership), scoreX, 20, 18, Color{255, 214, 0, 255});

    // 5. Commuter Satisfaction
    int satX = 412;
    DrawText("SATISFACTION", satX, 8, 9, Color{148, 163, 184, 255});
    Color satColor = (satisfaction > 70.0f) ? Color{74, 222, 128, 255} : (satisfaction > 40.0f ? Color{251, 146, 60, 255} : Color{248, 113, 113, 255});
    DrawText(TextFormat("%.0f%%", satisfaction), satX, 20, 18, satColor);

    // 6. EMU Speedometer
    int spdX = 502;
    DrawText("TRAIN SPEED", spdX, 8, 9, Color{148, 163, 184, 255});
    Color spdColor = (speedKmh > 55.0f) ? Color{56, 189, 248, 255} : (speedKmh > 10.0f ? Color{251, 191, 36, 255} : Color{148, 163, 184, 255});
    DrawText(TextFormat("%.1f km/h", speedKmh), spdX, 20, 18, spdColor);

    // 7. Wayside Signaling Indicator Lamp (3-Aspect)
    int sigX = 594;
    DrawRectangleRounded(Rectangle{(float)sigX, 10.0f, 62.0f, 32.0f}, 0.25f, 4, Color{30, 41, 59, 255});
    DrawRectangleRoundedLines(Rectangle{(float)sigX, 10.0f, 62.0f, 32.0f}, 0.25f, 4, Color{71, 85, 105, 255});

    // 3 Signal lenses
    Color gCol = (signalAspect == SIGNAL_GREEN) ? Color{34, 197, 94, 255} : Color{22, 101, 52, 140};
    Color aCol = (signalAspect == SIGNAL_AMBER) ? Color{245, 158, 11, 255} : Color{146, 64, 14, 140};
    Color rCol = (signalAspect == SIGNAL_RED)   ? Color{239, 68, 68, 255} : Color{153, 27, 27, 140};

    DrawCircle(sigX + 12, 26, 6, gCol);
    DrawCircle(sigX + 31, 26, 6, aCol);
    DrawCircle(sigX + 50, 26, 6, rCol);

    // 8. Circuit Status Badge
    int statX = 666;
    if (circuitClosed) {
        DrawRectangleRounded(Rectangle{(float)statX, 13.0f, 82.0f, 26.0f}, 0.3f, 4, Color{34, 197, 94, 220});
        DrawText("ACTIVE LOOP", statX + 9, 20, 10, WHITE);
    } else {
        DrawRectangleRounded(Rectangle{(float)statX, 13.0f, 82.0f, 26.0f}, 0.3f, 4, Color{239, 68, 68, 220});
        DrawText("OPEN TRACK", statX + 9, 20, 10, WHITE);
    }

    // 10. Simulation Controls & Audio Mute (Far right anchor)
    int btnX = screenW - 145;
    DrawRectangleRounded(Rectangle{(float)btnX, 13.0f, 26.0f, 26.0f}, 0.2f, 4, (gameSpeed == 0) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawText("||", btnX + 9, 19, 13, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 30, 13.0f, 26.0f, 26.0f}, 0.2f, 4, (gameSpeed == 1) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawText(">", btnX + 39, 19, 13, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 60, 13.0f, 26.0f, 26.0f}, 0.2f, 4, (gameSpeed == 2) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawText(">>", btnX + 66, 19, 13, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 90, 13.0f, 44.0f, 26.0f}, 0.2f, 4, muted ? Color{239, 68, 68, 220} : Color{30, 41, 59, 200});
    DrawText(muted ? "MUT" : "SND", btnX + 98, 20, 10, WHITE);

    // 9. OCC Action Toggle Buttons (Line Ops, Crew, Cab Cam, How to Play)
    // Anchored immediately left of btnX so it never collides!
    int actX = btnX - 12 - 322;

    // Line Operations Button
    DrawRectangleRounded(Rectangle{(float)actX, 13.0f, 74.0f, 26.0f}, 0.25f, 4, lineOpsOpen ? Color{220, 38, 38, 255} : Color{30, 41, 59, 220});
    DrawRectangleRoundedLines(Rectangle{(float)actX, 13.0f, 74.0f, 26.0f}, 0.25f, 4, Color{71, 85, 105, 200});
    DrawText("LINE OPS", actX + 13, 20, 10, lineOpsOpen ? WHITE : Color{203, 213, 225, 255});

    // Crew Button
    DrawRectangleRounded(Rectangle{(float)actX + 80, 13.0f, 62.0f, 26.0f}, 0.25f, 4, crewOpen ? Color{220, 38, 38, 255} : Color{30, 41, 59, 220});
    DrawRectangleRoundedLines(Rectangle{(float)actX + 80, 13.0f, 62.0f, 26.0f}, 0.25f, 4, Color{71, 85, 105, 200});
    DrawText("CREW", actX + 96, 20, 10, crewOpen ? WHITE : Color{203, 213, 225, 255});

    // Cab Cam Button
    DrawRectangleRounded(Rectangle{(float)actX + 148, 13.0f, 74.0f, 26.0f}, 0.25f, 4, cabCamActive ? Color{220, 38, 38, 255} : Color{30, 41, 59, 220});
    DrawRectangleRoundedLines(Rectangle{(float)actX + 148, 13.0f, 74.0f, 26.0f}, 0.25f, 4, Color{71, 85, 105, 200});
    DrawText("CAB CAM", actX + 160, 20, 10, cabCamActive ? WHITE : Color{203, 213, 225, 255});

    // How to Play Button (Highlighted)
    DrawRectangleRounded(Rectangle{(float)actX + 228, 13.0f, 94.0f, 26.0f}, 0.25f, 4, helpOpen ? Color{220, 38, 38, 255} : Color{14, 116, 144, 240});
    DrawRectangleRoundedLines(Rectangle{(float)actX + 228, 13.0f, 94.0f, 26.0f}, 0.25f, 4, helpOpen ? WHITE : Color{56, 189, 248, 255});
    DrawText("? HOW TO PLAY", actX + 233, 20, 9, helpOpen ? WHITE : Color{255, 214, 0, 255});
}

void UserInterface::DrawToolbar(
    ToolCategory activeTab,
    TrackType currentTrack,
    SceneryType currentScenery,
    GroundType currentGround,
    int currentZ,
    Direction currentDir,
    bool isBulldozing
) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int barW = 740;
    int barH = 72;
    int auxW = 146;
    int totalDockW = barW + 12 + auxW;
    int barX = std::max(10, (screenW - totalDockW) / 2);
    int barY = screenH - barH - 12;

    // 1. Category Tabs above toolbar
    const char* tabNames[] = {"1. TRACK & INFRA", "2. CONCOURSE & PLAZA", "3. URBAN SCENERY"};
    int tabW = 140;
    int tabH = 24;
    int tabStartY = barY - tabH + 2;

    for (int t = 0; t < 3; ++t) {
        int tx = barX + 16 + t * (tabW + 8);
        bool isCurrentTab = ((int)activeTab == t);
        Color tabBg = isCurrentTab ? Color{15, 23, 42, 255} : Color{30, 41, 59, 200};
        Color tabText = isCurrentTab ? Color{56, 189, 248, 255} : Color{148, 163, 184, 255};

        DrawRectangleRounded(Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4}, 0.2f, 4, tabBg);
        DrawRectangleRoundedLines(Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4}, 0.2f, 4, isCurrentTab ? Color{56, 189, 248, 255} : Color{51, 65, 85, 255});
        DrawText(tabNames[t], tx + 12, tabStartY + 6, 10, tabText);
    }

    // 2. Toolbar Body Card
    DrawRectangleRounded(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.2f, 6, Color{15, 23, 42, 248});
    DrawRectangleRoundedLines(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.2f, 6, Color{51, 65, 85, 255});

    // 3. Render Items according to Active Tab
    int itemBtnW = 68;
    int itemBtnH = 52;
    int itemStartY = barY + 10;

    if (activeTab == CAT_TRACK) {
        struct TrackBtn { const char* name; const char* key; TrackType type; };
        TrackBtn buttons[] = {
            {"Straight", "1", TRACK_STRAIGHT},
            {"L-Turn (L)", "2", TRACK_CURVE_LEFT},
            {"R-Turn (R)", "3", TRACK_CURVE_RIGHT},
            {"Elevated", "4", TRACK_VIADUCT_ELEVATED},
            {"Ramp Slope", "5", TRACK_VIADUCT_SLOPE},
            {"Tunnel", "6", TRACK_TUNNEL_PORTAL},
            {"Station", "7", TRACK_STATION},
            {"Signal", "8", TRACK_SIGNAL},
            {"Demolish", "X", TRACK_NONE}
        };

        int spacing = 11;
        int startX = barX + (barW - (9 * itemBtnW + 8 * spacing)) / 2;

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + spacing);
            bool isSelected = (i == 8) ? isBulldozing : (!isBulldozing && currentTrack == buttons[i].type);

            Color btnBg = isSelected ? Color{220, 38, 38, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{254, 202, 202, 255} : Color{71, 85, 105, 200});

            DrawText(buttons[i].key, bx + 5, itemStartY + 4, 10, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 4, itemStartY + 24, 8, textC);
        }
    } else if (activeTab == CAT_INFRA) {
        struct InfraBtn { const char* name; const char* key; GroundType ground; bool isBull; };
        InfraBtn buttons[] = {
            {"Sidewalk", "1", GROUND_PATH, false},
            {"Tactile Que", "2", GROUND_QUEUE, false},
            {"Plaza Stone", "3", GROUND_PLAZA, false},
            {"Bulldoze", "X", GROUND_GRASS, true}
        };

        int infraBtnW = 120;
        int spacing = 18;
        int startX = barX + (barW - (4 * infraBtnW + 3 * spacing)) / 2;

        for (int i = 0; i < 4; ++i) {
            int bx = startX + i * (infraBtnW + spacing);
            bool isSelected = buttons[i].isBull ? isBulldozing : (!isBulldozing && currentGround == buttons[i].ground);

            Color btnBg = isSelected ? Color{220, 38, 38, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)infraBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)infraBtnW, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{254, 202, 202, 255} : Color{71, 85, 105, 200});

            DrawText(buttons[i].key, bx + 8, itemStartY + 5, 10, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 10, itemStartY + 25, 10, textC);
        }
    } else if (activeTab == CAT_SCENERY) {
        struct SceneryBtn { const char* name; const char* key; SceneryType scn; int cost; };
        SceneryBtn buttons[] = {
            {"Subway Ent", "1", SCENERY_METRO_ENTRANCE, 120},
            {"Fare Gates", "2", SCENERY_TURNSTILE_GATE, 90},
            {"Oak Tree", "3", SCENERY_STREET_TREE, 35},
            {"Pine Tree", "4", SCENERY_PINE_TREE, 30},
            {"Bench", "5", SCENERY_BENCH, 20},
            {"LED Lamp", "6", SCENERY_LAMP_POST, 25},
            {"Fountain", "7", SCENERY_FOUNTAIN, 120},
            {"Metro Cafe", "8", SCENERY_NEWSSTAND, 150},
            {"Bulldoze", "X", SCENERY_NONE, 0}
        };

        int spacing = 11;
        int startX = barX + (barW - (9 * itemBtnW + 8 * spacing)) / 2;

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + spacing);
            bool isSelected = (i == 8) ? isBulldozing : (!isBulldozing && currentScenery == buttons[i].scn);

            Color btnBg = isSelected ? Color{220, 38, 38, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{254, 202, 202, 255} : Color{71, 85, 105, 200});

            DrawText(buttons[i].key, bx + 5, itemStartY + 4, 10, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 4, itemStartY + 20, 8, textC);
            if (buttons[i].cost > 0) {
                DrawText(TextFormat("$%d", buttons[i].cost), bx + 5, itemStartY + 35, 9, Color{52, 211, 153, 255});
            }
        }
    }

    // 4. Auxiliary Pills on Right (Elevation & Heading)
    int auxX = barX + barW + 12;

    // Height Pill
    DrawRectangleRounded(Rectangle{(float)auxX, (float)barY, (float)auxW, 34.0f}, 0.25f, 4, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)barY, (float)auxW, 34.0f}, 0.25f, 4, Color{51, 65, 85, 255});
    DrawText("HEIGHT (E/Q)", auxX + 8, barY + 5, 8, Color{148, 163, 184, 255});
    DrawText(TextFormat("Z = %d", currentZ), auxX + 10, barY + 17, 13, Color{255, 214, 0, 255});
    DrawRectangle((float)auxX + 96, (float)barY + 7, 20, 20, Color{30, 41, 59, 255});
    DrawText("-", auxX + 103, barY + 10, 14, WHITE);
    DrawRectangle((float)auxX + 120, (float)barY + 7, 20, 20, Color{30, 41, 59, 255});
    DrawText("+", auxX + 126, barY + 10, 14, WHITE);

    // Heading / Orientation Pill
    const char* dirLabels[] = {"NORTH [^]", "EAST [>]", "SOUTH [v]", "WEST [<]"};
    DrawRectangleRounded(Rectangle{(float)auxX, (float)barY + 38, (float)auxW, 34.0f}, 0.25f, 4, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)barY + 38, (float)auxW, 34.0f}, 0.25f, 4, Color{51, 65, 85, 255});
    DrawText("HEADING [R]", auxX + 8, barY + 43, 8, Color{148, 163, 184, 255});
    DrawText(dirLabels[currentDir], auxX + 10, barY + 55, 12, Color{56, 189, 248, 255});
    DrawRectangle((float)auxX + 104, (float)barY + 45, 34, 20, Color{30, 41, 59, 255});
    DrawText("ROT", auxX + 110, barY + 50, 10, WHITE);
}

void UserInterface::DrawLineOperations(const MetroLineStats& stats) {
    int screenW = GetScreenWidth();
    int winW = 330;
    int winH = 465;
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
    int cardX = 18;
    int cardY = 64;

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

    // Name & Destination Name
    DrawText(commuter->name.c_str(), cardX + 72, cardY + 13, 12, WHITE);
    DrawText(TextFormat("Bound for: %s", GetShapeName(commuter->targetShape)), cardX + 72, cardY + 29, 9, Color{148, 163, 184, 255});
    DrawText("[x]", cardX + cardW - 22, cardY + 10, 12, Color{148, 163, 184, 255});

    // Satisfaction Bar
    DrawText("Satisfaction", cardX + 15, cardY + 54, 9, Color{148, 163, 184, 255});
    DrawRectangle(cardX + 85, cardY + 56, 130, 8, Color{30, 41, 59, 255});
    DrawRectangle(cardX + 85, cardY + 56, (int)(130.0f * (commuter->happiness / 100.0f)), 8, Color{74, 222, 128, 255});
    DrawText(TextFormat("%.0f%%", commuter->happiness), cardX + 225, cardY + 54, 10, WHITE);

    // Smartcard Pass Balance
    DrawText("IC Card:", cardX + 15, cardY + 70, 9, Color{148, 163, 184, 255});
    DrawText(TextFormat("$%.2f Balance", commuter->metroPassBalance), cardX + 85, cardY + 70, 10, Color{52, 211, 153, 255});

    // Thoughts speech bubble
    DrawRectangleRounded(Rectangle{(float)cardX + 12, (float)cardY + 92, (float)cardW - 24, 40.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText(TextFormat("\"%s\"", commuter->thought.c_str()), cardX + 18, cardY + 104, 9, Color{255, 214, 0, 255});
}

void UserInterface::DrawToast(const ToastMessage& toast) {
    if (toast.timer <= 0.0f) return;

    int screenW = GetScreenWidth();
    int toastW = 440;
    int toastH = 34;
    int toastX = (screenW - toastW) / 2;
    int toastY = 62;

    DrawRectangleRounded(Rectangle{(float)toastX, (float)toastY, (float)toastW, (float)toastH}, 0.3f, 4, Color{15, 23, 42, 248});
    DrawRectangleRoundedLines(Rectangle{(float)toastX, (float)toastY, (float)toastW, (float)toastH}, 0.3f, 4, toast.color);

    DrawText(toast.text.c_str(), toastX + 16, toastY + 10, 11, WHITE);
}

void UserInterface::DrawQuickTipBanner(const std::string& tip) {
    if (tip.empty()) return;
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int textW = MeasureText(tip.c_str(), 10);
    int bannerW = textW + 36;
    int bannerH = 24;
    int bannerX = (screenW - bannerW) / 2;
    int bannerY = screenH - 122;

    DrawRectangleRounded(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{15, 23, 42, 235});
    DrawRectangleRoundedLines(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{56, 189, 248, 200});

    DrawCircle(bannerX + 14, bannerY + 12, 3.5f, Color{255, 214, 0, 255});
    DrawText(tip.c_str(), bannerX + 24, bannerY + 7, 10, Color{241, 245, 249, 255});
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

    // CARD 1: TRACK CONSTRUCTION
    DrawRectangleRounded(Rectangle{(float)card1X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)card1X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{56, 189, 248, 255});
    DrawRectangleRounded(Rectangle{(float)card1X + 4, (float)cardY + 4, (float)cardW - 8, 24.0f}, 0.2f, 4, Color{14, 116, 144, 255});
    DrawText("STEP 1: BUILD TRACK LOOP", card1X + 12, cardY + 10, 11, WHITE);

    int c1y = cardY + 36;
    DrawText("* [1] Straight Rails", card1X + 10, c1y, 10, Color{255, 214, 0, 255}); c1y += 16;
    DrawText("* [2] L-Turn Left (90 CCW)", card1X + 10, c1y, 10, Color{255, 214, 0, 255}); c1y += 16;
    DrawText("* [3] R-Turn Right (90 CW)", card1X + 10, c1y, 10, Color{255, 214, 0, 255}); c1y += 16;
    DrawText("* [R] Change Heading (N/E/S/W)", card1X + 10, c1y, 10, Color{56, 189, 248, 255}); c1y += 20;
    DrawText("Placement auto-advances your", card1X + 10, c1y, 9, Color{203, 213, 225, 255}); c1y += 14;
    DrawText("heading in the track's direction.", card1X + 10, c1y, 9, Color{203, 213, 225, 255}); c1y += 16;
    DrawText("Yellow arrow on ghost shows", card1X + 10, c1y, 9, Color{250, 204, 21, 255}); c1y += 14;
    DrawText("exact train travel path!", card1X + 10, c1y, 9, Color{250, 204, 21, 255}); c1y += 18;
    DrawText("When loop connects, trains", card1X + 10, c1y, 9, Color{74, 222, 128, 255}); c1y += 14;
    DrawText("launch into regular service!", card1X + 10, c1y, 9, Color{74, 222, 128, 255});

    // CARD 2: STATIONS & SIGNALS
    DrawRectangleRounded(Rectangle{(float)card2X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)card2X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{52, 211, 153, 255});
    DrawRectangleRounded(Rectangle{(float)card2X + 4, (float)cardY + 4, (float)cardW - 8, 24.0f}, 0.2f, 4, Color{16, 149, 106, 255});
    DrawText("STEP 2: ADD STATIONS", card2X + 14, cardY + 10, 11, WHITE);

    int c2y = cardY + 36;
    DrawText("* [7] Station Platforms", card2X + 10, c2y, 10, Color{255, 214, 0, 255}); c2y += 16;
    DrawText("Trains halt at PSD platform", card2X + 10, c2y, 9, Color{203, 213, 225, 255}); c2y += 14;
    DrawText("doors to board commuters.", card2X + 10, c2y, 9, Color{203, 213, 225, 255}); c2y += 20;
    DrawText("* [8] 3-Aspect Signals", card2X + 10, c2y, 10, Color{255, 214, 0, 255}); c2y += 16;
    DrawText("Green/Yellow/Red wayside lights", card2X + 10, c2y, 9, Color{203, 213, 225, 255}); c2y += 14;
    DrawText("prevent rear-end collisions.", card2X + 10, c2y, 9, Color{203, 213, 225, 255}); c2y += 20;
    DrawText("Tab 2 & 3: Concourse & Plaza", card2X + 10, c2y, 9, Color{56, 189, 248, 255}); c2y += 14;
    DrawText("Build subway stairs, turnstiles,", card2X + 10, c2y, 9, Color{203, 213, 225, 255}); c2y += 14;
    DrawText("kiosks, and lampposts!", card2X + 10, c2y, 9, Color{203, 213, 225, 255});

    // CARD 3: COMMUTER TRIAGE & WIN
    DrawRectangleRounded(Rectangle{(float)card3X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)card3X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{251, 146, 60, 255});
    DrawRectangleRounded(Rectangle{(float)card3X + 4, (float)cardY + 4, (float)cardW - 8, 24.0f}, 0.2f, 4, Color{194, 65, 12, 255});
    DrawText("STEP 3: COMMUTER TRIAGE", card3X + 10, cardY + 10, 11, WHITE);

    int c3y = cardY + 36;
    DrawText("* Commuter Target Shapes:", card3X + 10, c3y, 10, Color{255, 214, 0, 255}); c3y += 16;
    DrawText("Square, Triangle, Cross, Circle.", card3X + 10, c3y, 9, Color{203, 213, 225, 255}); c3y += 18;
    DrawText("* Overcrowding Triage Clock:", card3X + 10, c3y, 10, Color{239, 68, 68, 255}); c3y += 16;
    DrawText("If >= 10 commuters wait at any", card3X + 10, c3y, 9, Color{203, 213, 225, 255}); c3y += 14;
    DrawText("station, a countdown begins!", card3X + 10, c3y, 9, Color{203, 213, 225, 255}); c3y += 16;
    DrawText("Pick up riders in time to avoid", card3X + 10, c3y, 9, Color{203, 213, 225, 255}); c3y += 14;
    DrawText("a gridlock game over!", card3X + 10, c3y, 9, Color{203, 213, 225, 255}); c3y += 18;
    DrawText("Deliver to earn ticket revenue ($)", card3X + 10, c3y, 9, Color{74, 222, 128, 255});

    // BOTTOM SHORTCUTS BOX
    int scY = cardY + cardH + 10;
    int scH = 110;
    DrawRectangleRounded(Rectangle{(float)bx + 16, (float)scY, (float)boxW - 32, (float)scH}, 0.08f, 4, Color{23, 32, 51, 240});
    DrawRectangleRoundedLines(Rectangle{(float)bx + 16, (float)scY, (float)boxW - 32, (float)scH}, 0.08f, 4, Color{51, 65, 85, 255});

    DrawText("OCC DISPATCHER KEYBOARD & MOUSE CONTROLS", bx + 28, scY + 8, 10, Color{255, 214, 0, 255});

    int kCol1 = bx + 28;
    int kCol2 = bx + 380;
    int ky = scY + 28;

    DrawText("WASD / Right Drag : Pan isometric camera", kCol1, ky, 9, Color{203, 213, 225, 255});
    DrawText("Tab 1 / 2 / 3 : Switch Track / Concourse / Scenery", kCol2, ky, 9, Color{203, 213, 225, 255});
    ky += 16;
    DrawText("Mouse Scroll      : Zoom In / Zoom Out", kCol1, ky, 9, Color{203, 213, 225, 255});
    DrawText("X             : Bulldoze / Demolish track & props", kCol2, ky, 9, Color{203, 213, 225, 255});
    ky += 16;
    DrawText("F                 : Driver Cab Camera (Ride EMU)", kCol1, ky, 9, Color{203, 213, 225, 255});
    DrawText("E / Q         : Raise / Lower Elevation (Z=0..5)", kCol2, ky, 9, Color{203, 213, 225, 255});
    ky += 16;
    DrawText("Left Click        : Lay piece or Inspect Commuter", kCol1, ky, 9, Color{203, 213, 225, 255});
    DrawText("O / P         : Line Operations / Transit Crew Window", kCol2, ky, 9, Color{203, 213, 225, 255});

    // CLOSE BUTTON
    int btnW = 240;
    int btnH = 34;
    int btnX = bx + (boxW - btnW) / 2;
    int btnY = by + boxH - 44;

    DrawRectangleRounded(Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH}, 0.3f, 4, Color{220, 38, 38, 255});
    DrawText("START DISPATCHING (CLOSE)", btnX + 24, btnY + 11, 11, WHITE);
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

    DrawText("TRANSIT AUTHORITY EXPANSION GRANT", mx + 155, my + 24, 20, Color{255, 179, 0, 255});
    DrawText("Select 1 capital upgrade grant to expand urban network capacity:", mx + 150, my + 54, 11, Color{148, 163, 184, 255});

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
        DrawText(choices[i].perkTag.c_str(), cx + 18, cardY + 18, 9, WHITE);

        DrawText(choices[i].title.c_str(), cx + 14, cardY + 48, 13, WHITE);
        DrawText(choices[i].description.c_str(), cx + 14, cardY + 80, 10, Color{203, 213, 225, 255});

        DrawRectangleRounded(Rectangle{(float)cx + 20, (float)cardY + cardH - 38, (float)cardW - 40, 26.0f}, 0.3f, 4, hovered ? choices[i].accentColor : Color{51, 65, 85, 255});
        DrawText("AUTHORIZE", cx + 64, cardY + cardH - 31, 10, WHITE);
    }
}

void UserInterface::DrawGameOver(int finalRidership) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 210});

    int boxW = 480;
    int boxH = 260;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{220, 38, 38, 255});

    DrawText("NETWORK GRIDLOCK: OVERCROWDING!", bx + 45, by + 30, 20, Color{239, 68, 68, 255});
    DrawText("Overcrowding triage timers expired and platforms collapsed into gridlock.", bx + 35, by + 68, 10, Color{203, 213, 225, 255});
    DrawText(TextFormat("Total Commuters Transported: %d", finalRidership), bx + 110, by + 115, 15, Color{255, 214, 0, 255});

    DrawRectangleRounded(Rectangle{(float)bx + 140, (float)by + 175, 200.0f, 45.0f}, 0.3f, 4, Color{220, 38, 38, 255});
    DrawText("RESTART NETWORK", bx + 165, by + 189, 14, WHITE);
}

void UserInterface::DrawVictory(int finalRidership) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 210});

    int boxW = 520;
    int boxH = 280;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{34, 197, 94, 255});

    DrawText("TRANSIT TRIUMPH! 500 COMMUTERS!", bx + 65, by + 30, 22, Color{34, 197, 94, 255});
    DrawText("Your rapid transit network connected all district shapes flawlessly!", bx + 55, by + 70, 11, Color{203, 213, 225, 255});
    DrawText(TextFormat("Total Commuters Delivered: %d", finalRidership), bx + 140, by + 120, 16, Color{255, 214, 0, 255});

    DrawRectangleRounded(Rectangle{(float)bx + 150, (float)by + 185, 220.0f, 45.0f}, 0.3f, 4, Color{16, 185, 129, 255});
    DrawText("CONTINUE SERVICE", bx + 185, by + 199, 14, WHITE);
}

void UserInterface::DrawTitleScreen() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // 1. Sleek dark transit blueprint background
    DrawRectangle(0, 0, screenW, screenH, Color{15, 23, 42, 255}); // Slate 900

    // Schematic metro track network in background
    for (int x = -100; x < screenW + 300; x += 80) {
        DrawLine(x, 0, x - 180, screenH, Color{30, 41, 59, 70});
    }
    for (int y = 0; y < screenH; y += 80) {
        DrawLine(0, y, screenW, y + 40, Color{30, 41, 59, 50});
    }

    // Top accent metro line bar
    DrawRectangle(0, 0, screenW, 5, Color{220, 38, 38, 255});

    // 2. Title & Roundel Header
    int cx = screenW / 2;
    int headerY = 28;

    // Metro Roundel Icon
    int rx = cx - 180;
    int ry = headerY + 22;
    DrawCircle(rx, ry, 22, Color{220, 38, 38, 255});
    DrawCircle(rx, ry, 15, Color{15, 23, 42, 255});
    DrawRectangle(rx - 26, ry - 6, 52, 12, Color{220, 38, 38, 255});
    DrawText("M", rx - 6, ry - 6, 12, WHITE);

    DrawText("METRO GRID", cx - 140, headerY, 34, Color{248, 250, 252, 255});
    DrawText("Urban Rapid Transit Tycoon & Commuter Flow Dispatcher", cx - 190, headerY + 40, 13, Color{56, 189, 248, 255});

    // Goal Banner Pill
    int bannerW = 660;
    int bannerH = 26;
    int bannerX = (screenW - bannerW) / 2;
    int bannerY = headerY + 62;
    DrawRectangleRounded(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{30, 41, 59, 230});
    DrawRectangleRoundedLines(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{245, 158, 11, 200});
    DrawText("MISSION: Safely board & deliver 500 commuters without letting stations overcrowd!", bannerX + 18, bannerY + 6, 11, Color{255, 214, 0, 255});

    // 3. Three Simple Rule Cards
    int cardW = 350;
    int cardH = 370;
    int gap = 20;
    int totalCardsW = 3 * cardW + 2 * gap; // 1090
    int startCardsX = (screenW - totalCardsW) / 2;
    int cardY = 126;

    // CARD 1: TRAIN & BOARDING
    int c1X = startCardsX;
    DrawRectangleRounded(Rectangle{(float)c1X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)c1X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{56, 189, 248, 255});
    DrawRectangleRounded(Rectangle{(float)c1X + 4, (float)cardY + 4, (float)cardW - 8, 28.0f}, 0.2f, 4, Color{14, 116, 144, 255});
    DrawText("1. TRAIN RUNS & BOARDS", c1X + 14, cardY + 10, 12, WHITE);

    int y1 = cardY + 42;
    DrawText("* Automatic Train Operation:", c1X + 14, y1, 11, Color{56, 189, 248, 255}); y1 += 18;
    DrawText("The EMU train cruises continuously on your", c1X + 14, y1, 10, Color{203, 213, 225, 255}); y1 += 16;
    DrawText("closed rail circuit at full line speed.", c1X + 14, y1, 10, Color{203, 213, 225, 255}); y1 += 24;

    DrawText("* Station Stops & PSD Doors:", c1X + 14, y1, 11, Color{56, 189, 248, 255}); y1 += 18;
    DrawText("When reaching a Station [7], the train decelerates", c1X + 14, y1, 10, Color{203, 213, 225, 255}); y1 += 16;
    DrawText("to a smooth halt and aligns with platform doors.", c1X + 14, y1, 10, Color{203, 213, 225, 255}); y1 += 24;

    DrawText("* Commuters & Shape Destinations:", c1X + 14, y1, 11, Color{255, 214, 0, 255}); y1 += 18;
    DrawText("Waiting riders board the train. Each rider wants", c1X + 14, y1, 10, Color{203, 213, 225, 255}); y1 += 16;
    DrawText("a specific station shape (Square, Circle, Cross).", c1X + 14, y1, 10, Color{203, 213, 225, 255}); y1 += 24;

    DrawText("* Earn Revenue on Delivery:", c1X + 14, y1, 11, Color{34, 197, 94, 255}); y1 += 18;
    DrawText("Disembarking commuters pay $2.50 ticket fare,", c1X + 14, y1, 10, Color{203, 213, 225, 255}); y1 += 16;
    DrawText("funding your track and concourse expansions!", c1X + 14, y1, 10, Color{203, 213, 225, 255}); y1 += 28;

    // Mini visual preview card footer
    DrawRectangleRounded(Rectangle{(float)c1X + 12, (float)cardY + cardH - 52, (float)cardW - 24, 40.0f}, 0.15f, 4, Color{15, 23, 42, 220});
    DrawText("SHAPES: [ ] Square  (O) Circle  (+) Cross", c1X + 22, cardY + cardH - 38, 10, Color{148, 163, 184, 255});

    // CARD 2: BUILD & EXPAND
    int c2X = startCardsX + cardW + gap;
    DrawRectangleRounded(Rectangle{(float)c2X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)c2X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{34, 197, 94, 255});
    DrawRectangleRounded(Rectangle{(float)c2X + 4, (float)cardY + 4, (float)cardW - 8, 28.0f}, 0.2f, 4, Color{21, 128, 61, 255});
    DrawText("2. BUILD & EXPAND", c2X + 14, cardY + 10, 12, WHITE);

    int y2 = cardY + 42;
    DrawText("* Rail Tools [Keys 1 - 8]:", c2X + 14, y2, 11, Color{34, 197, 94, 255}); y2 += 18;
    DrawText("1=Straight, 2=Turn Left, 3=Turn Right, 4=Viaduct,", c2X + 14, y2, 10, Color{203, 213, 225, 255}); y2 += 16;
    DrawText("5=Ramp, 6=Tunnel, 7=Station, 8=Block Signal.", c2X + 14, y2, 10, Color{203, 213, 225, 255}); y2 += 24;

    DrawText("* Rotate & Elevation Controls:", c2X + 14, y2, 11, Color{255, 214, 0, 255}); y2 += 18;
    DrawText("Press [R] to rotate track heading 90 deg.", c2X + 14, y2, 10, Color{203, 213, 225, 255}); y2 += 16;
    DrawText("Press [E] / [Q] to raise / lower elevation (Z=0..5).", c2X + 14, y2, 10, Color{203, 213, 225, 255}); y2 += 24;

    DrawText("* Concourse & Plazas [Tab 2]:", c2X + 14, y2, 11, Color{56, 189, 248, 255}); y2 += 18;
    DrawText("Pave sidewalks and queue areas so commuters can", c2X + 14, y2, 10, Color{203, 213, 225, 255}); y2 += 16;
    DrawText("walk from entrances to train platform edges.", c2X + 14, y2, 10, Color{203, 213, 225, 255}); y2 += 24;

    DrawText("* Station Amenities [Tab 3]:", c2X + 14, y2, 11, Color{251, 146, 60, 255}); y2 += 18;
    DrawText("Place benches, turnstiles, trees, lamps, and cafes", c2X + 14, y2, 10, Color{203, 213, 225, 255}); y2 += 16;
    DrawText("to dramatically raise commuter happiness!", c2X + 14, y2, 10, Color{203, 213, 225, 255}); y2 += 28;

    DrawRectangleRounded(Rectangle{(float)c2X + 12, (float)cardY + cardH - 52, (float)cardW - 24, 40.0f}, 0.15f, 4, Color{15, 23, 42, 220});
    DrawText("HOTKEYS: [1-8] Build  [R] Rotate  [X] Bulldoze", c2X + 22, cardY + cardH - 38, 10, Color{148, 163, 184, 255});

    // CARD 3: PREVENT OVERCROWDING
    int c3X = startCardsX + 2 * (cardW + gap);
    DrawRectangleRounded(Rectangle{(float)c3X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)c3X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{239, 68, 68, 255});
    DrawRectangleRounded(Rectangle{(float)c3X + 4, (float)cardY + 4, (float)cardW - 8, 28.0f}, 0.2f, 4, Color{185, 28, 28, 255});
    DrawText("3. AVOID OVERCROWDING", c3X + 14, cardY + 10, 12, WHITE);

    int y3 = cardY + 42;
    DrawText("* Commuter Station Crowds:", c3X + 14, y3, 11, Color{239, 68, 68, 255}); y3 += 18;
    DrawText("Passengers arrive at entrance gates and line up", c3X + 14, y3, 10, Color{203, 213, 225, 255}); y3 += 16;
    DrawText("at platform gates waiting for train arrivals.", c3X + 14, y3, 10, Color{203, 213, 225, 255}); y3 += 24;

    DrawText("* Danger: 10+ Backlog Countdown:", c3X + 14, y3, 11, Color{255, 214, 0, 255}); y3 += 18;
    DrawText("If 10 or more commuters wait at any single station,", c3X + 14, y3, 10, Color{203, 213, 225, 255}); y3 += 16;
    DrawText("an urgent Overcrowding Timer begins ticking!", c3X + 14, y3, 10, Color{203, 213, 225, 255}); y3 += 24;

    DrawText("* Clear the Backlog in Time:", c3X + 14, y3, 11, Color{239, 68, 68, 255}); y3 += 18;
    DrawText("Pick up commuters before the timer expires to", c3X + 14, y3, 10, Color{203, 213, 225, 255}); y3 += 16;
    DrawText("prevent platform riots and customer walkouts!", c3X + 14, y3, 10, Color{203, 213, 225, 255}); y3 += 24;

    DrawText("* Victory & Failure Conditions:", c3X + 14, y3, 11, Color{34, 197, 94, 255}); y3 += 18;
    DrawText("WIN: Deliver 500 passengers across districts.", c3X + 14, y3, 10, Color{203, 213, 225, 255}); y3 += 16;
    DrawText("FAIL: Satisfaction drops to 0% due to delays.", c3X + 14, y3, 10, Color{203, 213, 225, 255}); y3 += 28;

    DrawRectangleRounded(Rectangle{(float)c3X + 12, (float)cardY + cardH - 52, (float)cardW - 24, 40.0f}, 0.15f, 4, Color{15, 23, 42, 220});
    DrawText("GOAL: Deliver 500  |  AVOID: Overcrowd timer", c3X + 22, cardY + cardH - 38, 10, Color{239, 68, 68, 255});

    // 4. Large Glowing START Button
    int btnW = 360;
    int btnH = 54;
    int btnX = (screenW - btnW) / 2;
    int btnY = cardY + cardH + 16;

    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH});
    Color btnColor = isHovered ? Color{239, 68, 68, 255} : Color{220, 38, 38, 255};

    DrawRectangleRounded(Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH}, 0.28f, 6, btnColor);
    DrawRectangleRoundedLines(Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH}, 0.28f, 6, isHovered ? WHITE : Color{254, 202, 202, 255});

    DrawText("START PLAYING", btnX + 90, btnY + 12, 19, WHITE);
    DrawText("(or press SPACE / ENTER)", btnX + 105, btnY + 34, 10, Color{255, 214, 0, 255});

    // 5. Bottom Navigation & Control Reference Bar
    int ctrlY = btnY + btnH + 12;
    int ctrlW = totalCardsW;
    int ctrlH = 34;
    int ctrlX = startCardsX;
    DrawRectangleRounded(Rectangle{(float)ctrlX, (float)ctrlY, (float)ctrlW, (float)ctrlH}, 0.3f, 4, Color{23, 32, 51, 230});
    DrawRectangleRoundedLines(Rectangle{(float)ctrlX, (float)ctrlY, (float)ctrlW, (float)ctrlH}, 0.3f, 4, Color{51, 65, 85, 200});

    DrawText("CONTROLS:  [WASD / Drag] Pan Camera   |   [Mouse Wheel] Zoom   |   [1 - 8] Select Tool   |   [R] Rotate   |   [E / Q] Elevation   |   [F] Cab Cam   |   [?] How to Play", ctrlX + 16, ctrlY + 11, 10, Color{203, 213, 225, 255});
}

bool UserInterface::CheckTitleStartClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int cardH = 370;
    int cardY = 126;
    int btnW = 360;
    int btnH = 54;
    int btnX = (screenW - btnW) / 2;
    int btnY = cardY + cardH + 16;

    return CheckCollisionPointRec(mousePos, Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH});
}

int UserInterface::CheckToolbarTabClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = 740;
    int barH = 72;
    int auxW = 146;
    int totalDockW = barW + 12 + auxW;
    int barX = std::max(10, (screenW - totalDockW) / 2);
    int barY = screenH - barH - 12;

    int tabW = 140;
    int tabH = 24;
    int tabStartY = barY - tabH + 2;

    for (int t = 0; t < 3; ++t) {
        int tx = barX + 16 + t * (tabW + 8);
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4})) {
            return t;
        }
    }
    return -1;
}

int UserInterface::CheckToolbarItemClick(Vector2 mousePos, ToolCategory activeTab) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = 740;
    int barH = 72;
    int auxW = 146;
    int totalDockW = barW + 12 + auxW;
    int barX = std::max(10, (screenW - totalDockW) / 2);
    int barY = screenH - barH - 12;

    int itemBtnH = 52;
    int itemStartY = barY + 10;

    if (activeTab == CAT_INFRA) {
        int infraBtnW = 120;
        int spacing = 18;
        int startX = barX + (barW - (4 * infraBtnW + 3 * spacing)) / 2;

        for (int i = 0; i < 4; ++i) {
            int bx = startX + i * (infraBtnW + spacing);
            if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx, (float)itemStartY, (float)infraBtnW, (float)itemBtnH})) {
                return i;
            }
        }
    } else {
        int itemBtnW = 68;
        int spacing = 11;
        int startX = barX + (barW - (9 * itemBtnW + 8 * spacing)) / 2;

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + spacing);
            if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH})) {
                return i;
            }
        }
    }
    return -1;
}

bool UserInterface::CheckToolbarAuxClick(Vector2 mousePos, int& outZDelta, bool& outRotate) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = 740;
    int barH = 72;
    int auxW = 146;
    int totalDockW = barW + 12 + auxW;
    int barX = std::max(10, (screenW - totalDockW) / 2);
    int barY = screenH - barH - 12;
    int auxX = barX + barW + 12;

    // Height down [-]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 96, (float)barY + 7, 20.0f, 20.0f})) {
        outZDelta = -1;
        return true;
    }
    // Height up [+]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 120, (float)barY + 7, 20.0f, 20.0f})) {
        outZDelta = 1;
        return true;
    }
    // Rotate [ROT]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 104, (float)barY + 45, 34.0f, 20.0f})) {
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
    bool& outToggleHelp
) const {
    int screenW = GetScreenWidth();

    int btnX = screenW - 145;
    int actX = btnX - 12 - 322;

    // Line Ops Button
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)actX, 13.0f, 74.0f, 26.0f})) {
        outToggleStats = true;
        return true;
    }
    // Crew Button
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)actX + 80, 13.0f, 62.0f, 26.0f})) {
        outToggleStaff = true;
        return true;
    }
    // Cab Cam Button
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)actX + 148, 13.0f, 74.0f, 26.0f})) {
        outToggleRideCam = true;
        return true;
    }
    // Operations Manual Button (? HOW TO PLAY)
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)actX + 228, 13.0f, 94.0f, 26.0f})) {
        outToggleHelp = true;
        return true;
    }

    // Time controls
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX, 13.0f, 26.0f, 26.0f})) {
        outNewSpeed = 0;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 30, 13.0f, 26.0f, 26.0f})) {
        outNewSpeed = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 60, 13.0f, 26.0f, 26.0f})) {
        outNewSpeed = 2;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 90, 13.0f, 44.0f, 26.0f})) {
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

    return false;
}

bool UserInterface::CheckPeepInspectorCloseClick(Vector2 mousePos) const {
    int cardW = 300;
    int cardX = 18;
    int cardY = 64;

    return CheckCollisionPointRec(mousePos, Rectangle{(float)cardX + cardW - 28, (float)cardY + 8, 24.0f, 24.0f});
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
    int bx = (screenW - 480) / 2;
    int by = (screenH - 260) / 2;

    Rectangle r = {(float)bx + 140, (float)by + 175, 200.0f, 45.0f};
    return CheckCollisionPointRec(mousePos, r);
}

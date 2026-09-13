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
    (void)satisfaction; (void)week; (void)weekTimer;
    (void)signalAspect;
    (void)lineOpsOpen; (void)crewOpen; (void)cabCamActive; (void)rushCombo;

    int screenW = GetScreenWidth();

    // Top Bar Background (Operations Control Center Navy)
    DrawRectangle(0, 0, screenW, 52, Color{15, 23, 42, 248});
    DrawLine(0, 52, screenW, 52, Color{51, 65, 85, 255});

    // 1. Metro Brand Logo & Roundel Badge
    DrawCircle(28, 26, 15, Color{220, 38, 38, 255});
    DrawCircle(28, 26, 11, Color{15, 23, 42, 255});
    DrawRectangle(17, 23, 22, 7, Color{220, 38, 38, 255});
    DrawText("M", 23, 18, 15, WHITE);
    DrawGameBoldText("METRO GRID", 50, 12, 16, Color{248, 250, 252, 255});
    DrawText("URBAN TRANSIT SIMULATOR", 50, 30, 12, Color{148, 163, 184, 255});

    // 2. Transit Authority Treasury (cash always in the corner)
    int bankX = 190;
    DrawText("CASH", bankX, 8, 13, Color{148, 163, 184, 255});
    DrawGameBoldText(TextFormat("$%.0f", balance), bankX, 18, 20, Color{52, 211, 153, 255});

    // 3. Commuters Transported + Goal progress bar
    int scoreX = 286;
    DrawText("RIDERS", scoreX, 8, 13, Color{148, 163, 184, 255});
    DrawGameBoldText(TextFormat("%d", ridership), scoreX, 16, 21, Color{255, 214, 0, 255});
    const int GOAL = 500;
    float goalPct = std::max(0.0f, std::min(1.0f, (float)ridership / (float)GOAL));
    DrawRectangle(scoreX, 42, 110, 6, Color{51, 65, 85, 255});
    DrawRectangle(scoreX, 42, (int)(110.0f * goalPct), 6, Color{255, 214, 0, 255});
    DrawText(TextFormat("GOAL %d", GOAL), scoreX + 116, 38, 13, Color{148, 163, 184, 255});

    // 4. Train speed (single clean number)
    int spdX = 500;
    Color spdColor = (speedKmh > 55.0f) ? Color{56, 189, 248, 255} : Color{203, 213, 225, 255};
    DrawText(TextFormat("%.0f km/h", speedKmh), spdX, 12, 18, spdColor);
    DrawText("TRAIN", spdX, 34, 13, Color{148, 163, 184, 255});

    // 4b. Circuit status badge (OPEN TRACK = red, LOOP CLOSED = green)
    int badgeX = spdX + 130;
    Color badgeBg = circuitClosed ? Color{22, 101, 52, 255} : Color{185, 28, 28, 255};
    Color badgeBorder = circuitClosed ? Color{34, 197, 94, 255} : Color{239, 68, 68, 255};
    DrawRectangleRounded(Rectangle{(float)badgeX, 10.0f, 110.0f, 30.0f}, 0.3f, 4, badgeBg);
    DrawRectangleRoundedLines(Rectangle{(float)badgeX, 10.0f, 110.0f, 30.0f}, 0.3f, 4, badgeBorder);
    const char* badgeText = circuitClosed ? "LOOP CLOSED" : "OPEN TRACK";
    DrawText(badgeText, badgeX + 8, 16, 14, circuitClosed ? Color{110, 231, 183, 255} : WHITE);
    if (!circuitClosed) {
        DrawText("CLOSE THE LOOP", badgeX + 8, 32, 9, Color{254, 202, 202, 255});
    }

    // 5. Simulation controls & mute (far right anchor)
    int btnX = screenW - 140;

    // 6. "How to Play" — a single, obvious help button
    int helpW = 158;
    int helpX = btnX - 12 - helpW;
    DrawRectangleRounded(Rectangle{(float)helpX, 12.0f, (float)helpW, 28.0f}, 0.25f, 4, helpOpen ? Color{220, 38, 38, 255} : Color{14, 116, 144, 240});
    DrawRectangleRoundedLines(Rectangle{(float)helpX, 12.0f, (float)helpW, 28.0f}, 0.25f, 4, Color{56, 189, 248, 255});
    DrawText("HOW TO PLAY  [?]", helpX + 21, 18, 14, helpOpen ? WHITE : Color{255, 214, 0, 255});

    // Pause / Normal / Fast
    DrawRectangleRounded(Rectangle{(float)btnX, 12.0f, 28.0f, 28.0f}, 0.2f, 4, (gameSpeed == 0) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawText("||", btnX + 9, 17, 14, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 32, 12.0f, 28.0f, 28.0f}, 0.2f, 4, (gameSpeed == 1) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawText(">", btnX + 41, 17, 14, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 64, 12.0f, 28.0f, 28.0f}, 0.2f, 4, (gameSpeed == 2) ? Color{220, 38, 38, 255} : Color{30, 41, 59, 200});
    DrawText(">>", btnX + 68, 17, 14, WHITE);

    // Mute
    DrawRectangleRounded(Rectangle{(float)btnX + 96, 12.0f, 42.0f, 28.0f}, 0.2f, 4, muted ? Color{239, 68, 68, 220} : Color{30, 41, 59, 200});
    DrawText(muted ? "MUTE" : "SOUND", btnX + 102, 18, 12, WHITE);
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
    int extraTrainCount
) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int barW = ToolbarMetrics::BAR_W;
    int barH = ToolbarMetrics::BAR_H;
    int auxW = ToolbarMetrics::AUX_W;
    int barX = ToolbarMetrics::BarX(screenW);
    int barY = ToolbarMetrics::BarY(screenH);

    // 1. Category Tabs above toolbar
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
        DrawGameBoldTextCentered(tabNames[t], tx + 75, tabStartY + 8, 15, tabText);
    }

    // 2. Toolbar Body Card
    DrawRectangleRounded(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.2f, 6, Color{15, 23, 42, 248});
    DrawRectangleRoundedLines(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.2f, 6, Color{51, 65, 85, 255});

    // 3. Render Items according to Active Tab
    int itemBtnW = ToolbarMetrics::ITEM_W;
    int itemBtnH = ToolbarMetrics::ITEM_H;
    int itemStartY = barY + ToolbarMetrics::ITEM_Y;

    if (activeTab == CAT_TRACK) {
        struct TrackBtn { const char* name; const char* key; TrackType type; };
        TrackBtn buttons[] = {
            {"Straight", "1", TRACK_STRAIGHT},
            {"L-Turn", "2", TRACK_CURVE_LEFT},
            {"R-Turn", "3", TRACK_CURVE_RIGHT},
            {"Viaduct", "4", TRACK_VIADUCT_ELEVATED},
            {"Slope", "5", TRACK_VIADUCT_SLOPE},
            {"Tunnel", "6", TRACK_TUNNEL_PORTAL},
            {"Station", "7", TRACK_STATION},
            {"Signal", "8", TRACK_SIGNAL},
            {"Demolish", "X", TRACK_NONE}
        };

        int spacing = 10;
        int startX = barX + (barW - (9 * itemBtnW + 8 * spacing)) / 2;

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + spacing);
            bool isSelected = (i == 8) ? isBulldozing : (!isBulldozing && currentTrack == buttons[i].type);

            Color btnBg = isSelected ? Color{220, 38, 38, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{254, 202, 202, 255} : Color{71, 85, 105, 200});

            DrawText(buttons[i].key, bx + 6, itemStartY + 8, 16, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 4, itemStartY + 34, 15, textC);
        }
    } else if (activeTab == CAT_INFRA) {
        struct InfraBtn { const char* name; const char* key; GroundType ground; bool isBull; };
        InfraBtn buttons[] = {
            {"Sidewalk", "1", GROUND_PATH, false},
            {"Queue Line", "2", GROUND_QUEUE, false},
            {"Plaza", "3", GROUND_PLAZA, false},
            {"Demolish", "X", GROUND_GRASS, true}
        };

        int infraBtnW = 150;
        int spacing = 20;
        int startX = barX + (barW - (4 * infraBtnW + 3 * spacing)) / 2;

        for (int i = 0; i < 4; ++i) {
            int bx = startX + i * (infraBtnW + spacing);
            bool isSelected = buttons[i].isBull ? isBulldozing : (!isBulldozing && currentGround == buttons[i].ground);

            Color btnBg = isSelected ? Color{220, 38, 38, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)infraBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)infraBtnW, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{254, 202, 202, 255} : Color{71, 85, 105, 200});

            DrawText(buttons[i].key, bx + 8, itemStartY + 8, 16, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 32, itemStartY + 34, 15, textC);
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
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{254, 202, 202, 255} : Color{71, 85, 105, 200});
DrawText(buttons[i].key, bx + 6, itemStartY + 8, 16, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 4, itemStartY + 34, 15, textC);

            if (buttons[i].cost > 0) {
                DrawText(TextFormat("$%d", buttons[i].cost), bx + 6, itemStartY + 62, 13, Color{52, 211, 153, 255});
            }
        }
    }

    // 3b. Fleet & Island info strip inside toolbar bottom padding
    DrawText(TextFormat("TRAIN: %d CARS  |  ISLAND R: %d", carCount, buildRadius), barX + 16, barY + ToolbarMetrics::BUY_Y + ToolbarMetrics::BUY_H - 16, 13, Color{148, 163, 184, 255});

    // Buy Extra Train (RCT-style purchase) on the right side of the strip
    int exX = barX + barW - 176;
    if (extraTrainCount >= Game::MaxExtraTrains()) {
        DrawRectangleRounded(Rectangle{(float)exX, (float)barY + ToolbarMetrics::BUY_Y - 2, 160.0f, (float)ToolbarMetrics::BUY_H}, 0.3f, 4, Color{74, 222, 128, 255});
        DrawText("FLEET AT MAX (4 EMU)", exX + 8, barY + ToolbarMetrics::BUY_Y + 2, 12, WHITE);
    } else {
        int nextCost = (int)Game::ExtraTrainCostP(extraTrainCount);
        Color extraC = (balance >= nextCost) ? Color{56, 189, 248, 255} : Color{71, 85, 105, 255};
        DrawRectangleRounded(Rectangle{(float)exX, (float)barY + ToolbarMetrics::BUY_Y - 2, 160.0f, (float)ToolbarMetrics::BUY_H}, 0.3f, 4, extraC);
        DrawText(extraTrainCount > 0
                     ? TextFormat("EXTRA TRAIN #%d $%d", extraTrainCount + 1, nextCost)
                     : TextFormat("BUY EXTRA TRAIN $%d", nextCost),
                 exX + 8, barY + ToolbarMetrics::BUY_Y + 2, 12, WHITE);
    }

    // 4. Auxiliary Pills on Right (Elevation & Heading)
    int auxX = barX + barW + 12;

    // Height Pill
    DrawRectangleRounded(Rectangle{(float)auxX, (float)barY, (float)auxW, 34.0f}, 0.25f, 4, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)barY, (float)auxW, 34.0f}, 0.25f, 4, Color{51, 65, 85, 255});
    DrawText("HEIGHT (E/Q)", auxX + 8, barY + 3, 12, Color{148, 163, 184, 255});
    DrawText(TextFormat("Z = %d", currentZ), auxX + 9, barY + 16, 18, Color{255, 214, 0, 255});
    DrawRectangle((float)auxX + 96, (float)barY + 6, 20, 22, Color{30, 41, 59, 255});
    DrawText("-", auxX + 103, barY + 9, 17, WHITE);
    DrawRectangle((float)auxX + 120, (float)barY + 6, 20, 22, Color{30, 41, 59, 255});
    DrawText("+", auxX + 126, barY + 9, 17, WHITE);

    // Heading / Orientation Pill
    const char* dirLabels[] = {"NORTH [^]", "EAST [>]", "SOUTH [v]", "WEST [<]"};
    DrawRectangleRounded(Rectangle{(float)auxX, (float)barY + 38, (float)auxW, 34.0f}, 0.25f, 4, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)barY + 38, (float)auxW, 34.0f}, 0.25f, 4, Color{51, 65, 85, 255});
    DrawText("HEADING [R]", auxX + 8, barY + 41, 12, Color{148, 163, 184, 255});
    DrawText(dirLabels[currentDir], auxX + 8, barY + 53, 16, Color{56, 189, 248, 255});
    DrawRectangle((float)auxX + 104, (float)barY + 44, 34, 22, Color{30, 41, 59, 255});
    DrawText("ROT", auxX + 110, barY + 48, 11, WHITE);

    // Big Buy Buttons (RCT-style purchases): Train Car + Land
    int buyY = barY + ToolbarMetrics::BUY_Y;
    int buyH = ToolbarMetrics::BUY_H;
    Color carOk = (balance >= 800.0f) ? Color{56, 189, 248, 255} : Color{71, 85, 105, 255};
    Color landOk = (balance >= 600.0f) ? Color{74, 222, 128, 255} : Color{71, 85, 105, 255};

    DrawRectangleRounded(Rectangle{(float)auxX + 4, (float)buyY, 82.0f, (float)buyH}, 0.3f, 4, carOk);
    DrawText(TextFormat("CAR+1 $%d", (int)800), auxX + 5, buyY + 4, 12, WHITE);

    DrawRectangleRounded(Rectangle{(float)auxX + 90, (float)buyY, 50.0f, (float)buyH}, 0.3f, 4, landOk);
    DrawText(TextFormat("LAND $%d", (int)600), auxX + 91, buyY + 4, 12, WHITE);
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
    int screenW = GetScreenWidth();
    int cardX = screenW - cardW - 18;
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

    // Name & Destination
    DrawText(commuter->name.c_str(), cardX + 72, cardY + 13, 13, WHITE);
    DrawGameBoldText(TextFormat("Wants: %s STATION", GetShapeName(commuter->targetShape)), cardX + 72, cardY + 29, 10, sc);
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
    int bannerY = screenH - 192;

    DrawRectangleRounded(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{15, 23, 42, 235});
    DrawRectangleRoundedLines(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{56, 189, 248, 200});

    DrawCircle(bannerX + 14, bannerY + 15, 3.5f, Color{255, 214, 0, 255});
    DrawText(tip.c_str(), bannerX + 24, bannerY + 9, 13, Color{241, 245, 249, 255});
}

static void DrawTutorialWrapped(const char* text, int x, int y, int fontSize, Color c, int maxWidth) {
    const char* p = text;
    int yy = y;
    int cw = (int)(fontSize * 0.66f);
    int maxChars = std::max(10, maxWidth / cw);
    while (p && *p) {
        int used = 0;
        std::string line;
        while (*p && used < maxChars) { line += *p; ++p; ++used; }
        line += '\0';
        DrawText(line.c_str(), x, yy, fontSize, c);
        yy += fontSize + 4;
    }
}

void UserInterface::DrawTutorialPanel(bool visible, const TutorialStageInfo& stage, int currentIdx, int total, const bool* doneFlags) {
    if (!visible || total <= 0) return;

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int cardW = std::min(920, screenW - 40);
    int cardX = (screenW - cardW) / 2;
    int cardY = 58;
    int cardH = 108;
    float pulse = 0.5f + 0.5f * sinf(GetTime() * 5.0f);

    // ---- Compact arcade directive card (sits below the HUD) ----
    DrawRectangleRounded(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.16f, 8, Color{15, 23, 42, 242});
    DrawRectangleRoundedLines(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.16f, 8,
                              Color{(unsigned char)(pulse ? 255 : 214), (unsigned char)(pulse ? 214 : 11), 0, 255});

    // Lesson chip
    DrawRectangleRounded(Rectangle{(float)cardX + 14, (float)cardY + 8, 130.0f, 20.0f}, 0.4f, 4, Color{220, 38, 38, 255});
    DrawText(TextFormat("LESSON %d / %d", currentIdx + 1, total), cardX + 22, cardY + 12, 12, WHITE);
    DrawText("(G = hide)", cardX + cardW - 78, cardY + 12, 10, Color{148, 163, 184, 255});

    // Bold directive title (single line)
    DrawGameBoldText(stage.title, cardX + 14, cardY + 32, 18, Color{248, 250, 252, 255});

    // Body text (wrapped, compact)
    DrawTutorialWrapped(stage.body, cardX + 14, cardY + 54, 12, Color{203, 213, 225, 255}, cardW - 28);

    // Hint pill
    if (stage.hint[0] != '\0') {
        DrawRectangleRounded(Rectangle{(float)cardX + 14, (float)cardY + 82, 220.0f, 20.0f}, 0.4f, 4, Color{30, 41, 59, 255});
        DrawRectangleRoundedLines(Rectangle{(float)cardX + 14, (float)cardY + 82, 220.0f, 20.0f}, 0.4f, 4, Color{255, 214, 0, 180});
        DrawText(stage.hint, cardX + 20, cardY + 86, 11, Color{255, 214, 0, 255});
    }

    // ---- Progress dots (inside card, bottom row) ----
    int dotY = cardY + cardH - 18;
    int dotGap = 16;
    int dotsStartX = cardX + cardW / 2 - (total / 2) * dotGap - (total % 2) * dotGap / 2;
    for (int i = 0; i < total; ++i) {
        int ddx = dotsStartX + i * dotGap;
        if (doneFlags[i]) {
            DrawRectangleRounded(Rectangle{(float)ddx, (float)dotY, 10.0f, 10.0f}, 0.35f, 4, Color{34, 197, 94, 255});
        } else if (i == currentIdx) {
            DrawCircleLines(ddx + 5, dotY + 5, 7.0f + 1.5f * pulse, Color{255, 214, 0, 255});
            DrawRectangleRounded(Rectangle{(float)ddx, (float)dotY, 10.0f, 10.0f}, 0.35f, 4, Color{255, 214, 0, 200});
        } else {
            DrawRectangleRounded(Rectangle{(float)ddx, (float)dotY, 10.0f, 10.0f}, 0.35f, 4, Color{71, 85, 105, 255});
        }
    }

    // ---- Pulsing ring + 'CLICK ME' arrow on the required toolbar button ----
    if (stage.toolbarTag > 0) {
        int barW = ToolbarMetrics::BAR_W;
        int auxW = ToolbarMetrics::AUX_W;
        int barX = ToolbarMetrics::BarX(screenW);
        int barY = ToolbarMetrics::BarY(screenH);
        int auxX = ToolbarMetrics::AuxX(screenW);

        // RCT-style spotlight: dim the whole dock, keep only the target bright
        DrawRectangle(barX - 12, barY - 10, barW + ToolbarMetrics::GAP + auxW + 24, ToolbarMetrics::BAR_H + 22, Color{2, 6, 23, 170});
        int cx = 0, cy = 0;
        if (stage.toolbarTag == 1) {
            DrawRectangleRounded(Rectangle{(float)auxX + 4, (float)barY + ToolbarMetrics::BUY_Y, 82.0f, (float)ToolbarMetrics::BUY_H}, 0.3f, 4, Color{56, 189, 248, 255});
            DrawText(TextFormat("CAR+1 $%d", (int)800), auxX + 5, barY + ToolbarMetrics::BUY_Y + 4, 12, WHITE);
            cx = auxX + 4 + 41;  cy = barY + ToolbarMetrics::BUY_Y + ToolbarMetrics::BUY_H / 2;
        } else if (stage.toolbarTag == 2) {
            DrawRectangleRounded(Rectangle{(float)auxX + 90, (float)barY + ToolbarMetrics::BUY_Y, 50.0f, (float)ToolbarMetrics::BUY_H}, 0.3f, 4, Color{74, 222, 128, 255});
            DrawText(TextFormat("LAND $%d", (int)600), auxX + 91, barY + ToolbarMetrics::BUY_Y + 4, 12, WHITE);
            cx = auxX + 90 + 25; cy = barY + ToolbarMetrics::BUY_Y + ToolbarMetrics::BUY_H / 2;
        } else if (stage.toolbarTag == 3) {
            DrawRectangleRounded(Rectangle{(float)barX + barW - 176, (float)barY + ToolbarMetrics::BUY_Y - 2, 160.0f, (float)ToolbarMetrics::BUY_H}, 0.3f, 4, Color{56, 189, 248, 255});
            DrawText("BUY EXTRA TRAIN $1,400", barX + barW - 168, barY + ToolbarMetrics::BUY_Y + 2, 12, WHITE);
            cx = barX + barW - 176 + 80; cy = barY + ToolbarMetrics::BUY_Y - 2 + ToolbarMetrics::BUY_H / 2;
        }

        float ringR = 20.0f + 6.0f * pulse;
        Color ringC = Color{255, 214, 0, 255};
        DrawCircleLines(cx, cy, ringR, ringC);
        DrawCircleLines(cx, cy, ringR * 0.7f, Color{255, 214, 0, 130});
        // Bouncing arrow above the button
        int arrowY = cy - (int)ringR - 28 + (int)(8.0f * pulse);
        DrawTriangle(Vector2{(float)cx, (float)(arrowY - 10)}, Vector2{(float)cx - 9, (float)arrowY}, Vector2{(float)cx + 9, (float)arrowY}, ringC);
        DrawText("CLICK ME!", cx - 30, arrowY - 28, 12, Color{255, 214, 0, 255});
    }
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
    DrawGameBoldText("STEP 1: BUILD TRACK LOOP", card1X + 12, cardY + 10, 11, WHITE);

    int c1y = cardY + 36;
    DrawText("* [1] Straight Rails", card1X + 10, c1y, 11, Color{255, 214, 0, 255}); c1y += 17;
    DrawText("* [2] L-Turn Left (90 CCW)", card1X + 10, c1y, 11, Color{255, 214, 0, 255}); c1y += 17;
    DrawText("* [3] R-Turn Right (90 CW)", card1X + 10, c1y, 11, Color{255, 214, 0, 255}); c1y += 21;
    DrawText("* [R] Change Heading (N/E/S/W)", card1X + 10, c1y, 11, Color{56, 189, 248, 255}); c1y += 20;
    DrawText("Placement auto-advances your", card1X + 10, c1y, 10, Color{203, 213, 225, 255}); c1y += 15;
    DrawText("heading in the track's direction.", card1X + 10, c1y, 10, Color{203, 213, 225, 255}); c1y += 17;
    DrawText("Yellow arrow on ghost shows", card1X + 10, c1y, 10, Color{250, 204, 21, 255}); c1y += 15;
    DrawText("exact train travel path!", card1X + 10, c1y, 10, Color{250, 204, 21, 255}); c1y += 18;
    DrawText("When loop connects, trains", card1X + 10, c1y, 10, Color{74, 222, 128, 255}); c1y += 15;
    DrawText("launch into regular service!", card1X + 10, c1y, 10, Color{74, 222, 128, 255});

    // CARD 2: STATIONS & SIGNALS
    DrawRectangleRounded(Rectangle{(float)card2X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)card2X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{52, 211, 153, 255});
    DrawRectangleRounded(Rectangle{(float)card2X + 4, (float)cardY + 4, (float)cardW - 8, 24.0f}, 0.2f, 4, Color{16, 149, 106, 255});
    DrawGameBoldText("STEP 2: ADD STATIONS", card2X + 14, cardY + 10, 11, WHITE);

    int c2y = cardY + 36;
    DrawText("* [7] Station Platforms", card2X + 10, c2y, 11, Color{255, 214, 0, 255}); c2y += 17;
    DrawText("Trains halt at PSD platform", card2X + 10, c2y, 10, Color{203, 213, 225, 255}); c2y += 15;
    DrawText("doors to board commuters.", card2X + 10, c2y, 10, Color{203, 213, 225, 255}); c2y += 21;
    DrawText("* [8] 3-Aspect Signals", card2X + 10, c2y, 11, Color{255, 214, 0, 255}); c2y += 17;
    DrawText("Green/Yellow/Red wayside lights", card2X + 10, c2y, 10, Color{203, 213, 225, 255}); c2y += 15;
    DrawText("prevent rear-end collisions.", card2X + 10, c2y, 10, Color{203, 213, 225, 255}); c2y += 21;
    DrawText("Tab 2 & 3: Concourse & Plaza", card2X + 10, c2y, 10, Color{56, 189, 248, 255}); c2y += 15;
    DrawText("Build subway stairs, turnstiles,", card2X + 10, c2y, 10, Color{203, 213, 225, 255}); c2y += 15;
    DrawText("kiosks, and lampposts!", card2X + 10, c2y, 10, Color{203, 213, 225, 255});

    // CARD 3: COMMUTER TRIAGE & WIN
    DrawRectangleRounded(Rectangle{(float)card3X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)card3X, (float)cardY, (float)cardW, (float)cardH}, 0.12f, 4, Color{251, 146, 60, 255});
    DrawRectangleRounded(Rectangle{(float)card3X + 4, (float)cardY + 4, (float)cardW - 8, 24.0f}, 0.2f, 4, Color{194, 65, 12, 255});
    DrawGameBoldText("STEP 3: COMMUTERS ARE AUTOMATIC", card3X + 10, cardY + 10, 11, WHITE);

    int c3y = cardY + 36;
    DrawText("* They do everything by themselves:", card3X + 10, c3y, 11, Color{255, 214, 0, 255}); c3y += 17;
    DrawText("arrive, swipe, queue, board, and ride.", card3X + 10, c3y, 10, Color{203, 213, 225, 255}); c3y += 18;
    DrawText("* Each has a shape badge on their head.", card3X + 10, c3y, 11, Color{255, 214, 0, 255}); c3y += 17;
    DrawText("Square / Circle / Triangle / Cross - they", card3X + 10, c3y, 10, Color{203, 213, 225, 255}); c3y += 15;
    DrawText("get off at the first station with that shape.", card3X + 10, c3y, 10, Color{203, 213, 225, 255}); c3y += 18;
    DrawText("* If a platform gets too crowded, a red", card3X + 10, c3y, 10, Color{239, 68, 68, 255}); c3y += 15;
    DrawText("clock appears - the train clears it each lap.", card3X + 10, c3y, 10, Color{239, 68, 68, 255}); c3y += 18;
    DrawText("* You never control commuters directly, like", card3X + 10, c3y, 10, Color{56, 189, 248, 255}); c3y += 15;
    DrawText("RollerCoaster Tycoon. Build track + stations", card3X + 10, c3y, 10, Color{56, 189, 248, 255}); c3y += 15;
    DrawText("and let the city come to you. Deliver 500!", card3X + 10, c3y, 10, Color{74, 222, 128, 255});

    // BOTTOM SHORTCUTS BOX
    int scY = cardY + cardH + 10;
    int scH = 124;
    DrawRectangleRounded(Rectangle{(float)bx + 16, (float)scY, (float)boxW - 32, (float)scH}, 0.08f, 4, Color{23, 32, 51, 240});
    DrawRectangleRoundedLines(Rectangle{(float)bx + 16, (float)scY, (float)boxW - 32, (float)scH}, 0.08f, 4, Color{51, 65, 85, 255});

    DrawGameBoldText("OCC DISPATCHER KEYBOARD & MOUSE CONTROLS", bx + 28, scY + 8, 11, Color{255, 214, 0, 255});

    int kCol1 = bx + 28;
    int kCol2 = bx + 380;
    int ky = scY + 30;

    DrawText("WASD / arrows / edges: pan   Right/Middle drag: pull map   HOME: recenter   Wheel: zoom", kCol1, ky, 10, Color{203, 213, 225, 255});
    DrawText("Tab 1 / 2 / 3 : Switch Track / Concourse / Scenery", kCol2, ky, 10, Color{203, 213, 225, 255});
    ky += 17;
    DrawText("Mouse Scroll      : Zoom In / Zoom Out", kCol1, ky, 10, Color{203, 213, 225, 255});
    DrawText("X             : Bulldoze / Demolish track & props", kCol2, ky, 10, Color{203, 213, 225, 255});
    ky += 17;
    DrawText("F                 : Driver Cab Camera (Ride EMU)", kCol1, ky, 10, Color{203, 213, 225, 255});
    DrawText("E / Q         : Raise / Lower Elevation (Z=0..5)", kCol2, ky, 10, Color{203, 213, 225, 255});
    ky += 17;
    DrawText("Left Click        : Lay piece or Inspect Commuter", kCol1, ky, 10, Color{203, 213, 225, 255});
    DrawText("O / P         : Line Operations / Transit Crew Window", kCol2, ky, 10, Color{203, 213, 225, 255});
    ky += 17;
    DrawText("BUY Train Car / Land (window) : Add cars or", kCol1, ky, 10, Color{52, 211, 153, 255});
    DrawText("expand the island via toolbar side buttons", kCol2, ky, 10, Color{52, 211, 153, 255});

    // CLOSE BUTTON
    int btnW = 240;
    int btnH = 34;
    int btnX = bx + (boxW - btnW) / 2;
    int btnY = by + boxH - 44;

    DrawRectangleRounded(Rectangle{(float)btnX, (float)btnY, (float)btnW, (float)btnH}, 0.3f, 4, Color{220, 38, 38, 255});
    DrawGameBoldText("START DISPATCHING (CLOSE)", btnX + 24, btnY + 11, 11, WHITE);
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

void UserInterface::DrawGameOver(int finalRidership, int stars, int best) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 210});

    int boxW = 480;
    int boxH = 290;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{220, 38, 38, 255});

    float fl = 0.6f + 0.4f * sinf(GetTime() * 5.0f);
    DrawGameBoldText("NETWORK GRIDLOCK: OVERCROWDING!", bx + 35, by + 26, 19, Color{239, 68, 68, (unsigned char)(255 * fl)});
    DrawText("Overcrowding triage timers expired and the platforms collapsed.", bx + 35, by + 62, 12, Color{203, 213, 225, 255});
    DrawGameBoldText(TextFormat("Total Commuters Transported: %d", finalRidership), bx + 100, by + 96, 16, Color{255, 214, 0, 255});

    // Arcade star rating earned this run
    char starsStr[16];
    for (int i = 0; i < 3; ++i) starsStr[i] = (i < stars) ? '*' : '-';
    starsStr[3] = '\0';
    DrawGameBoldText(TextFormat("RATING: %s", starsStr), bx + 165, by + 130, 18, Color{255, 214, 0, 255});
    DrawText(TextFormat("SESSION BEST: %d riders", best), bx + 140, by + 158, 12, Color{148, 163, 184, 255});

    DrawRectangleRounded(Rectangle{(float)bx + 140, (float)by + 192, 200.0f, 45.0f}, 0.3f, 4, Color{220, 38, 38, 255});
    DrawText("RESTART NETWORK", bx + 165, by + 206, 15, WHITE);
}

void UserInterface::DrawVictory(int finalRidership, int weeks, int stars, float balance, int best) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 215});

    int boxW = 600;
    int boxH = 350;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{34, 197, 94, 255});

    float fl = 0.6f + 0.4f * sinf(GetTime() * 4.0f);
    DrawGameBoldText("TRANSIT TRIUMPH! 500 COMMUTERS!", bx + 65, by + 24, 21, Color{34, 197, 94, (unsigned char)(255 * fl)});
    DrawText("Your rapid transit network connected all district shapes flawlessly!", bx + 60, by + 60, 13, Color{203, 213, 225, 255});
    DrawGameBoldText(TextFormat("Total Commuters Delivered: %d", finalRidership), bx + 160, by + 88, 17, Color{255, 214, 0, 255});

    // Big radiating arcade STAR RATING (★★★)
    char starsStr[16];
    for (int i = 0; i < 3; ++i) starsStr[i] = (i < stars) ? '*' : '-';
    starsStr[3] = '\0';
    DrawGameBoldText(starsStr, bx + 150, by + 118, 26, Color{255, 214, 0, 255});
    DrawText(TextFormat("Run: %d riders in %d weeks | Budget: $%.0f | Session Best: %d", finalRidership, weeks, balance, best), bx + 60, by + 156, 12, Color{148, 163, 184, 255});

    // 1. Continue in Endless Metropolis Mode button (Green glowing)
    int btnW = 340;
    int btnH = 44;
    int btn1X = bx + (boxW - btnW) / 2;
    int btn1Y = by + 186;
    DrawRectangleRounded(Rectangle{(float)btn1X, (float)btn1Y, (float)btnW, (float)btnH}, 0.3f, 4, Color{16, 185, 129, 255});
    DrawText("CONTINUE IN ENDLESS MODE (LOOP)", btn1X + 22, btn1Y + 13, 15, WHITE);

    // 2. Restart Network button
    int btn2W = 200;
    int btn2H = 32;
    int btn2X = bx + (boxW - btn2W) / 2;
    int btn2Y = by + 252;
    DrawRectangleRounded(Rectangle{(float)btn2X, (float)btn2Y, (float)btn2W, (float)btn2H}, 0.3f, 4, Color{51, 65, 85, 255});
    DrawText("START NEW NETWORK", btn2X + 22, btn2Y + 8, 13, Color{203, 213, 225, 255});
}

void UserInterface::DrawPauseOverlay() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    DrawRectangle(0, 0, screenW, screenH, Color{2, 6, 23, 190});
    float fl = 0.5f + 0.5f * sinf(GetTime() * 5.0f);
    DrawGameBoldTextCentered("PAUSED", (float)screenW / 2.0f, (float)(screenH / 2 - 120), 58, Color{255, 214, 0, (unsigned char)(255 * fl)});
    DrawText("THE CITY WAITS FOR YOU...", screenW / 2 - 105, screenH / 2 - 52, 14, Color{226, 232, 240, 255});

    int cx = screenW / 2;
    int by = screenH / 2 + 8;

    // RESUME (primary)
    DrawRectangleRounded(Rectangle{(float)cx - 130, (float)by, 260.0f, 44.0f}, 0.3f, 4, Color{16, 185, 129, 255});
    DrawRectangleRoundedLines(Rectangle{(float)cx - 130, (float)by, 260.0f, 44.0f}, 0.3f, 4, Color{209, 250, 229, 255});
    DrawGameBoldTextCentered("RESUME  [ESC]", (float)cx, (float)by + 13, 18, WHITE);

    // RESTART + QUIT TO MENU (secondary)
    DrawRectangleRounded(Rectangle{(float)cx - 155, (float)by + 56, 150.0f, 38.0f}, 0.3f, 4, Color{220, 38, 38, 220});
    DrawText("RESTART RUN", cx - 138, by + 71, 13, WHITE);

    DrawRectangleRounded(Rectangle{(float)cx + 5, (float)by + 56, 150.0f, 38.0f}, 0.3f, 4, Color{51, 65, 85, 255});
    DrawText("QUIT TO MENU", cx + 20, by + 71, 13, Color{226, 232, 240, 255});

    DrawText("[WASD / window edges] Move view   [Right or Middle drag] Pan   [HOME] Recenter   [Wheel] Zoom   [M] Mute", screenW / 2 - 262, screenH / 2 + 126, 12, Color{56, 189, 248, 255});
}

int UserInterface::CheckPauseClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int cx = screenW / 2;
    int by = screenH / 2 + 8;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx - 130, (float)by, 260.0f, 44.0f})) return 0;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx - 155, (float)by + 56, 150.0f, 38.0f})) return 1;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx + 5, (float)by + 56, 150.0f, 38.0f})) return 2;
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

    DrawGameBoldText("METRO GRID", cx - 140, headerY, 34, Color{248, 250, 252, 255});
    DrawText("Urban Rapid Transit Tycoon & Commuter Flow Dispatcher", cx - 190, headerY + 40, 13, Color{56, 189, 248, 255});

    // Goal Banner Pill
    int bannerW = 660;
    int bannerH = 26;
    int bannerX = (screenW - bannerW) / 2;
    int bannerY = headerY + 62;
    DrawRectangleRounded(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{30, 41, 59, 230});
    DrawRectangleRoundedLines(Rectangle{(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH}, 0.5f, 4, Color{245, 158, 11, 200});
    DrawText("MISSION: Serve 500 commuters. Follow the OBJECTIVES checklist (top-left), step by step!", bannerX + 18, bannerY + 6, 11, Color{255, 214, 0, 255});

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
    DrawGameBoldText("1. TRAIN SERVICE & FARES", c1X + 16, cardY + 8, 14, WHITE);

    int y1 = cardY + 44;
    DrawText("Continuous Rail Loop", c1X + 16, y1, 13, Color{56, 189, 248, 255}); y1 += 20;
    DrawText("Your EMU bullet train circles endlessly along", c1X + 16, y1, 12, Color{226, 232, 240, 255}); y1 += 18;
    DrawText("the closed track circuit at full line speed.", c1X + 16, y1, 12, Color{226, 232, 240, 255}); y1 += 28;

    DrawText("Shape-Coded Commuters", c1X + 16, y1, 13, Color{255, 214, 0, 255}); y1 += 20;
    DrawText("Riders wait at platforms, each with a shape on", c1X + 16, y1, 12, Color{226, 232, 240, 255}); y1 += 18;
    DrawText("their head. They get off at the first station", c1X + 16, y1, 12, Color{226, 232, 240, 255}); y1 += 18;
    DrawText("that matches that shape. You don't click them -", c1X + 16, y1, 12, Color{226, 232, 240, 255}); y1 += 18;
    DrawText("they work on their own. Just keep trains moving!", c1X + 16, y1, 12, Color{226, 232, 240, 255}); y1 += 28;

    DrawText("Fares", c1X + 16, y1, 13, Color{34, 197, 94, 255}); y1 += 20;
    DrawText("Every rider pays $2.50 when they get off. Just", c1X + 16, y1, 12, Color{226, 232, 240, 255}); y1 += 18;
    DrawText("keep the loop running and the money rolls in!", c1X + 16, y1, 12, Color{226, 232, 240, 255}); y1 += 28;

    // Mini visual preview card footer
    DrawRectangleRounded(Rectangle{(float)c1X + 12, (float)cardY + cardH - 52, (float)cardW - 24, 40.0f}, 0.15f, 4, Color{15, 23, 42, 220});
    DrawText("SHAPES: [ ] Square  (O) Circle  (+) Cross", c1X + 22, cardY + cardH - 38, 11, Color{148, 163, 184, 255});

    // CARD 2: BUILD & EXPAND
    int c2X = startCardsX + cardW + gap;
    DrawRectangleRounded(Rectangle{(float)c2X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)c2X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{34, 197, 94, 255});
    DrawRectangleRounded(Rectangle{(float)c2X + 4, (float)cardY + 4, (float)cardW - 8, 28.0f}, 0.2f, 4, Color{21, 128, 61, 255});
    DrawGameBoldText("2. SIMPLE TRACK BUILDING", c2X + 16, cardY + 8, 14, WHITE);

    int y2 = cardY + 44;
    DrawText("Smart Connect Assist", c2X + 16, y2, 13, Color{34, 197, 94, 255}); y2 += 20;
    DrawText("Hover near open track ends to automatically snap", c2X + 16, y2, 12, Color{226, 232, 240, 255}); y2 += 18;
    DrawText("and continue your rail line smoothly.", c2X + 16, y2, 12, Color{226, 232, 240, 255}); y2 += 28;

    DrawText("Quick Hotkeys (1 - 8)", c2X + 16, y2, 13, Color{255, 214, 0, 255}); y2 += 20;
    DrawText("1=Straight, 2=Turn Left, 3=Turn Right, 4=Viaduct,", c2X + 16, y2, 12, Color{226, 232, 240, 255}); y2 += 18;
    DrawText("5=Station. Press [R] to Rotate or [X] to Demolish.", c2X + 16, y2, 12, Color{226, 232, 240, 255}); y2 += 28;

    DrawText("Station Level Ups", c2X + 16, y2, 13, Color{56, 189, 248, 255}); y2 += 20;
    DrawText("Stations level up as riders arrive (Local -> Hub ->", c2X + 16, y2, 12, Color{226, 232, 240, 255}); y2 += 18;
    DrawText("Grand Central) granting +25% to +50% fare bonuses!", c2X + 16, y2, 12, Color{226, 232, 240, 255}); y2 += 28;

    DrawRectangleRounded(Rectangle{(float)c2X + 12, (float)cardY + cardH - 52, (float)cardW - 24, 40.0f}, 0.15f, 4, Color{15, 23, 42, 220});
    DrawText("HOTKEYS: [1-8] Build  [R] Rotate  [X] Bulldoze", c2X + 22, cardY + cardH - 38, 11, Color{148, 163, 184, 255});

    // CARD 3: PREVENT OVERCROWDING
    int c3X = startCardsX + 2 * (cardW + gap);
    DrawRectangleRounded(Rectangle{(float)c3X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{23, 32, 51, 250});
    DrawRectangleRoundedLines(Rectangle{(float)c3X, (float)cardY, (float)cardW, (float)cardH}, 0.10f, 6, Color{239, 68, 68, 255});
    DrawRectangleRounded(Rectangle{(float)c3X + 4, (float)cardY + 4, (float)cardW - 8, 28.0f}, 0.2f, 4, Color{185, 28, 28, 255});
    DrawGameBoldText("3. AVOID OVERCROWDING", c3X + 16, cardY + 8, 14, WHITE);

    int y3 = cardY + 44;
    DrawText("Platform Queue", c3X + 16, y3, 13, Color{239, 68, 68, 255}); y3 += 20;
    DrawText("Commuters queue automatically. If too many pile", c3X + 16, y3, 12, Color{226, 232, 240, 255}); y3 += 18;
    DrawText("up, a red clock appears - pick them up before", c3X + 16, y3, 12, Color{226, 232, 240, 255}); y3 += 18;
    DrawText("it runs out!", c3X + 16, y3, 12, Color{226, 232, 240, 255}); y3 += 28;

    DrawText("Your Only Job", c3X + 16, y3, 13, Color{255, 214, 0, 255}); y3 += 20;
    DrawText("Keep the loop closed so the train never stops.", c3X + 16, y3, 12, Color{226, 232, 240, 255}); y3 += 18;
    DrawText("Add stations to serve riders faster. That's it!", c3X + 16, y3, 12, Color{226, 232, 240, 255}); y3 += 28;

    DrawText("Winning", c3X + 16, y3, 13, Color{34, 197, 94, 255}); y3 += 20;
    DrawText("Deliver 500 riders, then keep going for big grants!", c3X + 16, y3, 12, Color{226, 232, 240, 255}); y3 += 28;

    DrawRectangleRounded(Rectangle{(float)c3X + 12, (float)cardY + cardH - 52, (float)cardW - 24, 40.0f}, 0.15f, 4, Color{15, 23, 42, 220});
    DrawText("GOAL: Deliver 500  |  AVOID: Overcrowd timer", c3X + 22, cardY + cardH - 38, 11, Color{239, 68, 68, 255});

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

    DrawGameBoldText("START PLAYING", btnX + 90, btnY + 12, 19, WHITE);
    DrawText("(or press SPACE / ENTER)", btnX + 105, btnY + 34, 10, Color{255, 214, 0, 255});

    // 4b. Arcade HIGH-SCORE pulse (replayability loop)
    float bestFl = 0.5f + 0.5f * sinf(GetTime() * 3.0f);
    int bestY = btnY + btnH + 116;
    DrawGameBoldText(TextFormat("BEST RUN: %d RIDERS", best), (screenW - 300) / 2, bestY, 15, Color{255, 214, 0, (unsigned char)(255 * bestFl)});

    // 5. Bottom Navigation & Control Reference Bar
    int ctrlY = bestY + 26;
    int ctrlW = totalCardsW;
    int ctrlH = 34;
    int ctrlX = startCardsX;
    DrawRectangleRounded(Rectangle{(float)ctrlX, (float)ctrlY, (float)ctrlW, (float)ctrlH}, 0.3f, 4, Color{23, 32, 51, 230});
    DrawRectangleRoundedLines(Rectangle{(float)ctrlX, (float)ctrlY, (float)ctrlW, (float)ctrlH}, 0.3f, 4, Color{51, 65, 85, 200});

    DrawText("CONTROLS:  [WASD/Click] Move  [1-8] Tool  [R] Rotate  [E/Q] Height  [G] Objectives  [?] How to Play", ctrlX + 16, ctrlY + 10, 12, Color{203, 213, 225, 255});
    DrawGameBoldText("[BUY] Train Cars + Land + Extra EMU on toolbar", ctrlX + 16 + MeasureText("CONTROLS:  [WASD/Click] Move  [1-8] Tool  [R] Rotate  [E/Q] Height  [G] Objectives  [?] How to Play", 12) + 20, ctrlY + 10, 12, Color{74, 222, 128, 255});
    DrawText("[Drag R/M] Pan  [Edges] Scroll  [HOME] Recenter  [Wheel] Zoom", ctrlX + 3, ctrlY + 22, 10, Color{56, 189, 248, 255});
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
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = ToolbarMetrics::BAR_W;
    int barX = ToolbarMetrics::BarX(screenW);
    int barY = ToolbarMetrics::BarY(screenH);

    int itemBtnH = ToolbarMetrics::ITEM_H;
    int itemStartY = barY + ToolbarMetrics::ITEM_Y;

    if (activeTab == CAT_INFRA) {
        int infraBtnW = 150;
        int spacing = 20;
        int startX = barX + (barW - (4 * infraBtnW + 3 * spacing)) / 2;

        for (int i = 0; i < 4; ++i) {
            int bx = startX + i * (infraBtnW + spacing);
            if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx, (float)itemStartY, (float)infraBtnW, (float)itemBtnH})) {
                return i;
            }
        }
    } else {
        int itemBtnW = ToolbarMetrics::ITEM_W;
        int spacing = 10;
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

bool UserInterface::CheckToolbarAuxClick(Vector2 mousePos, int& outZDelta, bool& outRotate, int& outBuy) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = ToolbarMetrics::BAR_W;
    int barX = ToolbarMetrics::BarX(screenW);
    int barY = ToolbarMetrics::BarY(screenH);
    int auxX = ToolbarMetrics::AuxX(screenW);
    int buyY = barY + ToolbarMetrics::BUY_Y;

    outZDelta = 0;
    outRotate = false;
    outBuy = 0;

    // Buy Extra Train [main-bar strip button]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)barX + barW - 176, (float)buyY - 2, 160.0f, (float)ToolbarMetrics::BUY_H})) {
        outBuy = 3;
        return true;
    }
    // Buy Train Car [CAR +1]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 4, (float)buyY, 82.0f, (float)ToolbarMetrics::BUY_H})) {
        outBuy = 1;
        return true;
    }
    // Buy Land [LAND]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 90, (float)buyY, 50.0f, (float)ToolbarMetrics::BUY_H})) {
        outBuy = 2;
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
    bool& outToggleHelp
) const {
    (void)outToggleStats; (void)outToggleStaff; (void)outToggleRideCam;

    int screenW = GetScreenWidth();

    int btnX = screenW - 140;
    int helpX = btnX - 12 - 158;

    // How to Play
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)helpX, 12.0f, 158.0f, 28.0f})) {
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
    int screenW = GetScreenWidth();
    int cardX = screenW - cardW - 18;
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

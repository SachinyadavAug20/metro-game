#include "ui.hpp"
#include <cmath>

UserInterface::UserInterface() {}

void UserInterface::Update(Vector2 mousePos, bool mouseClicked) {
    (void)mousePos;
    (void)mouseClicked;
    pulseAnim += GetFrameTime() * 3.0f;
}

void UserInterface::DrawHUD(
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
) {
    int screenW = GetScreenWidth();

    // Top Bar Background with drop shadow
    DrawRectangle(0, 0, screenW, 52, Color{15, 23, 42, 245}); // Slate navy
    DrawLine(0, 52, screenW, 52, Color{51, 65, 85, 255});

    // 1. Logo / Title
    DrawText("COASTER GRID", 16, 12, 20, Color{255, 152, 0, 255});
    DrawText("2.5D TYCOON", 195, 17, 10, Color{148, 163, 184, 255});

    // 2. In-Game Calendar
    int calX = 265;
    DrawText(TextFormat("WEEK %d", week), calX, 10, 14, WHITE);
    float weekPct = std::max(0.0f, std::min(1.0f, weekTimer / 60.0f));
    DrawRectangle(calX, 28, 70, 7, Color{51, 65, 85, 255});
    DrawRectangle(calX, 28, (int)(70.0f * weekPct), 7, Color{14, 165, 233, 255});

    // 3. Bank Account
    int bankX = 350;
    DrawText("PARK FUNDS", bankX, 8, 9, Color{148, 163, 184, 255});
    DrawText(TextFormat("$%.0f", balance), bankX, 20, 18, Color{52, 211, 153, 255});

    // 4. Guests Delivered (Score)
    int scoreX = 455;
    DrawText("DELIVERED", scoreX, 8, 9, Color{148, 163, 184, 255});
    DrawText(TextFormat("%d", score), scoreX, 20, 18, Color{255, 214, 0, 255});

    // 5. Park Rating
    int rateX = 545;
    DrawText("SATISFACTION", rateX, 8, 9, Color{148, 163, 184, 255});
    Color rateColor = (parkRating > 60.0f) ? Color{74, 222, 128, 255} : (parkRating > 30.0f ? Color{251, 146, 60, 255} : Color{248, 113, 113, 255});
    DrawText(TextFormat("%.0f%%", parkRating), rateX, 20, 18, rateColor);

    // 6. Coaster Speedometer
    int spdX = 645;
    DrawText("COASTER SPEED", spdX, 8, 9, Color{148, 163, 184, 255});
    Color spdColor = (speedKmh > 75.0f) ? Color{239, 68, 68, 255} : (speedKmh > 40.0f ? Color{251, 191, 36, 255} : Color{134, 239, 172, 255});
    DrawText(TextFormat("%.1f km/h", speedKmh), spdX, 20, 18, spdColor);

    // 7. Circuit Status Badge
    int statX = 755;
    if (circuitClosed) {
        DrawRectangleRounded(Rectangle{(float)statX, 12.0f, 95.0f, 26.0f}, 0.3f, 4, Color{34, 197, 94, 220});
        DrawText("ACTIVE", statX + 22, 19, 11, WHITE);
    } else {
        DrawRectangleRounded(Rectangle{(float)statX, 12.0f, 95.0f, 26.0f}, 0.3f, 4, Color{239, 68, 68, 220});
        DrawText("OPEN LOOP", statX + 12, 19, 11, WHITE);
    }

    // 8. Action Toggle Buttons (Stats, Staff, Ride Cam, Help)
    int actX = 860;
    // Stats Button
    DrawRectangleRounded(Rectangle{(float)actX, 12.0f, 62.0f, 26.0f}, 0.25f, 4, statsOpen ? Color{234, 88, 12, 255} : Color{30, 41, 59, 220});
    DrawRectangleRoundedLines(Rectangle{(float)actX, 12.0f, 62.0f, 26.0f}, 0.25f, 4, Color{71, 85, 105, 200});
    DrawText("STATS", actX + 13, 19, 11, statsOpen ? WHITE : Color{203, 213, 225, 255});

    // Staff Button
    DrawRectangleRounded(Rectangle{(float)actX + 66, 12.0f, 62.0f, 26.0f}, 0.25f, 4, staffOpen ? Color{234, 88, 12, 255} : Color{30, 41, 59, 220});
    DrawRectangleRoundedLines(Rectangle{(float)actX + 66, 12.0f, 62.0f, 26.0f}, 0.25f, 4, Color{71, 85, 105, 200});
    DrawText("STAFF", actX + 78, 19, 11, staffOpen ? WHITE : Color{203, 213, 225, 255});

    // Ride Cam Button
    DrawRectangleRounded(Rectangle{(float)actX + 132, 12.0f, 74.0f, 26.0f}, 0.25f, 4, rideCamActive ? Color{234, 88, 12, 255} : Color{30, 41, 59, 220});
    DrawRectangleRoundedLines(Rectangle{(float)actX + 132, 12.0f, 74.0f, 26.0f}, 0.25f, 4, Color{71, 85, 105, 200});
    DrawText("CAM [F]", actX + 140, 19, 11, rideCamActive ? WHITE : Color{203, 213, 225, 255});

    // Help Button
    DrawRectangleRounded(Rectangle{(float)actX + 210, 12.0f, 48.0f, 26.0f}, 0.25f, 4, helpOpen ? Color{234, 88, 12, 255} : Color{30, 41, 59, 220});
    DrawRectangleRoundedLines(Rectangle{(float)actX + 210, 12.0f, 48.0f, 26.0f}, 0.25f, 4, Color{71, 85, 105, 200});
    DrawText("HELP", actX + 218, 19, 11, helpOpen ? WHITE : Color{203, 213, 225, 255});

    // 9. Time Controls & Sound Mute
    int btnX = screenW - 145;
    DrawRectangleRounded(Rectangle{(float)btnX, 12.0f, 26.0f, 26.0f}, 0.2f, 4, (gameSpeed == 0) ? Color{234, 88, 12, 255} : Color{30, 41, 59, 200});
    DrawText("||", btnX + 9, 18, 13, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 30, 12.0f, 26.0f, 26.0f}, 0.2f, 4, (gameSpeed == 1) ? Color{234, 88, 12, 255} : Color{30, 41, 59, 200});
    DrawText(">", btnX + 39, 18, 13, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 60, 12.0f, 26.0f, 26.0f}, 0.2f, 4, (gameSpeed == 2) ? Color{234, 88, 12, 255} : Color{30, 41, 59, 200});
    DrawText(">>", btnX + 66, 18, 13, WHITE);

    DrawRectangleRounded(Rectangle{(float)btnX + 90, 12.0f, 44.0f, 26.0f}, 0.2f, 4, muted ? Color{239, 68, 68, 220} : Color{30, 41, 59, 200});
    DrawText(muted ? "MUT" : "SND", btnX + 98, 19, 10, WHITE);
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

    int barW = 760;
    int barH = 72;
    int barX = (screenW - barW) / 2;
    int barY = screenH - barH - 12;

    // 1. Category Tabs above toolbar
    const char* tabNames[] = {"1. TRACKS", "2. PATHS", "3. SCENERY"};
    int tabW = 100;
    int tabH = 24;
    int tabStartY = barY - tabH + 2;

    for (int t = 0; t < 3; ++t) {
        int tx = barX + 16 + t * (tabW + 6);
        bool isCurrentTab = ((int)activeTab == t);
        Color tabBg = isCurrentTab ? Color{15, 23, 42, 255} : Color{30, 41, 59, 200};
        Color tabText = isCurrentTab ? Color{255, 179, 0, 255} : Color{148, 163, 184, 255};

        DrawRectangleRounded(Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4}, 0.2f, 4, tabBg);
        DrawRectangleRoundedLines(Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4}, 0.2f, 4, isCurrentTab ? Color{255, 179, 0, 255} : Color{51, 65, 85, 255});
        DrawText(tabNames[t], tx + 14, tabStartY + 5, 10, tabText);
    }

    // 2. Toolbar Body Card
    DrawRectangleRounded(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.2f, 6, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)barX, (float)barY, (float)barW, (float)barH}, 0.2f, 6, Color{51, 65, 85, 255});

    // 3. Render Items according to Active Tab
    int itemBtnW = 66;
    int itemBtnH = 52;
    int itemStartY = barY + 10;
    int startX = barX + 12;

    if (activeTab == CAT_TRACK) {
        struct TrackBtn { const char* name; const char* key; TrackType type; };
        TrackBtn buttons[] = {
            {"Straight", "1", TRACK_STRAIGHT},
            {"Turn L", "2", TRACK_CURVE_LEFT},
            {"Turn R", "3", TRACK_CURVE_RIGHT},
            {"Lift +1Z", "4", TRACK_LIFT_HILL},
            {"Drop -1Z", "5", TRACK_DROP},
            {"Loop", "6", TRACK_LOOP},
            {"Brakes", "7", TRACK_BRAKES},
            {"Station", "8", TRACK_STATION},
            {"Bulldoze", "X", TRACK_NONE}
        };

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + 5);
            bool isSelected = (i == 8) ? isBulldozing : (!isBulldozing && currentTrack == buttons[i].type);

            Color btnBg = isSelected ? Color{234, 88, 12, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{255, 237, 213, 255} : Color{71, 85, 105, 200});

            DrawText(buttons[i].key, bx + 5, itemStartY + 4, 10, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 6, itemStartY + 24, 9, textC);
        }
    } else if (activeTab == CAT_INFRA) {
        struct InfraBtn { const char* name; const char* key; GroundType ground; bool isBull; };
        InfraBtn buttons[] = {
            {"Footpath", "1", GROUND_PATH, false},
            {"Queue Line", "2", GROUND_QUEUE, false},
            {"Bulldoze", "X", GROUND_GRASS, true}
        };

        for (int i = 0; i < 3; ++i) {
            int bx = startX + i * (itemBtnW + 15);
            bool isSelected = buttons[i].isBull ? isBulldozing : (!isBulldozing && currentGround == buttons[i].ground);

            Color btnBg = isSelected ? Color{234, 88, 12, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW + 10, (float)itemBtnH}, 0.2f, 4, btnBg);
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW + 10, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{255, 237, 213, 255} : Color{71, 85, 105, 200});

            DrawText(buttons[i].key, bx + 6, itemStartY + 4, 10, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 8, itemStartY + 24, 10, textC);
        }
    } else if (activeTab == CAT_SCENERY) {
        struct SceneryBtn { const char* name; const char* key; SceneryType scn; int cost; };
        SceneryBtn buttons[] = {
            {"Pine Tree", "1", SCENERY_PINE_TREE, 30},
            {"Oak Tree", "2", SCENERY_OAK_TREE, 45},
            {"Bench", "3", SCENERY_BENCH, 20},
            {"Fountain", "4", SCENERY_FOUNTAIN, 180},
            {"Flowers", "5", SCENERY_FLOWER_BED, 15},
            {"Soda Stall", "6", SCENERY_DRINK_STALL, 150},
            {"Balloons", "7", SCENERY_BALLOON_STALL, 120},
            {"Lamp Post", "8", SCENERY_LAMP_POST, 25},
            {"Bulldoze", "X", SCENERY_NONE, 0}
        };

        for (int i = 0; i < 9; ++i) {
            int bx = startX + i * (itemBtnW + 5);
            bool isSelected = (i == 8) ? isBulldozing : (!isBulldozing && currentScenery == buttons[i].scn);

            Color btnBg = isSelected ? Color{234, 88, 12, 255} : Color{30, 41, 59, 230};
            Color textC = isSelected ? WHITE : Color{203, 213, 225, 255};

            DrawRectangleRounded(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, btnBg);
            DrawRectangleRoundedLines(Rectangle{(float)bx, (float)itemStartY, (float)itemBtnW, (float)itemBtnH}, 0.2f, 4, isSelected ? Color{255, 237, 213, 255} : Color{71, 85, 105, 200});

            DrawText(buttons[i].key, bx + 5, itemStartY + 4, 10, Color{255, 214, 0, 255});
            DrawText(buttons[i].name, bx + 4, itemStartY + 20, 8, textC);
            if (buttons[i].cost > 0) {
                DrawText(TextFormat("$%d", buttons[i].cost), bx + 5, itemStartY + 35, 9, Color{52, 211, 153, 255});
            }
        }
    }

    // 4. Auxiliary Pills on Right (Elevation & Direction)
    int auxX = barX + barW + 10;
    if (auxX + 130 < screenW) {
        // Height Pill
        DrawRectangleRounded(Rectangle{(float)auxX, (float)barY, 130.0f, 34.0f}, 0.25f, 4, Color{15, 23, 42, 245});
        DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)barY, 130.0f, 34.0f}, 0.25f, 4, Color{51, 65, 85, 255});
        DrawText("HEIGHT (E/Q)", auxX + 8, barY + 4, 9, Color{148, 163, 184, 255});
        DrawText(TextFormat("Z = %d", currentZ), auxX + 10, barY + 16, 14, Color{255, 214, 0, 255});
        DrawRectangle((float)auxX + 85, (float)barY + 6, 18, 20, Color{30, 41, 59, 255});
        DrawText("-", auxX + 91, barY + 9, 14, WHITE);
        DrawRectangle((float)auxX + 106, (float)barY + 6, 18, 20, Color{30, 41, 59, 255});
        DrawText("+", auxX + 111, barY + 9, 14, WHITE);

        // Direction Pill
        const char* dirLabels[] = {"NORTH", "EAST", "SOUTH", "WEST"};
        DrawRectangleRounded(Rectangle{(float)auxX, (float)barY + 38, 130.0f, 34.0f}, 0.25f, 4, Color{15, 23, 42, 245});
        DrawRectangleRoundedLines(Rectangle{(float)auxX, (float)barY + 38, 130.0f, 34.0f}, 0.25f, 4, Color{51, 65, 85, 255});
        DrawText("ROTATE [R]", auxX + 8, barY + 42, 9, Color{148, 163, 184, 255});
        DrawText(dirLabels[currentDir], auxX + 10, barY + 54, 13, Color{56, 189, 248, 255});
        DrawRectangle((float)auxX + 96, (float)barY + 44, 26, 20, Color{30, 41, 59, 255});
        DrawText("ROT", auxX + 100, barY + 49, 10, WHITE);
    }
}

void UserInterface::DrawCoasterStats(const CoasterStats& stats) {
    int screenW = GetScreenWidth();
    int winW = 310;
    int winH = 415;
    int winX = screenW - winW - 16;
    int winY = 62;

    // Window shadow & body
    DrawRectangleRounded(Rectangle{(float)winX, (float)winY, (float)winW, (float)winH}, 0.1f, 6, Color{15, 23, 42, 250});
    DrawRectangleRoundedLines(Rectangle{(float)winX, (float)winY, (float)winW, (float)winH}, 0.1f, 6, stats.themeColor);

    // Window Header
    DrawRectangleRounded(Rectangle{(float)winX + 2, (float)winY + 2, (float)winW - 4, 32.0f}, 0.2f, 4, stats.themeColor);
    DrawText(TextFormat("COASTER: %s", stats.coasterName.c_str()), winX + 12, winY + 10, 12, WHITE);
    DrawText("[X]", winX + winW - 28, winY + 9, 14, WHITE);

    // Content rows
    int rowY = winY + 45;
    int rowH = 26;

    // Excitement
    DrawText("Excitement Rating:", winX + 15, rowY, 11, Color{203, 213, 225, 255});
    DrawText(TextFormat("%.1f / 10 (High)", stats.excitementRating), winX + 145, rowY, 11, Color{255, 214, 0, 255});
    rowY += rowH;

    // Intensity
    DrawText("Intensity Rating:", winX + 15, rowY, 11, Color{203, 213, 225, 255});
    DrawText(TextFormat("%.1f / 10 (Med)", stats.intensityRating), winX + 145, rowY, 11, Color{251, 146, 60, 255});
    rowY += rowH;

    // Nausea
    DrawText("Nausea Rating:", winX + 15, rowY, 11, Color{203, 213, 225, 255});
    DrawText(TextFormat("%.1f / 10 (Low)", stats.nauseaRating), winX + 145, rowY, 11, Color{74, 222, 128, 255});
    rowY += rowH + 6;

    DrawLine(winX + 15, rowY, winX + winW - 15, rowY, Color{51, 65, 85, 255});
    rowY += 10;

    // Speeds & Length
    DrawText("Maximum Speed:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.1f km/h", stats.maxSpeedKmh), winX + 180, rowY, 11, WHITE);
    rowY += rowH;

    DrawText("Ride Length:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.0f meters", stats.trackLengthM), winX + 180, rowY, 11, WHITE);
    rowY += rowH;

    DrawText("Highest Drop:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%.1f meters", stats.maxDropM), winX + 180, rowY, 11, WHITE);
    rowY += rowH;

    DrawText("Inversions:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%d loops", stats.inversions), winX + 180, rowY, 11, WHITE);
    rowY += rowH;

    DrawText("Total Riders:", winX + 15, rowY, 11, Color{148, 163, 184, 255});
    DrawText(TextFormat("%d guests", stats.totalRiders), winX + 180, rowY, 11, Color{255, 214, 0, 255});
    rowY += rowH + 6;

    DrawLine(winX + 15, rowY, winX + winW - 15, rowY, Color{51, 65, 85, 255});
    rowY += 10;

    // Ticket Price Setting
    DrawText("Ride Ticket Price:", winX + 15, rowY + 3, 11, Color{203, 213, 225, 255});
    DrawText(TextFormat("$%.2f", stats.ticketPrice), winX + 140, rowY + 3, 13, Color{52, 211, 153, 255});

    DrawRectangleRounded(Rectangle{(float)winX + 215, (float)rowY, 26.0f, 22.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText("-", winX + 224, rowY + 4, 14, WHITE);

    DrawRectangleRounded(Rectangle{(float)winX + 248, (float)rowY, 26.0f, 22.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText("+", winX + 256, rowY + 4, 14, WHITE);
    rowY += rowH + 4;

    // Color Palette Theme
    DrawLine(winX + 15, rowY, winX + winW - 15, rowY, Color{51, 65, 85, 255});
    rowY += 10;
    DrawText("Theme Color:", winX + 15, rowY + 3, 11, Color{203, 213, 225, 255});

    Color themes[] = {
        Color{229, 57, 53, 255}, // Red
        Color{37, 99, 235, 255}, // Blue
        Color{16, 185, 129, 255}, // Green
        Color{147, 51, 234, 255}, // Purple
        Color{245, 158, 11, 255}  // Amber
    };

    for (int c = 0; c < 5; ++c) {
        int cx = winX + 140 + c * 26;
        DrawRectangleRounded(Rectangle{(float)cx, (float)rowY, 22.0f, 20.0f}, 0.3f, 4, themes[c]);
        DrawRectangleRoundedLines(Rectangle{(float)cx, (float)rowY, 22.0f, 20.0f}, 0.3f, 4, WHITE);
    }
}

void UserInterface::DrawStaffWindow(const std::vector<StaffMember>& staff, float cleanliness, float balance) {
    int screenW = GetScreenWidth();
    int winW = 330;
    int winH = 340;
    int winX = screenW - winW - 16;
    int winY = 62;

    // Window shadow & body
    DrawRectangleRounded(Rectangle{(float)winX, (float)winY, (float)winW, (float)winH}, 0.1f, 6, Color{15, 23, 42, 250});
    DrawRectangleRoundedLines(Rectangle{(float)winX, (float)winY, (float)winW, (float)winH}, 0.1f, 6, Color{59, 130, 246, 255});

    // Window Header
    DrawRectangleRounded(Rectangle{(float)winX + 2, (float)winY + 2, (float)winW - 4, 32.0f}, 0.2f, 4, Color{30, 58, 138, 255});
    DrawText("PARK STAFF & MAINTENANCE", winX + 12, winY + 10, 12, Color{255, 214, 0, 255});
    DrawText("[X]", winX + winW - 28, winY + 9, 14, WHITE);

    // Cleanliness bar
    DrawText("PARK CLEANLINESS", winX + 16, winY + 44, 10, Color{148, 163, 184, 255});
    Color cleanCol = (cleanliness > 75.0f) ? Color{74, 222, 128, 255} : (cleanliness > 40.0f ? Color{251, 146, 60, 255} : Color{239, 68, 68, 255});
    DrawText(TextFormat("%.0f%%", cleanliness), winX + winW - 55, winY + 42, 12, cleanCol);
    DrawRectangle(winX + 16, winY + 58, winW - 32, 8, Color{30, 41, 59, 255});
    DrawRectangle(winX + 16, winY + 58, (int)((winW - 32) * (cleanliness / 100.0f)), 8, cleanCol);

    // Hiring Section
    DrawText("HIRE EMPLOYEES", winX + 16, winY + 76, 10, Color{148, 163, 184, 255});

    // Hire Handyman Button
    bool canAffordH = (balance >= 80.0f);
    float btnW = (float)(winW - 40) / 2.0f;
    DrawRectangleRounded(Rectangle{(float)winX + 16, (float)winY + 92, btnW, 36.0f}, 0.2f, 4, canAffordH ? Color{29, 78, 216, 220} : Color{51, 65, 85, 180});
    DrawRectangleRoundedLines(Rectangle{(float)winX + 16, (float)winY + 92, btnW, 36.0f}, 0.2f, 4, Color{96, 165, 250, 255});
    DrawText("+ Handyman", winX + 22, winY + 98, 10, WHITE);
    DrawText("$80 (Sweeps Vomit)", winX + 22, winY + 112, 8, Color{191, 219, 254, 255});

    // Hire Mechanic Button
    bool canAffordM = (balance >= 100.0f);
    DrawRectangleRounded(Rectangle{(float)winX + 24 + btnW, (float)winY + 92, btnW, 36.0f}, 0.2f, 4, canAffordM ? Color{217, 119, 6, 220} : Color{51, 65, 85, 180});
    DrawRectangleRoundedLines(Rectangle{(float)winX + 24 + btnW, (float)winY + 92, btnW, 36.0f}, 0.2f, 4, Color{251, 191, 36, 255});
    DrawText("+ Mechanic", winX + 30 + (int)btnW, winY + 98, 10, WHITE);
    DrawText("$100 (Checks Ride)", winX + 30 + (int)btnW, winY + 112, 8, Color{254, 240, 138, 255});

    // Active Staff Roster
    DrawText(TextFormat("ACTIVE ROSTER (%d)", (int)staff.size()), winX + 16, winY + 138, 10, Color{148, 163, 184, 255});
    int listY = winY + 154;
    int drawn = 0;
    for (const auto& s : staff) {
        if (drawn >= 5) break;
        DrawRectangle(winX + 16, listY, winW - 32, 28, (drawn % 2 == 0) ? Color{30, 41, 59, 180} : Color{20, 30, 45, 180});
        Color badgeC = (s.type == STAFF_HANDYMAN) ? Color{59, 130, 246, 255} : Color{245, 158, 11, 255};
        DrawCircle(winX + 26, listY + 14, 5, badgeC);
        DrawText(s.name.c_str(), winX + 38, listY + 8, 10, WHITE);
        const char* status = s.isWorking ? (s.type == STAFF_HANDYMAN ? "Sweeping" : "Inspecting") : "Patrolling";
        DrawText(status, winX + winW - 85, listY + 8, 9, Color{148, 163, 184, 255});
        listY += 32;
        drawn++;
    }
}

void UserInterface::DrawPeepInspector(const Peep* peep) {
    if (!peep) return;

    int cardW = 280;
    int cardH = 135;
    int cardX = 18;
    int cardY = 64;

    // Card background
    DrawRectangleRounded(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.15f, 6, Color{15, 23, 42, 250});
    DrawRectangleRoundedLines(Rectangle{(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 0.15f, 6, Color{71, 85, 105, 255});

    // Peep Head Portrait
    DrawCircle(cardX + 26, cardY + 28, 14, peep->shirtColor);
    DrawCircle(cardX + 26, cardY + 26, 9, Color{255, 224, 178, 255});

    // Peep Name & Archetype Tag
    DrawText(peep->name.c_str(), cardX + 50, cardY + 14, 14, WHITE);
    const char* typeTag = (peep->type == PEEP_THRILL_SEEKER) ? "[THRILL] Thrill Seeker" : (peep->type == PEEP_CASUAL ? "[CASUAL] Casual Guest" : "[QUEASY] Queasy Guest");
    DrawText(typeTag, cardX + 50, cardY + 30, 10, Color{148, 163, 184, 255});
    DrawText("[x]", cardX + cardW - 22, cardY + 12, 12, Color{148, 163, 184, 255});

    // Happiness Bar
    DrawText("Happiness", cardX + 15, cardY + 54, 10, Color{148, 163, 184, 255});
    DrawRectangle(cardX + 80, cardY + 56, 120, 8, Color{30, 41, 59, 255});
    DrawRectangle(cardX + 80, cardY + 56, (int)(120.0f * (peep->happiness / 100.0f)), 8, Color{74, 222, 128, 255});
    DrawText(TextFormat("%.0f%%", peep->happiness), cardX + 210, cardY + 54, 10, WHITE);

    // Nausea Bar
    DrawText("Nausea", cardX + 15, cardY + 70, 10, Color{148, 163, 184, 255});
    DrawRectangle(cardX + 80, cardY + 72, 120, 8, Color{30, 41, 59, 255});
    DrawRectangle(cardX + 80, cardY + 72, (int)(120.0f * (peep->nausea / 100.0f)), 8, Color{248, 113, 113, 255});
    DrawText(TextFormat("%.0f%%", peep->nausea), cardX + 210, cardY + 70, 10, WHITE);

    // Thoughts speech bubble
    DrawRectangleRounded(Rectangle{(float)cardX + 12, (float)cardY + 88, (float)cardW - 24, 34.0f}, 0.2f, 4, Color{30, 41, 59, 255});
    DrawText(TextFormat("\"%s\"", peep->thought.c_str()), cardX + 20, cardY + 98, 10, Color{255, 214, 0, 255});
}

void UserInterface::DrawToast(const ToastMessage& toast) {
    if (toast.timer <= 0.0f) return;

    int screenW = GetScreenWidth();
    int toastW = 420;
    int toastH = 34;
    int toastX = (screenW - toastW) / 2;
    int toastY = 60;

    DrawRectangleRounded(Rectangle{(float)toastX, (float)toastY, (float)toastW, (float)toastH}, 0.3f, 4, Color{15, 23, 42, 245});
    DrawRectangleRoundedLines(Rectangle{(float)toastX, (float)toastY, (float)toastW, (float)toastH}, 0.3f, 4, toast.color);

    DrawText(toast.text.c_str(), toastX + 16, toastY + 10, 12, WHITE);
}

void UserInterface::DrawHelpOverlay() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 210});

    int boxW = 620;
    int boxH = 460;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.1f, 8, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.1f, 8, Color{234, 88, 12, 255});

    DrawText("HOW TO PLAY & CONTROLS", bx + 180, by + 24, 20, Color{255, 179, 0, 255});
    DrawText("Close [X]", bx + boxW - 80, by + 24, 12, Color{148, 163, 184, 255});

    int col1 = bx + 30;
    int y = by + 65;

    DrawText("[NAVIGATION & CAMERA]", col1, y, 13, Color{56, 189, 248, 255});
    y += 20;
    DrawText("* W / A / S / D or Right Click Drag: Pan Camera", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* Mouse Wheel: Zoom in / Zoom out", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* F: Toggle Ride Cam (Locks camera to coaster train)", col1, y, 11, Color{203, 213, 225, 255});
    y += 28;

    DrawText("[TRACK CONSTRUCTION & TOOLS]", col1, y, 13, Color{56, 189, 248, 255});
    y += 20;
    DrawText("* 1-8: Select Track Pieces (Straight, Curves, Lift, Drop, Loop, Brakes, Station)", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* R: Rotate piece orientation (North, East, South, West)", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* E / Q: Raise / Lower placement elevation level (Z = 0..5)", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* P: Staff Management | T: Coaster Statistics & Ticket Pricing", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* X: Bulldozer tool (Demolishes track, paths, or scenery)", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* Left Click: Place selected piece or Inspect any Guest", col1, y, 11, Color{203, 213, 225, 255});
    y += 28;

    DrawText("[MINI METRO TRIAGE & PARK MANAGEMENT]", col1, y, 13, Color{251, 146, 60, 255});
    y += 20;
    DrawText("* Queues >= 10 guests trigger a ticking Overcrowding Clock! Keep trains dispatching.", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* Sick guests vomit on paths: hire Handymen (P) with brooms to keep park rating high!", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* Beware of Derailments (>75 km/h on sharp curves)! Press 'C' to recover train.", col1, y, 11, Color{203, 213, 225, 255});
    y += 18;
    DrawText("* Fountains, Flowerbeds, Benches & Balloons boost happiness and revenue!", col1, y, 11, Color{203, 213, 225, 255});

    DrawRectangleRounded(Rectangle{(float)bx + 210, (float)by + boxH - 45, 200.0f, 32.0f}, 0.3f, 4, Color{234, 88, 12, 255});
    DrawText("GOT IT! LET'S BUILD", bx + 242, by + boxH - 35, 12, WHITE);
}

void UserInterface::DrawWeeklyModal(const std::vector<UpgradeChoice>& choices, int hoveredChoice) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 190});

    int modalW = 680;
    int modalH = 340;
    int mx = (screenW - modalW) / 2;
    int my = (screenH - modalH) / 2;

    DrawRectangleRounded(Rectangle{(float)mx, (float)my, (float)modalW, (float)modalH}, 0.1f, 8, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)mx, (float)my, (float)modalW, (float)modalH}, 0.1f, 8, Color{234, 88, 12, 255});

    DrawText("WEEKLY PARK GRANT UNLOCKED!", mx + 160, my + 24, 22, Color{255, 179, 0, 255});
    DrawText("Select 1 resource grant to support your expanding theme park:", mx + 140, my + 54, 12, Color{148, 163, 184, 255});

    int cardW = 190;
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
        DrawText(choices[i].perkTag.c_str(), cx + 20, cardY + 18, 10, WHITE);

        DrawText(choices[i].title.c_str(), cx + 15, cardY + 50, 14, WHITE);
        DrawText(choices[i].description.c_str(), cx + 15, cardY + 85, 11, Color{203, 213, 225, 255});

        DrawRectangleRounded(Rectangle{(float)cx + 20, (float)cardY + cardH - 38, (float)cardW - 40, 26.0f}, 0.3f, 4, hovered ? choices[i].accentColor : Color{51, 65, 85, 255});
        DrawText("SELECT", cx + 66, cardY + cardH - 31, 11, WHITE);
    }
}

void UserInterface::DrawGameOver(int finalScore) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 200});

    int boxW = 460;
    int boxH = 260;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{220, 38, 38, 255});

    DrawText("PARK CONDEMNED!", bx + 115, by + 30, 24, Color{239, 68, 68, 255});
    DrawText("Queues overflowed and guest satisfaction collapsed.", bx + 55, by + 70, 12, Color{203, 213, 225, 255});
    DrawText(TextFormat("Final Delivered Guests: %d", finalScore), bx + 120, by + 115, 18, Color{255, 214, 0, 255});

    DrawRectangleRounded(Rectangle{(float)bx + 130, (float)by + 175, 200.0f, 45.0f}, 0.3f, 4, Color{234, 88, 12, 255});
    DrawText("RESTART PARK", bx + 165, by + 188, 16, WHITE);
}

void UserInterface::DrawVictory(int finalScore) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 200});

    int boxW = 500;
    int boxH = 280;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    DrawRectangleRounded(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{(float)bx, (float)by, (float)boxW, (float)boxH}, 0.15f, 6, Color{34, 197, 94, 255});

    DrawText("PARK TRIUMPH! 500 GUESTS!", bx + 70, by + 30, 24, Color{34, 197, 94, 255});
    DrawText("Your roller coaster network handled the rush with perfection!", bx + 65, by + 70, 12, Color{203, 213, 225, 255});
    DrawText(TextFormat("Total Transported: %d", finalScore), bx + 155, by + 120, 18, Color{255, 214, 0, 255});

    DrawRectangleRounded(Rectangle{(float)bx + 140, (float)by + 185, 220.0f, 45.0f}, 0.3f, 4, Color{16, 185, 129, 255});
    DrawText("KEEP PLAYING", bx + 185, by + 198, 16, WHITE);
}

void UserInterface::DrawTitleScreen() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, screenH, Color{15, 23, 42, 255});

    for (int i = 0; i < screenW; i += 60) {
        DrawLine(i, 0, i + 200, screenH, Color{30, 41, 59, 80});
    }

    DrawText("COASTER GRID", screenW / 2 - 210, screenH / 2 - 120, 52, Color{255, 152, 0, 255});
    DrawText("2.5D Isometric Theme Park Dispatcher", screenW / 2 - 190, screenH / 2 - 50, 18, Color{148, 163, 184, 255});
    DrawText("RollerCoaster Tycoon Physics meets Mini Metro Logistics", screenW / 2 - 215, screenH / 2 - 20, 14, Color{203, 213, 225, 255});

    DrawRectangleRounded(Rectangle{(float)screenW / 2 - 120, (float)screenH / 2 + 50, 240.0f, 54.0f}, 0.3f, 6, Color{234, 88, 12, 255});
    DrawText("START GAME", screenW / 2 - 68, screenH / 2 + 67, 20, WHITE);

    DrawText("Controls: WASD/Mouse Drag to Pan | Scroll to Zoom | Keys 1-8 to Select Tools | E/Q to Adjust Height", screenW / 2 - 340, screenH - 40, 12, Color{100, 116, 139, 255});
}

int UserInterface::CheckToolbarTabClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = 760;
    int barH = 72;
    int barX = (screenW - barW) / 2;
    int barY = screenH - barH - 12;

    int tabW = 100;
    int tabH = 24;
    int tabStartY = barY - tabH + 2;

    for (int t = 0; t < 3; ++t) {
        int tx = barX + 16 + t * (tabW + 6);
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)tx, (float)tabStartY, (float)tabW, (float)tabH + 4})) {
            return t;
        }
    }
    return -1;
}

int UserInterface::CheckToolbarItemClick(Vector2 mousePos, ToolCategory activeTab) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = 760;
    int barH = 72;
    int barX = (screenW - barW) / 2;
    int barY = screenH - barH - 12;

    int itemBtnW = 66;
    int itemBtnH = 52;
    int itemStartY = barY + 10;
    int startX = barX + 12;

    int count = (activeTab == CAT_INFRA) ? 3 : 9;
    int step = (activeTab == CAT_INFRA) ? (itemBtnW + 15) : (itemBtnW + 5);
    int width = (activeTab == CAT_INFRA) ? (itemBtnW + 10) : itemBtnW;

    for (int i = 0; i < count; ++i) {
        int bx = startX + i * step;
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx, (float)itemStartY, (float)width, (float)itemBtnH})) {
            return i;
        }
    }
    return -1;
}

bool UserInterface::CheckToolbarAuxClick(Vector2 mousePos, int& outZDelta, bool& outRotate) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int barW = 760;
    int barH = 72;
    int barX = (screenW - barW) / 2;
    int barY = screenH - barH - 12;
    int auxX = barX + barW + 10;

    // Height down [-]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 85, (float)barY + 6, 18.0f, 20.0f})) {
        outZDelta = -1;
        return true;
    }
    // Height up [+]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 106, (float)barY + 6, 18.0f, 20.0f})) {
        outZDelta = 1;
        return true;
    }
    // Rotate [↻]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)auxX + 96, (float)barY + 44, 26.0f, 20.0f})) {
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

    // Stats Button
    int actX = 860;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)actX, 12.0f, 62.0f, 26.0f})) {
        outToggleStats = true;
        return true;
    }
    // Staff Button
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)actX + 66, 12.0f, 62.0f, 26.0f})) {
        outToggleStaff = true;
        return true;
    }
    // Ride Cam Button
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)actX + 132, 12.0f, 74.0f, 26.0f})) {
        outToggleRideCam = true;
        return true;
    }
    // Help Button
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)actX + 210, 12.0f, 48.0f, 26.0f})) {
        outToggleHelp = true;
        return true;
    }

    // Time controls
    int btnX = screenW - 145;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX, 12.0f, 26.0f, 26.0f})) {
        outNewSpeed = 0;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 30, 12.0f, 26.0f, 26.0f})) {
        outNewSpeed = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 60, 12.0f, 26.0f, 26.0f})) {
        outNewSpeed = 2;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)btnX + 90, 12.0f, 44.0f, 26.0f})) {
        outToggleMute = true;
        return true;
    }

    return false;
}

bool UserInterface::CheckStatsWindowClick(Vector2 mousePos, float& outTicketPriceDelta, int& outColorChoice, bool& outClose) const {
    int screenW = GetScreenWidth();
    int winW = 310;
    int winX = screenW - winW - 16;
    int winY = 62;

    outColorChoice = -1;

    // Close button [X]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + winW - 32, (float)winY + 4, 28.0f, 28.0f})) {
        outClose = true;
        return true;
    }

    // Ticket price [-]
    int rowY = winY + 45 + 26 * 3 + 16 + 26 * 5 + 16;
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 215, (float)rowY, 26.0f, 22.0f})) {
        outTicketPriceDelta = -0.50f;
        return true;
    }
    // Ticket price [+]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 248, (float)rowY, 26.0f, 22.0f})) {
        outTicketPriceDelta = 0.50f;
        return true;
    }

    // Theme color buttons
    int palY = rowY + 26 + 14;
    for (int c = 0; c < 5; ++c) {
        int cx = winX + 140 + c * 26;
        if (CheckCollisionPointRec(mousePos, Rectangle{(float)cx, (float)palY, 22.0f, 20.0f})) {
            outColorChoice = c;
            return true;
        }
    }

    return false;
}

bool UserInterface::CheckStaffWindowClick(Vector2 mousePos, bool& outHireHandyman, bool& outHireMechanic, bool& outClose) const {
    int screenW = GetScreenWidth();
    int winW = 330;
    int winX = screenW - winW - 16;
    int winY = 62;

    // Close button [X]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + winW - 28, (float)winY + 6, 22.0f, 22.0f})) {
        outClose = true;
        return true;
    }

    float btnW = (float)(winW - 40) / 2.0f;
    // Hire Handyman
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 16, (float)winY + 92, btnW, 36.0f})) {
        outHireHandyman = true;
        return true;
    }

    // Hire Mechanic
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)winX + 24 + btnW, (float)winY + 92, btnW, 36.0f})) {
        outHireMechanic = true;
        return true;
    }

    return false;
}

bool UserInterface::CheckPeepInspectorCloseClick(Vector2 mousePos) const {
    int cardW = 280;
    int cardX = 18;
    int cardY = 64;

    return CheckCollisionPointRec(mousePos, Rectangle{(float)cardX + cardW - 28, (float)cardY + 8, 24.0f, 24.0f});
}

bool UserInterface::CheckHelpOverlayClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int boxW = 620;
    int boxH = 460;
    int bx = (screenW - boxW) / 2;
    int by = (screenH - boxH) / 2;

    // Close [X]
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx + boxW - 85, (float)by + 20, 70.0f, 25.0f})) {
        return true;
    }
    // Got it button
    if (CheckCollisionPointRec(mousePos, Rectangle{(float)bx + 210, (float)by + boxH - 45, 200.0f, 32.0f})) {
        return true;
    }
    return false;
}

int UserInterface::CheckUpgradeModalClick(Vector2 mousePos) const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int modalW = 680;
    int modalH = 340;
    int mx = (screenW - modalW) / 2;
    int my = (screenH - modalH) / 2;

    int cardW = 190;
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
    int bx = (screenW - 460) / 2;
    int by = (screenH - 260) / 2;

    Rectangle r = {(float)bx + 130, (float)by + 175, 200.0f, 45.0f};
    return CheckCollisionPointRec(mousePos, r);
}

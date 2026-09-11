#include "isometric.hpp"

namespace Iso {

Vector2 GridToScreen(float gx, float gy, float gz, Vector2 camOffset, float zoom) {
    float sx = (gx - gy) * (TILE_WIDTH / 2.0f) * zoom + camOffset.x;
    float sy = (gx + gy) * (TILE_HEIGHT / 2.0f) * zoom - (gz * HEIGHT_STEP * zoom) + camOffset.y;
    return {sx, sy};
}

Vector2 ScreenToGrid(Vector2 screenPos, Vector2 camOffset, float zoom, float gz) {
    float px = (screenPos.x - camOffset.x) / zoom;
    float py = (screenPos.y - camOffset.y) / zoom + (gz * HEIGHT_STEP);
    
    float halfW = TILE_WIDTH / 2.0f;
    float halfH = TILE_HEIGHT / 2.0f;
    
    float gx = (px / halfW + py / halfH) / 2.0f;
    float gy = (py / halfH - px / halfW) / 2.0f;
    return {gx, gy};
}

void DrawTile(int gx, int gy, int gz, GroundType type, Vector2 camOffset, float zoom, bool hovered) {
    Vector2 top = GridToScreen((float)gx, (float)gy, (float)gz, camOffset, zoom);
    Vector2 right = GridToScreen((float)(gx + 1), (float)gy, (float)gz, camOffset, zoom);
    Vector2 bottom = GridToScreen((float)(gx + 1), (float)(gy + 1), (float)gz, camOffset, zoom);
    Vector2 left = GridToScreen((float)gx, (float)(gy + 1), (float)gz, camOffset, zoom);

    Color topColor, leftColor, rightColor;

    switch (type) {
        case GROUND_GRASS:
            topColor = (gx % 2 == gy % 2) ? Color{60, 185, 95, 255} : Color{50, 165, 82, 255}; // Rich emerald lawn
            leftColor = Color{32, 100, 42, 255};
            rightColor = Color{40, 120, 52, 255};
            break;
        case GROUND_DIRT:
            topColor = Color{140, 100, 75, 255};
            leftColor = Color{95, 68, 52, 255};
            rightColor = Color{112, 80, 62, 255};
            break;
        case GROUND_WATER:
            topColor = Color{14, 165, 233, 220}; // Deep sparkling turquoise canal
            leftColor = Color{2, 132, 199, 240};
            rightColor = Color{3, 105, 161, 240};
            break;
        case GROUND_PATH:
            topColor = Color{248, 250, 252, 255}; // Bright architectural concrete
            leftColor = Color{210, 218, 228, 255};
            rightColor = Color{165, 178, 196, 255};
            break;
        case GROUND_QUEUE:
            topColor = Color{253, 224, 71, 255}; // High-vis tactile platform safety yellow
            leftColor = Color{234, 179, 8, 255};
            rightColor = Color{202, 138, 4, 255};
            break;
        case GROUND_PLAZA:
            topColor = (gx % 2 == gy % 2) ? Color{241, 245, 249, 255} : Color{226, 232, 240, 255}; // Granite plaza pavers
            leftColor = Color{155, 168, 185, 255};
            rightColor = Color{115, 130, 150, 255};
            break;
    }

    if (hovered) {
        topColor.r = (unsigned char)std::min(255, topColor.r + 40);
        topColor.g = (unsigned char)std::min(255, topColor.g + 40);
        topColor.b = (unsigned char)std::min(255, topColor.b + 40);
    }

    // Depth thickness for elevated terrain
    float cliffH = 16.0f * zoom;
    Vector2 leftBottom = {left.x, left.y + cliffH};
    Vector2 centerBottom = {bottom.x, bottom.y + cliffH};
    Vector2 rightBottom = {right.x, right.y + cliffH};

    // Draw left cliff side
    DrawTriangle(left, leftBottom, centerBottom, leftColor);
    DrawTriangle(left, centerBottom, bottom, leftColor);

    // Draw right cliff side
    DrawTriangle(bottom, centerBottom, rightBottom, rightColor);
    DrawTriangle(bottom, rightBottom, right, rightColor);

    // Draw geological earth strata lines on cliff sides
    if (cliffH > 6.0f) {
        float strataY1 = 5.0f * zoom;
        float strataY2 = 11.0f * zoom;
        DrawLineEx({left.x, left.y + strataY1}, {bottom.x, bottom.y + strataY1}, 1.2f * zoom, Color{20, 60, 28, 200});
        DrawLineEx({bottom.x, bottom.y + strataY1}, {right.x, right.y + strataY1}, 1.2f * zoom, Color{25, 75, 34, 200});
        DrawLineEx({left.x, left.y + strataY2}, {bottom.x, bottom.y + strataY2}, 1.0f * zoom, Color{80, 55, 40, 180});
        DrawLineEx({bottom.x, bottom.y + strataY2}, {right.x, right.y + strataY2}, 1.0f * zoom, Color{95, 65, 48, 180});
    }

    // Draw top diamond face
    DrawTriangle(top, left, bottom, topColor);
    DrawTriangle(top, bottom, right, topColor);

    // Grass micro-blades texture on selective tiles
    if (type == GROUND_GRASS) {
        unsigned int h = ((unsigned int)gx * 73856093u) ^ ((unsigned int)gy * 19349663u);
        if ((h % 4) == 0) {
            Vector2 c = { (top.x + bottom.x) * 0.5f, (top.y + bottom.y) * 0.5f };
            DrawLineEx(c, {c.x - 1.5f * zoom, c.y - 3.5f * zoom}, 1.1f * zoom, Color{90, 220, 120, 190});
            DrawLineEx(c, {c.x + 1.8f * zoom, c.y - 3.0f * zoom}, 1.1f * zoom, Color{110, 235, 140, 190});
        }
    }

    // Sidewalk & Plaza modular paver stone grid pattern
    if (type == GROUND_PATH || type == GROUND_PLAZA) {
        Vector2 midTopLeft = { (top.x + left.x) * 0.5f, (top.y + left.y) * 0.5f };
        Vector2 midBotRight = { (bottom.x + right.x) * 0.5f, (bottom.y + right.y) * 0.5f };
        Vector2 midTopRight = { (top.x + right.x) * 0.5f, (top.y + right.y) * 0.5f };
        Vector2 midBotLeft = { (bottom.x + left.x) * 0.5f, (bottom.y + left.y) * 0.5f };

        DrawLineEx(midTopLeft, midBotRight, 1.0f, Color{148, 163, 184, 110});
        DrawLineEx(midTopRight, midBotLeft, 1.0f, Color{148, 163, 184, 110});
    }

    // High-vis yellow tactile safety bumps on platform queue floor
    if (type == GROUND_QUEUE) {
        Vector2 mid = { (top.x + bottom.x) * 0.5f, (top.y + bottom.y) * 0.5f };
        for (int qx = -1; qx <= 1; ++qx) {
            for (int qy = -1; qy <= 1; ++qy) {
                Vector2 stud = { mid.x + (float)(qx - qy) * 4.5f * zoom, mid.y + (float)(qx + qy) * 2.2f * zoom };
                DrawCircle((int)stud.x, (int)stud.y, 1.3f * zoom, Color{202, 138, 4, 255});
                DrawCircle((int)stud.x, (int)(stud.y - 0.5f * zoom), 0.8f * zoom, Color{254, 240, 138, 255});
            }
        }
    }

    // Water multi-wave animated caustics & sunlit glints
    if (type == GROUND_WATER) {
        float t = (float)GetTime();
        float w1 = sinf((float)gx * 1.6f + (float)gy * 2.1f + t * 3.5f);
        float w2 = cosf((float)gx * 2.4f - (float)gy * 1.8f + t * 2.8f);
        float shimmer = (w1 + w2) * 0.5f;

        Vector2 mid = { (top.x + bottom.x) * 0.5f, (top.y + bottom.y) * 0.5f };
        Vector2 wStart = { mid.x - 7.0f * zoom, mid.y - 2.0f * zoom + shimmer * 1.5f * zoom };
        Vector2 wEnd   = { mid.x + 7.0f * zoom, mid.y + 2.0f * zoom + shimmer * 1.5f * zoom };
        DrawLineEx(wStart, wEnd, 1.8f * zoom, Color{255, 255, 255, (unsigned char)(110 + shimmer * 60)});

        if (shimmer > 0.35f) {
            DrawCircle((int)(mid.x + 3.0f * zoom), (int)(mid.y - 1.0f * zoom), 1.6f * zoom, Color{255, 255, 255, 220});
        }
    }

    // Subtle edge bevel outline
    DrawLineEx(top, right, 1.0f, Color{0, 0, 0, 30});
    DrawLineEx(right, bottom, 1.0f, Color{0, 0, 0, 45});
    DrawLineEx(bottom, left, 1.0f, Color{0, 0, 0, 45});
    DrawLineEx(left, top, 1.0f, Color{0, 0, 0, 30});
}

void DrawPillar(int gx, int gy, int groundZ, int trackZ, Vector2 camOffset, float zoom) {
    if (trackZ <= groundZ) return;

    Vector2 basePos = GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, (float)groundZ, camOffset, zoom);
    Vector2 topPos = GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, (float)trackZ, camOffset, zoom);

    float pierW = std::max(4.0f, 6.5f * zoom);
    float capW  = std::max(10.0f, 16.0f * zoom);
    float capH  = std::max(2.5f, 4.0f * zoom);

    // Pillar Base Shadow
    DrawEllipse((int)basePos.x, (int)basePos.y, 8.0f * zoom, 4.0f * zoom, Color{0, 0, 0, 90});

    // Concrete Viaduct Pier Body
    DrawRectangle((int)(topPos.x - pierW * 0.5f), (int)topPos.y, (int)pierW, (int)(basePos.y - topPos.y), Color{100, 116, 139, 255});
    // Pier Concrete Highlight
    DrawLineEx({topPos.x - pierW * 0.3f, topPos.y}, {basePos.x - pierW * 0.3f, basePos.y}, 1.2f * zoom, Color{148, 163, 184, 200});

    // Crosshead Pier Capital Beam
    DrawRectangle((int)(topPos.x - capW * 0.5f), (int)(topPos.y - capH), (int)capW, (int)capH, Color{71, 85, 105, 255});
    DrawRectangleLines((int)(topPos.x - capW * 0.5f), (int)(topPos.y - capH), (int)capW, (int)capH, Color{148, 163, 184, 255});
}

void DrawTrackBasePillars(float gx, float gy, float trackZ, Vector2 camOffset, float zoom) {
    if (trackZ <= 0.05f) return;
    Vector2 basePos = GridToScreen(gx, gy, 0.0f, camOffset, zoom);
    Vector2 topPos = GridToScreen(gx, gy, trackZ, camOffset, zoom);
    float poleW = std::max(3.0f, 5.0f * zoom);

    DrawEllipse((int)basePos.x, (int)basePos.y, 7.0f * zoom, 3.5f * zoom, Color{0, 0, 0, 80});
    DrawRectangle((int)(topPos.x - poleW * 0.5f), (int)topPos.y, (int)poleW, (int)(basePos.y - topPos.y), Color{100, 116, 139, 255});
}

void DrawCursor(int gx, int gy, int gz, Vector2 camOffset, float zoom, Color color) {
    Vector2 top = GridToScreen((float)gx, (float)gy, (float)gz, camOffset, zoom);
    Vector2 right = GridToScreen((float)(gx + 1), (float)gy, (float)gz, camOffset, zoom);
    Vector2 bottom = GridToScreen((float)(gx + 1), (float)(gy + 1), (float)gz, camOffset, zoom);
    Vector2 left = GridToScreen((float)gx, (float)(gy + 1), (float)gz, camOffset, zoom);

    float thick = 2.5f * zoom;
    DrawLineEx(top, right, thick, color);
    DrawLineEx(right, bottom, thick, color);
    DrawLineEx(bottom, left, thick, color);
    DrawLineEx(left, top, thick, color);

    Color fillC = color;
    fillC.a = 40;
    DrawTriangle(top, left, bottom, fillC);
    DrawTriangle(top, bottom, right, fillC);
}

void DrawShadow(float gx, float gy, float radius, Vector2 camOffset, float zoom) {
    Vector2 pos = GridToScreen(gx, gy, 0.0f, camOffset, zoom);
    DrawEllipse((int)pos.x, (int)pos.y, radius * zoom, (radius * 0.5f) * zoom, Color{0, 0, 0, 70});
}

void DrawScenery(int gx, int gy, int gz, SceneryType type, Vector2 camOffset, float zoom) {
    if (type == SCENERY_NONE) return;

    Vector2 center = GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, (float)gz, camOffset, zoom);

    switch (type) {
        case SCENERY_METRO_ENTRANCE: {
            // Shadow
            DrawEllipse((int)center.x, (int)center.y, 14.0f * zoom, 7.0f * zoom, Color{0, 0, 0, 70});

            // Subway Staircase cut into ground
            float stairW = 20.0f * zoom;
            float stairH = 12.0f * zoom;
            DrawRectangle((int)(center.x - stairW * 0.5f), (int)(center.y - stairH * 0.5f), (int)stairW, (int)stairH, Color{30, 41, 59, 255});
            // Stair steps
            for (int s = 0; s < 4; ++s) {
                float sy = center.y - stairH * 0.5f + (float)s * (stairH / 4.0f);
                DrawLineEx({center.x - stairW * 0.45f, sy}, {center.x + stairW * 0.45f, sy}, 1.5f * zoom, Color{71, 85, 105, 255});
            }

            // Glass/Steel Entrance Canopy Railings
            DrawRectangleLines((int)(center.x - stairW * 0.5f), (int)(center.y - stairH * 0.5f), (int)stairW, (int)stairH, Color{148, 163, 184, 255});

            // Glowing Metro "M" Totem Pole
            Vector2 totemBase = {center.x + stairW * 0.55f, center.y};
            float poleH = 28.0f * zoom;
            DrawLineEx(totemBase, {totemBase.x, totemBase.y - poleH}, 2.5f * zoom, Color{15, 23, 42, 255});

            // Glowing Metro Cube/Roundel
            Vector2 cubePos = {totemBase.x, totemBase.y - poleH};
            float cubeSz = 9.0f * zoom;
            DrawRectangleRounded(Rectangle{cubePos.x - cubeSz * 0.5f, cubePos.y - cubeSz * 0.5f, cubeSz, cubeSz}, 0.3f, 4, Color{229, 57, 53, 255});
            DrawRectangleRoundedLines(Rectangle{cubePos.x - cubeSz * 0.5f, cubePos.y - cubeSz * 0.5f, cubeSz, cubeSz}, 0.3f, 4, WHITE);
            DrawText("M", (int)(cubePos.x - 2.8f * zoom), (int)(cubePos.y - 4.0f * zoom), (int)(8.0f * zoom), WHITE);

            // Neon glow halo
            DrawCircleGradient(cubePos, 14.0f * zoom, Color{239, 68, 68, 90}, Color{239, 68, 68, 0});
            break;
        }

        case SCENERY_TURNSTILE_GATE: {
            DrawEllipse((int)center.x, (int)center.y, 14.0f * zoom, 7.0f * zoom, Color{0, 0, 0, 70});

            // TVM (Ticket Vending Machine) on left
            DrawRectangle((int)(center.x - 12.0f * zoom), (int)(center.y - 18.0f * zoom), (int)(8.0f * zoom), (int)(18.0f * zoom), Color{51, 65, 85, 255});
            DrawRectangle((int)(center.x - 11.0f * zoom), (int)(center.y - 15.0f * zoom), (int)(6.0f * zoom), (int)(6.0f * zoom), Color{14, 165, 233, 255}); // Touchscreen
            DrawCircle((int)(center.x - 8.0f * zoom), (int)(center.y - 6.0f * zoom), 1.2f * zoom, Color{250, 204, 21, 255}); // Coin/card slot

            // Bank of automated optical flap turnstiles
            float gateW = 16.0f * zoom;
            float gateH = 11.0f * zoom;
            DrawRectangle((int)(center.x), (int)(center.y - gateH), (int)gateW, (int)gateH, Color{148, 163, 184, 255});
            // Glass flap doors
            DrawRectangle((int)(center.x + 4.0f * zoom), (int)(center.y - gateH + 2.0f * zoom), (int)(3.0f * zoom), (int)(gateH - 3.0f * zoom), Color{56, 189, 248, 200});
            // Green LED directional arrow
            DrawCircle((int)(center.x + 8.0f * zoom), (int)(center.y - gateH - 1.5f * zoom), 2.0f * zoom, Color{34, 197, 94, 255});
            break;
        }

        case SCENERY_MAP_KIOSK: {
            DrawEllipse((int)center.x, (int)center.y, 10.0f * zoom, 5.0f * zoom, Color{0, 0, 0, 60});

            // Backlit transit map totem
            float mapW = 16.0f * zoom;
            float mapH = 22.0f * zoom;
            DrawRectangle((int)(center.x - mapW * 0.5f), (int)(center.y - mapH), (int)mapW, (int)mapH, Color{15, 23, 42, 255});
            DrawRectangleLines((int)(center.x - mapW * 0.5f), (int)(center.y - mapH), (int)mapW, (int)mapH, Color{71, 85, 105, 255});

            // Harry Beck style colored transit route lines on map face
            Vector2 m1 = {center.x - 6.0f * zoom, center.y - 16.0f * zoom};
            Vector2 m2 = {center.x + 5.0f * zoom, center.y - 8.0f * zoom};
            DrawLineEx(m1, m2, 1.8f * zoom, Color{239, 68, 68, 255}); // Red line
            Vector2 m3 = {center.x - 5.0f * zoom, center.y - 8.0f * zoom};
            Vector2 m4 = {center.x + 4.0f * zoom, center.y - 17.0f * zoom};
            DrawLineEx(m3, m4, 1.8f * zoom, Color{14, 165, 233, 255}); // Blue line
            // Station interchange dot
            DrawCircle((int)center.x, (int)(center.y - 12.0f * zoom), 2.2f * zoom, WHITE);
            break;
        }

        case SCENERY_STREET_TREE: {
            // Cast iron circular tree grate on sidewalk
            DrawEllipse((int)center.x, (int)center.y, 13.0f * zoom, 6.5f * zoom, Color{51, 65, 85, 255});
            DrawEllipse((int)center.x, (int)center.y, 9.0f * zoom, 4.5f * zoom, Color{30, 41, 59, 255});

            // Trunk
            DrawRectangle((int)(center.x - 2.5f * zoom), (int)(center.y - 18.0f * zoom), (int)(5.0f * zoom), (int)(18.0f * zoom), Color{101, 78, 64, 255});

            // Lush manicured spherical tree crown
            DrawCircle((int)(center.x - 8.0f * zoom), (int)(center.y - 24.0f * zoom), 9.0f * zoom, Color{34, 197, 94, 255});
            DrawCircle((int)(center.x + 8.0f * zoom), (int)(center.y - 24.0f * zoom), 9.0f * zoom, Color{22, 163, 74, 255});
            DrawCircle((int)center.x, (int)(center.y - 30.0f * zoom), 11.0f * zoom, Color{74, 222, 128, 255});
            break;
        }

        case SCENERY_PINE_TREE: {
            DrawEllipse((int)center.x, (int)center.y, 10.0f * zoom, 5.0f * zoom, Color{0, 0, 0, 60});
            DrawRectangle((int)(center.x - 2.0f * zoom), (int)(center.y - 12.0f * zoom), (int)(4.0f * zoom), (int)(12.0f * zoom), Color{93, 64, 55, 255});

            Vector2 b1 = {center.x - 13.0f * zoom, center.y - 10.0f * zoom};
            Vector2 b2 = {center.x + 13.0f * zoom, center.y - 10.0f * zoom};
            Vector2 bTop = {center.x, center.y - 26.0f * zoom};
            DrawTriangle(b1, b2, bTop, Color{27, 94, 32, 255});

            Vector2 m1 = {center.x - 10.0f * zoom, center.y - 20.0f * zoom};
            Vector2 m2 = {center.x + 10.0f * zoom, center.y - 20.0f * zoom};
            Vector2 mTop = {center.x, center.y - 34.0f * zoom};
            DrawTriangle(m1, m2, mTop, Color{46, 125, 50, 255});

            Vector2 t1 = {center.x - 7.0f * zoom, center.y - 28.0f * zoom};
            Vector2 t2 = {center.x + 7.0f * zoom, center.y - 28.0f * zoom};
            Vector2 tTop = {center.x, center.y - 42.0f * zoom};
            DrawTriangle(t1, t2, tTop, Color{56, 142, 60, 255});
            break;
        }

        case SCENERY_BENCH: {
            DrawEllipse((int)center.x, (int)center.y, 8.0f * zoom, 4.0f * zoom, Color{0, 0, 0, 50});
            // Architectural Teak + Brushed Steel Bench
            DrawRectangle((int)(center.x - 9.0f * zoom), (int)(center.y - 6.0f * zoom), (int)(18.0f * zoom), (int)(4.0f * zoom), Color{180, 83, 9, 255});
            DrawRectangle((int)(center.x - 9.0f * zoom), (int)(center.y - 10.0f * zoom), (int)(18.0f * zoom), (int)(3.0f * zoom), Color{217, 119, 6, 255});
            DrawLineEx({center.x - 9.0f * zoom, center.y - 11.0f * zoom}, {center.x - 9.0f * zoom, center.y}, 2.0f * zoom, Color{148, 163, 184, 255});
            DrawLineEx({center.x + 9.0f * zoom, center.y - 11.0f * zoom}, {center.x + 9.0f * zoom, center.y}, 2.0f * zoom, Color{148, 163, 184, 255});
            break;
        }

        case SCENERY_LAMP_POST: {
            DrawEllipse((int)center.x, (int)center.y, 5.0f * zoom, 3.0f * zoom, Color{0, 0, 0, 60});
            // Modern Minimalist Dark Steel LED Luminaire
            DrawLineEx({center.x, center.y}, {center.x, center.y - 26.0f * zoom}, 2.2f * zoom, Color{30, 41, 59, 255});
            // Curved cantilever head
            DrawLineEx({center.x, center.y - 26.0f * zoom}, {center.x + 6.0f * zoom, center.y - 26.0f * zoom}, 2.0f * zoom, Color{30, 41, 59, 255});
            DrawCircle((int)(center.x + 6.0f * zoom), (int)(center.y - 25.0f * zoom), 2.8f * zoom, Color{254, 240, 138, 255});
            DrawCircleGradient(Vector2{center.x + 6.0f * zoom, center.y - 25.0f * zoom}, 14.0f * zoom, Color{254, 240, 138, 70}, Color{254, 240, 138, 0});
            break;
        }

        case SCENERY_NEWSSTAND: {
            // Platform Coffee Kiosk & Newsstand
            DrawEllipse((int)center.x, (int)center.y, 14.0f * zoom, 7.0f * zoom, Color{0, 0, 0, 60});
            DrawRectangle((int)(center.x - 12.0f * zoom), (int)(center.y - 16.0f * zoom), (int)(24.0f * zoom), (int)(16.0f * zoom), Color{30, 41, 59, 255});
            // Counter opening
            DrawRectangle((int)(center.x - 10.0f * zoom), (int)(center.y - 12.0f * zoom), (int)(20.0f * zoom), (int)(6.0f * zoom), Color{15, 23, 42, 255});
            // Coffee Machine
            DrawRectangle((int)(center.x - 8.0f * zoom), (int)(center.y - 11.0f * zoom), (int)(4.0f * zoom), (int)(4.0f * zoom), Color{239, 68, 68, 255});

            // Modern Flat Canopy Roof (Teal)
            DrawRectangle((int)(center.x - 14.0f * zoom), (int)(center.y - 21.0f * zoom), (int)(28.0f * zoom), (int)(5.0f * zoom), Color{14, 165, 233, 255});
            DrawText("METRO CAFE", (int)(center.x - 13.0f * zoom), (int)(center.y - 27.0f * zoom), (int)(6.5f * zoom), Color{254, 240, 138, 255});
            break;
        }

        case SCENERY_BIKE_RACK: {
            DrawEllipse((int)center.x, (int)center.y, 13.0f * zoom, 6.0f * zoom, Color{0, 0, 0, 50});
            // City Bike Share Dock
            DrawRectangle((int)(center.x - 12.0f * zoom), (int)(center.y - 4.0f * zoom), (int)(24.0f * zoom), (int)(4.0f * zoom), Color{51, 65, 85, 255});
            // 3 Docked Bicycles
            for (int b = 0; b < 3; ++b) {
                float bx = center.x - 8.0f * zoom + (float)b * 8.0f * zoom;
                DrawLineEx({bx, center.y - 3.0f * zoom}, {bx, center.y - 9.0f * zoom}, 1.5f * zoom, Color{34, 197, 94, 255}); // Green city bikes
                DrawCircle((int)bx, (int)(center.y - 9.0f * zoom), 1.8f * zoom, Color{15, 23, 42, 255}); // Handlebars
            }
            break;
        }

        case SCENERY_FOUNTAIN: {
            // Splashing Park Water Fountain (RCT Style)
            DrawEllipse((int)center.x, (int)center.y, 16.0f * zoom, 8.0f * zoom, Color{0, 0, 0, 60});
            // Outer stone basin
            DrawEllipse((int)center.x, (int)(center.y - 3.0f * zoom), 15.0f * zoom, 7.5f * zoom, Color{148, 163, 184, 255});
            // Water pool
            DrawEllipse((int)center.x, (int)(center.y - 4.0f * zoom), 13.0f * zoom, 6.0f * zoom, Color{14, 165, 233, 230});

            // Center pedestal
            DrawRectangle((int)(center.x - 3.0f * zoom), (int)(center.y - 14.0f * zoom), (int)(6.0f * zoom), (int)(10.0f * zoom), Color{203, 213, 225, 255});
            // Upper basin tier
            DrawEllipse((int)center.x, (int)(center.y - 14.0f * zoom), 8.0f * zoom, 4.0f * zoom, Color{148, 163, 184, 255});
            DrawEllipse((int)center.x, (int)(center.y - 15.0f * zoom), 6.5f * zoom, 3.2f * zoom, Color{56, 189, 248, 240});

            // Animated water sprays
            float fTime = (float)GetTime() * 5.0f;
            for (int j = 0; j < 4; ++j) {
                float angle = (float)j * (PI / 2.0f) + fTime * 0.2f;
                float sprayH = 6.0f + 2.0f * sinf(fTime + (float)j);
                Vector2 sTip = { center.x + cosf(angle) * 7.0f * zoom, center.y - 15.0f * zoom - sprayH * zoom };
                DrawLineEx({center.x, center.y - 15.0f * zoom}, sTip, 1.2f * zoom, Color{224, 242, 254, 220});
                DrawCircle((int)sTip.x, (int)sTip.y, 1.5f * zoom, WHITE);
            }
            break;
        }

        case SCENERY_FLOWER_BED: {
            // Vibrant Multi-Color Botanical Blossoms
            DrawEllipse((int)center.x, (int)center.y, 12.0f * zoom, 6.0f * zoom, Color{0, 0, 0, 50});
            // Low stone border
            DrawEllipse((int)center.x, (int)(center.y - 1.5f * zoom), 11.0f * zoom, 5.5f * zoom, Color{100, 116, 139, 255});
            // Rich soil
            DrawEllipse((int)center.x, (int)(center.y - 2.5f * zoom), 9.5f * zoom, 4.5f * zoom, Color{74, 45, 30, 255});

            // Colorful flower clusters
            Color fColors[] = {
                Color{239, 68, 68, 255},  // Red poppy
                Color{245, 158, 11, 255}, // Marigold
                Color{168, 85, 247, 255}, // Lavender
                Color{236, 72, 153, 255}, // Rose pink
                Color{250, 204, 21, 255}  // Buttercup
            };
            for (int f = 0; f < 6; ++f) {
                float fx = center.x + (float)((f * 3) % 7 - 3) * 2.2f * zoom;
                float fy = center.y - 4.5f * zoom + (float)((f * 5) % 5 - 2) * 1.2f * zoom;
                DrawCircle((int)fx, (int)fy, 1.8f * zoom, fColors[f % 5]);
                DrawCircle((int)fx, (int)fy, 0.7f * zoom, Color{254, 240, 138, 255});
            }
            break;
        }

        default: break;
    }
}

void DrawStaff(const StaffMember& staff, Vector2 camOffset, float zoom) {
    Vector2 sPos = GridToScreen(staff.pos.x, staff.pos.y, 0.0f, camOffset, zoom);
    float bob = sinf(staff.walkTimer * 8.0f) * 1.5f * zoom;

    DrawEllipse((int)sPos.x, (int)sPos.y, 4.5f * zoom, 2.2f * zoom, Color{0, 0, 0, 80});

    float bodyH = 11.0f * zoom;
    float bodyW = 5.5f * zoom;

    if (staff.type == STAFF_CUSTODIAN) {
        // Transit Custodian: Navy uniform + High-vis armband
        DrawRectangle((int)(sPos.x - bodyW * 0.4f), (int)(sPos.y - bodyH * 0.45f + bob), (int)(bodyW * 0.8f), (int)(bodyH * 0.45f), Color{30, 41, 59, 255});
        DrawRectangle((int)(sPos.x - bodyW * 0.5f), (int)(sPos.y - bodyH + bob), (int)bodyW, (int)(bodyH * 0.6f), Color{30, 58, 138, 255});
        // High-vis yellow armband
        DrawRectangle((int)(sPos.x + bodyW * 0.3f), (int)(sPos.y - bodyH * 0.8f + bob), (int)(2.0f * zoom), (int)(3.0f * zoom), Color{250, 204, 21, 255});

        // Head + Navy peaked cap
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 3.0f * zoom + bob), 2.8f * zoom, Color{255, 224, 178, 255});
        DrawRectangle((int)(sPos.x - 3.0f * zoom), (int)(sPos.y - bodyH - 6.0f * zoom + bob), (int)(6.0f * zoom), (int)(2.5f * zoom), Color{15, 23, 42, 255});

        // Broom
        float sweepAngle = sinf(staff.walkTimer * 6.0f) * 20.0f;
        Vector2 broomEnd = { sPos.x + 6.0f * zoom + sweepAngle * 0.2f, sPos.y - 1.0f * zoom };
        DrawLineEx({sPos.x, sPos.y - bodyH * 0.5f + bob}, broomEnd, 1.5f * zoom, Color{148, 163, 184, 255});
        DrawRectanglePro(Rectangle{broomEnd.x, broomEnd.y, 6.0f * zoom, 3.0f * zoom}, Vector2{3.0f * zoom, 1.5f * zoom}, sweepAngle, Color{203, 213, 225, 255});
    } else {
        // Signal Engineer: High-vis orange safety vest + White hard hat
        DrawRectangle((int)(sPos.x - bodyW * 0.4f), (int)(sPos.y - bodyH * 0.45f + bob), (int)(bodyW * 0.8f), (int)(bodyH * 0.45f), Color{30, 41, 59, 255});
        DrawRectangle((int)(sPos.x - bodyW * 0.5f), (int)(sPos.y - bodyH + bob), (int)bodyW, (int)(bodyH * 0.6f), Color{249, 115, 22, 255});
        // Reflective silver stripe
        DrawLineEx({sPos.x - bodyW * 0.5f, sPos.y - bodyH * 0.7f + bob}, {sPos.x + bodyW * 0.5f, sPos.y - bodyH * 0.7f + bob}, 1.5f * zoom, Color{241, 245, 249, 255});

        // Head + White hardhat
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 3.0f * zoom + bob), 2.8f * zoom, Color{255, 224, 178, 255});
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 4.5f * zoom + bob), 3.2f * zoom, WHITE);

        // Tool / Diagnostic tablet
        DrawRectangle((int)(sPos.x + 3.0f * zoom), (int)(sPos.y - bodyH * 0.5f + bob), (int)(4.0f * zoom), (int)(5.0f * zoom), Color{15, 23, 42, 255});
        DrawCircle((int)(sPos.x + 5.0f * zoom), (int)(sPos.y - bodyH * 0.4f + bob), 1.0f * zoom, Color{14, 165, 233, 255});
    }
}

void DrawMess(const StationMess& mess, Vector2 camOffset, float zoom) {
    Vector2 sPos = GridToScreen(mess.pos.x, mess.pos.y, 0.0f, camOffset, zoom);

    if (mess.isSpill) {
        // Spilled Takeaway Coffee
        DrawEllipse((int)sPos.x, (int)sPos.y, 6.0f * zoom, 3.0f * zoom, Color{120, 53, 15, 220});
        DrawRectangle((int)(sPos.x - 2.0f * zoom), (int)(sPos.y - 4.0f * zoom), (int)(4.0f * zoom), (int)(4.0f * zoom), Color{254, 240, 138, 255}); // Paper cup
    } else {
        // Dropped subway newspaper / ticket
        DrawEllipse((int)sPos.x, (int)sPos.y, 4.0f * zoom, 2.0f * zoom, Color{0, 0, 0, 50});
        DrawRectangle((int)(sPos.x - 2.5f * zoom), (int)(sPos.y - 2.0f * zoom), (int)(5.0f * zoom), (int)(4.0f * zoom), Color{226, 232, 240, 255});
        DrawLine((int)(sPos.x - 2.0f * zoom), (int)(sPos.y), (int)(sPos.x + 2.0f * zoom), (int)(sPos.y), Color{100, 116, 139, 255});
    }
}

void DrawTransitPortalArch(Vector2 gridPos, Vector2 camOffset, float zoom) {
    Vector2 leftPillar = GridToScreen(gridPos.x, gridPos.y - 0.6f, 0.0f, camOffset, zoom);
    Vector2 rightPillar = GridToScreen(gridPos.x, gridPos.y + 0.6f, 0.0f, camOffset, zoom);

    float pHeight = 36.0f * zoom;
    float pWidth = 8.0f * zoom;

    // Architectural Dark Granite Portal Pillars
    DrawRectangle((int)(leftPillar.x - pWidth * 0.5f), (int)(leftPillar.y - pHeight), (int)pWidth, (int)pHeight, Color{30, 41, 59, 255});
    DrawRectangle((int)(rightPillar.x - pWidth * 0.5f), (int)(rightPillar.y - pHeight), (int)pWidth, (int)pHeight, Color{30, 41, 59, 255});

    // Beam linking pillars
    Vector2 archLeft = {leftPillar.x, leftPillar.y - pHeight};
    Vector2 archRight = {rightPillar.x, rightPillar.y - pHeight};
    DrawLineEx(archLeft, archRight, 4.0f * zoom, Color{15, 23, 42, 255});

    // Overhead Sign Board: METRO GRID
    Vector2 center = { (archLeft.x + archRight.x) * 0.5f, (archLeft.y + archRight.y) * 0.5f - 8.0f * zoom };
    float signW = 96.0f * zoom;
    float signH = 19.0f * zoom;
    DrawRectangleRounded(Rectangle{center.x - signW * 0.5f, center.y - signH * 0.5f, signW, signH}, 0.25f, 4, Color{15, 23, 42, 255});
    DrawRectangleRoundedLines(Rectangle{center.x - signW * 0.5f, center.y - signH * 0.5f, signW, signH}, 0.25f, 4, Color{229, 57, 53, 255});

    // Glowing "M" Roundel on sign
    DrawCircle((int)(center.x - 36.0f * zoom), (int)center.y, 6.0f * zoom, Color{229, 57, 53, 255});
    DrawText("M", (int)(center.x - 39.0f * zoom), (int)(center.y - 4.0f * zoom), (int)(8.0f * zoom), WHITE);

    DrawText("METRO GRID", (int)(center.x - 24.0f * zoom), (int)(center.y - 5.0f * zoom), (int)(9.0f * zoom), WHITE);
}

void DrawEntranceArch(Vector2 gridPos, Vector2 camOffset, float zoom) {
    DrawTransitPortalArch(gridPos, camOffset, zoom);
}

} // namespace Iso


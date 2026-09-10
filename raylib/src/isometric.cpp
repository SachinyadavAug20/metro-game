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
            topColor = (gx % 2 == gy % 2) ? Color{76, 175, 80, 255} : Color{67, 160, 71, 255}; // Cheerful RCT grass
            leftColor = Color{46, 125, 50, 255};
            rightColor = Color{56, 142, 60, 255};
            break;
        case GROUND_DIRT:
            topColor = Color{141, 110, 99, 255};
            leftColor = Color{109, 76, 65, 255};
            rightColor = Color{121, 85, 72, 255};
            break;
        case GROUND_WATER:
            topColor = Color{33, 150, 243, 220};
            leftColor = Color{21, 101, 192, 240};
            rightColor = Color{25, 118, 210, 240};
            break;
        case GROUND_PATH:
            topColor = Color{224, 224, 224, 255}; // Cobblestone walkway
            leftColor = Color{189, 189, 189, 255};
            rightColor = Color{158, 158, 158, 255};
            break;
        case GROUND_QUEUE:
            topColor = Color{255, 213, 79, 255}; // Queue path yellow
            leftColor = Color{255, 179, 0, 255};
            rightColor = Color{255, 193, 7, 255};
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

    // Draw top diamond face
    DrawTriangle(top, left, bottom, topColor);
    DrawTriangle(top, bottom, right, topColor);

    // Path pattern / cobblestone lines if path
    if (type == GROUND_PATH) {
        DrawLineV(top, bottom, Color{200, 200, 200, 180});
        DrawLineV(left, right, Color{200, 200, 200, 180});
    }

    // Queue Post-and-Rope stanchions
    if (type == GROUND_QUEUE) {
        float poleH = 8.0f * zoom;
        Vector2 pL = {left.x, left.y - poleH};
        Vector2 pR = {right.x, right.y - poleH};
        Vector2 pT = {top.x, top.y - poleH};
        Vector2 pB = {bottom.x, bottom.y - poleH};

        // Red velvet ropes
        DrawLineEx(pT, pR, 1.8f * zoom, Color{198, 40, 40, 230});
        DrawLineEx(pL, pB, 1.8f * zoom, Color{198, 40, 40, 230});

        // Brass stanchion poles
        Color brass = Color{255, 193, 7, 255};
        DrawLineEx(left, pL, 2.0f * zoom, brass);
        DrawLineEx(right, pR, 2.0f * zoom, brass);
        DrawCircle((int)pL.x, (int)pL.y, 2.0f * zoom, Color{255, 235, 59, 255});
        DrawCircle((int)pR.x, (int)pR.y, 2.0f * zoom, Color{255, 235, 59, 255});
    }

    // Water gentle ripples
    if (type == GROUND_WATER) {
        float shimmer = sinf((float)gx * 1.8f + (float)gy * 2.3f + (float)GetTime() * 4.0f);
        if (shimmer > 0.1f) {
            Vector2 mid1 = { (top.x + left.x) * 0.5f, (top.y + left.y) * 0.5f };
            Vector2 mid2 = { (bottom.x + right.x) * 0.5f, (bottom.y + right.y) * 0.5f };
            DrawLineEx(mid1, mid2, 1.5f * zoom, Color{255, 255, 255, (unsigned char)(shimmer * 130)});
        }
    }

    // Subtle edge outline
    DrawLineV(top, right, Color{0, 0, 0, 30});
    DrawLineV(right, bottom, Color{0, 0, 0, 40});
    DrawLineV(bottom, left, Color{0, 0, 0, 40});
    DrawLineV(left, top, Color{0, 0, 0, 30});
}

void DrawPillar(int gx, int gy, int groundZ, int trackZ, Vector2 camOffset, float zoom) {
    if (trackZ <= groundZ) return;

    Vector2 basePos = GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, (float)groundZ, camOffset, zoom);
    Vector2 topPos = GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, (float)trackZ, camOffset, zoom);

    float poleW = std::max(2.0f, 3.5f * zoom);

    // Shadow at base
    DrawEllipse((int)basePos.x, (int)basePos.y, 6.0f * zoom, 3.0f * zoom, Color{0, 0, 0, 80});

    // Vertical steel tubular pole
    DrawRectangle((int)(topPos.x - poleW / 2.0f), (int)topPos.y, (int)poleW, (int)(basePos.y - topPos.y), Color{120, 144, 156, 255});
    // Highlight
    DrawLineEx({topPos.x - poleW / 4.0f, topPos.y}, {basePos.x - poleW / 4.0f, basePos.y}, 1.0f, Color{207, 216, 220, 200});

    // Cross brace if tall
    if (trackZ - groundZ >= 2) {
        float midY = (topPos.y + basePos.y) / 2.0f;
        float braceW = 12.0f * zoom;
        DrawLineEx({topPos.x - braceW, midY}, {topPos.x + braceW, midY}, 2.0f * zoom, Color{90, 110, 120, 255});
    }
}

void DrawTrackBasePillars(float gx, float gy, float trackZ, Vector2 camOffset, float zoom) {
    if (trackZ <= 0.05f) return;
    Vector2 basePos = GridToScreen(gx, gy, 0.0f, camOffset, zoom);
    Vector2 topPos = GridToScreen(gx, gy, trackZ, camOffset, zoom);
    float poleW = std::max(2.0f, 3.0f * zoom);

    DrawEllipse((int)basePos.x, (int)basePos.y, 5.0f * zoom, 2.5f * zoom, Color{0, 0, 0, 70});
    DrawRectangle((int)(topPos.x - poleW / 2.0f), (int)topPos.y, (int)poleW, (int)(basePos.y - topPos.y), Color{144, 164, 174, 255});
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

    // Subtle pulsing fill
    Color fillC = color;
    fillC.a = 40;
    DrawTriangle(top, left, bottom, fillC);
    DrawTriangle(top, bottom, right, fillC);
}

void DrawShadow(float gx, float gy, float radius, Vector2 camOffset, float zoom) {
    Vector2 pos = GridToScreen(gx, gy, 0.0f, camOffset, zoom);
    DrawEllipse((int)pos.x, (int)pos.y, radius * zoom, (radius * 0.5f) * zoom, Color{0, 0, 0, 60});
}

void DrawScenery(int gx, int gy, int gz, SceneryType type, Vector2 camOffset, float zoom) {
    if (type == SCENERY_NONE) return;

    Vector2 center = GridToScreen((float)gx + 0.5f, (float)gy + 0.5f, (float)gz, camOffset, zoom);

    switch (type) {
        case SCENERY_PINE_TREE: {
            // Shadow
            DrawEllipse((int)center.x, (int)center.y, 10.0f * zoom, 5.0f * zoom, Color{0, 0, 0, 60});

            // Trunk
            DrawRectangle((int)(center.x - 2.0f * zoom), (int)(center.y - 12.0f * zoom), (int)(4.0f * zoom), (int)(12.0f * zoom), Color{93, 64, 55, 255});

            // Layered pine needles (bottom to top)
            Vector2 b1 = {center.x - 14.0f * zoom, center.y - 10.0f * zoom};
            Vector2 b2 = {center.x + 14.0f * zoom, center.y - 10.0f * zoom};
            Vector2 bTop = {center.x, center.y - 26.0f * zoom};
            DrawTriangle(b1, b2, bTop, Color{27, 94, 32, 255});

            Vector2 m1 = {center.x - 11.0f * zoom, center.y - 20.0f * zoom};
            Vector2 m2 = {center.x + 11.0f * zoom, center.y - 20.0f * zoom};
            Vector2 mTop = {center.x, center.y - 34.0f * zoom};
            DrawTriangle(m1, m2, mTop, Color{46, 125, 50, 255});

            Vector2 t1 = {center.x - 8.0f * zoom, center.y - 28.0f * zoom};
            Vector2 t2 = {center.x + 8.0f * zoom, center.y - 28.0f * zoom};
            Vector2 tTop = {center.x, center.y - 42.0f * zoom};
            DrawTriangle(t1, t2, tTop, Color{56, 142, 60, 255});
            break;
        }

        case SCENERY_OAK_TREE: {
            // Shadow
            DrawEllipse((int)center.x, (int)center.y, 12.0f * zoom, 6.0f * zoom, Color{0, 0, 0, 60});

            // Trunk
            DrawRectangle((int)(center.x - 3.0f * zoom), (int)(center.y - 16.0f * zoom), (int)(6.0f * zoom), (int)(16.0f * zoom), Color{109, 76, 65, 255});

            // Lush Oak Foliage (overlapping spheres)
            DrawCircle((int)(center.x - 7.0f * zoom), (int)(center.y - 22.0f * zoom), 9.0f * zoom, Color{56, 142, 60, 255});
            DrawCircle((int)(center.x + 7.0f * zoom), (int)(center.y - 22.0f * zoom), 9.0f * zoom, Color{67, 160, 71, 255});
            DrawCircle((int)(center.x), (int)(center.y - 28.0f * zoom), 11.0f * zoom, Color{76, 175, 80, 255});
            DrawCircle((int)(center.x - 2.0f * zoom), (int)(center.y - 30.0f * zoom), 5.0f * zoom, Color{129, 199, 132, 255}); // Highlight
            break;
        }

        case SCENERY_BENCH: {
            // Cast iron sides + wood planks
            DrawEllipse((int)center.x, (int)center.y, 8.0f * zoom, 4.0f * zoom, Color{0, 0, 0, 50});
            DrawRectangle((int)(center.x - 9.0f * zoom), (int)(center.y - 6.0f * zoom), (int)(18.0f * zoom), (int)(4.0f * zoom), Color{141, 110, 99, 255});
            DrawRectangle((int)(center.x - 9.0f * zoom), (int)(center.y - 10.0f * zoom), (int)(18.0f * zoom), (int)(3.0f * zoom), Color{121, 85, 72, 255});
            DrawLineEx({center.x - 9.0f * zoom, center.y - 11.0f * zoom}, {center.x - 9.0f * zoom, center.y}, 2.0f * zoom, Color{46, 125, 50, 255});
            DrawLineEx({center.x + 9.0f * zoom, center.y - 11.0f * zoom}, {center.x + 9.0f * zoom, center.y}, 2.0f * zoom, Color{46, 125, 50, 255});
            break;
        }

        case SCENERY_DRINK_STALL: {
            // Booth base
            DrawEllipse((int)center.x, (int)center.y, 14.0f * zoom, 7.0f * zoom, Color{0, 0, 0, 60});
            DrawRectangle((int)(center.x - 12.0f * zoom), (int)(center.y - 16.0f * zoom), (int)(24.0f * zoom), (int)(16.0f * zoom), Color{77, 208, 225, 255});
            DrawRectangle((int)(center.x - 10.0f * zoom), (int)(center.y - 12.0f * zoom), (int)(20.0f * zoom), (int)(6.0f * zoom), Color{38, 50, 56, 255}); // Counter opening

            // Striped Awning (Red & White)
            for (int s = 0; s < 5; ++s) {
                Color awningC = (s % 2 == 0) ? Color{229, 57, 53, 255} : WHITE;
                DrawRectangle((int)(center.x - 13.0f * zoom + (float)s * 5.2f * zoom), (int)(center.y - 22.0f * zoom), (int)(5.5f * zoom), (int)(6.0f * zoom), awningC);
            }
            // Sign
            DrawText("SODA", (int)(center.x - 9.0f * zoom), (int)(center.y - 28.0f * zoom), (int)(7.0f * zoom), Color{255, 214, 0, 255});
            break;
        }

        case SCENERY_BALLOON_STALL: {
            DrawEllipse((int)center.x, (int)center.y, 12.0f * zoom, 6.0f * zoom, Color{0, 0, 0, 60});
            DrawRectangle((int)(center.x - 10.0f * zoom), (int)(center.y - 15.0f * zoom), (int)(20.0f * zoom), (int)(15.0f * zoom), Color{255, 179, 0, 255});
            // Awning
            DrawRectangle((int)(center.x - 12.0f * zoom), (int)(center.y - 20.0f * zoom), (int)(24.0f * zoom), (int)(5.0f * zoom), Color{156, 39, 176, 255});
            // Floating balloons
            DrawCircle((int)(center.x - 5.0f * zoom), (int)(center.y - 27.0f * zoom), 4.0f * zoom, Color{239, 68, 68, 255}); // Red
            DrawCircle((int)(center.x + 2.0f * zoom), (int)(center.y - 31.0f * zoom), 4.0f * zoom, Color{33, 150, 243, 255}); // Blue
            DrawCircle((int)(center.x + 7.0f * zoom), (int)(center.y - 26.0f * zoom), 4.0f * zoom, Color{255, 235, 59, 255}); // Yellow
            DrawLine((int)(center.x - 5.0f * zoom), (int)(center.y - 23.0f * zoom), (int)center.x, (int)(center.y - 15.0f * zoom), Color{120, 144, 156, 255});
            DrawLine((int)(center.x + 2.0f * zoom), (int)(center.y - 27.0f * zoom), (int)center.x, (int)(center.y - 15.0f * zoom), Color{120, 144, 156, 255});
            DrawLine((int)(center.x + 7.0f * zoom), (int)(center.y - 22.0f * zoom), (int)center.x, (int)(center.y - 15.0f * zoom), Color{120, 144, 156, 255});
            break;
        }

        case SCENERY_LAMP_POST: {
            DrawEllipse((int)center.x, (int)center.y, 5.0f * zoom, 3.0f * zoom, Color{0, 0, 0, 60});
            DrawLineEx({center.x, center.y}, {center.x, center.y - 22.0f * zoom}, 2.0f * zoom, Color{38, 50, 56, 255});
            DrawCircle((int)center.x, (int)(center.y - 22.0f * zoom), 3.5f * zoom, Color{255, 235, 59, 220});
            DrawCircleGradient(Vector2{center.x, center.y - 22.0f * zoom}, 12.0f * zoom, Color{255, 235, 59, 60}, Color{255, 235, 59, 0});
            break;
        }

        case SCENERY_FOUNTAIN: {
            DrawEllipse((int)center.x, (int)center.y, 15.0f * zoom, 8.0f * zoom, Color{158, 158, 158, 255}); // Stone rim
            DrawEllipse((int)center.x, (int)center.y, 11.0f * zoom, 6.0f * zoom, Color{33, 150, 243, 230});  // Water pool

            // Splashing jet
            float jetH = (14.0f + sinf((float)GetTime() * 8.0f) * 3.0f) * zoom;
            DrawLineEx({center.x, center.y}, {center.x, center.y - jetH}, 2.5f * zoom, Color{227, 242, 253, 255});
            DrawCircle((int)center.x, (int)(center.y - jetH), 3.0f * zoom, Color{255, 255, 255, 240});
            DrawCircle((int)(center.x - 3.0f * zoom), (int)(center.y - jetH + 4.0f * zoom), 1.8f * zoom, Color{187, 222, 251, 220});
            DrawCircle((int)(center.x + 3.0f * zoom), (int)(center.y - jetH + 4.0f * zoom), 1.8f * zoom, Color{187, 222, 251, 220});
            break;
        }

        case SCENERY_FLOWER_BED: {
            DrawEllipse((int)center.x, (int)center.y, 13.0f * zoom, 7.0f * zoom, Color{109, 76, 65, 255}); // Soil bed
            // Clustered flowers
            DrawCircle((int)(center.x - 5.0f * zoom), (int)(center.y - 2.0f * zoom), 2.5f * zoom, Color{239, 68, 68, 255});  // Red
            DrawCircle((int)(center.x + 4.0f * zoom), (int)(center.y - 3.0f * zoom), 2.5f * zoom, Color{253, 216, 53, 255}); // Yellow
            DrawCircle((int)center.x, (int)(center.y - 4.0f * zoom), 2.8f * zoom, Color{171, 71, 188, 255});                  // Purple
            DrawCircle((int)(center.x + 1.0f * zoom), (int)(center.y + 1.0f * zoom), 2.2f * zoom, Color{255, 112, 67, 255}); // Coral
            break;
        }

        default: break;
    }
}

void DrawStaff(const StaffMember& staff, Vector2 camOffset, float zoom) {
    Vector2 sPos = GridToScreen(staff.pos.x, staff.pos.y, 0.0f, camOffset, zoom);
    float bob = sinf(staff.walkTimer * 8.0f) * 1.5f * zoom;

    // Shadow
    DrawEllipse((int)sPos.x, (int)sPos.y, 4.5f * zoom, 2.2f * zoom, Color{0, 0, 0, 80});

    float bodyH = 11.0f * zoom;
    float bodyW = 5.5f * zoom;

    if (staff.type == STAFF_HANDYMAN) {
        // Blue overalls
        DrawRectangle((int)(sPos.x - bodyW * 0.4f), (int)(sPos.y - bodyH * 0.45f + bob), (int)(bodyW * 0.8f), (int)(bodyH * 0.45f), Color{25, 118, 210, 255});
        DrawRectangle((int)(sPos.x - bodyW / 2.0f), (int)(sPos.y - bodyH + bob), (int)bodyW, (int)(bodyH * 0.6f), Color{25, 118, 210, 255});
        // Head + Straw hat
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 3.0f * zoom + bob), 2.8f * zoom, Color{255, 224, 178, 255});
        DrawEllipse((int)sPos.x, (int)(sPos.y - bodyH - 4.5f * zoom + bob), 4.5f * zoom, 2.0f * zoom, Color{255, 213, 79, 255});

        // Broom
        float sweepAngle = sinf(staff.walkTimer * 6.0f) * 20.0f;
        Vector2 broomEnd = { sPos.x + 6.0f * zoom + sweepAngle * 0.2f, sPos.y - 1.0f * zoom };
        DrawLineEx({sPos.x, sPos.y - bodyH * 0.5f + bob}, broomEnd, 1.5f * zoom, Color{141, 110, 99, 255});
        DrawRectanglePro(Rectangle{broomEnd.x, broomEnd.y, 6.0f * zoom, 3.0f * zoom}, Vector2{3.0f * zoom, 1.5f * zoom}, sweepAngle, Color{215, 204, 200, 255});
    } else {
        // Yellow mechanic jumpsuit
        DrawRectangle((int)(sPos.x - bodyW * 0.4f), (int)(sPos.y - bodyH * 0.45f + bob), (int)(bodyW * 0.8f), (int)(bodyH * 0.45f), Color{245, 124, 0, 255});
        DrawRectangle((int)(sPos.x - bodyW / 2.0f), (int)(sPos.y - bodyH + bob), (int)bodyW, (int)(bodyH * 0.6f), Color{255, 179, 0, 255});
        // Head + Red hardhat
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 3.0f * zoom + bob), 2.8f * zoom, Color{255, 224, 178, 255});
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 4.5f * zoom + bob), 3.0f * zoom, Color{229, 57, 53, 255});

        // Wrench
        DrawLineEx({sPos.x + 3.0f * zoom, sPos.y - bodyH * 0.4f + bob}, {sPos.x + 8.0f * zoom, sPos.y - bodyH * 0.7f + bob}, 2.0f * zoom, Color{176, 190, 197, 255});
    }
}

void DrawMess(const ParkMess& mess, Vector2 camOffset, float zoom) {
    Vector2 sPos = GridToScreen(mess.pos.x, mess.pos.y, 0.0f, camOffset, zoom);

    if (mess.isVomit) {
        // Green vomit puddle with bubbles
        DrawEllipse((int)sPos.x, (int)sPos.y, 7.0f * zoom, 3.5f * zoom, Color{129, 199, 132, 220});
        DrawCircle((int)(sPos.x - 2.0f * zoom), (int)(sPos.y - 1.0f * zoom), 1.5f * zoom, Color{102, 187, 106, 255});
        DrawCircle((int)(sPos.x + 2.5f * zoom), (int)(sPos.y + 0.5f * zoom), 1.2f * zoom, Color{76, 175, 80, 255});
    } else {
        // Crumpled soda cup
        DrawEllipse((int)sPos.x, (int)sPos.y, 4.0f * zoom, 2.0f * zoom, Color{0, 0, 0, 60});
        DrawRectangle((int)(sPos.x - 2.0f * zoom), (int)(sPos.y - 5.0f * zoom), (int)(4.0f * zoom), (int)(5.0f * zoom), Color{239, 83, 80, 255});
        DrawLineEx({sPos.x, sPos.y - 5.0f * zoom}, {sPos.x + 2.0f * zoom, sPos.y - 8.0f * zoom}, 1.2f * zoom, WHITE); // Straw
    }
}

void DrawEntranceArch(Vector2 gridPos, Vector2 camOffset, float zoom) {
    Vector2 leftPillar = GridToScreen(gridPos.x, gridPos.y - 0.6f, 0.0f, camOffset, zoom);
    Vector2 rightPillar = GridToScreen(gridPos.x, gridPos.y + 0.6f, 0.0f, camOffset, zoom);

    float pHeight = 36.0f * zoom;
    float pWidth = 8.0f * zoom;

    // Brick pillars
    DrawRectangle((int)(leftPillar.x - pWidth / 2.0f), (int)(leftPillar.y - pHeight), (int)pWidth, (int)pHeight, Color{121, 85, 72, 255});
    DrawRectangle((int)(rightPillar.x - pWidth / 2.0f), (int)(rightPillar.y - pHeight), (int)pWidth, (int)pHeight, Color{121, 85, 72, 255});

    // Arch beam linking pillars
    Vector2 archLeft = {leftPillar.x, leftPillar.y - pHeight};
    Vector2 archRight = {rightPillar.x, rightPillar.y - pHeight};
    DrawLineEx(archLeft, archRight, 4.0f * zoom, Color{38, 50, 56, 255});

    // Overhead sign board
    Vector2 center = { (archLeft.x + archRight.x) * 0.5f, (archLeft.y + archRight.y) * 0.5f - 8.0f * zoom };
    float signW = 90.0f * zoom;
    float signH = 18.0f * zoom;
    DrawRectangleRounded(Rectangle{center.x - signW / 2.0f, center.y - signH / 2.0f, signW, signH}, 0.2f, 4, Color{234, 88, 12, 255});
    DrawRectangleRoundedLines(Rectangle{center.x - signW / 2.0f, center.y - signH / 2.0f, signW, signH}, 0.2f, 4, Color{255, 214, 0, 255});
    DrawText("COASTER GRID", (int)(center.x - 38.0f * zoom), (int)(center.y - 5.0f * zoom), (int)(8.0f * zoom), WHITE);

    // Welcome flags
    DrawLineEx(archLeft, {archLeft.x, archLeft.y - 12.0f * zoom}, 2.0f * zoom, Color{189, 189, 189, 255});
    DrawTriangle({archLeft.x, archLeft.y - 12.0f * zoom}, {archLeft.x, archLeft.y - 6.0f * zoom}, {archLeft.x + 8.0f * zoom, archLeft.y - 9.0f * zoom}, Color{239, 68, 68, 255});

    DrawLineEx(archRight, {archRight.x, archRight.y - 12.0f * zoom}, 2.0f * zoom, Color{189, 189, 189, 255});
    DrawTriangle({archRight.x, archRight.y - 12.0f * zoom}, {archRight.x, archRight.y - 6.0f * zoom}, {archRight.x + 8.0f * zoom, archRight.y - 9.0f * zoom}, Color{33, 150, 243, 255});
}

} // namespace Iso

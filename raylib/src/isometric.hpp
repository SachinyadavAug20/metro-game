#pragma once

#include "common.hpp"

namespace Iso {
    // Coordinate conversions
    Vector2 GridToScreen(float gx, float gy, float gz, Vector2 camOffset, float zoom);
    Vector2 ScreenToGrid(Vector2 screenPos, Vector2 camOffset, float zoom, float gz = 0.0f);
    
    // Geometry rendering
    void DrawTile(int gx, int gy, int gz, GroundType type, Vector2 camOffset, float zoom, bool hovered = false);
    void DrawPillar(int gx, int gy, int groundZ, int trackZ, Vector2 camOffset, float zoom);
    void DrawTrackBasePillars(float gx, float gy, float trackZ, Vector2 camOffset, float zoom);
    void DrawCursor(int gx, int gy, int gz, Vector2 camOffset, float zoom, Color color);
    void DrawShadow(float gx, float gy, float radius, Vector2 camOffset, float zoom);
    void DrawScenery(int gx, int gy, int gz, SceneryType type, Vector2 camOffset, float zoom);
    void DrawStaff(const StaffMember& staff, Vector2 camOffset, float zoom);
    void DrawMess(const StationMess& mess, Vector2 camOffset, float zoom);
    void DrawTransitPortalArch(Vector2 gridPos, Vector2 camOffset, float zoom);
    void DrawEntranceArch(Vector2 gridPos, Vector2 camOffset, float zoom); // Alias for compatibility
}


#pragma once

#include "common.hpp"

struct StationInfo {
    int gx, gy, gz;
    StationShape shape = SHAPE_CIRCLE;
    std::string name = "Central Station";
    int waitingCount = 0;
    float overcrowdTimer = 20.0f;
    bool isOvercrowded = false;
};

struct TrackNode {
    int gx, gy, gz;
    int endZ;
    TrackType type;
    Direction inDir;
    Direction outDir;
    Color color;

    // Station & Signaling Properties
    StationShape stationShape = SHAPE_NONE;
    std::string stationName = "";
    SignalAspect signalAspect = SIGNAL_GREEN;
    float signalTimer = 0.0f;

    // Precomputed sub-spline points within this single tile in 3D grid space
    std::vector<Vector3> splinePoints;
};

class TrackSystem {
public:
    TrackSystem();

    void InitDefaultCircuit();
    bool AddPiece(int gx, int gy, int gz, TrackType type, Direction inDir, Direction outDir, Color color = Color{229, 57, 53, 255});
    bool RemovePiece(int gx, int gy);
    TrackNode* GetPiece(int gx, int gy);
    const TrackNode* GetPiece(int gx, int gy) const;
    bool HasPiece(int gx, int gy) const;

    void RecalculateCircuit();
    bool IsCircuitClosed() const { return circuitClosed; }
    const std::vector<Vector3>& GetCircuitPath() const { return masterPath; }
    float GetTotalCircuitLength() const { return totalLength; }
    Vector3 GetPointAtDistance(float distance, Vector3* outTangent = nullptr) const;

    // Station references & queries
    bool GetPrimaryStationLocation(int& outGx, int& outGy, int& outGz) const;
    std::vector<StationInfo> GetAllStations() const;
    int GetStationCount() const;
    const TrackNode* GetStationAt(int gx, int gy) const;
    StationShape GetStationShapeAt(int gx, int gy) const;

    // Dynamic Wayside Signaling
    void UpdateSignals(float trainDistance);
    SignalAspect GetActiveSignalAspect() const { return masterSignalAspect; }

    // Track Styling & Customization
    void SetTrackColor(Color c);
    Color GetTrackColor() const { return lineThemeColor; }

    // Drawing
    void DrawAllTracks(Vector2 camOffset, float zoom);
    void DrawGhostPiece(int gx, int gy, int gz, TrackType type, Direction inDir, Direction outDir, Vector2 camOffset, float zoom, bool canPlace);

    // Transit statistics
    float GetTrackLengthM() const { return totalLength * 28.0f; } // Scaled in meters
    int GetTrackCount() const { return (int)pieces.size(); }
    int GetSignalCount() const;

private:
    std::vector<TrackNode> pieces;
    bool circuitClosed = false;
    std::vector<Vector3> masterPath;
    std::vector<float> pathDistances;
    float totalLength = 0.0f;
    Color lineThemeColor = Color{229, 57, 53, 255}; // Tokyo Red
    SignalAspect masterSignalAspect = SIGNAL_GREEN;

    void GenerateTileSpline(TrackNode& node);
    void DrawSinglePiece(const TrackNode& node, Vector2 camOffset, float zoom);
    void DrawBallastBed(const TrackNode& node, Vector2 camOffset, float zoom);
    void DrawThirdRail(const TrackNode& node, Vector2 camOffset, float zoom);
    void DrawPlatformCanopy(const TrackNode& node, Vector2 camOffset, float zoom);
    void DrawSignalMast(const TrackNode& node, Vector2 camOffset, float zoom);
    void DrawTunnelPortal(const TrackNode& node, Vector2 camOffset, float zoom);
};


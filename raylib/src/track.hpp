#pragma once

#include "common.hpp"

struct TrackNode {
    int gx, gy, gz;
    int endZ;
    TrackType type;
    Direction inDir;
    Direction outDir;
    Color color;

    // Precomputed sub-spline points within this single tile in 3D grid space
    std::vector<Vector3> splinePoints;
};

class TrackSystem {
public:
    TrackSystem();

    void InitDefaultCircuit();
    bool AddPiece(int gx, int gy, int gz, TrackType type, Direction inDir, Direction outDir, Color color = Color{239, 83, 80, 255});
    bool RemovePiece(int gx, int gy);
    const TrackNode* GetPiece(int gx, int gy) const;
    bool HasPiece(int gx, int gy) const;

    void RecalculateCircuit();
    bool IsCircuitClosed() const { return circuitClosed; }
    const std::vector<Vector3>& GetCircuitPath() const { return masterPath; }
    float GetTotalCircuitLength() const { return totalLength; }
    Vector3 GetPointAtDistance(float distance, Vector3* outTangent = nullptr) const;

    // Station references
    bool GetStationLocation(int& outGx, int& outGy, int& outGz) const;

    // Track Styling
    void SetTrackColor(Color c);

    // Drawing
    void DrawAllTracks(Vector2 camOffset, float zoom);
    void DrawGhostPiece(int gx, int gy, int gz, TrackType type, Direction inDir, Direction outDir, Vector2 camOffset, float zoom, bool canPlace);

    // Track statistics
    float GetMaxDrop() const { return maxDropHeight; }
    int GetInversionCount() const { return inversionCount; }
    int GetTrackCount() const { return (int)pieces.size(); }

private:
    std::vector<TrackNode> pieces;
    bool circuitClosed = false;
    std::vector<Vector3> masterPath;
    std::vector<float> pathDistances;
    float totalLength = 0.0f;
    float maxDropHeight = 0.0f;
    int inversionCount = 0;

    void GenerateTileSpline(TrackNode& node);
    void DrawSinglePiece(const TrackNode& node, Vector2 camOffset, float zoom);
};

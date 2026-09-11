#include "track.hpp"
#include "isometric.hpp"
#include <sstream>

TrackSystem::TrackSystem() {
    InitDefaultCircuit();
}

void TrackSystem::InitDefaultCircuit() {
    pieces.clear();
    Color lineCol = Color{229, 57, 53, 255}; // Line 1 Tokyo Red

    // 34-Tile Scenic Metropolitan Transit Circuit:
    // Spacious, high-speed circuit connecting Central Hub, Marina Viaduct,
    // University Med Center, and Suburban Terminal with elevated SkyTrain viaduct,
    // concrete river bridge, tunnels, and block signaling!

    // 1. Turn East at (6, 9)
    AddPiece(6, 9, 0, TRACK_CURVE_RIGHT, DIR_SOUTH, DIR_EAST, lineCol);

    // 2. Central Station Platform [Square - Financial CBD] at (7, 9)
    AddPiece(7, 9, 0, TRACK_STATION, DIR_WEST, DIR_EAST, lineCol);
    if (TrackNode* n = GetPiece(7, 9)) {
        n->stationShape = SHAPE_SQUARE;
        n->stationName = "Central Hub [Square]";
    }

    // 3. Straight acceleration stretch
    AddPiece(8, 9, 0, TRACK_STRAIGHT, DIR_WEST, DIR_EAST, lineCol);

    // 4. Block Signal Mast
    AddPiece(9, 9, 0, TRACK_SIGNAL, DIR_WEST, DIR_EAST, lineCol);

    // 5. Viaduct Incline Ramp (Z=0 -> Z=1)
    AddPiece(10, 9, 0, TRACK_VIADUCT_ELEVATED, DIR_WEST, DIR_EAST, lineCol);

    // 6. Elevated Viaduct Straight (Z=1)
    AddPiece(11, 9, 1, TRACK_STRAIGHT, DIR_WEST, DIR_EAST, lineCol);

    // 7. Elevated Bridge over Canal (Z=1)
    AddPiece(12, 9, 1, TRACK_STRAIGHT, DIR_WEST, DIR_EAST, lineCol);
    AddPiece(13, 9, 1, TRACK_STRAIGHT, DIR_WEST, DIR_EAST, lineCol);

    // 8. Elevated Approach (Z=1)
    AddPiece(14, 9, 1, TRACK_STRAIGHT, DIR_WEST, DIR_EAST, lineCol);

    // 9. Marina Viaduct Station [Triangle - Commercial Waterfront] (Z=1 Elevated)
    AddPiece(15, 9, 1, TRACK_STATION, DIR_WEST, DIR_EAST, lineCol);
    if (TrackNode* n = GetPiece(15, 9)) {
        n->stationShape = SHAPE_TRIANGLE;
        n->stationName = "Marina Viaduct [Triangle]";
    }

    // 10. Elevated Turn South (Z=1)
    AddPiece(16, 9, 1, TRACK_CURVE_RIGHT, DIR_WEST, DIR_SOUTH, lineCol);

    // 11. Eastern Elevated Run
    AddPiece(16, 10, 1, TRACK_STRAIGHT, DIR_NORTH, DIR_SOUTH, lineCol);
    AddPiece(16, 11, 1, TRACK_SIGNAL, DIR_NORTH, DIR_SOUTH, lineCol);
    AddPiece(16, 12, 1, TRACK_STRAIGHT, DIR_NORTH, DIR_SOUTH, lineCol);

    // 12. Viaduct Ramp Descent (Z=1 -> Z=0)
    AddPiece(16, 13, 1, TRACK_VIADUCT_SLOPE, DIR_NORTH, DIR_SOUTH, lineCol);

    // 13. Ground Straight Run
    AddPiece(16, 14, 0, TRACK_STRAIGHT, DIR_NORTH, DIR_SOUTH, lineCol);
    AddPiece(16, 15, 0, TRACK_STRAIGHT, DIR_NORTH, DIR_SOUTH, lineCol);

    // 14. Turn West
    AddPiece(16, 16, 0, TRACK_CURVE_RIGHT, DIR_NORTH, DIR_WEST, lineCol);

    // 15. Southern Concourse Run
    AddPiece(15, 16, 0, TRACK_STRAIGHT, DIR_EAST, DIR_WEST, lineCol);

    // 16. Underground Subway Tunnel Entry
    AddPiece(14, 16, 0, TRACK_TUNNEL_PORTAL, DIR_EAST, DIR_WEST, lineCol);
    AddPiece(13, 16, 0, TRACK_STRAIGHT, DIR_EAST, DIR_WEST, lineCol);
    AddPiece(12, 16, 0, TRACK_STRAIGHT, DIR_EAST, DIR_WEST, lineCol);
    AddPiece(11, 16, 0, TRACK_TUNNEL_PORTAL, DIR_EAST, DIR_WEST, lineCol);

    // 17. University Med Center Station [Cross - Hospital & Campus]
    AddPiece(10, 16, 0, TRACK_STATION, DIR_EAST, DIR_WEST, lineCol);
    if (TrackNode* n = GetPiece(10, 16)) {
        n->stationShape = SHAPE_CROSS;
        n->stationName = "University Med [Cross]";
    }

    // 18. Wayside Signal Mast
    AddPiece(9, 16, 0, TRACK_SIGNAL, DIR_EAST, DIR_WEST, lineCol);
    AddPiece(8, 16, 0, TRACK_STRAIGHT, DIR_EAST, DIR_WEST, lineCol);
    AddPiece(7, 16, 0, TRACK_STRAIGHT, DIR_EAST, DIR_WEST, lineCol);

    // 19. Turn North
    AddPiece(6, 16, 0, TRACK_CURVE_RIGHT, DIR_EAST, DIR_NORTH, lineCol);

    // 20. Western Parkside Run & Suburban Terminal [Circle - Residential Suburbs]
    AddPiece(6, 15, 0, TRACK_STRAIGHT, DIR_SOUTH, DIR_NORTH, lineCol);
    AddPiece(6, 14, 0, TRACK_STATION, DIR_SOUTH, DIR_NORTH, lineCol);
    if (TrackNode* n = GetPiece(6, 14)) {
        n->stationShape = SHAPE_CIRCLE;
        n->stationName = "Suburban Heights [Circle]";
    }
    AddPiece(6, 13, 0, TRACK_SIGNAL, DIR_SOUTH, DIR_NORTH, lineCol);
    AddPiece(6, 12, 0, TRACK_STRAIGHT, DIR_SOUTH, DIR_NORTH, lineCol);
    AddPiece(6, 11, 0, TRACK_STRAIGHT, DIR_SOUTH, DIR_NORTH, lineCol);
    AddPiece(6, 10, 0, TRACK_STRAIGHT, DIR_SOUTH, DIR_NORTH, lineCol);

    RecalculateCircuit();
}

void TrackSystem::SetTrackColor(Color c) {
    lineThemeColor = c;
    for (auto& p : pieces) {
        p.color = c;
    }
}

bool TrackSystem::AddPiece(int gx, int gy, int gz, TrackType type, Direction inDir, Direction outDir, Color color) {
    RemovePiece(gx, gy);

    TrackNode node;
    node.gx = gx;
    node.gy = gy;
    node.gz = gz;
    node.type = type;
    node.inDir = inDir;
    node.outDir = outDir;
    node.color = color;

    // Elevation delta calculation
    if (type == TRACK_VIADUCT_ELEVATED) {
        node.endZ = gz + 1;
    } else if (type == TRACK_VIADUCT_SLOPE) {
        node.endZ = std::max(0, gz - 1);
    } else {
        node.endZ = gz;
    }

    GenerateTileSpline(node);
    pieces.push_back(node);
    RecalculateCircuit();
    return true;
}

bool TrackSystem::RemovePiece(int gx, int gy) {
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (pieces[i].gx == gx && pieces[i].gy == gy) {
            pieces.erase(pieces.begin() + i);
            RecalculateCircuit();
            return true;
        }
    }
    return false;
}

TrackNode* TrackSystem::GetPiece(int gx, int gy) {
    for (auto& p : pieces) {
        if (p.gx == gx && p.gy == gy) return &p;
    }
    return nullptr;
}

const TrackNode* TrackSystem::GetPiece(int gx, int gy) const {
    for (const auto& p : pieces) {
        if (p.gx == gx && p.gy == gy) return &p;
    }
    return nullptr;
}

bool TrackSystem::HasPiece(int gx, int gy) const {
    return GetPiece(gx, gy) != nullptr;
}

void TrackSystem::GenerateTileSpline(TrackNode& node) {
    node.splinePoints.clear();
    const int SUBDIVS = 14;

    Vector2 inOff = GetDirectionOffset(node.inDir);
    Vector2 outOff = GetDirectionOffset(node.outDir);

    // Entry point on tile border
    Vector3 pStart = {
        (float)node.gx + 0.5f + inOff.x * 0.5f,
        (float)node.gy + 0.5f + inOff.y * 0.5f,
        (float)node.gz
    };

    // Center of tile
    Vector3 pMid = {
        (float)node.gx + 0.5f,
        (float)node.gy + 0.5f,
        (float)(node.gz + node.endZ) * 0.5f
    };

    // Exit point on tile border
    Vector3 pEnd = {
        (float)node.gx + 0.5f + outOff.x * 0.5f,
        (float)node.gy + 0.5f + outOff.y * 0.5f,
        (float)node.endZ
    };

    // Quadratic Bezier interpolation
    for (int i = 0; i <= SUBDIVS; ++i) {
        float t = (float)i / SUBDIVS;
        float u = 1.0f - t;
        Vector3 pt = {
            u * u * pStart.x + 2.0f * u * t * pMid.x + t * t * pEnd.x,
            u * u * pStart.y + 2.0f * u * t * pMid.y + t * t * pEnd.y,
            u * u * pStart.z + 2.0f * u * t * pMid.z + t * t * pEnd.z
        };
        node.splinePoints.push_back(pt);
    }
}

void TrackSystem::RecalculateCircuit() {
    masterPath.clear();
    pathDistances.clear();
    circuitClosed = false;
    totalLength = 0.0f;

    if (pieces.empty()) return;

    // Find first station as start
    const TrackNode* startNode = nullptr;
    for (const auto& p : pieces) {
        if (p.type == TRACK_STATION) {
            startNode = &p;
            break;
        }
    }
    if (!startNode) startNode = &pieces[0];

    std::vector<const TrackNode*> chain;
    const TrackNode* current = startNode;
    chain.push_back(current);

    while (true) {
        Vector2 outOff = GetDirectionOffset(current->outDir);
        int nextGx = current->gx + (int)roundf(outOff.x);
        int nextGy = current->gy + (int)roundf(outOff.y);

        if (nextGx == startNode->gx && nextGy == startNode->gy) {
            circuitClosed = true;
            break;
        }

        const TrackNode* nextPiece = GetPiece(nextGx, nextGy);
        if (!nextPiece) break;

        Direction expectedIn = GetOppositeDir(current->outDir);
        if (nextPiece->inDir != expectedIn) break;

        // Prevent infinite loops on broken circuits
        bool alreadyInChain = false;
        for (const auto* visited : chain) {
            if (visited == nextPiece) {
                alreadyInChain = true;
                break;
            }
        }
        if (alreadyInChain) break;

        chain.push_back(nextPiece);
        current = nextPiece;
    }

    if (circuitClosed) {
        for (const auto* node : chain) {
            for (size_t i = 0; i < node->splinePoints.size() - 1; ++i) {
                masterPath.push_back(node->splinePoints[i]);
            }
        }

        pathDistances.resize(masterPath.size(), 0.0f);
        totalLength = 0.0f;
        for (size_t i = 1; i < masterPath.size(); ++i) {
            totalLength += Vector3Distance(masterPath[i - 1], masterPath[i]);
            pathDistances[i] = totalLength;
        }

        // Precompute exact circuit distance along spline for each chain node
        int curPtIdx = 0;
        for (auto* node : chain) {
            if (curPtIdx < (int)pathDistances.size()) {
                const_cast<TrackNode*>(node)->circuitDist = pathDistances[curPtIdx];
            }
            curPtIdx += (int)node->splinePoints.size() - 1;
        }
    }
}

Vector3 TrackSystem::GetPointAtDistance(float distance, Vector3* outTangent) const {
    if (masterPath.empty()) return {0, 0, 0};
    if (masterPath.size() == 1 || totalLength <= 0.001f) {
        if (outTangent) *outTangent = {1, 0, 0};
        return masterPath[0];
    }

    float d = fmodf(distance, totalLength);
    if (d < 0.0f) d += totalLength;

    auto it = std::lower_bound(pathDistances.begin(), pathDistances.end(), d);
    int idx = (int)(it - pathDistances.begin());
    if (idx <= 0) idx = 1;
    if (idx >= (int)masterPath.size()) idx = (int)masterPath.size() - 1;

    float d0 = pathDistances[idx - 1];
    float d1 = pathDistances[idx];
    float segLen = d1 - d0;
    float t = (segLen > 0.0001f) ? (d - d0) / segLen : 0.0f;

    Vector3 p0 = masterPath[idx - 1];
    Vector3 p1 = masterPath[idx];

    if (outTangent) {
        *outTangent = Vector3Normalize(Vector3Subtract(p1, p0));
    }

    return Vector3Lerp(p0, p1, t);
}

bool TrackSystem::GetPrimaryStationLocation(int& outGx, int& outGy, int& outGz) const {
    for (const auto& p : pieces) {
        if (p.type == TRACK_STATION) {
            outGx = p.gx;
            outGy = p.gy;
            outGz = p.gz;
            return true;
        }
    }
    return false;
}

std::vector<StationInfo> TrackSystem::GetAllStations() const {
    std::vector<StationInfo> stations;
    for (const auto& p : pieces) {
        if (p.type == TRACK_STATION) {
            StationInfo info;
            info.gx = p.gx;
            info.gy = p.gy;
            info.gz = p.gz;
            info.shape = p.stationShape;
            info.name = p.stationName.empty() ? GetShapeName(p.stationShape) : p.stationName;
            info.stationLevel = p.stationLevel;
            info.passengersServed = p.passengersServed;
            info.circuitDist = p.circuitDist;
            stations.push_back(info);
        }
    }
    return stations;
}

void TrackSystem::RecordStationAlight(int gx, int gy, int count, bool& outLeveledUp, int& outNewLevel) {
    outLeveledUp = false;
    TrackNode* n = GetPiece(gx, gy);
    if (!n || n->type != TRACK_STATION) return;

    n->passengersServed += count;
    int oldLevel = n->stationLevel;
    if (n->passengersServed >= 35) n->stationLevel = 3;
    else if (n->passengersServed >= 12) n->stationLevel = 2;
    else n->stationLevel = 1;

    if (n->stationLevel > oldLevel) {
        outLeveledUp = true;
        outNewLevel = n->stationLevel;
    }
}

bool TrackSystem::GetNextStationAhead(float currentDist, float& outDistToStation, StationInfo& outStation) const {
    std::vector<StationInfo> stations = GetAllStations();
    if (stations.empty() || totalLength <= 0.001f) return false;

    float bestDist = 999999.0f;
    int bestIdx = -1;

    for (size_t i = 0; i < stations.size(); ++i) {
        float d = stations[i].circuitDist - currentDist;
        while (d < 0.0f) d += totalLength;
        d = fmodf(d, totalLength);
        if (d < bestDist) {
            bestDist = d;
            bestIdx = (int)i;
        }
    }

    if (bestIdx != -1) {
        outDistToStation = bestDist;
        outStation = stations[bestIdx];
        return true;
    }
    return false;
}

int TrackSystem::GetStationCount() const {
    int count = 0;
    for (const auto& p : pieces) {
        if (p.type == TRACK_STATION) count++;
    }
    return count;
}

const TrackNode* TrackSystem::GetStationAt(int gx, int gy) const {
    for (const auto& p : pieces) {
        if (p.type == TRACK_STATION && p.gx == gx && p.gy == gy) return &p;
    }
    return nullptr;
}

StationShape TrackSystem::GetStationShapeAt(int gx, int gy) const {
    const TrackNode* s = GetStationAt(gx, gy);
    return s ? s->stationShape : SHAPE_NONE;
}

int TrackSystem::GetSignalCount() const {
    int count = 0;
    for (const auto& p : pieces) {
        if (p.type == TRACK_SIGNAL) count++;
    }
    return count;
}

void TrackSystem::UpdateSignals(float trainDistance) {
    if (masterPath.empty() || totalLength <= 0.001f) return;

    masterSignalAspect = SIGNAL_GREEN;

    for (auto& p : pieces) {
        if (p.type == TRACK_SIGNAL) {
            float forwardDist = trainDistance - p.circuitDist;
            while (forwardDist < 0.0f) forwardDist += totalLength;
            forwardDist = fmodf(forwardDist, totalLength);

            // If train is in the block immediately past the signal (0 to 3.5 units ahead)
            if (forwardDist >= 0.0f && forwardDist < 3.5f) {
                p.signalAspect = SIGNAL_RED;
            } else if (forwardDist >= 3.5f && forwardDist < 7.0f) {
                p.signalAspect = SIGNAL_AMBER;
            } else {
                p.signalAspect = SIGNAL_GREEN;
            }

            if (p.signalAspect == SIGNAL_RED) masterSignalAspect = SIGNAL_RED;
            else if (p.signalAspect == SIGNAL_AMBER && masterSignalAspect != SIGNAL_RED) masterSignalAspect = SIGNAL_AMBER;
        }
    }
}

void TrackSystem::DrawAllTracks(Vector2 camOffset, float zoom) {
    // 1. Draw Support Viaduct Pillars
    for (const auto& p : pieces) {
        if (p.gz > 0 || p.endZ > 0) {
            Iso::DrawPillar(p.gx, p.gy, 0, std::max(p.gz, p.endZ), camOffset, zoom);
        }
    }

    // 2. Draw Track Rails, Sleepers, 3rd Rail, Stations, Signals
    for (const auto& p : pieces) {
        DrawSinglePiece(p, camOffset, zoom);
    }
}

void TrackSystem::DrawBallastBed(const TrackNode& node, Vector2 camOffset, float zoom) {
    if (node.splinePoints.size() < 2) return;
    float bedWidth = 8.0f * zoom;
    Color ballastColor = Color{55, 65, 81, 230}; // Dark crushed granite

    for (size_t i = 0; i < node.splinePoints.size() - 1; i += 2) {
        Vector3 p1 = node.splinePoints[i];
        Vector3 p2 = node.splinePoints[i + 1];
        Vector2 s1 = Iso::GridToScreen(p1.x, p1.y, p1.z, camOffset, zoom);
        Vector2 s2 = Iso::GridToScreen(p2.x, p2.y, p2.z, camOffset, zoom);

        Vector2 tangent = Vector2Normalize({s2.x - s1.x, s2.y - s1.y});
        Vector2 normal = {-tangent.y, tangent.x};

        Vector2 b1 = {s1.x - normal.x * bedWidth, s1.y - normal.y * bedWidth};
        Vector2 b2 = {s1.x + normal.x * bedWidth, s1.y + normal.y * bedWidth};
        Vector2 b3 = {s2.x + normal.x * bedWidth, s2.y + normal.y * bedWidth};
        Vector2 b4 = {s2.x - normal.x * bedWidth, s2.y - normal.y * bedWidth};

        DrawTriangle(b1, b2, b3, ballastColor);
        DrawTriangle(b1, b3, b4, ballastColor);
    }
}

void TrackSystem::DrawThirdRail(const TrackNode& node, Vector2 camOffset, float zoom) {
    if (node.splinePoints.size() < 2) return;
    float thirdRailOffset = 6.2f * zoom;
    Color thirdRailC = Color{217, 119, 6, 255}; // Amber insulated 3rd rail

    for (size_t i = 0; i < node.splinePoints.size() - 1; ++i) {
        Vector3 p1 = node.splinePoints[i];
        Vector3 p2 = node.splinePoints[i + 1];
        Vector2 s1 = Iso::GridToScreen(p1.x, p1.y, p1.z, camOffset, zoom);
        Vector2 s2 = Iso::GridToScreen(p2.x, p2.y, p2.z, camOffset, zoom);

        Vector2 tangent = Vector2Normalize({s2.x - s1.x, s2.y - s1.y});
        Vector2 normal = {-tangent.y, tangent.x};

        Vector2 r1 = {s1.x + normal.x * thirdRailOffset, s1.y + normal.y * thirdRailOffset};
        Vector2 r2 = {s2.x + normal.x * thirdRailOffset, s2.y + normal.y * thirdRailOffset};

        DrawLineEx(r1, r2, std::max(1.2f, 1.8f * zoom), thirdRailC);

        // Porcelain insulator blocks
        if (i % 4 == 0) {
            DrawRectangle((int)(r1.x - 1.5f * zoom), (int)(r1.y - 1.5f * zoom), (int)(3.0f * zoom), (int)(3.0f * zoom), Color{243, 244, 246, 255});
        }
    }
}

void TrackSystem::DrawPlatformCanopy(const TrackNode& node, Vector2 camOffset, float zoom) {
    Vector2 top = Iso::GridToScreen((float)node.gx, (float)node.gy, (float)node.gz, camOffset, zoom);
    Vector2 right = Iso::GridToScreen((float)(node.gx + 1), (float)node.gy, (float)node.gz, camOffset, zoom);
    Vector2 bottom = Iso::GridToScreen((float)(node.gx + 1), (float)(node.gy + 1), (float)node.gz, camOffset, zoom);
    Vector2 left = Iso::GridToScreen((float)node.gx, (float)(node.gy + 1), (float)node.gz, camOffset, zoom);

    // 1. Polished Granite Platform Floor
    Color floorTop = (node.stationLevel >= 3) ? Color{248, 250, 252, 255} : ((node.stationLevel == 2) ? Color{226, 232, 240, 255} : Color{203, 213, 225, 255});
    DrawTriangle(top, left, bottom, floorTop);
    DrawTriangle(top, bottom, right, Color{148, 163, 184, 255});

    // 2. High-Vis Yellow Tactile Safety Warning Edge with Rubber Studs
    DrawLineEx(top, right, 2.8f * zoom, Color{250, 204, 21, 255});
    DrawLineEx(bottom, left, 2.8f * zoom, Color{250, 204, 21, 255});

    // 3. Modern Glass Canopy Roof Structure
    float roofH = (node.stationLevel >= 3 ? 30.0f : (node.stationLevel == 2 ? 26.0f : 22.0f)) * zoom;
    Vector2 rTop = {top.x, top.y - roofH};
    Vector2 rRight = {right.x, right.y - roofH};
    Vector2 rBottom = {bottom.x, bottom.y - roofH};
    Vector2 rLeft = {left.x, left.y - roofH};

    // Architectural Steel Columns supporting canopy
    Color pillarC = (node.stationLevel >= 3) ? Color{245, 158, 11, 255} : Color{71, 85, 105, 255};
    DrawLineEx(left, rLeft, 2.2f * zoom, pillarC);
    DrawLineEx(right, rRight, 2.2f * zoom, pillarC);

    // Glass Canopy Pane (High-transparency tinted cyan glass)
    DrawTriangle(rTop, rLeft, rBottom, Color{56, 189, 248, 85});
    DrawTriangle(rTop, rBottom, rRight, Color{56, 189, 248, 85});
    DrawLineEx(rTop, rRight, 1.8f * zoom, Color{224, 242, 254, 220});
    DrawLineEx(rRight, rBottom, 1.8f * zoom, Color{224, 242, 254, 220});
    DrawLineEx(rBottom, rLeft, 1.8f * zoom, Color{224, 242, 254, 220});
    DrawLineEx(rLeft, rTop, 1.8f * zoom, Color{224, 242, 254, 220});

    // 4. Platform Screen Doors (PSDs) along platform edge
    Vector2 midEdgeL = { (top.x + left.x) * 0.5f, (top.y + left.y) * 0.5f };
    float psdH = 9.5f * zoom;
    DrawRectangle((int)(midEdgeL.x - 5.0f * zoom), (int)(midEdgeL.y - psdH), (int)(10.0f * zoom), (int)psdH, Color{14, 165, 233, 175});
    DrawCircle((int)midEdgeL.x, (int)(midEdgeL.y - psdH), 2.2f * zoom, Color{74, 222, 128, 255}); // Green boarding indicator LED

    // 5. Dynamic LED Passenger Information Display (PIDS)
    Vector2 pidsCenter = { (rTop.x + rBottom.x) * 0.5f, (rTop.y + rBottom.y) * 0.5f + 4.0f * zoom };
    float pidsW = 46.0f * zoom;
    float pidsH = 10.0f * zoom;
    DrawRectangleRounded(Rectangle{pidsCenter.x - pidsW * 0.5f, pidsCenter.y - pidsH * 0.5f, pidsW, pidsH}, 0.3f, 4, Color{15, 23, 42, 235});
    DrawRectangleRoundedLines(Rectangle{pidsCenter.x - pidsW * 0.5f, pidsCenter.y - pidsH * 0.5f, pidsW, pidsH}, 0.3f, 4, Color{56, 189, 248, 190});
    DrawCircle((int)(pidsCenter.x - 14.0f * zoom), (int)pidsCenter.y, 2.0f * zoom, Color{74, 222, 128, 255});
    DrawText("METRO", (int)(pidsCenter.x - 9.0f * zoom), (int)(pidsCenter.y - 3.5f * zoom), (int)(7.0f * zoom), Color{241, 245, 249, 240});

    // 6. Prominent Floating Holographic District Shape Roundel
    if (node.stationShape != SHAPE_NONE) {
        Color shapeCol = GetShapeColor(node.stationShape);
        Vector2 rCenter = { (rTop.x + rBottom.x) * 0.5f, rTop.y - 18.0f * zoom };
        float badgeRadius = 12.0f * zoom;

        // Radiant glow bloom
        DrawCircleGradient(rCenter, badgeRadius * 2.2f, Color{shapeCol.r, shapeCol.g, shapeCol.b, 140}, Color{shapeCol.r, shapeCol.g, shapeCol.b, 0});
        // Dark circular backdrop
        DrawCircle((int)rCenter.x, (int)rCenter.y, badgeRadius, Color{15, 23, 42, 245});
        DrawCircleLines((int)rCenter.x, (int)rCenter.y, badgeRadius, shapeCol);

        // Crisp White Shape Glyph
        if (node.stationShape == SHAPE_SQUARE) {
            DrawRectangle((int)(rCenter.x - 4.2f * zoom), (int)(rCenter.y - 4.2f * zoom), (int)(8.4f * zoom), (int)(8.4f * zoom), WHITE);
        } else if (node.stationShape == SHAPE_TRIANGLE) {
            DrawTriangle({rCenter.x, rCenter.y - 5.5f * zoom}, {rCenter.x - 5.5f * zoom, rCenter.y + 4.5f * zoom}, {rCenter.x + 5.5f * zoom, rCenter.y + 4.5f * zoom}, WHITE);
        } else if (node.stationShape == SHAPE_CROSS) {
            DrawRectangle((int)(rCenter.x - 2.0f * zoom), (int)(rCenter.y - 5.5f * zoom), (int)(4.0f * zoom), (int)(11.0f * zoom), WHITE);
            DrawRectangle((int)(rCenter.x - 5.5f * zoom), (int)(rCenter.y - 2.0f * zoom), (int)(11.0f * zoom), (int)(4.0f * zoom), WHITE);
        } else if (node.stationShape == SHAPE_CIRCLE) {
            DrawCircle((int)rCenter.x, (int)rCenter.y, 4.8f * zoom, WHITE);
            DrawCircle((int)rCenter.x, (int)rCenter.y, 2.6f * zoom, Color{15, 23, 42, 245});
        }

        // Station District Name Banner & Level Stars
        std::string label = node.stationName.empty() ? GetShapeName(node.stationShape) : node.stationName;
        int textW = MeasureText(label.c_str(), 9);
        float pillW = (float)textW + 14.0f * zoom;
        float pillH = 14.0f * zoom;
        Vector2 pillPos = { rCenter.x - pillW * 0.5f, rCenter.y + badgeRadius + 2.0f * zoom };

        DrawRectangleRounded(Rectangle{pillPos.x, pillPos.y, pillW, pillH}, 0.35f, 4, Color{15, 23, 42, 240});
        DrawRectangleRoundedLines(Rectangle{pillPos.x, pillPos.y, pillW, pillH}, 0.35f, 4, shapeCol);
        DrawText(label.c_str(), (int)(pillPos.x + 7.0f * zoom), (int)(pillPos.y + 2.5f * zoom), (int)(8.5f * zoom), WHITE);

        // Level Star Tag
        const char* lvTag = (node.stationLevel >= 3) ? "★★★ LV 3" : ((node.stationLevel == 2) ? "★★ LV 2" : "★ LV 1");
        Color starColor = (node.stationLevel >= 3) ? Color{250, 204, 21, 255} : ((node.stationLevel == 2) ? Color{56, 189, 248, 255} : Color{148, 163, 184, 255});
        DrawText(lvTag, (int)(rCenter.x - 16.0f * zoom), (int)(rCenter.y - badgeRadius - 10.0f * zoom), (int)(8.0f * zoom), starColor);
    }
}

void TrackSystem::DrawSignalMast(const TrackNode& node, Vector2 camOffset, float zoom) {
    Vector2 center = Iso::GridToScreen((float)node.gx + 0.8f, (float)node.gy + 0.2f, (float)node.gz, camOffset, zoom);
    float mastH = 24.0f * zoom;

    // Steel Mast Post
    DrawLineEx(center, {center.x, center.y - mastH}, 2.0f * zoom, Color{51, 65, 85, 255});

    // Black Target Plate
    float plateW = 8.0f * zoom;
    float plateH = 16.0f * zoom;
    Vector2 plateCenter = {center.x, center.y - mastH};
    DrawRectangleRounded(Rectangle{plateCenter.x - plateW * 0.5f, plateCenter.y - plateH * 0.5f, plateW, plateH}, 0.4f, 4, Color{15, 23, 42, 255});

    // 3 Lenses: Top (Red), Mid (Amber), Bottom (Green)
    Vector2 redPos   = {plateCenter.x, plateCenter.y - 4.5f * zoom};
    Vector2 amberPos = {plateCenter.x, plateCenter.y};
    Vector2 greenPos = {plateCenter.x, plateCenter.y + 4.5f * zoom};

    // Base dark lenses
    DrawCircle((int)redPos.x, (int)redPos.y, 2.0f * zoom, Color{69, 10, 10, 255});
    DrawCircle((int)amberPos.x, (int)amberPos.y, 2.0f * zoom, Color{69, 39, 10, 255});
    DrawCircle((int)greenPos.x, (int)greenPos.y, 2.0f * zoom, Color{10, 69, 30, 255});

    // Active illuminated aspect with radiant bloom
    if (node.signalAspect == SIGNAL_RED) {
        DrawCircle((int)redPos.x, (int)redPos.y, 2.4f * zoom, Color{239, 68, 68, 255});
        DrawCircleGradient(redPos, 6.0f * zoom, Color{239, 68, 68, 140}, Color{239, 68, 68, 0});
    } else if (node.signalAspect == SIGNAL_AMBER) {
        DrawCircle((int)amberPos.x, (int)amberPos.y, 2.4f * zoom, Color{245, 158, 11, 255});
        DrawCircleGradient(amberPos, 6.0f * zoom, Color{245, 158, 11, 140}, Color{245, 158, 11, 0});
    } else {
        DrawCircle((int)greenPos.x, (int)greenPos.y, 2.4f * zoom, Color{34, 197, 94, 255});
        DrawCircleGradient(greenPos, 6.0f * zoom, Color{34, 197, 94, 140}, Color{34, 197, 94, 0});
    }
}

void TrackSystem::DrawTunnelPortal(const TrackNode& node, Vector2 camOffset, float zoom) {
    Vector2 center = Iso::GridToScreen((float)node.gx + 0.5f, (float)node.gy + 0.5f, (float)node.gz, camOffset, zoom);
    float portalW = 28.0f * zoom;
    float portalH = 26.0f * zoom;

    // Heavy Reinforced Concrete Portal Structure
    DrawRectangle((int)(center.x - portalW * 0.5f), (int)(center.y - portalH), (int)portalW, (int)portalH, Color{71, 85, 105, 255});
    DrawRectangle((int)(center.x - portalW * 0.4f), (int)(center.y - portalH + 2.0f * zoom), (int)(portalW * 0.8f), (int)(portalH - 2.0f * zoom), Color{15, 23, 42, 255}); // Dark tunnel void

    // Caution Chevron Striping along Portal Arch
    for (int s = 0; s < 5; ++s) {
        Color stripeC = (s % 2 == 0) ? Color{245, 158, 11, 255} : Color{15, 23, 42, 255};
        DrawRectangle((int)(center.x - portalW * 0.5f + (float)s * 5.6f * zoom), (int)(center.y - portalH), (int)(5.6f * zoom), (int)(3.5f * zoom), stripeC);
    }

    // Tunnel Portal Warning Beacon
    DrawCircle((int)center.x, (int)(center.y - portalH - 2.0f * zoom), 2.5f * zoom, Color{245, 158, 11, 255});
    DrawCircleGradient(Vector2{center.x, center.y - portalH - 2.0f * zoom}, 7.0f * zoom, Color{245, 158, 11, 140}, Color{245, 158, 11, 0});
}

void TrackSystem::DrawSinglePiece(const TrackNode& node, Vector2 camOffset, float zoom) {
    if (node.splinePoints.empty()) return;

    // 1. Draw Ballast Gravel Bed
    DrawBallastBed(node, camOffset, zoom);

    // 2. Draw Station Platform Canopy if station
    if (node.type == TRACK_STATION) {
        DrawPlatformCanopy(node, camOffset, zoom);
    }

    // 3. Draw Cross Sleepers (Ties)
    float railGauge = 5.2f * zoom;
    Color tieC = Color{148, 163, 184, 255}; // Modern concrete ties

    for (size_t i = 0; i < node.splinePoints.size() - 1; i += 2) {
        Vector3 pt = node.splinePoints[i];
        Vector3 nextPt = node.splinePoints[i + 1];
        Vector2 sPt = Iso::GridToScreen(pt.x, pt.y, pt.z, camOffset, zoom);
        Vector2 sNext = Iso::GridToScreen(nextPt.x, nextPt.y, nextPt.z, camOffset, zoom);

        Vector2 tangent = Vector2Normalize({sNext.x - sPt.x, sNext.y - sPt.y});
        Vector2 normal = {-tangent.y, tangent.x};

        Vector2 leftTie = {sPt.x - normal.x * railGauge, sPt.y - normal.y * railGauge};
        Vector2 rightTie = {sPt.x + normal.x * railGauge, sPt.y + normal.y * railGauge};

        DrawLineEx(leftTie, rightTie, std::max(1.8f, 2.4f * zoom), tieC);
    }

    // 4. Draw Third Rail (Electrification)
    DrawThirdRail(node, camOffset, zoom);

    // 5. Draw Dual Running Steel Rails
    Color railC = Color{226, 232, 240, 255}; // Steel silver
    Color railHighlight = WHITE;

    for (size_t i = 0; i < node.splinePoints.size() - 1; ++i) {
        Vector3 pt1 = node.splinePoints[i];
        Vector3 pt2 = node.splinePoints[i + 1];

        Vector2 s1 = Iso::GridToScreen(pt1.x, pt1.y, pt1.z, camOffset, zoom);
        Vector2 s2 = Iso::GridToScreen(pt2.x, pt2.y, pt2.z, camOffset, zoom);

        Vector2 tangent = Vector2Normalize({s2.x - s1.x, s2.y - s1.y});
        Vector2 normal = {-tangent.y, tangent.x};

        Vector2 rail1_start = {s1.x - normal.x * (railGauge * 0.72f), s1.y - normal.y * (railGauge * 0.72f)};
        Vector2 rail1_end   = {s2.x - normal.x * (railGauge * 0.72f), s2.y - normal.y * (railGauge * 0.72f)};

        Vector2 rail2_start = {s1.x + normal.x * (railGauge * 0.72f), s1.y + normal.y * (railGauge * 0.72f)};
        Vector2 rail2_end   = {s2.x + normal.x * (railGauge * 0.72f), s2.y + normal.y * (railGauge * 0.72f)};

        float thick = std::max(1.6f, 2.2f * zoom);
        DrawLineEx(rail1_start, rail1_end, thick, railC);
        DrawLineEx(rail2_start, rail2_end, thick, railC);

        // Specular rail-head highlight
        DrawLineEx(rail1_start, rail1_end, 1.0f, railHighlight);
        DrawLineEx(rail2_start, rail2_end, 1.0f, railHighlight);
    }

    // 6. Draw Wayside Signaling Mast if signal block
    if (node.type == TRACK_SIGNAL) {
        DrawSignalMast(node, camOffset, zoom);
    }

    // 7. Draw Subway Tunnel Portal if tunnel
    if (node.type == TRACK_TUNNEL_PORTAL) {
        DrawTunnelPortal(node, camOffset, zoom);
    }
}

void TrackSystem::DrawGhostPiece(int gx, int gy, int gz, TrackType type, Direction inDir, Direction outDir, Vector2 camOffset, float zoom, bool canPlace) {
    TrackNode ghost;
    ghost.gx = gx;
    ghost.gy = gy;
    ghost.gz = gz;
    ghost.type = type;
    ghost.inDir = inDir;
    ghost.outDir = outDir;
    ghost.endZ = (type == TRACK_VIADUCT_ELEVATED) ? gz + 1 : (type == TRACK_VIADUCT_SLOPE ? std::max(0, gz - 1) : gz);
    ghost.color = canPlace ? Color{34, 197, 94, 180} : Color{239, 68, 68, 180};

    GenerateTileSpline(ghost);

    if (ghost.gz > 0) {
        Iso::DrawPillar(gx, gy, 0, ghost.gz, camOffset, zoom);
    }

    Iso::DrawCursor(gx, gy, gz, camOffset, zoom, ghost.color);
    DrawSinglePiece(ghost, camOffset, zoom);

    // Directional flow arrow indicating train travel path
    if (ghost.splinePoints.size() >= 2) {
        size_t midIdx = ghost.splinePoints.size() / 2;
        Vector3 pA = ghost.splinePoints[midIdx - 1];
        Vector3 pB = ghost.splinePoints[midIdx];
        Vector2 sA = Iso::GridToScreen(pA.x, pA.y, pA.z + 0.35f, camOffset, zoom);
        Vector2 sB = Iso::GridToScreen(pB.x, pB.y, pB.z + 0.35f, camOffset, zoom);
        Vector2 fwd = Vector2Normalize({sB.x - sA.x, sB.y - sA.y});
        Vector2 norm = {-fwd.y, fwd.x};

        Vector2 tip = {sB.x + fwd.x * 7.0f * zoom, sB.y + fwd.y * 7.0f * zoom};
        Vector2 leftWing = {sB.x - fwd.x * 5.0f * zoom + norm.x * 5.0f * zoom, sB.y - fwd.y * 5.0f * zoom + norm.y * 5.0f * zoom};
        Vector2 rightWing = {sB.x - fwd.x * 5.0f * zoom - norm.x * 5.0f * zoom, sB.y - fwd.y * 5.0f * zoom - norm.y * 5.0f * zoom};

        DrawTriangle(tip, leftWing, rightWing, Color{255, 214, 0, 240});
        DrawCircle((int)tip.x, (int)tip.y, 2.5f * zoom, Color{255, 255, 255, 220});
    }
}

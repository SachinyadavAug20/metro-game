#include "track.hpp"
#include "isometric.hpp"

TrackSystem::TrackSystem() {
    InitDefaultCircuit();
}

void TrackSystem::InitDefaultCircuit() {
    pieces.clear();
    Color redCoaster = Color{229, 57, 53, 255}; // Classic RCT Red

    // Create a fun starter circuit:
    // Station at (8, 12) -> Lift Hill -> Drop -> Curve -> Brakes -> Station
    AddPiece(8, 12, 0, TRACK_STATION, DIR_SOUTH, DIR_EAST, redCoaster);
    AddPiece(9, 12, 0, TRACK_STRAIGHT, DIR_WEST, DIR_EAST, redCoaster);
    AddPiece(10, 12, 0, TRACK_LIFT_HILL, DIR_WEST, DIR_EAST, redCoaster); // Ends at Z=1
    AddPiece(11, 12, 1, TRACK_LIFT_HILL, DIR_WEST, DIR_EAST, redCoaster); // Ends at Z=2
    AddPiece(12, 12, 2, TRACK_LIFT_HILL, DIR_WEST, DIR_EAST, redCoaster); // Ends at Z=3
    AddPiece(13, 12, 3, TRACK_STRAIGHT, DIR_WEST, DIR_EAST, redCoaster);  // Crest at Z=3
    AddPiece(14, 12, 3, TRACK_CURVE_RIGHT, DIR_WEST, DIR_SOUTH, redCoaster); // Turn toward south at Z=3
    
    // First Steep Drop from Z=3 down to Z=1
    AddPiece(14, 13, 3, TRACK_DROP, DIR_NORTH, DIR_SOUTH, redCoaster); // Z=3 -> 2
    AddPiece(14, 14, 2, TRACK_DROP, DIR_NORTH, DIR_SOUTH, redCoaster); // Z=2 -> 1
    AddPiece(14, 15, 1, TRACK_DROP, DIR_NORTH, DIR_SOUTH, redCoaster); // Z=1 -> 0
    AddPiece(14, 16, 0, TRACK_CURVE_RIGHT, DIR_NORTH, DIR_WEST, redCoaster); // High-speed bottom turn to West!
    
    // Return run
    AddPiece(13, 16, 0, TRACK_LOOP, DIR_EAST, DIR_WEST, redCoaster); // Loop
    AddPiece(12, 16, 0, TRACK_STRAIGHT, DIR_EAST, DIR_WEST, redCoaster);
    AddPiece(11, 16, 0, TRACK_BRAKES, DIR_EAST, DIR_WEST, redCoaster); // Slow down before station
    AddPiece(10, 16, 0, TRACK_STRAIGHT, DIR_EAST, DIR_WEST, redCoaster);
    AddPiece(9, 16, 0, TRACK_CURVE_RIGHT, DIR_EAST, DIR_NORTH, redCoaster); // Turn North
    
    // Connecting back up to Station
    AddPiece(9, 15, 0, TRACK_STRAIGHT, DIR_SOUTH, DIR_NORTH, redCoaster);
    AddPiece(9, 14, 0, TRACK_STRAIGHT, DIR_SOUTH, DIR_NORTH, redCoaster);
    AddPiece(9, 13, 0, TRACK_CURVE_LEFT, DIR_SOUTH, DIR_WEST, redCoaster); // Turn West
    AddPiece(8, 13, 0, TRACK_CURVE_RIGHT, DIR_EAST, DIR_NORTH, redCoaster); // Turn North into Station

    RecalculateCircuit();
}

void TrackSystem::SetTrackColor(Color c) {
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

    // Calculate end elevation
    if (type == TRACK_LIFT_HILL) {
        node.endZ = gz + 1;
    } else if (type == TRACK_DROP) {
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
    const int SUBDIVS = 12;

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

    if (node.type == TRACK_LOOP) {
        // Vertical Loop: Circle standing in vertical plane
        for (int i = 0; i <= SUBDIVS; ++i) {
            float t = (float)i / SUBDIVS;
            
            float fwd = (t - 0.5f);
            float loopHeight = sinf(t * PI) * 1.6f; // Rise up 1.6 Z
            Vector3 pt = {
                (float)node.gx + 0.5f + outOff.x * fwd,
                (float)node.gy + 0.5f + outOff.y * fwd,
                (float)node.gz + loopHeight
            };
            node.splinePoints.push_back(pt);
        }
        return;
    }

    // Bezier quadratic interpolation through pStart, pMid, pEnd
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
    maxDropHeight = 0.0f;
    inversionCount = 0;

    if (pieces.empty()) return;

    // Find Station
    const TrackNode* station = nullptr;
    for (const auto& p : pieces) {
        if (p.type == TRACK_STATION) {
            station = &p;
            break;
        }
    }
    if (!station) return;

    // Traverse circuit piece by piece
    std::vector<const TrackNode*> chain;
    const TrackNode* current = station;
    chain.push_back(current);

    std::vector<const TrackNode*> visited;
    bool loopFound = false;

    while (current != nullptr && chain.size() <= pieces.size() + 1) {
        // Next coordinate
        Vector2 fwd = GetDirectionOffset(current->outDir);
        int nextX = current->gx + (int)fwd.x;
        int nextY = current->gy + (int)fwd.y;
        int expectedZ = current->endZ;

        const TrackNode* next = GetPiece(nextX, nextY);
        if (!next) break;

        // Verify connection matching
        if (next->inDir != GetOppositeDir(current->outDir) || next->gz != expectedZ) {
            break;
        }

        if (next == station) {
            loopFound = true;
            break;
        }

        // Avoid infinite ping pong
        if (std::find(visited.begin(), visited.end(), next) != visited.end()) {
            break;
        }

        visited.push_back(next);
        chain.push_back(next);
        current = next;
    }

    if (!loopFound) return;

    circuitClosed = true;

    // Compile master path
    for (const auto* piece : chain) {
        if (piece->type == TRACK_LOOP) inversionCount++;
        float drop = (float)(piece->gz - piece->endZ);
        if (drop > maxDropHeight) maxDropHeight = drop;

        for (size_t i = 0; i < piece->splinePoints.size() - 1; ++i) {
            masterPath.push_back(piece->splinePoints[i]);
        }
    }

    if (masterPath.size() < 4) {
        circuitClosed = false;
        return;
    }

    // Compute cumulative distances
    pathDistances.push_back(0.0f);
    for (size_t i = 1; i < masterPath.size(); ++i) {
        Vector3 delta = Vector3Subtract(masterPath[i], masterPath[i - 1]);
        totalLength += Vector3Length(delta);
        pathDistances.push_back(totalLength);
    }
    // Close the loop to first point
    Vector3 closeDelta = Vector3Subtract(masterPath[0], masterPath.back());
    totalLength += Vector3Length(closeDelta);
}

Vector3 TrackSystem::GetPointAtDistance(float distance, Vector3* outTangent) const {
    if (masterPath.empty() || totalLength <= 0.001f) return {0, 0, 0};

    float d = fmodf(distance, totalLength);
    if (d < 0.0f) d += totalLength;

    // Binary search for distance segment
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

bool TrackSystem::GetStationLocation(int& outGx, int& outGy, int& outGz) const {
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

void TrackSystem::DrawAllTracks(Vector2 camOffset, float zoom) {
    // 1. Draw Support Pillars first
    for (const auto& p : pieces) {
        if (p.gz > 0 || p.endZ > 0) {
            Iso::DrawPillar(p.gx, p.gy, 0, std::max(p.gz, p.endZ), camOffset, zoom);
        }
    }

    // 2. Draw Track Rails & Ties
    for (const auto& p : pieces) {
        DrawSinglePiece(p, camOffset, zoom);
    }
}

void TrackSystem::DrawSinglePiece(const TrackNode& node, Vector2 camOffset, float zoom) {
    if (node.splinePoints.empty()) return;

    Color railC = node.color;
    Color tieC = Color{55, 71, 79, 255}; // Dark steel ties
    

    // Station special platform
    if (node.type == TRACK_STATION) {
        Vector2 top = Iso::GridToScreen((float)node.gx, (float)node.gy, (float)node.gz, camOffset, zoom);
        Vector2 right = Iso::GridToScreen((float)(node.gx + 1), (float)node.gy, (float)node.gz, camOffset, zoom);
        Vector2 bottom = Iso::GridToScreen((float)(node.gx + 1), (float)(node.gy + 1), (float)node.gz, camOffset, zoom);
        Vector2 left = Iso::GridToScreen((float)node.gx, (float)(node.gy + 1), (float)node.gz, camOffset, zoom);

        // Station wood planking
        DrawTriangle(top, left, bottom, Color{161, 136, 127, 240});
        DrawTriangle(top, bottom, right, Color{141, 110, 99, 240});
        DrawLineEx(top, right, 2.0f * zoom, Color{255, 213, 79, 255}); // Yellow safety edge
        DrawLineEx(bottom, left, 2.0f * zoom, Color{255, 213, 79, 255});
    }

    // Draw Cross Ties
    float railGauge = 5.0f * zoom;
    for (size_t i = 0; i < node.splinePoints.size() - 1; i += 2) {
        Vector3 pt = node.splinePoints[i];
        Vector3 nextPt = node.splinePoints[i + 1];
        Vector2 sPt = Iso::GridToScreen(pt.x, pt.y, pt.z, camOffset, zoom);
        Vector2 sNext = Iso::GridToScreen(nextPt.x, nextPt.y, nextPt.z, camOffset, zoom);

        Vector2 tangent = Vector2Normalize({sNext.x - sPt.x, sNext.y - sPt.y});
        Vector2 normal = {-tangent.y, tangent.x};

        Vector2 leftTie = {sPt.x - normal.x * railGauge, sPt.y - normal.y * railGauge};
        Vector2 rightTie = {sPt.x + normal.x * railGauge, sPt.y + normal.y * railGauge};

        DrawLineEx(leftTie, rightTie, std::max(1.5f, 2.0f * zoom), tieC);

        // If Lift Hill: draw yellow motorized chain link in middle
        if (node.type == TRACK_LIFT_HILL) {
            DrawCircle((int)sPt.x, (int)sPt.y, std::max(1.0f, 2.0f * zoom), Color{255, 214, 0, 255});
        }
        // If Brakes: draw pneumatic brake fins
        else if (node.type == TRACK_BRAKES) {
            DrawRectangle((int)(sPt.x - 2 * zoom), (int)(sPt.y - 2 * zoom), (int)(4 * zoom), (int)(4 * zoom), Color{216, 27, 96, 255});
        }
    }

    // Draw Double Parallel Rails
    for (size_t i = 0; i < node.splinePoints.size() - 1; ++i) {
        Vector3 pt1 = node.splinePoints[i];
        Vector3 pt2 = node.splinePoints[i + 1];

        Vector2 s1 = Iso::GridToScreen(pt1.x, pt1.y, pt1.z, camOffset, zoom);
        Vector2 s2 = Iso::GridToScreen(pt2.x, pt2.y, pt2.z, camOffset, zoom);

        Vector2 tangent = Vector2Normalize({s2.x - s1.x, s2.y - s1.y});
        Vector2 normal = {-tangent.y, tangent.x};

        Vector2 rail1_start = {s1.x - normal.x * (railGauge * 0.7f), s1.y - normal.y * (railGauge * 0.7f)};
        Vector2 rail1_end   = {s2.x - normal.x * (railGauge * 0.7f), s2.y - normal.y * (railGauge * 0.7f)};

        Vector2 rail2_start = {s1.x + normal.x * (railGauge * 0.7f), s1.y + normal.y * (railGauge * 0.7f)};
        Vector2 rail2_end   = {s2.x + normal.x * (railGauge * 0.7f), s2.y + normal.y * (railGauge * 0.7f)};

        float thick = std::max(1.5f, 2.2f * zoom);
        DrawLineEx(rail1_start, rail1_end, thick, railC);
        DrawLineEx(rail2_start, rail2_end, thick, railC);
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
    ghost.endZ = (type == TRACK_LIFT_HILL) ? gz + 1 : (type == TRACK_DROP ? std::max(0, gz - 1) : gz);
    ghost.color = canPlace ? Color{76, 175, 80, 180} : Color{244, 67, 54, 180}; // Green if valid, Red if blocked

    GenerateTileSpline(ghost);

    // Draw ghost support
    if (ghost.gz > 0) {
        Iso::DrawPillar(gx, gy, 0, ghost.gz, camOffset, zoom);
    }
    // Draw ghost tile outline
    Iso::DrawCursor(gx, gy, gz, camOffset, zoom, ghost.color);
    DrawSinglePiece(ghost, camOffset, zoom);
}

#include "train.hpp"
#include "isometric.hpp"
#include "audio.hpp"

MetroTrain::MetroTrain() {
    SetCarriageCount(3); // Standard 3-car urban EMU trainset
}

void MetroTrain::SetCarriageCount(int count) {
    cars.clear();
    for (int i = 0; i < count; ++i) {
        MetroCar car;
        car.passengerCount = 0;
        car.maxCapacity = 4;
        cars.push_back(car);
    }
}

void MetroTrain::Reset(const TrackSystem& tracks) {
    (void)tracks;
    state = TRAIN_STOPPED_IN_STATION;
    distance = 0.0f;
    velocity = 0.0f;
    stationTimer = 3.0f;
    doorsOpen = true;
    doorProgress = 1.0f;
    lastStationGx = -1;
    lastStationGy = -1;

    for (auto& car : cars) {
        car.passengerCount = 0;
        car.targetShapes.clear();
        car.commuterShirtColors.clear();
        car.doorOpenProgress = 1.0f;
    }
}

int MetroTrain::GetTotalPassengers() const {
    int total = 0;
    for (const auto& c : cars) total += c.passengerCount;
    return total;
}

int MetroTrain::GetMaxCapacity() const {
    return (int)cars.size() * 4;
}

bool MetroTrain::BoardCommuter(StationShape targetShape, Color shirtColor) {
    for (auto& c : cars) {
        if (c.passengerCount < c.maxCapacity) {
            c.passengerCount++;
            c.targetShapes.push_back(targetShape);
            c.commuterShirtColors.push_back(shirtColor);
            return true;
        }
    }
    return false;
}

int MetroTrain::AlightCommutersAtStation(StationShape stationShape) {
    int alighted = 0;
    for (auto& c : cars) {
        for (size_t i = 0; i < c.targetShapes.size();) {
            if (c.targetShapes[i] == stationShape) {
                c.targetShapes.erase(c.targetShapes.begin() + i);
                c.commuterShirtColors.erase(c.commuterShirtColors.begin() + i);
                c.passengerCount--;
                alighted++;
            } else {
                ++i;
            }
        }
    }
    return alighted;
}

void MetroTrain::Update(float dt, TrackSystem& tracks, ParticleSystem& particles, int& outDeliveredCommuters, float& outFareRevenue) {
    if (!tracks.IsCircuitClosed()) {
        state = TRAIN_STOPPED_IN_STATION;
        velocity = 0.0f;
        return;
    }

    float circuitLen = tracks.GetTotalCircuitLength();
    if (circuitLen <= 0.1f) return;

    float curSpeed = GetSpeedKmh();
    if (curSpeed > recordSpeedKmh) {
        recordSpeedKmh = curSpeed;
    }

    // 1. Identify track element under locomotive
    Vector3 tangent;
    Vector3 headPos = tracks.GetPointAtDistance(distance, &tangent);
    int curGx = (int)roundf(headPos.x);
    int curGy = (int)roundf(headPos.y);
    const TrackNode* currentTile = tracks.GetPiece(curGx, curGy);

    // Update live signaling aspects based on current train position
    tracks.UpdateSignals(distance);

    // 2. Station Boarding / Dwell Cycle
    if (currentTile && currentTile->type == TRACK_STATION && curGx != lastStationGx) {
        if (state == TRAIN_CRUISING || state == TRAIN_APPROACHING_STATION || state == TRAIN_BRAKING) {
            // Decelerate smoothly into platform
            if (velocity > 1.8f) {
                state = TRAIN_BRAKING;
                velocity = std::max(0.0f, velocity - 10.0f * dt);
            } else {
                // Halts at platform center
                state = TRAIN_BOARDING;
                velocity = 0.0f;
                stationTimer = 3.8f; // Boarding window
                doorsOpen = true;
                doorProgress = 0.0f;
                lastStationGx = curGx;
                lastStationGy = curGy;

                AudioManager::Play(SFX_AIR_BRAKE, 0.7f);
                particles.SpawnSmoke(headPos, 3);

                // Passengers alight if current station matches their target shape!
                int alighted = AlightCommutersAtStation(currentTile->stationShape);
                if (alighted > 0) {
                    outDeliveredCommuters += alighted;
                    totalTransported += alighted;
                    outFareRevenue += (float)alighted * 2.50f;
                    AudioManager::Play(SFX_DELIVERY_CHIME, 0.9f);
                    particles.SpawnConfetti(headPos, 12);
                }
            }
            return;
        }
    }

    // While stopped in station
    if (state == TRAIN_BOARDING) {
        stationTimer -= dt;
        doorProgress = std::min(1.0f, doorProgress + dt * 3.5f);
        for (auto& c : cars) c.doorOpenProgress = doorProgress;

        // Departure warning chime before doors close
        if (stationTimer <= 1.2f && doorsOpen && stationTimer > 0.0f) {
            doorsOpen = false;
            AudioManager::Play(SFX_DOOR_CHIME, 0.85f);
        }

        if (stationTimer <= 0.0f) {
            state = TRAIN_ACCELERATING;
            doorProgress = 0.0f;
            for (auto& c : cars) c.doorOpenProgress = 0.0f;
            AudioManager::Play(SFX_VVVF_MOTOR, 0.7f);
            velocity = 2.0f;
        }
        return;
    }

    // Clear lastStationGx once far enough away from previous station
    if (lastStationGx != -1) {
        float distToLast = Vector2Distance(Vector2{(float)curGx, (float)curGy}, Vector2{(float)lastStationGx, (float)lastStationGy});
        if (distToLast > 1.8f) {
            lastStationGx = -1;
            lastStationGy = -1;
        }
    }

    // 3. Signaling & Speed Control
    SignalAspect activeAspect = tracks.GetActiveSignalAspect();
    float maxAllowedVelocity = 14.5f; // ~70 km/h cruising speed

    if (activeAspect == SIGNAL_RED) {
        maxAllowedVelocity = 0.0f;
        state = TRAIN_SIGNAL_STOP;
    } else if (activeAspect == SIGNAL_AMBER) {
        maxAllowedVelocity = 7.5f; // ~36 km/h caution speed
        state = TRAIN_APPROACHING_STATION;
    } else {
        state = TRAIN_CRUISING;
    }

    // Smooth electric traction acceleration / braking
    if (velocity < maxAllowedVelocity) {
        velocity = std::min(maxAllowedVelocity, velocity + 6.0f * dt);
    } else if (velocity > maxAllowedVelocity) {
        velocity = std::max(maxAllowedVelocity, velocity - 8.5f * dt);
    }

    // VVVF Inverter sound timing during acceleration
    if (velocity > 3.0f && velocity < 12.0f) {
        vvvfSoundTimer -= dt;
        if (vvvfSoundTimer <= 0.0f) {
            AudioManager::Play(SFX_VVVF_MOTOR, 0.35f);
            vvvfSoundTimer = 3.5f;
        }
    }

    // 4. Advance train along master spline
    distance += velocity * dt;
    if (distance >= circuitLen) {
        distance = fmodf(distance, circuitLen);
    }

    // 5. Update car positions & forward tangents
    float carSpacing = 0.85f;
    for (size_t i = 0; i < cars.size(); ++i) {
        float carDist = distance - (float)i * carSpacing;
        if (carDist < 0.0f) carDist += circuitLen;

        Vector3 carTangent;
        cars[i].pos = tracks.GetPointAtDistance(carDist, &carTangent);
        cars[i].forward = carTangent;
    }
}

void MetroTrain::Draw(Vector2 camOffset, float zoom) const {
    if (cars.empty()) return;

    // Draw from rear car to front car for correct isometric painter's order
    for (int i = (int)cars.size() - 1; i >= 0; --i) {
        const auto& car = cars[i];
        Vector2 sPos = Iso::GridToScreen(car.pos.x, car.pos.y, car.pos.z, camOffset, zoom);
        Vector2 sFwd = Iso::GridToScreen(car.pos.x + car.forward.x * 0.4f, car.pos.y + car.forward.y * 0.4f, car.pos.z + car.forward.z * 0.4f, camOffset, zoom);

        float angle = atan2f(sFwd.y - sPos.y, sFwd.x - sPos.x) * RAD2DEG;
        float carLen = 17.0f * zoom;
        float carWid = 10.0f * zoom;

        // 1. Soft Shadow on track / ground
        Iso::DrawShadow(car.pos.x, car.pos.y, 8.0f, camOffset, zoom);

        // 2. Dark Undercarriage & Wheel Bogies
        DrawRectanglePro(
            Rectangle{sPos.x, sPos.y, carLen + 2.0f * zoom, carWid + 1.5f * zoom},
            Vector2{(carLen + 2.0f * zoom) * 0.5f, (carWid + 1.5f * zoom) * 0.5f},
            angle,
            Color{30, 41, 59, 255}
        );

        // 3. Stainless Steel Subway Car Body (Sleek brushed silver metallic)
        DrawRectanglePro(
            Rectangle{sPos.x, sPos.y, carLen, carWid},
            Vector2{carLen * 0.5f, carWid * 0.5f},
            angle,
            Color{203, 213, 225, 255}
        );

        // 4. Line Color Livery Band along Car Flanks
        DrawRectanglePro(
            Rectangle{sPos.x, sPos.y, carLen, 3.2f * zoom},
            Vector2{carLen * 0.5f, 1.6f * zoom},
            angle,
            themeColor
        );

        // 5. Passenger Windows with Interior Lighting & Commuter Silhouettes
        float winW = 3.5f * zoom;
        float winH = 2.2f * zoom;
        Vector2 perp = Vector2Normalize({-(sFwd.y - sPos.y), sFwd.x - sPos.x});
        Vector2 fwdNorm = Vector2Normalize({sFwd.x - sPos.x, sFwd.y - sPos.y});

        for (int w = -1; w <= 1; ++w) {
            Vector2 winCenter = {
                sPos.x + fwdNorm.x * (float)w * 4.5f * zoom,
                sPos.y + fwdNorm.y * (float)w * 4.5f * zoom - 2.5f * zoom
            };
            DrawRectanglePro(
                Rectangle{winCenter.x, winCenter.y, winW, winH},
                Vector2{winW * 0.5f, winH * 0.5f},
                angle,
                Color{254, 240, 138, 220} // Warm interior passenger cabin glow
            );
        }

        // 6. Sliding Plug Doors (Bi-parting doors open indicator)
        if (car.doorOpenProgress > 0.1f) {
            Vector2 doorPos = { sPos.x + perp.x * (carWid * 0.45f), sPos.y + perp.y * (carWid * 0.45f) };
            DrawCircle((int)doorPos.x, (int)doorPos.y, 2.5f * zoom, Color{34, 197, 94, 255}); // Green boarding light
        }

        // 7. Lead Locomotive Cab & Front Headlamps
        if (i == 0) {
            Vector2 headLensPos = { sPos.x + fwdNorm.x * (carLen * 0.52f), sPos.y + fwdNorm.y * (carLen * 0.52f) };

            // Front Windshield (Dark Tinted Glass)
            DrawRectanglePro(
                Rectangle{headLensPos.x, headLensPos.y, 3.5f * zoom, carWid * 0.8f},
                Vector2{1.75f * zoom, (carWid * 0.8f) * 0.5f},
                angle,
                Color{15, 23, 42, 255}
            );

            // LED Destination Matrix Rollsign ("L1 DOWNTOWN")
            DrawRectanglePro(
                Rectangle{headLensPos.x, headLensPos.y - 2.0f * zoom, 2.0f * zoom, 6.0f * zoom},
                Vector2{1.0f * zoom, 3.0f * zoom},
                angle,
                Color{245, 158, 11, 255}
            );

            // Twin High-Beam Dual LED Headlights
            Vector2 hl1 = { headLensPos.x + perp.x * 2.8f * zoom, headLensPos.y + perp.y * 2.8f * zoom };
            Vector2 hl2 = { headLensPos.x - perp.x * 2.8f * zoom, headLensPos.y - perp.y * 2.8f * zoom };
            DrawCircle((int)hl1.x, (int)hl1.y, 2.2f * zoom, WHITE);
            DrawCircle((int)hl2.x, (int)hl2.y, 2.2f * zoom, WHITE);

            // Forward Light Illumination Cones on Rails
            Vector2 beamEnd1 = { hl1.x + fwdNorm.x * 32.0f * zoom + perp.x * 14.0f * zoom, hl1.y + fwdNorm.y * 32.0f * zoom + perp.y * 14.0f * zoom };
            Vector2 beamEnd2 = { hl2.x + fwdNorm.x * 32.0f * zoom - perp.x * 14.0f * zoom, hl2.y + fwdNorm.y * 32.0f * zoom - perp.y * 14.0f * zoom };
            DrawTriangle(headLensPos, beamEnd1, beamEnd2, Color{254, 240, 138, 45});
        }

        // 8. Trailing Rear Car Red Taillights
        if (i == (int)cars.size() - 1) {
            Vector2 tailPos = { sPos.x - fwdNorm.x * (carLen * 0.5f), sPos.y - fwdNorm.y * (carLen * 0.5f) };
            Vector2 tl1 = { tailPos.x + perp.x * 2.8f * zoom, tailPos.y + perp.y * 2.8f * zoom };
            Vector2 tl2 = { tailPos.x - perp.x * 2.8f * zoom, tailPos.y - perp.y * 2.8f * zoom };
            DrawCircle((int)tl1.x, (int)tl1.y, 1.8f * zoom, Color{239, 68, 68, 255});
            DrawCircle((int)tl2.x, (int)tl2.y, 1.8f * zoom, Color{239, 68, 68, 255});
        }

        // 9. Articulated Gangway Bellow between cars
        if (i < (int)cars.size() - 1) {
            Vector2 bellowPos = { sPos.x - fwdNorm.x * (carLen * 0.55f), sPos.y - fwdNorm.y * (carLen * 0.55f) };
            DrawRectanglePro(
                Rectangle{bellowPos.x, bellowPos.y, 3.5f * zoom, carWid * 0.7f},
                Vector2{1.75f * zoom, (carWid * 0.7f) * 0.5f},
                angle,
                Color{51, 65, 85, 255}
            );
        }

        // 10. Commuter Shape Badges on Passenger Silhouettes inside car
        if (car.passengerCount > 0) {
            for (size_t p = 0; p < car.targetShapes.size() && p < 4; ++p) {
                float offset = (float)p * 2.8f * zoom - 4.2f * zoom;
                Vector2 pPos = { sPos.x + fwdNorm.x * offset, sPos.y + fwdNorm.y * offset - 3.5f * zoom };
                Color sCol = GetShapeColor(car.targetShapes[p]);
                DrawCircle((int)pPos.x, (int)pPos.y, 2.0f * zoom, sCol);
            }
        }
    }
}

MetroLineStats MetroTrain::GetStats(const TrackSystem& tracks) const {
    MetroLineStats stats;
    stats.lineName = lineName;
    stats.themeColor = themeColor;
    stats.currentSpeedKmh = GetSpeedKmh();
    stats.maxSpeedKmh = std::max(stats.currentSpeedKmh, recordSpeedKmh);
    stats.trackLengthM = tracks.GetTrackLengthM();
    stats.stationCount = tracks.GetStationCount();
    stats.fleetCars = (int)cars.size();
    stats.totalRiders = totalTransported;
    stats.ticketFare = 2.50f;
    stats.currentSignal = tracks.GetActiveSignalAspect();

    // High performance punctuality based on speed and station coverage
    stats.onTimeRate = std::min(99.8f, 95.0f + (stats.currentSpeedKmh > 20.0f ? 4.2f : 1.0f));
    stats.commuterSatisfaction = std::min(98.5f, 90.0f + (float)stats.fleetCars * 1.5f);

    return stats;
}


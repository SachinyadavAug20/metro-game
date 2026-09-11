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
    state = TRAIN_BOARDING;
    distance = 0.5f; // Center of Central Hub Station
    velocity = 0.0f;
    stationTimer = 3.5f;
    doorsOpen = true;
    doorProgress = 1.0f;
    lastStationGx = 7;
    lastStationGy = 12;

    float circuitLen = tracks.GetTotalCircuitLength();
    float carSpacing = 0.85f;
    for (size_t i = 0; i < cars.size(); ++i) {
        cars[i].passengerCount = 0;
        cars[i].targetShapes.clear();
        cars[i].commuterShirtColors.clear();
        cars[i].doorOpenProgress = 1.0f;

        float carDist = distance - (float)i * carSpacing;
        if (circuitLen > 0.0f) {
            while (carDist < 0.0f) carDist += circuitLen;
            carDist = fmodf(carDist, circuitLen);
        }
        Vector3 carTangent;
        cars[i].pos = tracks.GetPointAtDistance(carDist, &carTangent);
        cars[i].forward = carTangent;
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
            if (stationShape == SHAPE_NONE || c.targetShapes[i] == stationShape) {
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
        doorsOpen = true;
        return;
    }

    float circuitLen = tracks.GetTotalCircuitLength();
    if (circuitLen <= 0.1f) return;

    float curSpeed = GetSpeedKmh();
    if (curSpeed > recordSpeedKmh) {
        recordSpeedKmh = curSpeed;
    }

    // Update live signaling aspects based on current train position
    tracks.UpdateSignals(distance);

    if (operatingMode == LINE_CLOSED) {
        // Line is closed - train halts
        state = TRAIN_STOPPED_IN_STATION;
        velocity = std::max(0.0f, velocity - 10.0f * dt);
        doorsOpen = false;
        doorProgress = std::max(0.0f, doorProgress - dt * 3.0f);
        for (auto& c : cars) c.doorOpenProgress = doorProgress;
    } else if (state == TRAIN_BOARDING) {
        // Stopped at station platform: boarding & alighting
        velocity = 0.0f;
        stationTimer -= dt;
        doorProgress = std::min(1.0f, doorProgress + dt * 3.5f);
        for (auto& c : cars) c.doorOpenProgress = doorProgress;

        // Departure warning chime before doors slide closed
        if (stationTimer <= 1.0f && doorsOpen) {
            doorsOpen = false;
            AudioManager::Play(SFX_DOOR_CHIME, 0.75f);
        }

        if (stationTimer <= 0.0f) {
            state = TRAIN_ACCELERATING;
            doorProgress = 0.0f;
            for (auto& c : cars) c.doorOpenProgress = 0.0f;
            AudioManager::Play(SFX_VVVF_MOTOR, 0.65f);
            velocity = 2.5f;
            distance += 0.35f; // Nudge past platform stop threshold
            if (distance >= circuitLen) distance = fmodf(distance, circuitLen);
        }
    } else {
        // Train is in motion
        StationInfo nextStation;
        float distToStation = 999.0f;
        bool hasStation = tracks.GetNextStationAhead(distance, distToStation, nextStation);

        // Smooth station deceleration and exact center stop
        if (hasStation && distToStation <= 2.6f && distToStation > 0.09f) {
            state = TRAIN_BRAKING;
            float targetV = std::max(1.4f, targetVelocity * (distToStation / 2.6f));
            if (velocity > targetV) {
                velocity = std::max(targetV, velocity - 12.0f * dt);
            }
        } else if (hasStation && distToStation <= 0.09f) {
            // Arrived squarely at the station platform!
            distance = nextStation.circuitDist;
            velocity = 0.0f;
            state = TRAIN_BOARDING;
            stationTimer = 3.2f;
            doorsOpen = true;
            doorProgress = 0.0f;

            AudioManager::Play(SFX_AIR_BRAKE, 0.6f);
            AudioManager::Play(SFX_STATION_BELL, 0.7f);

            Vector3 headPos = tracks.GetPointAtDistance(distance);
            particles.SpawnSmoke(headPos, 3);

            if (operatingMode == LINE_OPEN) {
                // Alight passengers matching station shape or round-trip riders
                int alighted = AlightCommutersAtStation(nextStation.shape);
                if (alighted == 0 && GetTotalPassengers() > 0 && (rand() % 100 < 60)) {
                    alighted = AlightCommutersAtStation(SHAPE_NONE); // round-trip scenic commuters alight
                }
                if (alighted > 0) {
                    outDeliveredCommuters += alighted;
                    totalTransported += alighted;

                    bool leveledUp = false;
                    int newLevel = 1;
                    tracks.RecordStationAlight(nextStation.gx, nextStation.gy, alighted, leveledUp, newLevel);

                    float lvlBonus = (nextStation.stationLevel == 3) ? 1.50f : ((nextStation.stationLevel == 2) ? 1.25f : 1.0f);
                    float rev = (float)alighted * ticketFare * lvlBonus;
                    outFareRevenue += rev;
                    totalRevenueEarned += rev;

                    AudioManager::Play(SFX_CASH_REGISTER, 0.85f);
                    particles.SpawnConfetti(headPos, 14);

                    if (leveledUp) {
                        AudioManager::Play(SFX_UPGRADE_FANFARE, 0.95f);
                        particles.SpawnConfetti(headPos, 28);
                        particles.SpawnFloatingText(
                            Vector3{headPos.x, headPos.y, headPos.z + 1.4f},
                            TextFormat("★ STATION LEVEL %d! ★", newLevel),
                            Color{255, 215, 0, 255}
                        );
                    } else if (lvlBonus > 1.0f) {
                        particles.SpawnFloatingText(
                            Vector3{headPos.x, headPos.y, headPos.z + 0.9f},
                            TextFormat("+$%.2f (LV %d BONUS)", rev, nextStation.stationLevel),
                            Color{52, 211, 153, 255}
                        );
                    } else {
                        particles.SpawnFloatingText(
                            Vector3{headPos.x, headPos.y, headPos.z + 0.9f},
                            TextFormat("+$%.2f", rev),
                            Color{34, 197, 94, 255}
                        );
                    }
                }
            }
        } else {
            // Normal cruising / acceleration
            state = TRAIN_CRUISING;
            if (velocity < targetVelocity) {
                velocity = std::min(targetVelocity, velocity + 6.0f * dt);
            } else if (velocity > targetVelocity) {
                velocity = std::max(targetVelocity, velocity - 8.0f * dt);
            }

            // Periodic electric motor hum while accelerating
            if (velocity > 3.0f && velocity < 12.0f) {
                vvvfSoundTimer -= dt;
                if (vvvfSoundTimer <= 0.0f) {
                    AudioManager::Play(SFX_VVVF_MOTOR, 0.35f);
                    vvvfSoundTimer = 3.5f;
                }
            }
        }
    }

    // Advance train along track spline
    if (velocity > 0.0f) {
        distance += velocity * dt;
        if (distance >= circuitLen) {
            distance = fmodf(distance, circuitLen);
        }
    }

    // Update car positions & forward tangents
    float carSpacing = 1.05f;
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
        Vector2 sPos = Iso::GridToScreen(car.pos.x, car.pos.y, car.pos.z + 0.32f, camOffset, zoom);
        Vector2 sFwd = Iso::GridToScreen(car.pos.x + car.forward.x * 0.4f, car.pos.y + car.forward.y * 0.4f, car.pos.z + 0.32f + car.forward.z * 0.4f, camOffset, zoom);

        float angle = atan2f(sFwd.y - sPos.y, sFwd.x - sPos.x) * RAD2DEG;
        float carLen = 32.0f * zoom;
        float carWid = 16.0f * zoom;

        // 1. Soft Shadow on track / ground
        Iso::DrawShadow(car.pos.x, car.pos.y, 12.0f, camOffset, zoom);

        // 2. Dark Undercarriage & Wheel Bogies
        DrawRectanglePro(
            Rectangle{sPos.x, sPos.y, carLen + 2.0f * zoom, carWid + 2.0f * zoom},
            Vector2{(carLen + 2.0f * zoom) * 0.5f, (carWid + 2.0f * zoom) * 0.5f},
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
            Rectangle{sPos.x, sPos.y, carLen, 5.0f * zoom},
            Vector2{carLen * 0.5f, 2.5f * zoom},
            angle,
            themeColor
        );

        // 5. Passenger Windows with Interior Lighting & Warm Glow
        float winW = 6.0f * zoom;
        float winH = 3.6f * zoom;
        Vector2 perp = Vector2Normalize({-(sFwd.y - sPos.y), sFwd.x - sPos.x});
        Vector2 fwdNorm = Vector2Normalize({sFwd.x - sPos.x, sFwd.y - sPos.y});

        for (int w = -1; w <= 1; ++w) {
            Vector2 winCenter = {
                sPos.x + fwdNorm.x * (float)w * 8.5f * zoom,
                sPos.y + fwdNorm.y * (float)w * 8.5f * zoom - 3.5f * zoom
            };
            DrawRectanglePro(
                Rectangle{winCenter.x, winCenter.y, winW, winH},
                Vector2{winW * 0.5f, winH * 0.5f},
                angle,
                Color{254, 240, 138, 230} // Warm interior passenger cabin glow
            );
        }

        // 6. Sliding Plug Doors (Bi-parting doors open indicator)
        if (car.doorOpenProgress > 0.1f) {
            Vector2 doorPos = { sPos.x + perp.x * (carWid * 0.45f), sPos.y + perp.y * (carWid * 0.45f) };
            DrawCircle((int)doorPos.x, (int)doorPos.y, 3.5f * zoom, Color{34, 197, 94, 255}); // Green boarding light
        }

        // 6b. Rooftop Aerodynamic HVAC Pods
        DrawRectanglePro(
            Rectangle{sPos.x, sPos.y - 4.5f * zoom, 12.0f * zoom, 3.8f * zoom},
            Vector2{6.0f * zoom, 1.9f * zoom},
            angle,
            Color{148, 163, 184, 255}
        );

        // Single-arm pantograph on 2nd car
        if (i == 1 && cars.size() > 1) {
            Vector2 pantoBase = { sPos.x, sPos.y - 6.0f * zoom };
            DrawLineEx(pantoBase, {pantoBase.x, pantoBase.y - 7.0f * zoom}, 1.5f * zoom, Color{239, 68, 68, 255});
            DrawLineEx({pantoBase.x - 4.0f * zoom, pantoBase.y - 7.0f * zoom}, {pantoBase.x + 4.0f * zoom, pantoBase.y - 7.0f * zoom}, 2.0f * zoom, Color{30, 41, 59, 255});
        }

        // 7. Lead Locomotive Cab & Front Headlamps
        if (i == 0) {
            Vector2 headLensPos = { sPos.x + fwdNorm.x * (carLen * 0.52f), sPos.y + fwdNorm.y * (carLen * 0.52f) };

            // Front Windshield (Dark Tinted Glass)
            DrawRectanglePro(
                Rectangle{headLensPos.x, headLensPos.y, 5.0f * zoom, carWid * 0.8f},
                Vector2{2.5f * zoom, (carWid * 0.8f) * 0.5f},
                angle,
                Color{15, 23, 42, 255}
            );

            // LED Destination Matrix Rollsign ("L1 DOWNTOWN")
            DrawRectanglePro(
                Rectangle{headLensPos.x, headLensPos.y - 3.0f * zoom, 3.0f * zoom, 8.0f * zoom},
                Vector2{1.5f * zoom, 4.0f * zoom},
                angle,
                Color{245, 158, 11, 255}
            );

            // Twin High-Beam Dual LED Headlights
            Vector2 hl1 = { headLensPos.x + perp.x * 4.5f * zoom, headLensPos.y + perp.y * 4.5f * zoom };
            Vector2 hl2 = { headLensPos.x - perp.x * 4.5f * zoom, headLensPos.y - perp.y * 4.5f * zoom };
            DrawCircle((int)hl1.x, (int)hl1.y, 3.0f * zoom, WHITE);
            DrawCircle((int)hl2.x, (int)hl2.y, 3.0f * zoom, WHITE);

            // Forward Light Illumination Cones on Rails
            Vector2 beamEnd1 = { hl1.x + fwdNorm.x * 45.0f * zoom + perp.x * 18.0f * zoom, hl1.y + fwdNorm.y * 45.0f * zoom + perp.y * 18.0f * zoom };
            Vector2 beamEnd2 = { hl2.x + fwdNorm.x * 45.0f * zoom - perp.x * 18.0f * zoom, hl2.y + fwdNorm.y * 45.0f * zoom - perp.y * 18.0f * zoom };
            DrawTriangle(headLensPos, beamEnd1, beamEnd2, Color{254, 240, 138, 55});
        }

        // 8. Trailing Rear Car Red Taillights
        if (i == (int)cars.size() - 1) {
            Vector2 tailPos = { sPos.x - fwdNorm.x * (carLen * 0.5f), sPos.y - fwdNorm.y * (carLen * 0.5f) };
            Vector2 tl1 = { tailPos.x + perp.x * 4.5f * zoom, tailPos.y + perp.y * 4.5f * zoom };
            Vector2 tl2 = { tailPos.x - perp.x * 4.5f * zoom, tailPos.y - perp.y * 4.5f * zoom };
            DrawCircle((int)tl1.x, (int)tl1.y, 2.5f * zoom, Color{239, 68, 68, 255});
            DrawCircle((int)tl2.x, (int)tl2.y, 2.5f * zoom, Color{239, 68, 68, 255});
        }

        // 9. Articulated Gangway Bellow between cars
        if (i < (int)cars.size() - 1) {
            Vector2 bellowPos = { sPos.x - fwdNorm.x * (carLen * 0.55f), sPos.y - fwdNorm.y * (carLen * 0.55f) };
            DrawRectanglePro(
                Rectangle{bellowPos.x, bellowPos.y, 5.0f * zoom, carWid * 0.75f},
                Vector2{2.5f * zoom, (carWid * 0.75f) * 0.5f},
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
    stats.mode = operatingMode;
    stats.currentSpeedKmh = GetSpeedKmh();
    stats.maxSpeedKmh = std::max(stats.currentSpeedKmh, recordSpeedKmh);
    stats.trackLengthM = tracks.GetTrackLengthM();
    stats.stationCount = tracks.GetStationCount();
    stats.fleetCars = (int)cars.size();
    stats.totalRiders = totalTransported;
    stats.totalRevenue = totalRevenueEarned;
    stats.ticketFare = ticketFare;
    stats.currentSignal = tracks.GetActiveSignalAspect();

    // High performance punctuality based on speed and station coverage
    stats.onTimeRate = std::min(99.8f, 95.0f + (stats.currentSpeedKmh > 20.0f ? 4.2f : 1.0f));
    stats.commuterSatisfaction = std::min(98.5f, 90.0f + (float)stats.fleetCars * 1.5f);

    return stats;
}


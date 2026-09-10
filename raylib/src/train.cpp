#include "train.hpp"
#include "isometric.hpp"
#include "audio.hpp"

CoasterTrain::CoasterTrain() {
    SetCarriageCount(3); // 1 Engine + 2 Passenger cars (6 seats)
}

void CoasterTrain::SetCarriageCount(int count) {
    cars.clear();
    for (int i = 0; i < count; ++i) {
        TrainCar car;
        car.passengerCount = 0;
        car.maxCapacity = 2;
        cars.push_back(car);
    }
}

void CoasterTrain::Reset(const TrackSystem& tracks) {
    (void)tracks;
    state = TRAIN_STOPPED_IN_STATION;
    distance = 0.0f;
    velocity = 0.0f;
    stationTimer = 2.0f; // Brief dwell before first dispatch
    crashCars.clear();

    for (auto& car : cars) {
        car.passengerCount = 0;
        car.peepShirtColors.clear();
    }
}

void CoasterTrain::RecoverFromCrash(const TrackSystem& tracks) {
    Reset(tracks);
}

int CoasterTrain::GetTotalPassengers() const {
    int total = 0;
    for (const auto& c : cars) total += c.passengerCount;
    return total;
}

int CoasterTrain::GetMaxCapacity() const {
    return (int)cars.size() * 2;
}

bool CoasterTrain::BoardPassenger(Color shirtColor) {
    for (auto& c : cars) {
        if (c.passengerCount < c.maxCapacity) {
            c.passengerCount++;
            c.peepShirtColors.push_back(shirtColor);
            return true;
        }
    }
    return false;
}

void CoasterTrain::Update(float dt, const TrackSystem& tracks, ParticleSystem& particles, int& outDeliveredGuests) {
    if (!tracks.IsCircuitClosed()) {
        state = TRAIN_STOPPED_IN_STATION;
        return;
    }

    float circuitLen = tracks.GetTotalCircuitLength();
    if (circuitLen <= 0.1f) return;

    float curSpeed = GetSpeedKmh();
    if (curSpeed > recordSpeedKmh) {
        recordSpeedKmh = curSpeed;
    }

    // 1. Crash State
    if (state == TRAIN_DERAILED) {
        for (auto& cc : crashCars) {
            cc.pos.x += cc.vel.x * dt;
            cc.pos.y += cc.vel.y * dt;
            cc.pos.z += cc.vel.z * dt;
            cc.vel.z -= 9.8f * dt; // Gravity
            if (cc.pos.z < 0.0f) {
                cc.pos.z = 0.0f;
                cc.vel.x *= 0.6f;
                cc.vel.y *= 0.6f;
            }
        }
        return;
    }

    // 2. Station Boarding / Dwell
    if (state == TRAIN_STOPPED_IN_STATION || state == TRAIN_BOARDING) {
        stationTimer -= dt;
        state = TRAIN_BOARDING;

        if (stationTimer <= 0.0f) {
            state = TRAIN_DISPATCHING;
            AudioManager::Play(SFX_STATION_BELL, 0.8f);
            particles.SpawnSmoke(cars[0].pos, 8);
            velocity = 2.0f; // Initial push out of station
        }
        return;
    }

    // 3. Kinetic Coaster Physics along Circuit
    Vector3 tangent;
    Vector3 headPos = tracks.GetPointAtDistance(distance, &tangent);

    // Identify current track tile under locomotive
    int curGx = (int)headPos.x;
    int curGy = (int)headPos.y;
    const TrackNode* currentTile = tracks.GetPiece(curGx, curGy);

    float slopeZ = tangent.z; // Elevation delta

    if (currentTile && currentTile->type == TRACK_LIFT_HILL) {
        // Chain lift motor: constant speed upward
        state = TRAIN_ON_LIFT;
        velocity = 3.2f;

        chainClickTimer -= dt;
        if (chainClickTimer <= 0.0f) {
            AudioManager::Play(SFX_CHAIN_CLICK, 0.65f);
            chainClickTimer = 0.22f; // Classic rhythmic click
        }
    } else if (currentTile && currentTile->type == TRACK_DROP) {
        // Gravity Drop: Massive acceleration!
        state = TRAIN_COASTING;
        velocity += (9.8f * 1.5f) * dt;

        sparkTimer -= dt;
        if (sparkTimer <= 0.0f) {
            particles.SpawnSparks(headPos, 4);
            sparkTimer = 0.08f;
        }
        if (velocity > 12.0f) {
            AudioManager::Play(SFX_WHOOSH, 0.4f);
        }
    } else if (currentTile && currentTile->type == TRACK_BRAKES) {
        // Brakes: smooth deceleration
        state = TRAIN_BRAKING;
        velocity = std::max(2.5f, velocity - 15.0f * dt);
        particles.SpawnSmoke(headPos, 2);
    } else if (currentTile && currentTile->type == TRACK_STATION) {
        if (state == TRAIN_DISPATCHING) {
            velocity = std::min(4.0f, velocity + 6.0f * dt);
            if (distance > 1.0f) {
                state = TRAIN_COASTING;
            }
        } else if (distance > 4.0f) {
            // Approaching station at circuit end: decelerate to stop
            if (velocity > 2.0f) {
                velocity -= 10.0f * dt;
            } else {
                // Arrived and stopped!
                state = TRAIN_STOPPED_IN_STATION;
                velocity = 0.0f;
                distance = 0.0f;
                stationTimer = 4.0f; // 4 second passenger boarding window
                int delivered = GetTotalPassengers();
                if (delivered > 0) {
                    outDeliveredGuests += delivered;
                    AudioManager::Play(SFX_DELIVERY_CHIME, 0.9f);
                    for (auto& c : cars) {
                        c.passengerCount = 0;
                        c.peepShirtColors.clear();
                    }
                }
                return;
            }
        }
    } else {
        // Standard coaster physics: Energy conservation + rolling friction
        state = TRAIN_COASTING;
        float gravityAccel = -slopeZ * 9.8f;
        float friction = (0.04f + 0.015f * velocity);
        velocity += (gravityAccel - friction) * dt;
        velocity = std::max(1.0f, velocity); // Minimum rolling speed to avoid dead stall
    }

    // Derailment Check: sharp turns taken at lethal velocity (>75 km/h)
    if (currentTile && (currentTile->type == TRACK_CURVE_RIGHT || currentTile->type == TRACK_CURVE_LEFT)) {
        if (velocity > 16.5f) { // ~82 km/h
            state = TRAIN_DERAILED;
            AudioManager::Play(SFX_CRASH, 1.0f);
            particles.SpawnConfetti(headPos, 50);
            particles.SpawnSparks(headPos, 30);

            // Populate tumbling crash cars
            crashCars.clear();
            for (const auto& c : cars) {
                CrashCar cc;
                cc.pos = c.pos;
                cc.vel = {
                    tangent.x * velocity * 0.8f + (((float)rand() / RAND_MAX) - 0.5f) * 4.0f,
                    tangent.y * velocity * 0.8f + (((float)rand() / RAND_MAX) - 0.5f) * 4.0f,
                    3.0f + ((float)rand() / RAND_MAX) * 4.0f
                };
                crashCars.push_back(cc);
            }
            return;
        }
    }

    // Advance train distance along track
    distance += velocity * dt;
    if (distance >= circuitLen) {
        distance = fmodf(distance, circuitLen);
    }

    // Update positions of all cars in the train
    float carSpacing = 0.75f;
    for (size_t i = 0; i < cars.size(); ++i) {
        float carDist = distance - (float)i * carSpacing;
        if (carDist < 0.0f) carDist += circuitLen;

        Vector3 carTangent;
        cars[i].pos = tracks.GetPointAtDistance(carDist, &carTangent);
        cars[i].forward = carTangent;
    }
}

void CoasterTrain::Draw(Vector2 camOffset, float zoom) const {
    if (state == TRAIN_DERAILED) {
        // Draw tumbling crash cars
        for (const auto& cc : crashCars) {
            Vector2 sPos = Iso::GridToScreen(cc.pos.x, cc.pos.y, cc.pos.z, camOffset, zoom);
            float sz = 10.0f * zoom;
            DrawRectanglePro(Rectangle{sPos.x, sPos.y, sz * 1.5f, sz}, Vector2{sz * 0.75f, sz * 0.5f}, 45.0f, Color{239, 83, 80, 255});
        }
        return;
    }

    // Draw Coaster Cars (from rear to front for correct depth overlap)
    for (int i = (int)cars.size() - 1; i >= 0; --i) {
        const auto& car = cars[i];
        Vector2 sPos = Iso::GridToScreen(car.pos.x, car.pos.y, car.pos.z, camOffset, zoom);
        Vector2 sFwd = Iso::GridToScreen(car.pos.x + car.forward.x * 0.4f, car.pos.y + car.forward.y * 0.4f, car.pos.z + car.forward.z * 0.4f, camOffset, zoom);

        float angle = atan2f(sFwd.y - sPos.y, sFwd.x - sPos.x) * RAD2DEG;
        float carLen = 14.0f * zoom;
        float carWid = 9.0f * zoom;

        Color bodyC = (i == 0) ? Color{255, 179, 0, 255} : themeColor; // Gold engine, Custom theme carriages
        Color trimC = Color{38, 50, 56, 255}; // Dark chassis

        // Shadow beneath car
        Iso::DrawShadow(car.pos.x, car.pos.y, 7.0f, camOffset, zoom);

        // Bogey wheel chassis
        DrawRectanglePro(
            Rectangle{sPos.x, sPos.y, carLen + 2.0f * zoom, carWid + 1.0f * zoom},
            Vector2{(carLen + 2.0f * zoom) / 2.0f, (carWid + 1.0f * zoom) / 2.0f},
            angle,
            trimC
        );

        // Coaster Coach Body
        DrawRectanglePro(
            Rectangle{sPos.x, sPos.y, carLen, carWid},
            Vector2{carLen / 2.0f, carWid / 2.0f},
            angle,
            bodyC
        );

        // Chrome Safety Lap Bar
        DrawRectanglePro(
            Rectangle{sPos.x, sPos.y, carLen * 0.7f, 2.0f * zoom},
            Vector2{(carLen * 0.7f) / 2.0f, 1.0f * zoom},
            angle,
            Color{207, 216, 220, 255}
        );

        // Locomotive Front Headlamp & Forward Light Beam
        if (i == 0) {
            Vector2 fwdDir = Vector2Normalize({sFwd.x - sPos.x, sFwd.y - sPos.y});
            Vector2 headPos = { sPos.x + fwdDir.x * (carLen * 0.5f), sPos.y + fwdDir.y * (carLen * 0.5f) };
            // Headlamp lens
            DrawCircle((int)headPos.x, (int)headPos.y, 2.8f * zoom, Color{255, 238, 88, 255});
            // Forward illumination cone
            Vector2 perp = {-fwdDir.y, fwdDir.x};
            Vector2 beamEnd1 = { headPos.x + fwdDir.x * 24.0f * zoom + perp.x * 12.0f * zoom, headPos.y + fwdDir.y * 24.0f * zoom + perp.y * 12.0f * zoom };
            Vector2 beamEnd2 = { headPos.x + fwdDir.x * 24.0f * zoom - perp.x * 12.0f * zoom, headPos.y + fwdDir.y * 24.0f * zoom - perp.y * 12.0f * zoom };
            DrawTriangle(headPos, beamEnd1, beamEnd2, Color{255, 245, 157, 50});
        }

        // Draw Passengers inside car
        if (car.passengerCount > 0) {
            float offsetHead = (carWid * 0.25f);
            for (int p = 0; p < car.passengerCount; ++p) {
                float headDir = (p == 0) ? -offsetHead : offsetHead;
                Vector2 headOffset = Vector2Rotate({headDir, 0.0f}, angle * DEG2RAD);
                Color shirt = (p < (int)car.peepShirtColors.size()) ? car.peepShirtColors[p] : Color{33, 150, 243, 255};

                // Peep head + colored hat
                DrawCircle((int)(sPos.x + headOffset.x), (int)(sPos.y + headOffset.y - 3.0f * zoom), 2.5f * zoom, Color{255, 224, 178, 255}); // Skin
                DrawCircle((int)(sPos.x + headOffset.x), (int)(sPos.y + headOffset.y - 4.5f * zoom), 2.2f * zoom, shirt); // Cap
            }
        }
    }
}

CoasterStats CoasterTrain::GetStats(const TrackSystem& tracks) const {
    CoasterStats stats;
    stats.themeColor = themeColor;
    stats.coasterName = coasterName;
    stats.currentSpeedKmh = GetSpeedKmh();
    stats.maxSpeedKmh = std::max(stats.currentSpeedKmh, recordSpeedKmh);
    stats.trackLengthM = tracks.GetTotalCircuitLength() * 12.0f; // Scale to meters
    stats.maxDropM = tracks.GetMaxDrop() * 4.5f;                 // Scale to meters
    stats.inversions = tracks.GetInversionCount();
    stats.totalRiders = totalTransported;

    // Classic RCT Excitement / Intensity / Nausea formulas
    float speedFactor = stats.maxSpeedKmh / 20.0f;
    float dropFactor = stats.maxDropM * 0.35f;
    float invFactor = (float)stats.inversions * 1.4f;

    stats.excitementRating = std::min(9.9f, 2.5f + speedFactor * 0.9f + dropFactor * 0.4f + invFactor * 0.6f);
    stats.intensityRating = std::min(9.9f, 1.6f + speedFactor * 1.1f + dropFactor * 0.6f + invFactor * 1.3f);
    stats.nauseaRating = std::min(9.5f, 0.8f + speedFactor * 0.5f + dropFactor * 0.4f + invFactor * 0.8f);

    return stats;
}

#pragma once

#include "common.hpp"
#include "track.hpp"
#include "particles.hpp"

struct MetroCar {
    Vector3 pos = {0, 0, 0};
    Vector3 forward = {1, 0, 0};
    int passengerCount = 0;
    int maxCapacity = 4; // 4 commuters per carriage
    std::vector<StationShape> targetShapes;
    std::vector<Color> commuterShirtColors;
    float doorOpenProgress = 0.0f; // 0.0f = closed, 1.0f = open
};

class MetroTrain {
public:
    MetroTrain();

    void Reset(const TrackSystem& tracks);
    void Update(float dt, TrackSystem& tracks, ParticleSystem& particles, int& outDeliveredCommuters, float& outFareRevenue);
    void Draw(Vector2 camOffset, float zoom) const;

    // State queries
    TrainState GetState() const { return state; }
    float GetSpeedKmh() const { return velocity * 4.8f; } // Scaled km/h
    int GetTotalPassengers() const;
    int GetMaxCapacity() const;
    bool CanBoard() const { return state == TRAIN_BOARDING && doorsOpen; }
    bool BoardCommuter(StationShape targetShape, Color shirtColor);
    int AlightCommutersAtStation(StationShape stationShape);
    void SetCarriageCount(int count);

    Vector3 GetLocomotivePos() const { return cars.empty() ? Vector3{0,0,0} : cars[0].pos; }
    MetroLineStats GetStats(const TrackSystem& tracks) const;
    void RecordPassengerDelivery(int count) { totalTransported += count; }
    void SetTrainTheme(Color c) { themeColor = c; }
    Color GetTrainTheme() const { return themeColor; }
    void SetLineName(const std::string& name) { lineName = name; }
    const std::string& GetLineName() const { return lineName; }

    // Compatibility aliases
    void SetCoasterName(const std::string& name) { SetLineName(name); }
    const std::string& GetCoasterName() const { return GetLineName(); }
    bool IsDerailed() const { return false; }
    void RecoverFromCrash(const TrackSystem& tracks) { Reset(tracks); }

private:
    std::vector<MetroCar> cars;
    Color themeColor = Color{229, 57, 53, 255}; // Line 1 Tokyo Red
    std::string lineName = "Line 1 - Central Loop";
    TrainState state = TRAIN_STOPPED_IN_STATION;
    float distance = 0.0f;
    float velocity = 0.0f;
    float targetVelocity = 14.5f; // ~70 km/h cruising speed
    float stationTimer = 0.0f;
    float vvvfSoundTimer = 0.0f;
    float recordSpeedKmh = 0.0f;
    int totalTransported = 0;
    bool doorsOpen = false;
    float doorProgress = 0.0f;
    int lastStationGx = -1;
    int lastStationGy = -1;
};

// Typedef for seamless drop-in replacement
using CoasterTrain = MetroTrain;
using CoasterStats = MetroLineStats;


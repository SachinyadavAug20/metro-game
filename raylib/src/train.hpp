#pragma once

#include "common.hpp"
#include "track.hpp"
#include "particles.hpp"

struct TrainCar {
    Vector3 pos = {0, 0, 0};
    Vector3 forward = {1, 0, 0};
    int passengerCount = 0;
    int maxCapacity = 2;
    std::vector<Color> peepShirtColors;
};

class CoasterTrain {
public:
    CoasterTrain();

    void Reset(const TrackSystem& tracks);
    void Update(float dt, const TrackSystem& tracks, ParticleSystem& particles, int& outDeliveredGuests);
    void Draw(Vector2 camOffset, float zoom) const;

    // State queries
    TrainState GetState() const { return state; }
    float GetSpeedKmh() const { return velocity * 5.0f; } // Conversion for HUD display
    int GetTotalPassengers() const;
    int GetMaxCapacity() const;
    bool CanBoard() const { return state == TRAIN_BOARDING; }
    bool BoardPassenger(Color shirtColor);
    void SetCarriageCount(int count);

    bool IsDerailed() const { return state == TRAIN_DERAILED; }
    void RecoverFromCrash(const TrackSystem& tracks);

    Vector3 GetLocomotivePos() const { return cars.empty() ? Vector3{0,0,0} : cars[0].pos; }
    CoasterStats GetStats(const TrackSystem& tracks) const;
    void RecordPassengerDelivery(int count) { totalTransported += count; }
    void SetTrainTheme(Color c) { themeColor = c; }
    Color GetTrainTheme() const { return themeColor; }
    void SetCoasterName(const std::string& name) { coasterName = name; }
    const std::string& GetCoasterName() const { return coasterName; }

private:
    std::vector<TrainCar> cars;
    Color themeColor = Color{229, 57, 53, 255}; // Classic Red
    std::string coasterName = "The Red Falcon";
    TrainState state = TRAIN_STOPPED_IN_STATION;
    float distance = 0.0f;
    float velocity = 0.0f;
    float stationTimer = 0.0f;
    float chainClickTimer = 0.0f;
    float sparkTimer = 0.0f;
    float recordSpeedKmh = 0.0f;
    int totalTransported = 0;

    // Derailment ragdoll physics
    struct CrashCar {
        Vector3 pos = {0, 0, 0};
        Vector3 vel;
        Vector3 rot;
    };
    std::vector<CrashCar> crashCars;
};

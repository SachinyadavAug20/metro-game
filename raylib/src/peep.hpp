#pragma once

#include "common.hpp"
#include "particles.hpp"
#include "train.hpp"

struct Commuter {
    std::string name;
    std::string occupation;
    Vector2 pos;
    Vector2 targetPos;
    CommuterType type = COMMUTER_WORKER;
    CommuterState state = COMMUTER_ENTERING;
    StationShape targetShape = SHAPE_SQUARE; // Mini Metro target shape
    Color shirtColor = Color{30, 58, 138, 255};
    Color pantsColor = Color{51, 65, 85, 255};
    float speed = 1.6f;
    float walkTimer = 0.0f;
    float happiness = 92.0f;
    float metroPassBalance = 25.0f;
    float queuePatience = 45.0f;
    float queueTimer = 0.0f;
    bool isAngry = false;
    bool hasCoffee = false;
    bool hasBriefcase = true;
    std::string thought = "Tapped my transit card, waiting for Line 1!";
};

class CommuterManager {
public:
    CommuterManager();

    void Init(Vector2 stationEntrance, Vector2 platformEntrance, Vector2 stationExit);
    void Update(float dt, MetroTrain& train, std::vector<MetroTrain>* extraTrains, ParticleSystem& particles, float& outSatisfaction, int& outAngryLeaves, std::vector<StationMess>& outMesses);
    void CheckSceneryInteractions(const SceneryType scenery[GRID_SIZE][GRID_SIZE], ParticleSystem& particles, float& outFunds, std::vector<StationMess>& outMesses);
    void Draw(Vector2 camOffset, float zoom) const;

    int GetTotalCommuters() const { return (int)commuters.size(); }
    int GetQueueCount() const { return (int)queueCommuters.size(); }
    int GetMaxQueueCapacity() const { return maxQueueCap; }
    float GetQueueOvercrowdRatio() const;
    float GetOvercrowdTimer() const { return overcrowdTimer; }
    bool IsOvercrowded() const { return overcrowdActive; }

    int FindCommuterAtScreenPos(Vector2 mouseScreen, Vector2 camOffset, float zoom) const;
    const Commuter* GetCommuter(int index) const {
        if (index >= 0 && index < (int)commuters.size()) return &commuters[index];
        return nullptr;
    }

    void SpawnCommuter();
    void SetSpawnInterval(float seconds) { spawnInterval = seconds; }
    void AlightPassengers(int count, Vector2 stationPos);

    // Compatibility aliases
    int GetTotalPeepsInPark() const { return GetTotalCommuters(); }
    int FindPeepAtScreenPos(Vector2 mouseScreen, Vector2 camOffset, float zoom) const {
        return FindCommuterAtScreenPos(mouseScreen, camOffset, zoom);
    }
    const Commuter* GetPeep(int index) const { return GetCommuter(index); }
    void SpawnPeep() { SpawnCommuter(); }

private:
    std::vector<Commuter> commuters;
    std::vector<size_t> queueCommuters;
    Vector2 entrancePos;
    Vector2 queueStartPos;
    Vector2 exitPos;

    float spawnTimer = 0.0f;
    float spawnInterval = 1.9f;
    int maxQueueCap = 18;

    // Overcrowding Mini-Metro style clock
    bool overcrowdActive = false;
    float overcrowdTimer = 26.0f;
    const float MAX_OVERCROWD_TIME = 26.0f;

    void UpdateCommuterMovement(Commuter& c, float dt);
};

// Aliases for seamless drop-in replacement
using Peep = Commuter;
using PeepManager = CommuterManager;
using PeepType = CommuterType;
using PeepState = CommuterState;


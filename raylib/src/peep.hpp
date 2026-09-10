#pragma once

#include "common.hpp"
#include "particles.hpp"
#include "train.hpp"

struct Peep {
    std::string name;
    Vector2 pos;
    Vector2 targetPos;
    PeepType type;
    PeepState state;
    Color shirtColor;
    Color pantsColor;
    float speed = 1.6f;
    float walkTimer = 0.0f;
    float happiness = 85.0f;
    float nausea = 0.0f;
    float queuePatience = 35.0f;
    float queueTimer = 0.0f;
    bool isAngry = false;
    bool hasBalloon = false;
    Color balloonColor = Color{239, 68, 68, 255};
    std::string thought = "Excited to ride!";
};

class PeepManager {
public:
    PeepManager();

    void Init(Vector2 parkEntrance, Vector2 stationQueueEntrance, Vector2 stationExit);
    void Update(float dt, CoasterTrain& train, ParticleSystem& particles, float& outParkRating, int& outAngryLeaves, std::vector<ParkMess>& outMesses);
    void CheckSceneryInteractions(const SceneryType scenery[GRID_SIZE][GRID_SIZE], ParticleSystem& particles, float& outMoney, std::vector<ParkMess>& outMesses);
    void Draw(Vector2 camOffset, float zoom) const;

    int GetTotalPeepsInPark() const { return (int)peeps.size(); }
    int GetQueueCount() const { return (int)queuePeeps.size(); }
    int GetMaxQueueCapacity() const { return maxQueueCap; }
    float GetQueueOvercrowdRatio() const;
    float GetOvercrowdTimer() const { return overcrowdTimer; }
    bool IsOvercrowded() const { return overcrowdActive; }

    int FindPeepAtScreenPos(Vector2 mouseScreen, Vector2 camOffset, float zoom) const;
    const Peep* GetPeep(int index) const {
        if (index >= 0 && index < (int)peeps.size()) return &peeps[index];
        return nullptr;
    }

    void SpawnPeep();
    void SetSpawnInterval(float seconds) { spawnInterval = seconds; }

private:
    std::vector<Peep> peeps;
    std::vector<size_t> queuePeeps; // Indices of peeps currently in queue line
    Vector2 entrancePos;
    Vector2 queueStartPos;
    Vector2 exitPos;

    float spawnTimer = 0.0f;
    float spawnInterval = 2.5f;
    int maxQueueCap = 14;

    // Overcrowding Mini-Metro style clock
    bool overcrowdActive = false;
    float overcrowdTimer = 18.0f;
    const float MAX_OVERCROWD_TIME = 18.0f;

    void UpdatePeepMovement(Peep& p, float dt);
};

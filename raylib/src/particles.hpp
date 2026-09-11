#pragma once

#include "common.hpp"

enum ParticleType {
    PARTICLE_SPARK = 0,
    PARTICLE_CONFETTI,
    PARTICLE_SMOKE,
    PARTICLE_VOMIT
};

struct Particle {
    Vector3 pos;      // Grid coords (gx, gy, gz)
    Vector3 vel;      // Velocity
    Color color;
    float life;       // Remaining life in seconds
    float maxLife;
    float size;
    ParticleType type;
};

struct FloatingText {
    Vector3 pos;
    std::string text;
    Color color;
    float life;
    float maxLife;
    float rise;
};

class ParticleSystem {
public:
    ParticleSystem() = default;

    void SpawnSparks(Vector3 gridPos, int count = 12);
    void SpawnConfetti(Vector3 gridPos, int count = 40);
    void SpawnSmoke(Vector3 gridPos, int count = 6);
    void SpawnVomit(Vector3 gridPos, int count = 15);
    void SpawnFloatingText(Vector3 gridPos, const std::string& text, Color color = Color{34, 197, 94, 255});

    void Update(float dt);
    void Draw(Vector2 camOffset, float zoom);
    void Clear();

private:
    std::vector<Particle> particles;
    std::vector<FloatingText> floatingTexts;
};

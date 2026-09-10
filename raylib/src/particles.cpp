#include "particles.hpp"
#include "isometric.hpp"

void ParticleSystem::SpawnSparks(Vector3 gridPos, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = gridPos;
        float angle = ((float)rand() / RAND_MAX) * 2.0f * PI;
        float speed = 1.5f + ((float)rand() / RAND_MAX) * 3.0f;
        p.vel = {
            cosf(angle) * speed,
            sinf(angle) * speed,
            1.5f + ((float)rand() / RAND_MAX) * 2.5f
        };
        p.color = (rand() % 2 == 0) ? Color{255, 235, 59, 255} : Color{255, 152, 0, 255};
        p.maxLife = 0.35f + ((float)rand() / RAND_MAX) * 0.3f;
        p.life = p.maxLife;
        p.size = 2.5f;
        p.type = PARTICLE_SPARK;
        particles.push_back(p);
    }
}

void ParticleSystem::SpawnConfetti(Vector3 gridPos, int count) {
    Color colors[] = {
        Color{244, 67, 54, 255},
        Color{33, 150, 243, 255},
        Color{255, 235, 59, 255},
        Color{76, 175, 80, 255},
        Color{156, 39, 176, 255},
        Color{255, 152, 0, 255}
    };
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = gridPos;
        float angle = ((float)rand() / RAND_MAX) * 2.0f * PI;
        float speed = 2.0f + ((float)rand() / RAND_MAX) * 4.0f;
        p.vel = {
            cosf(angle) * speed,
            sinf(angle) * speed,
            3.0f + ((float)rand() / RAND_MAX) * 5.0f
        };
        p.color = colors[rand() % 6];
        p.maxLife = 1.5f + ((float)rand() / RAND_MAX) * 1.0f;
        p.life = p.maxLife;
        p.size = 4.0f;
        p.type = PARTICLE_CONFETTI;
        particles.push_back(p);
    }
}

void ParticleSystem::SpawnSmoke(Vector3 gridPos, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = gridPos;
        p.vel = {
            (((float)rand() / RAND_MAX) - 0.5f) * 0.6f,
            (((float)rand() / RAND_MAX) - 0.5f) * 0.6f,
            0.8f + ((float)rand() / RAND_MAX) * 0.8f
        };
        unsigned char shade = 200 + (rand() % 45);
        p.color = Color{shade, shade, shade, 180};
        p.maxLife = 0.6f + ((float)rand() / RAND_MAX) * 0.4f;
        p.life = p.maxLife;
        p.size = 5.0f;
        p.type = PARTICLE_SMOKE;
        particles.push_back(p);
    }
}

void ParticleSystem::SpawnVomit(Vector3 gridPos, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = gridPos;
        float angle = ((float)rand() / RAND_MAX) * 2.0f * PI;
        float speed = 0.5f + ((float)rand() / RAND_MAX) * 1.5f;
        p.vel = {
            cosf(angle) * speed,
            sinf(angle) * speed,
            0.8f + ((float)rand() / RAND_MAX) * 1.2f
        };
        p.color = Color{139, 195, 74, 230}; // Sick green
        p.maxLife = 0.5f + ((float)rand() / RAND_MAX) * 0.4f;
        p.life = p.maxLife;
        p.size = 3.0f;
        p.type = PARTICLE_VOMIT;
        particles.push_back(p);
    }
}

void ParticleSystem::Update(float dt) {
    for (size_t i = 0; i < particles.size();) {
        auto& p = particles[i];
        p.life -= dt;
        if (p.life <= 0.0f) {
            particles[i] = particles.back();
            particles.pop_back();
            continue;
        }

        // Gravity and drag
        if (p.type == PARTICLE_SPARK || p.type == PARTICLE_VOMIT) {
            p.vel.z -= 9.8f * dt; // Gravity pulls down in Z
        } else if (p.type == PARTICLE_CONFETTI) {
            p.vel.z -= 3.0f * dt; // Slow flutter
            p.vel.x *= 0.98f;
            p.vel.y *= 0.98f;
        } else if (p.type == PARTICLE_SMOKE) {
            p.size += 4.0f * dt;
            p.color.a = (unsigned char)(180.0f * (p.life / p.maxLife));
        }

        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.pos.z += p.vel.z * dt;

        // Ground collision (stop at Z = 0)
        if (p.pos.z < 0.0f) {
            p.pos.z = 0.0f;
            p.vel = {0, 0, 0};
        }

        ++i;
    }
}

void ParticleSystem::Draw(Vector2 camOffset, float zoom) {
    for (const auto& p : particles) {
        Vector2 screenPos = Iso::GridToScreen(p.pos.x, p.pos.y, p.pos.z, camOffset, zoom);
        float s = p.size * zoom;

        if (p.type == PARTICLE_CONFETTI) {
            DrawRectanglePro(
                Rectangle{screenPos.x, screenPos.y, s, s * 0.6f},
                Vector2{s / 2.0f, s * 0.3f},
                (p.life * 360.0f),
                p.color
            );
        } else {
            DrawCircle((int)screenPos.x, (int)screenPos.y, std::max(1.5f, s), p.color);
        }
    }
}

void ParticleSystem::Clear() {
    particles.clear();
}

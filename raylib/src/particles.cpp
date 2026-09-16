#include "particles.hpp"
#include "isometric.hpp"
#include "font_system.hpp"

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

void ParticleSystem::SpawnRain(int screenW, int /*screenH*/, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        // Rain falls from top of screen in screen space (stored as grid coords with special handling)
        p.pos = Vector3{(float)(rand() % screenW), -10.0f - (float)(rand() % 100), 0.0f};
        p.vel = Vector3{(float)(rand() % 20 - 10), (float)(400 + rand() % 200), 0.0f};
        p.color = Color{56, 189, 248, (unsigned char)(80 + rand() % 60)};
        p.maxLife = 1.2f;
        p.life = p.maxLife;
        p.size = 1.0f + (rand() % 100) / 100.0f;
        p.type = PARTICLE_RAIN;
        particles.push_back(p);
    }
}

void ParticleSystem::SpawnPetals(int screenW, int /*screenH*/, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = Vector3{(float)(rand() % screenW), -5.0f - (float)(rand() % 50), 0.0f};
        p.vel = Vector3{(float)(rand() % 40 - 20), (float)(30 + rand() % 60), 0.0f};
        // Cherry blossom pink palette
        int pink = rand() % 3;
        if (pink == 0) p.color = Color{255, 183, 197, (unsigned char)(120 + rand() % 80)};
        else if (pink == 1) p.color = Color{255, 150, 180, (unsigned char)(100 + rand() % 80)};
        else p.color = Color{255, 200, 210, (unsigned char)(90 + rand() % 70)};
        p.maxLife = 3.0f + (rand() % 200) / 100.0f;
        p.life = p.maxLife;
        p.size = 2.0f + (rand() % 100) / 50.0f;
        p.type = PARTICLE_PETAL;
        particles.push_back(p);
    }
}

void ParticleSystem::SpawnFloatingText(Vector3 gridPos, const std::string& text, Color color) {
    FloatingText ft;
    ft.pos = gridPos;
    ft.text = text;
    ft.color = color;
    ft.maxLife = 1.4f;
    ft.life = ft.maxLife;
    ft.rise = 0.0f;
    floatingTexts.push_back(ft);
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
        } else if (p.type == PARTICLE_RAIN) {
            p.vel.x += sinf(p.pos.y * 0.1f) * 20.0f * dt; // wind sway
        } else if (p.type == PARTICLE_PETAL) {
            p.vel.x += sinf(p.pos.y * 0.05f + p.pos.x * 0.02f) * 30.0f * dt; // drift
            p.vel.y += 5.0f * dt; // slow fall
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

    // Update floating texts
    for (size_t i = 0; i < floatingTexts.size();) {
        auto& ft = floatingTexts[i];
        ft.life -= dt;
        if (ft.life <= 0.0f) {
            floatingTexts[i] = floatingTexts.back();
            floatingTexts.pop_back();
            continue;
        }
        ft.rise += dt * 1.2f;
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
        } else if (p.type == PARTICLE_RAIN) {
            // Rain: thin diagonal line
            Vector2 end = {screenPos.x + p.vel.x * 0.01f, screenPos.y + s * 3.0f};
            DrawLineEx(screenPos, end, 1.0f, p.color);
        } else if (p.type == PARTICLE_PETAL) {
            // Petal: small rotated ellipse
            DrawRectanglePro(
                Rectangle{screenPos.x, screenPos.y, s, s * 0.5f},
                Vector2{s / 2.0f, s * 0.25f},
                (p.life * 120.0f),
                p.color
            );
        } else {
            DrawCircle((int)screenPos.x, (int)screenPos.y, std::max(1.5f, s), p.color);
        }
    }

    // Draw floating texts with drop shadow (RCT style)
    for (const auto& ft : floatingTexts) {
        Vector2 sPos = Iso::GridToScreen(ft.pos.x, ft.pos.y, ft.pos.z + ft.rise, camOffset, zoom);
        float alpha = std::min(1.0f, ft.life / 0.4f);
        Color col = ft.color;
        col.a = (unsigned char)(255.0f * alpha);
        Color shadow = Color{0, 0, 0, (unsigned char)(180.0f * alpha)};
        int fontSize = (int)std::max(12.0f, 15.0f * zoom);
        int textW = MeasureText(ft.text.c_str(), fontSize);
        int tx = (int)(sPos.x - textW / 2.0f);
        int ty = (int)sPos.y;

        // Shadow outline
        DrawText(ft.text.c_str(), tx + 1, ty + 1, fontSize, shadow);
        DrawText(ft.text.c_str(), tx - 1, ty + 1, fontSize, shadow);
        DrawText(ft.text.c_str(), tx, ty, fontSize, col);
    }
}

void ParticleSystem::Clear() {
    particles.clear();
    floatingTexts.clear();
}

#include "peep.hpp"
#include "isometric.hpp"
#include "audio.hpp"

PeepManager::PeepManager() {
    Init({2.0f, 12.0f}, {7.0f, 13.0f}, {8.0f, 14.0f});
}

void PeepManager::Init(Vector2 parkEntrance, Vector2 stationQueueEntrance, Vector2 stationExit) {
    entrancePos = parkEntrance;
    queueStartPos = stationQueueEntrance;
    exitPos = stationExit;
    peeps.clear();
    queuePeeps.clear();
    overcrowdActive = false;
    overcrowdTimer = MAX_OVERCROWD_TIME;

    // Pre-spawn initial happy crowd
    for (int i = 0; i < 8; ++i) {
        SpawnPeep();
    }
}

void PeepManager::SpawnPeep() {
    static const char* peepNames[] = {
        "Alex", "Maya", "Sam", "Chloe", "Leo", "Zoe", "Ryan", "Ella",
        "Lucas", "Mia", "Noah", "Harper", "Ben", "Lily", "Ethan", "Aria",
        "Mason", "Ivy", "James", "Ruby", "Logan", "Nora", "Jack", "Grace"
    };

    Peep p;
    p.name = peepNames[rand() % 24];
    p.pos = entrancePos;
    p.targetPos = queueStartPos;
    p.state = PEEP_WALKING_TO_RIDE;
    p.walkTimer = ((float)rand() / RAND_MAX) * 2.0f * PI;
    p.happiness = 75.0f + (float)(rand() % 25);
    p.nausea = 0.0f;
    p.thought = "Can't wait to ride the coaster!";

    int r = rand() % 100;
    if (r < 35) {
        p.type = PEEP_THRILL_SEEKER;
        p.shirtColor = Color{229, 57, 53, 255}; // Red
    } else if (r < 80) {
        p.type = PEEP_CASUAL;
        p.shirtColor = Color{67, 160, 71, 255}; // Green
    } else {
        p.type = PEEP_QUEASY;
        p.shirtColor = Color{253, 216, 53, 255}; // Yellow
    }

    p.pantsColor = (rand() % 2 == 0) ? Color{30, 136, 229, 255} : Color{69, 90, 100, 255};
    peeps.push_back(p);
}

float PeepManager::GetQueueOvercrowdRatio() const {
    return (float)queuePeeps.size() / (float)maxQueueCap;
}

void PeepManager::Update(float dt, CoasterTrain& train, ParticleSystem& particles, float& outParkRating, int& outAngryLeaves, std::vector<ParkMess>& outMesses) {
    // 1. Spawning
    spawnTimer -= dt;
    if (spawnTimer <= 0.0f) {
        if ((int)peeps.size() < 60) {
            SpawnPeep();
        }
        spawnTimer = spawnInterval;
    }

    // 2. Queue Overcrowding Clock (Mini Metro Triage)
    if (queuePeeps.size() >= 10) {
        overcrowdActive = true;
        overcrowdTimer -= dt;

        if (fmodf(overcrowdTimer, 2.0f) < dt) {
            AudioManager::Play(SFX_QUEUE_ALARM, 0.4f);
        }

        // Triage Failed! Station explodes into riot
        if (overcrowdTimer <= 0.0f) {
            overcrowdActive = false;
            overcrowdTimer = MAX_OVERCROWD_TIME;
            outParkRating = std::max(0.0f, outParkRating - 15.0f); // Huge penalty!

            // All queue peeps leave angry
            for (size_t idx : queuePeeps) {
                peeps[idx].state = PEEP_LEAVING_ANGRY;
                peeps[idx].targetPos = entrancePos;
                peeps[idx].isAngry = true;
                outAngryLeaves++;
            }
            queuePeeps.clear();
        }
    } else {
        // Reset or recover clock slowly
        overcrowdActive = false;
        overcrowdTimer = std::min(MAX_OVERCROWD_TIME, overcrowdTimer + dt * 2.0f);
    }

    // 3. Train Boarding from Queue
    if (train.CanBoard() && !queuePeeps.empty()) {
        size_t peepIdx = queuePeeps.front();
        if (train.BoardPassenger(peeps[peepIdx].shirtColor)) {
            peeps[peepIdx].state = PEEP_RIDING;
            queuePeeps.erase(queuePeeps.begin());

            // Check nausea for queasy peeps
            if (peeps[peepIdx].type == PEEP_QUEASY) {
                peeps[peepIdx].nausea = 100.0f;
            }
        }
    }

    // 4. Update each peep
    queuePeeps.clear(); // Rebuild queue indices

    for (size_t i = 0; i < peeps.size();) {
        auto& p = peeps[i];
        p.walkTimer += dt * 6.0f;

        switch (p.state) {
            case PEEP_WALKING_TO_RIDE: {
                UpdatePeepMovement(p, dt);
                // Arrived at queue entrance
                if (Vector2Distance(p.pos, p.targetPos) < 0.2f) {
                    if ((int)queuePeeps.size() < maxQueueCap) {
                        p.state = PEEP_IN_QUEUE;
                    } else {
                        // Queue physically full: wander temporarily
                        p.targetPos = {p.pos.x + (((float)rand()/RAND_MAX)-0.5f)*3.0f, p.pos.y + (((float)rand()/RAND_MAX)-0.5f)*3.0f};
                        p.state = PEEP_WANDERING;
                    }
                }
                break;
            }

            case PEEP_IN_QUEUE: {
                // Determine slot position in queue line
                int qPos = (int)queuePeeps.size();
                queuePeeps.push_back(i);
                Vector2 targetSlot = {queueStartPos.x + (float)qPos * 0.35f, queueStartPos.y};
                p.pos = Vector2Lerp(p.pos, targetSlot, dt * 5.0f);
                break;
            }

            case PEEP_RIDING: {
                // Invisible while riding inside train car
                break;
            }

            case PEEP_EXITING: {
                UpdatePeepMovement(p, dt);
                if (p.nausea >= 90.0f) {
                    // Sick peep vomits on path!
                    particles.SpawnVomit(Vector3{p.pos.x, p.pos.y, 0.0f}, 14);
                    if (outMesses.size() < 40) {
                        ParkMess m;
                        m.pos = p.pos;
                        m.isVomit = true;
                        m.timer = 0.0f;
                        outMesses.push_back(m);
                    }
                    p.nausea = 0.0f;
                    p.thought = "Ugh, I feel so sick... *blarf*";
                }
                if (Vector2Distance(p.pos, p.targetPos) < 0.3f) {
                    // Ride again or wander
                    p.targetPos = queueStartPos;
                    p.state = PEEP_WALKING_TO_RIDE;
                }
                break;
            }

            case PEEP_WANDERING: {
                UpdatePeepMovement(p, dt);
                if (Vector2Distance(p.pos, p.targetPos) < 0.2f) {
                    p.targetPos = queueStartPos;
                    p.state = PEEP_WALKING_TO_RIDE;
                }
                break;
            }

            case PEEP_LEAVING_ANGRY: {
                UpdatePeepMovement(p, dt);
                if (Vector2Distance(p.pos, entrancePos) < 0.4f) {
                    // Departed the park
                    peeps.erase(peeps.begin() + i);
                    continue;
                }
                break;
            }

            default:
                break;
        }

        ++i;
    }
}

void PeepManager::UpdatePeepMovement(Peep& p, float dt) {
    Vector2 dir = Vector2Subtract(p.targetPos, p.pos);
    float dist = Vector2Length(dir);
    if (dist > 0.05f) {
        dir = Vector2Normalize(dir);
        p.pos.x += dir.x * p.speed * dt;
        p.pos.y += dir.y * p.speed * dt;
    }
}

void PeepManager::Draw(Vector2 camOffset, float zoom) const {
    for (const auto& p : peeps) {
        if (p.state == PEEP_RIDING) continue; // Inside train

        Vector2 sPos = Iso::GridToScreen(p.pos.x, p.pos.y, 0.0f, camOffset, zoom);
        float bob = (p.state != PEEP_IN_QUEUE) ? sinf(p.walkTimer) * 1.5f * zoom : 0.0f;

        // Feet shadow
        DrawEllipse((int)sPos.x, (int)sPos.y, 4.0f * zoom, 2.0f * zoom, Color{0, 0, 0, 70});

        float bodyH = 10.0f * zoom;
        float bodyW = 5.0f * zoom;

        // Legs / Pants
        DrawRectangle((int)(sPos.x - bodyW * 0.4f), (int)(sPos.y - bodyH * 0.4f + bob), (int)(bodyW * 0.8f), (int)(bodyH * 0.4f), p.pantsColor);

        // Torso / Shirt
        DrawRectangle((int)(sPos.x - bodyW / 2.0f), (int)(sPos.y - bodyH + bob), (int)bodyW, (int)(bodyH * 0.6f), p.shirtColor);

        // Head (Skin tone)
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 3.0f * zoom + bob), 2.8f * zoom, Color{255, 224, 178, 255});

        // Cap / Hat matching shirt
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 4.5f * zoom + bob), 2.2f * zoom, p.shirtColor);

        // Floating helium balloon
        if (p.hasBalloon) {
            float balloonSway = sinf(p.walkTimer * 2.5f) * 2.5f * zoom;
            Vector2 bPos = { sPos.x + 5.0f * zoom + balloonSway, sPos.y - bodyH - 15.0f * zoom + bob * 0.5f };
            DrawLineEx({sPos.x + 2.0f * zoom, sPos.y - bodyH * 0.4f + bob}, bPos, 1.0f * zoom, Color{180, 180, 180, 220});
            DrawEllipse((int)bPos.x, (int)bPos.y, 4.0f * zoom, 5.0f * zoom, p.balloonColor);
            DrawCircle((int)(bPos.x - 1.2f * zoom), (int)(bPos.y - 1.5f * zoom), 1.0f * zoom, WHITE);
        }

        // Angry red icon above head if furious
        if (p.isAngry) {
            DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 11.0f * zoom), 3.5f * zoom, Color{244, 67, 54, 255});
            DrawText("!", (int)(sPos.x - 2.0f * zoom), (int)(sPos.y - bodyH - 14.0f * zoom), (int)(6.0f * zoom), WHITE);
        }
    }

    // Draw Circular Radial Overcrowding Clock (Mini Metro Triage Icon) above station
    if (overcrowdActive) {
        Vector2 clockPos = Iso::GridToScreen(queueStartPos.x, queueStartPos.y, 1.2f, camOffset, zoom);
        float radius = 16.0f * zoom;

        // Background dark circle
        DrawCircle((int)clockPos.x, (int)clockPos.y, radius, Color{33, 33, 33, 220});
        DrawCircleLines((int)clockPos.x, (int)clockPos.y, radius, Color{255, 255, 255, 200});

        // Ticking pie sector (Red clock winding down)
        float pct = 1.0f - (overcrowdTimer / MAX_OVERCROWD_TIME);
        float endAngle = -90.0f + pct * 360.0f;
        DrawCircleSector(clockPos, radius - 2.0f * zoom, -90.0f, endAngle, 24, Color{244, 67, 54, 230});

        // Exclamation alert in center
        DrawText("!", (int)(clockPos.x - 3.0f * zoom), (int)(clockPos.y - 6.0f * zoom), (int)(12.0f * zoom), WHITE);
    }
}

int PeepManager::FindPeepAtScreenPos(Vector2 mouseScreen, Vector2 camOffset, float zoom) const {
    for (size_t i = 0; i < peeps.size(); ++i) {
        if (peeps[i].state == PEEP_RIDING) continue;
        Vector2 sPos = Iso::GridToScreen(peeps[i].pos.x, peeps[i].pos.y, 0.0f, camOffset, zoom);
        Vector2 peepCenter = {sPos.x, sPos.y - 8.0f * zoom};
        if (Vector2Distance(mouseScreen, peepCenter) < 14.0f * zoom) {
            return (int)i;
        }
    }
    return -1;
}

void PeepManager::CheckSceneryInteractions(const SceneryType scenery[GRID_SIZE][GRID_SIZE], ParticleSystem& particles, float& outMoney, std::vector<ParkMess>& outMesses) {
    for (auto& p : peeps) {
        if (p.state == PEEP_RIDING) continue;
        int gx = (int)roundf(p.pos.x);
        int gy = (int)roundf(p.pos.y);
        if (gx < 0 || gx >= GRID_SIZE || gy < 0 || gy >= GRID_SIZE) continue;

        SceneryType s = scenery[gx][gy];
        if (s == SCENERY_DRINK_STALL) {
            if (p.nausea > 30.0f || p.happiness < 70.0f) {
                p.nausea = std::max(0.0f, p.nausea - 50.0f);
                p.happiness = std::min(100.0f, p.happiness + 20.0f);
                p.thought = "That cold soda was so refreshing!";
                outMoney += 2.5f;
                AudioManager::Play(SFX_SODA_SLURP, 0.6f);
                particles.SpawnSparks(Vector3{p.pos.x, p.pos.y, 0.2f}, 4);

                // 35% chance to drop cup litter
                if ((rand() % 100) < 35 && outMesses.size() < 40) {
                    ParkMess cup;
                    cup.pos = {p.pos.x + (((float)rand()/RAND_MAX)-0.5f)*0.3f, p.pos.y + (((float)rand()/RAND_MAX)-0.5f)*0.3f};
                    cup.isVomit = false;
                    cup.timer = 0.0f;
                    outMesses.push_back(cup);
                }
            }
        } else if (s == SCENERY_BENCH) {
            if (p.nausea > 20.0f) {
                p.nausea = std::max(0.0f, p.nausea - 10.0f);
                p.thought = "Resting on the bench helps my stomach.";
            }
        } else if (s == SCENERY_FOUNTAIN) {
            p.happiness = std::min(100.0f, p.happiness + 0.2f);
            p.thought = "The fountain is so soothing and beautiful!";
        } else if (s == SCENERY_FLOWER_BED) {
            p.happiness = std::min(100.0f, p.happiness + 0.15f);
            p.thought = "The flowerbeds smell delightful!";
        } else if (s == SCENERY_BALLOON_STALL) {
            if (!p.hasBalloon && (rand() % 100) < 30) {
                p.hasBalloon = true;
                p.happiness = 100.0f;
                outMoney += 3.5f;
                Color bCols[] = {Color{239, 68, 68, 255}, Color{59, 130, 246, 255}, Color{234, 179, 8, 255}, Color{168, 85, 247, 255}};
                p.balloonColor = bCols[rand() % 4];
                p.thought = "Look at my awesome balloon!";
                AudioManager::Play(SFX_DELIVERY_CHIME, 0.5f);
            }
        }

        // React to nearby messes
        for (const auto& m : outMesses) {
            if (Vector2Distance(p.pos, m.pos) < 0.65f) {
                if (m.isVomit) {
                    p.happiness = std::max(0.0f, p.happiness - 0.4f);
                    p.nausea = std::min(100.0f, p.nausea + 0.2f);
                    p.thought = "Gross! Someone threw up on the path!";
                } else {
                    p.happiness = std::max(0.0f, p.happiness - 0.15f);
                    p.thought = "Trash on the path? Where are the cleaners?";
                }
                break;
            }
        }
    }
}

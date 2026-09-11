#include "peep.hpp"
#include "isometric.hpp"
#include "audio.hpp"

CommuterManager::CommuterManager() {
    Init({2.0f, 12.0f}, {7.0f, 12.0f}, {8.0f, 13.0f});
}

void CommuterManager::Init(Vector2 stationEntrance, Vector2 platformEntrance, Vector2 stationExit) {
    entrancePos = stationEntrance;
    queueStartPos = platformEntrance;
    exitPos = stationExit;
    commuters.clear();
    queueCommuters.clear();
    overcrowdActive = false;
    overcrowdTimer = MAX_OVERCROWD_TIME;

    // Initial pre-spawned city commuters
    for (int i = 0; i < 9; ++i) {
        SpawnCommuter();
    }
}

void CommuterManager::SpawnCommuter() {
    static const struct Profile {
        const char* name;
        const char* occupation;
        CommuterType type;
    } profiles[] = {
        {"Kenji Sato", "Software Architect", COMMUTER_WORKER},
        {"Elena Rostova", "Financial Analyst", COMMUTER_EXECUTIVE},
        {"Marcus Vance", "Urban Planner", COMMUTER_WORKER},
        {"Aoi Tanaka", "Graphic Designer", COMMUTER_WORKER},
        {"Liam Chen", "Biotech Researcher", COMMUTER_STUDENT},
        {"Sophia Dubois", "Operations Director", COMMUTER_EXECUTIVE},
        {"David Kim", "Data Scientist", COMMUTER_WORKER},
        {"Chloe Martin", "Architecture Student", COMMUTER_STUDENT},
        {"Mateo Silva", "Photojournalist", COMMUTER_TOURIST},
        {"Yuki Mori", "Medical Resident", COMMUTER_STUDENT},
        {"Lucas Weber", "Structural Engineer", COMMUTER_WORKER},
        {"Zoe Al-Mansoor", "Diplomatic Envoy", COMMUTER_EXECUTIVE},
        {"James O'Connor", "Sound Designer", COMMUTER_WORKER},
        {"Nora Lindqvist", "Environmental Chemist", COMMUTER_STUDENT},
        {"Arjun Patel", "Financial Trader", COMMUTER_EXECUTIVE},
        {"Mei Lin", "Museum Curator", COMMUTER_TOURIST}
    };

    int pIdx = rand() % 16;
    Commuter c;
    c.name = profiles[pIdx].name;
    c.occupation = profiles[pIdx].occupation;
    c.type = profiles[pIdx].type;
    c.pos = entrancePos;
    c.targetPos = queueStartPos;
    c.state = COMMUTER_ENTERING;
    c.walkTimer = ((float)rand() / RAND_MAX) * 2.0f * PI;
    c.happiness = 88.0f + (float)(rand() % 12);
    c.metroPassBalance = 20.0f + (float)(rand() % 40);
    c.hasBriefcase = (c.type == COMMUTER_WORKER || c.type == COMMUTER_EXECUTIVE);
    c.hasCoffee = (rand() % 100 < 30);

    // Assign Mini Metro Destination Shape (Circle, Triangle, Square, Cross)
    int rShape = 1 + (rand() % 4);
    c.targetShape = static_cast<StationShape>(rShape);

    if (c.type == COMMUTER_EXECUTIVE) {
        c.shirtColor = Color{30, 41, 59, 255}; // Charcoal suit
        c.pantsColor = Color{15, 23, 42, 255};
        c.thought = "Heading to Financial CBD [Square]";
    } else if (c.type == COMMUTER_STUDENT) {
        c.shirtColor = Color{37, 99, 235, 255}; // Royal blue jacket
        c.pantsColor = Color{71, 85, 105, 255};
        c.thought = "Rushing to Campus [Cross] for lecture!";
    } else if (c.type == COMMUTER_TOURIST) {
        c.shirtColor = Color{234, 88, 12, 255}; // Bright orange windbreaker
        c.pantsColor = Color{100, 116, 139, 255};
        c.thought = "Exploring the Waterfront [Triangle]!";
    } else {
        c.shirtColor = Color{13, 148, 136, 255}; // Teal blazer
        c.pantsColor = Color{51, 65, 85, 255};
        c.thought = "Line 1 is always fast and on time.";
    }

    commuters.push_back(c);
}

void CommuterManager::AlightPassengers(int count, Vector2 stationPos) {
    int leftToAlight = count;
    for (auto& c : commuters) {
        if (leftToAlight <= 0) break;
        if (c.state == COMMUTER_RIDING) {
            c.state = COMMUTER_ALIGHTING;
            c.pos = stationPos;
            c.targetPos = exitPos;
            c.happiness = std::min(100.0f, c.happiness + 20.0f);
            static const char* thoughts[] = {
                "Smooth and rapid transit! Love Line 1!",
                "Stunning view of the river from the viaduct!",
                "Quick and comfortable commute!",
                "Metro Grid is running like clockwork!",
                "Great value for money, arrived right on time!"
            };
            c.thought = thoughts[rand() % 5];
            leftToAlight--;
        }
    }
}

float CommuterManager::GetQueueOvercrowdRatio() const {
    return (float)queueCommuters.size() / (float)maxQueueCap;
}

void CommuterManager::Update(float dt, MetroTrain& train, ParticleSystem& particles, float& outSatisfaction, int& outAngryLeaves, std::vector<StationMess>& outMesses) {
    (void)outMesses;

    // 1. Spawning Commuters
    spawnTimer -= dt;
    if (spawnTimer <= 0.0f) {
        if ((int)commuters.size() < 65) {
            SpawnCommuter();
        }
        spawnTimer = spawnInterval;
    }

    // 2. Station Overcrowding Triage Clock (Mini Metro)
    if (queueCommuters.size() >= 10) {
        overcrowdActive = true;
        overcrowdTimer -= dt;

        if (fmodf(overcrowdTimer, 1.8f) < dt) {
            AudioManager::Play(SFX_QUEUE_ALARM, 0.45f);
        }

        // Triage Countdown expired: Service Failure / Platform Evacuation!
        if (overcrowdTimer <= 0.0f) {
            overcrowdActive = false;
            overcrowdTimer = MAX_OVERCROWD_TIME;
            outSatisfaction = std::max(0.0f, outSatisfaction - 10.0f);

            for (size_t idx : queueCommuters) {
                commuters[idx].state = COMMUTER_LEAVING_ANGRY;
                commuters[idx].targetPos = entrancePos;
                commuters[idx].isAngry = true;
                commuters[idx].thought = "Platform overcrowded! Taking a cab!";
                outAngryLeaves++;
            }
            queueCommuters.clear();
        }
    } else {
        overcrowdActive = false;
        overcrowdTimer = std::min(MAX_OVERCROWD_TIME, overcrowdTimer + dt * 2.2f);
    }

    // 3. Boarding Train from Platform Queue
    if (train.CanBoard() && train.GetOperatingMode() == LINE_OPEN && !queueCommuters.empty()) {
        size_t cIdx = queueCommuters.front();
        if (train.BoardCommuter(commuters[cIdx].targetShape, commuters[cIdx].shirtColor)) {
            commuters[cIdx].state = COMMUTER_RIDING;
            commuters[cIdx].thought = "Boarded Line 1 EMU!";
            queueCommuters.erase(queueCommuters.begin());
            AudioManager::Play(SFX_SMARTCARD_BEEP, 0.4f);
            particles.SpawnSparks(Vector3{commuters[cIdx].pos.x, commuters[cIdx].pos.y, 0.2f}, 4);
        }
    }

    // 4. Update each commuter state & movement
    queueCommuters.clear();

    for (size_t i = 0; i < commuters.size();) {
        auto& c = commuters[i];
        c.walkTimer += dt * 6.5f;

        switch (c.state) {
            case COMMUTER_ENTERING: {
                UpdateCommuterMovement(c, dt);
                if (Vector2Distance(c.pos, c.targetPos) < 0.25f) {
                    c.state = COMMUTER_SWIPING_GATE;
                    c.queueTimer = 0.45f;
                    AudioManager::Play(SFX_SMARTCARD_BEEP, 0.5f);
                    particles.SpawnSparks(Vector3{c.pos.x, c.pos.y, 0.4f}, 3);
                }
                break;
            }

            case COMMUTER_SWIPING_GATE: {
                c.queueTimer -= dt;
                if (c.queueTimer <= 0.0f) {
                    c.state = COMMUTER_ON_PLATFORM;
                    c.targetPos = {queueStartPos.x + 0.5f, queueStartPos.y};
                }
                break;
            }

            case COMMUTER_ON_PLATFORM: {
                int qPos = (int)queueCommuters.size();
                queueCommuters.push_back(i);

                // Line up along the tactile floor edge
                Vector2 slotPos = {
                    queueStartPos.x - (float)qPos * 0.4f,
                    queueStartPos.y + (float)(qPos % 2) * 0.25f
                };
                c.targetPos = slotPos;
                UpdateCommuterMovement(c, dt);

                c.queuePatience -= dt;
                if (c.queuePatience < 15.0f) {
                    c.thought = "Train is delayed... checking transit app.";
                    c.happiness = std::max(20.0f, c.happiness - dt * 1.5f);
                }
                break;
            }

            case COMMUTER_RIDING: {
                // Inside the train
                c.pos = {train.GetLocomotivePos().x, train.GetLocomotivePos().y};
                break;
            }

            case COMMUTER_ALIGHTING: {
                c.state = COMMUTER_EXITING_STATION;
                c.targetPos = entrancePos;
                c.happiness = 100.0f;
                c.thought = "Arrived safely at my destination!";
                break;
            }

            case COMMUTER_EXITING_STATION:
            case COMMUTER_LEAVING_ANGRY: {
                UpdateCommuterMovement(c, dt);
                if (Vector2Distance(c.pos, entrancePos) < 0.3f) {
                    commuters.erase(commuters.begin() + i);
                    continue;
                }
                break;
            }

            default: break;
        }

        ++i;
    }
}

void CommuterManager::UpdateCommuterMovement(Commuter& c, float dt) {
    Vector2 dir = Vector2Subtract(c.targetPos, c.pos);
    float dist = Vector2Length(dir);

    if (dist > 0.08f) {
        dir = Vector2Normalize(dir);
        c.pos.x += dir.x * c.speed * dt;
        c.pos.y += dir.y * c.speed * dt;
    } else {
        c.pos = c.targetPos;
    }
}

void CommuterManager::CheckSceneryInteractions(const SceneryType scenery[GRID_SIZE][GRID_SIZE], ParticleSystem& particles, float& outFunds, std::vector<StationMess>& outMesses) {
    for (auto& c : commuters) {
        if (c.state == COMMUTER_RIDING) continue;
        int gx = (int)roundf(c.pos.x);
        int gy = (int)roundf(c.pos.y);
        if (gx < 0 || gx >= GRID_SIZE || gy < 0 || gy >= GRID_SIZE) continue;

        SceneryType s = scenery[gx][gy];
        if (s == SCENERY_NEWSSTAND && !c.hasCoffee) {
            c.hasCoffee = true;
            c.happiness = std::min(100.0f, c.happiness + 15.0f);
            c.thought = "Got an espresso from the Metro Cafe!";
            outFunds += 3.50f;
            AudioManager::Play(SFX_COFFEE_SIP, 0.6f);
            particles.SpawnSparks(Vector3{c.pos.x, c.pos.y, 0.2f}, 4);

            // 15% chance to drop paper cup
            if ((rand() % 100) < 15 && outMesses.size() < 30) {
                StationMess mess;
                mess.pos = {c.pos.x + (((float)rand()/RAND_MAX)-0.5f)*0.3f, c.pos.y + (((float)rand()/RAND_MAX)-0.5f)*0.3f};
                mess.isSpill = true;
                mess.timer = 0.0f;
                outMesses.push_back(mess);
            }
        } else if (s == SCENERY_BENCH) {
            c.happiness = std::min(100.0f, c.happiness + 0.1f);
            c.thought = "Resting on the station bench.";
        } else if (s == SCENERY_MAP_KIOSK) {
            c.thought = "Checking the Harry Beck route diagram.";
        }
    }
}

void CommuterManager::Draw(Vector2 camOffset, float zoom) const {
    for (const auto& c : commuters) {
        if (c.state == COMMUTER_RIDING) continue;

        Vector2 sPos = Iso::GridToScreen(c.pos.x, c.pos.y, 0.0f, camOffset, zoom);
        float bob = sinf(c.walkTimer) * 1.5f * zoom;

        // Commuter Foot Shadow
        DrawEllipse((int)sPos.x, (int)sPos.y, 4.0f * zoom, 2.0f * zoom, Color{0, 0, 0, 75});

        float bodyH = 10.0f * zoom;
        float bodyW = 5.0f * zoom;

        // Slacks / Trousers
        DrawRectangle((int)(sPos.x - bodyW * 0.4f), (int)(sPos.y - bodyH * 0.45f + bob), (int)(bodyW * 0.8f), (int)(bodyH * 0.45f), c.pantsColor);

        // Blazer / Coat
        DrawRectangle((int)(sPos.x - bodyW * 0.5f), (int)(sPos.y - bodyH + bob), (int)bodyW, (int)(bodyH * 0.6f), c.shirtColor);

        // Head (Skin tone)
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 2.8f * zoom + bob), 2.5f * zoom, Color{255, 224, 178, 255});

        // Hair / Cap
        DrawCircle((int)sPos.x, (int)(sPos.y - bodyH - 4.2f * zoom + bob), 2.0f * zoom, Color{51, 65, 85, 255});

        // Briefcase / Bag
        if (c.hasBriefcase) {
            Vector2 bagPos = { sPos.x + 3.5f * zoom, sPos.y - bodyH * 0.4f + bob };
            DrawRectangle((int)bagPos.x, (int)bagPos.y, (int)(3.5f * zoom), (int)(2.8f * zoom), Color{120, 53, 15, 255}); // Leather briefcase
        }

        // Floating Thought & Destination Shape Speech Bubble
        float floatY = sinf(c.walkTimer * 2.0f) * 1.5f * zoom;
        Vector2 badgePos = { sPos.x, sPos.y - bodyH - 12.0f * zoom + bob + floatY };
        Color badgeColor = GetShapeColor(c.targetShape);

        // Speech bubble pointer tail
        Vector2 tailTip = { sPos.x, sPos.y - bodyH - 6.0f * zoom + bob };
        Vector2 b1 = { sPos.x - 2.5f * zoom, sPos.y - bodyH - 9.0f * zoom + bob };
        Vector2 b2 = { sPos.x + 2.5f * zoom, sPos.y - bodyH - 9.0f * zoom + bob };
        DrawTriangle(tailTip, b2, b1, Color{15, 23, 42, 235});

        // Speech bubble circular body
        float bRad = 5.2f * zoom;
        DrawCircle((int)badgePos.x, (int)badgePos.y, bRad, Color{15, 23, 42, 245});
        DrawCircleLines((int)badgePos.x, (int)badgePos.y, bRad, badgeColor);
        DrawCircle((int)badgePos.x, (int)badgePos.y, bRad - 1.2f * zoom, badgeColor);

        // Crisp White Shape Glyph
        if (c.targetShape == SHAPE_SQUARE) {
            DrawRectangle((int)(badgePos.x - 1.8f * zoom), (int)(badgePos.y - 1.8f * zoom), (int)(3.6f * zoom), (int)(3.6f * zoom), WHITE);
        } else if (c.targetShape == SHAPE_TRIANGLE) {
            DrawTriangle({badgePos.x, badgePos.y - 2.4f * zoom}, {badgePos.x - 2.2f * zoom, badgePos.y + 2.2f * zoom}, {badgePos.x + 2.2f * zoom, badgePos.y + 2.2f * zoom}, WHITE);
        } else if (c.targetShape == SHAPE_CROSS) {
            DrawRectangle((int)(badgePos.x - 0.8f * zoom), (int)(badgePos.y - 2.2f * zoom), (int)(1.6f * zoom), (int)(4.4f * zoom), WHITE);
            DrawRectangle((int)(badgePos.x - 2.2f * zoom), (int)(badgePos.y - 0.8f * zoom), (int)(4.4f * zoom), (int)(1.6f * zoom), WHITE);
        } else if (c.targetShape == SHAPE_CIRCLE) {
            DrawCircle((int)badgePos.x, (int)badgePos.y, 2.2f * zoom, WHITE);
            DrawCircle((int)badgePos.x, (int)badgePos.y, 1.2f * zoom, badgeColor);
        }
    }

    // Mini Metro Radial Overcrowding Triage Clock above station
    if (overcrowdActive) {
        Vector2 clockPos = Iso::GridToScreen(queueStartPos.x, queueStartPos.y, 1.5f, camOffset, zoom);
        float radius = 18.0f * zoom;

        // Glowing red alert halo
        DrawCircleGradient(clockPos, radius * 1.8f, Color{239, 68, 68, 160}, Color{239, 68, 68, 0});

        DrawCircle((int)clockPos.x, (int)clockPos.y, radius, Color{15, 23, 42, 245});
        DrawCircleLines((int)clockPos.x, (int)clockPos.y, radius, WHITE);

        float pct = 1.0f - (overcrowdTimer / MAX_OVERCROWD_TIME);
        float endAngle = -90.0f + pct * 360.0f;
        DrawCircleSector(clockPos, radius - 2.0f * zoom, -90.0f, endAngle, 32, Color{239, 68, 68, 240});

        int secsLeft = (int)ceilf(overcrowdTimer);
        DrawText(TextFormat("%d", secsLeft), (int)(clockPos.x - 3.5f * zoom), (int)(clockPos.y - 5.0f * zoom), (int)(11.0f * zoom), WHITE);
    }
}

int CommuterManager::FindCommuterAtScreenPos(Vector2 mouseScreen, Vector2 camOffset, float zoom) const {
    for (size_t i = 0; i < commuters.size(); ++i) {
        if (commuters[i].state == COMMUTER_RIDING) continue;
        Vector2 sPos = Iso::GridToScreen(commuters[i].pos.x, commuters[i].pos.y, 0.0f, camOffset, zoom);
        Vector2 center = {sPos.x, sPos.y - 8.0f * zoom};
        if (Vector2Distance(mouseScreen, center) < 14.0f * zoom) {
            return (int)i;
        }
    }
    return -1;
}

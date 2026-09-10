#include "audio.hpp"
#include <map>
#include <cmath>

namespace AudioManager {

static bool initialized = false;
static bool muted = false;
static std::map<SfxType, Sound> sounds;

// Helper to construct procedural sound from generated float samples
static Sound CreateProceduralSound(const std::vector<float>& samples, int sampleRate = 44100) {
    int count = (int)samples.size();
    short* data = new short[count];
    for (int i = 0; i < count; ++i) {
        float s = std::max(-1.0f, std::min(1.0f, samples[i]));
        data[i] = (short)(s * 32767.0f);
    }
    Wave wave = {
        .frameCount = (unsigned int)count,
        .sampleRate = (unsigned int)sampleRate,
        .sampleSize = 16,
        .channels = 1,
        .data = data
    };
    Sound snd = LoadSoundFromWave(wave);
    delete[] data;
    return snd;
}

void Init() {
    if (initialized) return;
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
    }
    if (!IsAudioDeviceReady()) return;

    int sr = 44100;

    // 1. SFX_CHAIN_CLICK: Short metallic ratchet click (0.035s)
    {
        int n = (int)(sr * 0.035f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = 1.0f - (float)i / n;
            float wave1 = sinf(2.0f * PI * 1800.0f * t);
            float wave2 = sinf(2.0f * PI * 3200.0f * t);
            buf[i] = (wave1 * 0.6f + wave2 * 0.4f) * env * env;
        }
        sounds[SFX_CHAIN_CLICK] = CreateProceduralSound(buf, sr);
    }

    // 2. SFX_WHOOSH: Low-pass filtered noise rush (0.35s)
    {
        int n = (int)(sr * 0.35f);
        std::vector<float> buf(n);
        float last = 0.0f;
        for (int i = 0; i < n; ++i) {
            float t = (float)i / n;
            float env = sinf(t * PI); // Smooth bell curve
            float white = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            last += 0.15f * (white - last); // Simple low pass
            buf[i] = last * env * 0.8f;
        }
        sounds[SFX_WHOOSH] = CreateProceduralSound(buf, sr);
    }

    // 3. SFX_STATION_BELL: Clean double chime (0.4s)
    {
        int n = (int)(sr * 0.4f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = expf(-6.0f * t);
            float tone1 = sinf(2.0f * PI * 880.0f * t);
            float tone2 = sinf(2.0f * PI * 1760.0f * t);
            buf[i] = (tone1 * 0.7f + tone2 * 0.3f) * env;
        }
        sounds[SFX_STATION_BELL] = CreateProceduralSound(buf, sr);
    }

    // 4. SFX_DELIVERY_CHIME: Pentatonic bright arpeggio (C-E-G-C, 0.3s)
    {
        int n = (int)(sr * 0.35f);
        std::vector<float> buf(n);
        float freqs[4] = {523.25f, 659.25f, 783.99f, 1046.50f};
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float subT = t * 4.0f;
            int note = std::min(3, (int)subT);
            float noteT = fmodf(subT, 1.0f) * 0.0875f;
            float env = expf(-12.0f * noteT);
            buf[i] = sinf(2.0f * PI * freqs[note] * t) * env * 0.6f;
        }
        sounds[SFX_DELIVERY_CHIME] = CreateProceduralSound(buf, sr);
    }

    // 5. SFX_QUEUE_ALARM: Urgent high dual beep (0.2s)
    {
        int n = (int)(sr * 0.2f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float freq = (t < 0.1f) ? 950.0f : 1200.0f;
            (void)t;
            buf[i] = sinf(2.0f * PI * freq * t) * 0.5f;
        }
        sounds[SFX_QUEUE_ALARM] = CreateProceduralSound(buf, sr);
    }

    // 6. SFX_CRASH: Low explosion with pitch drop & noisy crunch (0.6s)
    {
        int n = (int)(sr * 0.6f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = expf(-4.0f * t);
            float freq = 160.0f * expf(-8.0f * t) + 40.0f;
            float noise = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            buf[i] = (sinf(2.0f * PI * freq * t) * 0.6f + noise * 0.4f) * env;
        }
        sounds[SFX_CRASH] = CreateProceduralSound(buf, sr);
    }

    // 7. SFX_UPGRADE_FANFARE: Grand rising fanfare chord (0.6s)
    {
        int n = (int)(sr * 0.6f);
        std::vector<float> buf(n);
        float f1 = 440.0f, f2 = 554.37f, f3 = 659.25f, f4 = 880.0f;
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = (t < 0.1f) ? (t / 0.1f) : expf(-2.5f * (t - 0.1f));
            float s = sinf(2.0f * PI * f1 * t) * 0.3f +
                      sinf(2.0f * PI * f2 * t) * 0.3f +
                      sinf(2.0f * PI * f3 * t) * 0.3f +
                      sinf(2.0f * PI * f4 * t) * 0.3f;
            buf[i] = s * env * 0.7f;
        }
        sounds[SFX_UPGRADE_FANFARE] = CreateProceduralSound(buf, sr);
    }

    // 8. SFX_BUTTON_CLICK: Subtle UI click (0.05s)
    {
        int n = (int)(sr * 0.05f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            buf[i] = sinf(2.0f * PI * 1400.0f * t) * expf(-30.0f * t) * 0.4f;
        }
        sounds[SFX_BUTTON_CLICK] = CreateProceduralSound(buf, sr);
    }

    // 9. SFX_CASH_REGISTER: "Ka-ching!" metallic chime (0.28s)
    {
        int n = (int)(sr * 0.28f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float s = 0.0f;
            if (t < 0.08f) {
                s = sinf(2.0f * PI * 987.0f * t) * expf(-25.0f * t);
            } else {
                float t2 = t - 0.08f;
                s = (sinf(2.0f * PI * 1318.0f * t2) * 0.7f + sinf(2.0f * PI * 1975.0f * t2) * 0.3f) * expf(-14.0f * t2);
            }
            buf[i] = s * 0.5f;
        }
        sounds[SFX_CASH_REGISTER] = CreateProceduralSound(buf, sr);
    }

    // 10. SFX_CONSTRUCTION: Mallet / steel rail lock thud (0.12s)
    {
        int n = (int)(sr * 0.12f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float s = sinf(2.0f * PI * 160.0f * t) * expf(-35.0f * t) +
                      sinf(2.0f * PI * 480.0f * t) * expf(-50.0f * t) * 0.4f;
            buf[i] = s * 0.6f;
        }
        sounds[SFX_CONSTRUCTION] = CreateProceduralSound(buf, sr);
    }

    // 11. SFX_BULLDOZE: Rubble crumble scrape (0.25s)
    {
        int n = (int)(sr * 0.25f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float noise = (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
            float s = noise * expf(-10.0f * t) * (0.5f + 0.5f * sinf(2.0f * PI * 40.0f * t));
            buf[i] = s * 0.5f;
        }
        sounds[SFX_BULLDOZE] = CreateProceduralSound(buf, sr);
    }

    // 12. SFX_PEEP_CHEER: Thrill crowd cheer (0.6s)
    {
        int n = (int)(sr * 0.6f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = sinf(t / 0.6f * PI);
            float noise = (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
            float s = (sinf(2.0f * PI * 340.0f * t) * 0.3f + sinf(2.0f * PI * 520.0f * t) * 0.2f + noise * 0.3f) * env;
            buf[i] = s * 0.5f;
        }
        sounds[SFX_PEEP_CHEER] = CreateProceduralSound(buf, sr);
    }

    // 13. SFX_SODA_SLURP: Refreshing fizz (0.3s)
    {
        int n = (int)(sr * 0.3f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float noise = (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
            buf[i] = noise * expf(-8.0f * t) * 0.35f;
        }
        sounds[SFX_SODA_SLURP] = CreateProceduralSound(buf, sr);
    }

    initialized = true;
}

void Cleanup() {
    if (!initialized) return;
    for (auto& pair : sounds) {
        UnloadSound(pair.second);
    }
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
    initialized = false;
}

void Play(SfxType type, float volume) {
    if (!initialized || muted) return;
    auto it = sounds.find(type);
    if (it != sounds.end()) {
        SetSoundVolume(it->second, volume);
        PlaySound(it->second);
    }
}

void SetMute(bool isMuted) { muted = isMuted; }
bool IsMuted() { return muted; }

} // namespace AudioManager

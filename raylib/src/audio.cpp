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

    // 1. SFX_DOOR_CHIME: Japanese/European Metro 4-note melodic departure chime (E5, G#5, B5, E6)
    {
        int n = (int)(sr * 0.52f);
        std::vector<float> buf(n);
        float freqs[4] = {659.25f, 830.61f, 987.77f, 1318.51f};
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float stepT = t * 7.5f;
            int note = std::min(3, (int)stepT);
            float noteT = fmodf(stepT, 1.0f) * 0.133f;
            float env = expf(-8.5f * noteT);
            float harmonic = sinf(2.0f * PI * freqs[note] * 2.0f * t) * 0.15f;
            buf[i] = (sinf(2.0f * PI * freqs[note] * t) * 0.85f + harmonic) * env * 0.65f;
        }
        sounds[SFX_DOOR_CHIME] = CreateProceduralSound(buf, sr);
    }

    // 2. SFX_VVVF_MOTOR: Electric traction inverter motor whine sweeping up in frequency
    {
        int n = (int)(sr * 0.65f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float progress = (float)i / n;
            float env = sinf(progress * PI);
            // Multi-harmonic carrier frequency rising in pitch
            float freqCarrier = 140.0f + 550.0f * powf(progress, 1.3f);
            float carrier = sinf(2.0f * PI * freqCarrier * t);
            float harmonic2 = sinf(2.0f * PI * (freqCarrier * 2.0f) * t) * 0.35f;
            float harmonic3 = sinf(2.0f * PI * (freqCarrier * 3.0f) * t) * 0.15f;
            buf[i] = (carrier + harmonic2 + harmonic3) * env * 0.45f;
        }
        sounds[SFX_VVVF_MOTOR] = CreateProceduralSound(buf, sr);
    }

    // 3. SFX_AIR_BRAKE: Pneumatic brake pressure release hiss (0.38s)
    {
        int n = (int)(sr * 0.38f);
        std::vector<float> buf(n);
        float last = 0.0f;
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = expf(-6.0f * t);
            float white = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            last += 0.22f * (white - last); // Low-pass filter for air hiss
            buf[i] = last * env * 0.75f;
        }
        sounds[SFX_AIR_BRAKE] = CreateProceduralSound(buf, sr);
    }

    // 4. SFX_SMARTCARD_BEEP: High-frequency contactless IC card gate tap ("Pip-pip!") (0.12s)
    {
        int n = (int)(sr * 0.12f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float freq = (t < 0.05f) ? 2300.0f : (t > 0.06f ? 2800.0f : 0.0f);
            float env = (t < 0.05f) ? expf(-30.0f * t) : (t > 0.06f ? expf(-30.0f * (t - 0.06f)) : 0.0f);
            buf[i] = sinf(2.0f * PI * freq * t) * env * 0.5f;
        }
        sounds[SFX_SMARTCARD_BEEP] = CreateProceduralSound(buf, sr);
    }

    // 5. SFX_STATION_BELL: Clean double arrival chime (0.42s)
    {
        int n = (int)(sr * 0.42f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = expf(-6.5f * t);
            float tone1 = sinf(2.0f * PI * 880.0f * t);
            float tone2 = sinf(2.0f * PI * 1760.0f * t);
            buf[i] = (tone1 * 0.7f + tone2 * 0.3f) * env * 0.7f;
        }
        sounds[SFX_STATION_BELL] = CreateProceduralSound(buf, sr);
    }

    // 6. SFX_DELIVERY_CHIME: Pentatonic bright arpeggio for completed commute (0.35s)
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

    // 7. SFX_QUEUE_ALARM: Urgent high dual alert beep for platform congestion (0.2s)
    {
        int n = (int)(sr * 0.2f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float freq = (t < 0.1f) ? 1050.0f : 1350.0f;
            buf[i] = sinf(2.0f * PI * freq * t) * 0.5f;
        }
        sounds[SFX_QUEUE_ALARM] = CreateProceduralSound(buf, sr);
    }

    // 8. SFX_UPGRADE_FANFARE: Grand transit grant fanfare chord (0.6s)
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

    // 9. SFX_BUTTON_CLICK: Subtle OCC console click (0.05s)
    {
        int n = (int)(sr * 0.05f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            buf[i] = sinf(2.0f * PI * 1400.0f * t) * expf(-30.0f * t) * 0.4f;
        }
        sounds[SFX_BUTTON_CLICK] = CreateProceduralSound(buf, sr);
    }

    // 10. SFX_CONSTRUCTION: Steel rail and concrete sleeper placement clank (0.12s)
    {
        int n = (int)(sr * 0.12f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float s = sinf(2.0f * PI * 180.0f * t) * expf(-35.0f * t) +
                      sinf(2.0f * PI * 520.0f * t) * expf(-50.0f * t) * 0.45f;
            buf[i] = s * 0.6f;
        }
        sounds[SFX_CONSTRUCTION] = CreateProceduralSound(buf, sr);
    }

    // 11. SFX_BULLDOZE: Demolition rubble scrape (0.25s)
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

    // 12. SFX_COMMUTER_CHATTER: Warm platform boarding murmur (0.45s)
    {
        int n = (int)(sr * 0.45f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = sinf(t / 0.45f * PI);
            float noise = (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
            float s = (sinf(2.0f * PI * 320.0f * t) * 0.3f + sinf(2.0f * PI * 480.0f * t) * 0.2f + noise * 0.3f) * env;
            buf[i] = s * 0.5f;
        }
        sounds[SFX_COMMUTER_CHATTER] = CreateProceduralSound(buf, sr);
    }

    // 13. SFX_COFFEE_SIP: Station kiosk coffee sip (0.25s)
    {
        int n = (int)(sr * 0.25f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float noise = (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
            buf[i] = noise * expf(-9.0f * t) * 0.35f;
        }
        sounds[SFX_COFFEE_SIP] = CreateProceduralSound(buf, sr);
    }

    // 14. SFX_CASH_REGISTER: RCT style metallic ka-ching chime (0.4s)
    {
        int n = (int)(sr * 0.40f);
        std::vector<float> buf(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float bell1 = sinf(2.0f * PI * 1864.0f * t) * expf(-14.0f * t);
            float bell2 = (t > 0.08f) ? sinf(2.0f * PI * 2793.0f * (t - 0.08f)) * expf(-18.0f * (t - 0.08f)) : 0.0f;
            float coin = (t > 0.16f) ? sinf(2.0f * PI * 3520.0f * (t - 0.16f)) * expf(-25.0f * (t - 0.16f)) * 0.5f : 0.0f;
            buf[i] = (bell1 * 0.5f + bell2 * 0.6f + coin) * 0.7f;
        }
        sounds[SFX_CASH_REGISTER] = CreateProceduralSound(buf, sr);
    }

    initialized = true;
}

void Cleanup() {
    if (!initialized) return;
    for (auto& pair : sounds) {
        UnloadSound(pair.second);
    }
    sounds.clear();
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


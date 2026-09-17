#pragma once

#include "common.hpp"

enum SfxType {
    SFX_DOOR_CHIME = 0,     // Melodic Japanese/Parisian metro departure door chime
    SFX_VVVF_MOTOR,         // Variable-frequency traction inverter electric motor whine
    SFX_AIR_BRAKE,          // Pneumatic brake pressure release hiss
    SFX_SMARTCARD_BEEP,     // Contactless IC fare gate turnstile tap ("Pip-pip!")
    SFX_STATION_BELL,       // Station arrival announcement chime
    SFX_DELIVERY_CHIME,     // Commuter reached destination reward chime
    SFX_QUEUE_ALARM,        // Overcrowding triage alert beep
    SFX_UPGRADE_FANFARE,    // Weekly transit authority grant unlocked fanfare
    SFX_BUTTON_CLICK,       // OCC console control click
    SFX_CONSTRUCTION,       // Steel rail and concrete sleeper placement
    SFX_BULLDOZE,           // Infrastructure demolition
    SFX_COMMUTER_CHATTER,   // Commuters boarding / cheering
    SFX_COFFEE_SIP,         // Station newsstand coffee sip
    SFX_CASH_REGISTER,      // RCT style cash register ka-ching chime
    SFX_TRAIN_HORN,         // Two-tone pneumatic electric train departure horn
    SFX_OCEAN_AMBIENT,      // Gentle rolling ocean surf swell
    SFX_NIGHT_CRICKET,      // Nighttime crickets chirping
    SFX_MORNING_BIRD,       // Cheerful morning park songbird chirp
    SFX_THUNDER_ROLL,       // Distant rolling atmospheric thunder rumble
    SFX_TUNNEL_REVERB,      // Hollow resonant subway tunnel rolling wheel echo
    SFX_EXPRESS_WHOOSH,     // High-speed express train slipstream wind roar & Doppler whoosh
    SFX_VENDING_DISPENSE    // Cold beverage can drop and coin clink
};

namespace AudioManager {
    void Init();
    void Cleanup();
    void Play(SfxType type, float volume = 1.0f);
    void SetMute(bool muted);
    bool IsMuted();
}


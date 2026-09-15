# Metro Grid

A 2.5D isometric urban transit simulator for **Cognition GameJam '26**.
Design a metro network, run the dispatch desk, and keep the city moving.

Built in **C++17** with **Raylib 6.0** — 100% procedural audio, zero external assets.

---

## What Is It?

Metro Grid is a RollerCoaster Tycoon-style metro network builder. You lay tracks,
stations and scenery across a 2.5D isometric city grid. Commuters arrive at the
entrance, swipe through turnstiles, queue at platforms, and board your train.
They get off at the station matching the shape badge on their head — you never
control them directly. You just build the network and keep it running.

---

## Core Mechanics

- **3-Aspect Signalling** — Green/Yellow/Red wayside signals enforce safe train
  separation automatically.
- **Shape-Coded Commuters** — Each rider carries a Square, Circle, Triangle or
  Cross badge and alights at the first matching station (Mini Metro style).
- **Overcrowding Triage** — Platforms that fill up show a red clock. Clear the
  backlog with timely dispatches before riders walk out.
- **Line Operating Modes** — Toggle between OPEN (revenue), TEST RUN (empty
  loops) and CLOSED (suspended).
- **Weekly Upgrades** — Choose from randomized transit authority grants each week
  (fleet expansions, subsidies, express lines, etc.).
- **Transit Crew** — Hire custodians and signal engineers to maintain cleanliness
  and infrastructure.
- **Economy** — Earn fares per delivery, spend on track, stations, extra carriages,
  land reclamation, and additional trains.

---

## Controls

| Key | Action |
|---|---|
| WASD / Arrow Keys | Pan camera |
| Right Mouse Drag | Pan camera |
| Mouse Wheel | Zoom in / out |
| Left Click | Place piece / Inspect commuter / Click UI |
| 1–8 | Select tool in active category |
| TAB | Switch category (Track / Concourse / Scenery) |
| X | Bulldozer (demolish, refunds cost) |
| R | Rotate build heading |
| E / Q | Raise / Lower elevation |
| Space | Pause / Resume |
| ESC | Hard pause (Resume / Restart / Quit menu) |
| F | Toggle Cab Camera (ride along the train) |
| T | Toggle Line Operations Window (mode, fleet, fare, livery) |
| P | Toggle Transit Crew Window (cleanliness, staff hiring) |
| H or ? | Toggle Operations Manual & Tutorial |
| C | Recalibrate train to Central Hub |
| G | Show / Hide objectives checklist |
| M | Toggle audio mute |

---

## Features

- 2.5D isometric engine — 64x32 dimetric tiles, 48x48 grid, multi-level elevation
- Track pieces: straights, curves, elevated viaducts, tunnel portals, stations
- 3-aspect automatic signalling
- Commuter archetypes: workers, students, tourists, executives
- Per-station overcrowd timers with dispatch pressure
- EMU rolling stock with configurable car count (2–5 cars, 12–30 capacity)
- Ticket fare control ($0.50–$10.00)
- Floating financial feedback (+$2.50 / -$40) on the isometric map
- Commuter inspector card (profile, target shape, balance, satisfaction)
- Line Operating Modes: Open, Test Run, Closed
- Staff management: custodians and signal engineers
- Weekly upgrade choices with randomized grants
- Shop: extra carriages, land reclamation, additional EMU trains (up to 6 extras)
- Rush hour windows with fare multiplier
- Day / sunset / night cycle with lamppost and headlamp lighting
- 13-lesson guided onboarding with spotlight highlights
- Arcade pause overlay, victory/game-over screens, session high-score
- Endless Metropolis mode after reaching the 1500-rider goal
- 100% procedural audio — VVVF motor whine, door chimes, cash registers, alarms
- WebAssembly build for browser play

---

## Build & Run

### Linux / macOS

```bash
cd raylib
make clean && make
./metro_grid
```

### Web (Emscripten)

```bash
cd raylib
make web
# produces metro_grid_web.zip ready for itch.io upload
```

### Pre-built

The `metro_grid_web.zip` in the project root is the itch.io-ready HTML build
(1280x720, play in browser).

---

## Tech Stack

- **Language:** C++17 (`-Wall -Wextra`)
- **Graphics/Audio:** Raylib 6.0 (statically linked)
- **Fonts:** Liberation Sans (Regular + Bold)
- **Audio:** 100% procedural synthesis — no audio files shipped
- **Build:** Make (desktop), Emscripten (web)
- **Platforms:** Linux x86_64 (primary), WebAssembly, Windows/macOS via Raylib

---

## Repository

```
raylib/          C++17 + Raylib source, Makefile, screenshots
raylib/src/      Game source (game, track, train, peep, audio, ui, particles)
raylib/assets/   Font files (Liberation Sans)
game/            Early Godot prototype (unused)
build/           Submission package (README.pdf)
metro_grid_web.zip   itch.io-ready web build
*.pdf            Design concept documents
```

---

## Credits

Built for **Cognition GameJam '26** — Navi Mumbai, India.
Inspired by RollerCoaster Tycoon and Mini Metro.

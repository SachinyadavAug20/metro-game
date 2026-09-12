# Metro Grid

A 2.5D isometric urban transit simulator for **Cognition GameJam '26**.
Design a metro network, run the dispatch desk, and keep the city moving.
Built in **C++17** with **Raylib 6.0** — 100% procedural audio, zero external assets.

---

## How to Run

```bash
cd raylib
make          # builds the game
./metro_grid
```

Windows/macOS and details are in [raylib/README.md](raylib/README.md).

## Controls

| Key | Action |
|---|---|
| WASD / Arrows | Pan camera |
| Mouse Wheel | Zoom |
| Left Click | Place / inspect |
| 1–8 | Select tool |
| TAB | Change category |
| X | Bulldozer |
| R | Rotate piece |
| E / Q | Raise / lower |
| Space | Pause |
| H | Help |

## Features

- Lay tracks, stations, viaducts and tunnels on a 2.5D isometric map
- 3-aspect signalling stops trains from colliding
- Commuters (workers, students, tourists, executives) flood the network
- Overcrowded stations trigger dispatch timers — miss them and riders walk out
- Hire staff, manage the OCC dashboard, balance the treasury
- Day / night cycle with lampposts and headlamp lighting

## Repository

```
game/    # early Godot experiment (unused)
raylib/  # the actual game (C++17 + Raylib)
build/   # game-jam submission package (zip + README.pdf)
```
# Lava Rising Mod

Rising/falling lava with a heat zone system for Hamsterball Physicus.

## What It Does

Replaces the Up Race lifter state machine with a lava-themed system:

- **Lava rises and falls** on timing cycles (rise → pause → fall → pause → repeat)
- **Heat zones** detect when the ball is within the lava's X/Z footprint:
  - **Touching lava** (gap ≤ 26 units) → ball shatters (death)
  - **Very close** (gap < 30 units) → max heat: ball turns red, speed +50%
  - **Moderate distance** (30-200 units) → partial heat: gradual red tint and speed boost
  - **Far away** (> 200 units) → no effect, ball returns to normal
- **Ball tint** shifts via color multiplier (ball+0x2AC/0x2B0/0x2B4)

## How Heat Detection Works

Uses **Option 2**: X/Z bounds check + vertical Y gap.

1. Check if ball X/Z is within the lifter's footprint (±75 units, same as carry logic)
2. If yes, compute vertical gap between ball Y and lava Y
3. Map gap to heat level (0.0 to 1.0)
4. Apply effects: color tint, speed scaling, death check

This avoids expensive raycasting — just simple float comparisons per frame.

## Installation

1. Rename your original `bass.dll` to `bass_real.dll`
2. Put this `bass.dll` in the game folder
3. Play Up Race (the lifters are on the Up Race level)

## Configuration

Edit the constants in `mod.c`:

| Constant | Default | Description |
|---|---|---|
| `HEAT_CUTOFF` | 200.0 | Beyond this distance, no heat effect |
| `HEAT_DEATH_DIST` | 26.0 | Ball radius — touching lava = death |
| `HEAT_FULL_DIST` | 30.0 | Distance at which heat is maximum |
| `NORMAL_MAX_SPEED` | 1000.0 | Normal ball max speed |
| `T_RISE` / `T_FALL` | 1000 | Frames for rise/fall duration |
| `T_BOTTOM_PAUSE_1` | 300 | Bottom pause duration |
| `T_TOP_PAUSE` | 300 | Top pause duration |

## Future Plans

- Blue tint for cold/ice zones
- Configurable per-lifter timing for different lava sections
- Custom MESHWORLD level designed around lava mechanics

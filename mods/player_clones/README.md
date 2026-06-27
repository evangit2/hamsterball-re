# Player Clones Mod (v13)

## Overview
Spawns AI-controlled clone balls for any player slot (1-4) in both races and arenas.
Clones chase and attack all entities everywhere on the track, regardless of Y level or position.

## Hotkeys
- **1** — Spawn Player 1 clone
- **2** — Spawn Player 2 clone
- **3** — Spawn Player 3 clone
- **4** — Spawn Player 4 clone

## Features
- Works in **both races and arenas**
- Clones chase all entities everywhere (no "going home" behavior)
- Can spawn P2-P4 clones even when original players don't exist (uses P1 as appearance source)
- Per-player color via copied appearance from source ball
- Up to 8 simultaneous clones
- Automatic cleanup of dead/stale clones

## What v13 Fixes (vs V8/V11)
1. **"Going home" bug** — Clear `0x2E8` (respawn needed flag) every frame. This prevents `Ball_FindClosestRespawnPoint` from teleporting clones back to spawn.
2. **"Not chasing" bug** — Clear ALL force guards every frame: `0x2F9` (falling), `0x2CC` (block), `0x808` (state), `0x2F0` (impact count). V8 only cleared `0x2F8` and `0x808`.
3. **"All black" in arenas** — Copy render context index (`0x154`), render mode flags (`0x748`, `0x182`), and render state flags (`0x6FC`, `0x700`, `0x708`, `0x70D`, `0x734`) from source ball. Without these, the clone uses an uninitialized render context.
4. **P2-4 spawning** — P1 fallback already worked in V8. Added `0x310=1` and `0x29C=1.0` at spawn to prevent respawn behavior.
5. **"Nothing spawned" in races** — Clear all guard flags at spawn time, not just in the AI loop.

## How It Works

### Hook Point
`0x0041B540` — `Scene_UpdateBallsAndState`. Called every frame for both races and arenas.

### AI Function
Calls `0x004222D0` (Computer AI) with ball swap trick:
1. Temporarily replaces player table entry with clone pointer
2. Sets control mode to `0x63` (AI)
3. Calls AI function which finds nearest target in `scene+0x29D4` list
4. Computes direction force toward target
5. Restores original player table entry
6. Applies force via `vtable[5]` (`Ball_ApplyForceWithMultipliers`)

### Force Guard Clearing
Every frame, the AI loop clears these flags on each clone:
| Offset | Name | Why Clear |
|--------|------|-----------|
| `0x2E8` | respawn_needed | Prevents teleport to respawn point ("going home") |
| `0x2E9` | sticky_limit | Prevents stuck-on-limit behavior |
| `0x2F8` | show_stars | Prevents respawn sequence |
| `0x2F9` | falling | Allows force application while "falling" |
| `0x2CC` | block | Allows force application while "blocked" |
| `0x808` | state | Must be 0 for AI to process ball |
| `0x2F0` | impact_count | Must be < 0x51 (81) for force to apply |

### CHASE/HOME Values
Set to `99999999.0` (`0x4CBEBC20`) at spawn and every frame. These are used by the 8ball AI path if activated.

## File
- Script: `player_clones_CE_script.CEA`

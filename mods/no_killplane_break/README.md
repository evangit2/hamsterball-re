# No Killplane Break for Entity Balls

## What It Does

Entity balls (badballs) no longer break when hitting killplanes or falling from height. They still respawn when falling into the void.

## How It Works

`Ball_FallDeath` (vtable[8], 0x409480) is called when a ball hits a killplane or falls. It creates Ball_Split fragments (ball breaks visually) and ArenaScoreParticle ring effects.

`death_pending` (ball+0x2E9) is set by the **caller** before vtable[8] is called. So even if we skip Ball_FallDeath entirely, `death_pending` is already 1. `Scene_UpdateBallsAndState` will see it and call `Ball_Respawn` — the ball respawns without breaking.

## Patch

| Address | Original | Patched | Effect |
|---------|----------|---------|--------|
| 0x409480 | `6A FF 64 A1 00 00 00 00` (SEH setup) | `JMP cave + NOP×3` | Entity balls: RET immediately (no breaking). Player balls: normal. |

## Code Cave

At Ball_FallDeath entry:
1. Check `ball+0x18 == -1` (entity ball)
2. If entity: `RET` — skip all breaking/fragment logic
3. If player: execute original SEH setup, continue normally

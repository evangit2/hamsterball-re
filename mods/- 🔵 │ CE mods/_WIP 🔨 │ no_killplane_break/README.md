# No Killplane Break — All Balls

## What It Does

ALL balls (Players 1-4 + badballs) no longer break when hitting killplanes or falling from height. They still respawn when falling into the void.

## How It Works

`Ball_FallDeath` (vtable[8], 0x409480) creates Ball_Split fragments (ball breaks visually) and ArenaScoreParticle ring effects.

`death_pending` (ball+0x2E9) is set by the **caller** before vtable[8] is called. So even if Ball_FallDeath does nothing (just RETs), `death_pending` is already 1. `Scene_UpdateBallsAndState` sees it and calls `Ball_Respawn` — balls respawn without breaking.

## Patch

| Address | Original | Patched | Effect |
|---------|----------|---------|--------|
| 0x409480 | `6A FF 64 A1 00 00 00 00` (SEH setup) | `C3 90 90 90 90 90 90 90` (RET + NOP×7) | All balls: skip breaking entirely |

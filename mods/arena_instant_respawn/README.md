# Arena Instant Respawn v8

## What It Does

Entity balls in arenas respawn directly on the arena ground — no respawn platform comes from above.

## How It Works

Single patch: skip `Scene_StartCountdown` for entity balls.

When an entity ball touches a SinkPlatform, `SinkPlatformArenaCollisionEvents` calls `Scene_StartCountdown(scene, ball)`. Our code cave checks if the ball is an entity ball (`ball+0x18 == -1`). If so, it returns immediately — the platform never activates.

The ball continues falling. `ArenaBoard_Update` (0x420DA0) detects when the ball falls below the Y-threshold and calls `Ball_Respawn` directly using the original game code. No patches to Ball_Respawn needed.

## Patch

| Address | Original | Patched | Effect |
|---------|----------|---------|--------|
| 0x437130 | `8A 81 F1 10 00 00` (MOV AL,[ECX+0x10F1]) | `JMP cave + NOP` | Skip platform for entity balls |

## Version History

| Version | Approach | Result |
|---------|----------|--------|
| v1-v4 | Various Ball_Respawn patches | Failed |
| v5-v7 | Code caves + multiple patches | Failed |
| v8 | Single patch: skip Scene_StartCountdown for entity balls | Simplest possible |

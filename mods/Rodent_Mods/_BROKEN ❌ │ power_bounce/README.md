# Power Bounce

Automatically launches the player ball straight up when landing on top of a shrunken 8-ball.

## How It Works

Two hooks work together:

1. **Collision Detection Hook** (0x406FD1 in Ball_Update): When any ball-ball collision occurs, checks if a player ball hit an 8-ball (player_index == -1). If so, verifies:
   - 8-ball radius < 70% of player radius (shrunken)
   - Player center Y > 8-ball center Y (player is above)
   - (playerX - ball8X)² + (playerZ - ball8Z)² < (ball8 radius)² (player center is within 8-ball's radius in XZ)

2. **Impulse Hook** (0x407BB4 in Ball_Update Phase 15): If the collision check passed, adds an upward impulse of **40.0f** (2× the jump mod's 20.0f) to the player ball's Y velocity accumulator (ball+0x174).

## Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| Jump impulse | 40.0f | 2× the jump mod's impulse (20.0f) |
| Size ratio threshold | 0.7 | 8-ball must be < 70% of player radius |
| Player index check | -1 | 8-balls have player_index = -1 |

## Ball Offsets Used

| Offset | Type | Description |
|--------|------|-------------|
| +0x018 | int | player_index (0-3 = player, -1 = NPC 8-ball) |
| +0x164 | float | position X |
| +0x168 | float | position Y |
| +0x16C | float | position Z |
| +0x174 | float | Y velocity accumulator (impulse target) |
| +0x284 | float | ball radius |

## Hook Points

| Address | Original Bytes | Description |
|---------|---------------|-------------|
| 0x406FD1 | D9 87 84 02 00 00 | fld [edi+0x284] — ball-ball collision scoring |
| 0x407BB4 | 8B 4C 24 1C 8B 11 | MOV ECX,[ESP+1C]; MOV EDX,[ECX] — Phase 15 impulse point |

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll power_bounce.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Install

1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball directory
2. Copy the compiled `bass.dll` to the same directory
3. Launch Hamsterball.exe

## Crash Test

Passed via hbtestd: 38.62s runtime, no crash.

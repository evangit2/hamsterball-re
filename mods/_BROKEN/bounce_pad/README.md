# Bounce Pad Mod (N:BOUNCE)

Custom collision event that makes the ball bounce proportionally to its fall speed.

## How It Works

- **Hook target**: `DispatchCollisionEvents` (0x0040C5D0) — 8-byte inline detour
- **Event name**: `N:BOUNCE` — tag objects with this name in MESHWORLD levels
- **Bounce formula**: `new_vy = -vy * 0.8` (80% energy retention, 20% loss per bounce)
- **Velocity field**: `phys+0xCA8` (PhysicsObject Y velocity) — same field E:TRAJECTORY uses
- **Cooldown**: 10 frames (ball+0x7DC, same field E:JUMP uses)
- **X/Z velocities**: Never read or modified

## Bounce Factor

`0.8` — the ball loses 20% of its vertical energy on each bounce. Fall at -30 → bounce at +24. Fall at -10 → bounce at +8.

## MESHWORLD Usage

Name an object `N:BOUNCE` in a custom level. When the ball falls onto it, it bounces back up proportional to fall speed.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll bounce_pad.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Crash Test

hbtestd: 19.03s runtime, no crash. ✅

## Files

- `bounce_pad.c` — source
- `bass.dll` — compiled DLL
- `bounce_pad.zip` — distributable

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` to the game folder
3. Set Wine DLL override to `native` for `bass.dll`

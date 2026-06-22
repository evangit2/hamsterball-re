# Wall Bumpers Mod

**ALL walls are bumpers.** Every wall collision in the game launches the ball away like a pinball bumper.

## What It Does

Hooks `Ball_FallUpdate` (0x408830) — the player's per-frame physics/collision function. After the original physics runs, the mod scans the collision entry list for type==2 (wall) hits. For each wall hit, it reads the surface normal and pushes the ball away along that normal, simulating a bumper effect on every wall in the game.

Also sets `ball+0x808` (speed_boost counter) to 3, briefly disabling player input — just like real Hamsterball bumpers do during a launch.

## Controls

| Key | Action |
|-----|--------|
| **F8** | Toggle mod on/off (default: ON) |
| **F9** | Cycle bumper force: 40 → 60 → 80 → 120 → 200 |

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll wall_bumper_mod.c \
    -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
    -Wl,--add-stdcall-alias
```

## Install

1. Rename original `bass.dll` → `bass_real.dll` in your Hamsterball folder
2. Copy this `bass.dll` into the game folder
3. Launch the game

## Technical Details

- **Hook target:** `Ball_FallUpdate` at 0x00408830 (`__thiscall`, ECX = ball)
- **Detour:** 7-byte (5-byte JMP + 2 NOPs) — first two instructions span 7 bytes
- **Trampoline:** Original 7 bytes + JMP rel32 back to 0x408837
- **Collision entries:** Read from PhysicsObject (ball+0x1A4) → count at +0x1C, array at +0x424
- **Wall detection:** Entry type == 2, normal at +0x20/+0x24/+0x28
- **Bumper effect:** Ball position += normal × force, speed_boost = 3

## Files

| File | Description |
|------|-------------|
| `wall_bumper_mod.c` | Source code |
| `bass.dll` | Compiled mod |
| `wall_bumpers.zip` | Distributable package |

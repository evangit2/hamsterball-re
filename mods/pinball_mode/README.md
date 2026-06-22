# Pinball Mode

Turns every wall in every level into a pinball bumper. Wall collisions amplify the ball's bounce velocity by a configurable multiplier.

## Controls

| Key | Action |
|-----|--------|
| **F8** | Toggle pinball mode on/off (silent — no popup) |
| **F9** | Cycle bounce multiplier (2x → 3x → 5x → 10x → 2x) |

Pinball mode is **OFF by default**. Press F8 in-game to enable it.

## How It Works

Two code cave hooks inside `Ball_Update` (0x405E00):

1. **Collision type check hook** (0x407300): When the game processes a collision entry with `type == 2` (wall), sets a `g_wall_hit` flag before the game's own bounce calculation runs.

2. **Phase 15 convergence hook** (0x407BB4): After the game computes the post-collision velocity vector, if `g_wall_hit` was set, multiplies the velocity vector by the bounce multiplier. This amplifies the bounce without changing the direction.

The physics object (at ball+0x1A4) fields:
- `+0xC64` = speed magnitude (float) — **multiplied** by bounce mult
- `+0xC8C/C90/C94` = collision normal XYZ (float × 3) — **NOT modified** (v3 bug: was corrupted)
- `+0xC98/C9C/CA0` = velocity vector = speed × normal (float × 3) — **multiplied** by bounce mult

### v4 Fix (v3 → v4)

v3 multiplied the collision normals (+0xC8C/+0xC90/+0xC94) by speed×mult, which:
1. Had no effect on bounce (velocity vector was already computed before the hook point)
2. Corrupted normals for subsequent collisions in the same frame
3. Turned unit normals into velocity components, breaking collision math

v4 directly multiplies the velocity vector at +0xC98/+0xC9C/+0xCA0 (and speed at +0xC64). This is the correct field — the collision callback at 0x407BBC reads these values to determine bounce response.

## Installation

1. Rename original `bass.dll` → `bass_real.dll` (for audio)
2. Copy this `bass.dll` to the Hamsterball directory
3. Launch Hamsterball.exe
4. If `bass_real.dll` is missing, game runs without audio (no crash)

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll bass_pinball_v4.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

- **BASS proxy**: Lazy-load v3 loader — forwards 10 BASS audio functions to `bass_real.dll`. If `bass_real.dll` is missing, stubs return success (no audio, no crash).
- **Code caves**: Uses `VirtualAlloc` + hand-assembled x86 machine code (no C function calls from inside caves — follows the code cave safety rules from the DLL modding skill).
- **Keyboard polling**: Background thread reads DirectInput8 keyboard buffer via the game's input chain (App → InputHandler → KeyboardDevice → key buffer).
- **Silent toggle**: v4 removes MessageBoxA popups on F8/F9 — toggles are silent.
- **Works on all 15 levels**: No per-level configuration. The hooks are inside Ball_Update which processes all balls in all game modes.

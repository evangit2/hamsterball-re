# Pinball Mode

Turns every wall in every level into a pinball bumper. Wall collisions amplify the ball's bounce velocity by a configurable multiplier.

## Controls

| Key | Action |
|-----|--------|
| **F8** | Toggle pinball mode on/off |
| **F9** | Cycle bounce multiplier (2x → 3x → 5x → 10x → 2x) |

Pinball mode is **OFF by default**. Press F8 in-game to enable it.

## How It Works

Two code cave hooks inside `Ball_Update` (0x405E00):

1. **Collision type check hook** (0x407300): When the game processes a collision entry with `type == 2` (wall), sets a `g_wall_hit` flag before the game's own bounce calculation runs.

2. **Phase 15 convergence hook** (0x407BB4): After the game computes the post-collision speed and velocity vector, if `g_wall_hit` was set, multiplies the collision node's speed (`+0xC64`) by the bounce multiplier and recomputes the velocity vector (`+0xC98/C9C/CA0 = speed × direction`). This amplifies the bounce without changing the direction — the ball still bounces off the wall correctly, just much harder.

The collision node's persistent velocity fields:
- `+0xC64` = speed magnitude (float)
- `+0xC8C/C90/C94` = normalized direction vector (float × 3)
- `+0xC98/C9C/CA0` = velocity vector = speed × direction (float × 3)

## Installation

1. Rename original `bass.dll` → `bass_real.dll`
2. Copy this `bass.dll` to the Hamsterball directory
3. Launch Hamsterball.exe

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll bass_pinball.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

- **BASS proxy**: Forwards 10 BASS audio functions to `bass_real.dll` (the original BASS library). All other BASS exports are empty stubs since the game never calls them.
- **Code caves**: Uses `VirtualAlloc` + hand-assembled x86 machine code (no C function calls from inside caves — follows the code cave safety rules from the DLL modding skill).
- **Keyboard polling**: Background thread reads the DirectInput8 keyboard buffer via the game's input chain (App → InputHandler → KeyboardDevice → key buffer).
- **Works on all 15 levels**: No per-level configuration. The hooks are inside Ball_Update which processes all balls in all game modes.

# Pinball Mode

Turns every wall in every level into a pinball bumper. Wall collisions amplify the ball's bounce velocity by a configurable multiplier.

## Controls

| Key | Action |
|-----|--------|
| **F8** | Toggle pinball mode on/off (silent) |
| **F9** | Cycle bounce multiplier (2x → 3x → 5x → 10x → 2x) |

Pinball mode is **OFF by default**. Press F8 in-game to enable it.

## How It Works

Two code cave hooks inside `Ball_Update` (0x405E00):

1. **Collision type check hook** (0x407300): When the game processes a collision entry with `type == 2` (wall), sets a `g_wall_hit` flag.

2. **Bounce offset amplification hook** (0x407CE0): After the collision callback has computed and stored the bounce displacement at `ball+0x2C0/+0x2C4/+0x2C8`, but BEFORE it gets added to the ball's position, multiplies the offset by the bounce multiplier. This is the actual bounce displacement — amplifying it makes the ball bounce harder without touching any collision math.

### Ball struct fields used:
- `+0x164/+0x168/+0x16C` = position X/Y/Z (float)
- `+0x2C0/+0x2C4/+0x2C8` = bounce offset X/Y/Z (float, set by collision callback, added to position, then cleared)

### v5 fix history (v3→v4→v5):

| Version | Hook target | Bug |
|---------|------------|-----|
| v3 | physics_obj +0xC8C/C90/C94 (normals) | Corrupted collision normals for subsequent collisions |
| v4 | physics_obj +0xC64/C98/C9C/CA0 (speed/velocity) | Corrupted collision response math → ball stuck to ground |
| **v5** | **ball +0x2C0/2C4/2C8 (bounce offset)** | **No bugs — correct field, zero collision interference** |

v3/v4 modified physics_obj internal fields that the collision callback reads, corrupting collision math. v5 hooks at the correct point: AFTER the callback sets the bounce offset, BEFORE it's applied to position. Zero interference with collision internals.

## Installation

1. Rename original `bass.dll` → `bass_real.dll` (for audio)
2. Copy this `bass.dll` to the Hamsterball directory
3. Launch Hamsterball.exe
4. If `bass_real.dll` is missing, game runs without audio (no crash)

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll bass_pinball_v5.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

- **BASS proxy**: Lazy-load v3 loader — forwards 10 BASS audio functions to `bass_real.dll`. If missing, stubs return success (no audio, no crash).
- **Code caves**: Uses `VirtualAlloc` + hand-assembled x86 machine code (no C function calls from inside caves).
- **Keyboard polling**: Background thread reads DirectInput8 keyboard buffer via the game's input chain (App → InputHandler → KeyboardDevice → key buffer).
- **Silent toggle**: F8/F9 are silent — no popups.
- **Works on all 15 levels**: No per-level configuration. The hooks are inside Ball_Update which processes all balls in all game modes.

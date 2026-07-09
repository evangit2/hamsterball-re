# WARP (Level Warp) Mod v7

When the player ball approaches a MESHWORLD Section 1 node named `WARP(levelname)`, a multi-phase special effect sequence plays before loading the target level. This replaces the old `E:WARP()` collision-event approach — no collision planes needed, just a node point.

## How It Works

The mod scans MeshWorld Section 1 nodes every frame (same access pattern as the Up Race vacuum system's `VAC-IN`/`VAC-OUT`/`VAC-VEC` markers). When the player ball gets within **30 units** (XZ plane) and **50 units** (Y axis) of a `WARP(...)` node, the warp sequence triggers.

### Node Format

Place a node in your MESHWORLD Section 1 with the name:

```
WARP(3)       → loads Intermediate
WARP(neon)    → loads Neon Race
WARP(15)      → loads Impossible
WARP(warmup)  → loads Warm-Up
```

### Proximity Detection

Matches the vacuum system's thresholds (from decompiled `CollisionFace_Update` @ 0x43D160):
- **XZ distance** < 30.0 units (`Math_FastDistance2D < 0x1E`)
- **|Y difference|** < 50.0 units (`_DAT_004d5d10 = 50.0`)

A 2-second cooldown after each warp prevents immediate re-triggering.

## Effect Sequence

| Phase | Duration | Description |
|-------|----------|-------------|
| **1. RUMBLE** | 2.0s | Ball frozen + vibrating (native render jitter). Music starts fading out. |
| **2. FLASH** | 0.25s | Ball invisible. Screen flashes white (quick ramp up/down). |
| **3. HOLD** | 1.0s | Pause — screen is clear, ball stays invisible. |
| **4. FADE** | 2.0s | Screen fades from transparent to solid white. |
| **5. LOAD** | instant | Target level loads via `App_StartPracticeRace(app, levelIndex)`. |
| **6. REVEAL** | 1.0s | Screen fades from white to reveal the new level. |

**Total sequence: ~6.25 seconds** (real-time, framerate-independent)

## Hooks (1 total)

1. **App_FrameUpdate epilogue** (0x46C1F1) — 5-byte detour. Runs the node scanner + warp state machine every frame.

No collision hook needed — the mod reads MeshWorld node positions directly and checks ball proximity.

## Game Systems Used

- **Node access**: `board+0x8AC` → MeshWorld → `+0x480` → Section 3 data → `+0x898` count, `+0xCA0` data array. Each node: `[char* name, float x, float y, float z]`.
- **Ball list**: `board+0x2DE0` → data array → first entry = player ball.
- **Ball freeze**: `ball+0x808=1000` + `ball+0x2CC=1` + `ball+0x2D4=1` (render jitter).
- **Ball invisible**: `ball+0x2FC=0.0` (alpha, forced every frame).
- **Music fade**: Manual per-channel volume ramp over 3.0s.
- **White screen**: Game's native fade at `board+0x3624`.
- **Level load**: `App_StartPracticeRace(app, levelIndex)` with tournament mode preservation.

## Usage

1. Rename original `bass.dll` to `bass_real.dll`
2. Place this `bass.dll` next to `Hamsterball.exe`
3. In your custom MESHWORLD level, add a Section 1 node named `WARP(X)` where X is a number (1-15) or level name

## Tag Format

| Tag | Level Loaded |
|-----|-------------|
| `WARP(1)` | Warm-Up |
| `WARP(2)` | Beginner |
| `WARP(3)` | Intermediate |
| ... | ... |
| `WARP(15)` | Impossible |

Also accepts names: `WARP(neon)`, `WARP(dizzy)`, `WARP(warmup)`, etc.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v7.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Version History

- **v7**: Replaced collision-event trigger with MeshWorld node-point proximity scanner. Level designers place `WARP(Name)` nodes in MESHWORLD Section 1 (same pattern as Up Race `VAC-IN`/`VAC-OUT`/`VAC-VEC` markers). Removed DispatchCollisionEvents hook entirely. Added 2-second cooldown after warp. Removed all collision-related code.
- **v6f**: Tournament mode preservation, per-channel music fade, ball vtable validation.
- **v6e**: Native render jitter (ball+0x2D4), HOLD phase, board+0x3624 fade.
- **v6d**: Code review cleanup — removed 155 lines of dead code.
- **v6**: Race index off-by-one fix, FVF fix, timestamp init fix.
- **v5**: D3D texture stage state restoration, GetTickCount-based timing, vsnprintf, PHASE_REVEAL.
- **v4**: Initial special effects edition.

# E:WARP (Level Warp) Mod v4 — Special Effects Edition

When the ball touches a floor panel tagged `E:WARP(X)`, a multi-phase special effect sequence plays before loading the target level.

## Effect Sequence

| Phase | Duration | Description |
|-------|----------|-------------|
| **1. JIGGLE** | 2.0s (50 frames) | Ball is frozen (unmovable), jiggles upward slightly each frame (like Up Race vacuum suck). Music starts fading out. Sound effect `warp_enter.wav` plays (placeholder — not yet wired). |
| **2. FLASH** | 0.5s (12 frames) | Ball jiggling stops. Ball becomes invisible. Screen flashes white (quick fade in to peak, then fade out). Second sound effect `warp_exit.wav` plays (placeholder). |
| **3. FADE** | 1.0s (25 frames) | Screen fades from transparent to solid white. Music should be fully faded out by end of this phase. |
| **4. LOAD** | instant | Target level loads via `App_StartPracticeRace(app, levelIndex)`. All ball state restored. |

**Total sequence: ~3.5 seconds** (at 25fps)

## How It Works

### Hooks (3 total)
1. **DispatchCollisionEvents** (0x40C5D0) — 8-byte trampoline. Detects `E:WARP(...)` collisions and starts the effect sequence.
2. **App_FrameUpdate epilogue** (0x46C1F1) — 5-byte trampoline. Runs the warp state machine every frame (jiggle, flash, fade, load).
3. **Graphics_PresentOrEnd** (0x455A90) — 7-byte trampoline. Draws the white screen overlay via D3D8 `DrawPrimitiveUP` before Present/Swap.

### Game Systems Used
- **Ball freeze**: `ball+0x808=1000` (impact/freeze counter) + `ball+0x2CC=1` (in-tar flag) — blocks `Ball_ApplyForce`
- **Ball jiggle**: `ball+0x168 += 0.25` per frame (same rate as Up Race vacuum suck phase)
- **Ball invisible**: `ball+0x2FC = 0.0` (alpha)
- **Music fade**: Manipulates MusicChannel struct (`+0x528`=volume, `+0x530`=fade-out flag, `+0x52C`=fade rate) via MusicDevice at App+0x17C. Also calls `BASS_ChannelSetAttributes` directly as fallback.
- **White screen**: D3D8 alpha-blended fullscreen quad via `DrawPrimitiveUP` (vtable[72]), `SetRenderState` (vtable[50]), `SetTextureStageState` (vtable[63]). Uses `D3DFVF_XYZRHW | D3DFVF_DIFFUSE` vertex format.
- **Level load**: `App_StartPracticeRace(app, levelIndex)` — same as practice race menu selection

## Usage

1. Rename original `bass.dll` to `bass_real.dll`
2. Place this `bass.dll` next to `Hamsterball.exe`
3. In your custom MESHWORLD level, add a floor plane with event name `E:WARP(X)` where X is a number (1-15) or level name

## Tag Format

| Tag | Level Loaded |
|-----|-------------|
| `E:WARP(1)` | Warm-Up |
| `E:WARP(2)` | Beginner |
| `E:WARP(3)` | Intermediate |
| ... | ... |
| `E:WARP(15)` | Impossible |

Also accepts names: `E:WARP(neon)`, `E:WARP(dizzy)`, `E:WARP(warmup)`, etc.

## Sound Effects (TODO)

Two placeholder sound slots are defined:
- `warp_enter.wav` — plays at start of jiggle phase
- `warp_exit.wav` — plays at start of flash phase

These are not yet implemented (DirectSound integration pending). Will be wired up once sound file names are confirmed.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v4.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Crash Test

Passed: 11.6s runtime, no crash (hbtestd Wine/Xvfb).

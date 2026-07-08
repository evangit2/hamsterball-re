# E:WARP (Level Warp) Mod v6f

When the ball touches a floor panel tagged `E:WARP(X)`, a multi-phase special effect sequence plays before loading the target level.

## Effect Sequence

| Phase | Duration | Description |
|-------|----------|-------------|
| **1. RUMBLE** | 2.0s | Ball is frozen (unmovable). Music starts fading out. |
| **2. FLASH** | 0.25s | Ball becomes invisible. Screen flashes white (quick ramp up to peak, then back down). |
| **3. HOLD** | 1.0s | Pause — screen is clear, ball stays invisible. |
| **4. FADE** | 2.0s | Screen fades from transparent to solid white. Music should be fully faded out by mid-fade. |
| **5. LOAD** | instant | Target level loads via `App_StartPracticeRace(app, levelIndex)`. All ball state restored. |
| **6. REVEAL** | 1.0s | Screen fades from white to reveal the new level. |

**Total sequence: ~6.25 seconds** (real-time, framerate-independent)

## How It Works

### Hooks (2 total)
1. **DispatchCollisionEvents** (0x40C5D0) — 8-byte trampoline. Detects `E:WARP(...)` collisions and starts the effect sequence.
2. **App_FrameUpdate epilogue** (0x46C1F1) — 5-byte trampoline. Runs the warp state machine every frame (rumble, flash, hold, fade, load, reveal).

### Game Systems Used
- **Ball freeze**: `ball+0x808=1000` (impact/freeze counter) + `ball+0x2CC=1` (in-tar flag) — blocks `Ball_ApplyForce`
- **Ball invisible**: `ball+0x2FC=0.0` (alpha) — forced every frame during FLASH, HOLD, and FADE (game overwrites it to 1.0 each frame, so continuous writes are required). Respects respawn flag (`ball+0x2F9`).
- **Music fade**: Manual volume ramp over 3.0 seconds. Writes `MusicChannel+0x528` (volume) and calls `BASS_ChannelSetAttributes` directly. Does NOT use game's fade-out flag (cuts too fast).
- **White screen**: Game's native fade at `board+0x3624` (float alpha). The board render function (`FUN_0041b710`) reads this every frame and draws a fullscreen rect through the game's own material pipeline — no D3D state corruption.
- **Level load**: `App_StartPracticeRace(app, levelIndex)` — same as practice race menu selection. Difficulty is saved and restored around the call.

### Phase Timing
All timing uses `GetTickCount()` for framerate independence. Timestamps are initialized when the warp triggers (in the collision handler), so elapsed=0 on the first frame.

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

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v6.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Version History

- **v6f**: Fixed FADE duration to 2.0s (was 3.0s in code, 6.0s in README — now consistent). Fixed version label mismatch (header said v6e, DllMain said v6f). Removed dead savedRaceIdx variable. Added ball vtable validation (is_valid_ball) to prevent heap corruption if ball is destroyed mid-warp. Fixed music fade to track per-channel original volumes instead of using a single global volume for all channels. Fixed tournament mode preservation: saves/copies score+time arrays from old profile, frees BestTimeTrackers created by App_StartPracticeRace, restores tournament flags on new profile.
- **v6e**: Renamed JIGGLE → RUMBLE phase. Added HOLD phase (1s pause between flash and fade). Tripled FADE duration to 6s. Halved FLASH to 0.25s. Fixed ball invisibility (per-frame alpha force). Fixed screen fade (write to board+0x3624 not App+0x3624). NULL ball/board guard.
- **v6d**: Code review cleanup — removed 155 lines of dead code (dead PresentHook, setWinState, g_trampoline, D3D8 constants). Fixed uninitialized board variable.
- **v6**: Race index off-by-one fix. FVF set before DrawPrimitiveUP. Jiggle timestamp initialization. Inline asm clobber fix.
- **v5**: D3D texture stage state restoration. GetTickCount-based timing. vsnprintf. PHASE_REVEAL.
- **v4**: Initial special effects edition.

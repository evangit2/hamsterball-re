# E:WARP (Level Warp) Mod v7d

When the ball touches a `WARP(Name)` node point in a custom MESHWORLD level, a multi-phase special effect sequence plays before loading the target level.

## Effect Sequence

| Phase | Duration | Description |
|-------|----------|-------------|
| **1. RUMBLE** | 2.0s | Ball is frozen (steering disabled). Ball vibrates (native render jitter). Ball color fades from original to **yellow**. Ball alpha fades from 1.0 to **0.5** (becomes ghostly). Music starts fading out. |
| **2. FLASH** | 0.25s | Screen flashes white (quick ramp up to peak, then back down). Ball becomes **invisible** exactly when the screen reaches full white. **Timer freezes** at this moment. |
| **3. HOLD** | 1.0s | Pause — screen is clear, ball stays invisible, timer frozen. |
| **4. FADE** | 2.0s | Screen fades from transparent to solid white. Music should be fully faded out by mid-fade. Timer stays frozen. |
| **5. LOAD** | instant | Target level loads via `App_StartPracticeRace(app, levelIndex)`. All ball state restored. Tournament mode preserved. |
| **6. REVEAL** | 1.0s | Screen fades from white to reveal the new level. |

**Total sequence: ~6.25 seconds** (real-time, framerate-independent)

## How It Works

### Hooks (2 total)
1. **App_FrameUpdate epilogue** (0x46C1F1) — 5-byte trampoline. Runs the warp state machine every frame (rumble, flash, hold, fade, load, reveal).
2. **Node-point proximity scanner** — scans MeshWorld Section 1 nodes for `WARP(Name)` entries each frame. No collision hook needed.

### Game Systems Used

- **Ball freeze**: `ball+0x808=1000` (impact/freeze counter) — blocks `Ball_ApplyForce` so the player can't steer, but physics continues normally.
- **Ball vibration**: `ball+0x2D4=1` (native render jitter flag) — same mechanism as the Up Race vacuum. CPUID-based random jitter applied in the render function.
- **Ball color fade**: Writes RGBA floats to `board+0x3AB0` (P1 color table, same as ball_tint mod). Original color saved on first frame, lerped to yellow `(1,1,0)` over the RUMBLE phase.
- **Ball alpha fade**: `ball+0x2FC` (alpha float) lerped from 1.0 to 0.5 over RUMBLE. Game overwrites alpha to 1.0 each frame, so continuous writes are required.
- **Ball invisible**: `ball+0x2FC=0.0` (alpha) — forced only when the screen reaches full white (`g_whiteAlpha >= 0.99`). Respects respawn flag (`ball+0x2F9`).
- **Timer freeze**: `board+0x2990` (tournament timer, int) saved at flash peak and written back every frame through HOLD/FADE/LOAD phases. This purely freezes the timer value — does NOT set goal-reached or player-finished flags, so no race-end sequence (popup, music) is triggered.
- **Music fade**: Manual volume ramp over 3.0 seconds. Writes `MusicChannel+0x528` (volume) and calls `BASS_ChannelSetAttributes` directly. Does NOT use game's fade-out flag (cuts too fast).
- **White screen**: Game's native fade at `board+0x3624` (float alpha). The board render function reads this every frame and draws a fullscreen rect through the game's own material pipeline — no D3D state corruption.
- **Level load**: `App_StartPracticeRace(app, levelIndex)` — same as practice race menu selection. Difficulty is saved and restored around the call. Tournament mode is preserved by saving/restoring profile flags, scores, and timer.

### Phase Timing
All timing uses `GetTickCount()` for framerate independence. Timestamps are initialized when the warp triggers (in the collision handler), so elapsed=0 on the first frame.

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

- **v7f**: Pause-aware timing — replaced all GetTickCount() calls in the warp state machine with a pause-aware game clock. When the game is paused (board+0x874 set by Scene_CreateGameOverMenu), the warp clock freezes, pausing the entire effect sequence (rumble, flash, fade, music fade) with the game. Resumes seamlessly when unpaused.
- **v7e**: Three fixes: (1) Music fade — clear game's fade_out/fade_in/fade_rate flags on all music channels before starting manual volume ramp, preventing MusicChannel_FadeUpdate from fighting our writes. (2) 1-frame visual gap — immediately write white alpha to the new board after App_StartPracticeRace returns, before the next frame renders. (3) Score corruption — board+0x2994 is the score (int), not a timer; stopped writing savedTimeRemaining to it (only board+0x2990 = time limit is correct).
- **v7d**: Timer freeze — tournament timer (`board+0x2990`) freezes the moment the ball vanishes (flash peak). Purely freezes the value — no goal-reached or player-finished flags set, so no race-end sequence triggered. Timer stays frozen through HOLD/FADE/LOAD phases. Updated README.
- **v7c**: Color fade to yellow + alpha fade to 50% during RUMBLE. Ball color lerps from original to `(1,1,0)` over 2s. Ball alpha lerps from 1.0 to 0.5 over 2s (progressive ghostly effect).
- **v7b**: Ball invisible at flash peak — ball stays visible during flash ramp-up, turns invisible only when `g_whiteAlpha >= 0.99` (screen fully white).
- **v7**: Node-point proximity trigger (no collision hook). Scans MeshWorld Section 1 for `WARP(Name)` entries. Native render jitter (`ball+0x2D4`). Removed `DispatchCollisionEvents` hook entirely.
- **v6f**: Native render jitter, no pause, no sin/cos oscillation.
- **v6e**: Renamed JIGGLE → RUMBLE phase. Added HOLD phase. Tripled FADE duration. Halved FLASH. Fixed ball invisibility. Fixed screen fade.
- **v6d**: Code review cleanup — removed 155 lines of dead code.
- **v6**: Race index off-by-one fix. FVF set before DrawPrimitiveUP. Inline asm clobber fix.
- **v5**: D3D texture stage state restoration. GetTickCount-based timing. vsnprintf. PHASE_REVEAL.
- **v4**: Initial special effects edition.

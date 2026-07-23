# Level Warp Mod (v8.5)

Warp between levels by touching WARP(levelname) nodes placed in the MESHWORLD.

## v8.5 Changes (current)

- **Heliotrope purple ball color during RUMBLE.** Ball color multiplier (ball+0x2AC/0x2B0/0x2B4) is lerped from original to #db03fc (heliotrope purple) over the 2-second RUMBLE phase. These are the same fields the magnifying glass heat system uses — Ball_Render reads them every frame and passes to Graphics_SetColorMultiplier (0x00401160). Color is restored to original in PHASE_LOAD.
- **Physics freeze on ball disappear.** When the ball vanishes at peak white flash (PHASE_FLASH), `ball+0x2CC` (`in_tar` flag) is set to 1 — freezing ball physics. Cleared back to 0 in PHASE_LOAD alongside other ball state restores.
- **Tournament timeout abort.** If the tournament timer expires during RUMBLE or early FLASH (before `g_freezeTimer` is set), the warp is aborted. Checks `App+0x5D6` (obj+0x0A "finished" flag, set by Board_UpdateRaceState when timer < 0). On abort: restores physics state (jitter, in_tar, impact freeze), music, timer, pause — but **keeps the purple color and partial alpha** so the ball remains heliotrope purple and faded during the game's natural timeout popup (RaceResultPopup + "Game Over" music).

## v8 Changes

- **Replaced NOP-based timer freeze with code caves.** v7 NOP'd 3 FPU instructions (FSUB + 2× FADD) on `board+0x3624` to freeze the timer. We discovered `board+0x3624` is the **fade alpha**, not the race timer. The actual race timer is `App+PID*0xA0+0x5E8` (obj+0x1C), decremented by `DEC [EDI+0x1C]` at `0x41B3EB`.
- v8 patches the `JNZ+DEC` (9 bytes at `0x41B3E5`) and `JNZ+INC` (5 bytes at `0x41B50C`) with JMPs to code caves. The caves check both the original "finished" flag (`obj+0x0A`) and a custom `g_freezeTimer` byte. If either is set, the DEC/INC is skipped — same passive mechanism the game uses for N:GOAL, but without N:GOAL side effects.
- Deleted ~200 lines of NOP save/restore logic and per-frame `board+0x3624` write-back code.

## How It Works

1. Level designers place a node named `WARP(levelname)` in the MESHWORLD Section 1 (e.g. `WARP(3)` or `WARP(neon)`)
2. When the player ball gets within 30 units (XZ) and 50 units (Y) of the node, the warp sequence triggers
3. Effect sequence: RUMBLE (2s) → FLASH (0.15s) → HOLD (1s) → FADE (2s) → LOAD → REVEAL (1s)
4. During RUMBLE, ball color lerps to heliotrope purple (#db03fc) and alpha fades from 1.0 to 0.5
5. At peak FLASH, ball goes invisible (alpha=0), physics freezes (`in_tar=1`), and timer freezes
6. If tournament timer expires mid-warp (before timer freeze), the warp aborts — ball stays purple/faded, game's timeout popup plays out normally
7. The race timer is frozen during the warp via code caves (no N:GOAL side effects)
8. After loading the new level, all ball state is restored (color, alpha, in_tar, jitter, impact freeze) and timer is unfrozen

## Ball color multiplier system

The ball's runtime color is controlled by `ball+0x2AC` (R), `ball+0x2B0` (G), `ball+0x2B4` (B). These are read by `Ball_Render` (0x00403DB8) every frame and passed to `Graphics_SetColorMultiplier` (0x00401160), which stores a 4×4 diagonal matrix at `gfx+0x7B0`.

This is the **same system** the magnifying glass heat system uses: when `ball+0xC50 > 0`, Ball_Render subtracts the heat value from G and B, shifting the ball from white → red.

**Do NOT use `ball+0x20C/0x210/0x214`** (material diffuse) — these are NOT read by the color multiplier path and writing to them has no visible effect.

## Tournament timer timeout detection

`Board_UpdateRaceState` (0x41B130) decrements `obj+0x1C` (= `App+0x5E8` for P0) each frame. When it drops below 0:
- Sets `obj+0x0A` (= `App+0x5D6` for P0) = 1 (finished flag)
- Sets `ball+0x14C = 1` (ball death)
- Creates `RaceResultPopup` and plays "Game Over" music

The warp mod checks `App+0x5D6` every frame during RUMBLE and early FLASH. If set, the warp aborts immediately.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v8.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Files

- `bass.dll` — compiled mod (rename original bass.dll to bass_real.dll)
- `warp_mod_v8.c` — source code
- `level_warp_v8_5.zip` — distributable package
- `warp_log.txt` — diagnostic log (auto-generated next to bass.dll)

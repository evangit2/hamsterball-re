# Level Warp Mod (v8)

Warp between levels by touching WARP(levelname) nodes placed in the MESHWORLD.

## v8 Changes

- **Replaced NOP-based timer freeze with code caves.** v7 NOP'd 3 FPU instructions (FSUB + 2× FADD) on `board+0x3624` to freeze the timer. We discovered `board+0x3624` is the **fade alpha**, not the race timer. The actual race timer is `App+PID*0xA0+0x5E8` (obj+0x1C), decremented by `DEC [EDI+0x1C]` at `0x41B3EB`.
- v8 patches the `JNZ+DEC` (9 bytes at `0x41B3E5`) and `JNZ+INC` (5 bytes at `0x41B50C`) with JMPs to code caves. The caves check both the original "finished" flag (`obj+0x0A`) and a custom `g_freezeTimer` byte. If either is set, the DEC/INC is skipped — same passive mechanism the game uses for N:GOAL, but without N:GOAL side effects.
- Deleted ~200 lines of NOP save/restore logic and per-frame `board+0x3624` write-back code.

## How It Works

1. Level designers place a node named `WARP(levelname)` in the MESHWORLD Section 1 (e.g. `WARP(3)` or `WARP(neon)`)
2. When the player ball gets within 30 units (XZ) and 50 units (Y) of the node, the warp sequence triggers
3. Effect sequence: RUMBLE (2s) → FLASH (0.25s) → HOLD (1s) → FADE (2s) → LOAD → REVEAL (1s)
4. The race timer is frozen during the warp via code caves (no N:GOAL side effects)
5. After loading the new level, the timer is unfrozen and the saved time/score is carried over (tournament mode)

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v8.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Files

- `bass.dll` — compiled mod (rename original bass.dll to bass_real.dll)
- `warp_mod_v8.c` — source code
- `warp_log.txt` — diagnostic log (auto-generated next to bass.dll)

# _mknp_devmode

F8 toggles a `devmode` variable. A status line top-left always shows the state:

- `devmode: off`
- `devmode: on`

## Install

Rename `_mknp_devmode.dll` to `bass.dll`, keep the stock DLL as `bass_real.dll`.

## How it works

- **F8 poll** runs on a private background thread (`Sleep(16)` loop, edge-detect so holding F8 toggles exactly once). Zero game interaction, works in every game state.
- **Status draw** piggybacks the game's own text rendering: a `Font_DrawGlyph (0x00457440)` entry detour runs the original via trampoline (SEH setup replayed verbatim), then draws one status line with the live hook font through an emitted machine-code stub (no inline-asm pushes).
- Font cascade: live hook font first, then App slots `0x318/0x320/0x324`, with per-glyph readiness probing and null-gating throughout.
- Log: `mknp_devmode.log` next to the DLL (`%TEMP%` fallback), INIT line on load, one line per toggle.

## Files

- `_mknp_devmode.dll` — the compiled mod (rename to `bass.dll` to install)
- `source/mknp_devmode.c` — source code
- `source/bass_proxy.h` — shared BASS proxy boilerplate
- `source/bass.def` — undecorated BASS export definitions
- `source/build.sh` — cross-compile script

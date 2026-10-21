# _mknp_devmode

F8 toggles a `devmode` variable. A status line top-left always shows the state:

- `devmode: off`
- `devmode: on`

## Set file

Copy `_mknp_devmode.set` next to the installed DLL. Add more `"Name": { "keybinding": "X" }` entries (A-Z, 0-9) as new actions land — unknown names log `DEVBIND unhandled action` and change nothing.

## Install

Rename `_mknp_devmode.dll` to `bass.dll`, keep the stock DLL as `bass_real.dll`.

## How it works

- **F8 poll** runs on a private background thread (`Sleep(16)` loop, edge-detect so holding F8 toggles exactly once). Zero game interaction, works in every game state.
- **Status draw** piggybacks the game's own text rendering: a `Font_DrawGlyph (0x00457440)` entry detour runs the original via trampoline (SEH setup replayed verbatim), then draws two status lines per frame (devmode line + 4-state indicator) through an emitted machine-code stub (no inline-asm pushes).
- A `0x46C1F1` frame-epilogue hook arms exactly one draw per frame (no blink); small font first (App `0x320` gothic16, then `0x318/0x324`, hook font last); two-pass shadow (black +1,+1, then white).
- Dev keybinds from `_mknp_devmode.set` (jsonc, next to the DLL): each `"Action": { "keybinding": "X" }` fires on CTRL+X only while devmode is ON (1s global cooldown, devmode OFF = nothing). Set hot-reloads on change.
- WAIT freeze: while devmode is ON and the indicator is WAIT, the game is frozen with its native pause (`board+0x874` held at 1, prior value restored on every WAIT exit). WAIT-enter logs `board/name/flag` resolution facts. The native pause menu may show while frozen. Dev keys are never eaten. Identity guard: `App+0x178` must name a `*Board`; menu shells are never written.

- Indicator states: white `-` rest, yellow `RESET BEST TIME: ...` level select (Left/Right cycle ALL<->LEVEL01..15 with wrap, Enter confirms, Esc cancels), green `DONE - ...` success, red `FAIL - ...` failure. Success/fail revert to rest after 3s. Turning devmode off mid-select cancels.
- `Time Trials - Reset Best Times` (CTRL+B from rest only): writes the native fresh-profile default `0x05F5E09C` to all 20 BestTime slots (`App+0x86c`, LEVEL01..15 = slots 0..14, ALL = slots 0..14). The default is NOT zero (verified in the loader: missing registry = `rep stos` fill). The indicator line shows `Time Trials - Best Times - reset` for 3s.
- Indicator line under the devmode line, same format with shadow, slightly smaller (0.8x via the native per-font scale at `Font+0x428` — the game itself writes `0x3F4CCCCD` there). Resting text `-`; dev events show a message for 3s, then revert.
- Devmode remembered: F8 toggle persists in `_mknp_devmode.state` next to the DLL (loaded at boot, default OFF).
- Log: `_mknp_devmode.log` next to the DLL (`%TEMP%` fallback), INIT line on load, one line per toggle.

## Files

- `_mknp_devmode.dll` — the compiled mod (rename to `bass.dll` to install)
- `_mknp_devmode.set` — keybinding map (copy next to the installed DLL, unrenamed)
- `source/mknp_devmode.c` — source code
- `source/bass_proxy.h` — shared BASS proxy boilerplate
- `source/bass.def` — undecorated BASS export definitions
- `source/build.sh` — cross-compile script

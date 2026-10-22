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
- Loader blindspot: while the current object names a `Loading*` screen the draw hook runs orig text only (no overlay) — the loader's first draw is poisonous to hook into. Polled at 2Hz (`LOADERGATE on/off` in log). Every other screen keeps the overlay.
- WAIT freeze+lock: while devmode is ON and the indicator is WAIT/WAIT2, the native pause is held AND pause-bound inputs are eaten (LL keyboard eats arrows/Enter/Escape + all but F8/Ctrl/Alt; LL mouse eats right-button), so the game can neither pause nor unpause. Selector fed from hook flags (pid-based foreground gate, `GetAsyncKeyState` fallback). Prior flag restored on every WAIT exit. A 400ms message quarantine plus a release-hold (freeze kept until Enter/Escape/right-button are physically up, 1.5s cap) stop the exit keypress from pausing the game via DirectInput.
- Overlay: DrawGlyph hook is branchless dual-mode (CMOV routes orig return: boot = straight back to caller, post-boot = via afterCode which draws then returns). Frame hook draws until `POSTBOOT` arms the AFTER path, then stands down (no double-draw). Loader gate + NULL-font skip enforced in C.

- Set Medals (`Time Trials - Set Medals`, CTRL+M): two yellow questions — WAIT1 `TT - SET MEDALS: LEVELxx` (level, ALL first), Enter locks it and asks WAIT2 `TT - SET MEDALS - LEVELxx - MEDAL: X` (BRONZE/SILVER/GOLD/WEASEL, arrows cycle, Esc goes back to WAIT1). Writes cumulative earned bytes at `App+0x8BC` (byte0=BRONZE/byte1=SILVER/byte2=GOLD, proven vs threshold/board mapping). WEASEL has no persistent byte in this build, so it sets all three (max visible state).

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

- v03a-diag: same behavior as v02z plus a 40-line draw-path trace (`DTRACE NULLFONT/GATED/THROTTLE/NOFONT/PICK`) and a 10s `BEAT hits/draws/gate/pres` heartbeat so a Windows log shows exactly what the menu/race text path does.

- v03b: sprite-hook piggyback — draws right before each menu/HUD sprite with the game's own font atlas already bound (the cold-call DrawGlyph never paints because no atlas is bound in our hook phases). Same stable shape, no aftercode.

- v03c-fix: sprite post-call cave preserves ECX (live texture) via EAX/EDX only; LIFO stash fixes nested returns (was EIP=-1, then EIP=0).

- v03d: self-contained pixel-text overlay (raw D3D8, 3x5 font, stateblock save/restore) — no game fonts needed; verified live on title screen.

- v03e: double-size pixel text (line1 x4/line2 x2, per-pass flush), WAIT1/WAIT2 never auto-revert, medals exclusive tiers + realtime apply on every arrow.

- v03f: medals back to cumulative tiers (BRONZE/SILVER/GOLD/WEASEL=all), per-level vs ALL scope kept, realtime arrows kept.

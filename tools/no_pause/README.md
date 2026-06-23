# No-Pause Mod

Prevents the pause menu from appearing when ESC is pressed. The game continues running normally — no pause overlay, no physics freeze, no camera stop.

## How It Works

Hamsterball checks for the ESC key every frame inside `Scene_Update` (0x419c00):

```
00419d52: PUSH 0x2                    ; ESC key combo ID
00419d54: CALL 0x00428f10             ; Input_CheckKeyCombo(app, 2)
00419d59: TEST AL,AL                 ; was ESC pressed?
00419d5b: JZ 0x00419d66              ; skip if NOT pressed (74 09)
00419d5d: PUSH 0x1
00419d5f: MOV ECX,ESI
00419d61: CALL 0x0040a920            ; Scene_CreateGameOverMenu(scene, 1)
```

When ESC is pressed, `Scene_CreateGameOverMenu` creates a `PauseMenu` UI overlay and sets `scene+0x874 = 1` (the pause flag). While this flag is set, the game skips all physics and scene update vtable calls.

### The Patch

A single-byte change at address `0x419d5b`:

| Byte | Original | Patched | Effect |
|------|----------|---------|--------|
| 0x419d5b | `74` (JZ) | `EB` (JMP) | Always jump past pause creation, even when ESC is pressed |

This changes `JZ 0x419d66` (conditional skip when ESC not pressed) to `JMP 0x419d66` (unconditional skip — never create the pause menu).

## Installation

1. Extract the zip into your Hamsterball game folder (next to `Hamsterball.exe`)
2. Run `install.bat`
3. Launch the game

## Uninstallation

Run `uninstall.bat` to restore the original `bass.dll`.

## Technical Details

- **Mod type:** BASS.dll proxy (v3 lazy loader pattern)
- **Patch type:** Single byte — conditional jump → unconditional jump
- **Patch address:** RVA 0x19d5b (VA 0x419d5b)
- **Original byte:** `74 09` (JZ +0x09)
- **Patched byte:** `EB 09` (JMP +0x09)
- **Side effects:** None — the ESC key is simply ignored. No menu, no pause, no freeze. The game runs identically to not pressing ESC.
- **Crash test:** PASSED (35s Wine/Xvfb, exit 124 = timeout kill, no crash)

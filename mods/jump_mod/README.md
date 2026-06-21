# jump_mod

Press SPACE to jump (Player 1 only, grounded check).

## Behavior

- **SPACE** applies an upward velocity impulse to Player 1's ball.
- **Grounded requirement:** The ball must be touching level geometry this frame.
  The mod checks `Ball+0x2E9` (`on_surface`), which is set to 1 inside
  `Ball_Update`'s type-5 floor-collision branch. If the ball is airborne
  (midair from a previous jump, ramp, or fall), the jump is blocked.
- **No double-jump:** Edge detection on SPACE ensures one jump per keypress.
  The `on_surface` check ensures one jump per ground contact.
- **Death state excluded:** `Ball+0xC4C` (`fall_mode`) is also checked —
  the ball cannot jump while in the fall-off-level/respawn state.

## Files
- `jump_mod.c` — C source code (BASS proxy + code cave hook)
- `bass.dll` — Compiled DLL (PE32 i386)
- `jump_mod.zip` — Packaged zip

## Technical Details

- **Hook point:** End of `Ball_Update` (0x004082B6) via code cave.
  Runs after physics + collision detection completes, so `on_surface` is fresh.
- **Ground check:** `Ball+0x2E9` (`on_surface`) — set by type-5 floor collision
  in `Ball_Update`. This is the authoritative ground-contact flag, unlike
  `Ball+0x281` (`is_falling`, legacy/init only) or `Ball+0xC4C` (`fall_mode`,
  death/respawn animation state).
- **Keyboard:** Reads the game's own DirectInput8 buffer (not `GetAsyncKeyState`).
  Chain: `App → App+0x180 → InputHandler → InputHandler+0x434 → KeyboardDevice → +0xC + DIK_SPACE`.

## Proxy Type
BASS.dll proxy. Installation:
1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod's `bass.dll` into the game folder
3. Launch Hamsterball

# jump_mod

Press SPACE to jump (Player 1 only, cooldown-based ground check).

## Behavior

- **SPACE** applies an upward velocity impulse (500.0) to Player 1's ball.
- **Cooldown-based anti-double-jump:** After jumping, a 60-frame (~1 second)
  cooldown timer prevents re-jumping. This covers the entire jump arc:
  - Apex at ~33 frames (velocity 500 - gravity×33 ≈ 0)
  - Landing at ~66 frames
  - The 60-frame cooldown blocks midair re-jumps through most of the arc
- **Ramp-friendly:** No velocity threshold check — you can jump while rolling
  down ramps, on slopes, or moving at any speed. The cooldown is the only
  midair protection.
- **Death state excluded:** `Ball+0xC4C` (`fall_mode`) is checked —
  the ball cannot jump while in the fall-off-level/respawn state.
- **Edge detection:** SPACE uses rising-edge detection (one jump per keypress).

## Files
- `jump_mod.c` — C source code (BASS proxy + code cave hook)
- `bass.dll` — Compiled DLL (PE32 i386)
- `jump_mod.zip` — Packaged zip

## Technical Details

- **Hook point:** End of `Ball_Update` (0x004082B6) via code cave.
- **Cooldown:** 60-frame counter stored in `g_jump_cooldown` (volatile DWORD).
  Decremented each frame. When >0, jump is blocked. Set to 60 after each jump.
- **Keyboard:** Reads the game's own DirectInput8 buffer.
  Chain: `App → App+0x180 → InputHandler → InputHandler+0x434 → KeyboardDevice → +0xC + DIK_SPACE`.

## Previous approaches that didn't work

1. **`Ball+0x281` (`is_falling`):** Legacy/init flag, not read during Ball_Update.
   Did not prevent midair jumps at all.
2. **`Ball+0x2E9` (`on_surface`):** Documented as a ground-contact flag, but actually
   a sticky limit/trajectory flag (`E:LIMIT` arena event). Once set to 1, it stays 1
   until respawn — caused inverted behavior (can't jump on ground, can jump midair).
   The clears in `Ball_FindClosestRespawnPoint` use `int*` pointer arithmetic
   (`param_1 + 0x2e9` = byte offset `0xBA4`), NOT byte offset `0x2E9`.
3. **Velocity threshold:** Would block jumping on ramps (user wants ramp jumps).

## Proxy Type
BASS.dll proxy. Installation:
1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod's `bass.dll` into the game folder
3. Launch Hamsterball

# jump_mod v2

Press SPACE to jump (Player 1 only, collision-based ground check).

## What changed from v1

**v1 crash fix:** The background raycast thread in v1 called
`Mesh_FindClosestCollision` from a separate thread while Ball_Update
was building its collision tree on the main thread. This data race
corrupted the CollisionLevel's internal AABB state, causing crashes
at `0x407BCA` (vtable call on garbage pointer) in Ball_Update.

**v2 fix:** No background thread. Ground detection uses the game's
own type-5 (floor) collision result:

1. **Cave 1** hooks the type-5 handler at `0x407391` (where
   `ball+0x2E9 = 1` is set). It also sets `g_on_ground = 1`.
2. **Cave 2** at Phase 15 (`0x407BB4`) reads `g_on_ground` for jump
   permission, then clears it to `0` at the end of each frame.
3. If the ball is airborne next frame, no type-5 fires, `g_on_ground`
   stays `0`, and jumping is blocked.

No raycast, no threads, no data races.

## Features

- **Collision-based ground detection:** Uses the game's own type-5
  floor collision result. No raycast thread, no data races.
- **Instant re-jump:** The player can jump again the exact frame they
  touch ground after a jump arc. No artificial cooldown.
- **Ramp-friendly:** Type-5 fires on ramps too, so jumping on slopes
  works naturally.
- **Air momentum:** Horizontal velocity is preserved during jump arcs.
- **Safety checks:** Won't jump during fall/respawn (ball+0xC4C) or
  if not Player 1.
- **Edge detection:** SPACE uses rising-edge detection (one jump per keypress).

## Hook points

| Hook | Address | Original bytes | Purpose |
|------|---------|---------------|---------|
| Cave 1 | `0x407391` | `C6 86 E9 02 00 00 01` (7 bytes) | Type-5 floor → set `g_on_ground` |
| Cave 2 | `0x407BB4` | `8B 4C 24 1C 8B 11` (6 bytes) | Jump logic + clear `g_on_ground` |

## Files

- `jump_mod.c` — C source code (BASS proxy + two code caves)
- `bass.dll` — Compiled DLL

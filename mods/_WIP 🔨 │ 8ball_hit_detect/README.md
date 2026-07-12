# 8-Ball Hit Detection Mod

Detects whenever a player ball collides with an 8-ball (NPC). **Pure detection only — no gameplay changes.**

## How It Works

Hooks `Ball_Update` (0x405E00) at address 0x406FD1 — the start of the ball-ball collision scoring section. At this point in the code:

- **ESI** = this ball (running Ball_Update)
- **EDI** = other ball (collision partner)
- Both balls are confirmed colliding (past the `collision_type == 1` check)

The mod checks `ball+0x18` (player_index) on both balls:
- `0-3` = Player 1-4
- `-1` (0xFFFFFFFF) = NPC 8-ball

If exactly one ball is a player and the other is an 8-ball, it increments `g_hit_count` and appends a line to `hitlog.txt` in the game directory.

Uses pointer comparison (ESI < EDI) to avoid double-counting, since Ball_Update runs for both balls in a collision pair.

## Log Output

The mod writes to `hitlog.txt` in the Hamsterball game directory. Example:

```
[Hit 1] Player 1 struck an 8-ball
[Hit 2] Player 1 struck an 8-ball
[Hit 3] Player 2 struck an 8-ball
```

## Installation

1. Rename the original `bass.dll` → `bass_real.dll` in the Hamsterball directory
2. Copy the mod's `bass.dll` to the same directory
3. Launch Hamsterball.exe

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll 8ball_hit_detect.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Technical Details

| Item | Value |
|------|-------|
| Hook address | 0x00406FD1 |
| Original instruction | `fld dword [edi+0x284]` (6 bytes) |
| Hook type | JMP code cave (5-byte JMP + 1 NOP) |
| Activation delay | 5 seconds after DLL load |
| Player index offset | ball+0x18 (int: -1 = NPC, 0-3 = player) |
| Hit counter | `g_hit_count` (volatile DWORD, readable via debugger) |

## Verification

Hook site bytes at 0x406FD1 (must match before patching):
```
D9 87 84 02 00 00
```

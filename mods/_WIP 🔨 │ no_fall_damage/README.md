# No Fall Damage Mod

## Info
- **File**: `bass.dll` (proxy)
- **Effect**: Player ball never dies from falling off edges
- **Toggle**: F8 key or `no_fall_damage.txt` config file
- **Android/Wine-safe**: No IAT hooks, no code caves, no threads for byte patches

## What it does

Prevents the player ball from dying when falling off level edges. The game normally kills the ball when it **stops moving** on its primary axis after going off an edge (not from a timer — see [ball-fall-shatter-system.md](https://github.com/kkuhn317/hamsterball-re/blob/main/docs/agent-knowledge/ball-fall-shatter-system.md) for full analysis).

Patches 2 points in `Hamsterball.exe`:

| # | Address | Original | Patch | Effect |
|---|---------|----------|-------|--------|
| 1 | 0x40C761 | `74 07` (JZ +7) | `90 90` (2× NOP) | Skip the stopped-moving death check |
| 2 | 0x40C767 | `C6 85 E9 02 00 00 01` (7 bytes) | `90` ×7 (NOP) | Prevent falling flag (ball+0x2E9) from being set |

## How it works

1. On load: creates `no_fall_damage.txt` next to the DLL (if missing)
2. Background thread polls every 30ms
3. Re-reads config file every ~2 seconds
4. Applies byte patches using `VirtualProtect` + `memcpy`
5. Verifies original bytes before patching (won't double-patch)
6. Restores original bytes on DLL unload
7. F8 toggles the mod on/off at runtime

## Config File

`no_fall_damage.txt` (auto-generated):

```
enabled
# No Fall Damage mod
# Set to 'enabled' or 'disabled'
# F8 toggles on/off at runtime
```

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll no_fall_damage.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game folder
3. On Android/Wine: set DLL override to `native` for `bass.dll`

## Technical Details

### Player Ball Fall Death (NOT timer-based)

The player ball dies when it **stops moving** on its primary axis after going off an edge:

1. **Edge detection** (collision type 5): ball rolls past surface edge → `ball+0x2E9 = 1` (falling mode), camera follows
2. **Stopped-moving check**: each frame while falling, checks if `|pos - prev_pos| < threshold` on the primary axis (selected by `ball+0x1D2`)
3. **Death**: if stopped, calls `vtable[8]` = `Ball_FallDeath` (0x409480) → sets `ball+0x2E8 = 1` (needs_respawn)
4. **Respawn**: `Ball_FindClosestRespawnPoint` teleports ball to last SAFESPOT

### Patch Strategy

This mod takes a **dual approach**:

- **Patch 1** (NOP the JZ): Prevents the stopped-moving check from branching to the death path
- **Patch 2** (NOP the flag write): Prevents `ball+0x2E9` (falling flag) from being set at all

This ensures the ball never enters the falling/death state, so it can fall indefinitely without dying. The ball will still collide with surfaces below normally.

### Important: Verify Patch Addresses

The patch addresses in this mod are for the **V3.6.c** binary. You MUST verify them against your specific `Hamsterball.exe` using Ghidra or a disassembler. The byte sequences depend on the compiler version and binary. If the original bytes don't match, the patches will silently fail (safe but non-functional).

To verify:
1. Open `Hamsterball.exe` in Ghidra
2. Navigate to `Ball_Update` (0x405E00)
3. Find the stopped-moving check (look for `ball+0x2E9` and the conditional jump)
4. Confirm the byte sequences match `g_patches[]` in the source

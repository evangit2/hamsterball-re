# Global Object Functions Loader

## What This Does

This CEA script makes level-specific object collision functions work **globally** on ALL levels.

### How It Works

1. **Hooks** the collision dispatch call (`vtable[29]`) at both call sites in Ball_Update (0x40728F) and Ball_FallUpdate (0x408B85)
2. **Calls the original collision handler first** — preserves all native level events
3. **After the original returns**, checks the collision mesh name for known event prefixes
4. If matched (e.g. `N:BUMPER`), runs that object's physics globally

### Current Objects

| Object | Event Tag | Physics |
|--------|-----------|---------|
| Bumper | `N:BUMPER` | Normalizes ball velocity to 12.0 (horizontal only, Y=0) |

### Adding More Objects

Copy the Bumper block pattern in the CEA code:

```cea
  // ============================================
  // Load SpeedCylinder function globally
  // ============================================
  push N                        // string length
  push str_speedcyl             // pointer to "N:SPEEDCYLINDER" string
  push eax                      // mesh name pointer
  call 004C7677                 // _strnicmp
  add esp, 0C
  test eax, eax
  jnz not_speedcyl

    // ... object physics code here ...

  jmp cave1_done
  // ----------------- End SpeedCylinder -----------------
not_speedcyl:
```

### Bumper Physics Details

- **physMesh** = `[ball+0x1A4]` (collision physics mesh pointer)
- **Velocity** at physMesh+0xCA4 (X), +0xCA8 (Y), +0xCAC (Z)
- Kick normalizes horizontal velocity (XZ plane) to magnitude **12.0**
- Y velocity zeroed (horizontal kick only)
- If ball is stationary (speed = 0), kick is skipped

### Key Addresses

| Address | Description |
|---------|-------------|
| 0x40728F | Call site 1 (Ball_Update collision dispatch) |
| 0x408B85 | Call site 2 (Ball_FallUpdate collision dispatch) |
| 0x004C7677 | `_strnicmp` function |
| 0x4CFD34 | "N:BUMPER" string in game data |
| 0x4CF3DC | 12.0 float constant |

### Important Notes

- **Disable before exiting a level** to avoid stale pointer crashes
- The bumper kick is **idempotent**: normalizing velocity to 12.0 twice gives the same result, so levels that natively handle bumpers are unaffected
- Original game uses 5.0 as minimum kick for slow balls; this script always uses 12.0 for simplicity

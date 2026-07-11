# SnowMod — "Snow Storm" Visibility Mod

Makes all races look like a blinding snow storm with heavy fog and white background.

## What It Does

1. **Heavy fog** — Reduces visibility by setting fog start very close (50 units) and fog end very tight (300 units). The ball is barely visible in the distance, simulating a snow storm.
2. **White background** — Changes the background clear color to pure white (snow), applied before the frame is rendered.

## How to Use

1. Open Cheat Engine, attach to `Hamsterball.exe`
2. Load `SnowMod.CEA`
3. Enable the script
4. Start any race — the world will be foggy white

## Adjustable Parameters

Edit these in the CE address list (add them manually via `Add Address Manually`):

| Address | Symbol | Type | Default | Description |
|---------|--------|------|---------|-------------|
| `snow_fog_start` | `snow_fog_start` | Float | 50.0 | Distance from camera where fog begins. 0 = fog right at camera. |
| `snow_fog_end` | `snow_fog_end` | Float | 300.0 | Distance where fog is total (fully opaque). Lower = less visibility. |
| `snow_fog_color` | `snow_fog_color` | D3DCOLOR (4-byte) | `FFFFFFFF` (white) | Fog and background color. Format: `0xAARRGGBB`. |

### Tuning Tips

- **Blizzard mode**: Set `snow_fog_end` to 100.0 — you'll barely see 100 units ahead
- **Light snow**: Set `snow_fog_end` to 1000.0 — gentle haze
- **Fog at camera**: Set `snow_fog_start` to 0.0
- **Colored storm**: Change `snow_fog_color` — try `FF808080` for gray fog, `FFC0C0FF` for ice blue

## Technical Details

Two hooks, both traced from Ghidra disassembly:

### Hook 1: Background Color (0x46C214)

In `App_ResetFrame` (0x46C200), before the game clears the viewport. The game reads the background color from `gfx+0x738` and passes it to `ClearViewport`. Our hook writes white to `gfx+0x738` before the read, so the background clears to white.

Since `gfx+0x738` is also the fog color (read at 0x454CEE in `Graphics_RenderScene`), this also makes the fog white.

### Hook 2: Fog Parameters (0x454CD3)

In `Graphics_RenderScene` (0x454BC0), before the game calls `FUN_004539A0` to apply fog parameters. The game reads fog start from `gfx+0x73C` and fog end from `gfx+0x740`. Our hook writes our custom values before the read.

### D3D8 Fog States Set by the Game

| State | Value | Address |
|-------|-------|---------|
| D3DRS_FOGENABLE (8) | 3 (enabled) | 0x4546C6 |
| D3DRS_FOGVERTEXMODE (0x23) | 0 or 3 | 0x454CE9 (via FUN_004539A0) |
| D3DRS_FOGTABLEMODE (0x8C) | 0 or 3 | 0x454CE9 (via FUN_004539A0) |
| D3DRS_FOGSTART (0x24) | float from gfx+0x73C | 0x454CE9 (via FUN_004539A0) |
| D3DRS_FOGEND (0x25) | float from gfx+0x740 | 0x454CE9 (via FUN_004539A0) |
| D3DRS_FOGCOLOR (0x22) | dword from gfx+0x738 | 0x454D00 |
| D3DRS_RANGEFOGENABLE (0x30) | 0 | 0x454CE9 (via FUN_004539A0) |

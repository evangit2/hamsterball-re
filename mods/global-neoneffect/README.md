# Global Neon Effect

Turns every race and arena into a dark Neon Race — everything goes dark, and the player's ball produces its own light (white glow), just like how Neon Race works.

## How Neon Race Works (Binary Trace)

### Key Functions
| Function | Address | Role |
|----------|---------|------|
| `Scene_SetupLevelDark` | 0x416270 | Loads LevelDark.MESHWORLD, sets `obj+0xC80=1` on all scene objects |
| `RumbleBoard_Neon_Init` | 0x416F40 | Same for Neon Arena |
| `Graphics_RenderScene` | 0x454BC0 | Reads `gfx+0x730` (ambient) → `SetRenderState(D3DRS_AMBIENT, value)` |
| `Ball_Render_AI` | 0x402DE0 | Checks `obj+0xC80`: if dark, sets ambient=white → renders ball → sets ambient=black |
| `Player Ball Render` | 0x403DC0 | Does NOT check `+0xC80` — no built-in glow for player ball |
| `Graphics_SetupLights` | 0x454630 | Generic D3D render state setup (not neon-specific) |
| `Level_InitScene` | 0x40B090 | Sets random ambient via `Gfx_PackColorRGB` |

### The Neon Dark+Glow Mechanism
1. **Dark ambient**: `gfx+0x730` (D3DRS_AMBIENT, render state 0x8B) is set to black/dark from MESHWORLD Section 4 data
2. **Ball glow**: When `Ball_Render_AI` sees `obj+0xC80=1` (dark mode flag):
   - Calls `SetRenderState(0x8B, 0xFFFFFFFF)` — white ambient
   - Renders the 8Ball mesh (ball appears bright in the dark)
   - Calls `SetRenderState(0x8B, 0x00000000)` — black ambient restored
3. **Player ball**: Uses `Player Ball Render` (0x403DC0) which does NOT have the `+0xC80` check — so the player ball doesn't glow natively in Neon Race (it relies on MESHWORLD lights placed near the start position)

### How This Mod Replicates It Globally

| Hook | Address | What It Does |
|------|---------|-------------|
| Ball_Update | 0x405E22 | Sets `gfx+0x730=0` (black ambient) + `ball+0xC80=1` on all balls |
| Player Render Entry | 0x403DCE | Sets `gfx+0x730=0xFFFFFFFF` (white ambient — ball glows) |
| Player Render Exit | 0x4050D9 | Sets `gfx+0x730=0` (black ambient restored after ball renders) |

The entry/exit hooks wrap the player ball render: ambient goes white during render (ball visible), then back to black (everything else stays dark). AI 8-balls get their own glow from the `+0xC80` flag in `Ball_Render_AI`.

## Usage

1. Open `GlobalNeonEffect.CEA` in Cheat Engine
2. Activate the script
3. Start any race or arena
4. Toggle `g_NeonEnabled` between 0/1 to turn effect on/off

**Hotkey**: Set Numpad 1 → toggle `g_NeonEnabled` (0↔1) in CE table.

## CE Address Reference

| Symbol | Description |
|--------|-------------|
| `g_NeonEnabled` | 1 = neon effect on, 0 = off |
| `g_GfxDevice` | Cached gfx device pointer (set each frame by Ball_Update hook) |

## Gfx Device Struct Offsets

| Offset | Type | Field | Render State |
|--------|------|-------|-------------|
| +0x154 | ptr | D3D device | — |
| +0x730 | u32 | Ambient color | D3DRS_AMBIENT (0x8B) |
| +0x734 | byte | Fog enable | D3DRS_FOGENABLE (0x1C) |
| +0x738 | u32 | Fog color | D3DRS_FOGCOLOR (0x22) |

## Ball Struct Offsets

| Offset | Type | Field |
|--------|------|-------|
| +0x10 | ptr | App* (→ App+0x174 = gfx device) |
| +0x18 | int32 | player_index (0=P1, -1=AI) |
| +0xC80 | byte | Dark mode flag (1 = glow in AI render) |

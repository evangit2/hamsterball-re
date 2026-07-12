# Global Neon Mod — Spawn Objects + Neon Lighting

Two independent features, toggled via CE address symbols:

1. **Spawn Neon Objects** — Spawns 6 Neon Race objects at ball position
2. **Neon Lighting** — Darkens the scene and attaches a point light to the ball

## Controls

| Symbol | Address (CE) | Default | Description |
|---|---|---|---|
| `SpawnNeon` | alloc'd | 0 | Set to 1 to spawn neon objects at ball position |
| `NeonLighting` | alloc'd | 1 | 1 = neon lighting ON (dark scene + ball light), 0 = OFF |
| `NeonAmbient` | alloc'd | 0x000C0C14 | Ambient color (0x00RRGGBB). Lower = darker. Set to 0 for pitch black. |

## Feature 1: Neon Object Spawning

Spawns all 6 Neon Race objects on any level with a flag.

### Objects Spawned

1. **NEONPLATFORM** — Disappearing neon floor (alloc 0x10EC, ctor 0x43E110)
2. **DFLOOR1** — Disappearing floor 1 (alloc 0x1104, ctor 0x43E450)
3. **DFLOOR2** — Disappearing floor 2
4. **DFLOOR3** — Disappearing floor 3
5. **DFLOOR4** — Disappearing floor 4
6. **TRODE** — Neon electrode/tube (alloc 0x1104, ctor 0x43E450)

### Usage

1. Load `GlobalNeon.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnNeon` to 1 in CE (or use a hotkey to set it)
4. All 6 objects spawn at the player's position, spread out in a line

### Mesh Loading

All 6 meshes are loaded on first spawn (cached):
- `Levels\LevelDark-NeonPlatform` → mesh_neonplatform
- `Levels\LevelDark-DFloor1` → mesh_dfloor1
- `Levels\LevelDark-DFloor2` → mesh_dfloor2
- `Levels\LevelDark-DFloor3` → mesh_dfloor3
- `Levels\LevelDark-DFloor4` → mesh_dfloor4
- `Levels\LevelDark-Trode` → mesh_trode

### Object Placement

- NEONPLATFORM: at player position (X, Y, Z)
- DFLOOR1: Y+20 (above player)
- DFLOOR2: X+20
- DFLOOR3: X+40
- DFLOOR4: X+60
- TRODE: Z+20

## Feature 2: Neon Lighting

Replicates Neon Race's dark atmosphere on any level. The scene background and ambient are darkened, and a D3D point light is attached to the ball position.

### How It Works

The hook intercepts `Graphics_RenderScene` (0x454BC0) per frame:

1. **Darkens ambient** — Overrides gfx+0x730 (ambient color) with `NeonAmbient` value
2. **Disables fog** — Sets gfx+0x734 = 0, gfx+0x738 = black
3. **Enables D3D lighting** — Calls `SetRenderState(D3DRS_LIGHTING, TRUE)`
4. **Sets point light at ball position** — Reads ball XYZ from gfx+0x854/858/85C (populated by `Scene_Render`), updates a pre-allocated `D3DLIGHT8` structure, and calls `SetLight(7, ...)` + `LightEnable(7, TRUE)` on the D3D device

### Light Properties

| Property | Value |
|---|---|
| Type | D3DLIGHT_POINT |
| Diffuse | (0.8, 0.9, 1.0) — cool white-cyan |
| Specular | (0.3, 0.4, 0.5) — faint highlight |
| Range | 500.0 |
| Attenuation0 | 0.0 (no constant) |
| Attenuation1 | 0.02 (linear falloff) |
| Light Index | 7 (unused by game's default lights) |

### Tuning

- **Darkness**: Change `NeonAmbient` in CE. `0x00000000` = pitch black, `0x00202020` = dim, `0x00404040` = moderate.
- **Light brightness**: Edit the `light_data` diffuse values in the script (offsets 4-12 in the D3DLIGHT8 struct).
- **Light range**: Edit offset 64 in `light_data` (float). 500.0 = covers visible area, 250.0 = tight spotlight.
- **Light falloff**: Edit offset 72 (Attenuation1). Higher = shorter range. 0.02 = gradual, 0.05 = steep.

### Hook Points

| Hook | Address | Function | Original Bytes |
|---|---|---|---|
| 1 (spawn) | 0x405E22 | Ball_Update | `8B 86 5C 0C 00 00` (6 bytes) |
| 2 (lighting) | 0x454BC0 | Graphics_RenderScene | `81 EC C0 00 00 00` (6 bytes) |

### Technical Notes

- The lighting hook fires once per frame, before `Graphics_SetupLights` is called by the original function. This means the game's own lighting setup runs afterward, but our ambient/light overrides persist because we write directly to the gfx struct and D3D device state.
- Ball position (gfx+0x854/858/85C) is written by `Scene_Render` (0x41A2E0) each frame before `Graphics_RenderScene` is called, so the light position is always current.
- Light index 7 is used to avoid conflicts with the game's own light setup (which uses indices 0-3 at most).
- When `NeonLighting` is set to 0, the hook disables light 7 and lets the game's original lighting run unmodified.
- `D3DLIGHT8` struct is 104 bytes (not 88 — includes Direction vector between Position and Range). Position is at offset 52 (three floats). The struct is pre-filled in the script with all necessary fields; only Position.x/y/z are updated per-frame.

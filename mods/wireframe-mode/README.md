# Wireframe Mode Toggle

CEA script that toggles all 3D geometry into wireframe rendering. 2D sprites and text remain solid.

## How It Works

**Hook point:** `Graphics_BeginFrame` at `0x453B50` — called at the start of every frame. ECX = Graphics object (`this`).

**Mechanism:** The hook does two things when wireframe is enabled:
1. Calls `device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME)` to set fill mode to wireframe
2. Writes `1` to the fillmode cache byte at `graphics+0x70d` to trick the game's internal caching

### Why the cache trick? (v2 fix)

The game resets `D3DRS_FILLMODE` to `1` (POINT) at multiple points DURING 3D rendering:

| Location | Address | Condition | What it does |
|----------|---------|-----------|--------------|
| `Ball_Render` | `0x40433E` | `if [graphics+0x70d] != 1` | `SetRenderState(7, 1)` |
| `FUN_00402860` | `0x004028F5` | `if [graphics+0x70d] != 1` | `SetRenderState(7, 1)` |
| `CopyVec2` | `0x45E05B` | `if [graphics+0x6fc] != 2` | `SetRenderState(7, 1)` |

Both `Ball_Render` and `FUN_00402860` check a **cache byte** at `graphics+0x70d`. If it's already `1`, they skip the `SetRenderState(7, 1)` call entirely. By writing `1` to this cache in our hook, we trick the game into thinking FILLMODE is already set to POINT, so it never overrides our wireframe setting.

The `CopyVec2` path (0x45E05B) checks a different cache (`graphics+0x6fc` against `2`), but it only runs during specific render state transitions (when `6fc` is not `2`), which happens less frequently. If it triggers, it will override wireframe for that draw call, but most 3D meshes will render in wireframe.

### Sprite/2D handling

`Graphics_SetRenderMode` (0x454190) is called by sprite-drawing functions (`Sprite_DrawRect`, `Graphics_DrawScreenRect`, etc.) to restore solid fill mode for 2D elements. Since sprites set their own fill mode before drawing and reset it after, wireframe only applies to 3D meshes — 2D UI stays readable.

## Key Addresses

| Component | Address | Details |
|-----------|---------|---------|
| Graphics_BeginFrame | `0x453B50` | Hook target, 7-byte trampoline |
| Graphics object | ECX at entry | `this` pointer |
| D3D Device | graphics+0x154 | `IDirect3DDevice8*` |
| SetRenderState | vtable+0xC8 | `device->SetRenderState(state, value)` |
| Fillmode cache | graphics+0x70d | Cache byte — game skips SetRenderState(7,1) if already 1 |
| D3DRS_FILLMODE | 7 | D3D8 render state constant |
| D3DFILL_WIREFRAME | 2 | Wireframe fill mode |
| D3DFILL_SOLID | 3 | Normal solid fill mode |
| Ball_Render reset | 0x40433E | `SetRenderState(7, 1)` — skipped if cache=1 |
| FUN_00402860 reset | 0x004028F5 | `SetRenderState(7, 1)` — skipped if cache=1 |
| CopyVec2 reset | 0x45E05B | `SetRenderState(7, 1)` — different cache check |

## Usage

1. Load `wireframe-mode.CEA` in Cheat Engine
2. Activate the script
3. Set `g_WireframeEnabled` to `1` (ON) or `0` (OFF)
4. Optional: Set a hotkey in CE to toggle the value

## Files

- `wireframe-mode.CEA` — Pure CEA script, no Lua, no DLL

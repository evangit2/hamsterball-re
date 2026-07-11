# Wireframe Mode Toggle

CEA script that toggles all 3D geometry into wireframe rendering. 2D sprites and text remain solid.

## How It Works

**Hook point:** `Graphics_BeginFrame` at `0x453B50` — called at the start of every frame from `App_Run`, `Level_RenderObjects`, `Sprite_RenderQuad`, `Ball_RenderWithMaterial`, etc. This is the universal per-frame entry point for all 3D rendering.

**Mechanism:** The hook intercepts before the frame begins, reads the Graphics object (ECX = `this`), extracts the D3D8 device pointer from `graphics+0x154`, then calls:

```
device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME)
```

Since `Graphics_SetRenderMode` (0x454190) is called by sprite-drawing functions to restore solid fill mode for 2D elements, wireframe will only apply to 3D meshes — 2D UI stays readable.

## Key Addresses

| Component | Address | Details |
|-----------|---------|---------|
| Graphics_BeginFrame | `0x453B50` | Hook target, 7-byte trampoline |
| Graphics object | ECX at entry | `this` pointer |
| D3D Device | graphics+0x154 | `IDirect3DDevice8*` |
| SetRenderState | vtable+0xC8 | `device->SetRenderState(state, value)` |
| D3DRS_FILLMODE | 7 | D3D8 render state constant |
| D3DFILL_WIREFRAME | 2 | Wireframe fill mode |
| D3DFILL_SOLID | 3 | Normal solid fill mode |

## Usage

1. Load `wireframe-mode.CEA` in Cheat Engine
2. Activate the script
3. Set `g_WireframeEnabled` to `1` (ON) or `0` (OFF)
4. Optional: Set a hotkey in CE to toggle the value

## Files

- `wireframe-mode.CEA` — Pure CEA script, no Lua, no DLL

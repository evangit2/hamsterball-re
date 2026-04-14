# Hamsterball Rendering Pipeline

## Overview

The rendering pipeline is a 3-pass system: opaque objects first, then alpha-
blended objects, then shadow/depth objects. It uses D3D8 Direct3D with
custom sorting and z-buffer interleaving.

## Pipeline Entry Points

```
App_Run (0x46BD80)
  └─ App::Render() [vtable call]
       └─ Graphics_BeginFrame (0x453B50)
            └─ Graphics_RenderScene (0x454BC0)
                 ├─ Graphics_SetupLights
                 ├─ Gfx_SetViewMatrix
                 ├─ Matrix_ComputeFrustum (0x4762B0)
                 ├─ 8 render callbacks at Graphics+0x710
                 ├─ D3DRS_ZENABLE, ZFUNC, ZWRITEENABLE restore
                 └─ Graphics_SetViewportZ
            └─ Scene_RenderAllObjects (0x45E0E0)
                 ├─ Pass 1: Opaque objects (flag 0x85F, 0x860, 0x862, 0x863 all 0)
                 ├─ Pass 2: Alpha objects (flag 0x860=1)
                 ├─ Pass 3: Shadow objects (flag 0x85F=1)
                 └─ Cleanup callbacks
       └─ Graphics_PresentOrEnd (0x455A90)
            └─ IDirect3DDevice8::Present
```

## Graphics_RenderScene (0x454BC0)

Core D3D8 scene setup:
1. **SetupLights** — configure D3D lights
2. **Copy matrices** — World (Graphics+0x224), View (Graphics+0x264), Projection (Graphics+0x2A4)
3. **SetViewMatrix** — apply view transform, compute frustum
4. **D3D SetTransform** — `device->vtable[0x94](0x100, &projection)` — set projection matrix
5. **Copy back world matrix**, recompute frustum
6. **8 render callbacks** — function pointers at Graphics+0x710..0x72C
7. **Restore render states** — ZENABLE (0x8B), ZFUNC, ZWRITEENABLE (0x1C)
8. **SetViewportZ** — restore Z range from Graphics+0x73C/0x740
9. **Final callbacks** — Graphics+0x5C vtable[0x88] and vtable[0x78]

### Graphics Object Matrix Offsets
| Offset | Size | Description |
|--------|------|-------------|
| +0x224 | 64B (4x4) | World matrix |
| +0x264 | 64B (4x4) | View matrix |
| +0x2A4 | 64B (4x4) | Projection matrix |

## Scene_RenderAllObjects (0x45E0E0)

**Signature**: `Scene_RenderAllObjects(this, do_sorting, render_mode)`

### Sorting Phase (param_1 != 0)
1. Clear 3 lists: shadow list (this+0x488), alpha list (this+0x8A0), deferred list (this+0xCB8)
2. If ball rendering enabled (Scene+0x480→+0x434): call `Ball_InitRenderState`

### Object Classification
Each scene object has flags at these offsets:
| Flag | Offset | Meaning |
|------|--------|---------|
| 0x85F | +0x85F | Shadow caster |
| 0x860 | +0x860 | Alpha blended |
| 0x861 | +0x861 | Depth bias (z-fight fix) |
| 0x862 | +0x862 | Deferred render |
| 0x863 | +0x863 | Skip rendering |

### Pass 1: Opaque Objects (all flags = 0)
For objects where **no flags are set**:
1. Set render context: `obj+0x83C = Graphics+0x7C4` (current frame counter)
2. Compute material address: `obj_index * 0x50 + mesh_base_ptr` (at mesh+0x28)
3. Call `Graphics_ApplyMaterialAndDraw(graphics, material_address)`
4. If ball mesh NOT in cutscene mode (`Scene+0x480→+0x10C4 == 0`):
   - Iterate material list (obj+0x424), set D3D stream source per sub-material
   - device->vtable[0x118](5, stride, offset) — SetStreamSource
5. If ball IS in cutscene: set vertex declaration and draw

### Pass 2: Alpha Objects (flag 0x860 = 1)
Objects with alpha flag are collected into `this+0x8A0` list:
1. Disable Z-write: `D3DRS_ZENABLE = 0` (device vtable[200](0xE, 0))
2. For each alpha object:
   - Apply material and draw
   - Handle sub-materials same as opaque pass
3. Re-enable Z-write: `D3DRS_ZENABLE = 1` (device vtable[200](0xE, 1))

### Pass 3: Shadow Objects (flag 0x85F = 1)
Objects with shadow flag are collected into `this+0x488` list:
1. **Adjust projection** for depth bias: `proj_x += bias`, `proj_y += bias` (DAT_004CF308)
2. Enable stencil: `D3DRS_STENCILENABLE = 1` (device vtable[200](0x37, 1))
3. For each shadow object:
   - If depth-bias flag (0x861) set: toggle Z-write mid-render
   - Apply material and draw
4. Disable stencil: `D3DRS_STENCILENABLE = 3` (device vtable[200](0x37, 3))
5. **Restore projection**: `proj_x -= bias`, `proj_y -= bias`
6. Re-enable Z if needed

### Post-Render Cleanup
Iterate scene children at `this+0x480→+0x428`, call `vtable[0x48](1, 1)` for each
(child cleanup callback).

## Render Pass D3D State Changes

| Pass | Z-write | Z-test | Stencil | Alpha blend |
|------|---------|--------|---------|-------------|
| Opaque | ON | ON | OFF | OFF |
| Alpha | OFF | ON | OFF | ON |
| Shadow | ON | ON | ON | OFF (depth bias) |

## SceneObject Render Flags (at object+offset)

| Offset | Flag | Description |
|--------|------|-------------|
| +0x83C | uint | Frame counter (set to Graphics+0x7C4 each render) |
| +0x858 | uint | Vertex declaration index (for cutscene mode) |
| +0x850 | uint | Vertex buffer offset (for SetStreamSource) |
| +0x861 | byte | Depth bias flag — toggles Z-write during shadow pass |

## Key Render Functions

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x453B50 | Graphics_BeginFrame | 39 | Begin D3D frame, clear backbuffer |
| 0x454BC0 | Graphics_RenderScene | — | Full scene setup (lights, matrices, frustum) |
| 0x45E0E0 | Scene_RenderAllObjects | 33 | 3-pass object render (opaque→alpha→shadow) |
| 0x455D60 | Graphics_DrawScreenRect | 63 | 2D rectangle via TLVERTEX tristrip |
| 0x455110 | Graphics_ApplyMaterialAndDraw | 17 | Set material + draw primitive |
| 0x455A90 | Graphics_PresentOrEnd | — | D3D Present or EndScene |
| 0x454A30 | Gfx_SetViewMatrix | — | Set D3DTS_VIEW matrix |
| 0x453900 | Graphics_Clear | — | D3D Clear (target+Z or Z+stencil) |
| 0x4539A0 | Graphics_SetupFog | — | Linear fog: FOGSTART, FOGEND, FOGCOLOR |
| 0x460450 | Scene_RenderBallShadow | 38 | Ball shadow with depth bias pass |
| 0x460DA0 | Scene_RenderFrame | 38 | Full scene render with z-buffer interleaving |
| 0x454190 | Graphics_SetRenderMode | — | Set shading mode, vertex shader, render states |

## Graphics Object Key Offsets

| Offset | Type | Description |
|--------|------|-------------|
| +0x07C | IDirect3D8* | D3D8 interface |
| +0x154 | IDirect3DDevice8* | D3D device |
| +0x224 | float[16] | World matrix (4x4) |
| +0x264 | float[16] | View matrix (4x4) |
| +0x2A4 | float[16] | Projection matrix (4x4) |
| +0x2E4 | AthenaList* | Texture cache |
| +0x5C | void** | Secondary render vtable (callbacks 0x78/0x88) |
| +0x70C | byte | Z-write state cache (0=off, 1=on) |
| +0x710 | void*[8] | 8 render callback function pointers |
| +0x730 | uint | Cached ZENABLE state |
| +0x734 | byte | Cached ZWRITEENABLE state |
| +0x738 | uint | Cached ZFUNC state |
| +0x748 | void* | Frustum data (for Matrix_ComputeFrustum) |
| +0x790 | float | Projection X offset (for depth bias) |
| +0x794 | float | Projection Y offset (for depth bias) |
| +0x7C4 | uint | Frame counter (incremented each render) |
| +0x7C8 | uint | Render call counter |
| +0x7CC | uint | Sub-material draw counter |

## Depth Bias Value

`DAT_004CF308` is a small float offset added to the projection matrix X/Y during
the shadow render pass. This pushes shadow geometry slightly forward in clip
space to prevent z-fighting with the floor geometry.

## Mirror Mode

When `App+0x7D2` (mirror_cull_flag) is set:
- `Gfx_SetCullMode(0x427940)` changes D3DRS_CULLMODE
- `Gfx_SetViewMatrix` flips the view matrix horizontally
- This effectively mirrors the entire 3D scene
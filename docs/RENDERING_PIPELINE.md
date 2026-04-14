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

## Level Rendering Pipeline

### Level_UpdateAndRender (0x40B600)
6-phase render for level geometry and objects:
1. **Build visible_list** from primary (+0x29D4) and secondary (+0x3204) ball object lists
2. **Opaque pass**: AlphaTest OFF, iterate both lists calling vtable[0x1C]() per object
3. **Alpha pass**: AlphaTest ON, iterate both lists calling vtable[0x1C]() per object
4. **Waypoint arrow**: If race active and not game-over, show next waypoint arrow (timer=0.45s)
5. **Visible list render**: iterate combined visible_list, call vtable[0x08]() per object
6. **Ball shadows**: If ripple_list has entries, render shadows for all balls

### Level_RenderObjects (0x40B570)
Transparent pass renderer:
1. Graphics_BeginFrame(gfx, 0)
2. level->vtable[0x4C]() - render level mesh (terrain geometry)
3. Graphics_BeginFrame(gfx, 0)
4. For each object in visible_list: vtable[0x0C]() (RenderTransparent)

### Scene_RenderFrame (0x60DA0)
Per-frame scene update with vertex buffer construction:
1. Set scene+0x10C4 = 1 (render active)
2. If root scene: allocate MeshWorld + transform buffer
3. Sprite update: call vtable[0x3C] per sprite
4. **Triangle strip construction**: For each mesh object, build zigzag strips:
   - Alternating vertex triples (degenerate-triangle strip pattern)
   - 0x20 bytes per vertex (pos + normal + texcoord)
   - SpriteAnim_SetRange for each object
5. Font_RenderToTextureComplex for text→texture
6. Mesh_SaveAndFree + free temp MeshWorld

### Scene_CheckPath (0x57EC0)
Ring topology pathfinder on 360-cell (0x167=359) circular grid.
Used by Ball_Update for track-snapping collision.
- Returns 1 = forward reachable, -1 = backward reachable, 0 = unreachable
- Two walkers: forward (+1) and backward (-1), wrap at 0↔359

### Scene_SpawnBallsAndObjects (0x41C5B0)
Level startup: spawn player balls + all interactive objects:
1. For each start entry: lookup "START%d-%d" in hash table for position
2. Ball_ctor2(0xC60 bytes) at position, radius=26.0, max_speed=5.0, gravity=0.5
3. 1-player random start for race types 5/11/12/14
4. Scan SAFESPOT/SAFEPOS entries
5. Tournament/demo: CreateBadBall + CreateMouseTrap
6. CreateSecretObjects + Scene_CreateFlags + Scene_CreateSigns + Scene_CreateDynamicObjects

## D3D8 Device Vtable Dispatch Map

The game uses IDirect3DDevice8 COM vtable calls throughout. Key vtable offsets:

| Vtable Offset | D3D Method | Usage in Game |
|---------------|-----------|---------------|
| +0x28 | DrawIndexedPrimitiveUP | D3DDevice_DrawIndexedPrimitiveUP (0x47DD56) |
| +0x2C | DrawIndexedPrimitive | Graphics_DrawIndexedPrimitive (0x47DFB9) |
| +0x78 | EndScene | Post-render cleanup |
| +0x88 | BeginScene | Pre-render setup |
| +0x94 | SetTransform | Matrix setup (View/Projection/World/Texture) |
| +0xA8 | SetMaterial | Apply D3DMATERIAL8 |
| +0xF4 | SetTexture | Texture stage 0 setup |
| +0xFC | SetTextureStageState | Multi-texture config |
| +0x118 | SetStreamSource | Vertex buffer binding |
| +0x14C | DrawPrimitiveUP | Immediate-mode vertex draw |
| +0xC8 | SetVertexShader | FVF / declarator setup |
| +0x200 (0x200 = SetRenderState) | SetRenderState | D3D render state changes |

### Graphics_DrawIndexedPrimitive (0x47DFB9)
Thin wrapper: `device->vtable[0x2C](device, 0, 0, prim_type, index_count | 0x800)`
- The 0x800 flag is D3DPT_TRIANGLELIST with additional index buffer flags
- Called from Graphics_ApplyMaterialAndDraw

### D3DDevice_DrawIndexedPrimitiveUP (0x47DD56)
User-pointer draw: parses vertex declaration via D3DX_ParseDeclarationType,
then calls device->vtable[0x28]. Falls back to cached declaration if NULL.

### Graphics_ApplyMaterialAndDraw (0x455110)
Complex material system dispatch:
1. **Material selection**: gfx+0x7C0 custom material override, or param_1
2. **Scale transform**: If gfx+0x7A8 flag, apply Matrix_ScaleTransform with gfx+0x7B0..0x7BC
3. **Render state setup** based on material type:
   - Simple (material[0x12]==0): Set alpha test mode via D3DTS_ALPHA
   - Complex: Set texture stage, blend mode, cull mode from material struct
4. **D3DDevice->SetMaterial** (vtable[0xA8]) with material+4
5. **Lighting state**: D3DRS_LIGHTING (0x39) per material flag
6. **Transform**: Level_SetObjectTransform sets world matrix
7. **Draw**: If scaled, call vtable[0x4]() (render callback)

## Material System

### Material Structure (0x50 bytes per material)
Materials are stored as arrays of 0x50-byte entries in the mesh vertex data.
Referenced via: `object_index * 0x50 + mesh_base_ptr`

| Offset | Size | Description |
|--------|------|-------------|
| +0x00 | 4 | Material vtable / type flag |
| +0x04 | 64 | D3DMATERIAL8 structure (SetMaterial payload) |
| +0x12 | 4* | Sub-material pointer (NULL=simple) |
| +0x13 | 1 | Alpha blend mode (0=solid, 1=alpha blend) |
| +0x1C | 1 | Source blend factor |
| +0x1D | 1 | Destination blend factor |
| +0x1E | 1 | Cull mode (0=CCW, 1=CW) |
| +0x4D | 1 | Lighting mode (0=off, 1=on) |

### Material Application Flow
```
Graphics_ApplyMaterialAndDraw(gfx, material_ptr)
  ├─ Select material (override or default)
  ├─ Apply scale transform if needed
  ├─ Set alpha test state (D3DRS_ALPHATESTENABLE via vtable[200](0x1B))
  ├─ Set texture blend operation (vtable[0xFC])
  ├─ Set texture (vtable[0xF4])
  ├─ Set material (vtable[0xA8])
  ├─ Set lighting (vtable[200](0x39))
  ├─ Level_SetObjectTransform (world matrix)
  └─ Draw (vtable[4])
```

## Graphics Subsystem Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x453B50 | Graphics_BeginFrame | Set view transform + copy to frustum |
| 0x453900 | Graphics_ClearViewport | D3D Clear (target+Z+stencil) |
| 0x453C90 | Graphics_CreateDevice | D3D8 device creation + FPU control |
| 0x454BC0 | Graphics_RenderScene | Full 3D render: lights+matrices+callbacks |
| 0x454AB0 | Graphics_SetProjection | Set projection matrix |
| 0x455110 | Graphics_ApplyMaterialAndDraw | Material + draw dispatch |
| 0x455A90 | Graphics_PresentOrEnd | D3D Present / EndScene |
| 0x455C50 | Graphics_FindOrCreateTexture | Texture cache lookup/create |
| 0x455D60 | Graphics_DrawScreenRect | 2D rectangle (TLVERTEX) |
| 0x457A50 | Graphics_DisableRenderState | Disable a render state |
| 0x57A90 | Gfx_SetRenderStateThunk | Thin SetRenderState wrapper |
| 0x457AB0 | Gfx_LoadMatrixFromStack | Load matrix from stack to D3D |
| 0x457B50 | Gfx_SetPosition | Set world position |
| 0x457B80 | Gfx_SetPositionAndRender | Position + render |
| 0x457BB0 | Gfx_RotateY | Y-axis rotation |
| 0x457C60 | Gfx_ScaleX | X-axis scale |
| 0x457C90 | Gfx_ScaleY | Y-axis scale |
| 0x457CC0 | Gfx_ScaleZ | Z-axis scale |
| 0x457D40 | Gfx_SetAlphaBlendState | Alpha blend state change |
| 0x459400 | Gfx_ResetRenderState | Reset all render states |
| 0x46F100 | Gfx_ApplyLightingState | Apply lighting from scene |
| 0x46F1E0 | Gfx_ResetLighting | Reset D3D lights |
| 0x53960 | Gfx_CallVTable8C | Call device vtable[0x8C] |
| 0x53970 | Graphics_SetCullMode2 | D3DRS_CULLMODE wrapper |
| 0x53940 | Gfx_ResetRenderState | Full state reset |

## D3D Texture System Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x472B80 | D3DTexture_Ctor | Texture constructor |
| 0x472C00 | D3DTexture_DeletingDtor | Texture destructor (calls Release) |
| 0x48300C | D3DTexture_Init | Initialize from D3D surface |
| 0x483A44 | D3DTexture_InitLocked | Init with locked rect |
| 0x483BA4 | D3DTexture_CreateFromDesc | Create from texture desc |
| 0x483F2A | D3DTexture_CopyIndexData | Copy index (16-bit) |
| 0x483D6D | D3DTexture_CopyLockedData | Copy locked surface data |
| 0x85525 | D3DTexture_CreateSimple | Create simple texture |
| 0x85610 | D3DTexture_CloneFromDesc16 | Clone from 16-bit desc |
| 0x4C3A | D3DTexture_CopySurfaceData | Copy surface data |
| 0x82F9C | D3DResource_ReleaseA | Release D3D resource |
| 0x82FD4 | D3DResource_ReleaseB | Release variant |
| 0x8A900 | D3DResourcePool_Release | Release from pool |

## Mirror Mode

When `App+0x7D2` (mirror_cull_flag) is set:
- `Gfx_SetCullMode(0x427940)` changes D3DRS_CULLMODE
- `Gfx_SetViewMatrix` flips the view matrix horizontally
- This effectively mirrors the entire 3D scene
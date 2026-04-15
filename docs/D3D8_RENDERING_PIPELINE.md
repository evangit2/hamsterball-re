# D3D8 Rendering Pipeline Deep Dive

## Overview
Hamsterball uses Direct3D 8 (D3D8) for all rendering. The pipeline is organized
into multiple passes with specific render states for each. Graphics_RenderScene
(0x454BC0) is the main entry point called each frame.

## Graphics Struct (0x800 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| +0x154 | IDirect3DDevice8* | D3D8 device pointer |
| +0x224 | float[16] | View matrix |
| +0x264 | float[16] | Projection matrix |
| +0x2A4 | float[16] | World matrix |
| +0x5C | void* | D3D8 vtable |
| +0x710 | void*[8] | Render pass list (8 passes) |
| +0x730 | DWORD | Z enable state |
| +0x734 | byte | Z write enable |
| +0x738 | DWORD | Alpha blend enable |
| +0x73C | float | Z near |
| +0x740 | float | Z far |
| +0x748 | Matrix* | Frustum matrix |

## Render Pipeline (Graphics_RenderScene 0x454BC0)

### Pass 1: Setup Lights
```c
Graphics_SetupLights(param_1);  // +0x454630
```
Sets up D3D8 light sources before rendering.

### Pass 2: Copy Matrices
Copies view/projection/world matrices from Graphics struct to local variables:
- +0x224 → local_40 (view, 16 floats)
- +0x264 → local_40 (projection, 16 floats)  
- +0x2A4 → auStack_8c (world, 16 floats)

### Pass 3: Set View Matrix
```c
Gfx_SetViewMatrix(param_1, local_40);  // +0x454C18
D3DDevice_SetTransform(D3DTS_VIEW, ...)
```
Applies view matrix to D3D device.

### Pass 4: Compute Frustum (First Pass)
```c
Matrix_ComputeFrustum(Graphics+0x748);
D3DDevice_SetTransform(D3DTS_PROJECTION, 0x100);
```
Computes view frustum for culling, sets projection.

### Pass 5: Render Pass 1 (Opaque Objects)
Iterates 8 render passes at Graphics+0x710:
```c
for (i = 0; i < 8; i++) {
    if (pass[i] != NULL) {
        pass[i]->vtable[0x3]();  // Render pass
    }
}
```
Pass 1 typically renders opaque geometry (no alpha).

### Pass 6: Z-Buffer Management
```c
// Save current Z state
DWORD saved_z_enable = Graphics+0x730;
byte saved_z_write = Graphics+0x734;

// Disable Z write for alpha pass
D3DDevice_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

// Set viewport Z
Graphics_SetViewportZ(near, far);

// Enable alpha blending  
D3DDevice_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
```
Switches from opaque to alpha-transparent rendering.

### Pass 7: Render Pass 2 (Alpha Objects)
Renders same 8 passes again but with alpha-enabled states.

### Pass 8: Present
```c
D3DDevice_BeginScene();   // vtable[0x88]
D3DDevice_EndScene();     // vtable[0x78]
```
Presents frame to screen.

## Key D3D8 Render States

| State | Value | Purpose |
|-------|-------|---------|
| D3DRS_ZENABLE | 0x8B | Z-buffer enable |
| D3DRS_ZWRITEENABLE | 0x1C | Z write enable |
| D3DRS_ALPHABLENDENABLE | 0x22 | Alpha blending |
| D3DTS_VIEW | 0x100 | View matrix transform |
| D3DTS_PROJECTION | 0x102 | Projection matrix transform |
| D3DTS_WORLD | 0x100 | World matrix transform |

## Scene Rendering (Scene_RenderAllObjects 0x45E0E0)

Called within render passes. Iterates all scene objects:
1. For each gadget in Scene+0x858 (gadget list):
   - gadget->vtable[0xC]() = Render method
2. For each object in Scene+0x25B8 (object list):
   - object->vtable[0xC]() = Render method

## Material & Shader System

### Graphics_ApplyMaterialAndDraw (0x455110)
Applies material properties before drawing:
```c
void Graphics_ApplyMaterialAndDraw(Mesh *mesh, Material *mat) {
    // Set diffuse color
    D3DDevice_SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, ...);
    
    // Set ambient
    D3DDevice_SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, ...);
    
    // Set specular
    D3DDevice_SetRenderState(D3DRS_SPECULARMATERIALSOURCE, ...);
    
    // Apply texture
    if (mat->texture != NULL) {
        D3DDevice_SetTexture(0, mat->texture->d3d_tex);
    }
    
    // Draw mesh
    Mesh_Draw(mesh);
}
```

### Texture Loading (Graphics_FindOrCreateTexture 0x455C50)
- Checks texture cache first
- Loads PNG/BMP via D3DXCreateTextureFromFile
- Stores in cache for reuse
- 7 xrefs - called for all texture loads

## Depth Bias & Shadow Rendering

The game uses depth bias for shadow mapping:
```c
Graphics+0x730 = z_enable flag
Graphics+0x73C = z_near (viewport near)
Graphics+0x740 = z_far (viewport far)
Graphics+0x738 = alpha_blend_enable
```
These are toggled between opaque and alpha passes.

## Viewport Management

### Graphics_SetViewport (0x454F10)
Sets D3D viewport from Graphics struct:
- X, Y position
- Width, Height
- MinZ, MaxZ (depth range)

### Graphics_SetProjection (0x454AB0)
31 xrefs - most called graphics function.
Sets projection matrix with FOV and aspect ratio.

## Lighting

### Graphics_SetupLights (0x454630)
Configures up to 8 D3D lights:
- Light type (point/spot/directional)
- Position/Direction
- Diffuse/Specular/Ambient colors
- Attenuation factors

## Complete Graphics API

| Address | Function | Purpose |
|---------|----------|---------|
| 0x4542C0 | Graphics_ctor | Initialize graphics engine |
| 0x453B50 | Graphics_BeginFrame | Start frame, clear buffers |
| 0x454BC0 | Graphics_RenderScene | Main render entry |
| 0x454AB0 | Graphics_SetProjection | Set perspective projection |
| 0x454B50 | Graphics_SetViewport | Set viewport dimensions |
| 0x454F10 | Graphics_SetViewport | Set viewport (8 xrefs) |
| 0x453C90 | Graphics_CreateDevice | Create D3D8 device |
| 0x455110 | Graphics_ApplyMaterialAndDraw | Material + mesh draw |
| 0x455C50 | Graphics_FindOrCreateTexture | Load/cached texture |
| 0x455D60 | Graphics_DrawScreenRect | 2D screen rectangle (63 xrefs!) |
| 0x455A90 | Graphics_PresentOrEnd | Present to screen |
| 0x454630 | Graphics_SetupLights | Configure D3D lights |
| 0x453900 | Graphics_ClearViewport | Clear color/depth buffers |
| 0x453970 | Graphics_SetCullMode2 | Set cull mode |
| 0x4539A0 | Graphics_SetViewportZ | Set depth range |
| 0x454190 | Graphics_SetRenderMode | Set render mode flags |
| 0x454D30 | Graphics_Reset | Reset graphics state |
| 0x455A60 | Graphics_Defaults | Set default render states |
| 0x454550 | Graphics_Cleanup | Shutdown graphics |
| 0x455360 | Graphics_dtor | Destructor |
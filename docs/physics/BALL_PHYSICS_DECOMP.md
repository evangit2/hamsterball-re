# Hamsterball - Ball Physics Decompilation

## Overview

The ball (hamsterball) is the player-controlled object. It derives from `GameObject` and has a rich vtable with physics, collision, and rendering methods.

## Ball Vtable (0x4CF3A0)

| Offset | Address | Name | Description |
|--------|---------|------|-------------|
| +0x00 | 0x4027F0 | Ball_dtor | Destructor (calls Ball_Cleanup) |
| +0x04 | 0x405100 | (thunk) | Update method (jumps to Ball_Update at 0x405190) |
| +0x08 | 0x402DE0 | Ball_CollisionCheck | Per-frame collision check against mesh |
| +0x0C | 0x402A70 | (not defined) | Possibly render setup |
| +0x10 | 0x408390 | (not defined) | Unknown method |
| +0x14 | 0x401590 | (not defined) | Unknown method |
| +0x18 | 0x402650 | Ball_ApplyForce | Apply directional force (x,y,z,magnitude) |
| +0x1C | 0x402C10 | (not defined) | Unknown method |
| +0x20 | 0x409480 | (not defined) | Unknown method |

Base class vtable: GameObject at 0x4CF314

## Ball Object Layout (0xC98 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| 0x00 | void** | Vtable pointer |
| 0x04 | int | Level reference (param_1 in ctor) |
| 0x08 | int | Parent object reference |
| 0x0C | float[6] | Collision planes (4 component each: aX + bY + cZ + d) |
| 0x42 | Timer | Per-object timer |
| 0x59 | float | Ball X position |
| 0x5A | float | Ball Y position |
| 0x5B | float | Ball Z position |
| 0x62 | float | Ball size (default 0x40400000 = 3.0f) |
| 0x69 | void* | Sound object |
| 0x82 | unknown | Sound data |
| 0x96 | int | Sound handle |
| 0x99 | unknown | More sound data |
| 0xAA | unknown | String data |
| 0xC8 | float | Position component? |
| 0x154 | IDirect3DDevice8* | D3D device pointer (set in render) |
| 0x164 | float | **X position** (authoritative) |
| 0x168 | float | **Y position** (authoritative) |
| 0x16C | float | **Z position** (authoritative) |
| 0x170 | float | **X velocity** (zeroed on collision) |
| 0x174 | float | **Y velocity** (zeroed on collision) |
| 0x178 | float | **Z velocity** (zeroed on collision) |
| 0x17C | float | Acceleration X (zeroed on collision) |
| 0x180 | float | Acceleration Y (zeroed on collision) |
| 0x184 | float | Acceleration Z (zeroed on collision) |
| 0x1A4 | int | Level state reference |
| 0x1C | void** | Physics vtable (for collision mesh interaction) |
| 0x284 | float | Ball radius / height offset |
| 0x2CC | byte | Force disable flag |
| 0x2DC | float | X position (secondary) |
| 0x2E0 | float | Y position (secondary) |
| 0x2E4 | float | Z position (secondary) |
| 0x2F0 | int | Frame counter (affects force scaling) |
| 0x2F8 | byte | Active flag |
| 0x2F9 | byte | Collision occurred flag |
| 0x2FC | int | Physics parameter |
| 0x300 | int | Value 0x96 (150) - possibly mass or timer constant |
| 0x310 | byte | State flag (1 = active) |
| 0x314 | float | Home X position |
| 0x318 | float | Home Y position |
| 0x31C | byte | Home valid flag |
| 0x31D | byte | Home valid flag 2 |
| 0x324 | byte | Special state (shrunk/slow -> affects force multiplier) |
| 0x440 | void* | Allocated physics data |
| 0x6F8 | int | D3D device sub-count |
| 0x6FC | int | Initialized flag for rendering |
| 0x700 | byte | Render mode flag (affects lighting: on/off) |
| 0x708 | int | Render parameter |
| 0x70D | byte | Render state initialized flag |
| 0x734 | byte | Sub-render mode |
| 0x748 | int | Collision mesh pointer (0 = use default) |
| 0x76A | byte | Flag (zeroed on collision) |
| 0x7C8 | int | Render call counter |
| 0x808 | int | Freeze counter (skip force if > 0) |
| 0x810 | AthenaList | Force application list |
| 0x894 | AthenaList | Object tracking list |
| 0xA1 | float | Ball radius (from SIZE parameter) |
| 0x9F | int | Counter (0 = start) |
| 0xC28 | void* | Allocated buffer |
| 0xC2C | char[] | Collision target name buffer |
| 0xC5C | int | Alternate physics state flag (-> different force multiplier) |
| 0xC74 | int | Collision counter (incremented on collision) |
| 0xC80 | byte | Flag (0) |
| 0xC84 | Quaternion | Object rotation (identity = 0,0,0,1) |
| 0xC88 | float[4] | Rotation matrix/quaternion data |

## Key Physics Constants (Data Section)

| Address | Value | Description |
|---------|-------|-------------|
| 0x4CF368 | float | Collision radius threshold |
| 0x4CF374 | float | Force multiplier when C5C flag set |
| 0x4CF378 | float | Force multiplier when 324 flag set (shrunk) |
| 0x4CF380 | float | Force scaling for frame 0 (first frame) |
| 0x4CF3E8 | float | Force direction multiplier |
| 0x4CF48C | float | Gravity/height offset constant |

## Ball_Update (0x405190) - Main Physics Loop

18,499 chars of decompiled code. Key operations:
1. Reset frame state (flags, counters, collision mesh)
2. If collision mesh flag set, call Ball_ResetCollisionMesh (0x4030B0)
3. Iterate through level object list, checking for nearby objects
4. For each object, check distance (sqrt(dx²+dy²+dz²))
5. Find closest collision point using Mesh_FindClosestCollision (0x465D90)
6. On collision:
   - Set ball position to collision point + radius offset
   - Zero out velocity and acceleration
   - Set collision flag (+0x2F9 = 1)
   - Reset physics parameters

## Ball_ApplyForce (0x402650) - Force Application

Takes (float x, float y, float z, float magnitude).
- Skipped if collision flag (+0x2F9) or disable flag (+0x2CC) set
- Skipped if freeze counter (+0x808) > 0
- Frame 0 applies different force scaling (_DAT_004CF380)
- Shrunk state (+0x324) applies multiplier _DAT_004CF378
- Alternate state (+0xC5C) applies multiplier _DAT_004CF374
- Force direction modified by _DAT_004CF3E8
- Updates velocity components at +0xFC, +0x100

## Ball_CheckCollisionPlanes (0x402810) - Plane Collision

Tests ball position against 6 collision planes stored at object+0xC.
Each plane has 4 float components (a,b,c,d) for plane equation: ax + by + cz + d = 0.
Returns byte result: 0 = no collision, 1 = collision detected.

## Ball_CollisionCheck (0x402DE0) - Frame Collision

Called from vtable+8. Performs:
1. Graphics_BeginFrame
2. Ball_CheckCollisionPlanes with ball position + radius 200.0
3. If collision: increment counter, call collision response vtable methods
4. If no collision: decrement counter (clamped to 0)

## Ball_Render (0x402860) - D3D8 Rendering

Sets up D3D8 render states:
- SetRenderState(D3DRS_ZENABLE, 1)
- SetRenderState(D3DRS_CULLMODE, 1)
- SetRenderState(D3DRS_SPECULARENABLE, 1)
- SetTexture(0x112, ...)
- SetRenderState(D3DRS_LIGHTING, flag) based on +0x700 byte
- DrawPrimitiveUP with vertex format depending on +0x700
- Calls FUN_00453970 (sub-object render based on +0x734 mode)

## Collision System Summary

Two-tier collision:
1. **Ball_CheckCollisionPlanes (0x402810)**: Simple plane test for basic boundaries
2. **Mesh_FindClosestCollision (0x465D90)**: Full mesh ray-trace for accurate terrain collision
   - Uses AthenaList to build collision planes
   - Calls FUN_004564C0 for actual intersection math
   - Returns closest hit point with 0.01 precision threshold
# Electric Lights Mod — Design Document

## Overview

A light/dark level mod for Hamsterball Physicus. The ball is the light source,
powered by an electric charge that drains over time. Navigate to recharge pads
to restore energy and see where you're going.

## Core Mechanics

### Energy/Charge System
- Ball starts with some charge (or zero — pitch black start, feel your way to a charger)
- Charge drains over time
- Recharge pads scattered around the level restore energy
- Ball glow radius/brightness is proportional to charge level
  - Full charge = bright neon glow (like Neon Race)
  - Low charge = dim, flickering
  - Zero charge = pitch black

### Light/Dark Collision
- Some meshes only have collision when **illuminated** (in the light) — solid platforms that fade away in darkness
- Some meshes only have collision when **dark** (in shadow) — obstacles that materialize when the light goes out
- Creates dynamic routing — paths that are safe when dim might kill you when bright, and vice versa

### Energy Transfer
- Gates/objects that require you to deposit charge to activate
- Creates risk/reward: give away your light to open a gate, leaving yourself in the dark

## How Neon Race Works (Ghidra-Verified)

### Native Light Follow System

The neon light follows the ball via **Neon board vtable[19]** (0x00424790),
called per-frame from Board_Update (vtable[1] at 0x419C00):

```c
void Neon_UpdateLight(int board) {
    Board_UpdateRaceState(board);  // standard race state update
    
    // P1 light: position at ball + offset
    int ball = App+0x5DC;  // player 1 ball pointer
    SceneObject_SetPosition(board+0x436C, 
        ball.x + 20.0,     // _DAT_004CF370
        ball.y + 30.0,     // _DAT_004CF528
        ball.z - 20.0      // _DAT_004CF370
    );
    
    // P2 light (if P2 exists)
    if (App+0x677 == 0) {
        int ball2 = App+0x67C;
        SceneObject_SetPosition(board+0x4370,
            ball2.x + 20.0,
            ball2.y + 30.0,
            ball2.z - 20.0
        );
    }
}
```

### SceneObject System

- **Size**: 0xD4 bytes (allocated via operator_new(0xD4))
- **Vtable**: 0x004D934C
- **Created in**: Scene_LoadLevelNeon (0x00416270)
- **Stored at**: board+0x436C (P1), board+0x4370 (P2)
- **Registered via**: Scene_RegisterObject(scene, slot, obj) at 0x00453BD0
  - Stores in scene+0x710 + slot*4
  - Sets obj+0x8C = slot index (D3D light index)
  - Calls vtable[3] to initialize the D3D light

### SceneObject Vtable

| Slot | Offset | Address | Function |
|------|--------|---------|----------|
| 0 | 0x00 | 0x0046B650 | dtor |
| 1 | 0x04 | 0x0046B490 | SetPosition(x,y,z) → sets +0x08/0x0C/0x10, calls vtable[3] |
| 2 | 0x08 | 0x0046B4B0 | SetDirection(x,y,z) → sets +0x14/0x18/0x1C, calls vtable[3] |
| 3 | 0x0C | 0x0046B670 | RefreshLight → writes D3DLIGHT8 struct, calls SetLight+LightEnable |
| 4 | 0x10 | 0x0046B4D0 | SetVisible(bool) → sets +0x88, calls vtable[3] |

### Per-Frame Light Update

Graphics_RenderScene (0x00454BC0) iterates 8 SceneObject slots at gfx+0x710:

```c
for (int i = 0; i < 8; i++) {
    if (gfx+0x710[i] != NULL) {
        (gfx+0x710[i])->vtable[3]();  // RefreshLight
    }
}
```

This calls SetLight + LightEnable on the D3D device every frame.

### SceneObject Field Layout

| Offset | Size | Field |
|--------|------|-------|
| +0x00 | 4 | vtable ptr |
| +0x04 | 4 | parent/gfx ptr |
| +0x08 | 4 | Light Position X (set by SetPosition) |
| +0x0C | 4 | Light Position Y |
| +0x10 | 4 | Light Position Z |
| +0x14 | 4 | Direction Target X (set by SetDirection) |
| +0x18 | 4 | Direction Target Y |
| +0x1C | 4 | Direction Target Z |
| +0x20-0x67 | 104 | D3DLIGHT8 struct (written by vtable[3]) |
| +0x88 | 1 | Visible flag |
| +0x8C | 4 | D3D light index (set by Scene_RegisterObject) |
| +0xCC | 4 | Range/intensity (float, 400.0 = 0x43C80000) |
| +0xD0 | 4 | D3DLIGHT8.Type (3 = D3DLIGHT_POINT) |

### D3DLIGHT8 Written by vtable[3]

| D3DLIGHT8 Offset | SceneObject Source | Value |
|------------------|--------------------|-------|
| +0x00 Type | +0xD0 | 3 (POINT) |
| +0x04 Diffuse | +0x94 (Matrix row) | from Vec3_Init(10,10,0) |
| +0x14 Specular | +0xA8 (Matrix row) | from Matrix_Scale4x4(0,0,0,0) |
| +0x24 Ambient | +0xBC (Matrix row) | from Matrix_Scale4x4(0,0,0,0) |
| +0x34 Position | +0x08/0x0C/0x10 | ball position + offset |
| +0x40 Direction | +0x14/0x18/0x1C - +0x08/0x0C/0x10 | target - position |
| +0x4C Range | +0xCC | 400.0 |
| +0x58 Attenuation1 | hardcoded | 0.04 |

### Key Constants

| Address | Value | Description |
|---------|-------|-------------|
| 0x004CF370 | 20.0 | X/Z offset from ball |
| 0x004CF528 | 30.0 | Y offset above ball |
| 0x43C80000 | 400.0 | Default light range/intensity |
| 0x3D23D70A | 0.04 | Attenuation1 (linear falloff) |
| 0x41200000 | 10.0 | Vec3_Init param for diffuse |

### Key Addresses

| Address | Function |
|---------|----------|
| 0x00416270 | Scene_LoadLevelNeon — creates SceneObjects |
| 0x00424790 | Neon vtable[19] — per-frame light position update |
| 0x00453BD0 | Scene_RegisterObject |
| 0x0046B4F0 | SceneObject_ctor |
| 0x0046B490 | SceneObject_SetPosition (vtable[1]) |
| 0x0046B670 | SceneObject_RefreshLight (vtable[3]) |
| 0x00454BC0 | Graphics_RenderScene — iterates 8 light slots |
| 0x00419C00 | Board_Update — calls vtable[19] |

## Implementation Plan

### Phase 1: Charge System + Ball Glow (THIS MOD)

**Approach**: Hook the per-frame Ball_Update (0x405E22, same hook as other RodentRacer mods).
Create a SceneObject light at level start. Each frame, update the light position to follow
the ball (same as Neon vtable[19]) and scale the light intensity based on charge level.

**Charge system**:
- Track charge as float (0.0 to 1.0)
- Drain charge per frame at configurable rate
- Scale light range (SceneObject+0xCC) based on charge:
  - charge = 1.0 → range 400.0 (full glow)
  - charge = 0.5 → range 200.0 (dim)
  - charge = 0.0 → range 0.0 (dark)

**Ball glow color**: Scale ball color multiplier (ball+0x2AC/0x2B0/0x2B4) based on charge
to give visual feedback even beyond the D3D light range.

**Light creation**: At level load, create a SceneObject via the same pattern as Neon Race:
1. operator_new(0xD4) + SceneObject_ctor(gfx_ptr)
2. Set obj+0x34 = 1 (type = point light)
3. Set obj+0xCC = 400.0 (range)
4. Call Scene_RegisterObject(scene, slot, obj) to register

**Light update**: Each frame in Ball_Update hook:
1. Read ball position (ball+0x164/168/16C)
2. Call SceneObject_SetPosition(obj, ball.x, ball.y+30, ball.z)
3. Scale obj+0xCC based on charge level
4. Update ball color multiplier for glow effect

### Phase 2: Recharge Pads (SEPARATE MOD)
- Scan S1 ref points for `CHARGER` prefix
- Proximity detection restores charge

### Phase 3: Light/Dark Collision (SEPARATE MOD)
- MeshBuffer name scanning for light-only / dark-only collision
- User says there's "a really easy way to do this" — TBD

### Phase 4: Energy Gates (SEPARATE MOD)
- S1 ref points that consume charge on contact

## Configuration

Expected config values (in mod.c or external config file):
- `CHARGE_DRAIN_RATE` — how much charge lost per frame (default ~0.001)
- `LIGHT_FULL_INTENSITY` — max light range (default 400.0)
- `GLOW_COLOR_R/G/B` — ball glow color (default: white/cyan)

## Mod Pattern

- bass.dll proxy (same as all RodentRacer mods)
- Per-frame hook via Ball_Update (0x405E22)
- SceneObject light creation at level start (Neon Race pattern)
- SceneObject position update per frame (Neon vtable[19] pattern)
- Charge scaling on SceneObject+0xCC (light range)

## Status

- **Concept**: Finalized
- **Phase 1 Implementation**: Starting now
- **Dependencies**: None (standalone mod)

## Related Mods

- **Lava Rising** — uses similar MeshBuffer name scanning (N:LAVA) and ball color tinting
- **Magnet Mod** — uses similar S1 ref point proximity detection pattern
- **Warp Mod** — uses similar ball color multiplier for visual effects
- **GlobalNeon CEA** — XRow's CEA mod that creates a D3D point light following the ball

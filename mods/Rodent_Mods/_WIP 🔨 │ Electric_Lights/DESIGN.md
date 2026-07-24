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

## How Neon Race Works (Reference Implementation)

Decompiled from `Scene_LoadLevelNeon` (0x00416270):

1. Loads `levels\leveldark` — a dark version of the level mesh
2. Sets ball material specular to 0 via `Matrix_Scale4x4(1,1,0,1)` — no shine
3. Creates a **SceneObject** (0xD4 bytes, allocated via `operator_new(0xD4)`)
4. Positions it at `ball.x, ball.y + 30.0, ball.z` — hovers above the ball
5. Sets `obj[0x33] = 0x43c80000` (float 400.0) — light range/intensity
6. Sets `Vec3_Init(10, 10, 0)` for rotation/scale parameters
7. Calls `Scene_RegisterObject(scene, 0, obj)` — registers the light in the scene
8. The light follows the ball because its position is updated from ball coords

### Key Offsets
- SceneObject size: 0xD4 bytes
- Light intensity field: obj + 0x33 (DWORD index) = byte offset 0xCC (float 400.0)
- Light Y offset above ball: 30.0 (from `_DAT_004CF528`)
- Ball position: ball+0x164 (X), ball+0x168 (Y), ball+0x16C (Z)
- Ball color multiplier: ball+0x2AC (R), ball+0x2B0 (G), ball+0x2B4 (B)
- App struct: 0x004FD680
- Scene ptr: 0x005341E4

### Second Light Source (P2)
Neon Race also creates a second SceneObject for Player 2's ball (gated by
`App+0x677 == 0`), following the same pattern but using the P2 ball pointer
at `App+0x67C`.

## Implementation Plan

### Phase 1: Charge System + Ball Glow
- Hook a per-frame function (Ball_Update or Graphics_PresentOrEnd)
- Track charge as a float (0.0 to 1.0)
- Drain charge per frame at configurable rate
- Create a SceneObject light at level start (same as Neon Race)
- Each frame, update light position to follow the ball
- Scale light intensity (obj+0x33) based on charge:
  - charge = 1.0 → intensity 400.0 (full glow)
  - charge = 0.5 → intensity 200.0 (dim)
  - charge = 0.0 → intensity 0.0 (dark)
- Optionally scale ball color multiplier for visual glow effect

### Phase 2: Recharge Pads
- Scan S1 ref points for `CHARGER` prefix (like magnet_mod scans for `MAGNET`)
- When ball is within proximity range of a charger pad, increase charge
- Configurable recharge rate and proximity range
- Visual feedback: charger pads could pulse or glow

### Phase 3: Light/Dark Collision
- Scan MeshWorld for meshes with special name prefixes:
  - `N:LIGHTONLY` — collision enabled only when ball charge > threshold
  - `N:DARKONLY` — collision enabled only when ball charge < threshold
- Toggle collision by modifying MeshBuffer flags at runtime
- Need to investigate how collision flags work on MeshBuffers
  - The game's collision system iterates MeshBuffers for raycast/collision
  - May need to toggle a flag that excludes them from collision checks
  - Alternative: move meshes in/out of the collision AthenaList

### Phase 4: Energy Gates
- S1 ref points named `GATE` that consume charge when ball touches them
- Gate opens (mesh disappears or moves) after consuming charge
- Configurable amount of charge required per gate

## Configuration

Expected config values (in mod.c or external config file):
- `CHARGE_DRAIN_RATE` — how much charge lost per frame (default ~0.001)
- `CHARGE_RECHARGE_RATE` — how much charge gained per frame on charger (default ~0.01)
- `CHARGER_PROXIMITY` — distance to trigger recharge (default ~100.0)
- `LIGHT_FULL_INTENSITY` — max light intensity (default 400.0)
- `LIGHT_DIM_THRESHOLD` — charge level below which light flickers (default 0.2)
- `COLLISION_LIGHT_THRESHOLD` — charge needed for light-only meshes (default 0.3)
- `COLLISION_DARK_THRESHOLD` — charge below which dark-only meshes appear (default 0.3)

## Mod Pattern

- bass.dll proxy (same as all RodentRacer mods)
- Per-frame hook via Ball_Update or Graphics_PresentOrEnd
- S1 ref point scanning for chargers and gates
- MeshBuffer name scanning for light/dark collision meshes
- SceneObject light creation at level start

## Status

- **Concept**: Finalized
- **Implementation**: Not started
- **Dependencies**: None (standalone mod)

## Related Mods

- **Lava Rising** — uses similar MeshBuffer name scanning (N:LAVA) and ball color tinting
- **Magnet Mod** — uses similar S1 ref point proximity detection pattern
- **Warp Mod** — uses similar ball color multiplier for visual effects

# Hamsterball - Struct Definitions

This directory contains C header files defining the core game structures
extracted from decompiler analysis of Hamsterball.exe. These are also applied
as Ghidra data types for improved decompilation.

## Ghidra Structs (Applied)

| Struct | Size | Fields | Source |
|--------|------|--------|--------|
| Ball | 3148 bytes | 38 named fields | Ball_ctor2, Ball_Update, Ball_dtor2 |
| Vec3 | 12 bytes | 3 float fields | Vec3_Init |
| App | 2328 bytes | 22 fields | App_Initialize_Full, App_Run |
| Gadget | 2156 bytes | 16 fields | Gadget_ctor |
| SceneObject | 212 bytes | 15 fields | SceneObject_ctor |
| RumbleBoard | 16 bytes | 4 fields | RumbleBoard_InitTimer |
| RumbleBoardObj | 18380 bytes | 6 fields | RumbleBoard_ctor |

## C Header Files

| File | Struct | Description |
|------|--------|-------------|
| `ball_struct.h` | Ball | Player ball physics object (~0xC98 bytes) |
| `gadget_struct.h` | Gadget | Base class for scene objects (0x870 bytes) |
| `scene_struct.h` | Scene | Main game state container (~0x1000 bytes) |
| `board_struct.h` | Board | Level/board state (inherits Gadget+Scene, ~0x4368) |
| `app_struct.h` | App | Global app singleton (g_App at 0x4FD680) |
| `rumbleboard_struct.h` | RumbleBoard | Haptic feedback timer (0x14 bytes) |

## Inheritance Hierarchy

```
SceneObject (vtable 0x4D934C) - base 3D object with position/rotation/scale
  └─ Gadget (+0x000, vtable 0x4D9170, size 0x870) - adds lists, app_ptr, name
       └─ Board (+0x870 base, vtable 0x4D0260, size ~0x4368) - level state
            └─ RumbleBoard (vtable 0x4D1358, size ~0x47D4) - rumble mode board

Ball (vtable 0x4CF3A0, size 0xC98) - player ball (NOT in Gadget hierarchy)
  - Created by Board_ctor, stored at Board+0x361C
```

## Key Vtables

| Name | Address | Functions |
|------|---------|-----------|
| Ball | 0x4CF3A0 | 9 methods (+0x27F0, +0x5100, +0x2DE0, +0x2A70, +0x8390, +0x1590, +0x2650, +0x2C10, +0x9480) |
| Scene/Board | 0x4D0260 | Scene_DeletingDtor |
| SceneObject | 0x4D9170 | SceneObject_ScalarDtor (Gadget base) |
| SceneObject | 0x4D934C | SceneObject_dtor (standalone) |
| RumbleBoard | 0x4D1358 | RumbleBoard_dtor |
| GameObject | 0x4CF314 | sub2_dtor (Ball ctor initial vtable) |

## Ball Field Map (Key Offsets)

| Offset | Ghidra Name | Type | Value/Description |
|--------|-------------|------|-------------------|
| 0x000 | dwVtable | uint | 0x4CF3A0 |
| 0x008 | nCollision_result | int | Collision system result |
| 0x00C | nString_timer | int | Countdown (200=show), frees display_string |
| 0x010 | dwApp_state | uint | App state ptr |
| 0x014 | dwScene | uint | Scene ptr |
| 0x018 | nPlayer_index | int | -1=AI, 0+=player |
| 0x01C | dwRender_callback | uint | Render/audio vtable |
| 0x108 | dwTimer | uint | Timer_Init target |
| 0x150 | flAccumulated_time | float | Frame delta accumulator |
| 0x158 | flPrev_pos_x/y/z | float[3] | Previous frame position |
| 0x164 | flCur_pos_x/y/z | float[3] | Current position |
| 0x170 | flVel_x/y/z | float[3] | Velocity |
| 0x188 | flMax_speed | float | 5000.0f |
| 0x18C | flSpeed_scale | float | 1.0f |
| 0x1A4 | dwCollision_mesh | uint | CollisionMesh ptr |
| 0x1A8 | flGravity_vec_x/y/z | float[3] | Gravity vector |
| 0x1B8 | dwRender_ctx_1 | uint | RenderContext 1 |
| 0x1C8 | flRender_alpha | float | 0.75f |
| 0x208 | dwRender_ctx_2 | uint | RenderContext 2 |
| 0x20C | flColor_a/r/g/b | float[4] | RGBA (1.0f init) |
| 0x254 | bUses_alpha | byte | color_a != 1.0f |
| 0x264 | pRumble_timer1 | byte[20] | RumbleBoard timer 1 |
| 0x278 | flGravity_scale | float | 0.1f |
| 0x281 | bIs_falling | byte | 1=falling |
| 0x284 | flRadius | float | 27.0f |
| 0x290 | pRumble_timer2 | byte[20] | RumbleBoard timer 2 |
| 0x2A4 | - | float | 5.0f |
| 0x2FC | flTimer_bf | float | 1.0f, countdown |
| 0x768 | bCam_active | byte | Camera follow on |
| 0x764 | flCam_follow | float | 1.0f cam lerp |
| 0xC28 | dwDisplay_string | uint | Timer string ptr |
| 0xC3C | bTeleport_active | byte | Teleport flag |
| 0xC40 | flTeleport_x/y/z | float[3] | Teleport destination |

## Applying Structs

The `create_struct` MCP tool requires JSON format for the `fields` parameter:
```json
[{"name": "field_name", "offset": 0, "type": "uint"}]
```

Ghidra ECX `this` parameters cannot be retyped via API. Plate comments document intended types.
To fully apply: manually retype `this` in Ghidra GUI (right-click -> Retype Variable -> Ball*).
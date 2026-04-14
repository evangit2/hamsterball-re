# Hamsterball - Struct Definitions Index

This directory contains C header files defining the core game structures
extracted from decompiler analysis of Hamsterball.exe.

## Files

| File | Struct | Size | Source | Description |
|------|--------|------|--------|-------------|
| `ball_struct.h` | Ball | ~0xC98 | Ball_ctor2 (0x4039E0), Ball_Update (0x405E00) | Player ball physics object |
| `gadget_struct.h` | Gadget | 0x870 | Gadget_ctor (0x4690F0) | Base class for all scene objects |
| `scene_struct.h` | Scene | ~0x1000 | Scene_dtor (0x419770), Board_ctor (0x419071) | Main game state container |
| `board_struct.h` | Board | ~0x4368 | Board_ctor (0x419071) | Level/board state (inherits Gadget+Scene) |
| `app_struct.h` | App | ~0xA00 | App_Initialize_Full (0x429530) | Global app singleton (g_App at 0x4FD680) |
| `rumbleboard_struct.h` | RumbleBoard | 0x14 | RumbleBoard_InitTimer | Haptic feedback timer sub-object |

## Inheritance Hierarchy

```
SceneObject (vtable 0x4D9170)
  └─ Gadget (+0x000, SceneObject_ScalarDtor)
       └─ Board (+0x870 base, Scene vtable 0x4D0260)
            ├─ RumbleBoard (+0x884, +0x898 embedded timers)
            └─ Ball (+0x361C referenced ptr)
```

## Key Vtables

| Name | Address | Description |
|------|---------|-------------|
| Ball | 0x4CF3A0 | Player ball vtable (9 methods) |
| Scene | 0x4D0260 | Scene/Board vtable |
| SceneObject | 0x4D9170 | Base object vtable (ScalarDtor) |
| GameObject | 0x4CF314 | Game object sub-dtor |

## Applying to Ghidra

The Ghidra MCP `create_struct` tool is broken (returns "No valid fields provided"
for all syntax variations). To apply these structs to Ghidra:

1. **Manual GUI import**: Copy the struct definition into Ghidra's Data Type Manager
2. **Headless script**: Write a Python/Java Ghidra script using the DataTypeManager API
3. **C header import**: Use Ghidra's "Import C Definitions" from the File menu

## Ball Field Map (Key Offsets)

| Offset | Name | Type/Value | Description |
|--------|------|------------|-------------|
| 0x000 | vtable | void* | 0x4CF3A0 (Ball vtable) |
| 0x008 | collision_result | int | Collision system result |
| 0x00C | string_timer | int | Countdown (200=show), frees string at 0xC28 |
| 0x010 | app_state | void* | App state ptr (*(this+0x878)) |
| 0x014 | scene | void* | Scene ptr (param_1 in ctor) |
| 0x018 | player_index | int | Player index, -1=AI |
| 0x01C | render_callback | void* | vtable for render/audio |
| 0x108 | timer | int | Timer_Init target |
| 0x1B8 | render_ctx_1 | int | RenderContext ptr 1 |
| 0x1C8 | render_alpha | float | 0.75f alpha |
| 0x208 | render_ctx_2 | int | RenderContext ptr 2 |
| 0x20C | color_a | float | RGBA alpha (1.0f) |
| 0x254 | uses_alpha | bool | (alpha != 1.0f) |
| 0x264 | rumble_timer1 | RumbleBoard[0x14] | Embedded rumble timer 1 |
| 0x278 | gravity_scale | float | 0.1f (0x3DCCCCCD) |
| 0x281 | is_falling | bool | Set 1 in ctor |
| 0x284 | radius | float | 27.0f (0x41D80000) |
| 0x290 | rumble_timer2 | RumbleBoard[0x14] | Embedded rumble timer 2 |
| 0x2A4 | spin_timer | int | Speed modifier reset timer |
| 0x2A8 | speed_modifier | float | 1.0f, decays |
| 0x2B8 | accel_x/y/z | float[3] | Accumulated acceleration |
| 0x2DC | checkpoint | Vec3 | Last collision/bump position |
| 0x2E8 | event_flag | bool | Checkpoint hit event |
| 0x2E9 | on_ramp | bool | On slope/ramp flag |
| 0x2FC | timer_bf | float | Timer decreased by constant |
| 0xC28 | display_string | char* | Timer display string |
| 0xC3C | teleport_active | bool | Teleport flag |
| 0xC40 | teleport_dest | Vec3 | Teleport destination |
| 0xC88 | matrix | float[16] | 4x4 transform matrix |
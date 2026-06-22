# Hamsterball DLL Mods

This folder contains every finished DLL mod for the original Hamsterball.exe.
Each subfolder has the C source, compiled DLL, and (where available) a zip package.

## Catalog

| # | Mod | Description | Proxy Type |
|---|-----|-------------|------------|
| 1 | [jump_mod](jump_mod/) | Press SPACE to jump (Player 1 only, raycast ground check, Phase 15 hook) | bass.dll |
| 2 | [half_size_p1](half_size_p1/) | Halves Player 1's ball size only | bass.dll |
| 3 | [half_size_all](half_size_all/) | Halves ALL balls' size | bass.dll |
| 4 | [player_clones](player_clones/) | Spawns AI clone balls (P1-P4) that chase all entities everywhere (v13 CE script) | CE script |
| 5 | [ai_8ball_fix](ai_8ball_fix/) | Fixes 8-ball AI so it moves in races | bass.dll |
| 6 | [unlimited_tris](unlimited_tris/) | Removes triangle count limit on custom levels | bass.dll |
| 7 | [water_mod](water_mod/) | Water visual effect mod | bass.dll |
| 8 | [fps_unlock_v9](fps_unlock_v9/) | Uncaps FPS (v9, QPC timer + vsync disable) | bass.dll |
| 9 | [fps_mod_custom](fps_mod_custom/) | Custom FPS mod (writes App+0x16C/0x170) | bass.dll |
| 10 | [fps_unlock_standalone](fps_unlock_standalone/) | Standalone FPS unlock DLL (injector) | standalone |
| 11 | [collision_hook](collision_hook/) | Hooks collision detection for debugging | standalone |
| 12 | [8ball_hit_detect](8ball_hit_detect/) | Detects player→8-ball collisions + 1.5x knockback | bass.dll |
| 13 | [entity-limit-fixer](entity-limit-fixer/) | Prevents freeze+crash with many entities (v5: skips Mesh_FindClosestCollision + throttles respawn + AI loops) | bass.dll |
| 14 | [direction_detect](direction_detect/) | Detects ball facing direction, shows compass + heading angle on-screen (press D to toggle) | bass.dll |


## Installation (bass.dll proxy mods)

All mods marked `bass.dll` follow the same installation procedure:

1. **Rename** the original `bass.dll` to `bass_real.dll` in your Hamsterball game folder
2. **Copy** the mod's compiled DLL into the game folder, renamed to `bass.dll`
3. **Launch** Hamsterball — the mod activates automatically

To uninstall: delete the mod `bass.dll` and rename `bass_real.dll` back to `bass.dll`.

## Installation (standalone DLLs)

Standalone mods require a DLL injector. Load the DLL into the `Hamsterball.exe`
process after launch.

## Build

All mods are cross-compiled with MinGW on Linux:

```bash
i686-w64-mingw32-gcc -shared -o bass.dll mod_source.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Key Struct Offsets (reference)

### Ball struct
| Offset | Type | Field |
|--------|------|-------|
| 0x000 | DWORD* | vtable pointer (0x4CF3A0 for player, 0x4CF540 for 8-ball) |
| 0x010 | void* | App pointer |
| 0x014 | void* | Scene/Board pointer |
| 0x018 | int | player_index (-1=NPC, 0=Player 1) |
| 0x164 | float | display X |
| 0x168 | float | display Y |
| 0x16C | float | display Z |
| 0x170 | float | velocity X |
| 0x174 | float | velocity Y (up = positive) |
| 0x178 | float | velocity Z |
| 0x188 | float | max_speed |
| 0x281 | byte | is_falling (1=falling off level) |
| 0x284 | float | radius (default 26.0) |
| 0x748 | int | gravity_plane_axis (0=X, 1=Y, 2=Z) |
| 0xC4C | int | fall_mode (0=normal, 1=fall-off-level) |

### Scene/Board struct
| Offset | Type | Field |
|--------|------|-------|
| 0x29D4 | AthenaList | bad_balls_list (8-balls + AI balls) |
| 0x2DEC | AthenaList | all_balls_list (all balls including players) |

### AthenaList struct
| Offset | Type | Field |
|--------|------|-------|
| 0x004 | int | element count |
| 0x40C | int* | heap-allocated array of pointers |

### App global
| Address | Type | Field |
|---------|------|-------|
| 0x005341E0 | App* | g_App global pointer |

### Key function addresses
| Address | Name |
|---------|------|
| 0x00405E00 | Ball_Update (player physics tick) |
| 0x00408390 | 8-ball AI tick |
| 0x0041B540 | Scene_UpdateBallsAndState |
| 0x00402200 | Ball_StartFall |
| 0x00402270 | Ball_EndFall |
| 0x0046EC30 | Ball_GetInputForce |
| 0x0046E0B0 | Input_IsKeyDown |

# Entity Limit Fixer v7

Prevents freezes and crashes when spawning many entities (8-balls, player clones) in Hamsterball arenas.

## What it does

When ball count exceeds `MAX_BALLS` (default 5), all `Mesh_FindClosestCollision` calls are skipped — returning "no collision" (99999.0f) instead of building an expensive SpatialTree from level geometry each call.

## Root cause

`Mesh_FindClosestCollision` (0x465D90) builds a full `SpatialTree` + `CollisionMesh` from level geometry on **every call**, traverses it, then frees everything. Each call takes ~1ms. Called from **5 call sites**:

1. `Ball_Update` (0x40651F): 2× per ball per frame
2. `Ball_Update` (0x407557): 2× per ball per frame  
3. `Ball_FindClosestRespawnPoint` (0x405C46): ~16× per fallen ball per frame
4. `Scene_UpdateArenaPhysics` (0x4406EE): 1× per ball per frame
5. `Ball_FindMeshCollision` (0x4039CD): 1× per BounceBall per frame

With 10 balls: ~50+ SpatialTree builds per frame → 50ms+ → freeze.

## v7 approach

Instead of patching each call site individually (v5/v6 only covered 3 of 5), v7 patches `Mesh_FindClosestCollision`'s **entry point** directly. A global flag `SKIP_COLLISIONS` is set once per frame at `Scene_UpdateBallsAndState` entry when ball count > `MAX_BALLS`. When the flag is set, the function returns immediately with 99999.0f — covering all 5 call sites with a single patch.

| Patch | Address | Description |
|-------|---------|-------------|
| A | 0x41B540 | Hook `Scene_UpdateBallsAndState` → set `SKIP_COLLISIONS` flag |
| B | 0x465D90 | Hook `Mesh_FindClosestCollision` → skip if flag set |
| C | 0x4BA58D | `operator_new` → return NULL (crash protection) |
| D | 0x4083D9 | Skip AI O(N²) loop 1 when flag set |
| E | 0x408548 | Skip AI O(N²) loop 2 when flag set |

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll bass_proxy.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Adjustable parameters (CE address list)

- `MAX_BALLS` (default 5): Ball count threshold for skipping collision
- `SKIP_COLLISIONS`: Runtime flag (1=skip, 0=normal), set automatically

## Behavior

- **≤5 balls**: Full collision detection, game runs 100% normally
- **>5 balls**: All `Mesh_FindClosestCollision` calls skip the expensive SpatialTree build. Balls won't collide with level geometry but still bounce off each other (ball-ball collision is a separate system). Balls that fall still respawn normally.

## Files

- `EntityLimitFixer.CEA` — CE AutoAssembler script
- `bass_proxy.c` — DLL proxy source
- `bass.dll` — Compiled DLL proxy

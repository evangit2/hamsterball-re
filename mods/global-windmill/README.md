# Global Windmill Mod

Spawns Tower Race windmills on any level with a hotkey. Creates BOTH a visual mesh AND a collision level — the ball gets spun (N:SWIRL) when it touches the windmill.

## Usage

1. Load `GlobalWindmill.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnWindmill` to 1 in CE (or use a hotkey)
4. Windmill spawns at player 1's position (both visual + collision)

## How It Works

This mod creates **two objects** per spawn:

### Part A: Visual Mesh (for rendering)
- `operator_new(0x10D0)` → `Stands_ctor` (0x462850) with windmill mesh
- Sets vtable to 0x4D8FB0 (renderable mesh)
- Position at `obj+0x10D4/+0x10D8/+0x10DC`
- Registered to `board+0x2578` (general list) + `board+0xCD4` + scene tree

### Part B: CollisionLevel (for N:SWIRL ball spinning)
- `operator_new(0x10D0)` → `CollisionLevel_ctorWithLevel` (0x465080)
- `Level_LoadMeshes` (0x465200) loads collision data
- Position at `coll+0x10D8/+0x10DC/+0x10E0`
- Registered to `board+0x2578` + `board+0xCD4` + scene tree
- Registered with scene manager via `SceneObject_SetupCallback(0x4F7360, 0x168, 0)`
- N:SWIRL events baked into mesh → fire automatically when ball touches

## Mesh Loading

- On first spawn: loads `Levels\Level4-Windmill` mesh via `MeshWorld_ctor`
- Stores at `board+0x437C` (same slot as Tower ctor)
- On Tower Race, mesh is already loaded — reuses it

## How N:SWIRL Works

The windmill mesh's collision triangles have `N:SWIRL` event names embedded in them. When the ball touches them:

```c
Rotator_AddBall(board, ball);  // 0x43B6F0
```

This applies a rotation matrix to the ball's position and velocity, spinning it around the windmill center. A 10-frame tick counter resets each frame the ball stays on the rotator.

## Object Details

| Component | Alloc | Constructor | Vtable |
|-----------|-------|-------------|--------|
| Visual | 0x10D0 | Stands_ctor (0x462850) | 0x4D8FB0 |
| Collision | 0x10D0 | CollisionLevel_ctorWithLevel (0x465080) | N/A (collision-only) |

## Full Analysis

See `docs/WINDMILL_JUDGE_SYSTEM.md` for complete reverse-engineering documentation.

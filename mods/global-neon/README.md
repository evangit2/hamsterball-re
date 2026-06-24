# Global Neon Objects Mod

Spawns all 6 Neon Race objects on any level with a hotkey.

## Objects Spawned

1. **NEONPLATFORM** — Disappearing neon floor (alloc 0x10EC, ctor 0x43E110)
2. **DFLOOR1** — Disappearing floor 1 (alloc 0x1104, ctor 0x43E450)
3. **DFLOOR2** — Disappearing floor 2
4. **DFLOOR3** — Disappearing floor 3
5. **DFLOOR4** — Disappearing floor 4
6. **TRODE** — Neon electrode/tube (alloc 0x1104, ctor 0x43E450)

## Usage

1. Load `GlobalNeon.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnNeon` to 1 in CE (or use a hotkey to set it)
4. All 6 objects spawn at the player's position, spread out in a line

## Mesh Loading

All 6 meshes are loaded on first spawn (cached):
- `Levels\LevelDark-NeonPlatform` → mesh_neonplatform
- `Levels\LevelDark-DFloor1` → mesh_dfloor1
- `Levels\LevelDark-DFloor2` → mesh_dfloor2
- `Levels\LevelDark-DFloor3` → mesh_dfloor3
- `Levels\LevelDark-DFloor4` → mesh_dfloor4
- `Levels\LevelDark-Trode` → mesh_trode

## Collision

Each object creates its own CollisionLevel sub-object internally via `CollisionLevel_ctorWithLevel` (0x465080). The ball will physically collide with the geometry.

**Note:** Neon-specific collision events (lights on/off, zoop effect, platform activation) are handled by `TowerCollisionEvents` at 0x416CA0 (vtable[0x74] of Neon Board). On non-Neon levels, these events will NOT fire. Objects have physical collision but no special behaviors.

## Object Placement

- NEONPLATFORM: at player position (X, Y, Z)
- DFLOOR1: Y+20 (above player)
- DFLOOR2: X+20
- DFLOOR3: X+40
- DFLOOR4: X+60
- TRODE: Z+20

## Hook Point

- Address: 0x405E22 (inside Ball_Update)
- Original: `mov eax, [esi+0x0C5C]` (6 bytes: 8B 86 5C 0C 00 00)
- esi = ball pointer at this point in Ball_Update

# Global Drawbridge Mod

Spawns Tower Race drawbridges on any level with a hotkey.

## Usage

1. Load `GlobalDrawbridge.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnDrawbridge` to 1 in CE (or use a hotkey on that address)
4. The drawbridge spawns at the player's position (slightly above)

## How It Works

- Hooks at `Ball_Update` (0x405E22) — same as Global Lifters
- On first activation: loads mesh `Levels\Level4-Drawbridge` via `MeshWorld_ctor` → stores at Board+0x4370
- Initializes `AthenaList` at Board+0x4BE8 (drawbridge list) if not already initialized
- Allocates 0x113C bytes, calls `Glass_Level_ctor(alloc, Board, Board+0x4370)` @ 0x4384A0
- Sets position at obj+0x10D8/+0x10DC/+0x10E0
- Appends to Board+0x2578 (general objects) and Board+0x4BE8 (drawbridge list)

## Mesh Dependency

- `Levels\Level4-Drawbridge` (at VA 0x4D099C) — loaded on first spawn, cached in Board+0x4370
- On Tower Race, this mesh is already loaded by `BoardLevel5_Tower_ctor`

## Collision

- Drawbridge has a CollisionLevel sub-object at obj+0x10D4 (created by constructor)
- Collision works through the generic spatial tree collision system
- No dedicated handler in Level_HandleCollision

## Addresses

| Symbol | Address |
|--------|---------|
| Glass_Level_ctor | 0x4384A0 |
| MeshWorld_ctor | 0x461510 |
| operator_new | 0x4BA57B |
| AthenaList_Init | 0x453210 |
| AthenaList_Append | 0x453810 |
| "Levels\Level4-Drawbridge" | 0x4D099C |
| Board+0x4370 (mesh) | — |
| Board+0x4BE8 (drawbridge list) | — |

# Global Trapdoor Mod

Spawns Tower Race trapdoors on any level with a hotkey.

## Usage

1. Load `GlobalTrapdoor.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnTrapdoor` to 1 in CE (or use a hotkey on that address)
4. The trapdoor spawns at the player's position (slightly above)

## How It Works

- Hooks at `Ball_Update` (0x405E22) — same as Global Lifters
- No mesh loading needed — trapdoor meshes are globally pre-loaded at App level
- Initializes `AthenaList` at Board+0x47D0 (trapdoor list) if not already initialized
- Allocates 0x10F8 bytes, calls `GlassStands_Ctor(alloc, Board)` @ 0x438290
- Sets position at obj+0x10E0/+0x10E4/+0x10E8
- Appends to:
  - Board+0x2578 (general objects list)
  - Board+0x47D0 (trapdoor list — for N:TRAPDOOR collision)
  - Board+0xCD4 (sub-object 1: Stands/Trapdoor2 mesh)
  - Board+0x10EC (sub-object 2: TipperVisual/Trapdoor2 collision)
  - Board+0x8AC→+0x480→+0x1C (primary collision level)
  - Board+0x8B0→+0x18 (secondary collision level)

## Mesh Dependency

**NONE!** All trapdoor meshes are globally pre-loaded by the resource loader (0x4298C0):
- App+0x594 = MeshWorld("Levels\Level4-Trapdoor1")
- App+0x598 = MeshWorld("Levels\Level4-Trapdoor2")
- App+0x59C = CollisionLevel(App+0x594)
- App+0x5A0 = CollisionLevel(App+0x598)

## Collision

Handled in `TowerCollisionEvents` (0x40DCD0):
- **"N:TRAPDOOR"** → `Trapdoor_Activate(obj)` @ 0x438410 — sets obj+0x10F4=10, plays sound
- **"E:OPENSESAME"** → `Trapdoor_Open(obj)` @ 0x4344D0 — opens first trapdoor in drawbridge list

Sub-objects at obj+0x10D8 and obj+0x10DC are appended to Board collision level lists
for physical ball-vs-trapdoor collision.

## Addresses

| Symbol | Address |
|--------|---------|
| GlassStands_Ctor | 0x438290 |
| operator_new | 0x4BA57B |
| AthenaList_Init | 0x453210 |
| AthenaList_Append | 0x453810 |
| App+0x594 (Trapdoor1 mesh) | global |
| App+0x598 (Trapdoor2 mesh) | global |
| App+0x59C (Trapdoor1 collision) | global |
| App+0x5A0 (Trapdoor2 collision) | global |
| Board+0x47D0 (trapdoor list) | — |

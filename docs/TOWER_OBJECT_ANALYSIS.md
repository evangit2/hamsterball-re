# Hamsterball Tower & Dizzy Object System — Complete Reverse Engineering

## Factory Functions

### CreateDizzyObjects @ 0x0040A5F0
Dizzy Race level-specific factory. Dispatches via `__strnicmp` on object names from MESHWORLD data.

| Object | Match String | VA | Alloc | Constructor | Board Mesh | AthenaList | Difficulty |
|--------|-------------|-----|-------|-------------|------------|------------|------------|
| TIPPER | "TIPPER", 6 | 0x4CF69C | 0x1104 | Tipper_ctor | +0x4394 | +0x2578 | YES |
| WATERWHEEL | "WATERWHEEL", 10 | 0x4CF690 | NONE | NONE (Board fields) | +0x4BA8/+0x4BAC | NO | NO |
| SWIRL | "SWIRL", 5 | 0x4CF688 | NONE | NONE (Board fields) | +0x4BC4/+0x4BC8 | NO | NO |
| GLUEBIE | "GLUEBIE", 7 | 0x4CF680 | 0x110C | Gluebie_ctor | +0x4374 | +0x2578/+0x6080 | YES |

### CreateTowerObjects @ 0x0040D7C0
Tower Race level-specific factory. Handles 7 object types.

| Object | Match String | VA | Alloc | Constructor | Board Mesh | AthenaList | Difficulty |
|--------|-------------|-----|-------|-------------|------------|------------|------------|
| CATAPULT | "CATAPULT", 8 | 0x4CF99C | 0x1108 | Catapult_ctor @ 0x437E10 | +0x436C | +0x2578/+0x584C | NO |
| MACE | "MACE", 4 | 0x4CF994 | 0x110C | 0x438750 | +0x4378 | +0x2578 | NO |
| DRAWBRIDGE | "DRAWBRIDGE", 10 | 0x4CF988 | 0x113C | Glass_Level_ctor @ 0x4384A0 | +0x4370 | +0x2578 (×2) | NO |
| WINDMILL | "WINDMILL", 8 | 0x4CF97C | 0x10D0 | CollisionLevel_ctorWithLevel @ 0x465080 | +0x437C | NO | NO |
| TRAPDOOR | "TRAPDOOR", 8 | 0x4CF970 | 0x10F8 | 0x438290 | N/A | +0x2578 (×6) | NO |
| CHOMPER | "CHOMPER", 7 | 0x4CF968 | NONE | NONE (Board fields) | +0x4390 | NO | NO |
| TURRET | "TURRET", 6 | 0x4CF960 | 0x10D0 | Stands_ctor @ 0x462850 | +0x43B4 | NO (vtable) | NO |

## Mesh Path Strings

| VA | String | Board Offset | Used By |
|-----|--------|-------------|---------|
| 0x4D0794 | Levels\Level3-WaterWheel | +0x4BA8 | Dizzy WATERWHEEL |
| 0x4D099C | Levels\Level4-Drawbridge | +0x4370 | Tower DRAWBRIDGE |
| 0x4D0974 | Levels\Level4-Mace | +0x4378 | Tower MACE |
| 0x4D095C | Levels\Level4-Windmill | +0x437C | Tower WINDMILL |
| 0x4D094C | Meshes\Chomper | +0x4390 | Tower CHOMPER |
| 0x4D0932 | Levels\Level4-Turret | +0x43B4 | Tower TURRET |

## MESHWORLD Placement

| File | Objects Found |
|------|--------------|
| Level3-WaterWheel.MESHWORLD | WATERWHEEL |
| Level4.MESHWORLD | CHOMPER, DRAWBRIDGE, WINDMILL, MACE, TURRET |
| Level4-Trapdoor1.MESHWORLD | TRAPDOOR |
| Level4-Trapdoor2.MESHWORLD | TRAPDOOR |
| Arena-Tower.MESHWORLD | MACE, TURRET |
| Level9.MESHWORLD | TRAPDOOR |

## Spawnability Verdict

| Object | Spawn? | Reason |
|--------|--------|--------|
| WaterWheel | NO | No alloc, no ctor, no vtable — just Board field write. Static mesh only. |
| Chomper | NO | No alloc, no ctor — MeshNode visual only. No collision or game logic. |
| Drawbridge | YES | Alloc 0x113C, Glass_Level_ctor, appended to +0x2578. Needs Board+0x4370 mesh. |
| Mace | YES | Alloc 0x110C, ctor 0x438750, appended to +0x2578. Needs Board+0x4378 mesh. |
| Windmill | PARTIAL | CollisionLevel only (collision, no game object/render). Not in +0x2578. |
| Trapdoor | YES | Alloc 0x10F8, ctor 0x438290, appended to +0x2578. No mesh dependency. |
| Turret | PARTIAL | Stands_ctor but no AthenaList_Append. Uses vtable dispatch for rendering. |

## Key Functions

- MeshWorld_ctor: 0x00461510 — alloc(0x10D0, App+0x174, "path")
- CollisionLevel_ctorWithLevel: 0x00465080
- Stands_ctor: 0x462850
- AthenaList_Append: 0x00453810
- operator_new: 0x4BA57B (jmp to malloc)
- __strnicmp: 0x4C7677

## Global Spawn Pattern (for functional objects)

For objects that CAN be spawned (Drawbridge, Mace, Trapdoor):

1. Get Board pointer: ball+0x14 → Board
2. Get App pointer: Board+0x878 → App
3. Load required mesh: MeshWorld_ctor(0x10D0, App+0x174, meshPath) → store at Board+offset
4. Allocate object: operator_new(size)
5. Call constructor: Ctor(alloc, Board, mesh) — thiscall (ecx=alloc, push Board+mesh)
6. Set position: obj+0x10D4/+0x10D8/+0x10DC = param_block pos
7. Append to list: AthenaList_Append(Board+0x2578, obj)

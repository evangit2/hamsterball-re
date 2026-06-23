# Global Bonk (Hammer) Mod

Spawns Bonk the Hammer from Expert Race on any level with a hotkey.

## Object Details

- **Name string:** "BONK" (0x4CFA4C)
- **Alloc size:** 0x1200 (4608 bytes)
- **Constructor:** 0x438850 (ret 0x10 — 4 stack params)
- **Vtable:** 0x4D5120
- **Mesh:** "levels\level5-bonk" (0x4D5C10) — loaded INTERNALLY by the constructor

## Usage

1. Load `GlobalBonk.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnBonk` to 1 in CE (or use a hotkey to set it)
4. Bonk spawns at the player's position

## How It Works

### Constructor Flow (0x438850):
1. Reads Board+0x878 → App → App+0x174 (D3D device)
2. Calls MeshWorld_ctor(0x461510) with D3D device + "levels\level5-bonk"
   → loads Level5-Bonk.MESHWORLD mesh
3. Sets vtable = 0x4D5120
4. Stores Board at obj+0x10D0
5. Copies position → obj+0x10D4 (XYZ), obj+0x10E0 (XYZ copy)
6. Adjusts Y by ±10.0 (float at 0x4CF9F8)
7. Creates CollisionLevel: operator_new(0x10D0) → CollisionLevel_ctorWithLevel(0x465080) → obj+0x10F8
8. Copies collision data via Level_LoadMeshes(0x465200)
9. Sets initial state:
   - obj+0x10FC = 1 (active flag)
   - obj+0x10FD = 0 (not chasing)
   - obj+0x1100 = 1000 (0x3E8 — timer/state)
   - obj+0x1138 = 0
10. Looks up 6 named sub-meshes from the mesh via 0x4605E0:
    - "HAMMERSAFESMACK" → obj+0x10EC (safe zone position)
    - "IMPACT" → obj+0x112C (impact position)
    - "HAMMERAREA1" → obj+0x1140/1144 (patrol area 1)
    - "HAMMERAREA2" → obj+0x1160/1164 (patrol area 2)
    - "HAMMERAREA3" → obj+0x1180/1184 (patrol area 3)
    - "HAMMERAREA4" → obj+0x11A0/11A4 (patrol area 4)
    - (Lookups for AREA1-4 go through Board+0x8AC collision level)

### Factory Post-Processing:
- Appends to Board+0x2578 (general list — always available)
- Stores at Board+0x436C (for E:CALLHAMMER event)

### Collision Events:
- **E:CALLHAMMER** (0x438B30): If active (obj+0x10FC), deactivates, plays Sound3D at position, calls Board vtable[0x88] with "BONKPOPUP"
- **E:HAMMERCHASE** (0x438BB0): If not chasing, sets obj+0x10FD=1, obj+0x1104=1, copies start/target positions, sets speed=0.5

### Update Function (vtable[0x2C] = 0x43F930):
- Reads obj+0x10FC (active) and obj+0x10FD (chasing)
- If not chasing: manages timer at obj+0x1100 (min 150, reset to 0)
- If chasing: executes chase AI based on obj+0x1104 (chase state)

## Key Object Fields

| Offset | Size | Description |
|--------|------|-------------|
| +0x10D0 | 4 | Board pointer |
| +0x10D4 | 12 | Position XYZ (float) |
| +0x10E0 | 12 | Position copy XYZ (float) |
| +0x10EC | 12 | HAMMERSAFESMACK position |
| +0x10F8 | 4 | CollisionLevel pointer |
| +0x10FC | 1 | Active flag (1=active) |
| +0x10FD | 1 | Chasing flag (1=chasing) |
| +0x1100 | 4 | Timer/state (init 1000) |
| +0x1104 | 4 | Chase state |
| +0x1108 | 12 | Chase start position |
| +0x1120 | 12 | Chase target position |
| +0x112C | 12 | IMPACT position |
| +0x1138 | 4 | Chase speed (0.5) |
| +0x1140-11E4 | ~168 | Patrol area positions (AREA1-4) |

## No External Dependencies

- Mesh loaded internally by constructor (no Board+0x43xx needed)
- CollisionLevel created internally
- Only appends to Board+0x2578 (always init'd)
- Board+0x436C is a simple store (no list init needed)

## Hook Point

- Address: 0x405E22 (inside Ball_Update)
- Original: `mov eax, [esi+0x0C5C]` (6 bytes: 8B 86 5C 0C 00 00)

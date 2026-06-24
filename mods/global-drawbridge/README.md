# Global Drawbridge Mod

Spawns Tower Race drawbridges on any level with a hotkey.

## v2 Changes (June 2026)

**Fixed:** Now uses the correct constructor `Spinner_Level_ctor` (0x4396F0) instead of `Glass_Level_ctor`.
The old version used the Glass level vtable, which had incorrect update/render behavior.
The Spinner vtable (0x4D51E0) is what the original game uses for spinning drawbridge platforms.

**Added:** JIT mesh injection pattern — saves/restores `board+0x4378` around the constructor call,
since `Spinner_Level_ctor` reads the mesh from `board+0x4378` internally via `Stands_ctor`.

**Added:** Per-frame vtable[11] call for proper spinning animation.

## Usage

1. Load `GlobalDrawbridge.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnDrawbridge` to 1 in CE (or use a hotkey)
4. Drawbridge spawns at player 1's position

## How It Works

- Hooks at `Ball_Update` (0x405E22)
- On first spawn: loads `Levels\Level4-Drawbridge` mesh via `MeshWorld_ctor` → stores at `board+0x4370`
- JIT mesh injection: temporarily writes drawbridge mesh to `board+0x4378` (read by `Stands_ctor`)
- Calls `Spinner_Level_ctor` (0x4396F0) with position + rotation=0
- Restores `board+0x4378` after construction
- Per-frame: calls vtable[11] (Spinner_Update @ 0x439870) for spin animation
- Registered to `board+0x2578` (general list) and `board+0xCD4`

## Object Details

- **Alloc size:** 0x10FC (4348 bytes)
- **Constructor:** 0x4396F0 (Spinner_Level_ctor, thiscall, params: board, x, y, z, rotation)
- **Vtable:** 0x4D51E0 (Spinner/Intermediate vtable)
- **Mesh:** "Levels\Level4-Drawbridge" (at VA 0x4D099C)
- **Update:** vtable[11] @ 0x439870
- **Render:** vtable[18] @ 0x45E0E0

## Key Fields

| Offset | Description |
|--------|-------------|
| +0x10D0 | Board pointer |
| +0x10D4/+0x10D8/+0x10DC | Position XYZ |
| +0x10E0 | Rotation angle |
| +0x10E8 | Angular velocity |
| +0x10F0 | Timer (init 100) |
| +0x10F4 | CollisionLevel pointer |
| +0x10F8 | Direction multiplier (1.0 or -1.0) |

## Mesh Dependency

`Levels\Level4-Drawbridge` loaded on first spawn, cached at `board+0x4370`.
On Tower Race, this mesh is already loaded by `BoardLevel5_Tower_ctor`.

## Full Analysis

See `docs/SAW_DRAWBRIDGE_SYSTEM.md` for complete reverse-engineering documentation.

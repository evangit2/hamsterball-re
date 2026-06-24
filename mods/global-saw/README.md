# Global Sawblade Mod

Spawns Expert Race sawblades on any level with a hotkey.

## Usage

1. Load `GlobalSaw.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnSaw` to 1 in CE (or use a hotkey)
4. Saw spawns at player 1's position and auto-activates

## How It Works

- Hooks at `Ball_Update` (0x405E22)
- On spawn: allocates 0x111C bytes, calls `Sawblade_Level_Ctor` (0x434660)
- Constructor loads `Meshes\sawblade` mesh internally (no pre-loaded mesh needed)
- Auto-activates: sets +0x110D=0 (clear alert) and +0x1114=1 (activated)
- Direction: +0x10F8=1 (Z-axis movement)
- Per-frame: calls vtable[11] (Sawblade_Update @ 0x439BB0) for spin, debris, collision, movement
- Registered to board+0x2578 (general list) and board+0xCD4

## Object Details

- **Alloc size:** 0x111C (4380 bytes)
- **Constructor:** 0x434660 (Sawblade_Level_Ctor, thiscall, params: board, x, y, z)
- **Vtable:** 0x4D5240
- **Mesh:** "Meshes\sawblade" (loaded internally by Level_ctor)
- **Update:** vtable[11] @ 0x439BB0
- **Render:** vtable[18] @ 0x4347E0

## Key Fields

| Offset | Description |
|--------|-------------|
| +0x10D0 | Board pointer |
| +0x10D4/+0x10D8/+0x10DC | Current position XYZ |
| +0x10E0/+0x10E4/+0x10E8 | Home position XYZ |
| +0x10F0 | Rotation angle (RNG 0-360) |
| +0x10F4 | Spin speed (0→25.0) |
| +0x10F8 | Direction (1=Z-axis, 2=X-axis) |
| +0x10FC | Debris spawn counter |
| +0x110D | Alert flag |
| +0x1110 | Spin velocity (500.0 init, decays ×0.95) |
| +0x1114 | Activated flag (0=idle, 1=active) |
| +0x1118 | Movement velocity |

## No Difficulty Gate

The original factory checks `App+0x23C != 0` (Normal/Frenzied only).
This script bypasses that check — saws spawn on ALL difficulty levels.

## Full Analysis

See `docs/SAW_DRAWBRIDGE_SYSTEM.md` for complete reverse-engineering documentation.

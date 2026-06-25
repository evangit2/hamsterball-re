# Global Mace (Pendulum) Mod

Spawns Tower Race swinging maces on any level with a hotkey.

## Usage

1. Load `GlobalMace.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnMace` to 1 in CE (or use a hotkey on that address)
4. Mace spawns at player 1's position (slightly above for hang-down swing)
5. Each hotkey press spawns another mace (up to 16)

## How It Works

- Hooks at `Ball_Update` (0x405E22) — same pattern as SpeedCylinder/Bonk
- On first spawn: loads `Levels\Level4-Mace` mesh via `MeshWorld_ctor` → stores at `board+0x4378`
- On Tower Race, mesh is already loaded by `BoardLevel5_Tower_ctor` — reuses it
- Allocates 0x110C bytes, calls `CascadeStands_Ctor` (0x438750)
- Sets position at `obj+0x10D8` (X), `+0x10DC` (Y), `+0x10E0` (Z)
- Sets swing amplitude (+0x10E8 = 80.0), active flag (+0x10F4 = 1), timer (+0x10F8 = 50)
- Registered to `board+0x2578` (general), `board+0x5000` (mace list), `board+0xCD4`
- Per-frame: calls vtable[11] (Pendulum_Update @ 0x43F3C0) for swing animation

## Object Details

- **Alloc size:** 0x110C (4364 bytes)
- **Constructor:** CascadeStands_Ctor (0x438750) — thiscall(obj, board, mesh) ret 8
- **Vtable:** 0x4D50C0
- **Mesh:** "Levels\Level4-Mace" (VA 0x4D0974)
- **Update:** vtable[11] @ 0x43F3C0 — swing animation with sin/cos oscillation
- **Render:** vtable[18] @ 0x45E0E0 — shared base render

## Key Fields

| Offset | Type | Value | Description |
|--------|------|-------|-------------|
| +0x10D8 | float | player X | Current position X |
| +0x10DC | float | player Y + 20 | Current position Y (elevated) |
| +0x10E0 | float | player Z | Current position Z |
| +0x10E8 | float | 80.0 | Swing amplitude (radius) |
| +0x10F4 | int | 1 | Active flag |
| +0x10F8 | int | 50 | Timer |

## Collision

- **N:MACE** — ball touching mace gets bounced (calls ball->vtable[8])
- **E:MACETRIGGER** — activates mace swing (sets +0x10F0=1)
- Both handled in `Level_HandleCollision` (0x40DCD0) via `board+0x5000` mace list

## Notes

- Position offset is +0x10D8 (not +0x10D4 like most other objects)
- Constructor takes 2 params (board + mesh), unlike Bonk/Sawblade which take position params
- CascadeStands_Ctor internally calls Stands_ctor + allocates CollisionLevel
- Swing animation uses amplitude 80.0 and 0.05/frame angle increment (~71.5°/sec at 25fps)

## Full Analysis

See `docs/MACE_WINDMILL_CHOMPER_SYSTEM.md` for complete reverse-engineering documentation of Mace, Windmill, and Chomper systems.

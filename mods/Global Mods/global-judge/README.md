# Global Judge (Hammy Judge) Mod

Spawns Expert Race Hammy Judges on any level with a hotkey.

## Usage

1. Load `GlobalJudge.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnJudge` to 1 in CE (or use a hotkey)
4. Judge spawns at player 1's position and animates

## How It Works

- Hooks at `Ball_Update` (0x405E22)
- Allocates 0x1100 bytes, calls `Judge_Ctor` (0x43A150)
- Constructor internally calls `Level_ctor` which loads "meshes\hammyjudge" mesh
- Sets vtable to 0x4D52B8
- Sets position at `obj+0x10D4` (X), `+0x10D8` (Y), `+0x10DC` (Z)
- Registered to `board+0x4BBC` (judge list), `board+0x2578` (general), `board+0xCD4`
- Per-frame: calls vtable[11] (Judge_Update @ 0x434B60) for animation

## Object Details

- **Alloc size:** 0x1100 (4352 bytes)
- **Constructor:** Judge_Ctor (0x43A150) — thiscall(obj, board) ret 4
- **Vtable:** 0x4D52B8
- **Mesh:** "meshes\hammyjudge" (VA 0x4D0AA8) — loaded internally by Level_ctor
- **Update:** vtable[11] @ 0x434B60 — trig oscillation (same pattern as Pendulum)
- **Render:** vtable[18] @ 0x43A270
- **No JIT mesh injection needed** — Level_ctor loads the mesh internally

## Collision Events

| Event | Action |
|-------|--------|
| E:ALERTJUDGES | Activates all judges in board+0x4BBC list |
| E:BELL | Rings bell sound |
| E:SCORE | Awards points |

## Full Analysis

See `docs/WINDMILL_JUDGE_SYSTEM.md` for complete reverse-engineering documentation.

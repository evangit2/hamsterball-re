# Global Expert Objects Mod

Spawns all 6 Expert Race objects on any level with a hotkey.

## Objects Spawned

1. **BONK (Hammer)** — The hammer that chases you (ctor 0x438850, alloc 0x1200)
2. **FAN** — The fanstorm (ctor 0x438C20, alloc 0x1188)
3. **SAWBLADE** — Saw blade (ctor 0x434660, alloc 0x111C)
4. **BRIDGE** — Breakable bridge (ctor 0x4396F0, alloc 0x10FC)
5. **JUDGE** — Judge object (ctor 0x43A150, alloc 0x1100)
6. **BELL** — Bell to ring (ctor 0x434D70, alloc 0x10E8)

## Usage

1. Load `GlobalExpert.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnExpert` to 1 in CE (or use a hotkey to set it)
4. All 6 objects spawn at the player's position, spread out

## Mesh Loading

Only BRIDGE needs a mesh: `Levels\Level5-Bridge` loaded via MeshWorld_ctor (cached on first spawn).

All other objects (BONK, FAN, SAWBLADE, JUDGE, BELL) create their geometry internally via Stands_ctor(0x461740) using the D3D device — no external mesh needed.

## AthenaList Initialization

On non-Expert levels, three lists are initialized on first spawn:
- Board+0x4380 (bridge list 1)
- Board+0x4798 (bridge list 2)
- Board+0x4BBC (judge list)

Uses vtable check `cmp dword [ecx], 0x004D875C` to avoid double-init.

## Post-Spawn Board Field Writes

- BONK → Board+0x436C (for E:CALLHAMMER event)
- SAWBLADE → Board+0x4370 (for E:ALERTSAW1/ACTIVATESAW1 events)
- BELL → Board+0x4FD4 (for bell ring event)

## Collision Events

Expert Race collision handler at 0x40E6A0 handles:
- E:CALLHAMMER, E:HAMMERCHASE — activate hammer chase
- E:ALERTSAW1/2, E:ACTIVATESAW1/2 — activate saw blades
- E:ALERTJUDGES — cycle judges
- E:SCORE — scoring event
- E:JUMP — jump pad

On non-Expert levels, objects have physical collision but special events won't fire.

## Hook Point

- Address: 0x405E22 (inside Ball_Update)
- Original: `mov eax, [esi+0x0C5C]` (6 bytes: 8B 86 5C 0C 00 00)

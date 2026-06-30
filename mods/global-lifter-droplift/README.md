# Global Lifter (DropLift) Spawner

Spawns a Lifter at Player 1's position in any level. The lifter rises when the ball approaches — replicating the Up Race lifter with E:DROPLIFT behavior globally.

## Files
- `GlobalLifterDropLift.CEA` — CE AutoAssembler script (pure CEA, no Lua)

## Installation
1. Copy to your Cheat Engine scripts folder
2. In CE: File → Load → select `GlobalLifterDropLift.CEA`
3. Enable the script

## Usage
1. Add the address `SpawnLifter` to your CE address list
2. Enter a race or arena (any level works)
3. Set `SpawnLifter` to `1` — a Lifter spawns at Player 1's current position
4. The flag auto-resets to `0` after spawning
5. Set to `1` again to spawn another

The lifter automatically rises when the ball gets within 100.0 units.

## How It Works

### Lifter Creation Chain

The original game creates lifters through this chain:
1. Board constructor loads `Levels\Level6-Lifter` mesh into `App+0x5C8` (global lifter mesh slot)
2. `CreateLevelObjects` (0x4121D0) dispatches to `TryCreateLifter` (0x40EC40)
3. `TryCreateLifter` checks if object name == `"LIFTER"` (strnicmp, 6 chars)
4. If match: `operator_new(0x10FC)` → `Lifter_ctor(0x434E60)` → register in `board+0x2578`

### Lifter_ctor (0x434E60)

`__thiscall: ECX=this, push board, push x, push y, push z (ret 0x10)`

1. Gets lifter mesh from `board→[board+0x878]→[App+0x5C8]`
2. Calls `SceneObject_ctor(0x462850)` — clones SpatialTree from the lifter mesh for rendering
3. Sets vtable = `0x4D5390` (Lifter vtable)
4. Copies position to `+0x10D8` (x, y, z as 3 floats)
5. Creates `CollisionLevel` (0x10D0 bytes) via `CollisionLevel_ctorWithLevel(0x465080)`
6. Stores collision mesh at `+0x10D4`
7. Sets state `+0x10E4 = 3` (WAITING)
8. Sets `+0x10F0 = -1` (no current collision target)

### State Machine (vtable[0x2C] = 0x434F60)

| State | Name | Behavior |
|-------|------|----------|
| 3 | WAITING | Initial state, lifter at bottom, doing nothing |
| 0 | RISING | Height increases (`+0x10E8` accumulates to 95.0 max) |
| 1 | DESCENDING | Height decreases back to 0, plays sound every 20 frames |
| 2 | DROP | Quick descent (not used in normal operation) |

Key fields:
- `+0x10D8/+0x10DC/+0x10E0` — position (x, y, z)
- `+0x10E4` — state (0-3)
- `+0x10E8` — current height offset (0=bottom, 95.0=top)
- `+0x10EC` — sound timer (counts down from 20)
- `+0x10F0` — last height (for collision detection)
- `+0x10F4` — descent/wait timer
- `+0x10F8` — rise speed accumulator

### DropLift Trigger

When the ball touches an `E:DROPLIFT` collision surface in the level mesh:
1. Collision handler at `0x0040F06C` checks for `"E:DROPLIFT"` event
2. Gets lifter pointer from `board+0x436C`
3. Calls `Lifter_TriggerDrop(0x435170)` — `__thiscall: ECX=lifter`
4. `Lifter_TriggerDrop` only fires if state == 3 (WAITING)
5. Sets state = 0 (RISING), rise_speed = 0.25
6. Plays sound at lifter position via `Sound_Play3D`

### The NOP Patch (Hamsterball.exe+EC74)

The user's existing patch at `0x0040EC74` NOPs a `JNE` instruction in `TryCreateLifter`:
- Original: `JNE 0x0040ED01` (skip if name != "LIFTER")
- Patched: `NOP × 6` (always create lifter, regardless of name)

This makes the Lifter factory accept ANY mesh object name, causing lifters to be created for all objects in the Odd Race. The CEA script takes a different approach — it directly creates the lifter without going through the factory.

## Verified Addresses (Ghidra/r2, 2026-06-25)

| Address | Function | Convention |
|---------|----------|------------|
| 0x40EC40 | TryCreateLifter | ECX=board, stack=[name, out1, out2, pos], RET 0x10 |
| 0x434E60 | Lifter_ctor | ECX=this, stack=[board, x, y, z], RET 0x10 |
| 0x434F60 | Lifter_Update (vtable[0x2C]) | ECX=this |
| 0x435170 | Lifter_TriggerDrop | ECX=lifter |
| 0x462850 | SceneObject_ctor | ECX=this, push source_mesh |
| 0x465080 | CollisionLevel_ctorWithLevel | ECX=new, push source_mesh |
| 0x4BA57B | operator_new | push size, RET |
| 0x453810 | AthenaList_Append | ECX=list, push item, RET 0x4 |
| 0x461510 | MeshWorld_ctor | ECX=this, push d3d, push path, RET 0x8 |
| 0x459860 | Sound_Play3D | push pos_vec, push vol, ECX=sound_obj |
| 0x4D5390 | Lifter vtable | — |
| 0x4D3308 | "Levels\Level6-Lifter" string | — |
| 0x4CFB90 | "E:DROPLIFT" string | — |
| 0x4CFB1C | "LIFTER" string | — |

# Global Toob Race Object Spawner

Spawns any of the 10 Toob Race objects at Player 1's position in any level.

## Usage

1. Set `ObjectType` (1-10) to select which object to spawn:
   - 1 = SPINNY (rotating platform)
   - 2 = LOOPER (looping track)
   - 3 = GEAR (spinning gear)
   - 4 = BIGGEAR (big gear)
   - 5 = ROTATOR (generic rotator, uses Blockdawg1 mesh)
   - 6 = SPEEDCYLINDER (swinging pendulum)
   - 7 = LIFTER (lifter elevator)
   - 8 = TIMEBUTTON (timed button)
   - 9 = BLOCKDAWG1 (block dog enemy 1)
   - 10 = BLOCKDAWG2 (block dog enemy 2)
2. Set `SpawnObject` to 1 to spawn at player position
3. Objects update automatically via vtable[0x2C] each frame

## How It Works

The script bypasses the factory chain (CreateSpinny → CreateLevelObjects → etc.)
and calls constructors directly with our own MeshWorld pointer.

**Key advantage over Drawbridge/PopCylinder scripts**: No JIT board slot injection needed.
All Toob constructors take the mesh pointer as the **last stack parameter**, so we pass
our own MeshWorld directly without touching any board+0x43xx slots.

## Toob Race Board Mesh Slots

Set by `LevelBoard_Toob_ctor` (0x41F4B0):

| Offset | Mesh File | String Addr | Used By |
|--------|-----------|-------------|---------|
| board+0x436C | Levels\Level8-Spinny | 0x4D0E38 | LOOPER |
| board+0x4370 | Levels\Level8-Saw | 0x4D0E24 | GEAR |
| board+0x4374 | Levels\Level8-Fallout | 0x4D0E0C | BIGGEAR |
| board+0x4378 | Levels\Level8-Blockdawg1 | 0x4D0DF0 | ROTATOR |
| board+0x437C | Levels\Level8-Blockdawg2 | 0x4D0DD4 | BLOCKDAWG2 |

Additional slots set during LoadRaceData (not by board ctor):
- board+0x4784 = LIFTER mesh (runtime, unknown file)
- board+0x4788 = SPEEDCYLINDER mesh (runtime, unknown file)
- board+0x478C = TIMEBUTTON mesh (runtime, unknown file)
- board+0x47E0 = SPINNY mesh (runtime, unknown file)

For objects with unknown mesh files (SPEEDCYLINDER, LIFTER, TIMEBUTTON),
the script uses Level8-Spinny as a fallback mesh.

## Constructor Summary

| Object | Constructor | Addr | Alloc | RET | CollisionLevel Offset |
|--------|-------------|------|-------|-----|---------------------|
| SPINNY | Rotator_ctor | 0x435940 | 0x1508 | 0x14 | +0x10D4 |
| LIFTER | Rotator_ctor_sound | 0x436920 | 0x10F4 | 0x18 | +0x10E0 |
| SPEEDCYLINDER | Pendulum_ctor | 0x436A20 | 0x150C | 0x18 | +0x10E0 |
| TIMEBUTTON | Rotator_ctor_nosound | 0x436C10 | 0x10E8 | 0x14 | +0x10E0 |
| LOOPER | Looper_ctor | 0x435800 | 0x1500 | 0x14 | +0x10D4 |
| GEAR | Gear_ctor | 0x437590 | 0x1514 | 0x20 | +0x10D4 |
| BIGGEAR | Gear_ctor | 0x437590 | 0x1514 | 0x20 | +0x10D4 |
| ROTATOR | Rotator_ctor | 0x435940 | 0x1508 | 0x14 | +0x10D4 |
| BLOCKDAWG1 | Blockdawg_ctor | 0x43C310 | 0x1154 | 0x18 | +0x10D4 |
| BLOCKDAWG2 | Blockdawg_ctor | 0x43C310 | 0x1154 | 0x18 | +0x10D4 |

## Factory Chain (How the Game Creates Toob Objects)

```
LoadRaceData("TOOBRACE")
  → reads MESHWORLD ref objects
  → for each ref, calls factory chain:
    CreateBumper (0x40FA20)     → BUMPER
    CreateSpinny (0x4143D0)     → SPINNY (→ Rotator_ctor, mesh from board+0x47E0)
    CreateSpeedCylinder (0x4117B0) → LIFTER, SPEEDCYLINDER, TIMEBUTTON
    CreateMechanicalObjects (0x417FE0) → LOOPER, GEAR, BIGGEAR, ROTATOR
    CreateLevelObjects (0x4121D0) → BRIDGE, TIPPER, BONK, POPCYLINDER, BLOCKDAWG, CATAPULT, GLUEBIE
```

## Toob Race Collision Handler Events

The Toob collision handler at vtable[0x1D] (0x410020) handles:
- N:SPINNY, N:SAWTEETH, N:BUMPER — collision with rotating objects
- E:ALERTSAW2, E:BRANCH — event triggers
- PILLAR, MAGNIFYER, CLOUDSCAPE — special objects
- POPCYLINDER, TRAPDOOR — pop/door effects
- E:PEGS, E:TRAPPOP, E:NOPEGS — peg state machine
- E:HEATON, E:HEATOFF — heat/lava toggle
- E:LIMIT — boundary limit

## Object Registration

All objects are registered in:
- `board+0x2578` — active objects list (update + render)
- `board+0x10EC` — collision level list
- `board+0x8B0+0x18` — collision dispatch list
- `board+0x8AC→+0x480→+0x1C` — render list (post-alpha)

## Notes

- BLOCKDAWG objects need a DAWGPATH ref for movement. With pathId=0 they stay stationary.
- GEAR/BIGGEAR use dual-position params (X,Y,Z + X2,Y2,Z2). Script uses same position for both.
- SPEEDCYLINDER/LIFTER/TIMEBUTTON use fallback mesh (Level8-Spinny) since their actual
  mesh files are loaded at runtime via board+0x4784/0x4788/0x478C and the filenames
  are not known at compile time.

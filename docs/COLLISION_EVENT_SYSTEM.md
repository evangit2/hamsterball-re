# Hamsterball Collision Event System

## Architecture

The collision event system uses a **3-tier dispatcher chain**:

```
Ball Physics (Ball_Update 0x405190)
  └─ Per-object collision test
       └─ Event dispatch based on object type string
            ├─ Arena_HandleCollision (0x40E6A0)  ← Rumble arenas
            │    └─ Level_HandleCollision (0x40DCD0)  ← Race levels
            │         └─ GameObject_HandleCollision (0x40C5D0)  ← Base (ALL events)
            └─ Direct collision handler (some objects bypass chain)
```

Each handler processes its domain-specific events first, then calls the base
handler (`CreateNoDizzy`) for common events. This means ALL collision events
eventually reach `GameObject_HandleCollision`.

## Event Dispatch Table

### Tier 1: Arena_HandleCollision (0x40E6A0)
**Domain**: Rumble arena multiplayer events. Most events gated behind `App+0x23C` (multiplayer flag).

| Event | Effect | Gated |
|-------|--------|-------|
| E:CALLHAMMER | Spawn bonk popup | Yes |
| E:HAMMERCHASE | Start hammer chase | Yes |
| E:ALERTSAW1 | Saw blade 1 alert mode | Yes |
| E:ALERTSAW2 | Saw blade 2 alert mode | Yes |
| E:ACTIVATESAW1 | Saw blade 1 full active | Yes |
| E:ACTIVATESAW2 | Saw blade 2 full active | Yes |
| E:ALERTJUDGES | Reset all judge objects | No |
| E:SCORE(N) | Set score display to N | No |
| E:JUMP | Ball bounce + sound + 200pts | No |
| E:BELL | Extra time (+500) + popup | No |

After processing: **delegates to Level_HandleCollision**.

### Tier 2: Level_HandleCollision (0x40DCD0)
**Domain**: Race level mechanical objects.

| Event | Effect | Details |
|-------|--------|---------|
| E:CATAPULTBOTTOM | Launch catapult | Ball freeze=1000, catapult stores ball ref |
| E:OPENSESAME | Open all trapdoors | Opens first trapdoor in list |
| N:TRAPDOOR | Activate matching trapdoor | Matches by object ID or secondary ID |
| E:BITE | Set damage | Timer=25.0 (0x41C80000), clear counter |
| E:MACETRIGGER | Activate all maces | Sets each mace active flag at +0x10F0 |
| N:MACE | Ball bounces off mace | Only if mace swinging (not 0x42A00000) and not already hit |

After processing: **delegates to GameObject_HandleCollision**.

### Tier 3: GameObject_HandleCollision (0x40C5D0)
**Domain**: All game events (base handler, processes everything).

| Event | Effect | Key Details |
|-------|--------|-------------|
| N:SECRET | Mark rotator triggered | Secret found marker |
| N:UNLOCKSECRET | Check arena unlock | May unlock new arena |
| E:NODIZZY | Anti-dizzy zone | Parses `<TIME>N</TIME>` tags, records best time |
| E:SAFESWITCH | Copy switch data | Data in parentheses → ball+0xC2C |
| E:LIMIT | Arena boundary | Tracks completions per player (0-3) |
| E:BREAK | Ball bounce | Calls ball vtable[0x20] |
| E:JUMP | Ball jump | 3D sound, impact=10, force=0.025, +200pts |
| E:ACTION | Score award | `<ONCE>TRUE</ONCE>` prevents repeat, `<SCORE>N</SCORE>` with difficulty mod |
| E:TRAJECTORY | Set trajectory | Parses `<X>`, `<Y>`, `<Z>` XML tags |
| N:NOCONTROL | Disable input | 10 frame freeze |
| N:WATER | Water effect | Sets in-water flag + 10 frame timer |
| N:TARPIT | Tar slowdown | 3D sound, marks tar state, clears velocity |
| N:GOAL | Finish race | Plays "Goal!" music, sets finish flags, records time |
| N:MOUSETRAP | Ball deflect | Normalizes direction × speed (0x4CF370), finds rotator |
| DROPIN | Pipe drop-in | Sound + +200pts if speed > threshold |
| PIPEBONK | Pipe bonk | Random sound from 3, +100pts, 10 frame cooldown |
| POPOUT | Pipe pop-out | Sound + +100pts, 50 frame cooldown |

## Event Name Format

Event names are stored in collision objects at `object+0x864` (string pointer).
Two prefixes:
- **N:** = Named/physical object (something the ball interacts with physically)
- **E:** = Event/trigger (something that activates on ball contact)

Some events have XML-style tag parameters:
```
E:NODIZZY<TIME>500</TIME>
E:ACTION<ONCE>TRUE</ONCE><SCORE>500</SCORE>
E:TRAJECTORY<X>1.0</X><Y>0.5</Y><Z>-1.0</Z>
E:SCORE250
E:SAFESWITCH(my_data)
```

Tags are parsed by `MWParser_ReadTag` which extracts name-value pairs.

## Ball State Offsets Used by Events

| Ball Offset | Type | Used By |
|-------------|------|---------|
| +0x059/5A/5B | float | Position X/Y/Z (for 3D sound) |
| +0x0A7/0A8 | float | Jump force direction/mode |
| +0x0B3 | byte | In-tar flag |
| +0x0B4 | float | Tar entry Y position |
| +0x0B6 | int | Water timer (10 frames) |
| +0x0CB | AthenaList | ONCE event tracker (prevents re-trigger) |
| +0x1DA | byte | Active flag (cleared on LIMIT/TARPIT) |
| +0x1F2 | int | DROPIN cooldown |
| +0x1F5 | int | Unknown cooldown (50 frames) |
| +0x1F7 | int | Impact counter (jump/bounce cooldown) |
| +0x202 | int | Freeze counter (input disabled while > 0) |
| +0x2D5 | byte | In-water flag |
| +0x2E9 | byte | LIMIT hit flag |
| +0x30B | byte | SAFESWITCH state |
| +0xC2C | char[] | SAFESWITCH data buffer |
| player_index*0xA0 | per-player | Score, timer, finish state offsets |

## App Offsets Used by Events

| App Offset | Type | Description |
|------------|------|-------------|
| +0x0210 | char* | Activity log string ("Reach Goal 1"..."Update") |
| +0x0220 | ptr | Player profile (checks +0x10/11 for multiplayer) |
| +0x0234 | byte | Some flag |
| +0x023C | int | Multiplayer flag (gates arena events) |
| +0x0294-2C4 | float* | Race timer positions (4 players × mirror/normal) |
| +0x0460 | int | DROPIN sound handle |
| +0x0464 | int | Catapult sound handle |
| +0x0468 | int | POPOUT sound handle |
| +0x046C+ | int[3] | PIPEBONK sound handles (3 random choices) |
| +0x0484 | int | TARPIT sound handle |
| +0x049C | int | JUMP sound handle |
| +0x04CC | int | Unknown sound handle |
| +0x053C | void* | Music player handle |
| +0x05CC+ | per-player | Score timer position |
| +0x05D6+ | per-player | Finish flag (bool) |
| +0x05E4+ | per-player | Accumulated score (float) |
| +0x05EC+ | per-player | Extra time (bell, 500 units) |
| +0x05F0+ | per-player | Another finish flag |
| +0x05FC+ | per-player | Race finished flag |
| +0x05E8 | int | Tournament time record |
| +0x090C | int | Tournament scene manager (optional) |

## Per-Player Offset Calculation

Many app offsets use the pattern: `app + playerIndex * 0xA0 + baseOffset`

Ball player index is at `ball[6]` (param_1[6]).

Examples:
- Score: `app + playerIdx*0xA0 + 0x5E4`
- Timer: `app + playerIdx*0xA0 + 0x5CC`
- Finish: `app + playerIdx*0xA0 + 0x5D6`
- Extra time: `app + playerIdx*0xA0 + 0x5EC`

## Sound System Integration

3D positioned sounds use `Sound_Play3D(handle, x, y, z)` where position comes
from the ball. Non-positioned sounds use `Sound_PlayChannel(handle)`.

Each sound event has a cooldown timer (impact/freeze counter) to prevent
re-triggering every frame while the ball is inside the trigger volume.

## Arena Object Storage

Arena-specific objects are stored in the Board/Level:

| Level Offset | List | Objects |
|-------------|------|---------|
| +0x436C | BonkList | Hammer/bonk objects |
| +0x4370 | SawList1 | Saw blade 1 |
| +0x4374 | SawList2 | Saw blade 2 |
| +0x43B8 | CatapultList | Catapults (count at +0x43BC, array at +0x47C4) |
| +0x47D0 | TrapdoorList | Trapdoors (count at +0x47D4, array at +0x4BDC) |
| +0x4BBC | JudgeList | Judges (count at +0x4BC0, array at +0x4FC8) |
| +0x4FD4 | BellList | Bell objects |
| +0x5000 | MaceList | Maces (count at +0x5004, array at +0x540C) |

Each object in these lists has its ID at `+0x10D4` which is compared against
the collision object's ID to find the matching game object.
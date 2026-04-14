# Level Object System

Interactive objects in race levels and RumbleBoard arenas. Created by the
level parser and triggered by collision events.

## Level Object Factory Hierarchy

```
Level (base, 0x10D0 bytes)
├── Stands (inherits Level)
│   └── Catapult (0x10D0 base + extra)
├── Sawblade_Level (0x111C bytes)
├── TowerLevel (0x1188 bytes)
├── Spinner_Level / Bridge (0x10FC bytes)
├── Gear_Level / Judge (0x1100 bytes)
├── Tipper_Level / Bell (0x10E8 bytes)
├── Bonk / Hammer (0x1200 bytes)
├── Trapdoor
└── CollisionLevel (0x10D0 bytes)
```

## Level Base Structure (0x10D0 bytes minimum)

| Offset | Type | Field |
|--------|------|-------|
| +0x000 | vtable* | Virtual function table |
| +0x434 | int | Type identifier |
| +0x480 | MeshWorld* | Mesh world pointer |
| +0x10D0 | int | Parent board pointer |

## Catapult System

### Catapult_ctor (0x437E10)

Inherits from Stands (which inherits from Level).
Creates a CollisionLevel for the catapult mesh.

| Offset | Type | Default | Field |
|--------|------|---------|-------|
| +0x10D0 | int | param_1 | App pointer |
| +0x10D4 | CollisionLevel* | new(0x10D0) | Collision mesh |
| +0x10D8 | Vec3 | (0,0,0) | Launch direction |
| +0x10E4 | int | 0 | State |
| +0x10E8 | float | -1.0 | Gravity multiplier |
| +0x10F0 | byte | 0 | is_active |
| +0x10F4 | int | — | Launch timer (set to 50 on launch) |
| +0x10F8 | byte | 0 | Launch state flag |
| +0x1100 | byte | 0 | Complete flag |
| +0x1104 | float | 16.5 | Launch velocity (0x41880000) |

### Catapult_Launch (0x434290)

Triggered by "E:CATAPULTBOTTOM" collision event (if ball cooldown < 1):
1. Set `is_active = 1` (offset 0x10F0)
2. Set `launch_timer = 50` ticks (0x32)

The catapult then updates in `Catapult_Update` (0x43E600) each frame,
animating the launch arc and applying velocity to the ball.

### Catapult_AddObjectConditional (0x43E9C0)

Conditionally adds an object to the catapult's launch target list.
Only adds if the object meets certain criteria (checked internally).

### Catapult_Render (0x43EA70)

Renders the catapult mesh with the current animation state.

## Trapdoor System

### Trapdoor_Open (0x4344D0)

Triggered by "E:OPENSESAME" collision event.
Opens the first trapdoor in the door list.

### Trapdoor_Activate (0x438410)

Triggered by "N:TRAPDOOR" collision event.
Activates all doors matching the collider's ID.

## ScoreDisplay System

The arena scoring display, created by App_CreateScoreDisplay (0x46CB00).

| Ctor Address | Variant |
|-------------|---------|
| 0x448590 | ScoreDisplay_CtorA |
| 0x4485C0 | ScoreDisplay_CtorB |
| 0x4485F0 | ScoreDisplay_CtorC |

### ScoreDisplay_SetTime (0x434C80)

Called from "E:SCORE\<time\>" collision events.
Parses the time value from the suffix string and updates the display.

## HighScore System

### HighScoreEntry (0x42B470)

Individual high score entry display. Created by App_CreateHighScoreEntry
(0x446A60) after a race completes.

### HighScoreMenu (0x42B290)

High scores viewer, accessible from Main Menu "HIGH SCORES" button (cmd="HS").

### CheckPurchaseOrHighScore (0x40A420)

Gatekeeper function — checks if the game is registered before showing
high scores. Also may check for purchase/unlock status.

## Damage System

From Level_HandleCollision (0x40DCD0):

### "E:BITE" Collision Event

When the ball hits a "BITE" object:
- Sets `damage_amount` = 25.0 (at App+0x43A0)
- Sets `damage_timer` (at App+0x43A8)

This causes the ball to take damage, likely displayed as a visual effect
and potentially affecting ball speed/handling temporarily.

## Jump Pad System

From Arena_HandleCollision (0x40E6A0):

### "E:JUMP" Collision Event

When the ball hits a jump pad:
1. `ball.cooldown` = 10 (jump cooldown ticks)
2. `ball.vert_vel` = 0.008 (upward velocity, 0x3B03126F)
3. `ball.vert_vel_on` = 1 (enable vertical velocity)
4. Play jump pad sound

## Collision Level Relationship

Each arena hazard that needs collision detection creates its own
CollisionLevel (0x10D0 bytes) via:

```c
CollisionLevel *cl = CollisionLevel_ctorWithLevel(new(0x10D0), parent_level);
```

The CollisionLevel wraps a MeshWorld with collision data for ray-casting
against that specific hazard's geometry. Multiple CollisionLevels can exist
simultaneously — the Catapult creates one, the RumbleBoard creates one for
the main arena, and some levels have additional sub-levels.

## Related Functions

| Address | Name | Purpose |
|---------|------|---------|
| 0x437E10 | Catapult_ctor | Launch pad constructor |
| 0x434290 | Catapult_Launch | Activate catapult |
| 0x43E600 | Catapult_Update | Per-frame catapult animation |
| 0x43E9C0 | Catapult_AddObjectConditional | Add launch target |
| 0x43EA70 | Catapult_Render | Render catapult |
| 0x4344D0 | Trapdoor_Open | Open trapdoor (OPENSESAME) |
| 0x438410 | Trapdoor_Activate | Activate trapdoor by ID |
| 0x434C80 | ScoreDisplay_SetTime | Set arena score time |
| 0x46CB00 | App_CreateScoreDisplay | Create score display widget |
| 0x446A60 | App_CreateHighScoreEntry | Create high score entry |
| 0x40A420 | CheckPurchaseOrHighScore | Purchase/registration gate |
| 0x40E250 | CreateSawblade | Master arena factory |
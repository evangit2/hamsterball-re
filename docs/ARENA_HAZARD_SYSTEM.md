# Arena Hazard System (RumbleBoard Objects)

The RumbleBoard arenas contain interactive hazards created by the master factory
function `CreateSawblade` (0x40E250). Despite its name, this function creates
ALL arena object types, not just sawblades.

## Object Factory: CreateSawblade (0x40E250)

Dispatches on name prefix (case-insensitive `__strnicmp`). Only creates objects
when `App+0x23C != 0` (Frenzied difficulty check for tournament-only objects).

### Object Type Table

| Name Prefix | Class | Size | Ctor Address | Storage |
|-------------|-------|------|-------------|---------|
| "BONK" | Bonk | 0x1200 | Bonk_ctor | this+0x2578 list, this+0x436C ptr |
| "SAW" (3 chars) | TowerLevel | 0x1188 | TowerLevel_Ctor | this+0x2578 list |
| "SAWBLADE" | Sawblade_Level | 0x111C | Sawblade_Level_Ctor | this+0x2578 list, this+0x4370/4374 |
| "BRIDGE" | Spinner_Level | 0x10FC | Spinner_Level_ctor | this+0x4380/4798 lists |
| "JUDGE" | Gear_Level | 0x1100 | Gear_Level_ctor | this+0x4BBC list |
| "BELL" | Tipper_Level | 0x10E8 | Tipper_Level_Ctor | this+0x2578 list, this+0x4FD4 ptr |

### Name Suffix Modifiers

Objects can be named with suffix modifiers that change behavior:

| Suffix | Object Type | Effect | Offset Modified |
|--------|-------------|--------|----------------|
| "SLOW" | TowerLevel | Slow speed mode | obj+0x43B = 1 |
| "SUPER" | TowerLevel | Super mode | obj+0x10ED = 1 |
| "UP" | TowerLevel | Initialize sound channels | Sound_InitChannels(1) |
| "1" | Sawblade | Saw #1, break sound 1 | this+0x4370, Sawblade_SetBreakSound(1) |
| "2" | Sawblade | Saw #2, break sound 2 | this+0x4374, Sawblade_SetBreakSound(2) |
| "1" | Bridge | Add to bridge list 1 | this+0x4380 list |
| "2" | Bridge | Add to bridge list 2 | this+0x4798 list |
| "NEG" | Bridge | Reverse rotation (speed=-1.0) | obj[0x43E] = 0xBF800000 |

## Sawblade System

### Sawblade_Level Structure

| Offset | Type | Field | Default |
|--------|------|-------|---------|
| +0x10D0 | int | board_ptr | Parent RumbleBoard |
| +0x10D4 | Vec3 | position | Creation position |
| +0x10E0 | Vec3 | base_position | Copy of position |
| +0x10EC | int | state | 0 (inactive) |
| +0x10F0 | float | angle | RNG_Rand(0x168) random start angle |
| +0x10F4 | int | tick_count | 0 |
| +0x10FC | int | anim_frame | 0 |
| +0x110C | byte | is_alert | 0 |
| +0x110D | byte | alert_enabled | 1 (can show alert) |
| +0x1110 | float | alert_timer | 500.0 (0x43FA0000) |
| +0x1114 | byte | is_active | 0 |
| +0x1118 | int | break_sound_id | 0 |

### Sawblade Lifecycle

1. **Alert Phase**: `Saw_AlertActivate` (0x434770)
   - Sets `alert_enabled = 0` (one-shot)
   - Plays warning sound at saw position (3D positioned)
   - Triggered by "E:ALERTSAW1"/"E:ALERTSAW2" collision events

2. **Active Phase**: `Saw_Activate` (0x434A50)
   - Sets `is_active = 1`
   - Plays break/activate sound at position
   - Triggered by "E:ACTIVATESAW1"/"E:ACTIVATESAW2" collision events

3. **Render**: `Sawblade_Render` (0x4347E0)
   - Draws the spinning sawblade mesh

4. **Sound**: `Sawblade_SetBreakSound` (0x434AB0)
   - Sets which sound ID to play on break (1 or 2)
   - Two saw slots per arena (offset 0x4370 and 0x4374)

### Sawblade Sound References

| App Offset | Sound Type |
|-----------|------------|
| +0x4BC | Alert/ping sound |
| +0x4C0 | Break/activate sound |

## Spinner (Bridge) System

### Spinner_Level (0x10FC bytes)

Rotating bridge platforms. Created with "BRIDGE" prefix.

Two independent bridge lists allow separate control:
- Bridge list 1: `this+0x4380` (triggered by one event)
- Bridge list 2: `this+0x4798` (triggered by another event)

The "NEG" suffix reverses rotation direction (speed = -1.0).

## Judge (Gear) System

### Gear_Level (0x1100 bytes)

Rotating gear hazards named "JUDGE" prefix. Stored in a judge list at
`this+0x4BBC`. Triggered by "E:ALERTJUDGES" collision event which calls
`Judge_Reset` for all gears.

## Bell (Tipper) System

### Tipper_Level (0x10E8 bytes)

Tilting/bell hazards named "BELL" prefix. Only one bell stored directly at
`this+0x4FD4`. Also added to the general object list at `this+0x2578`.

Triggered by "E:BELL" collision events — activates bell and awards 500 bonus
time if not already playing.

## Bonk (Hammer) System

### Bonk (0x1200 bytes)

Hammer obstacles named "BONK" prefix. Only created on Frenzied difficulty
(App+0x23C != 0). Stored at `this+0x436C` (single hammer) and `this+0x2578` list.

Hammer_ChaseStart (0x438BB0) starts the chase sequence for tournament mode.
The collision event "E:CALLHAMMER" creates a bonk popup via CreateBonkPopup.

## TowerLevel (Spinning Tower) System

### TowerLevel (0x1188 bytes)

Spinning tower obstacles with "SAW" (3-char) prefix. Not the same as sawblades.
Has speed modifiers via name suffixes:
- "SLOW": Reduced rotation speed (obj+0x43B=1)
- "SUPER": Enhanced mode (obj+0x10ED=1)
- "UP": Initialize additional sound channel

## Object Storage Map (RumbleBoard offsets)

| Offset | Type | Purpose |
|--------|------|---------|
| +0x2578 | AthenaList | All arena objects (master list) |
| +0x436C | void* | Bonk/Hammer pointer |
| +0x4370 | void* | Sawblade #1 pointer |
| +0x4374 | void* | Sawblade #2 pointer |
| +0x4380 | AthenaList | Bridge list #1 |
| +0x4798 | AthenaList | Bridge list #2 |
| +0x4BBC | AthenaList | Judge/Gear list |
| +0x4FD4 | void* | Bell/Tipper pointer |

## Collision-to-Action Dispatch

See COLLISION_SYSTEM.md for the full Arena_HandleCollision dispatch table.
Summary of arena collision events:

| Event Name | Action | Object |
|-----------|--------|--------|
| E:ALERTSAW1 | Saw_AlertActivate | Saw #1 warning ping |
| E:ALERTSAW2 | Saw_AlertActivate | Saw #2 warning ping |
| E:ACTIVATESAW1 | Saw_Activate | Saw #1 full activate |
| E:ACTIVATESAW2 | Saw_Activate | Saw #2 full activate |
| E:ALERTJUDGES | Judge_Reset | All gears warning |
| E:SCORE\<n\> | ScoreDisplay_SetTime | Set time from suffix |
| E:JUMP | Jump pad activation | +vert_vel, sound |
| E:BELL\<suffix\> | Bell_Activate | +500 bonus time |
| E:CALLHAMMER | CreateBonkPopup | Tournament bonk |
| E:HAMMERCHASE | Hammer_ChaseStart | Tournament chase |

## App Difficulty Values

From DifficultyMenu_ctor and Tournament_AdvanceRace:

| Difficulty | Value | Time Bonus | Speed |
|-----------|-------|-----------|-------|
| PIPSQUEAK | "EASY" | +1000ms | Slower hazards |
| NORMAL | "NORMAL" | Standard | Normal speed |
| FRENZIED! | "HARD" | +500ms | Faster + BONK/JUDGE objects |

Frenzied mode enables creation of BONK and JUDGE objects (App+0x23C check)
and reduces time bonus from +1000 to +500.

## Related Functions

| Address | Name | Purpose |
|---------|------|---------|
| 0x40E250 | CreateSawblade | Master arena object factory |
| 0x434660 | Sawblade_Level_Ctor | Sawblade constructor |
| 0x434A50 | Saw_Activate | Full activation |
| 0x434770 | Saw_AlertActivate | Warning phase |
| 0x4347E0 | Sawblade_Render | Render spinning blade |
| 0x434640 | Sawblade_SetActive | Set active state |
| 0x434AB0 | Sawblade_SetBreakSound | Set break sound ID |
| 0x438BB0 | Hammer_ChaseStart | Start hammer chase |
| 0x40E2A4 | Bonk_ctor | Hammer/bonk constructor |
| 0x40E340 | TowerLevel_Ctor | Spinning tower constructor |
| 0x40E4C0 | Spinner_Level_ctor | Bridge/spinner constructor |
| 0x40E5AC | Gear_Level_ctor | Judge/gear constructor |
| 0x40E61F | Tipper_Level_Ctor | Bell/tipper constructor |
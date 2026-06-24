# Critical Function Deep Analysis

Two functions were flagged as CRITICAL misnomers during the full decompilation pass.
Each was analyzed three times to verify accuracy. This document records the verified
findings.

---

## 1. TimerDisplay (0x004298C0) — Actually: App_ResourceLoader

### Misnomer Summary

| Field | Value |
|---|---|
| **Current name** | `TimerDisplay` |
| **Correct name** | `App_ResourceLoader` (or `LoadingScreenGadget_Factory`) |
| **Severity** | CRITICAL — name is completely unrelated to function behavior |
| **Evidence** | Zero timer logic. Loads 195 game resources via vtable dispatch. |

### Calling Convention

```c
void __fastcall TimerDisplay(int param_1);
```

- **`param_1`** (ECX, `__fastcall`): Pointer to the **App** struct (the global
  application object, ~2328 bytes). Not a timer, not a display widget.
- **Returns:** void. All output is written through the App struct.

The function pointer `0x004298C0` is stored in a dispatch table at VA `0x004D2700`
in the `.rdata` section, alongside other App initialization functions:
`App_FrameTick` (`0x0046C9E0`), `App_SetTitleString` (`0x0046CB70`), etc.

### What It Actually Does

**Step 1 — Allocate LoadingScreenGadget (lines 20-28):**

```c
this = operator_new(0x3628);          // Allocate 13,864 bytes
piVar1 = LoadingScreenGadget_Ctor(this, param_1);  // Construct
*(int**)(param_1 + 0x22c) = piVar1;   // Store gadget pointer at App+0x22C
*(undefined4*)(param_1 + 0x31c) = 0;  // Clear font slot
```

Creates a `LoadingScreenGadget` object (0x3628 = 13,864 bytes) and stores it at
`App+0x22C`. This gadget is the master resource manager that owns all game assets.

**Step 2 — Load 195 resources via vtable dispatch:**

The gadget's vtable (`*piVar1`) has 7 resource-loading methods at different offsets.
Each call writes a resource handle into a specific slot in the App struct.

| vtable Offset | Method Type | Count | App Slots Used | Resources |
|---|---|---|---|---|
| +0x58 | Texture loader (simple) | 59 | +0x318–0x438 | titletext, hammy1-3, blueblot, goal.png, timerblot.png, star.png, dust.png, chrome.png, medals, tourney-*.png, etc. |
| +0x48 | Texture loader (with flags) | 37 | +0x27C–0x314 | sign-bewarethetar, goal.png, goal-lit.png, goal-mirrored.png, locktile.png, arrow1.png, checker textures (pink/blue/green/red/orange), brick textures, etc. |
| +0x5C | Font loader | 3 | +0x318–0x328 | showcardgothic28, arialnarrow12bold, showcardgothic72, showcardgothic14, showcardgothic16 |
| +0x4C | Mesh loader | 24 | +0x244–0x5C4 | Sphere, SphereBreak1-2, Hamster-Waiting, Hamster-trot1-3, 8Ball, FunBall, Bell, Dizzy, RBGlare, Sphere+Tar, tarbubble, fanblades, fanbody, sawblade, sawface1-2, dawgshoe1-2, dawgshadow, GlassBonus, GlassBonus-Smashed, mousetrapshadow |
| +0x50 | Level loader | 7 | +0x570–0x5C8 | MouseTrap, Secret, Secret-Unlock, Level4-Trapdoor1-2, PopupSign, Level6-Lifter |
| +0x54 | Level reference copier | 4 | +0x574–0x5A0 | Copies level handles from one App slot to another (aliases) |
| +0x60 | Sound loader | 61 | +0x43C–0x52C | collide(10ch), roll(10ch), whistle(1ch), bumper(10ch), ballbreak(5ch), ballbreaksmall(5ch), thwomp(2ch), snap(2ch), popup(2ch), dropin(2ch), dropinshort(2ch), popout(2ch), pipebump1-3(10ch each), gearclank(20ch), bridgeslam(2ch), platformtick(5ch), gluestuck(5ch), bubble1-2(5ch), wheelcreak(2ch), catapult(2ch), trapdoor(2ch), fwing(2ch), clink(3ch), whoosh(3ch), chomp(1ch), fan-start(10ch), fan-blow(10ch), crack(2ch), crumble(2ch), sawstartup(2ch), sawcut(2ch), minipop(5ch), bell(3ch), zip(2ch), ting(20ch), shrink(3ch), grow(3ch), tweet(3ch), creakyplatform(20ch), wubba(5ch), saw(2ch), sawspeedy(2ch), dawgstep1-2(10ch), dawgsmash(10ch), sizzle(2ch), explode(3ch), vac-o-sux(3ch), speedcylinder(2ch), bonuspop(5ch), buzzbonus(1ch), breakbridge(1ch), unlock(1ch), NeonRide(1ch), NeonFlicker(50ch), ZoopDown(2ch), LightsOff(2ch), GlassBonus(2ch) |

**Step 3 — Merge and register (lines 246-249):**

```c
Menu_MergeAllLists(*(int*)(param_1 + 0x22c));    // Merge all resource lists
Scene_AddObject(*(void**)(param_1 + 0x184),      // Add to scene at App+0x184
                *(int**)(param_1 + 0x22c));
```

Merges all loaded resource lists into the gadget and adds the gadget as a scene
object so it participates in the render/tick loop.

### Summary

This function is the **master game asset preloader**. It creates the
`LoadingScreenGadget` that owns every font, texture, mesh, level, and sound
effect in the game. The name `TimerDisplay` is completely wrong — the only
timer-related string is `"timerblot.png"` (a texture used for timer display,
not the function itself).

### Parameters

| Parameter | Type | Direction | Meaning |
|---|---|---|---|
| `param_1` | `int` (App*) | IN | Pointer to the global App struct |
| (no output params) | — | — | Results stored via App struct fields |

---

## 2. DispatchCollisionEvents (0x0040C5D0) — Actually: LevelCollisionEventHandler

### Misnomer Summary

| Field | Value |
|---|---|
| **Current name** | `DispatchCollisionEvents` |
| **Correct name** | `LevelCollisionEventHandler` (or `HandleCollisionEvent`) |
| **Severity** | CRITICAL — name describes 1 of 18 event types handled |
| **Evidence** | 28 call sites, 18 event branches, 0 allocations (doesn't "create" anything) |

### Calling Convention

```c
void __thiscall DispatchCollisionEvents(void *this, int *param_1, int *param_2);
```

Verified by decompiling `TowerCollisionEvents` (0x0040DCD0), which calls
`DispatchCollisionEvents` with the same parameters it receives:

```c
// In TowerCollisionEvents(this, ball, collObj):
//   ... handles E:CATAPULTBOTTOM, E:OPENSESAME, N:TRAPDOOR, E:BITE, E:MACETRIGGER, N:MACE ...
//   ... then falls through to:
DispatchCollisionEvents(this, ball, collObj);
```

### Parameters

| Parameter | Register | Type | Direction | Meaning |
|---|---|---|---|---|
| `this` | ECX | `void*` (Scene/Board) | IN | The containing scene or board object |
| `param_1` | — | `int*` (Ball*) | IN/OUT | The ball that triggered the collision. Read for position, player index, cooldowns, CollisionMesh. Written to for velocity, flags, timers, scores. |
| `param_2` | — | `int*` (CollisionEvent*) | IN | The collision object data. `param_2[1]+0x864` = event string (the N:/E: tag). `*param_2+0x47C` = rotator ID. |
| (returns) | EAX | void | — | No return value |

### Ball Struct Offsets Accessed (param_1)

| Offset | Type | Field Name | Access | Used By |
|---|---|---|---|---|
| +0x059 (param_1[0x59]) | float | ball.pos_x | READ | E:JUMP, N:TARPIT, PIPEBONK (3D sound position) |
| +0x05A (param_1[0x5A]) | float | ball.pos_y | READ | E:JUMP, N:TARPIT, PIPEBONK |
| +0x05B (param_1[0x5B]) | float | ball.pos_z | READ | E:JUMP, N:TARPIT, PIPEBONK |
| +0x0B3 (param_1[0xB3]) | byte | ball.in_tar | READ/WRITE | N:TARPIT (set=1) |
| +0x0B4 (param_1[0xB4]) | float | ball.tar_entry_y | WRITE | N:TARPIT (stores entry Y) |
| +0x0B6 (param_1[0xB6]) | int | ball.water_timer | WRITE | N:WATER (=10) |
| +0x0A7 (param_1[0xA7]) | float | ball.vel_x | WRITE | E:JUMP (=0.002) |
| +0x0A8 (param_1[0xA8]) | float | ball.vel_y | WRITE | E:JUMP (=1.0) |
| +0x0CB (param_1+0xCB) | AthenaList | ball.action_list | READ/WRITE | E:ACTION (ONCE tracking) |
| +0x018 (param_1[6]) | int | ball.player_index | READ | E:LIMIT, E:ACTION, N:GOAL (0-3) |
| +0x014 (param_1[5]) | int* | ball.board_ptr | READ | E:LIMIT (RumbleBoard) |
| +0x028 (param_1[0x69>>2]) | int* | ball.collision_mesh | READ | E:TRAJECTORY, DROPIN, N:MOUSETRAP |
| +0x053 (param_1[0x53]) | byte | ball.goal_reached | READ | N:GOAL (gate) |
| +0x1DA (param_1+0x1DA) | byte | ball.enabled/active | READ/WRITE | N:GOAL (gate), E:LIMIT, N:TARPIT (=0) |
| +0x1F2 (param_1[0x1F2]) | int | ball.dropin_cooldown | READ/WRITE | DROPIN (=50) |
| +0x1F5 (param_1[0x1F5]) | int | ball.zip_cooldown | READ/WRITE | ZIP event (=50) |
| +0x1F7 (param_1[0x1F7]) | int | ball.jump_cooldown | READ/WRITE | E:JUMP (=10) |
| +0x1F8 (param_1[500]) | int | ball.pipebonk_cooldown | READ/WRITE | PIPEBONK (=10) |
| +0x1F4 (param_1[499]) | int | ball.popout_cooldown | READ/WRITE | POPOUT (=50) |
| +0x202 (param_1[0x202]) | int | ball.impact_timer | WRITE | E:JUMP, N:NOCONTROL (=10) |
| +0x2D5 (param_1+0x2D5) | byte | ball.in_water | WRITE | N:WATER (=1) |
| +0x2E9 (param_1+0x2E9) | byte | ball.limit_flag | WRITE | E:LIMIT (=1) |
| +0x30B (param_1+0x30B) | byte | ball.safe_switch_flag | WRITE | E:SAFESWITCH (=0 if no paren) |
| +0xC2C (param_1+0xC2C) | char[] | ball.switch_data | WRITE | E:SAFESWITCH (copies parenthesized data) |

### CollisionMesh Offsets (param_1[0x69])

| Offset | Type | Field | Used By |
|---|---|---|---|
| +0xCA4 | float | velocity_x | E:TRAJECTORY (write), DROPIN (read), N:MOUSETRAP (read/write) |
| +0xCA8 | float | velocity_y | E:TRAJECTORY (write), DROPIN (read), N:MOUSETRAP (write=15.0) |
| +0xCAC | float | velocity_z | E:TRAJECTORY (write), DROPIN (read), N:MOUSETRAP (read/write) |

### The 18 Event Branches

The function reads the event string from `param_2[1]+0x864` and dispatches
based on its content. All comparisons are case-insensitive.

#### N: prefix events (Named collision zones)

| # | Event String | Comparison | Condition | Action | Ball Fields Written | Points |
|---|---|---|---|---|---|---|
| 1 | `N:SECRET` | strnicmp(8) | — | Calls `Rotator_MarkTriggered(rotator_id)` | none | 0 |
| 2 | `N:UNLOCKSECRET` | strnicmp(14) | — | Calls `CheckArenaUnlock(this)` | none | 0 |
| 3 | `E:NODIZZY` | strnicmp(9) | — | Parses `<TIME>N</TIME>` XML. Calls `Ball_RecordBest(ball, N)`. | Ball_RecordBest | 0 (records time) |
| 4 | `E:SAFESWITCH` | strnicmp(12) | — | Finds `(` in string. Copies parenthesized data to `ball+0xC2C`. If no `(`, clears `ball+0x30B=0`. | +0xC2C, +0x30B | 0 |
| 5 | `E:LIMIT` | stricmp | — | Clears `ball+0x1DA=0`, sets `ball+0x2E9=1`. Tracks arena knockoff counts for all 4 players at `RumbleBoard+0x47B4–0x47C0`. Checks each player's ball completion flags at `App+0x5D7/0x677/0x717/0x7B7` and ball validity at `App+0x5DC/0x67C/0x71C/0x7BC`. | +0x1DA, +0x2E9, RumbleBoard counters | 0 |
| 6 | `E:BREAK` | stricmp | — | Calls `ball->vtable[0x20]()` — virtual break method. | vtable dispatch | 0 |
| 7 | `E:JUMP` | stricmp | `ball.jump_cooldown < 1` | Plays 3D jump sound at ball position. Sets `vel_x=0.002`, `vel_y=1.0`, `impact=10`, `jump_cooldown=10`. | +0x1F7=10, +0xA7=0.002, +0xA8=1.0, +0x202=10 | +200 |
| 8 | `E:ACTION` | strnicmp(8) | — | Parses XML. `<ONCE>TRUE</ONCE>`: adds event to `ball.action_list` if not present (one-shot gate). `<SCORE>N</SCORE>`: adds `N × difficulty_modifier` to player score at `App+0x5E4+playerIndex×0xA0`. Updates ball name with score. | +0xCB (list), App+0x5E4 | N × difficulty |
| 9 | `E:TRAJECTORY` | strnicmp(12) | — | Parses `<X>f</X> <Y>f</Y> <Z>f</Z>` XML. Writes Vec3 to `CollisionMesh+0xCA4/0xCA8/0xCAC` (velocity override). | CM+0xCA4/CA8/CAC | 0 |
| 10 | `N:NOCONTROL` | stricmp | — | Sets `ball.impact_timer = 10` (disables input). | +0x202=10 | 0 |
| 11 | `N:WATER` | stricmp | — | Sets `ball.in_water = 1`, `ball.water_timer = 10`. | +0x2D5=1, +0xB6=10 | 0 |
| 12 | `N:TARPIT` | stricmp | — | If not already in tar: plays tar sound, stores `ball.pos_y` as entry height. Sets `ball.in_tar=1`, clears `ball+0x1DA=0`. | +0xB3=1, +0xB4=pos_y, +0x1DA=0 | 0 |
| 13 | `N:GOAL` | manual(7 bytes) | `ball.goal_reached==0 AND ball.enabled!=0` | **RACE FINISH.** Plays "Goal!" music. Sets `scene+0xCD0=1`. For multiplayer: copies race timer. Sets `App+0x5D6+playerIdx×0xA0=1` (goal reached). Swaps goal textures unlit→lit. If mirror mode: swaps mirrored textures too. Sets `App+0x5F0+playerIdx×0xA0=1` (race finished). Updates status text to "Update". | scene+0xCD0, App+0x5D6/5FC/5F0 (+playerIdx×0xA0) | 0 |
| 14 | `N:MOUSETRAP` | manual(12 bytes) | — | Reads ball velocity from `CollisionMesh+0xCA4/CA8/CAC`. Normalizes it. Sets `vel_y = 15.0`. Scales entire vector by `20.0` (DAT_004cf370). Writes back. Iterates rotator list at `scene+0x1D3C`, finds matching rotator by ID (`rotator+0x10D4 == *param_2+0x47C`). On match: plays collision sound, adds rotator to `scene+0x2578` (collision list). | CM+0xCA4/CA8/CAC (velocity modified), scene+0x2578 (list append) | 0 |

#### X: prefix events (Anonymous collision events, matched by suffix only)

These use `stricmp(event+2, ...)` — skipping the 2-character prefix,
so they match any `X:` prefix (e.g., `E:DROPIN`, `N:DROPIN`, etc.).

| # | Event Suffix | Condition | Action | Ball Fields Written | Points |
|---|---|---|---|---|---|
| 15 | `DROPIN` | `velocity_mag in range (0.0, >2.0) AND dropin_cooldown < 1` | Plays dropin sound. Sets cooldown=50. | +0x1F2=50 | +200 |
| 16 | `PIPEBONK` | `pipebonk_cooldown < 1` | Generates 2 random numbers. Plays random pipe bump sound (pipebump1/2/3). Sets cooldown=10. | +0x1F8=10 | +100 |
| 17 | `POPOUT` | `popout_cooldown < 1` | Plays popout sound. Sets cooldown=50. | +0x1F4=50 | +100 |
| 18 | `ZIP` | `zip_cooldown < 1` | Plays zip sound. Sets cooldown=50. | +0x1F5=50 | 0 |

### Verified Constants

| Address | Value | Type | Used By |
|---|---|---|---|
| `0x004CF368` | 0.0 | float | DROPIN: velocity upper bound (velocity must be < 0.0, i.e., the < 0.0 == == 0.0 check is always true) |
| `0x004CF370` | 20.0 | float | N:MOUSETRAP: velocity scale factor |
| `0x004CF48C` | 2.0 | float | DROPIN: minimum velocity magnitude (must be > 2.0) |
| `0x41700000` | 15.0 | float | N:MOUSETRAP: velocity Y override |
| `0x3B03126F` | 0.002 | float | E:JUMP: velocity X drift |

### Call Sites (28 total)

```
TowerCollisionEvents     (0x0040DCD0) — primary level collision dispatcher
ExpertCollisionEvents     (0x0040EA6B) — arena mode collision dispatcher
HandleArenaCollisionEvents             (0x00412850) — spinner/bumper/launch collision
NeonCollisionEvents               (0x00410E6F) — arena limit collision
SinkPlatformArenaCollisionEvents  (0x00413C09) — sinking platform collision
+ 23 other Create* and collision handler functions
+ 2 DATA references (vtable entries)
```

### Summary

`DispatchCollisionEvents` is the **universal level collision event handler**. It is called
as a fall-through from `TowerCollisionEvents` and `ExpertCollisionEvents` after
those functions handle their own specific events. It dispatches 18 different event
types based on the collision event string:

- **Race progression:** N:GOAL (finish), N:SECRET (mark secret found)
- **Arena mode:** E:LIMIT (track knockoffs), N:UNLOCKSECRET (unlock check)
- **Ball physics:** E:JUMP (impulse), E:TRAJECTORY (velocity override),
  N:MOUSETRAP (deflect), E:BREAK (shatter)
- **Environmental:** N:WATER (water zone), N:TARPIT (tar zone), N:NOCONTROL (stun)
- **Pipe events:** DROPIN, PIPEBONK, POPOUT, ZIP (sound + score)
- **Scoring/tracking:** E:ACTION (once-gated score), E:NODIZZY (time record)
- **Switches:** E:SAFESWITCH (copy switch data to ball)

The name `DispatchCollisionEvents` describes only the `E:NODIZZY` branch (branch #3), which
is one of 18. The function creates nothing — it is a pure event handler that reads
the collision event string and dispatches side effects to the ball, App, and
scene structs.

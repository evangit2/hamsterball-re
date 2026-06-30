# Sawblade & Drawbridge System (Expert + Tower Races)

Complete reverse-engineering analysis of the Sawblade (Expert Race) and Drawbridge (Tower Race) systems.

---

## SAWBLADE (Expert Race)

### Binary Addresses

| Function | Address | Purpose |
|----------|---------|---------|
| CreateSawblade (Arena Factory) | 0x40E590 | Factory — creates SAWBLADE from level refs |
| Sawblade_Level_Ctor | 0x434660 | Constructor |
| Sawblade_Update (vtable[11]) | 0x439BB0 | Per-frame update (spin, debris, collision, movement) |
| Sawblade_Render (vtable[18]) | 0x4347E0 | Renders saw mesh + effects |
| Saw_Activate | 0x434A50 | Activates saw (E:ACTIVATESAW1/2) |
| Saw_AlertActivate | 0x434770 | Alert sound (E:ALERTSAW1/2) |
| Sawblade_SetActive | 0x434640 | Set active state |
| Sawblade_SetBreakSound | 0x434AB0 | Assign break sound |
| Sawblade_Level_Dtor | 0x434760 | Destructor |
| Sawblade vtable | 0x4D5240 | Vtable pointer |

### Strings

| String | VA | Purpose |
|--------|-----|---------|
| `SAWBLADE` | 0x4CFA28 | Factory lookup (strnicmp 8 chars) |
| `Meshes\sawblade` | 0x4D3390 | Mesh file |
| `SAW1-BREAK` / `SAW2-BREAK` | — | Break sound names |
| `SAWPATH` / `SMALLSAWPATH` | — | Path names for saw movement |
| `sounds\saw` / `sawcut` / `sawspeedy` / `sawstartup` | — | Sound files |

### Object Structure (0x111C bytes = 4380)

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x0000 | 4 | ptr | Vtable (0x4D5240) |
| +0x10D0 | 4 | ptr | Board pointer |
| +0x10D4 | 4 | float | Position X (current, moves during activation) |
| +0x10D8 | 4 | float | Position Y |
| +0x10DC | 4 | float | Position Z (current, moves during activation) |
| +0x10E0 | 4 | float | Home position X |
| +0x10E4 | 4 | float | Home position Y |
| +0x10E8 | 4 | float | Home position Z (boundary check) |
| +0x10EC | 4 | int | Unused? |
| +0x10F0 | 4 | float | Rotation angle (RNG 0–360, decreases = clockwise) |
| +0x10F4 | 4 | float | Spin speed accumulator (0 → 25.0) |
| +0x10F8 | 4 | int | Direction (1=Z-axis movement, 2=X-axis movement) |
| +0x10FC | 4 | int | Debris spawn counter |
| +0x1100 | 4 | float | X-axis boundary limit |
| +0x1108 | 4 | float | Z-axis boundary limit |
| +0x110C | 1 | byte | Break triggered flag |
| +0x110D | 1 | byte | Alert flag (1=can alert, 0=already alerted) |
| +0x1110 | 4 | float | Spin speed (500.0 initial, decays ×0.95 when not alerted) |
| +0x1114 | 1 | byte | Activated flag (0=idle, 1=active/moving) |
| +0x1118 | 4 | float | Movement velocity (accumulates) |

### Creation Flow

**Factory** (`CreateSawblade` @ 0x40E590, Arena mode only):
```c
if (strnicmp(name, "SAWBLADE", 8) == 0 && App+0x23C != 0) {  // difficulty gate!
    obj = operator_new(0x111C);                    // 4380 bytes
    Sawblade_Level_Ctor(obj, board, x, y, z);
    AthenaList_Append(board+0x2578, obj);           // general list
    if (strstr(name, "1")) {
        board+0x4370 = obj;                         // saw 1
        Sawblade_SetBreakSound(obj, 1);
    }
    if (strstr(name, "2")) {
        board+0x4374 = obj;                         // saw 2
        Sawblade_SetBreakSound(obj, 2);
    }
}
```

**Constructor** (`Sawblade_Level_Ctor` @ 0x434660):
```c
Level_ctor(this, d3d_device);                     // loads "Meshes\sawblade" mesh internally
vtable = 0x4D5240;
board = param_1;                                   // +0x10D0
pos = {x, y, z};                                   // +0x10D4 (current) AND +0x10E0 (home)
rotation = RNG(0, 360);                            // +0x10F0
spin_speed = 0;                                    // +0x10F4
alert_flag = 1;                                    // +0x110D (can alert)
spin_velocity = 500.0;                             // +0x1110
activated = 0;                                     // +0x1114
velocity = 0;                                     // +0x1118
```

### Activation

Two-stage activation via collision events in `ExpertCollisionEvents` (0x40E6A0):

1. **Alert** (`E:ALERTSAW1` / `E:ALERTSAW2`):
   ```c
   Saw_AlertActivate(board+0x4370 or board+0x4374);
   // If alert_flag (+0x110D) != 0: set to 0, play alert sound
   ```

2. **Activate** (`E:ACTIVATESAW1` / `E:ACTIVATESAW2`):
   ```c
   Saw_Activate(board+0x4370 or board+0x4374);
   // Set +0x1114 = 1 (activated), play startup sound
   ```

### Update Function (vtable[11] @ 0x439BB0)

```c
void Sawblade_Update(this) {
    if (alert_flag == 0) {                         // +0x110D — no longer alerting
        // Decay spin speed
        spin_velocity *= 0.95;                     // +0x1110
        if (spin_velocity < 1.0) spin_velocity = 0;
        
        // Build up movement speed
        spin_speed += 0.1;                         // +0x10F4
        if (spin_speed > 25.0) spin_speed = 25.0;
        
        // Debris spawning
        debris_counter--;                          // +0x10FC
        if (debris_counter <= 0) {
            if (activated) debris_counter = 2;     // fast debris when active
            else debris_counter = 10;               // slow debris when idle
            spawn_debris_particles(this);
        }
        
        // Movement (only when activated)
        if (activated) {                           // +0x1114
            if (direction == 1) {                 // +0x10F8 — Z-axis
                pos.z -= velocity;                 // +0x10DC
                velocity += 4.17e-8;               // +0x1118 (gravity-like accel)
                if (abs(pos.z - home.z) >= boundary) {
                    // Hit boundary — trigger break
                    if (!break_triggered) {
                        break_triggered = 1;       // +0x110C
                        SceneObject_SpawnWithSound(board+0x4380);
                    }
                }
            }
            if (direction == 2) {                 // X-axis
                pos.x += velocity;                 // +0x10D4
                velocity += 4.17e-8;
                // Similar boundary check with board+0x4798
            }
        }
    }
    
    // Ball collision detection (always runs)
    for each ball in board+0x29D4:
        distance = sqrt((ball.x-pos.x)² + (ball.y-pos.y)² + (ball.z-pos.z)²)
        if (distance <= ball.radius + 100.0) {    // 100.0 = saw kill radius
            // Alignment check (axis-specific)
            if (alignment < ball.radius) {
                ball->vtable[8]();                // call damage/split function
            }
        }
    
    // Update rotation
    rotation -= spin_speed;                        // +0x10F0 (clockwise spin)
}
```

### Key Constants

| Address | Value | Purpose |
|---------|-------|---------|
| 0x4D092C | 0.95 | Spin velocity decay |
| 0x4D0924 | 0.1 | Spin speed increment |
| 0x4CFECC | 25.0 | Max spin speed |
| 0x4CF4DC | 8.0 | Speed range check |
| 0x4CF48C | 2.0 | Speed range check |
| 0x4CF480 | 75.0 | Position offset for debris |
| 0x4D5C78 | 4.17e-8 | Velocity increment (gravity) |
| 0x4D5C68 | 9999.0 | Large boundary value |
| 0x4CF454 | 100.0 | Ball collision radius |
| 0x4CF310 | 1.0 | Minimum spin threshold |

### Vtable (0x4D5240)

| Index | Offset | Address | Function |
|-------|--------|---------|----------|
| 0 | 0x00 | 0x439B90 | Sawblade_Level_scalar_dtor |
| 11 | 0x2C | 0x439BB0 | Sawblade_Update |
| 18 | 0x48 | 0x4347E0 | Sawblade_Render |

---

## DRAWBRIDGE (Tower Race)

The Tower race has TWO types of bridge objects:

### Type 1: BRIDGE (Spinning Drawbridge Platform)

Created by the Arena factory (`CreateSawblade` @ 0x40E590) for Arena mode.
In Tower RACE mode, bridges are pre-placed (configured via `CreateLevelObjects`).

| Function | Address | Purpose |
|----------|---------|---------|
| Spinner_Level_ctor | 0x4396F0 | Constructor |
| Spinner_Update (vtable[11]) | 0x439870 | Per-frame update (spin animation) |
| Spinner_Render (vtable[18]) | 0x45E0E0 | Render |
| Spinner vtable | 0x4D51E0 | Vtable pointer |

**Object Structure** (0x10FC bytes = 4348):

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x10D0 | 4 | ptr | Board pointer |
| +0x10D4 | 4 | float | Position X |
| +0x10D8 | 4 | float | Position Y |
| +0x10DC | 4 | float | Position Z |
| +0x10E0 | 4 | float | Rotation angle |
| +0x10E4 | 4 | int | State (0=normal) |
| +0x10E8 | 4 | float | Angular velocity |
| +0x10F0 | 4 | int | Timer (init 100) |
| +0x10F4 | 4 | ptr | CollisionLevel pointer |
| +0x10F8 | 4 | float | Direction multiplier (1.0 or -1.0) |

**Constructor** (`Spinner_Level_ctor` @ 0x4396F0):
```c
SceneObject_ctor(this, board+0x4378);                  // uses pre-loaded mesh at board+0x4378
vtable = 0x4D51E0;                                // Spinner vtable
board = param_1;                                   // +0x10D0
pos = {x, y, z};                                   // +0x10D4
rotation = param_5;                                // +0x10E0
angular_velocity = 0;                              // +0x10E8
timer = 100;                                       // +0x10F0
direction = 1.0;                                   // +0x10F8
// Creates CollisionLevel internally
collision_level = CollisionLevel_ctorWithLevel(operator_new(0x10D0), this);
```

**Mesh Dependency**: `SceneObject_ctor` reads from `board+0x4378`. The Tower constructor loads `Levels\Level4-Drawbridge` at `board+0x4370` and `Levels\Level4-Mace` at `board+0x4378`. For global spawn, use JIT mesh injection: save `board+0x4378`, load drawbridge mesh there, call ctor, restore.

### Type 2: BBRIDGE (Breakable Bridge Sections)

Created by `CreateLevelObjects` (0x4121D0) for "BBRIDGE1" / "BBRIDGE2" refs.

| Function | Address | Purpose |
|----------|---------|---------|
| BreakBridge_ctor | 0x436D70 | Constructor |
| BreakBridge_Update (vtable[11]) | 0x43DD80 | Per-frame update |
| BreakBridge_Render (vtable[18]) | 0x45E0E0 | Render |
| BreakBridge vtable | 0x4D5890 | Vtable pointer |

**Object Structure** (0x1100 bytes = 4352):

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x10D0 | 4 | ptr | Board pointer |
| +0x10D4 | 4 | float | Position X |
| +0x10D8 | 4 | float | Position Y |
| +0x10DC | 4 | float | Position Z |
| +0x10E0 | 4 | ptr | CollisionLevel pointer |
| +0x10E4 | 1 | byte | Active flag (1=active) |
| +0x10E8 | 4 | float | Z position (boundary) |
| +0x10EC | 4 | float | Position offset |
| +0x10F0 | 4 | float | Position offset |
| +0x10F4 | 4 | int | Trigger counter (0=not triggered) |
| +0x10F8 | 4 | float | Direction (0.0) |
| +0x10FC | 1 | byte | Active flag 2 |

**Constructor** (`BreakBridge_ctor` @ 0x436D70):
```c
SceneObject_ctor(this, mesh_ptr);                      // uses pre-loaded mesh (board+0x5410 or +0x5414)
vtable = 0x4D5890;                                // Pendulum/BreakBridge vtable
board = param_1;
pos = {x, y, z};
collision_level = CollisionLevel_ctorWithLevel(operator_new(0x10D0), this);
active = 0;                                       // +0x10FC
trigger_counter = 0;                              // +0x10F4
active_flag = 1;                                  // +0x10E4
```

### Tower Level Pre-loaded Meshes

`LevelBoard_Tower_ctor` (0x41E340) loads:

| Board Offset | Mesh File | Purpose |
|-------------|-----------|---------|
| +0x436C | `Levels\Level4-Catapult` | Catapult mesh |
| +0x4370 | `Levels\Level4-Drawbridge` | Drawbridge mesh |
| +0x4374 | `Meshes\YellowLink` | Yellow link mesh |
| +0x4378 | `Levels\Level4-Mace` | Mace mesh |
| +0x437C | `Levels\Level4-Windmill` | Windmill mesh |
| +0x4390 | `Meshes\Chomper` | Chomper mesh |
| +0x43B4 | `Levels\Level4-Turret` | Turret mesh |

### Arena Collision Events (Tower/Expert Arena)

| Event | Action |
|-------|--------|
| `E:ALERTSAW1` | `Saw_AlertActivate(board+0x4370)` — alert saw 1 |
| `E:ALERTSAW2` | `Saw_AlertActivate(board+0x4374)` — alert saw 2 |
| `E:ACTIVATESAW1` | `Saw_Activate(board+0x4370)` — activate saw 1 |
| `E:ACTIVATESAW2` | `Saw_Activate(board+0x4374)` — activate saw 2 |

All saw events require `App+0x23C != 0` (difficulty gate — Normal/Frenzied only).

---

## Global Spawn Approach

### Sawblade Global Spawn

Uses the same pattern as Global Bonk:
1. Hook at Ball_Update (0x405E22)
2. Save player position
3. On spawn trigger:
   - `operator_new(0x111C)`
   - `Sawblade_Level_Ctor(obj, board, x, y, z)` — loads saw mesh internally
   - Set `+0x10F8 = 1` (direction 1, Z-axis) or `2` (X-axis)
   - `AthenaList_Append(board+0x2578, obj)` — register to general list
   - Append collision level to `board+0x10EC`
4. Per-frame: call `Saw_Activate(obj)` to auto-activate (set `+0x1114=1`)

No pre-loaded mesh needed — `Level_ctor` loads `Meshes\sawblade` internally.

### Drawbridge Global Spawn

Uses JIT mesh injection (same pattern as Global Lifters):
1. Pre-load `Levels\Level4-Drawbridge` mesh → `board+0x4378` (temporarily)
2. Save old `board+0x4378` value
3. `operator_new(0x10FC)`
4. `Spinner_Level_ctor(obj, board, x, y, z, 0.0)` — reads mesh from `board+0x4378`
5. Restore `board+0x4378`
6. `AthenaList_Append(board+0x2578, obj)`
7. Append collision level to scene lists

The drawbridge mesh is loaded once and cached for subsequent spawns.

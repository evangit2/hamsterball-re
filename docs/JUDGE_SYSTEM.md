# Hammy Judge System (Expert Race)

Complete reverse-engineering analysis of the Judge ("Hammy Judge") system from the Expert Race in Hamsterball.

## Overview

Hammy Judges are floating, spinning score-display objects placed around the Expert Race track. They show time values above their heads and activate when a ball rolls over `E:ALERTJUDGES` trigger zones.

## Binary Addresses

| Function | Address | Purpose |
|----------|---------|---------|
| LevelBoard_Expert_ctor | 0x41EA40 | Expert level constructor — pre-loads judge meshes |
| CreateSawblade (Arena Factory) | 0x40E590 | Factory — creates JUDGE objects from level refs |
| Gear_Level_ctor | 0x43A150 | Judge constructor |
| Judge_Reset | 0x434C40 | Activates judge (called by E:ALERTJUDGES) |
| Gear_Update (vtable[11]) | 0x434B60 | Per-frame update (countdown + rotation) |
| Gear_Render (vtable[18]) | 0x43A270 | Renders mesh + floating 3D text |
| ScoreDisplay_SetTime | 0x434C80 | Sets score text with random modifiers |
| ExpertCollisionEvents | 0x40E6A0 | Collision handler (E:ALERTJUDGES, E:SCORE) |
| Gear_Level_Dtor | 0x434B50 | Destructor |
| Gear_DeletingDtor | 0x43A250 | Deleting destructor |

## Strings

| String | VA | Purpose |
|--------|-----|---------|
| `JUDGE` | 0x4CFA14 | Object name (factory lookup, strnicmp 5 chars) |
| `E:ALERTJUDGES` | 0x4CFA70 | Collision event — activates all judges |
| `meshes\hammyjudge` | 0x4D0AA8 | Mesh file for the judge visual |
| `E:SCORE` | — | Collision event — sets score on all judges |

## Object Structure (0x1100 bytes = 4352)

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x0000 | 4 | ptr | Vtable pointer (0x4D52B8 = Gear vtable) |
| +0x10D0 | 4 | ptr | Board pointer |
| +0x10D4 | 4 | float | Position X |
| +0x10D8 | 4 | float | Position Y |
| +0x10DC | 4 | float | Position Z |
| +0x10E0 | 4 | float | Scale (RNG 0–10) |
| +0x10E4 | 4 | float | Rotation angle (RNG 0–360, +2.0/frame) |
| +0x10E8 | ~32 | char[] | Display text buffer ("%1.1f" formatted) |
| +0x10F0 | 4 | int | Text width (for centering) |
| +0x10F4 | 1 | byte | Active flag (1=active/idle, 0=activated/counting down) |
| +0x10F8 | 4 | int | Display value (init 400, decays ×0.8) |
| +0x10FC | 4 | int | Countdown timer (RNG 100–150 frames) |

## Vtable (0x4D52B8)

| Index | Offset | Address | Function |
|-------|--------|---------|----------|
| 0 | 0x00 | 0x43A250 | Gear_DeletingDtor |
| 11 | 0x2C | 0x434B60 | Gear_Update (countdown + rotation) |
| 18 | 0x48 | 0x43A270 | Gear_Render (mesh + 3D text) |

## Creation Flow

### 1. Level Constructor — `LevelBoard_Expert_ctor` (0x41EA40)

```
AthenaList_Init(board+0x4BBC, 0)           ← initialize judge list (empty)

MeshNode("meshes\hammyjudge") → board+0x4BB0  ← pre-load judge mesh 1
MeshNode("meshes\hammyjudge") → board+0x4BB4  ← pre-load judge mesh 2
MeshNode("meshes\hammyjudge") → board+0x4BB8  ← pre-load judge mesh 3
```

The Expert level uses `Level5.MESHWORLD` for its geometry. This mesh file contains `N:JUDGE` ref points and `E:ALERTJUDGES` / `E:SCORE` collision triggers.

### 2. Factory — `CreateSawblade` (0x40E590)

When the level parser encounters `N:JUDGE` refs in the mesh:

```c
if (strnicmp(name, "JUDGE", 5) == 0) {
    obj = operator_new(0x1100);                    // 4352 bytes
    Gear_Level_ctor(obj, board, x, y, z);          // constructor
    AthenaList_Append(board+0x4BBC, obj);           // register to judge list
}
```

**No difficulty gate!** Unlike BONK (which checks `App+0x23C != 0`), JUDGE spawns on ALL difficulty levels.

### 3. Constructor — `Gear_Level_ctor` (0x43A150)

```c
void Gear_Level_ctor(this, board, x, y, z) {
    Level_ctor(this, d3d_device);                  // base class init
    this->vtable = &Gear_Vtable;                   // 0x4D52B8
    this->pos = {x, y, z};                         // +0x10D4/+0x10D8/+0x10DC
    this->board = board;                           // +0x10D0
    this->scale = RNG(0, 10);                      // +0x10E0
    this->active = 1;                              // +0x10F4 (starts ACTIVE)
    this->display_value = 400;                     // +0x10F8
    this->countdown = RNG(100, 150);               // +0x10FC
    this->rotation = RNG(0, 360);                  // +0x10E4
    ScoreDisplay_SetTime(this, 0);                 // format display text
}
```

## Activation — `E:ALERTJUDGES`

When a ball rolls over the `E:ALERTJUDGES` collision trigger, `ExpertCollisionEvents` (0x40E6A0) runs:

```c
if (stricmp(event_name, "E:ALERTJUDGES") == 0) {
    // Iterate ALL judges in board+0x4BBC list
    for each judge in board+0x4BBC:
        Judge_Reset(judge);
}
```

### `Judge_Reset` (0x434C40)

```c
void Judge_Reset(this) {
    this->active = 0;                              // +0x10F4 = 0 (deactivate)
    
    // If not already in general render list, add it
    if (!AthenaList_Contains(board+0x2578, this)) {
        AthenaList_Append(board+0x2578, this);     // make visible
    }
}
```

This starts the countdown — the judge becomes visible and its display begins decaying.

## Update — `Gear_Update` (vtable[11] @ 0x434B60)

Runs every frame from `Scene_Update`:

```c
void Gear_Update(this) {
    if (this->active != 0) {                       // +0x10F4
        // Active/idle — just spin
        goto update_rotation;
    }
    
    // Inactive — counting down
    this->countdown--;                             // +0x10FC
    if (this->countdown > 0) {
        goto update_rotation;                      // still waiting
    }
    
    // Countdown finished — decay display
    if (this->display_value == 400) {              // +0x10F8
        // First decay step — play sound/effect at position
        play_effect(board->app->mgr, pos.x, pos.y, pos.z, 1.0);
    }
    
    this->countdown = 0;                           // reset timer
    this->display_value = (int)(this->display_value * 0.8);  // decay
    if (this->display_value < 1) {
        this->display_value = 0;                   // fully decayed
    }
    
update_rotation:
    this->rotation += 2.0;                         // +0x10E4 (always spins)
}
```

### Display Decay Sequence

Starting at 400, each step takes RNG(100–150) frames (~1.7–2.5 seconds at 60fps):

| Step | Value | Time |
|------|-------|------|
| 0 | 400 | Initial (plays sound) |
| 1 | 320 | ~2s |
| 2 | 256 | ~2s |
| 3 | 204 | ~2s |
| 4 | 163 | ~2s |
| 5 | 131 | ~2s |
| ... | ... | ... |
| ~27 | 0 | Fully decayed (~54s total) |

## Scoring — `E:SCORE` event

When a ball hits an `E:SCORE` trigger:

```c
if (strnicmp(event_name, "E:SCORE", 7) == 0) {
    long value = atol(event_name + 7);  // parse number after "E:SCORE"
    for each judge in board+0x4BBC:
        ScoreDisplay_SetTime(judge, value);
}
```

### `ScoreDisplay_SetTime` (0x434C80)

```c
void ScoreDisplay_SetTime(this, base_time) {
    float value = base_time + RNG(0, 2);           // add random 0-2
    
    switch (RNG(0, 5)) {
        case 0: value += 0.0; break;               // 20% no change
        case 1: value -= 0.0; break;               // 20% no change (DAT=0.0)
        case 2: value += 1.0; break;               // 20% +1.0
        default: break;                             // 40% no change
    }
    
    // Format as text
    if (value < 0 || value > 10) {
        sprintf(this->display_buffer, "%s", "");   // empty if out of range
    } else {
        sprintf(this->display_buffer, "%1.1f", value);
    }
    
    this->text_width = Font_MeasureText(this->display_buffer) / 2;
}
```

## Rendering — `Gear_Render` (vtable[18] @ 0x43A270)

```c
void Gear_Render(this) {
    // Apply scale
    Gfx_ScaleX(this->scale + 290.0);               // +0x10E0 + 290
    
    // Position with oscillation (bobbing animation)
    float bob = Wave_Sin(this->rotation) * 0.0;
    float y = this->pos_y - 20.0 - this->countdown + bob;
    Gfx_SetPosition(this->pos_x, y, this->pos_z);
    
    // Draw the hammyjudge mesh
    board->judge_mesh_1->vtable[0x1C]();           // render mesh
    
    // Draw floating 3D score text
    Font_DrawGlyph3D(
        board->app->font,                          // font resource
        this->display_buffer,                       // text ("%1.1f" formatted)
        pos_with_transforms,                        // position
        scale, 0, -1.0, 0, 1.0, 1.0, 0,            // orientation/scale
        ...
    );
}
```

The judge mesh is drawn at board+0x4BB0's MeshNode render function, which renders the pre-loaded `meshes\hammyjudge` mesh. The 3D text floats above showing the score value.

## Key Constants

| Address | Value | Purpose |
|---------|-------|---------|
| 0x4D5318 | 0.8 | Display value decay multiplier |
| 0x4CF48C | 2.0 | Rotation increment per frame |
| 0x4D5C80 | 290.0 | Base scale offset |
| 0x4CF370 | 20.0 | Y position offset (raises judge above ground) |
| 0x4CF9F8 | 10.0 | RNG range for scale |
| 0x4CF310 | 1.0 | Score modifier (add) |

## Board Fields

| Offset | Size | Description |
|--------|------|-------------|
| +0x4BB0 | 4 | Judge mesh 1 (MeshNode) |
| +0x4BB4 | 4 | Judge mesh 2 (MeshNode) |
| +0x4BB8 | 4 | Judge mesh 3 (MeshNode) |
| +0x4BBC | ~24 | AthenaList — Judge object list |
| +0x4FC8 | 4 | Internal AthenaList iteration array |

## ExpertCollisionEvents Events

The Expert board collision handler (0x40E6A0) processes these events:

| Event | Action |
|-------|--------|
| `E:CALLHAMMER` | CreateBonkPopup (Bonk appears) |
| `E:HAMMERCHASE` | Hammer_ChaseStart (Bonk chases) |
| `E:ALERTSAW1/2` | Saw_AlertActivate (Sawblade warning) |
| `E:ACTIVATESAW1/2` | Saw_Activate (Sawblade fires) |
| `E:ALERTJUDGES` | Judge_Reset on all judges (activate countdown) |
| `E:SCORE` | ScoreDisplay_SetTime on all judges (set score) |
| `E:JUMP` | Jump boost (Ball_DizzyImmunity +200) |
| `E:BELL` | Bell_Activate + extra time bonus |

## Notes

- The Expert race level uses `Level5.MESHWORLD` for its geometry
- Three judge meshes are pre-loaded but the number of judge objects depends on how many `N:JUDGE` refs exist in the mesh
- Judges are NOT difficulty-gated (unlike BONK which requires `App+0x23C != 0`)
- The judge continuously spins at +2.0 degrees/frame regardless of state
- The score display uses random ±modifiers to create slight variation between judges
- When `E:ALERTJUDGES` fires, all judges activate simultaneously and begin their countdown
- The display value decays by ×0.8 each step (400→320→256→...) over ~54 seconds total

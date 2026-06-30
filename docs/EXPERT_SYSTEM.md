# Expert Race Objects: Complete Reverse Engineering

## Overview

Expert Race (Level5) contains 6 unique object types. All can be spawned globally.

## Objects

| Object | String | String Addr | Alloc Size | Constructor | Ret | Vtable |
|--------|--------|-------------|------------|-------------|-----|--------|
| BONK (Hammer) | "BONK" | 0x4CFA4C | 0x1200 | 0x438850 | 0x10 | 0x4D5120 |
| FAN | "FAN" | 0x4CFA48 | 0x1188 | 0x438C20 | 0x14 | 0x4D5180 |
| SAWBLADE | "SAWBLADE" | 0x4CFA28 | 0x111C | 0x434660 | 0x10 | 0x4D5240 |
| BRIDGE | "BRIDGE" | 0x4CF678 | 0x10FC | 0x4396F0 | 0x14 | 0x4D51E0 |
| JUDGE | "JUDGE" | 0x4CFA14 | 0x1100 | 0x43A150 | 0x10 | 0x4D52B8 |
| BELL | "BELL" | 0x4CFA0C | 0x10E8 | 0x434D70 | 0x10 | 0x4D5330 |

## Factory Function

**Arena factory:** `CreateExpertLevelObjects` @ `0x40E250` (ret 0x10)
- Handles ALL 6 object types
- Called via Board vtable[33] at `0x40E250`
- Also handles post-creation events (E:CALLHAMMER, E:HAMMERCHASE, E:ALERTSAW1/2, etc.)

**Race factory:** @ `0x414BD0` (ret 0x10)
- Only handles FAN
- Falls through to `FACTORY_RACE_BASE` (0x4133E0) for unknown refs
- Appends FAN to BOTH Board+0x2578 (general) AND Board+0x47E0

## Constructor Calling Conventions

### BONK (Hammer) — 4 stack params, ret 0x10
```
ecx = this (alloc 0x1200)
push Board        ; param_1
sub esp, 0xC      ; pos XYZ (3 floats from refEntry+4)
call 0x438850     ; ret 0x10
```
- Reads Board+0x878 → App → App+0x174 (D3D device)
- Calls SceneObject_ctor(0x461740) with D3D device — creates geometry internally
- Sets vtable 0x4D5120
- Stores Board at obj+0x10D0
- After factory: stored at Board+0x436C, collision sub-object at obj+0x10F8

### FAN — 5 stack params, ret 0x14
```
ecx = this (alloc 0x1188)
push refEntry[0x14]  ; param_5 (ext_flag integer, NOT mesh)
sub esp, 0xC        ; pos XYZ
push Board           ; param_1
call 0x438C20        ; ret 0x14
```
- Reads Board+0x878 → App → App+0x174 (D3D device)
- Calls SceneObject_ctor(0x461740) with D3D device
- Sets vtable 0x4D5180
- Sets obj+0x10E8=0.5, obj+0x10EE=1, obj+0x10F0=500.0
- Factory post-checks: "SLOW" → obj+0x10EC=1, "SUPER" → obj+0x10ED=1, "UP" → call 0x434580
- 5th param (ext_flag) is read by ctor but only as a float for position adjustment

### SAWBLADE — 4 stack params, ret 0x10
```
ecx = this (alloc 0x111C)
push Board        ; param_1
sub esp, 0xC      ; pos XYZ
call 0x434660     ; ret 0x10
```
- Reads Board+0x878 → App → App+0x174 (D3D device)
- Sets vtable 0x4D5240
- After factory: "1" → stored at Board+0x4370, call 0x434AB0(obj, push 1)
- After factory: "2" → stored at Board+0x4374, call 0x434AB0(obj, push 2)

### BRIDGE — 5 stack params, ret 0x14
```
ecx = this (alloc 0x10FC)
push refEntry[0x14]  ; param_5 (ext_flag)
sub esp, 0xC        ; pos XYZ
push Board           ; param_1
call 0x4396F0        ; ret 0x14
```
- **READS Board+0x4378** (bridge mesh) — calls SceneObject_ctor(0x462850) with it
- Sets vtable 0x4D51E0
- After factory: "1" → Append(Board+0x4380, obj), "2" → Append(Board+0x4798, obj)
- After factory: "NEG" → obj+0x10F8 = -1.0f (0xBF800000)

### JUDGE — 4 stack params, ret 0x10
```
ecx = this (alloc 0x1100)
push Board        ; param_1
sub esp, 0xC      ; pos XYZ
call 0x43A150     ; ret 0x10
```
- Reads Board+0x878 → App → App+0x174 (D3D device)
- Sets vtable 0x4D52B8
- After factory: Append(Board+0x4BBC, obj)

### BELL — 4 stack params, ret 0x10
```
ecx = this (alloc 0x10E8)
push Board        ; param_1
sub esp, 0xC      ; pos XYZ
call 0x434D70     ; ret 0x10
```
- Reads Board+0x878 → App → App+0x174 (D3D device)
- Calls SceneObject_ctor(0x461740) with D3D device
- Sets vtable 0x4D5330
- After factory: stored at Board+0x4FD4, Append(Board+0x2578, obj)

## Mesh Loading

**LevelBoard_Expert_ctor** @ `0x41EA40` loads:
- `Board+0x4378` = `MeshWorld_ctor(0x10D0, App+0x174, "Levels\Level5-Bridge")` @ `0x4D0ABC`
- `Board+0x437C` = `CollisionLevel_ctorWithLevel(0x10D0, Board+0x4378)` @ `0x465080`
- `Board+0x4BB0/4BB4/4BB8` = 3× hammyjudge objects (0x18 bytes, ctor 0x471C20, mesh "meshes\hammyjudge")
- `Board+0x4344` = "Fight!" string ptr (0x4D0AA0)

**Pre-loaded by resource loader (0x4298C0):** NONE — all Expert meshes are level-specific.

## AthenaList Initialization

LevelBoard_Expert_ctor initializes:
- `Board+0x4380` (saw1 bridge list)
- `Board+0x4798` (saw2 bridge list)
- `Board+0x4BBC` (judge list)

On non-Expert levels, these are NOT initialized → must call `AthenaList_Init(0x453210)`.

`Board+0x2578` (general list) is initialized by Board_ctor on ALL levels.

## Collision Events

**TowerCollisionEvents** @ `0x40E6A0` (Expert Board vtable[29] at 0x74):

| Event | String Addr | Handler | Action |
|-------|-------------|---------|--------|
| E:CALLHAMMER | 0x4CFAC8 | 0x438B30 | Calls Board+0x436C (BONK) to chase ball |
| E:HAMMERCHASE | 0x4CFAB8 | 0x438BB0 | Activates hammer chase on Board+0x436C |
| E:ALERTSAW1 | 0x4CFAAC | 0x434770 | Alerts Board+0x4370 (SAW1) |
| E:ALERTSAW2 | 0x4CFAA0 | 0x434770 | Alerts Board+0x4374 (SAW2) |
| E:ACTIVATESAW1 | 0x4CFA90 | 0x434A50 | Activates Board+0x4370 (SAW1) |
| E:ACTIVATESAW2 | 0x4CFA80 | 0x434A50 | Activates Board+0x4374 (SAW2) |
| E:ALERTJUDGES | 0x4CFA70 | iterates Board+0x4BBC | Cycles through judge list |
| E:SCORE | 0x4CFA68 | creates 3D sound | Sets score, plays sound |
| E:JUMP | 0x4CF890 | creates 3D sound | Jump pad effect |

**On non-Expert levels:** Objects have physical collision (via internal CollisionLevel) but special events won't fire because the collision handler is level-specific.

## Object Updates

Each object's vtable contains update/render functions:
- vtable[0x54] = render function (called each frame)
- vtable[0x58] = update function (called each frame)
- vtable[0x60] = collision response

## Global Spawn Requirements

### No mesh needed (create from D3D device):
- BONK, FAN, SAWBLADE, JUDGE, BELL
- Just need: alloc + Board + position

### Mesh needed:
- BRIDGE: requires Board+0x4378 = MeshWorld("Levels\Level5-Bridge")
  - Load via: `MeshWorld_ctor(0x10D0, App+0x174, "Levels\Level5-Bridge")` @ 0x461510
  - String at 0x4D0ABC

### AthenaList_Init required:
- Board+0x4380 (for BRIDGE "1")
- Board+0x4798 (for BRIDGE "2")
- Board+0x4BBC (for JUDGE)

### Post-spawn Board field writes:
- BONK → Board+0x436C
- SAWBLADE → Board+0x4370 (saw1) or Board+0x4374 (saw2)
- BELL → Board+0x4FD4

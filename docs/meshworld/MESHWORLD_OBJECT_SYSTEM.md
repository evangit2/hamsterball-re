# MESHWORLD File Format & Object System — Definitive Reference

**Verified against**: Binary .MESHWORLD files, official Raptisoft exporter source (MeshWorldExport.cpp, WorldMesh.cpp, NodeEnumerator.cpp), and Ghidra decompilations of Hamsterball.exe.

## 1. Two File Formats

Hamsterball uses **two** mesh file formats:

### 1.1 .MESHWORLD (Binary) — Level/world files
- Written by the official Raptisoft 3DS Max exporter plugin (`MeshWorldExport.cpp`)
- Loaded by the game engine via binary file I/O (`__read` calls)
- Contains: ref points, splines, lights, colors, vertex buffer, octree mesh
- File extension: `.MESHWORLD` (e.g., `Level1.MESHWORLD`, `Arena-WarmUp.MESHWORLD`)
- 37 level files + 15 arena files + sub-level files (bridges, etc.)

### 1.2 .ASE (Text) — Individual mesh objects
- 3DS Max ASCII Scene Export format
- Parsed by `MeshWorld_Parse` (0x470930) — line-by-line text parser
- Keywords: `*MATERIAL`, `*GEOMOBJECT`, `*MESH_VERTEX`, `*MESH_FACE`, etc.
- Used for loading individual mesh objects (not entire levels)
- Material struct: 0x50 bytes per material (ambient/diffuse/specular/shine/texture)

## 2. Binary .MESHWORLD Format (6 Sections)

### Section 1: Ref Points (Game Objects)
```
[int32]   point_count
FOR each point:
  [int32,len][string]  name          (length-prefixed, NUL-terminated)
  [float]  position.x                 (NOTE: x,z,y order — Max Z-up → engine Y-up)
  [float]  position.z
  [float]  position.y
  [float]  rotation.x                 (yaw)
  [float]  rotation.z
  [float]  rotation.y                 (roll)
  [int32]  has_material               (0=no material, 1=has material)
  IF has_material:
    [float×4]  Ambient   (r,g,b,a)
    [float×4]  Diffuse   (r,g,b,a)
    [float×4]  Specular  (r,g,b,a)
    [float×4]  Emissive   (r,g,b,a)
    [float]    Power                (shininess)
    [int32]    has_reflection
    [int32]    has_texture          (0 or 1)
    IF has_texture:
      [int32,len][string] texture_filename
```
Ref points are the **game logic objects** — spawn points, triggers, factory objects.

### Section 2: Splines (Path Objects)
```
[int32]   spline_count
FOR each spline:
  [int32,len][string]  name
  [int32]   point_count
  FOR each point:
    [float]  x        (x,z,y order)
    [float]  z
    [float]  y
```
Splines are used for BlockDawg patrol paths (`DAWGPATH1`, `DAWGPATH2`, `DAWGPATH3`).

### Section 3: Lights
```
[int32]   light_count
FOR each light:
  [int32]   type                    (0 = DISTANTLIGHT)
  IF type == 0:
    [float×3] position (x,z,y)
    [float×3] lookat   (x,z,y)
    [float×3] color     (r,g,b)
```

### Section 4: Background & Ambient Colors
```
[float×3] background_color (r,g,b)
[float×3] ambient_color    (r,g,b)
```

### Section 5: Global Vertex Buffer
```
[int32]     vertex_count
[Vertex×N]  vertex_list     (32 bytes per vertex)
```
**Vertex struct (32 bytes):**
```c
struct Vertex {
    float mX, mY, mZ;              // Position (12 bytes)
    float mNormalX, mNormalY, mNormalZ;  // Normal (12 bytes)
    float mTextureU, mTextureV;    // UV coords (8 bytes)
};
```

### Section 6: Octree Mesh Dump (Recursive)
```
[Cube]  bounding_box          // 6 floats: corner1(x,y,z) + corner2(x,y,z)
[int32] submesh_count
IF submesh_count > 0:
  FOR each submesh:
    [recursive Section 6]       // child cube
ELSE (leaf node):
  [int32] geom_count           // number of geometry objects
  FOR each geom:
    [int32,len][string] name  // "" unless name[1]==':' OR contains "NOCOLLIDE"
    [float×4] Ambient
    [float×4] Diffuse
    [float×4] Specular
    [float×4] Emissive
    [float]   Power
    [int32]   has_reflection
    [int32]   has_texture
    IF has_texture:
      [int32,len][string] texture_filename
    [int32]   strip_count
    FOR each strip:
      [int32] triangle_count
      [int32] vertex_ref_offset   // offset into global vertex buffer
```

**Key exporter rule** (WorldMesh.cpp:291): A geom name is only written if:
- `name[1] == ':'` (any `X:` prefix), OR
- The name contains `"NOCOLLIDE"`
Otherwise, an empty string `""` is written (unnamed geometry).

## 3. Object Name Prefix System

### 3.1 Ref Point Names (Section 1) — Game Logic Objects
Parsed into a hash table during scene construction. Looked up by name during `Scene_SpawnBallsAndObjects` (0x41C5B0) and `CreateLevelObjects` (0x4121D0).

| Category | Examples | Handler |
|-----------|----------|---------|
| Spawn points | `START1-1`, `START2-1`, `START2-2`, `START-DEBUG` | `Scene_SpawnBallsAndObjects` — ball placement |
| Checkpoints | `SAFESPOT`, `SAFEPOS`, `SAFESPOT(A)` | Appended to `scene->safespots` list |
| Race flags | `FLAG02`–`FLAG18` | `Scene_CreateFlags` — checkpoint flags |
| Enemy balls | `BADBALL <CHASE>100</CHASE><HOME>400</HOME>` | `CreateBadBall` — XML-parameterized AI ball |
| Factory objects | `BRIDGE`, `TIPPER`, `BONK`, `BBRIDGE1`, `BBRIDGE2`, `POPCYLINDER`, `BLOCKDAWG1/2`, `CATAPULT`, `GLUEBIE` | `CreateLevelObjects` (0x4121D0) — prefix-dispatched |
| Arena objects | `BONK`, `TIP`, `SAWBLADE`, `BRIDGE`, `JUDGE`, `BELL` | `CreateExpertLevelObjects` (0x40E250) — arena sub-factory |
| Bumpers | `BUMPER1`–`BUMPER8` | `CreateBumper` (0x40FA20) — loads `levels\level8` |
| Camera | `CAMERALOOKAT` | Camera target point (present in all 15 race levels) |
| Secrets | `SECRET`, `SECRETUNLOCK`, `N:SECRET` | `CreateSecretObjects` — hidden collectibles |
| Signs | `SIGN-TARPIT` | `Scene_CreateSigns` |
| Splines | `DAWGPATH1`, `DAWGPATH2`, `DAWGPATH3` | `Level_FindObjectByName` → BlockDawg patrol path |
| Misc refs | `FAN`, `FAN(UP)`, `FAN(SUPER)(UP)`, `GEAR01`–`GEAR24`, `BIGGEAR01`–`BIGGEAR14`, `TURRET`, `DRAWBRIDGE`, etc. | Various level-specific handlers |

### 3.2 Octree Geom Names (Section 6) — Collision & Render Objects

Parsed by the binary MESHWORLD loader (function at ~0x461680) into MeshBuffer objects. The loader checks geom name prefixes and sets **render classification flags** on the MeshBuffer struct:

```c
// Binary MESHWORLD loader (~0x461680) — checks ALL prefixes:
if (name[1] == ':')                 obj->has_named_prefix = 1;     // +0x85C
if (strnicmp(name, "O:", 2) == 0)   obj->is_translucent = 1;       // +0x862
if (strnicmp(name, "T:", 2) == 0)   obj->is_decal = 1;             // +0x85F
if (strnicmp(name, "N:GLASS", 7)==0)obj->is_alpha_test = 1;        // +0x860
if (strnicmp(name, "E:", 2) == 0)   obj->no_render = 1;            // +0x863
if (strstr(name, "(NOSHADOW)"))     obj->no_shadow = 1;            // +0x85E

// Text/.COL loader (Level_LoadCollision 0x465260) — checks N: and E: only:
if (strnicmp(name, "N:", 2) == 0)   buf->interactive = 1;          // +0x85D
if (strnicmp(name, "E:", 2) == 0) { buf->interactive = 1; buf->no_render = 1; }  // +0x85D, +0x863
```

These flags control how `Scene_RenderAllObjects` (0x45E0E0) classifies each geom into render passes:
- **Opaque pass** (default): AlphaBlend OFF, AlphaTest OFF
- **Translucent pass** (`+0x862`): AlphaBlend ON — objects behind transparent parts are still visible. This is why the ball renders correctly behind `O:` tubes.
- **Alpha test pass** (`+0x860`): Sorted into translucent bucket with per-pixel alpha cutoff
- **Decal pass** (`+0x85F`): Stencil-based, with depth bias
- **Skip** (`+0x863`): Not rendered at all (invisible collision zones)
- **No shadow** (`+0x85E`): Excluded from shadow rendering pass

**Verified by user testing**: Changing `O:` to `A:` in a level file causes the ball to stop rendering behind tubes, confirming `O:` directly sets the translucent render flag (`+0x862`).

| Prefix | Count | Engine Flag | Offset | Render Pass | Behavior |
|--------|-------|-------------|--------|-------------|----------|
| `N:` | 278 | interactive | +0x85D | Opaque | Named collision, triggers event handler on hit (set by text loader) |
| `E:` | 581 | no_render | +0x863 | Skip (invisible) | Invisible collision zone, triggers event on hit |
| `O:` | 60 | is_translucent | +0x862 | Translucent (alpha blend) | See-through render — ball visible behind transparent parts (tubes, saws) |
| `T:` | 175 | is_decal | +0x85F | Decal (stencil + depth bias) | Texture overlays: arrows, warnings, bullseyes |
| `N:GLASS` | — | is_alpha_test | +0x860 | Alpha test (sorted into translucent) | Alpha-tested transparency (glass platforms) |
| `(NOSHADOW)` | — | no_shadow | +0x85E | Excluded from shadow pass | Designer tag in name, checked via `strstr` |
| `S:` | 831 | *(none specific)* | — | Opaque (standard) | Standard collision + standard render. `(NOSHADOW)` suffix in name sets +0x85E. |
| (none) | — | *(none)* | — | Opaque (standard) | Standard level geometry (walls, floors) |

### 3.3 Complete N: Object Catalog (44 unique types)
| Name | Triangles | Texture | Files | Behavior |
|------|-----------|---------|-------|----------|
| `N:BOUNCE(NOSHADOW)` | 38 | — | 1 | Bouncy surface (no shadow) |
| `N:BRIDGE` | 244 | — | 1 | Bridge segment (movable) |
| `N:BUMP` | 100 | — | 1 | Bump/perturbation surface |
| `N:BUMPER1`–`N:BUMPER8` | 24-156 | — | 2-4 | Arena bumpers |
| `N:DROPIN` | 2 | — | 1 | Pipe drop-in trigger |
| `N:EXTRATIME` | 8 | FiveBonus.png | 1 | Extra time collectible |
| `N:GLASS` | 32 | — | 2 | Glass platform (breakable?) |
| `N:GOAL` | 8 | GreyOutlineChecker.png | 15 | Race finish line |
| `N:JUMPFIRST` / `N:JUMPSECOND` | 2 | — | 1 | Jump pad (sequential) |
| `N:LOOPER(NOSHADOW)` | 54 | RedChecker.bmp | 1 | Loop-the-loop section |
| `N:MOUSETRAP` | 2-6 | TrapTop/TrapSpring/TrapMetal.jpg | 1 | Mouse trap obstacle |
| `N:NEONPLATFORM` | 18 | — | 1 | Neon-lit platform |
| `N:NOCONTROL` | 2 | GreenChecker.bmp | 1 | Disable ball input zone |
| `N:ONGEAR(NOSHADOW)` | 16 | — | 1 | Gear surface (rides on gear) |
| `N:ONPENDULUM` | 26 | RedChecker.bmp | 1 | Pendulum surface |
| `N:ONROTATOR` | 8 | RedChecker.bmp | 1 | Rotator surface |
| `N:SAWTEETH` | 4 | — | 1 | Saw teeth hazard |
| `N:SECRET` / `N:SECRET(NOSHADOW)` | 6-12 | GreyOutlineChecker.png | 1 | Hidden collectible |
| `N:SINKPLATFORM` | 10 | PinkChecker.bmp | 1 | Sinking platform |
| `N:SPEEDCYLINDER` | 17 | SpeedCylinder.png | 1 | Speed boost cylinder |
| `N:SPINNY` | 62 | — | 1 | Spinning platform |
| `N:SQUAREWOBBLY` / `(NOSHADOW)` | 2-336 | BrightGreenChecker.bmp | 1-8 | Wobbly square platform |
| `N:SWIRL` | 158 | — | 1 | Swirl/vortex section |
| `N:TARPIT` | 8 | — | 2 | Tar pit (slows ball) |
| `N:TENBONUS1` / `N:TENBONUS2` | 2 | — | 1 | +10 bonus collectible |
| `N:TRAPDOOR` | 2 | YelllowChecker.png | 2 | Trapdoor (opens on trigger) |
| `N:UNLOCKSECRET` | 2 | LockTile.png | 12 | Secret unlock trigger |
| `N:WATERWHEEL(NOSHADOW)` | 144 | — | 1 | Water wheel obstacle |
| `N:WAVY` | 168 | BrightGreenChecker.bmp | 1 | Wavy platform |
| `N:WHEELEMBED` | 6 | — | 1 | Embedded wheel section |

### 3.4 Complete E: Event Catalog (86 unique types)
| Name | Behavior |
|------|----------|
| `E:LIMIT` | Arena boundary / fall-off zone (323 instances — most common) |
| `E:DROPIN` | Pipe drop-in sound + score (28 instances, 12 files) |
| `E:POPOUT` | Pipe pop-out sound + score (30 instances, 11 files) |
| `E:PIPEBONK` | Pipe collision sound (random of 3) (22 instances, 4 files) |
| `E:SAFESWITCH(A)`–`(H)` | Track which checkpoint/safe zone was hit |
| `E:NODIZZY<TIME>N</TIME>` | dizzy immunity zone: grants 50-600 frames of dizzy protection |
| `E:JUMP` | Jump pad: upward velocity + sound + score |
| `E:CATAPULTBOTTOM` | Launch catapult |
| `E:OPENSESAME` | Open first trapdoor |
| `E:BITE` | Damage: 25.0 |
| `E:MACETRIGGER` | Activate all maces |
| `E:CALLHAMMER` | Spawn hammer chase (arena MP) |
| `E:HAMMERCHASE` | Start hammer chase sequence |
| `E:ALERTSAW1/2` | Pre-activate saw blade (warning) |
| `E:ACTIVATESAW1/2` | Full activate saw blade |
| `E:ALERTJUDGES` | Reset all judge objects |
| `E:SCORE1/5/7/9` | Set score display time |
| `E:BELL` | Extra time +5s + "EXTRA TIME:" popup |
| `E:GRAVITY<TYPE>X/Z/NORMAL</TYPE>` | Change gravity direction |
| `E:TRAJECTORY<X>..</X><Y>..</Y><Z>..</Z>` | Set ball trajectory |
| `E:ACTION<ONCE>TRUE</ONCE><SCORE>N</SCORE>` | Score action (one-time or repeatable) |
| `E:LAUNCH` | Launch ball |
| `E:SHRINK` | Shrink ball |
| `E:SWALLOW` | Swallow/absorb ball |
| `E:TRAPPOP` | Trapdoor pop |
| `E:VACPOPOUT` | Vacuum pop-out |
| `E:ZOOP` | Speed boost |
| `E:HEATON` / `E:HEATOFF` | Heat effect on/off |
| `E:LIGHTSON` / `E:LIGHTSOFF` | Lights on/off |
| `E:HELPINERTIA` / `E:UNHELPINERTIA` | Assist inertia on/off |
| `E:PEGS` / `E:NOPEGS` | Toggle pegs |
| `E:GROWSOUND` | Grow sound effect |
| `E:BRANCH(A)` / `E:BRANCH(B)` | Branch path selection |
| `E:DROPLIFT` | Drop lift |
| `E:LIMITPIPE1/2` | Pipe limit zones |
| `E:LIMITX` / `E:LIMITZ` | Axis-specific limit zones |

## 4. How Objects Interact with the EXE

### 4.1 Loading Pipeline
```
1. App selects level → MeshWorld_ctor(gfx) creates MeshWorld struct (0x488 bytes)
2. Level_LoadCollision (0x465260) reads binary .MESHWORLD:
   a. Reads header (24 bytes into MeshWorld+0x45C)
   b. Reads sublevel_count
   c. If single-level: reads objects with named collision faces
   d. If multi-level: creates sub-levels recursively
3. Each named collision object → CreateMeshBuffer (0x874 bytes)
   - Name stored at +0x864 (char*)
   - N: prefix → +0x85D = 1 (interactive)
   - E: prefix → +0x85D = 1, +0x863 = 1 (interactive + no_render)
4. Scene construction:
   a. Scene_SpawnBallsAndObjects (0x41C5B0) — reads ref points by name
   b. CreateLevelObjects (0x4121D0) — factory dispatches by ref point name prefix
   c. CreateExpertLevelObjects (0x40E250) — arena object sub-factory
   d. CreateBumper (0x40FA20) — bumper loader
   e. CreateMouseTrap (0x40BF50) — mouse trap spawner
5. Per-frame: Ball_AdvancePositionOrCollision checks CollisionLevel
   → Mesh_FindClosestCollision (0x465D90) raycasts against named faces
   → On hit: TowerCollisionEvents or ExpertCollisionEvents dispatches by name
```

### 4.2 Collision Dispatch Chain
```
Ball physics update
  → Mesh_FindClosestCollision (0x465D90) — raycast against CollisionLevel
    → If hit named collision object:
      → TowerCollisionEvents (0x40DCD0) [Tower board]
         OR ExpertCollisionEvents (0x40E6A0) [arenas]
        → __stricmp / __strnicmp on object name (at collider+0x864)
        → Dispatch to specific handler (Catapult_Launch, Trapdoor_Open, etc.)
        → Always ends with: DispatchCollisionEvents (0x40C5D0) — base event handler
```

### 4.3 DispatchCollisionEvents — The Base Event Handler (0x40C5D0)
Handles ALL events not consumed by level/arena-specific handlers. Uses `__strnicmp`/`__stricmp` on the event name string:

- **N:SECRET** → `Rotator_MarkTriggered`
- **N:UNLOCKSECRET** → `CheckArenaUnlock`
- **E:NODIZZY\<TIME\>N\</TIME\>** → Parse XML tags, `Ball_DizzyImmunity`
- **E:SAFESWITCH(data)** → Copy parenthesized data to ball state
- **E:LIMIT** → Arena fall-off: increment other players' completion counts
- **E:BREAK** → Call `ball->vtable[0x20]()` (bounce)
- **E:JUMP** → Sound + upward velocity + score
- **E:ACTION\<ONCE\>..\<SCORE\>..\</SCORE\>** → Parse XML, award points
- **E:TRAJECTORY(X,Y,Z)** → Parse XML tags, set ball trajectory
- **N:NOCONTROL** → Disable input for 10 frames
- **N:WATER** → Set water flag + 10-frame counter
- **N:TARPIT** → Slow ball + tar sound
- **E:DROPIN** → Drop-in sound + score (distance-gated)
- **E:PIPEBONK** → Random pipe bonk sound (1 of 3)
- **E:POPOUT** → Pop-out sound + score
- **N:GOAL** → Finish race! Play music, store time
- **N:MOUSETRAP** → Deflect ball, track rotator collision

### 4.4 Factory Dispatch (CreateLevelObjects 0x4121D0)
Ref point names are matched via `__strnicmp` to instantiate game objects:

| Prefix | Match Length | Constructor | Size | Scene Offset |
|--------|-------------|-------------|------|-------------|
| `BRIDGE` | 6 | (configures existing mesh) | — | +0x436C |
| `TIPPER` | 6 | `Tipper_ctor` + `TipperVisual_ctor` | 0x1104 + 0x10D0 | +0x2578 |
| `BONK` | 4 | `Bonk_ctor` | 0x1200 | +0x2578, +0x540C |
| `BBRIDGE1` | 8 | `BreakBridge_ctor` | 0x1100 | +0x5418 |
| `BBRIDGE2` | 8 | `BreakBridge_ctor` | 0x1100 | +0x541C |
| `POPCYLINDER` | 11 | `PopCylinder_ctor` | 0x10E8 | +0x5428 |
| `BLOCKDAWG1` | 10 | `Blockdawg_ctor` | 0x1154 | +0x2578 |
| `BLOCKDAWG2` | 10 | `Blockdawg_ctor` | 0x1154 | +0x2578 |
| `CATAPULT` | 8 | `Catapult_ctor` | 0x1108 | +0x584C |
| `GLUEBIE` | 7 | `Gluebie_ctor` | 0x110C | +0x6080 |

### 4.5 Arena Sub-Factory (CreateExpertLevelObjects 0x40E250)
Despite the name, this handles 6 arena object types:

| Prefix | Match Length | Constructor | Size | Scene Offset |
|--------|-------------|-------------|------|-------------|
| `BONK` | 4 | `Bonk_ctor` | 0x1200 | +0x436C |
| `TIP` | 3 | `TowerLevel_Ctor` | 0x1188 | +0x2578 |
| `SAWBLADE` | 8 | `Sawblade_Level_Ctor` | 0x111C | +0x4370/4374 |
| `BRIDGE` | 6 | `Spinner_Level_ctor` | 0x10FC | +0x4380/4798 |
| `JUDGE` | 5 | `Gear_Level_ctor` | 0x1100 | +0x4BBC |
| `BELL` | 4 | `Tipper_Level_Ctor` | 0x10E8 | +0x4FD4 |

Modifiers via `strstr`: `SLOW`, `SUPER`, `UP` (TowerLevel), `1`/`2` (Sawblade/Spinner), `NEG` (Spinner direction).

## 5. BADBALL — Parameterized Enemy Ball

BADBALL ref points use XML-style tags for parameters:
```
BADBALL <CHASE>100</CHASE><HOME>400</HOME><SIZE>18</SIZE><SPINDISTANCE>25</SPINDISTANCE>
```

| Tag | Meaning | Example |
|-----|---------|---------|
| `<CHASE>` | Chase distance (when to start pursuing) | 100-300 |
| `<HOME>` | Home distance (return-to-home radius) | 100-1000 |
| `<SIZE>` | Ball radius | 18 |
| `<SPINDISTANCE>` | Spin activation distance | 25-45 |

## 6. Render Pipeline Integration

Objects are classified into render buckets by flags on SceneObject:
- `+0x863`: has_bounding_sphere (skip if set)
- `+0x862`: is_translucent (alpha blend)
- `+0x85F`: is_decal (stencil-based)
- `+0x860`: is_alpha_test (sorted into translucent)
- `+0x861`: has_per_object_alpha

**Scene_RenderAllObjects** (0x45E0E0) — 3-pass render:
1. **Opaque pass**: AlphaBlend OFF, AlphaTest OFF
2. **Translucent pass**: AlphaBlend ON, AlphaTest OFF
3. **Decal pass**: Stencil + depth bias

Render bucket classification is driven by **name prefix flags** set during MESHWORLD loading (see §3.2 above), NOT by material properties. The binary loader at ~0x461680 checks each prefix and sets the corresponding flag byte on the MeshBuffer struct before rendering.

| Flag | Offset | Set by | Render effect |
|------|--------|--------|---------------|
| has_named_prefix | +0x85C | `name[1]==':'` | Name was preserved in file |
| no_shadow | +0x85E | `(NOSHADOW)` substring | Excluded from shadow pass |
| is_decal | +0x85F | `T:` prefix | Stencil + depth bias render pass |
| is_alpha_test | +0x860 | `N:GLASS` prefix | Sorted into translucent with alpha cutoff |
| is_translucent | +0x862 | `O:` prefix | Alpha blend ON — see-through rendering |
| no_render | +0x863 | `E:` prefix | Skipped entirely (invisible) |

## 7. String Format Convention

All strings in .MESHWORLD files are length-prefixed:
```
[int32]   length (including NUL terminator)
[length bytes]  string data (NUL-terminated)
```
Empty string = length 1, single `\x00` byte.

## References
- Official exporter: `reference/raptisoft-exporter/MeshWorldExport/`
- Binary loader: `Level_LoadCollision` at 0x465260 (Ghidra decompilation)
- ASE parser: `MeshWorld_Parse` at 0x470930 (Ghidra decompilation)
- Collision dispatch: `TowerCollisionEvents` at 0x40DCD0, `ExpertCollisionEvents` at 0x40E6A0
- Base event handler: `DispatchCollisionEvents` at 0x40C5D0
- Object factory: `CreateLevelObjects` at 0x4121D0, `CreateExpertLevelObjects` at 0x40E250

# MESHWORLD Technical Specification — For Tool Builders

**Purpose:** Raw binary spec for building custom .MESHWORLD files from scratch.
No Blender, no external tools. Just you, bytes, and code.

---

## 1. FILE FORMAT

Binary. Little-endian. 6 sections, written sequentially.

### Section 1: Ref Points
```
Offset  Size    Field
0       4       point_count (uint32) = 0
```
Not used for spawned meshes. Write 0.

### Section 2: Splines
```
Offset  Size    Field
0       4       spline_count (uint32) = 0
```

### Section 3: Lights
```
Offset  Size    Field
0       4       light_count (uint32) = 0
```

### Section 4: Global Info
```
Offset  Size    Field
0       24      6 floats (all 0.0): unknown, write zeros
```

### Section 5: Global Vertex Buffer
```
Offset  Size    Field
0       4       vertex_count (uint32)
4       32*N    N vertices, each 32 bytes:
                  [float] x      -- position X
                  [float] y      -- position Y (UP in game)
                  [float] z      -- position Z (FORWARD in game)
                  [float] nx     -- normal X
                  [float] ny     -- normal Y
                  [float] nz     -- normal Z
                  [float] u      -- texture U (0.0 or 1.0)
                  [float] v      -- texture V (0.0 or 1.0)
```

**Coordinate system:**
- X = left/right (width)
- Y = up/down (height)
- Z = forward/back (depth)
- This is a **left-handed** system (D3D convention)
- Write coordinates directly — NO axis swapping

**Normals:** Required per-vertex. The game does NOT auto-calculate them.
Must point outward from the surface. Wrong normals = dark rendering + broken collision.

**UV:** Set to 0.0 or 1.0. Doesn't matter when has_texture=0.

### Section 6: Octree (Materials + Strips)

The octree is a recursive tree. For spawned meshes, use a **flat single leaf**:

```
Offset  Size    Field
0       24      bounds: [float]min_x, min_y, min_z, max_x, max_y, max_z
24      4       submesh_count (uint32) = 0  (0 = leaf node)
28      4       has_data (uint32) = 1
32      4       geom_count (uint32)

FOR each geom:
  [uint32] name_length (includes NUL terminator)
  [chars]  name (NUL-terminated string)
  [float4] ambient RGBA (4 floats, 0.0-1.0 each)
  [float4] diffuse RGBA
  [float4] specular RGBA
  [float4] emissive RGBA
  [float]  power (shininess, 10.0 is fine)
  [uint32] has_reflection = 0
  [uint32] has_texture = 0  (MUST be 0 — no textures for spawned meshes)
  [uint32] strip_count
  FOR each strip:
    [uint32] triangle_count = 1  (always 1 for individual triangles)
    [uint32] vertex_ref = index into global vertex buffer (first vertex of triangle)
```

**If has_texture=1** (DON'T DO THIS for spawned meshes):
After has_texture, add:
```
[uint32] texture_filename_length (includes NUL)
[chars]  texture_filename (NUL-terminated)
```
Setting has_texture=1 with a filename that doesn't exist causes "TEXTURE LOAD FAILED!" popup.
Use has_texture=0 and material diffuse colors instead.

#### Full Material Block (per geom)
Total bytes per material: 16+16+16+16+4+4+4 = **76 bytes**

```
Ambient:  4 floats (R,G,B,A) — shadow color, typically diffuse × 0.7-0.8
Diffuse:  4 floats (R,G,B,A) — main color, what the surface looks like
Specular: 4 floats (R,G,B,A) — highlight, typically (0.1, 0.1, 0.1, 1.0)
Emissive: 4 floats (R,G,B,A) — self-illumination, typically (0, 0, 0, 1.0)
Power:    1 float — shininess exponent (10.0 is fine)
has_refl: 1 uint32 — 0 = no
has_tex:  1 uint32 — 0 = no texture, use material colors
```

#### Geom Names

| Name (bytes) | Visible | Collision | Notes |
|---|---|---|---|
| `\x00` (1 byte NUL) | Yes | Yes | Normal solid surface |
| `E:DROPIN\x00` | No | Events only | Pipe entry sound + dizzy immunity |
| `E:POPOUT\x00` | No | Events only | Pipe exit sound + dizzy immunity |
| `E:PIPEBONK\x00` | No | Events only | Ball hitting pipe walls |
| `E:ZOOP\x00` | No | Events only | Whoosh sound |
| `E:JUMP\x00` | No | Events only | Jump pad (upward velocity + sound) |
| `E:BREAK\x00` | No | Events only | Shatters the ball |
| `E:ACTION\x00` | No | Events only | Custom action (SCORE, ONCE tags) |
| `E:TRAJECTORY\x00` | No | Events only | Set ball trajectory |
| `E:NODIZZY\x00` | No | Events only | Anti-dizzy zone |
| `N:GOAL\x00` | No | Events only | Race finish line |
| `N:TARPIT\x00` | No | Events only | Tar pit (slows ball) |
| `N:WATER\x00` | No | Events only | Water zone |
| `N:BOUNCE\x00` | No | Events only | Bounce pad |
| `N:MOUSETRAP\x00` | No | Events only | Rotator sound |
| `(NOCOLLIDE)\x00` | Yes | No collision | Visible but ball passes through |

**name_length includes the NUL terminator.** For empty name: length=1, data=`\x00`.
For `E:DROPIN\x00`: length=9, data=`E:DROPIN\x00`.

---

## 2. GEOMETRY REQUIREMENTS

### Triangle Winding
**Counter-clockwise (CCW)** when viewed from outside (front-facing).

For a box face viewed from outside:
```
v0 --- v1
 |  /  |
v3 --- v2

Tri 1: v0, v1, v2  (CCW)
Tri 2: v0, v2, v3  (CCW)
```

Wrong winding = faces point inward = invisible (backface culling) + collision broken (physics sees backface, ball passes through or gets stuck).

### What Causes Ball to Get Stuck / Fall Through

| Cause | Effect | Fix |
|---|---|---|
| Inverted normals | Faces point inward, collision reversed | Ensure normals point outward |
| Wrong winding order | Same as inverted normals | Use CCW from outside |
| Zero-thickness planes | Ball falls through | Minimum thickness = 20 units |
| Too-thin geometry (<20 units) | Ball tunnels through at speed | Use 20+ unit thickness |
| Mirroring Z without winding fix | All faces flip inside-out | Swap v1↔v2 + negate normals |
| Floating-point rotation noise | Sub-pixel vertex misalignment | Use exact integer swaps for 90°/180°/270° |
| Concave shapes with thin walls | Ball catches on internal edges | Keep walls thick (20+ units) |

### Minimum Platform Thickness
**20 units minimum.** Anything thinner and the ball can tunnel through at high speed.
The ball's collision radius is 26 units, so the ball center can be 26 units inside a surface
before collision registers. A 20-unit-thick wall ensures the ball can't pass through in one frame.

### Convex vs Concave
No strict requirement. Both work. However:
- **Concave shapes with thin walls** can cause the ball to catch on internal edges
- **Convex shapes** (boxes, cylinders) are the safest
- For complex shapes (pipes, L-junctions), overlap wall geometry at joints to seal gaps

### Triangle Count
- **Per MeshBuffer (per geom):** 65,534 vertices max (~21,844 triangles)
  - Enforced by `MeshBuffer_Allocate` (0x00480CB4)
  - Uses D3DFMT_INDEX16 (uint16 vertex indices, max 65535, capped at 65534)
  - Split large meshes into multiple geoms to exceed this
- **Per file:** No hard limit, but keep total under ~50,000 vertices for performance
- **Per strip:** `triangle_count` field. Always use 1 (one triangle per strip) for simplicity.
  The game supports multi-triangle strips but individual triangles are safest.

---

## 3. SCALE AND UNITS

### Coordinate System
- **Left-handed** (Direct3D convention)
- X = right, Y = up, Z = forward
- Units are **game units** (no real-world meter conversion)

### Ball Dimensions
| Property | Value (game units) |
|---|---|
| Ball radius | 26 |
| Ball diameter | 52 |
| 8-ball radius | 35 (set by Ball_InitPhysicsDefaults, not player ball) |
| Normal ball collision sphere | 26 units from center |

### Practical Scale Reference
| Object | Size (units) |
|---|---|
| Standard platform tile | 50 × 50 |
| Pipe inner diameter (min safe) | 60 (ball diam 52 + 4 clearance per side) |
| Pipe wall thickness | 20 |
| Staircase step height | 15 (easy climbing, < ball radius) |
| Standard platform thickness | 20 |
| Player spawn Y offset | -20 (spawn below ball center) |

### Size Limits
- **Per vertex:** No limit on coordinate values (float)
- **Per geom:** 65,534 vertices (INDEX16 limit)
- **Per file:** No hard limit, performance-dependent
- **Bounds field:** Must encompass all vertices (used for octree culling)

---

## 4. COLLISION SYSTEM

### Single-Mesh or Separate Collision?
**Single-mesh.** The game does NOT use separate collision geometry.
The same mesh triangles are used for both rendering and collision.

The collision system is built by `Fallout_ctor` (0x0043BBC0), which:
1. Takes the loaded MeshWorld data
2. Creates a collision tree from the triangles
3. Registers it with the board's collision lists

### What Makes a Triangle Collidable?
1. The geom name is **NOT** prefixed with `(NOCOLLIDE)`
2. The geom name is **NOT** prefixed with `E:` or `N:` (those are triggers, not solid)
3. The triangle has correct winding (CCW from outside)
4. The triangle has correct normals (pointing outward)
5. The geom is registered in collision lists via CEA (board+0x10EC and board+0x8B0)

### Trigger Geoms (E:/N:)
These are **invisible** — they have geometry (triangles/vertices) but:
- The game skips rendering them (name prefix check)
- The game's `DispatchCollisionEvents` (0x0040C5D0) checks for their names on collision
- They fire events when the ball's collision sphere intersects them
- They do NOT provide solid collision (ball passes through)

### Level-Specific vs Global Events
| Handler | Scope | Events |
|---|---|---|
| `DispatchCollisionEvents` (0x40C5D0) | ALL levels (global) | E:DROPIN, E:POPOUT, E:PIPEBONK, E:ZOOP, E:JUMP, E:BREAK, E:ACTION, E:TRAJECTORY, E:NODIZZY, E:SAFESWITCH, E:LIMIT, N:GOAL, N:TARPIT, N:WATER, N:BOUNCE, N:NOCONTROL, N:MOUSETRAP |
| `UpRaceCollisionEvents` (0x4119B0) | Up Race only | N:SPEEDCYLINDER, E:VACPOPOUT, VAC-IN, VAC-OUT |
| `ExpertCollisionEvents` (0x40E9C0) | Expert only | E:BELL, E:CALLHAMMER, E:ALERTSAW |

**Only global events work when spawning via CEA.** Level-specific events require the level's own collision handler.

---

## 5. COLORING AND TEXTURING

### No Textures — Material Colors Only
For spawned meshes, always use `has_texture=0`.
Colors are defined per-geom in the material block.

### Color Values
Colors are **per-geom**, not per-vertex. Each geom has one material with one diffuse color.

```
Diffuse RGBA: 4 floats (Red, Green, Blue, Alpha), each 0.0 to 1.0
Ambient RGBA: typically Diffuse × 0.7-0.8
```

### Common Colors
| Color | Diffuse | Ambient |
|---|---|---|
| White | (0.9, 0.9, 0.9, 1.0) | (0.7, 0.7, 0.7, 1.0) |
| Gray | (0.4, 0.4, 0.4, 1.0) | (0.3, 0.3, 0.3, 1.0) |
| Yellow | (0.9, 0.8, 0.0, 1.0) | (0.7, 0.6, 0.0, 1.0) |
| Orange | (0.9, 0.5, 0.1, 1.0) | (0.7, 0.4, 0.1, 1.0) |
| Purple | (0.5, 0.0, 0.5, 1.0) | (0.4, 0.0, 0.4, 1.0) |
| Blue | (0.0, 0.3, 0.9, 1.0) | (0.0, 0.2, 0.7, 1.0) |
| Red | (0.9, 0.1, 0.1, 1.0) | (0.7, 0.1, 0.1, 1.0) |
| Green | (0.1, 0.9, 0.1, 1.0) | (0.1, 0.7, 0.1, 1.0) |
| Black | (0.05, 0.05, 0.05, 1.0) | (0.03, 0.03, 0.03, 1.0) |

### Checker Pattern
Create alternating geoms with different material colors.
Each tile = separate geom = separate material.
Tile (row+col) % 2 == 0 → white, else → gray.

### Why Not Textures?
The game's texture loader (`Level_AssignTextures`, vtable[0x90]) crashes at runtime
when called from spawned meshes. It tries to copy texture pointers from a main level
mesh that doesn't exist in the spawn context. Using has_texture=0 skips this entirely.

---

## 6. BOX GEOMETRY REFERENCE

### Standard Box (36 vertices, 12 triangles, 12 strips)

Corner layout:
```
0=(x0,y0,z0)  1=(x1,y0,z0)  2=(x1,y1,z0)  3=(x0,y1,z0)
4=(x0,y0,z1)  5=(x1,y0,z1)  6=(x1,y1,z1)  7=(x0,y1,z1)
```

Faces (each = 2 triangles = 6 vertices):
```
Bottom (y=y0, normal=(0,-1,0)): corners 0,1,5,4
Top    (y=y1, normal=(0, 1,0)): corners 3,7,6,2
Left   (x=x0, normal=(-1,0,0)): corners 0,4,7,3
Right  (x=x1, normal=( 1,0,0)): corners 1,2,6,5
Front  (z=z0, normal=(0,0,-1)): corners 0,3,2,1
Back   (z=z1, normal=(0,0, 1)): corners 4,5,6,7
```

Per face, emit 6 vertices (2 triangles):
```
Tri 1: v0, v1, v2  (3 consecutive vertices, CCW from outside)
Tri 2: v0, v2, v3
```

Per box: 6 faces × 6 vertices = 36 vertices
Per box: 6 faces × 2 triangles = 12 strips
Each strip: triangle_count=1, vertex_ref = base_offset + (triangle_index × 3)

### Arbitrary Box from 8 Explicit Corners
Same face definitions, but corners can be any 3D points (not axis-aligned).
Use this for tilted/angled geometry (ramps, slides, legs).

---

## 7. ROTATION

### 90° Multiples (Exact Integer Swaps)
Never use `math.cos`/`math.sin` — floating-point noise accumulates across vertices.

| Angle | X' | Z' |
|---|---|---|
| 0° (identity) | x | z |
| 90° CCW | -z | x |
| 180° | -x | -z |
| 270° CCW | z | -x |

Apply to both position AND normals (nx, nz). Y stays unchanged.

### Arbitrary Angles
Use `math.cos`/`math.sin` for non-90° angles. Acceptable for small vertex counts
(<100). For large meshes, verify no sub-pixel drift by checking that rotated
coordinates round to reasonable values.

### Rotation Axes
| Axis | Effect |
|---|---|
| Y axis | Rotates compass direction (left/right turn). Ball still rolls on flat surface. |
| Z axis | Tilts platform (creates ramp/slide). Changes which way is "up" on the surface. |
| X axis | Tilts depth (creates ramp going into screen). |

### Mirroring Warning
- Mirroring X alone: safe for symmetric shapes, invisible on non-symmetric ones
- Mirroring Z alone: **REVERSES triangle winding** → inside-out → collision freeze
  - Fix: swap v1↔v2 in every triangle AND negate all normals
- Mirroring both X+Z (180° rotation): safe (double reversal cancels out)

---

## 8. CEA SPAWN SCRIPT STRUCTURE

### Hook Point
```
Address: 0x00405E22
Original bytes: 8B 86 5C 0C 00 00  (mov eax, [esi+0x0C5C])
```
ESI = ball struct pointer at hook time.

### Spawn Flow
1. Save player position: ball+0x164 (X), ball+0x168 (Y), ball+0x16C (Z)
2. Y offset: subtract 20.0 from ball Y for spawn height
3. Allocate Level object: `call 004BA57B` with size 0x10D0
4. Load mesh: `call 00461510` (Level_MeshWorldCtor) with filename string
5. Allocate SceneObject: `call 004BA57B` with size 0x10E8
6. Create object: `call 0043BBC0` (Fallout_ctor) with (sceneobj, board, x, y, z, level)
7. Register render: `call 00453810` (AthenaList_Append) on board+0x2578
8. Register render: `call 00453810` on board+0xCD4
9. Register collision: `call 00453810` on board+0x10EC with sceneobj+0x10D4
10. Register collision: `call 00453810` on board+0x8B0->+0x18 with sceneobj+0x10D4
11. Register render: `call 00453810` on board+0x8AC->+0x480->+0x1C

### Key Offsets
| Offset | Meaning |
|---|---|
| ball+0x164 | Player X (float) |
| ball+0x168 | Player Y (float) |
| ball+0x16C | Player Z (float) |
| ball+0x18 | Menu/loading flag (skip if !=0) |
| ball+0x324 | Broken flag (set 1 = invincible to killplanes) |
| board+0x14 | Board pointer |
| board+0x878 | Graphics context |
| board+0x878->0x174 | Mesh loader context |
| sceneobj+0x10D4 | Collision mesh data |
| sceneobj+0x10D0 | Board reference |

### Mesh Filename String
Hex-encoded ASCII, NUL-terminated:
```
"Levels\Level-MyMesh\0"
```
In CEA:
```
BridgeStr:
  db 4C 65 76 65 6C 73 5C 4C 65 76 65 6C 2D 4D 79 4D 65 73 68 00
```

---

## 9. COMPLETE FILE STRUCTURE SUMMARY

```
[4 bytes]   section 1: ref_point_count = 0
[4 bytes]   section 2: spline_count = 0
[4 bytes]   section 3: light_count = 0
[24 bytes]  section 4: 6 floats = 0.0
[4 bytes]   section 5: vertex_count
[32*N]      section 5: N vertices (x,y,z,nx,ny,nz,u,v each)
[24 bytes]  section 6: bounds (min_x,min_y,min_z,max_x,max_y,max_z)
[4 bytes]   section 6: submesh_count = 0 (leaf)
[4 bytes]   section 6: has_data = 1
[4 bytes]   section 6: geom_count

FOR each geom:
  [4 bytes]   name_length
  [N bytes]    name (NUL-terminated)
  [16 bytes]   ambient RGBA
  [16 bytes]   diffuse RGBA
  [16 bytes]   specular RGBA
  [16 bytes]   emissive RGBA
  [4 bytes]    power (float)
  [4 bytes]    has_reflection = 0
  [4 bytes]    has_texture = 0
  [4 bytes]    strip_count
  [8*strips]  strip data (uint32 tri_count, uint32 vertex_ref each)
```

### Total File Size Formula
```
header = 4 + 4 + 4 + 24 = 36 bytes
vertex_section = 4 + (32 * vertex_count)
geom_header = 24 + 4 + 4 + 4 = 36 bytes
per_geom = 4 + name_len + 76 + 4 + (8 * strip_count)
total = 36 + vertex_section + geom_header + sum(per_geom)
```

---

## 10. PITFALLS (ALL 22 LEARNED)

1. **No Y/Z swap** — write coordinates directly, Y is up
2. **Box bottom face uses all-y0 corners** — NOT spanning y0 and y1
3. **36 verts per box** (12 tris × 3), NOT 24 (that causes OOB crash)
4. **12 strips per box** (1 per triangle), vref = base + tri_index × 3
5. **has_texture=0** — no textures for spawned meshes
6. **No texture name corruption** — rebuild S6 cleanly
7. **Rotation: exact integer swaps** for 90° multiples, not math.cos/sin
8. **Mirror Z needs winding fix** — swap v1↔v2 + negate normals
9. **65,534 vertex limit per geom** — split large meshes
10. **Runtime color via CEA doesn't work** — bake colors into the file
11. **vtable[0x90] crashes** — don't call Level_AssignTextures from CEA
12. **E:/N: triggers are invisible** — events only, no solid collision
13. **Mirroring symmetric shapes is invisible** — use 90° rotation instead
14. **Floating-point trig noise accumulates** — cos(270°)=-4.9e-16, not 0
15. **has_texture=1 with corrupt name** → "TEXTURE LOAD FAILED!" popup
16. **Flipping Y without winding fix** → dark + collision broken (DownPipe bug)
17. **Junction walls create lids** — shorten walls at L/T junctions to keep open
18. **Overlap walls at junctions** — start horizontal section inside vertical tube
19. **N:SPEEDCYLINDER is Up-Race-specific** — not in global handler
20. **E:VACPOPOUT is Up-Race-specific** — VAC-IN/VAC-OUT need level setup
21. **Ghidra param_1[0x69] = byte offset 0x1A4** — dword index × 4
22. **Ball position vs trajectory** — setting trajectory (scene+0xCA4) teleports;
    move ball+0x164/0x168/0x16C directly for smooth vacuum effect

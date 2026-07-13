# Custom MESHWORLD Mesh Guide for Hamsterball

## Overview
This guide covers creating custom `.MESHWORLD` files containing arbitrary 3D geometry (solid platforms, pipes, staircases, etc.) with collision and optional collision events (E:/N: trigger zones). These meshes are spawned at runtime via Cheat Engine Assembly (CEA) scripts using `FUN_0043BBC0` (which builds collision automatically).

You are NOT creating full levels — just mesh objects that get spawned into existing levels.

---

## MESHWORLD Binary Format

The file has 6 sections. All multi-byte values are **little-endian**.

### Section 1: Ref Points (game objects)
```
[uint32] point_count = 0
```
Leave empty (count=0). Not needed for spawned mesh objects.

### Section 2: Splines
```
[uint32] spline_count = 0
```

### Section 3: Lights
```
[uint32] light_count = 0
```

### Section 4: Global Info
```
[float x6] = 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
```

### Section 5: Global Vertex Buffer (THE geometry)
```
[uint32] vertex_count
FOR each vertex (32 bytes):
  [float] x     -- D3D X (write directly, NO coordinate swap)
  [float] y     -- D3D Y (up direction)
  [float] z     -- D3D Z (forward direction)
  [float] nx    -- Normal X
  [float] ny    -- Normal Y
  [float] nz    -- Normal Z
  [float] u     -- Texture U (0.0 or 1.0)
  [float] v     -- Texture V (0.0 or 1.0)
```

**CRITICAL**: Write coordinates directly. Do NOT swap Y and Z. The parser reads them as-is. Swapping makes flat platforms render vertical.

### Section 6: Octree (materials + strips)
```
[6 floats] bounds: min_x, min_y, min_z, max_x, max_y, max_z
[uint32] 0              -- 0 = leaf node (no submeshes)
[uint32] geom_count    -- number of geoms

FOR each geom:
  [uint32, chars+NUL] name     -- geom name (see Name Prefixes below)
  [float x4] Ambient RGBA      -- ambient color (R,G,B,A floats 0.0-1.0)
  [float x4] Diffuse RGBA      -- diffuse color
  [float x4] Specular RGBA     -- specular color
  [float x4] Emissive RGBA     -- emissive color
  [float] Power                -- shininess (10.0 is fine)
  [uint32] has_reflection      -- 0 = no reflection
  [uint32] has_texture         -- 0 = no texture (use material colors)
  [uint32] strip_count
  FOR each strip:
    [uint32] triangle_count    -- 1 for a single triangle
    [uint32] vertex_ref_offset -- index into global vertex buffer
```

If `has_texture=1`, you must also write `[uint32,len][chars+NUL] texture_filename`.
Do NOT set has_texture=1 with a corrupted filename — the game pops up "TEXTURE LOAD FAILED!".
Use `has_texture=0` and set material colors instead.

---

## Building Geometry

### Box (thick platform tile)

A thick box has 6 faces x 2 triangles x 3 vertices = **36 vertices** and **12 strips**.

Each face uses 4 corner indices that must ALL belong to the same plane:
- Bottom (y=y0): corners 0,1,5,4
- Top (y=y1): corners 3,7,6,2
- Left (x=x0): corners 0,4,7,3
- Right (x=x1): corners 1,2,6,5
- Front (z=z0): corners 0,3,2,1
- Back (z=z1): corners 4,5,6,7

Corner definitions:
```
0=(x0,y0,z0)  1=(x1,y0,z0)  2=(x1,y1,z0)  3=(x0,y1,z0)
4=(x0,y0,z1)  5=(x1,y0,z1)  6=(x1,y1,z1)  7=(x0,y1,z1)
```

For each face, create 2 triangles:
- Tri 1: v0, v1, v2 (3 consecutive vertices)
- Tri 2: v0, v2, v3 (3 consecutive vertices)

Each strip: `tri_count=1, vref = base_offset + triangle_index * 3`

### Checker pattern
Create multiple box geoms with alternating material colors (white/gray).
Each tile is a separate geom with its own material.

### Rotation (exact, no floating point)
- 0 deg: (x,z) -> (x, z)
- 90 deg: (x,z) -> (-z, x)
- 180 deg: (x,z) -> (-x, -z)
- 270 deg: (x,z) -> (z, -x)

Apply same transform to normals (nx, nz). Do NOT use math.cos/sin.

### Mirror warning
Mirroring Z alone reverses triangle winding -> inside-out geometry -> collision freeze.
Fix by swapping v1<->v2 in each triangle AND negating normals.
Mirroring both X+Z (180 deg rotation) does NOT need winding fix.

---

## Geom Name Prefixes

| Name | Effect | Visible | Collision |
|------|--------|---------|-----------|
| (empty, 1 byte NUL) | Normal solid surface | Yes | Yes |
| N:* | Notification/trigger zone | No | Events only |
| E:* | Edge trigger | No | Events only |
| *(NOCOLLIDE)* | No collision | Yes | No |

For solid geometry, use empty name: `[uint32]1 [byte]0x00`

---

## Collision Events (N:/E: Triggers)

Add trigger geoms as invisible boxes that fire events when the ball touches them.
Handled by `DispatchCollisionEvents` (0x0040C5D0), shared across ALL levels.

### Pipe events (verified working):

| Event | Effect | Cooldown |
|-------|--------|----------|
| E:DROPIN | Pipe entry sound + dizzy immunity | 50 frames |
| E:PIPEBONK | Ball hitting pipe walls + dizzy immunity | 10 frames |
| E:POPOUT | Pipe exit sound + dizzy immunity | 50 frames |
| E:ZOOP | Whoosh sound + dizzy immunity | 50 frames |

### Other events:

| Event | Effect |
|-------|--------|
| E:JUMP | Jump pad (sound + upward velocity + dizzy immunity) |
| E:BREAK | Break/shatter ball |
| E:ACTION | Custom action with tags (ONCE, SCORE) |
| E:TRAJECTORY | Set ball trajectory direction (X/Y/Z tags) |
| E:NODIZZY | Anti-dizzy zone (TIME tag) |
| E:SAFESWITCH | Switch to safe area |
| E:LIMIT | Limit ball movement |
| N:GOAL | Race finish line |
| N:TARPIT | Tar pit (slows ball + sound) |
| N:WATER | Water zone |
| N:NOCONTROL | Disable ball control |
| N:BOUNCE | Bounce pad |
| N:SPEEDCYLINDER | Speed boost |

### Adding trigger geoms:
Create a thin invisible box at the desired location with an E: or N: name.
Material colors don't matter (invisible). has_texture=0.
The trigger fires when ball collision sphere intersects the trigger box.

---

## CEA Spawn Script

Hooks `Ball_Update` (0x00405E22) and spawns mesh at player position.

### Key addresses:
- Hook: 0x00405E22 (original bytes: 8B 86 5C 0C 00 00)
- FUN_00461510: LoadMeshWorld
- FUN_0043BBC0: SceneObject constructor (builds collision + render)
- FUN_00453810: AthenaList_Append
- FUN_004BA57B: operator_new

### Spawn flow:
1. Allocate Level object (0x10D0 bytes)
2. Load mesh: FUN_00461510(level, gfx, "Levels\Level-YourMesh")
3. Allocate SceneObject (0x10E8 bytes)
4. Create: FUN_0043BBC0(sceneobj, board, x, y, z, level)
5. Register render: AthenaList_Append(board+0x2578, sceneobj)
6. Register render: AthenaList_Append(board+0xCD4, sceneobj)
7. Register collision: AthenaList_Append(board+0x10EC, sceneobj+0x10D4)
8. Register collision: AthenaList_Append(board+0x8B0->+0x18, sceneobj+0x10D4)
9. Register render: AthenaList_Append(board+0x8AC->+0x480->+0x1C, sceneobj)

### Player position:
- ball+0x164 = X (float)
- ball+0x168 = Y (float, adjust by -20.0 for spawn height)
- ball+0x16C = Z (float)

### File naming:
String must match filename WITHOUT extension:
`Levels\Level-YourMesh` -> file at `Levels/Level-YourMesh.MESHWORLD`

---

## Python Builder Template

```python
import struct

def make_box_tris(x0, y0, z0, x1, y1, z1):
    c = [(x0,y0,z0),(x1,y0,z0),(x1,y1,z0),(x0,y1,z0),
         (x0,y0,z1),(x1,y0,z1),(x1,y1,z1),(x0,y1,z1)]
    faces = [(0,1,5,4,(0,-1,0)),(3,7,6,2,(0,1,0)),
             (0,4,7,3,(-1,0,0)),(1,2,6,5,(1,0,0)),
             (0,3,2,1,(0,0,-1)),(4,5,6,7,(0,0,1))]
    verts = []
    for v0,v1,v2,v3,n in faces:
        verts += [(*c[v0],*n,0,0),(*c[v1],*n,1,0),(*c[v2],*n,1,1)]
        verts += [(*c[v0],*n,0,0),(*c[v2],*n,1,1),(*c[v3],*n,0,1)]
    return verts  # 36 vertices

def make_strips(base):
    return [(1, base + i*3) for i in range(12)]

def make_material(color_idx):
    if color_idx == 0:
        d=(0.9,0.9,0.9,1.0); a=(0.7,0.7,0.7,1.0)
    else:
        d=(0.4,0.4,0.4,1.0); a=(0.3,0.3,0.3,1.0)
    s=(0.1,0.1,0.1,1.0); e=(0,0,0,1.0)
    return (struct.pack('<4f',*a)+struct.pack('<4f',*d)+
            struct.pack('<4f',*s)+struct.pack('<4f',*e)+
            struct.pack('<f',10.0)+struct.pack('<i',0)+struct.pack('<i',0))

def build_meshworld(geoms):
    all_verts = []
    all_strips = []
    geom_colors = []
    geom_names = []
    for g in geoms:
        base = len(all_verts)
        all_verts.extend(g['verts'])
        all_strips.append(g['strips'])
        geom_colors.append(g['color'])
        geom_names.append(g['name'])
    cx = sum(v[0] for v in all_verts)/len(all_verts)
    cy = sum(v[1] for v in all_verts)/len(all_verts)
    cz = sum(v[2] for v in all_verts)/len(all_verts)
    all_verts = [(v[0]-cx,v[1]-cy,v[2]-cz,v[3],v[4],v[5],v[6],v[7])
                 for v in all_verts]
    xs=[v[0] for v in all_verts]; ys=[v[1] for v in all_verts]
    zs=[v[2] for v in all_verts]
    bounds=(min(xs),min(ys),min(zs),max(xs),max(ys),max(zs))
    s1=struct.pack('<i',0); s2=struct.pack('<i',0); s3=struct.pack('<i',0)
    s4=struct.pack('<6f',0,0,0,0,0,0)
    s5=struct.pack('<i',len(all_verts))
    for v in all_verts:
        s5+=struct.pack('<ffffffff',*v)
    s6=struct.pack('<6f',*bounds)+struct.pack('<i',0)+struct.pack('<i',len(all_strips))
    for i,strips in enumerate(all_strips):
        name=geom_names[i]
        s6+=struct.pack('<i',len(name))+name
        s6+=make_material(geom_colors[i])
        s6+=struct.pack('<i',len(strips))
        for tc,vr in strips:
            s6+=struct.pack('<ii',tc,vr)
    return s1+s2+s3+s4+s5+s6

# --- Build your mesh ---
geoms = []

# Solid box platform
base = 0
verts = make_box_tris(-100, -10, -100, 100, 10, 100)
geoms.append({
    'verts': verts,
    'strips': make_strips(base),
    'color': 0,  # white
    'name': b'\x00'  # empty = solid collision
})

# Trigger zone (E:DROPIN at entrance)
base = len(verts)
verts2 = make_box_tris(-50, -5, -50, 50, 5, 50)
geoms.append({
    'verts': verts2,
    'strips': make_strips(base),
    'color': 0,
    'name': b'E:DROPIN\x00'  # invisible trigger
})

# Build and save
meshworld = build_meshworld(geoms)
with open('Level-MyMesh.MESHWORLD', 'wb') as f:
    f.write(meshworld)
```

---

## Pitfalls Summary

1. **No Y/Z swap** -- write coordinates directly
2. **Box faces use same-plane corners** -- bottom = all y0, top = all y1
3. **36 verts per box** (3 per triangle x 12 triangles), NOT 24
4. **12 strips per box** (1 per triangle, vref = base + tri_index x 3)
5. **has_texture=0** to avoid texture loading -- use material colors
6. **No texture name corruption** -- rebuild S6 without texture data
7. **Rotation: exact swaps** -- use integer coordinate swaps, not trig
8. **Mirror Z needs winding fix** -- swap v1<->v2 + negate normals
9. **65,534 vertex limit per MeshBuffer** -- split large meshes into multiple geoms
10. **Runtime color writing via CEA doesn't work** -- bake colors into the file
11. **vtable[0x90] crashes at runtime** -- don't call Level_AssignTextures from CEA
12. **E:/N: trigger geoms are invisible** -- they only fire events, no collision

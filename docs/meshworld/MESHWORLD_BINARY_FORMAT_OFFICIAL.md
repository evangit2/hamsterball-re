# MESHWORLD Binary Format — Definitive Specification

**Source**: Official Raptisoft 3DS Max exporter (MeshWorldExport.cpp by John Raptis)
**Reference**: `reference/raptisoft-exporter/MeshWorldExport/`

## Vertex Structure (32 bytes)
```c
typedef struct Vertex {
    float mX, mY, mZ;          // Position (12 bytes)
    float mNormalX, mNormalY, mNormalZ;  // Normal (12 bytes)
    float mTextureU, mTextureV;  // UV coords (8 bytes)
} Vertex;  // Total: 32 bytes
```

## Binary File Layout

```
SECTION 1: Ref Points (game objects like START, FLAG, N:GOAL, etc.)
  [int]     point_count
  FOR each point:
    [int,len][string]  name (length-prefixed, NUL-terminated)
    [float]   position.x      // NOTE: x,z,y order (Max→D3D swap)
    [float]   position.z
    [float]   position.y
    [float]   rotation.x
    [float]   rotation.z
    [float]   rotation.y
    [int]     has_material    // 0=no material, 1=has material
    IF has_material:
      [float×4]  Ambient  (r,g,b,a)
      [float×4]  Diffuse  (r,g,b,a)
      [float×4]  Specular (r,g,b,a)
      [float×4]  Emissive (r,g,b,a)
      [float]    Power (shininess)
      [int]      has_reflection
      [int]      has_texture (0 or 1)
      IF has_texture:
        [int,len][string] texture_filename

SECTION 2: Splines (BallPath objects)
  [int]     spline_count
  FOR each spline:
    [int,len][string]  name
    [int]     point_count
    FOR each point:
      [float]   x        // NOTE: x,z,y order
      [float]   z
      [float]   y

SECTION 3: Lights
  [int]     light_count
  FOR each light:
    [int]     type (0=DISTANTLIGHT)
    IF DISTANTLIGHT:
      [float]   position.x,z,y
      [float]   lookat.x,z,y
      [float]   color.r,g,b

SECTION 4: Background/Ambient Colors
  [float×3] background_color (r,g,b)
  [float×3] ambient_color (r,g,b)

SECTION 5: Global Vertex Buffer
  [int]     vertex_count
  [Vertex×vertex_count] vertex_list  // 32 bytes each

SECTION 6: Octree Mesh Dump (recursive)
  [Cube]    bounding_box     // 6 floats: corner1(x,y,z) + corner2(x,y,z)
  IF subdivided:
    [int]     submesh_count   // >0 means has children
    FOR each submesh:
      [recursive SECTION 6]
  ELSE (leaf node):
    [int]     0               // 0 = no submeshes, leaf flag
    [int]     geom_count      // Number of geometry objects
    FOR each geom:
      [int,len][string]  name  // "" unless N: or E: prefix or NOCOLLIDE
      [float×4]  Ambient  (r,g,b,a)
      [float×4]  Diffuse  (r,g,b,a)
      [float×4]  Specular (r,g,b,a)
      [float×4]  Emissive (r,g,b,a)
      [float]    Power
      [int]      has_reflection
      [int]      has_texture (0 or 1)
      IF has_texture:
        [int,len][string] texture_filename
      [int]      strip_count
      FOR each strip:
        [int]    triangle_count   // # triangles in this strip
        [int]    vertex_ref_offset // mSecondaryVertexListReference
                                       // (offset into global vertex buffer)
```

## Key Insights from Exporter Source

1. **X/Z/Y swap**: All positions written as x,z,y (3DS Max Z-up → D3D Y-up)
2. **Octree structure**: Mesh is recursively subdivided into cubes for visibility culling
3. **Global vertex buffer**: All vertices in a single buffer, strips reference into it
4. **Triangle strips**: Not raw indices — each strip stores `triangle_count` + `vertex_ref_offset`
   into the global vertex buffer (WorldOptimize adjusts offsets)
5. **Geometry names**: Only written if they have `N:` or `E:` prefix or contain `NOCOLLIDE`
6. **Materials stored per-geom**: Ambient/Diffuse/Specular/Emissive + Power + texture + reflection
7. **16384 triangle cap**: From Carsillas' notes — game crashes if level exceeds 2^14 triangles

## String Format
```
[int]     length (including NUL terminator)
[length bytes]  string data (NUL-terminated, may be empty string with length=1)
```

## Cube Structure (24 bytes)
```
[float] corner1.x
[float] corner1.y  
[float] corner1.z
[float] corner2.x
[float] corner2.y
[float] corner2.z
```

## Reading Order for Level Rendering
1. Skip ref points (Section 1) — already parsed by our existing code
2. Skip splines (Section 2)
3. Skip lights (Section 3)
4. Read background + ambient colors (Section 4)
5. Read global vertex buffer (Section 5) — THIS is the geometry to render
6. Walk octree (Section 6) — each leaf has geoms with materials + strip references into vertex buffer

## Original Code References
- Format writer: `MeshWorldExport.cpp:DoExport()` line 186
- Vertex struct: `Math.h` line 18-30
- Octree dump: `WorldMesh.cpp:Dump()` line 259
- String write: `MeshWorldExport.cpp:WriteString()` line 147 (length-prefixed)
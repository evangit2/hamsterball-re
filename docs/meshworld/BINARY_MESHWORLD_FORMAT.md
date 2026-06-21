# Binary MESHWORLD Format — Complete Decompilation

Based on Ghidra decompilation of FUN_004629E0 (Level::BinaryLoader)

## File Structure (Sequential reads)

```
=== SECTION 1: Materials (0x7C structure each) ===
[uint32] material_count

For each material:
  [uint32] name_length
  [name_length bytes] name  (allocated, stored at material+0x00)
  [uint32] field1 -> material[1]  (position X or face_start, float value for objects)
  [uint32] field2 -> material[2]  (position Y)
  [uint32] field3 -> material[3]  (position Z)
  [uint32] field4 -> material[4]  (often 0)
  [uint32] field5 -> material[5]  (often 0)
  [uint32] field6 -> material[6]  (often 0x80000000 = -0.0f)
  [4 bytes]  extended_flag    -> material[10] (checked as char, rest padding)
  
  If extended_flag != 0:
    [4 × float] ambient_rgba    -> material[0x10..0x13]
    [4 × float] diffuse_rgba    -> material[0x0C..0x0F]
    [4 × float] specular_rgba   -> material[0x14..0x17]
    [4 × float] emission_rgba   -> material[0x18..0x1B]   *** WAS MISSING ***
    [float]     shine           -> material[0x1C]
    [uint32]    has_reflective  -> stored as bool at material+0x79
    [uint32]    has_texture     (checked with == 1, not != 0)
    
    If has_texture == 1:
      [uint32] texture_name_length
      [texture_name_length bytes] texture_filename
      (loaded via Graphics_LoadTexture)

NOTE: Section 1 items include BOTH materials AND game objects!
In WarmUp (16 items): START2-2, START2-1, CAMERALOOKAT, 
PLATFORM(x5, ext=1), SAFESPOT(x5), STANDS, START2-4, START2-3
The 6 u32 "fields" are actually float[3] position + 3 padding values.

=== SECTION 2: Mesh Buffers (0x54 structure each) ===
[uint32] mesh_buffer_count

For each mesh buffer:
  [uint32] data_length
  [data_length bytes] raw_data (stored at buffer+0x50)
  [uint32] face_count
  For each face:
    [3 × uint32] face_data (12 bytes per face)
    FUN_004685e0(buffer) processes vertex data
  FUN_00469090(buffer, 0) finalizes buffer

=== SECTION 3: Game Objects (0xD4 structure each) ===
[uint32] object_count

For each object:
  [uint32] object_type        (0 = straight/ramp, non-zero = special)
  
  If object_type == 0:
    [3 × float] position       (3 reads via vtable+4)
    [3 × float] euler_rotation (3 reads, 2 via vtable+8, skip pattern)
    [3 × float] scale/transform
    [D3D color/material init]
  
  (All objects added to level+0x478 list)

=== SECTION 4: Bounding Box ===
[float] min_x -> level+0x454
[float] min_y -> level+0x458
[float] min_z -> level+0x45C
[float] max_x -> level+0x468
[float] max_y -> level+0x46C
[float] max_z -> level+0x470

Note: In the Level STRUCT, there's a 12-byte gap between +0x45C and +0x468
(+0x460, +0x464, +0x468). But from the FILE it's 6 sequential reads (24 bytes).

**IMPORTANT**: For many Arena levels, the bbox values in the file are INVALID
(min > max for some axes). They appear to be placeholder/garbage values.
The game likely computes the real bbox from vertex data at runtime.
Our reimpl recomputes bbox from vertex positions if invalid or all-zeros.

Example invalid bbox from Arena-WarmUp: (0.98, 0.46, 1.0, 0.56, 0.56, 0.56)
Example zero bbox from Arena-SpawnPlatform: (0, 0, 0, 0, 0, 0)

=== SECTION 5: Vertex Array ===
[uint32] vertex_count -> level+0x438
(level+0x43C = 0, buffer allocated = vertex_count * 32 bytes at level+0x440)
[vertex_count * 32 bytes] vertex data

Vertex format (32 bytes each):
  [3 × float] position (x, y, z)     // 12 bytes
  [3 × float] normal (nx, ny, nz)    // 12 bytes
  [2 × float] texcoord (u, v)        // 8 bytes

**RENDERING NOTE**: Vertex data is in TRIANGLE-LIST order.
Every 3 consecutive vertices form a single triangle.
Confirmed with SpawnPlatform: V0-V5 = top face (2 tris), V6-V11 = front face,
V12-V17 = right face, V18-V23 = back face, V24-V29 = left face (10 tris total).
Use D3DPT_TRIANGLELIST with DrawPrimitive, prim_count = vertex_count / 3.

=== SECTION 6: Post-load ===
level+0x43C = 0 (vertex_data_ready flag)
level->self_ptr = this              (level+0x47C)
level+0x431 = 1 (loaded flag)
Timer_Init for level+0x10D
(**vtable+0x34)(file_handle, this) // callback
__close(file_handle)
FUN_004601A0(this) // finalize load

=== Trailing data (after vertex array in some files) ===
After the vertex data, some files contain:
- Material color overrides (ambient/diffuse/specular RGBA, shine, etc.)
- Texture name strings (e.g. "PinkChecker.bmp\0")
- Face index arrays (pairs of: vertex_index + triangle_group_flag)
```

## Level Structure Offsets (from MeshWorld object)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x120 | ptr | mesh_data | Pointer to mesh data master struct |
| +0x438 | uint32 | vertex_count | Number of vertices |
| +0x43C | uint32 | zero_flag | Set to 0 after vertex allocation |
| +0x440 | ptr | vertex_array | Vertex data (32 bytes each) |
| +0x454 | float | min_x | Bounding box min X |
| +0x458 | float | min_y | Bounding box min Y |
| +0x45C | float | min_z | Bounding box min Z |
| +0x460 | ??? | ??? | (gap in struct layout) |
| +0x468 | float | max_x | Bounding box max X |
| +0x46C | float | max_y | Bounding box max Y |
| +0x470 | float | max_z | Bounding box max Z |
| +0x478 | list | object_list | Game objects (AthenaList) |
| +0x47C | ptr | self_ptr | Pointer back to this object |
| +0x894 | list | material_list | Materials (AthenaList) |
| +0xCAC | list | mesh_buffer_list | Mesh buffers (AthenaList) |
| +0x10D | ptr | timer | Timer object |

## Material Structure (0x7C bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x00 | ptr | name | Material name string |
| +0x04 | uint32 | field1 | Position X (float) or face_start |
| +0x08 | uint32 | field2 | Position Y (float) or face_count |
| +0x0C | float[4] | diffuse | Diffuse color RGBA |
| +0x10 | float[4] | ambient | Ambient color RGBA |
| +0x14 | float[4] | specular | Specular color RGBA |
| +0x18 | float[4] | emission | Emission color RGBA |
| +0x1C | float | shine | Shininess |
| +0x28 | byte | extended | Extended flag (byte from u32 read) |
| +0x74 | bool | has_reflective | Reflective material flag |
| +0x75 | bool | has_texture | Texture present flag |
| +0x78 | ptr | texture_ptr | Loaded texture pointer |
| +0x79 | bool | is_transparent | Alpha != -0.0 |

## Global Constants

| Address | Float Value | Usage |
|---------|-------------|-------|
| 0x4CF3C8 | -0.0 (0x80000000) | Transparency comparison value |

## Verified File Data

### Arena-SpawnPlatform.MESHWORLD (1,476 bytes)
- material_count=0, meshbuf_count=0, obj_count=0
- bbox at offset 12: all zeros (placeholder)
- vertex_count=34 at offset 36
- 34 vertices at offset 40 (34 × 32 = 1088 bytes)
- Real bbox at offset 1128: (-160, 2, -149) to (172, 13, 145)
- Trailing data: face indices + material data

### Arena-WarmUp.MESHWORLD (83,254 bytes)
- material_count=16 (includes START, PLATFORM, SAFESPOT, STANDS items)
- 5 PLATFORM items have ext=1 with ambient/diffuse data
- Items 0-2,8-16 have no extended section
- meshbuf_count=0 after materials (at offset 1042)
- obj_count=1 at offset 1046
- Object type=0 at offset 1050, position=(634.2, 286.6, -375.3)
- Bbox+vertices location: TBD (need to parse object section correctly)
- File ends with "PinkChecker.bmp" material data and face indices
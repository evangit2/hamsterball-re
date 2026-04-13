# Binary MESHWORLD Format — Complete Decompilation

Based on Ghidra decompilation of FUN_004629E0 (Level::BinaryLoader)

## File Structure (Sequential reads)

```
=== SECTION 1: Materials ===
[uint32] material_count

For each material:
  [uint32] name_length
  [name_length bytes] name  (allocated, stored at material+0x00)
  [uint32] face_start         -> material[1]  (face index start?)
  [uint32] face_count         -> material[2]  (face count?)
  [uint32] unk3               -> material[3]
  [uint32] unk4               -> material[4]
  [uint32] unk5               -> material[5]
  [uint32] unk6               -> material[6]
  [4 bytes]  extended_flag    -> material[10] (checked as char, rest padding)
  
  If extended_flag != 0:
    [4 bytes] ambient_r        -> material[0x40] after shuffle
    [4 bytes] ambient_g        -> material[0x44 after shuffle]
    [4 bytes] ambient_b        
    [4 bytes] ambient_a        -> material[0x4C] = alpha
    [4 bytes] diffuse_r       -> material[0xC]
    [4 bytes] diffuse_g       -> material[0xD]
    [4 bytes] diffuse_b       -> material[0xE]
    [4 bytes] diffuse_a       -> material[0xF] (compared to 0x4CF3C8 = -0.0 to detect transparency)
    [4 bytes] specular_r      -> material[0x14]
    [4 bytes] specular_g      -> material[0x15]
    [4 bytes] specular_b      -> material[0x16]
    [4 bytes] specular_a      -> material[0x17]
    [4 bytes] shine_strength   -> material[0x18]
    [uint32] has_reflective    -> stored as bool at material+0x79
    [uint32] has_texture      
    
    If has_texture == 1:
      [uint32] texture_name_length
      [texture_name_length bytes] texture_filename
      (loaded via Graphics_LoadTexture)

=== SECTION 2: Mesh Buffers (Geometry Submeshes) ===
[uint32] mesh_buffer_count

For each mesh buffer:
  [uint32] name_length
  [name_length bytes] name   (stored in buffer object)
  [void*]  name_ptr           (allocated, stored at buffer+0x50)
  [uint32] face_count        (stored at buffer+0x34 area)
  
  For each face:
    [float] some_value        (3 reads per "face")
    FUN_004685e0(buffer)     (processes vertex data)
  
  FUN_00469090(buffer, 0)    (finalizes buffer)

=== SECTION 3: Game Objects ===
[uint32] object_count

For each object:
  [uint32] object_type        (0 = straight/ramp, others = special)
  
  If object_type == 0:
    [4 bytes] unk1
    [4 bytes] unk2  
    [4 bytes] unk3
    (**vtable+4)(obj, unk1, unk2, unk3)   // 3D position
    [4 bytes] unk4
    [4 bytes] unk5
    [4 bytes] unk6
    (**vtable+8)(obj, unk4, unk5, unk6)   // 3D size/rotation
    [4 bytes] scale_u          // UV or transform
    [4 bytes] scale_v          // UV or transform  
    [4 bytes] scale_w          // UV or transform
    // D3D color/material init:
    // scale values -> object+0x94..0xA0
    // ambient color (0,0,0)  -> object+0xA8..0xB4
    // diffuse color (0.75,0.75,0.75) -> object+0xBC..0xC8
  
  (All objects added to level+0x478 list)

=== SECTION 4: Bounding Box ===
[float] min_x -> level+0x454
[float] min_y -> level+0x458
[float] min_z -> level+0x45C
[float] max_x -> level+0x468
[float] max_y -> level+0x46C
[float] max_z -> level+0x470

=== SECTION 5: Vertex Array ===
[uint32] vertex_count -> level+0x438
(allocated buffer = vertex_count * 32 bytes at level+0x440)
[vertex_count * 32 bytes] -> vertex data

=== SECTION 6: Post-load ===
level+0x43C = 0 (vertex_data_ready flag)
level->self_ptr = this           (level+0x47C)
level+0x431 = 1 (loaded flag)
Timer_Init for level+0x10D
(**vtable+0x34)(file_handle, this) // callback
__close(file_handle)
FUN_004601A0(this) // finalize load
```

## Level Structure Offsets (from MeshWorld object)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x120 | ptr | mesh_data | Pointer to mesh data master struct |
| +0x438 | uint32 | vertex_count | Number of vertices |
| +0x440 | ptr | vertex_array | Vertex data (32 bytes each) |
| +0x454 | float | min_x | Bounding box min X |
| +0x458 | float | min_y | Bounding box min Y |
| +0x45C | float | min_z | Bounding box min Z |
| +0x468 | float | max_x | Bounding box max X |
| +0x46C | float | max_y | Bounding box max Y |
| +0x470 | float | max_z | Bounding box max Z |
| +0x47C | ptr | self_ptr | Pointer back to this object |
| +0x478 | list | object_list | Game objects (AthenaList) |
| +0x894 | list | material_list | Materials (AthenaList) |
| +0xCAC | list | mesh_buffer_list | Mesh buffers (AthenaList) |
| +0x10D | ptr | timer | Timer object |

## Material Structure (0x7C bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x00 | ptr | name | Material name string |
| +0x04 | uint32 | face_start | First face index |
| +0x08 | uint32 | face_count | Number of faces |
| +0x0C | uint32 | unk3 | Unknown |
| +0x10 | uint32 | unk4 | Unknown |
| +0x14 | uint32 | unk5 | Unknown |
| +0x18 | uint32 | unk6 | Unknown |
| +0x28 | float[4] | ambient | Ambient color RGBA |
| +0x34 | float[4] | diffuse | Diffuse color RGBA |
| +0x44 | float[4] | specular | Specular color RGBA |
| +0x54 | float | shine | Shininess |
| +0x74 | bool | has_reflective | Reflective material flag |
| +0x74+1| bool | has_texture | Texture present flag |
| +0x74+4| ptr | texture_ptr | Loaded texture pointer |
| +0x79 | bool | is_transparent | Alpha != -0.0 |

## Global Constants

| Address | Float Value | Usage |
|---------|-------------|-------|
| 0x4CF3C8 | -0.0 (0x80000000) | Transparency comparison value |
# MESH Binary Format Specification

## Overview

The `.MESH` format is Hamsterball's custom binary 3D model format. It is NOT a standard DirectX .X file — it's a proprietary format containing submeshes with materials, vertex buffers, index buffers, and animation data.

## File Structure

```
[MESH File]
├── Header (version, name)
├── SubMesh Array (N submeshes)
│   ├── Submesh[0]: vertex buffer + material + textures
│   ├── Submesh[1]: ...
│   └── Submesh[N-1]: ...
├── Animation Data (M submesh animations)
├── Object Array (K objects with transforms)
└── Footer (transform data, index arrays)
```

## Parsed Format (from decompiled MeshWorld loader at 0x4629E0)

### Section 1: SubMeshes (Materials + Vertex Data)

```
[4 bytes] submesh_count (iStack_578)

FOR each submesh (i = 0..submesh_count-1):
  [4 bytes]     vertex_data_size (uStack_598)
  [vertex_data_size bytes] raw_vertex_buffer (allocated, read in one block)
  [4 bytes]     field_1 (position X or similar)
  [4 bytes]     field_2 (position Y)
  [4 bytes]     field_3 (position Z)
  [4 bytes]     field_4
  [4 bytes]     field_5
  [4 bytes]     field_6
  [4 bytes]     has_material_flag (puVar5+10, checked as char)
  
  IF has_material_flag != 0:
    [4 bytes]   diffuse_r (uStack_594)
    [4 bytes]   diffuse_g (uStack_590)  
    [4 bytes]   diffuse_b (iStack_59c)
    [4 bytes]   diffuse_a (fStack_5a4) → puVar5[0x13]
    [4 bytes]   ambient_r → puVar5[0x10]
    [4 bytes]   ambient_g → puVar5[0x11]
    [4 bytes]   ambient_b → puVar5[0x12]
    has_custom_color = (diffuse_a != _DAT_004CF3C8)  // != default
    
    [4 bytes]   emissive_r → puVar5[0xC]
    [4 bytes]   emissive_g → puVar5[0xD]
    [4 bytes]   emissive_b → puVar5[0xE]
    [4 bytes]   emissive_a → puVar5[0xF]
    
    [4 bytes]   specular_r → puVar5[0x14]
    [4 bytes]   specular_g → puVar5[0x15]
    [4 bytes]   specular_b → puVar5[0x16]
    [4 bytes]   specular_a → puVar5[0x17]
    
    [4 bytes]   unknown_r → puVar5[0x18]
    [4 bytes]   unknown_g → puVar5[0x19]
    [4 bytes]   unknown_b → puVar5[0x1A]
    [4 bytes]   unknown_a → puVar5[0x1B]
    
    [4 bytes]   specular_power → puVar5[0x1C]
    [4 bytes]   has_texture_flag (iStack_554, bool stored at +0x79)
    [4 bytes]   texture_name_length (iStack_55C)
    
    IF texture_name_length == 1:
      [4 bytes]   filename_length (puStack_584)
      [filename_length bytes] texture_filename (acStack_50c)
      texture = Graphics_LoadTexture(graphics, filename, 1)
      puVar5[0x1D] = texture
  ENDIF
  
  Store submesh in meshworld+0x894 list
END FOR
```

### Section 2: Animation Data

```
[4 bytes]     animation_count (iStack_57C)

FOR each animation (i = 0..animation_count-1):
  [4 bytes]   animation_data_size (uStack_598)
  [animation_data_size bytes] raw animation buffer
  [4 bytes]   keyframe_count (iStack_59C)
  
  FOR each keyframe (j = 0..keyframe_count-1):
    [4 bytes]   keyframe_time (fStack_5A4)
    [... keyframe data ...]  (processed by FUN_004685E0 / FUN_00468510)
  END FOR
  
  FUN_00469090(animation, 0) // Finalize animation
  Store in meshworld+0xCAC list
END FOR
```

### Section 3: Objects (Transforms)

```
[4 bytes]     object_count (iStack_574)

FOR each object (i = 0..object_count-1):
  [4 bytes]   object_type (iStack_550, checked for 0)
  
  IF object_type == 0:
    piVar8[0x34] = 3  // Set render mode
    
    [4 bytes]   position_x (iStack_5A0)
    [4 bytes]   position_y (uStack_58C) 
    [4 bytes]   position_z (uStack_580)
    vtable[4](x, y, z)  // SetPosition
    
    [4 bytes]   rotation_param1
    [4 bytes]   rotation_param2 (uStack_598)
    [4 bytes]   rotation_param3 (uStack_58C)
    vtable[8](param1, param2, param3)  // SetRotation
    
    [4 bytes]   scale_x (puStack_588)
    [4 bytes]   scale_y (uStack_568)
    [4 bytes]   scale_z (uStack_56C)
    Matrix_Scale4x4(scale_x, scale_y, scale_z, 1.0)
    
    // Apply matrix transforms
    piVar8[0x25..0x28] = transform matrix row 0
    piVar8[0x2F..0x32] = scale matrix (0.8, 0.8, 0.8, 1.0) default
    piVar8[0x2A..0x2D] = zero matrix
    
    Store in meshworld+0x478 list
  ENDIF
  
  uStack_598++
END FOR
```

### Section 4: Footer Data

```
[4 bytes]   meshworld+0x454 value
[4 bytes]   meshworld+0x458 value
[4 bytes]   meshworld+0x45C value
[4 bytes]   meshworld+0x468 value
[4 bytes]   meshworld+0x46C value
[4 bytes]   meshworld+0x470 value

[4 bytes]   index_count (meshworld+0x438)
meshworld+0x43C = 0
[index_count * 32 bytes] index data (read as single block, stride=0x20)

meshworld+0x10F = this  // Back-pointer
meshworld+0x131 = 1     // Initialized flag

// Timer init
timer = Timer_Init(0x44 bytes)
meshworld+0x10D = timer

// Custom vtable callback
vtable[0x34](filehandle, meshworld)

__close(filehandle)
```

## Vertex Buffer Layout (Inferred from Data)

Based on binary analysis of Sphere.MESH:
- Position: 3 floats (X, Y, Z)
- Normal: 3 floats (NX, NY, NZ)
- TexCoord: 2 floats (U, V)
- Total: 8 floats = 32 bytes per vertex

Sphere.MESH vertex 0: pos=(25.0, 0.0, 0.0) norm=(0.999, -0.01, 0.054) uv=(0.75, 0.5)
This confirms a sphere of radius 25 units with UV wrapping.

## Submesh Entry Structure (0x7C = 124 bytes)

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x00 | 4 | void* | Vertex buffer pointer |
| +0x04 | 4 | float | Field 1 (position X?) |
| +0x08 | 4 | float | Field 2 (position Y?) |
| +0x0C | 4 | float | Emissive R |
| +0x10 | 4 | float | Ambient R |
| +0x14 | 4 | float | Ambient G |
| +0x18 | 4 | float | Ambient B |
| +0x1C | 4 | float | Specular Power |
| +0x20 | 4 | float | Field (init 1.0) |
| +0x24 | 4 | float | Field (init 1.0) |
| +0x28 | 4 | float | Field (init 1.0) |
| +0x2C | 4 | float | Diffuse A |
| +0x30 | 4 | float | Diffuse R |
| +0x34 | 4 | float | Diffuse G |
| +0x38 | 4 | float | Diffuse B |
| +0x3C | 4 | float | Emissive G |
| +0x40 | 4 | float | Emissive B |
| +0x44 | 4 | float | Emissive A |
| +0x48 | 4 | float | Specular R |
| +0x4C | 4 | float | Specular G |
| +0x50 | 4 | float | Specular B |
| +0x54 | 4 | float | Specular A |
| +0x58 | 4 | float | Unknown R |
| +0x5C | 4 | float | Unknown G |
| +0x60 | 4 | float | Unknown B |
| +0x64 | 4 | float | Unknown A |
| +0x68 | 4 | float | Specular Power |
| +0x6C | 1 | bool | Has texture |
| +0x70 | 4 | void* | Texture pointer |
| +0x74 | 1 | bool | Has custom color |

## Known Mesh Files

| File | Size | Submeshes | Vertices | Name | Texture |
|------|------|-----------|----------|------|---------|
| Sphere.MESH | 10181 | 6 | 59 | Sphere01 | HamsterBall.png |
| 8Ball.MESH | 7675 | 44 | 42 | Sphere01 | 8Ball.png |
| Sawblade.MESH | 6723 | 1 | 60 | Box01 | sawblade.png |
| Fanblades.MESH | 3864 | - | - | REF:FAN | - |
| Hamster-Waiting.MESH | 18492 | - | - | Hamster | - |

## Reimplementation Notes (OpenGL/Assimp)

### Loading Strategy
```cpp
struct MeshSubmesh {
    std::vector<float> vertex_buffer;  // pos(3)+normal(3)+uv(2) per vertex
    float position[3];
    struct { float r, g, b, a; } diffuse, ambient, emissive, specular;
    float specular_power;
    bool has_texture;
    Texture* texture;
};

struct MeshObject {
    float position[3];
    float rotation[3];
    float scale[3];
    float transform_matrix[4][4];
};

struct MeshFile {
    std::vector<MeshSubmesh> submeshes;
    std::vector<Animation> animations;
    std::vector<MeshObject> objects;
    
    static MeshFile Load(const std::string& filename);
};
```

### Conversion Roadmap
1. Write a `mesh2gltf` converter tool (Python/C++)
2. Parse .MESH binary → extract submeshes + vertex data
3. Generate GLTF2 meshes with materials
4. For animations: export as glTF2 animation channels
5. Load in engine via Assimp or tinygltf
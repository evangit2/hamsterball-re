# MESHWORLD File Format (CONFIRMED)

## Overview
Binary file format used for Hamsterball levels. Contains 3D geometry data
followed by object definitions with positions, transforms, and materials.

## File Structure

```
[Geometry Section]
  - Binary mesh data (vertices, faces, normals, UVs)
  - Bounding box data
  - May be empty (count=0 in Arena-SpawnPlatform)

[Object Section]
  - uint32: total object count
  - Sequential object definitions (variable length per type)
```

## Geometry Section
Starts at offset 0. The section contains:
- First uint32 may be 0 (Arena-SpawnPlatform) or a vertex count
- Followed by floats for bounding box and vertex data
- Variable-length mesh geometry data

## Object Section Format

Each object definition has this structure:
```
[uint32: type_string_length]
[type_string: variable bytes, null-padded to length]
[object data: variable size depending on type]
  - Position: 3 floats (x, y, z)
  - Rotation/scale: 4 values (Quaternion or Euler+scale)
  - Material properties (for complex types)
  - Texture reference (for complex types):
    [uint32: texture_string_length]  
    [texture_string: null-terminated filename]
  - Face/vertex indices (for complex types)
```

## Object Type Sizes (CONFIRMED from binary analysis)

### Simple Objects (28 bytes after type string)

#### START positions (START1-1, START2-1, START2-2)
```
[float: x_position]
[float: y_position]  
[float: z_position]
[uint32: 0]           # rotation x (0)
[uint32: 0]           # rotation y (0)
[float: 0x80000000]   # rotation z (-0.0 or direction flag)
[uint32: 0]           # padding/flags
```

#### SAFESPOT (28 bytes after type string)
Same structure as START.

### Complex Objects (variable size)

#### FLAG (FLAG02, FLAG04, FLAG06, FLAG07)
- 104 bytes after type string
- Then texture reference string (e.g., "BlackCheckerFlag.png")
- Then 0 bytes before next object
- Contains: position(12) + rotation(16) + scale/matrix(32) + color(16) + size_param(4) + flags(8) + face indices

#### PLATFORM / N:SINKPLATFORM
- 76 bytes after type string (transform + color + size)  
- Then texture reference string
- Then mesh face index data (pairs of index + value)
- Size parameter: 35.0 for standard platforms

#### E:NODIZZY<TIME>N</TIME>
- Contains embedded XML property for timer
- Followed by transform matrix (16 floats = 4x4 identity-like)
- Then position data (3 floats for center, repeated as 4x3 matrix)
- Closing: uint32(0), uint32(0), uint32(0), uint32(1), uint32(2), uint32(global_count)

## Material/Transform Data (for complex objects)
```
Offset  Size  Description
0x00    32    Rotation/scale matrix (8 floats, identity = all 1.0)
0x20    12    Diffuse color RGB (3 floats, e.g., 0.45, 0.45, 0.45)
0x2C    4     Alpha/opacity (float, e.g., 0.5)
0x30    12    Ambient/specular (3 floats, often 0, 0, 0)
0x3C    4     Ambient alpha (float, often 1.0)
0x40    4     Platform size parameter (float, e.g., 35.0)
0x44    4     Flag 1 (uint32, often 1)
0x48    4     Flag 2 (uint32, often 1)
[then texture string reference]
[then face index data]
```

## File Trailer (CONFIRMED from Arena-SpawnPlatform)
After all objects:
```
00 00 00 00  # position zeros
00 00 00 00  
00 00 00 00  
01 00 00 00  # uint32 = 1
02 00 00 00  # uint32 = 2 (same as object count)
1E 00 00 00  # uint32 = 30 (total object/geometry count?)
```

## Arena-SpawnPlatform Example Parse
- File size: 1,476 bytes
- Geometry: mostly zeros (minimal geometry)
- Object count: 2 (N:SINKPLATFORM + E:NODIZZY)
- N:SINKPLATFORM: position at identity, diffuse color 0.45,0.45,0.45, alpha 0.5, size 35.0, texture "PinkChecker.bmp"
- E:NODIZZY<TIME>50</TIME>: identity transform, scale 0.5

## Level1 Example Parse
- Object count: 30 objects
- Types including: START1-1, FLAG04, FLAG07, SAFESPOT (x8), START2-1, START2-2, FLAG02, FLAG06, SAFESPOT (x8+), PLATFORM, CAMERALOOKAT, etc.
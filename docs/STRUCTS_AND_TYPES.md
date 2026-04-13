# Hamsterball - Structures and Types

## Game Data Types (INFERRED from binary and asset analysis)

### 3D Vector Types
```c
// CONFIRMED - observed in MESH and MESHWORLD files
typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y, z, w;  // w often = 1.0 or 0.0
} Vec4;

typedef struct {
    float m[3][3];  // or m[4][3] with padding
} Matrix3x3;
```

### MESH File Header (HIGH CONFIDENCE)
```c
// Based on analysis of Sphere.MESH, Bell.MESH, Hamster.MESH
typedef struct {
    uint32_t version;          // Always 1
    uint32_t name_length;      // Length of mesh name string
    char     name[];            // Null-terminated, padded to name_length
    // Followed by transform/material data
} MeshHeader;

// After header, per-object data:
// Position: Vec3 (3 floats)
// Scale: Vec3 (3 floats, typically 1.0)
// Bounding sphere/box data
// Material properties:
//   - Diffuse color: Vec3 (RGB floats)
//   - Ambient color: Vec3 (RGB floats)  
//   - Specular color: Vec3 (RGB floats)
//   - Specular power: float (typically 25.0)
// Texture name: length-prefixed string (e.g., "HamsterBall.png")
// Then vertex/index data
```

### MESH Vertex Data (CONFIRMED from string references)
```
*MESH_NUMVERTEX - vertex count
*MESH_VERTEX - vertex position data
*MESH_NUMTVERTEX - texture vertex count  
*MESH_TVERT - texture coordinate data
*MESH_NUMFACES - face count
*MESH_FACE - face indices
*MESH_FACENORMAL - face normals
*MESH_VERTEXNORMAL - vertex normals
```
These are 3DS Max ASE format strings — the MESH binary format contains
equivalent data in binary form.

### MESHWORLD Level Format (HIGH CONFIDENCE)
```c
// Based on Arena-SpawnPlatform.MESHWORLD and Arena-Beginner.MESHWORLD analysis
typedef struct {
    uint32_t object_count;  // Number of objects in level
    // Followed by object_count objects
} MeshWorldHeader;

// Each object starts with:
// uint32_t type_string_length;
// char type_string[type_string_length];  // Null-terminated/padded
//   Known types: START1-1, START2-1, START2-2, FLAG02, FLAG04,
//   FLAG06, FLAG07, SAFESPOT, CAMERALOOKAT, PLATFORM,
//   N:SINKPLATFORM, E:NODIZZY<TIME>50</TIME>
//
// Position data: Vec3 (3 floats)
// Orientation: Vec3 (3 floats, Euler angles or direction)  
// Scale: float (1.0 = normal)
// Additional per-type data:
//   - PLATFORM: texture name, mesh reference, physics properties
//   - CAMERALOOKAT: position, look-at target
//   - FLAG: position, checkpoint type
//   - SAFESPOT: position, safe zone properties

// Object types with embedded XML properties:
// N:SINKPLATFORM = platform that sinks after player steps on it
// E:NODIZZY<TIME>N</TIME> = disables dizzy effect for N seconds
// E:GROWSOUND = sound trigger for growth effect
```

### Game Configuration (HS.CFG) - SPECULATIVE
```c
// Based on hex analysis of HS.CFG (1300 bytes)
// Fixed-size records
typedef struct {
    char name[64];      // Null-padded player name
    float scores[?];    // High scores per race
    int32_t flags[?];   // Unlock flags
} PlayerRecord;

// File appears to contain 5 player slots:
// "R. FINK", "SQUEAKS", "HAMMSTURABBI", "PEEPUMS", "MR. RAPTIS"
```

### Race Configuration (RaceData.xml) - CONFIRMED
```xml
<!-- Per-race timing data -->
<RACENAME>
    <TIME>time_limit_seconds</TIME>
    <PAR>par_time_seconds</PAR>
    <WEASEL>weasel_time</WEASEL>      <!-- Best possible time -->
    <GOLD>gold_time</GOLD>            <!-- Gold medal threshold -->
    <SILVER>silver_time</SILVER>      <!-- Silver medal threshold -->
    <BRONZE>bronze_time</BRONZE>      <!-- Bronze medal threshold -->
    <CAM>camera_param</CAM>           <!-- Camera behavior parameter -->
</RACENAME>
```

### Music Configuration (Jukebox.xml) - CONFIRMED
```xml
<SONG>
    <NAME>display_name</NAME>
    <HEX>mo3_position_hex</HEX>  <!-- Pattern position in MO3 file -->
</SONG>
```

### Font Description Format - CONFIRMED
```c
typedef struct {
    uint32_t version;          // = 1
    uint32_t height;           // e.g., 111 (for ShowcardGothic72)
    uint32_t base_char;        // First character code (e.g., 32 = space)
    uint32_t texture_count;    // Number of texture pages
    // ... per-glyph entries with:
    //   char_code (uint32)
    //   x_position (float)     // X in texture atlas
    //   width (float)          // Glyph width in texture
    //   x_offset (float)       // Horizontal offset
    //   y_offset (float)       // Vertical offset
    //   advance (float)        // X advance
    //   texture_index (uint32) // Which texture page
} FontDescription;
```

### Import Table Structures (Addresses)
```
D3D8 (d3d8.dll):
  Direct3DCreate8 @ IAT 0x4CF050

DINPUT8 (DINPUT8.dll):
  DirectInput8Create @ IAT 0x4CF29C

DSOUND (DSOUND.dll):
  DirectSoundCreate @ IAT 0x4CF01C (ordinal import)

BASS (BASS.dll):
  BASS_Init @ IAT 0x4CF04C (string ref 0x4D9238 -> string at VA 0x4D9238)
  BASS_Stop @ IAT 0x4CF050 (same as D3D8 entry area?)
  BASS_Free
  BASS_Start
  BASS_SetConfig
  BASS_ErrorGetCode
  BASS_MusicLoad
  BASS_MusicPlayEx
  BASS_ChannelSetAttributes
  BASS_ChannelStop
```

## Key Global Addresses (from string cross-references)

| Address (VA) | Description | Confidence |
|---|---|---|
| 0x4D9384 | App::Initialize debug strings | CONFIRMED |
| 0x4D8A98 | Graphics::Initialize debug strings | CONFIRMED |
| 0x4D88A8 | Graphics::Defaults debug strings | CONFIRMED |
| 0x4D0634 | BoardLevel3::BoardLevel3 debug strings | CONFIRMED |
| 0x4D5DB0 | "BallPath" string | CONFIRMED |
| 0x4D8708 | "HAMSTERBALL TOURNAMENT!" string | CONFIRMED |
| 0x4D3EAC | "CLICK HERE TO PLAY!" string | CONFIRMED |
| 0x4D03D0 | "RACE TIME:" string | CONFIRMED |
| 0x4D2659 | "DATA\\HS.CFG" path string | CONFIRMED |
| 0x4D8F9E | "%s.mesh" format string | CONFIRMED |
| 0x4D8D80 | "Direct3DCreate8" import name | CONFIRMED |
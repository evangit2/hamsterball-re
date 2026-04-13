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

## SceneObject Structure (vtable 0x4D934C, CONFIRMED from Ghidra decompilation)

```c
// Size: 0xD4 (212 bytes)
// Constructor: SceneObject_ctor at 0x46B4F0
typedef struct {
    void*    vtable;          // +0x000 - Always 0x4D934C
    int      gfxContext;      // +0x004 - Graphics context pointer (param_1 of ctor)
    int      field_08;        // +0x008 - Zeroed
    int      field_0C;        // +0x00C - Zeroed
    int      field_10;        // +0x010 - Zeroed
    int      field_14;        // +0x014 - Zeroed
    int      field_18;        // +0x018 - Zeroed
    int      field_1C;        // +0x01C - Zeroed
    // +0x20 to +0x87: padding / unknown
    char     visible;         // +0x088 - Visibility flag (1=visible, 0=hidden)
    int      zOrder;          // +0x08C - Z-order / object ID (-1 = unregistered)
    float    baseScale[5];   // +0x094 - 4x4 base scale matrix (initialized to identity 1.0,1.0,1.0,1.0)
    float    rotation[5];    // +0x0A8 - 4x4 rotation matrix (initialized to zero)
    float    worldMatrix[5]; // +0x0BC - 4x4 world transform matrix (initialized to zero)
    float    radius;         // +0x0CC - Bounding radius = sqrt(global_constant)
    int      type;            // +0x0D0 - Object type (3 = default)
} SceneObject; // Total: 0xD4

// Scene registration:
// Scene_RegisterObject (0x453BD0): obj->zOrder = id; obj->vtable[3](); scene[0x710 + id*4] = obj;
```

## Vec3 Structure (vtable 0x4CF300, CONFIRMED)

```c
// Size: 20 bytes (0x14)
// Constructor: Vec3_Init at 0x453180
typedef struct {
    void*  vtable;    // +0x000 - 0x4CF300
    float  x;         // +0x004
    float  y;         // +0x008
    float  z;         // +0x00C
    float  w;         // +0x010 - Default 1.0 (0x3F800000)
} Vec3;
```

## Scene Object Offsets (CONFIRMED from Scene_dtor and Scene_Update)

```c
// Scene object (large, ~0xFC8+ bytes based on dtor iteration)
// Scene_dtor at 0x419770 shows these lists:
#define SCENE_STATIC_OBJECTS    0x22E   // Static objects (persistent, updated each frame)
#define SCENE_DYNAMIC_OBJECTS   0x335   // Dynamic objects (moving platforms etc.)
#define SCENE_SUB_OBJECTS       0x43B   // Sub-objects (linked to dynamic, removable)
#define SCENE_LEVEL_ORIG        0x22B   // Original Level object pointer
#define SCENE_LEVEL_CLONE       0x22C   // Cloned Level object pointer
#define SCENE_RESOURCE1          0x21F   // Resource pointer 1
#define SCENE_RESOURCE2         0xE92   // Resource pointer 2
#define SCENE_BALL_LIST         0xA75   // Ball/character list (position updates)
#define SCENE_EXTRA_LIST        0xC81   // Additional object list
#define SCENE_RENDER_LIST       0xD8B   // Render update list
// Scene_Update at 0x419C00:
#define SCENE_FRAME_COUNTER    0xD88   // Incremented each tick
#define SCENE_GAME_STATE        0x708   // 3 = racing
#define SCENE_SCREEN_OFFSET    0xA6E   // Screen offset (decrements 10/frame)
#define SCENE_DEMO_TIMER_ON    0x10D6  // Demo expiration timer active
#define SCENE_DEMO_COUNTDOWN   0x10D7  // Demo countdown frames
#define SCENE_UNPAUSE_FLAG     0x10DA  // Unpause gate
```

## Key Global Addresses (continued)

| Address (VA) | Description | Confidence |
|---|---|---|
| 0x4D8D80 | "Direct3DCreate8" import name | CONFIRMED |
| 0x4D0260 | Scene vtable (36 virtual method entries) | CONFIRMED |
| 0x4D934C | SceneObject vtable (10 virtual method entries) | CONFIRMED |
| 0x4CF300 | Vec3 vtable | CONFIRMED |
| 0x4D9368 | SceneObject secondary vtable (set by BaseDtor) | CONFIRMED |

### Scene Vtable Constants

```c
// Scene vtable at 0x4D0260 - slot offsets
#define SCENE_VT_DELETING_DTOR    0x00  // 0x425020: ~Scene + free
#define SCENE_VT_UPDATE           0x04  // 0x419C00: Scene_Update
#define SCENE_VT_RENDER           0x08  // 0x41A2E0: Scene_Render
#define SCENE_VT_HANDLE_INPUT     0x0C  // 0x4692F0: Menu item input handling
#define SCENE_VT_ACTIVATE_ITEM    0x10  // 0x469220: Activate current item
#define SCENE_VT_SELECT_ITEM      0x18  // 0x469280: Select current item
#define SCENE_VT_CLEAR_ITEM       0x2C  // 0x4692A0: Clear current item ptr
#define SCENE_VT_SAVE_CLEANUP     0x44  // 0x4692B0: Save and cleanup
#define SCENE_VT_INIT_SCENE       0x48  // 0x40B090: Level_InitScene
#define SCENE_VT_NOOP             0x24  // 0x44B840: Default empty stub

// SceneObject vtable at 0x4D934C - slot offsets
#define SOBJ_VT_DTOR              0x00  // 0x46B650: ~SceneObject
#define SOBJ_VT_SET_POSITION      0x04  // 0x46B490: Set position + update
#define SOBJ_VT_SET_SCALE         0x08  // 0x46B4B0: Set scale + update
#define SOBJ_VT_RENDER            0x0C  // 0x46B670: Build world matrix + D3D
#define SOBJ_VT_SET_VISIBLE       0x10  // 0x46B4D0: Toggle visibility
#define SOBJ_VT_DELETING_DTOR     0x1C  // 0x46B9F0: BaseDtor + free

// Scene current item pointer (used by vtable slots 3-6, 11-12)
#define SCENE_CURRENT_ITEM        0x864  // ptr to current menu/scene item
```

### Rumble Board Arena Paths

```c
// 14 arena level paths loaded by RumbleBoard_*_Init functions
"levels\\arena-WarmUp"       // 0x413C20
"levels\\arena-beginner"     // 0x413CE0
"levels\\arena-intermediate" // 0x414180
"levels\\arena-dizzy"       // 0x414240
"levels\\arena-tower"        // 0x4144B0
"levels\\arena-up"          // 0x414960
"levels\\arena-expert"      // 0x414B10
"levels\\arena-Odd"         // 0x414CE0
"levels\\arena-Toob"        // 0x414F00
"levels\\arena-Wobbly"     // 0x4153A0
"levels\\arena-Sky"         // 0x4158C0
"levels\\arena-Master"      // 0x416080
"levels\\arena-neon"        // 0x416F40
"levels\\arena-glass"       // 0x417DF0
"levels\\arena-impossible"  // 0x418540

// Tournament sub-level paths (Board subclasses)
"Levels\\Level8-Spinny"       // Toob Board "Rodenthood"
"Levels\\Level8-Saw"         // Toob Board "Rodenthood"
"Levels\\Level8-Fallout"     // Toob Board "Rodenthood"
"Levels\\Level8-Blockdawg1"  // Toob Board "Rodenthood"
"Levels\\Level8-Blockdawg2"  // Toob Board "Rodenthood"
```
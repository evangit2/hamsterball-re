# Hamsterball File Formats

## MESHWORLD Format (.MESHWORLD)

Binary level format containing object definitions with positions, rotations, and properties.

### Header
- **Bytes 0-3**: Object count (uint32 LE)
- **Bytes 4-7**: First object type string length (uint32 LE)

### Object Types (confirmed from Arena-Beginner.MESHWORLD)
- `START1-1`, `START2-1`, `START2-2` - Player start positions (1P and 2P)
- `FLAG02`, `FLAG04`, `FLAG06`, `FLAG07` - Checkpoint flags
- `SAFESPOT` - Safe landing spot  
- `CAMERALOOKAT` - Camera look-at target (from earlier strings analysis)
- `PLATFORM` - Platform object

### Arena-SpawnPlatform.MESHWORLD Analysis
First 0x22 bytes: Header with object count = 0x22 (34)
Then each object contains:
- 4 bytes: string length
- N bytes: null-terminated type string (e.g., "START2-2\0")
- Floats: position (x, y, z), orientation, scale data
- Per-object variable-length data

Key floats observed:
- `0x3F800000` = 1.0f (scale/identity matrix values)
- `0xC2480000` = -50.0f
- `0x41555555` ≈ 13.33f (y-position values)
- `0x3E2B01F6` ≈ 0.167f (small scale factors)
- `0x3F550000` ≈ 0.832f 
- `0xC2000000` = -32.0f

Special object type: `N:SINKPLATFORM` - sinking platforms
`E:NODIZZY<TIME>50</TIME>` - modifier with XML-like embedded properties

### Sub-objects within MESHWORLD
Each MESHWORLD contains:
1. Vertex data (arrays of floats for positions, normals, UVs)
2. Object definitions with type strings
3. Embedded XML-like properties (e.g., `<TIME>50</TIME>`)
4. Texture references (e.g., "PinkChecker.bmp", "BlackCheckerFlag.png")
5. Face/index data near end of file

## MESH Format (.MESH)

Custom binary 3D mesh format used for game models.

### Header Structure
```
Offset  Size  Description
0x00    4     Version (uint32 LE) = 1
0x04    4     Mesh name length (uint32 LE)
0x08    N     Mesh name (null-padded ASCII)
```

### Post-Header Data (Sphere.MESH)
After name string:
- Position: 3 floats (x, y, z) = (0, 0, 0)
- Scale: 3 floats (1.0, 1.0, 1.0)  
- Unknown: 3 floats (1.0, 1.0, 1.0) - possibly bounding box extents
- Material properties: diffuse color RGB (3 floats), ambient (3 floats), specular (3 floats)
- Specular power: 1 float (25.0)
- More transform/material data

Then texture references:
```
4 bytes: texture name length (uint32 LE)
N bytes: texture filename (null-terminated, e.g., "HamsterBall.png")
```

After textures: vertex data arrays.

### Known Meshes
- `Sphere.MESH` - Default hamster ball
- `Sphere+Tar.MESH` - Ball with tar texture
- `SphereBreak1.MESH`, `SphereBreak2.MESH` - Broken ball fragments
- `Hamster.MESH`, `Hamster-Trot1/2/3.MESH`, `Hamster-Waiting.MESH` - Hamster model + animations
- `Sawblade.MESH`, `SawFace.MESH` - Sawblade obstacle
- Various obstacle models (Fanblades, Chomper, etc.)

## HS.CFG Format (Save/Config)

Binary format, 1300 bytes:
- Fixed-size records with null-padded strings
- Player names: "R. FINK", "SQUEAKS", "HAMMSTURABBI", "PEEPUMS", "MR. RAPTIS"
- Appears to store high scores per race category
- Each record: 64-byte name field + score data

## RaceData.xml Format

XML structure per race:
```xml
<RACENAME>
    <TIME>seconds</TIME>
    <PAR>seconds</PAR>
    <WEASEL>time</WEASEL>
    <GOLD>time</GOLD>
    <SILVER>time</SILVER>
    <BRONZE>time</BRONZE>
    <CAM>float</CAM>  <!-- camera parameter -->
</RACENAME>
```

### Race Categories (in order)
1. BEGINNERRACE (60s, par 47s)
2. CASCADERACE (50s, par 25s)
3. INTERMEDIATERACE (45s, par 35s)
4. DIZZYRACE (40s, par 35s)
5. TOWERRACE (35s, par 35s)
6. UPRACE (30s, par 25s)
7. NEONRACE (30s, par 25s)
8. EXPERTRACE (30s, par 20s)
9. ODDRACE (30s, par 20s)
10. TOOBRACE (25s, par 20s)
11. WOBBLYRACE (25s, par 20s)
12. GLASSRACE (25s, par 10s)
13. SKYRACE (25s, par 5s)
14. MASTERRACE (55s, par 2s)
15. IMPOSSIBLERACE (50s, par 2s)

## Jukebox.xml Format

Music configuration mapping race names to MO3 song indices:
```xml
<SONG> * comment *
    <NAME>display name</NAME>
    <HEX>hex_index</HEX>
</SONG>
```

The HEX value is a position/pattern index into the MO3 music file.

## Font Description Format

Binary font metric file in `Fonts/<name>/font.description`:
- Entry count (uint32)
- Glyph count/character height  
- Per-glyph entries: character code, x position (float), width (float), UV coordinates
- Paired with texture atlas PNGs (Data0.png, Data1.png, etc.)

## MO3 Music Format

`Music/Music.mo3` - MO3 format (BASS library tracker format)
- Contains all game music in a single module
- Indexed by HEX values from Jukebox.xml
- Uses BASS_MusicLoad/BASS_MusicPlayEx for playback

## Sound Effects

All in `Sounds/` directory as OGG Vorbis files.
Loaded via BASS library (BASS_ChannelSetAttributes, BASS_ChannelStop).
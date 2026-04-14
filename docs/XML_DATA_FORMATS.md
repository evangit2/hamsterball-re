# XML Data Formats Specification

## RaceData.xml — Race Configuration

Full game race parameters, loaded at startup.

### Format
```xml
<RACENAME>
    <TIME>seconds</TIME>       <!-- Time pool for tournament -->
    <PAR>seconds</PAR>          <!-- Par time (target completion) -->
    <WEASEL>time</WEASEL>       <!-- Golden weasel (best) medal time -->
    <GOLD>time</GOLD>           <!-- Gold medal time -->
    <SILVER>time</SILVER>       <!-- Silver medal time -->
    <BRONZE>time</BRONZE>       <!-- Bronze medal time -->
    <CAM>float</CAM>            <!-- Camera profile parameter -->
</RACENAME>
```

### Complete Race Data (from game files)

| Race | Time Pool | Par | Weasel | Gold | Silver | Bronze | CAM |
|------|-----------|-----|--------|------|--------|--------|-----|
| BEGINNERRACE | 60 | 47.0 | 6.6 | 15.0 | 10.3 | 7.6 | 2.57 |
| CASCADERACE | 50 | 25.0 | 15.9 | 17.5 | 24.5 | 30.3 | 0.64 |
| INTERMEDIATERACE | 45 | 35.0 | 23.0 | 26.5 | 35.2 | 46.7 | 0.0 |
| DIZZYRACE | 40 | 35.0 | 37.2 | 41.4 | 48.0 | 58.8 | 0.88 |
| TOWERRACE | 35 | 35.0 | 36.5 | 40.0 | 47.8 | 59.2 | 0.03 |
| UPRACE | 30 | 25.0 | 29.7 | 32.0 | 35.1 | 40.8 | 0.33 |
| NEONRACE | 30 | 25.0 | 37.7 | 46.0 | 55.3 | 65.8 | 0.0 |
| EXPERTRACE | 30 | 20 | 34.0 | 39.5 | 48.0 | 61.2 | 0.0 |
| ODDRACE | 30 | 20 | 44.6 | 48.0 | 61.8 | 80.7 | 1.28 |
| TOOBRACE | 25 | 20 | 42.3 | 45.2 | 53.5 | 60.6 | 0.13 |
| WOBBLYRACE | 25 | 20 | 37.0 | 44.0 | 52.1 | 63.8 | 0.0 |
| GLASSRACE | 25 | 10 | 36.0 | 43.5 | 52.1 | 65.0 | 0.71 |
| SKYRACE | 25 | 5 | 40 | 46.0 | 53.5 | 60.0 | 0.44 |
| MASTERRACE | 55 | 2 | 65 | 73.4 | 88.8 | 112.4 | 0.0 |
| IMPOSSIBLERACE | 50 | 2 | 44 | 60.0 | 80.3 | 100.4 | 0.5 |

### Notes
- TIME is tournament time pool (decreasing), not race duration limit
- PAR is the expected completion time; used for tournament scoring
- Medal times: WEASEL < GOLD < SILVER < BRONZE (lower is better)
- CAM value selects camera profile: 0.0 = default, >0 = alternate
- No warm-up race in RaceData.xml (race indices 1-2 aren't competitive)
- Silver in BEGINNERRACE (10.3) < Gold (15.0) — data file has SILVER/GOLD swapped for that race

## Jukebox.xml — Music Track Mapping

Maps track names to MO3 pattern indices.

### Format
```xml
<SONG> * comment *
    <NAME>display name</NAME>
    <HEX>pattern_index</HEX>
</SONG>
```

### Complete Track List

| Context | Name | HEX (Pattern) |
|---------|------|---------------|
| Title Screen | Main Theme | 0x02 |
| Title Screen (no intro) | Main Theme - No Intro | 0x03 |
| Beginner Race | Hamster Nation | 0x50 |
| Beginner Race (no intro) | Hamster Nation - No Intro | 0x50 |
| Cascade Race | Cascade Race | 0x7F |
| Cascade Race (no intro) | Cascade Race - No Intro | 0x81 |
| Intermediate Race | Gerbil Groove | 0x26 |
| Intermediate Race (no intro) | Gerbil Groove - No Intro | 0x28 |
| Dizzy Race | Dizzy! | 0x47 |
| Dizzy Race (no intro) | Dizzy! - No Intro | 0x47 |
| Tower Race | Happy Rush | 0x2F |
| Tower Race (no intro) | Happy Rush - No Intro | 0x2F |
| Up Race | Up Race | 0x8B |
| Up Race (no intro) | Up Race - No Intro | 0x8B |
| Neon Race | Neon Theme | 0xA8 |
| Expert Race | Fight! | 0x08 |
| Expert Race (no intro) | Fight! - No Intro | 0x09 |
| Odd Race | Ninja Hamster | 0x38 |
| Odd Race (no intro) | Ninja Hamster - No Intro | 0x3A |
| Toob Race | Rodenthood | 0x55 |
| Toob Race (no intro) | Rodenthood - No Intro | 0x56 |
| Wobbly Race | Hamster Chase | 0x19 |
| Wobbly Race (no intro) | Hamster Chase - No Intro | 0x1B |
| Sky Race | Bucky Break | 0x5C |
| Glass Race | Glass Theme | 0x9F |
| Sky Race (no intro) | Bucky Break - No Intro | 0x5C |
| Master Race | Master Theme | 0x71 |
| Master Race (no intro) | Master Theme - No Intro | 0x71 |
| Impossible Race | Impossible Theme | 0x94 |
| Game Over | Game Over | 0x62 |
| Tournament Overview | Tournament | 0x63 |
| Goal Reached | Goal! | 0x6B |
| High Scores | High Scores | 0x13 |
| Loading | Loading | 0x6F |

### Notes
- HEX is a BASS_MusicPlayEx position/pattern index into Music.mo3
- "No Intro" variants start from a later pattern in the same song, skipping the intro section
- Some songs share the same HEX for both variants (Hamster Nation 0x50, Dizzy! 0x47, Happy Rush 0x2F, Up Race 0x8B) — these probably have the intro as part of the same pattern or skip it differently
- The MO3 file contains ALL music in a single module — different HEX values select different pattern/sub-song positions

## Font Description Format

Binary font metric file at `Fonts/<name>/font.description`, paired with texture atlas PNGs.

### Structure
```
[4 bytes]  version_or_count = 2
[4 bytes]  glyph_height = 11 (for ShowcardGothic72)
[4 bytes]  char_height = 48 (pixel height)
```

Then per-glyph entries (mixed int/float, approximately 20 bytes each):
- Character code
- X position in atlas (float)
- Character width (float)
- UV coordinates for texture mapping
- Atlas page index (Data0.png vs Data1.png)

### Font Atlas
Each font has texture atlas PNG files:
- `Data0.png` — Primary glyph atlas
- `Data1.png` — Secondary atlas (for larger fonts needing more glyphs)

### Known Fonts

| Directory | Height (px) | Atlases | Usage |
|-----------|-------------|---------|-------|
| ShowcardGothic72 | 72 | Data0.png | Timer display (large numbers) |
| ShowcardGothic28 | 28 | Data0.png | Title screens |
| ShowcardGothic16 | 16 | Data0.png | HUD text |
| ShowcardGothic14 | 14 | Data0.png | Medium dialogue |
| ArialNarrow12Bold | 12 | Data0.png | Small labels |

### Rendering
```cpp
// Font render from decompiled code
Graphics_DrawText(font, text, x, y):
  For each character in text:
    Lookup glyph in font.description → UV coords + width
    Draw textured quad from atlas at (x, y)
    x += glyph.width
```

## Reimplementation Notes (SDL2)

### RaceData
- Same XML format, load with tinyxml2 or pugixml
- Medal times determine rank display after race
- CAM value maps to camera profile selection

### Jukebox
- MO3 format: use BASS library with libbass, or convert to individual .ogg/.wav tracks
- For SDL_mixer: extract each pattern as separate .ogg during asset conversion
- Pattern index → track file mapping table

### Fonts
- Replace with TTF fonts (Showcard Gothic is a standard Windows font, freely available)
- SDL2: `TTF_RenderText_Blended()` for anti-aliased text
- For pixel-perfect match: render Showcard Gothic TTF at same sizes
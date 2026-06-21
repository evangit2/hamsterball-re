# Font & Text Rendering System

## Overview
Hamsterball uses a custom bitmap font system with a binary descriptor format.
Fonts are loaded from `font.description` files with associated `dataN.png` glyph
atlases. The font renderer supports both 2D and 3D text rendering.

## Font Loading (LoadFont 0x457130)

### font.description Binary Format
```
Header:
  uint32 page_count;              // Number of glyph atlas pages
  uint32 glyph_count;            // Number of glyph definitions

Pages (page_count entries):
  // Each page: "%s\\data%d.png" → Graphics_FindOrCreateTexture()
  // Textures loaded as PNG files, alpha-enabled

Glyphs (glyph_count entries):
  uint8  char_code;               // ASCII character code (1 byte)
  uint32 x_advance;               // Horizontal advance pixels (4 bytes)
  uint8[4] unknown1;              // Unknown field (4 bytes)
  uint8[4] unknown2;              // Unknown field (4 bytes)
  uint8[4] unknown3;             // Unknown field (4 bytes)
  float  height;                  // Glyph height (4 bytes)
  uint32 unknown4;               // Unknown (4 bytes)
  uint32 unknown5;               // Unknown (4 bytes)
  uint32 unknown6;               // Unknown (4 bytes)
  // Total: 33 bytes per glyph entry
```

### FontList Structure (large, ~0x1490 bytes)
| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | vtable* | FontList_ScalarDtor (0x4D8E30) |
| +0x04 | void* | Graphics device pointer (param_1) |
| +0x08 | AthenaList | Texture pages |
| +0x420 | int | Glyph count |
| +0x424 | int | Max glyph height (updated during load) |
| +0x428 | float | Scale (default 1.0f = 0x3F800000) |
| +0x42C | char[5120] | Per-character flags (0x500 * 4 = 0x1400) — glyph availability |
| +0x43C | void*[256] | Per-character sprite slots (256 * 0x14) |
| +0x434 | int[256] | Per-character x position offsets |
| +0x10E0 | int[256] | Per-character y position offsets |
| +0x430 | int[256] | Per-character widths |

### Font Load Process
1. Zero-initialize glyph table (0x500 * 4 bytes at +0x42C)
2. Open `{font_dir}\\font.description` with `_open(path, 0x8000)` (O_RDONLY)
3. Read page_count and glyph_count from header
4. For each page: load `{font_dir}\\data{N}.png` via Graphics_FindOrCreateTexture
5. For each glyph: read 33 bytes, call Gadget_AddSpriteSlot with glyph params
6. Track max height at FontList+0x424
7. Close file

## Font Rendering

### Font_DrawGlyph (0x457440)
Renders a string character by character:
```
for each char in text:
  if char == '\n':
    y_offset += line_height (+0x424)
    x_position = start_x
  elif glyph_available[char] (+0x42C offset):
    if scale == 1.0:
      Sprite_DrawRect(glyph_sprite, x + glyph_x_offset, y + glyph_y_offset)
    else:
      Scene_CreateObject4f(glyph_sprite, x, y, 
                           scaled_width, scaled_height, ...)
  advance x by glyph width
```

### Font_DrawCentered (0x42C870)
8 xrefs — renders text centered at position:
1. Measure text width with Font_MeasureText
2. Center x = position - (width / 2)
3. Call Font_DrawGlyph at centered position

### Font_MeasureText (0x456E20)
56 xrefs — measures pixel width of text string:
1. Sum per-character x_advance values
2. Account for scale factor
3. Return total width in pixels

### Font_DrawGlyph3D (0x457690)
1 xref — renders text in world-space 3D coordinates.
Used for floating text in game world (goal markers, score displays).

### Font_WordWrap (0x456E80)
3 xrefs — word-wraps text to fit within a maximum width.
Inserts newline characters where needed.

### Font_RenderToTextureComplex (0x472340)
4 xrefs — renders text to an off-screen texture for compositing.
Used for complex text rendering (shadows, outlines, gradients).

### Font_RenderChannels (0x4A91D0)
2 xrefs — renders font glyph channels (for font building).

## Font Processing Pipeline (SDF Builder)

The game includes a full SDF (Signed Distance Field) font builder:
- **Font_DecodeGlyphBits** (0x4AD716) — Decode glyph bitmap from font description
- **Font_IncrementGlyphMask** (0x4B4CF2) — Advance glyph mask for coverage
- **Font_ComputeGlyphBounds** (0x4B4D5E) — Compute bounding box of glyph
- **Font_SplitGlyphQuadrants** (0x4B5096) — Quadtree split for SDF generation
- **Font_ComputeGlyphCentroid** (0x4B51E9) — Compute glyph center of mass
- **Font_ComputeGlyphCoverage** (0x4B5321) — Compute pixel coverage percentage
- **Font_CompositeGlyph** (0x4B60EE) — Composite glyph onto final texture

This pipeline generates the `dataN.png` glyph atlas files from TrueType fonts
during development. The runtime only loads the pre-built atlases.

## Font Resources

### Font Files (5 fonts loaded in App_ResourceLoader)
| App Offset | Path | Point Size | Usage |
|-----------|------|-----------|-------|
| +0x318 | fonts\showcardgothic28 | 28pt | Title text, HUD |
| +0x31C | fonts\showcardgothic14 | 14pt | Small labels, scores |
| +0x320 | fonts\showcardgothic16 | 16pt | Medium labels |
| +0x324 | fonts\arialnarrow12bold | 12pt | UI body text |
| +0x328 | fonts\showcardgothic72 | 72pt | Large display text |

Each font directory contains:
- `font.description` — Binary font descriptor (header + glyph defs)
- `data0.png`, `data1.png`, ... — Glyph atlas pages

## Font Drawing API

| Address | Function | Description |
|---------|----------|-------------|
| 0x457130 | LoadFont | Load font from font.description file |
| 0x457440 | Font_DrawGlyph | Render text string (2D) |
| 0x457690 | Font_DrawGlyph3D | Render text in 3D world space |
| 0x42C870 | Font_DrawCentered | Center-aligned text rendering |
| 0x456E20 | Font_MeasureText | Measure pixel width of text |
| 0x456E80 | Font_WordWrap | Word-wrap text to max width |
| 0x472340 | Font_RenderToTextureComplex | Render to off-screen texture |
| 0x4A91D0 | Font_RenderChannels | Render glyph channels |
| 0x4AD716 | Font_DecodeGlyphBits | Decode glyph bitmap |
| 0x4B4CF2 | Font_IncrementGlyphMask | Advance glyph mask |
| 0x4B4D5E | Font_ComputeGlyphBounds | Compute glyph bounding box |
| 0x4B5096 | Font_SplitGlyphQuadrants | Quadtree split for SDF |
| 0x4B51E9 | Font_ComputeGlyphCentroid | Compute glyph center |
| 0x4B5321 | Font_ComputeGlyphCoverage | Compute pixel coverage |
| 0x4B60EE | Font_CompositeGlyph | Composite glyph onto atlas |
| 0x473940 | FontFormatString_Parse | Parse printf-style format strings |
| 0x475390 | Menu_AddFont | Add font to menu system |
| 0x472570 | MeshWorld_BuildFontMeshes | Build 3D text meshes from font data |

## Format String Parsing (FontFormatString_Parse)

Parses printf-style format strings with `%d`, `%s`, `%f`, etc.
Uses CRT_ParseFormatString (0x4F7820) for format specifier extraction.

Supports format flags: digits, `.`, `-`, `+`, `#`, ` ` (standard printf).

## Menu Text Positioning

Menu items use Matrix_Scale4x4 for text scaling:
- Each item has a 4-component scale: (X1, X2, Y1, Y2)
- (1.0, 1.0, 1.0, 1.0) = normal size
- (0.75, 1.0, 0.75, 1.0) = 75% width for emphasized items
- (1.0, 0.5, 0.5, 1.0) = half Y-size for "FRENZIED!" difficulty
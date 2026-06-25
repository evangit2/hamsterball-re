# Level Base Color System

## Overview

Each race/arena level in Hamsterball has a **base color** — an RGB triplet hardcoded
in the BoardLevel constructor. This color is NOT stored in the MESHWORLD file. It
controls the tint of:

1. **Timer oval background** (`timerblot.png` quad tinted with the level color)
2. **Timer text** (the countdown clock numbers, rendered with `showcardgothic72` font)
3. **Arena score display oval** (same mechanism, different position/size)

When the timer is running low, the G and B channels are zeroed (leaving only R),
making the timer turn **red** as a visual warning.

## Where the Color Is Set

Each `BoardLevel*_ctor` function calls `Vec3_Init` with three float constants, then
writes the result to the board struct:

```c
// Example: BoardLevel1_WarmUp_ctor (0x0041CA40)
iVar1 = Vec3_Init(local_20, 0x3f800000, 0, 0x3f800000);  // R=1.0, G=0.0, B=1.0 = Magenta
*(board + 0x1508) = *(iVar1 + 4);   // R
*(board + 0x150C) = *(iVar1 + 8);   // G
*(board + 0x1510) = *(iVar1 + 0xC); // B
*(board + 0x1514) = *(iVar1 + 0x10); // A (always 1.0 from Vec3_Init)
Matrix_Identity(local_20);
```

### Board Struct Fields

| Offset | Type | Description |
|--------|------|-------------|
| +0x1508 | float | Base color R |
| +0x150C | float | Base color G |
| +0x1510 | float | Base color B |
| +0x1514 | float | Base color A (always 1.0) |
| +0x4340 | float | Render scale factor (always 1.0, set in `Board_ctor` at 0x00419030) |

## Where the Color Is Used

### Timer HUD — `Scene_RenderTimerHUD` (vtable[0x1C] @ 0x0041BFD0)

This is Scene vtable slot 0x1C (offset 0x70 in vtable), called during the overlay
render pass (after opaque + translucent passes).

**1P race mode:**
```c
// Read level color from board
R = *(float*)(board + 0x1508);
G = *(float*)(board + 0x150C);
B = *(float*)(board + 0x1510);
Scale = *(float*)(board + 0x4340);  // 1.0

// Draw timer oval (timerblot.png quad, tinted with level color)
Scene_CreateObject4f(gfx, &timer_vtable, x, 10.0, 180.0, 105.0,
    &Vec3_dtor, R, G, B, Scale);

// Build color matrix for text
Matrix_Scale4x4(&matrix, 1.0, 1.0, 1.0, Scale);  // Normal: full color

// Time warning thresholds:
if (time < 0x44C) {  // ~1100ms = 1.1 seconds
    Matrix_Scale4x4(&matrix, 1.0, 0, 0, Scale);  // Zero G,B → RED
}
if (time < 600) {  // 600ms = 0.6 seconds
    Matrix_Scale4x4(&matrix, 1.0, 0, 0, Scale);  // Keep RED
}

// Draw timer text (showcardgothic72 font)
UI_DrawTextCentered(font72, formatted_time, ..., R, G, B, Scale);
// Draw decimal text (showcardgothic28 font, smaller)
UI_DrawTextShadow_Wrapper(font28, ".N", ..., R, G, B, Scale);
```

**2P race mode:** Same mechanism, draws two timer displays (one per viewport).

### Arena Score HUD — `Scene_RenderScoreHUD` (vtable[0x1B] @ 0x0041B710)

This is Scene vtable slot 0x1B (offset 0x6C in vtable), called during the post-FX
render pass.

```c
// Arena mode score display also uses the level color
R = *(float*)(board + 0x1508);
G = *(float*)(board + 0x150C);
B = *(float*)(board + 0x1510);
Scale = *(float*)(board + 0x4340);

// Draw score oval (timerblot.png quad, tinted with level color)
Scene_CreateObject4f(gfx, &timer_vtable, 460.0, 17.0, 80.0, 46.67,
    &Vec3_dtor, R, G, B, Scale);

// Score text uses WHITE (1,1,1) with level color only on the oval
Matrix_Scale4x4(&matrix, 1.0, 1.0, 1.0, Scale);
```

### Level Name Text — NOT Level-Colored

The level name string (e.g., "WARM-UP RACE") is stored at `board+0x29B4` and rendered
in the ScoreHUD with **hardcoded white text** and a **black shadow**:

```c
// Background rect: semi-transparent (alpha=0.75)
Matrix_Scale4x4(&matrix, 1.0, 1.0, 0, 0.75);
UI_DrawRectAndReset(gfx);

// Shadow: black (0,0,0,1.0)
// Main text: white (1,1,1,1.0)
Matrix_Scale4x4(&matrix, 1.0, 1.0, 1.0, 1.0);
UI_DrawTextCenteredAbsolute(font28, level_name, x+400, 0x82, 5, 5, ...);
```

## Timer Oval Texture

The oval behind the timer uses **`timerblot.png`** (loaded at `App+0x390` by
`App_ResourceLoader` at 0x004298C0). This is a white/neutral texture that gets
tinted by the level color via D3D8 MODULATE blending (texture × vertex diffuse color).

Related textures also loaded but used elsewhere:
- `blueblot.png` (App+0x354) — alternative blot, possibly for 2P indicator
- `blueblot2.png` (App+0x358) — another blot variant
- `bluecircle.png` (App+0x35C) — circle overlay

## Color Table — Race Levels

| Level | MESHWORLD | Ctor Address | R | G | B | RGB255 | Color |
|-------|-----------|-------------|---|---|---|--------|-------|
| Warm-Up | Level1 | 0x0041CA40 | 1.0 | 0.0 | 1.0 | (255,0,255) | Magenta/Pink |
| Intermediate | Level2 | 0x0041CB20 | 0.0 | 0.0 | 1.0 | (0,0,255) | Blue |
| Dizzy | Level3 | 0x0041D060 | 0.0 | 1.0 | 0.0 | (0,255,0) | Green |
| Tower | Level4 | 0x0041E340 | 1.0 | 0.75 | 0.0 | (255,191,0) | Orange |
| Expert | Level8 | 0x0041EA40 | 1.0 | 0.0 | 0.0 | (255,0,0) | Red |
| Odd | Level9 | 0x0041ED80 | 1.0 | 0.5 | 0.0 | (255,128,0) | Orange |
| Wobbly | Level7 | 0x0041F110 | 0.62 | 0.84 | 0.30 | (158,214,77) | Yellow-Green |
| Toob | Level10 | 0x0041F4B0 | 0.5 | 0.5 | 1.0 | (128,128,255) | Light Blue |
| Sky | Level12 | 0x0041F930 | 0.0 | 0.5 | 1.0 | (0,128,255) | Sky Blue |
| Beginner | LevelCascade | 0x004200E0 | 1.0 | 0.75 | 0.25 | (255,191,64) | Gold |
| Up | LevelUp | 0x00420390 | 1.0 | 0.0 | 1.0 | (255,0,255) | Magenta/Pink |
| Master | Level13 | 0x004206D0 | 0.5 | 0.5 | 0.5 | (128,128,128) | Gray |
| Neon | LevelDark | 0x00424440 | 1.0 | 1.0 | 0.0 | (255,255,0) | Yellow |
| Impossible | LevelImpossible | 0x00424C20 | 1.0 | 0.0 | 0.0 | (255,0,0) | Red |

## Color Table — Arena Levels

Arena levels use the same mechanism (same struct offset, same `Vec3_Init` pattern).
Each RumbleBoard constructor sets the color identically to its race counterpart.

| Arena | Ctor Address | R | G | B | RGB255 | Color |
|-------|-------------|---|---|---|--------|-------|
| Warmup Arena | 0x004224A0 | 1.0 | 0.0 | 1.0 | (255,0,255) | Magenta/Pink |
| Beginner Arena | 0x00422550 | 1.0 | 0.75 | 0.25 | (255,191,64) | Gold |
| Intermediate Arena | 0x004226E0 | 0.0 | 0.0 | 1.0 | (0,0,255) | Blue |
| Dizzy Arena | 0x00422790 | 0.0 | 1.0 | 0.0 | (0,255,0) | Green |
| Tower Arena | 0x004228C0 | 1.0 | 0.75 | 0.0 | (255,191,0) | Orange |
| Expert Arena | 0x00423060 | 1.0 | 0.0 | 0.0 | (255,0,0) | Red |
| Odd Arena | 0x00423220 | 1.0 | 0.5 | 0.0 | (255,128,0) | Orange |
| Toob Arena | 0x004234E0 | 0.5 | 0.5 | 1.0 | (128,128,255) | Light Blue |
| Wobbly Arena | 0x00423690 | 0.62 | 0.84 | 0.30 | (158,214,77) | Yellow-Green |
| Sky Arena | 0x00423BF0 | 0.0 | 0.5 | 1.0 | (0,128,255) | Sky Blue |
| Master Arena | 0x00424380 | 0.5 | 0.5 | 0.5 | (128,128,128) | Gray |
| Neon Arena | 0x00424860 | 1.0 | 1.0 | 0.0 | (255,255,0) | Yellow |

## Floor Checker Textures (Separate System)

The floor checker/brick textures are a **separate** color system — they are NOT
controlled by board+0x1508. Instead, each level's MESHWORLD file references textures
by name, and the game loads them all at startup into App struct slots:

| App Offset | Texture File | Used By |
|------------|-------------|---------|
| +0x2C8 | pinkchecker.bmp | Warm-Up, Up |
| +0x2CC | bluechecker.bmp | Intermediate |
| +0x2D0 | bluebrick.png | Intermediate (brick variant) |
| +0x2D4 | greenchecker.bmp | Dizzy |
| +0x2D8 | greenbrick.png | Dizzy (brick variant) |
| +0x2DC | yelllowchecker.png | Neon |
| +0x2E0 | greyoutlinechecker.png | Master |
| +0x2E4 | redchecker.bmp | Tower, Expert, Impossible |
| +0x2E8 | redbrick.png | Tower/Expert (brick variant) |
| +0x2EC | orangechecker.bmp | Odd, Beginner |
| +0x2F0 | orangebrick.png | Odd/Beginner (brick variant) |
| +0x2F4 | brightgreenchecker.bmp | Wobbly |
| +0x2F8 | brightgreenbrick.png | Wobbly (brick variant) |
| +0x2FC | toobchecker.png | Toob |
| +0x300 | toobbrick.png | Toob (brick variant) |
| +0x304 | skychecker.png | Sky |
| +0x308 | purplechecker.bmp | (unused?) |
| +0x30C | purplebrick.png | (unused?) |
| +0x310 | brownbrick.png | (unused?) |
| +0x314 | blackchecker.png | (unused?) |

The MESHWORLD material section (`*BITMAP` entries in the ASCII format, or texture
name strings in the binary format) references these textures by filename. The floor
color comes from the texture itself, not from the board struct.

## MESHWORLD Colors (Section 4)

The MESHWORLD binary format DOES contain color data in Section 4:

| Offset | Type | Description |
|--------|------|-------------|
| MW+0x45C | float[3] | Background color (R,G,B) — skybox/clear color |
| MW+0x468 | float[3] | Ambient light color (R,G,B) — scene-wide ambient |

These are read by `Scene_LoadMeshWorld` (0x00461890) via `__read(file, MW+0x45C, 0x18)`
(24 bytes = 6 floats). They control:

- **Background color**: The D3D8 clear color / skybox tint
- **Ambient light**: The minimum illumination for all lit surfaces

These are **completely independent** from the board+0x1508 timer oval color.

## Race Selection Menu Colors (Separate System)

The race selection menu (Practice/Time Trial) uses **separate hardcoded colors**
in `PracticeMenu_ctor` (0x0042EA30). Each `UIList_AddItem` call is preceded by a
`Matrix_Scale4x4` that sets the text color for that item. These colors are **NOT**
the same as board+0x1508 — they are independently defined and generally brighter
(more pastel) versions of the level colors.

| Level | Menu RGB255 | Board RGB255 | Same? |
|-------|-------------|-------------|-------|
| Warm-Up | (255,191,255) | (255,0,255) | No |
| Beginner | (255,191,64) | (255,191,64) | Yes |
| Intermediate | (191,191,255) | (0,0,255) | No |
| Dizzy | (191,255,191) | (0,255,0) | No |
| Tower | (255,229,115) | (255,191,0) | No |
| Up | (191,115,255) | (255,0,255) | No |
| Neon | (255,255,0) | (255,255,0) | Yes |
| Expert | (255,64,64) | (255,0,0) | No |
| Odd | (255,191,0) | (255,128,0) | No |
| Toob | (191,191,255) | (128,128,255) | No |
| Wobbly | (156,240,74) | (158,214,77) | No |
| Glass | (255,191,255) | (255,0,255) | No |
| Sky | (64,191,255) | (0,128,255) | No |
| Master | (217,184,69) | (128,128,128) | No |
| Impossible | (255,0,0) | (255,0,0) | Yes |

**Conclusion:** Changing board+0x1508 would NOT affect the race selection menu text
colors. Those are separate hardcoded values in `PracticeMenu_ctor` at 0x0042EA30.
Locked levels all use gray (0x3f266666 = ~0.65) for all channels.

The menu also loads level preview images (`practice-level1.png` through
`practice-impossible.png`) stored as Sprite objects at menu+0xCDC through +0xD14.

## Summary

| Color Source | Where Set | What It Controls |
|-------------|------------|-------------------|
| Board ctor `Vec3_Init` | board+0x1508/0x150C/0x1510 | Timer oval tint, timer text tint, arena score oval tint |
| PracticeMenu_ctor `Matrix_Scale4x4` | Per-item in UIList | Race selection menu text color (NOT same as board color) |
| MESHWORLD Section 4 | MW+0x45C (bg) / MW+0x468 (ambient) | Background clear color, ambient scene lighting |
| MESHWORLD `*BITMAP` | Per-material texture reference | Floor checker/brick texture (the actual pixel colors) |
| MESHWORLD `*MATERIAL_DIFFUSE` | Per-material diffuse color | Mesh surface diffuse color |
| MESHWORLD `*MATERIAL_AMBIENT` | Per-material ambient color | Mesh surface ambient color |

The timer oval color and level text color are **hardcoded per-level in the EXE** —
they cannot be changed by editing the MESHWORLD file. To change them, you must
either patch the EXE's `Vec3_Init` call arguments or use a DLL mod to overwrite
board+0x1508/0x150C/0x1510 at runtime after the board constructor runs. The race
selection menu text colors are a **separate set** of hardcoded values in
`PracticeMenu_ctor` — changing board+0x1508 will NOT change the menu text.

## Key Function Addresses

| Function | Address | Description |
|----------|---------|-------------|
| `Scene_RenderTimerHUD` | 0x0041BFD0 | Draws timer oval + timer text (race mode) |
| `Scene_RenderScoreHUD` | 0x0041B710 | Draws arena score oval + level name |
| `Scene_CreateObject4f` | 0x00418870 | Creates tinted quad (used for timer oval) |
| `Gfx_DrawQuadRandomColor` | 0x0045D450 | Low-level quad renderer with color |
| `Board_ctor` | 0x00419030 | Base board ctor (sets scale=1.0) |
| `App_ResourceLoader` | 0x004298C0 | Loads timerblot.png and all textures |
| `Vec3_Init` | (inline) | Initializes 4-float vector from 3 components |

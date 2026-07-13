# mkn_plus_xtreme_organization

**Author:** MAKYUNI 🦇  
**Contributors:** Hamsterbot  
**HB+ API Version:** v2.1 (HAMSTERBALL_API_VERSION 3)  
**Build:** MinGW cross-compile (nocrt, no msvcrt dependency)

## Description

Xtreme File Organization is a comprehensive file redirection mod that lets you customize virtually every file path the game uses. It works by patching PUSH instruction immediates in Hamsterball.exe — each instruction loads a string pointer (like `"levels\level1"`), and this mod replaces those pointers with your custom paths.

## Features

- **Ignore Cache** — When enabled (default), the game will not read or write `.cached` files. This ensures the game always loads the original MESHWORLD files directly, never cached versions.
- **Custom Level Files** — Redirect all 15 Tournament race MESHWORLD files to custom paths/filenames.
- **Custom Arena Files** — Redirect all 15 Rumble mode arena MESHWORLD files.
- **Custom Mesh Files** — Redirect 30 mesh files (8Ball, FunBall, Sphere, Hamster animations, Chomper, Sawblade, etc.).
- **Custom Object Files** — Redirect 54 level object files (Swirl, Bridge, Windmill, Pendulum, Rotator, etc.).
- **Custom Tournament Thumbnails** — Redirect all 15 tournament menu thumbnail images.
- **Custom Practice Thumbnails** — Redirect all 15 time trials menu thumbnail images.

## Installation

1. Place `mkn_plus_xtreme_organization.dll` in your `Mods\` folder (inside the HB+ game directory).
2. Place `mkn_plus_xtreme_organization.txt` in the same `Mods\` folder.
3. The mod auto-generates the config file with defaults on first run if it doesn't exist.

## Configuration

Edit `mkn_plus_xtreme_organization.txt`:

```
ignore_cache = true
level01_file = levels\level1
arena01_file = levels\arena-WarmUp
mesh_file_8ball = Meshes\8Ball
obj_file_swirl = Levels\Level3-Swirl
img_file_tourneythumb01 = tourney-beginner.png
img_file_practicethumb01 = Textures\practice-level1.png
```

### Config Keys

| Key | Description |
|-----|-------------|
| `ignore_cache` | `true` = disable .cached file reading/writing |
| `level01_file` through `level15_file` | Tournament race MESHWORLD paths |
| `arena01_file` through `arena15_file` | Rumble mode arena MESHWORLD paths |
| `mesh_file_*` | Individual mesh file paths (30 entries) |
| `obj_file_*` | Level object file paths (54 entries) |
| `img_file_tourneythumb01` through `15` | Tournament menu thumbnails |
| `img_file_practicethumb01` through `15` | Time Trials menu thumbnails |

### In-Game Options

The mod adds an "Xtreme Organization" submenu in the HB+ options menu with:
- **Ignore Cache Files** toggle (YES/NO)

## Technical Details

The mod patches ~130 PUSH imm32 instructions in Hamsterball.exe. Each instruction is 5 bytes (`68 XX XX XX 00`) and loads a string pointer. The mod overwrites bytes [1..4] (the string pointer) with the address of a custom string allocated in the DLL's memory.

Cache disabling works by patching 3 locations:
1. `MeshWorld_ctor` (0x0046F439): `JZ → JMP` to always skip cache file reading
2. `Mesh_SaveAndFree` (0x0046F67E): `JE → JMP` to skip cache file writing
3. `Mesh_SaveAndFree` (0x0046F684): `JNZ → JMP` to skip cache flag check

## Build

```bash
cd source/
bash build.sh
```

Requires `i686-w64-mingw32-g++` (MinGW cross-compiler).

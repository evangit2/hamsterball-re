# Drop-in Levels Folder (Beginner ↔ Intermediate Swapped)

This is a **complete copy of the game's `Levels/` folder** with Beginner Race and Intermediate Race MESHWORLD files swapped.

## What's swapped

| File | Original (game) | This folder |
|------|-----------------|-------------|
| `LevelCascade.MESHWORLD` | Beginner Race (239 KB) | **Intermediate Race** (1 MB) |
| `Level3.MESHWORLD` | Intermediate Race (1 MB) | **Beginner Race** (239 KB) |

All other files are identical to the original game. Level3 sub-meshes (Gluebie, Swirl, Tipper, WaterWheel) are unchanged — they're referenced by internal string names, not filenames.

## Usage

1. Back up your game's `Levels/` folder (rename to `Levels_backup/`)
2. Copy this entire `levels/` folder as the game's `Levels/` folder
3. Install `mods/rsks-jit/bass.dll` as the game's `bass.dll` (back up original as `bass_real.dll`)
4. Create `C:\tmp\` directory
5. Launch game → select **Beginner Race** → you'll get Intermediate Race geometry
6. Read `C:\tmp\ref_loader_log.txt` for the full JIT injection trace

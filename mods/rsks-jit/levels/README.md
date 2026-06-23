# Drop-in Levels Folder (Beginner ↔ Intermediate Swapped)

Complete copy of the game's `Levels/` folder with Beginner Race and Intermediate Race MESHWORLD files swapped.

## What's swapped

| File | Original (game) | This folder |
|------|-----------------|-------------|
| `Level2.MESHWORLD` | Beginner Race (788 KB) | **Intermediate Race** (1026 KB) |
| `Level3.MESHWORLD` | Intermediate Race (1026 KB) | **Beginner Race** (788 KB) |
| `LevelCascade.MESHWORLD` | Dizzy Race — restored to original (unchanged) | 239 KB |

All other files unchanged. Level3 sub-meshes (Gluebie, Swirl, Tipper, WaterWheel) stay in place.

## Usage

1. Back up your game's `Levels/` folder
2. Copy this entire `levels/` folder as the game's `Levels/` folder
3. Install `mods/rsks-jit/bass.dll` as the game's `bass.dll` (back up original as `bass_real.dll`)
4. Create `C:\tmp\` directory
5. Launch game → select **Beginner Race** → you'll get Intermediate Race geometry
6. Read `C:\tmp\ref_loader_log.txt` for the full JIT injection trace

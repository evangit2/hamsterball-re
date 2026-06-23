# Swapped MESHWORLD Levels for JIT Logging Test

This folder contains pre-swapped MESHWORLD files for testing the JIT mesh injection DLL.

## What's swapped

### BeginnerRace/ — Intermediate Race geometry loaded as Beginner Race
- `LevelCascade.MESHWORLD` — actually Level3.MESHWORLD (Intermediate Race, 1026 KB)
- `Level3-Gluebie.MESHWORLD` — Intermediate sub-mesh
- `Level3-Swirl.MESHWORLD` — Intermediate sub-mesh
- `Level3-Tipper.MESHWORLD` — Intermediate sub-mesh
- `Level3-WaterWheel.MESHWORLD` — Intermediate sub-mesh

Copy these into the game's `Levels/` folder. When you select **Beginner Race** in-game, you'll get Intermediate Race geometry.

### IntermediateRace/ — Beginner Race geometry loaded as Intermediate Race
- `Level3.MESHWORLD` — actually LevelCascade.MESHWORLD (Beginner Race, 239 KB)

Copy this into the game's `Levels/` folder. When you select **Intermediate Race** in-game, you'll get Beginner Race geometry.

## Usage

1. Back up your original `Levels/` folder
2. Copy the contents of `BeginnerRace/` into the game's `Levels/` folder
3. Install `mods/rsks-jit/bass.dll` as `bass.dll` (back up original first)
4. Create `C:\tmp\` directory
5. Launch game, select Beginner Race
6. Read `C:\tmp\ref_loader_log.txt` for full JIT injection trace

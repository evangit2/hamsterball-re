# Drop-in Levels Folder (Beginner ↔ Intermediate Swapped)

Complete copy of the game's `Levels/` folder with Beginner Race and Intermediate Race swapped.

## What's swapped

| File | Original | This folder |
|------|----------|-------------|
| `LevelCascade.MESHWORLD` | Beginner Race | **Intermediate Race** (Level2 data) |
| `Level2.MESHWORLD` | Intermediate Race | **Beginner Race** (LevelCascade data) |

Everything else is original. No `.cached` or `.bak` files included.

## CRITICAL: Delete .cached files in YOUR game folder!

The game caches pre-processed level data in `.cached` files. **If any `.cached` files exist in your game's `Levels/` folder, the game will use the cached version and IGNORE your swapped `.MESHWORLD` files.**

Before copying this folder:

1. Go to your game's `Levels/` folder
2. **Delete ALL `*.cached` files** (e.g. `level1.cached`, `level2.cached`, `Level2-Bridge.cached`, etc.)
3. **Delete ALL `*.bak` files** (e.g. `Level1.MESHWORLD.bak`)
4. **Delete the entire `Levels/` folder** (or rename to `Levels_backup/`)
5. Copy this entire `levels/` folder as the new `Levels/` folder
6. Do NOT run the game before installing the bass.dll mod — running the game regenerates `.cached` files from whatever `.MESHWORLD` files are present

## Level mapping (for reference)

| Race name | Game filename |
|-----------|--------------|
| Warm-up Race | `Level1.MESHWORLD` |
| Beginner Race | `LevelCascade.MESHWORLD` |
| Intermediate Race | `Level2.MESHWORLD` |
| Dizzy Race | `Level3.MESHWORLD` |
| Tower Race | `Level4.MESHWORLD` |
| Up Race | `LevelUp.MESHWORLD` |
| Expert Race | `Level8.MESHWORLD` |
| Odd Race | `Level9.MESHWORLD` |
| Toob Race | `Level10.MESHWORLD` |
| Wobbly Race | `Level7.MESHWORLD` |
| Glass Race | `LevelGlass.MESHWORLD` |
| Sky Race | `LevelImpossible.MESHWORLD` |
| Master Race | `LevelDark.MESHWORLD` |
| Neon Race | (check EXE strings) |
| Impossible Race | (check EXE strings) |

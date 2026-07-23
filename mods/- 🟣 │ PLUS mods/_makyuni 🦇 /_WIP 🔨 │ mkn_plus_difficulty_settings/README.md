# Difficulty Settings (HB+ Mod)

Difficulty-based level file redirect. Redirects MESHWORLD files based on tournament difficulty.

## Installation

Place `mkn_plus_difficulty_settings.dll` in the `Mods\` folder.

## Features

- **Pipsqueak (Tournament)**: Loads `level1-easy.meshworld` instead of `level1.meshworld`
- **Frenzied (Tournament)**: Loads `level1-hard.meshworld`
- **Normal (Tournament)**: Uses default level files
- **Practice/Party**: Tries `-hard` variant, falls back to default if missing
- **Entity gates**: NOPs difficulty gates so 8-balls and Mousetraps spawn on all difficulties

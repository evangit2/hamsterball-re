# Animated Textures Mod

A Hamsterball Plus API mod that adds runtime texture animation support. Reads `.txt` config files from the `Textures/` folder and cycles through texture frames at runtime.

## How It Works

1. At mod load, scans `Textures/*.txt` for files containing `framerate` keyword
2. For each config file (e.g. `arrowanim.txt`), scans for matching frame files (`arrowanim_01.png`, `arrowanim_02.png`, etc.)
3. At `onLevelStart`, finds the base texture in the game's Graphics texture cache
4. Loads all additional frames using the game's own texture loader
5. Background thread swaps the `IDirect3DTexture8*` pointer in the D3DTexture object at the configured framerate

## Config File Format

Place in `Textures/` folder next to the game exe. Filename must match the texture base name.

```
framerate = 0.5
looptype = 1
```

- **framerate**: Seconds between frame swaps (float)
- **looptype**: 
  - `0` = Play once, stop on last frame
  - `1` = Loop (wrap back to first frame)
  - `2` = Ping-pong (reverse direction at ends)

## File Naming Convention

- Config: `Textures/arrowanim.txt`
- Frame 1: `Textures/arrowanim_01.png` (referenced by MESHWORLD)
- Frame 2: `Textures/arrowanim_02.png`
- Frame 3: `Textures/arrowanim_03.png`
- etc.

The MESHWORLD references the first frame (`arrowanim_01.png`). The mod loads frames 2+ and cycles through them.

## Technical Details

- Uses the game's own texture loader (Graphics+0x2E4 cache, function at RVA 0x55C50) to load frames
- Swaps `IDirect3DTexture8*` at D3DTexture+0x04 in a background thread (16ms tick)
- Thread-safe: uses `IsBadReadPtr`/`IsBadWritePtr` guards, restores original texture on scene end
- Supports up to 16 concurrent animations, 32 frames each
- Frame files sorted alphabetically before loading

## Build

Compile as 32-bit DLL using Visual Studio with the Hamsterball Plus API template. Place the resulting `.dll` in the game's `Mods/` folder.

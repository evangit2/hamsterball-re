# Animated Textures

Cycle between numbered texture variants at a custom framerate. When the game renders an object using any texture from an animation sequence, the mod automatically substitutes the next frame in the sequence at the configured speed.

## How It Works

1. The user places `.txt` config files in the game's `Textures/` folder
2. Each `.txt` filename (without extension) becomes the **animation prefix**
3. The mod scans `Textures/` for files named `prefix_NN.ext` (e.g. `exampleanimtex_01.png`, `exampleanimtex_02.bmp`)
4. Any texture using one of these frames gets animated — the mod swaps the D3D8 texture pointer at render time via a `SetTexture` vtable hook

## Config File Format

Create a `.txt` file in the `Textures/` folder. The filename determines the animation prefix.

**Example:** `Textures/exampleanimtex.txt`

```
framerate = 1
looptype = 1
```

| Setting | Type | Description |
|---------|------|-------------|
| `framerate` | float | Frames to advance per tick. `1` = advance every tick, `0.5` = every 2 ticks, `2` = double speed |
| `looptype` | int | `0` = play once and stop on last frame, `1` = loop forever, `3` = ping-pong (forward then reverse) |

## Texture File Naming

Textures must be named `prefix_NN.ext` where `NN` is the frame number (starting from 01):

```
Textures/
  exampleanimtex.txt          ← config file (prefix = "exampleanimtex")
  exampleanimtex_01.png       ← frame 1
  exampleanimtex_02.png       ← frame 2
  exampleanimtex_03.bmp       ← frame 3
  ...
```

- Supported formats: `.png`, `.bmp`, `.jpg`, `.tga` (anything the game's D3DXCreateTextureFromFileA supports)
- Frame numbers start at 01 and go up
- Frames are sorted numerically, not alphabetically
- All frames should be the same dimensions

## How to Use

1. Install `bass.dll` and `bass_real.dll` in the game folder (rename original `bass.dll` to `bass_real.dll` first)
2. Create texture frames and name them `prefix_01.png`, `prefix_02.png`, etc.
3. Create a config `.txt` file named `prefix.txt` in the `Textures/` folder
4. In the MESHWORLD level file, reference any frame (e.g. `exampleanimtex_01`) as the texture for an object
5. When the game renders that object, the mod will cycle through all frames automatically

## Technical Details

- **Hook target:** `Graphics_BeginFrame` (0x453B50) — installs D3D8 vtable hook on first frame + advances animation timers
- **D3D8 vtable hook:** `SetTexture` (vtable[61]) — intercepts every texture bind, substitutes the current animation frame
- **Texture loading:** Uses the game's own `LoadTexture` function (0x455C50) to load all frames into the texture cache at startup
- **Texture cache monitoring:** Rescans when texture count changes (level load)
- **No threads, no IAT hooks** — runs entirely in the game's render thread
- **Android/Wine safe** — no threads, no IAT hooks, vtable patching only

## Files

| File | Description |
|------|-------------|
| `animated_textures.c` | Source code |
| `bass.dll` | Compiled proxy DLL |
| `bass_real.dll` | Original BASS library (rename original bass.dll) |

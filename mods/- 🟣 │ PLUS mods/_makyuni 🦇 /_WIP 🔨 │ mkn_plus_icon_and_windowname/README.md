# mkn_plus_icon_and_windowname

**Author:** MAKYUNI  
**Type:** Hamsterball Plus (HB+) mod  
**Ported from:** bass.dll proxy mod `icon_and_windowname`

## Description

Customizes the Hamsterball window title and icon:

1. **Custom window title** — replaces the "Hamsterball" window name with text from config
2. **Custom icon** — searches for `icon.ico` in the game root folder; if found, replaces the runtime window icon via WM_SETICON

The original bass.dll proxy mod used a background thread to poll for the window handle. In the HB+ port, this polling is done via `onGameUpdate()` callbacks instead.

## Installation

1. Place `mkn_plus_icon_and_windowname.dll` in the Hamsterball `Mods\` folder
2. On first launch, the mod auto-generates `mkn_plus_icon_and_windowname.txt` in the same folder
3. Edit the text file to change the window title
4. To change the game icon, place a file named `icon.ico` in the game root folder (next to `Hamsterball.exe`)

## Config Format

```
# Icon & Window Name Mod Configuration
# Lines starting with # are comments

# Custom window title (shown in title bar)
# Leave as Hamsterball for default
window_name = Hamsterball

# To change the game icon, place a file named
# icon.ico in the game root folder (next to Hamsterball.exe)
```

## Build

```bash
cd source && bash build.sh
```

Requires `i686-w64-mingw32-g++` (MinGW cross-compiler).

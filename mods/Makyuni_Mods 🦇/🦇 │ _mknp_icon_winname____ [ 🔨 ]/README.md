# _mknp_icon_winname (HB+ v2.1)

Custom icon and window title mod for Hamsterball Plus.

## Features

- **Custom window title**: `window_name` from the config is applied to the game window title bar (re-applied if anything changes it).
- **Custom icon**: Place `icon.ico` in the game root folder. The mod automatically:
  - Applies it at runtime via `WM_SETICON` (immediate effect)
  - Permanently updates `Hamsterball.exe`'s RT_ICON resources (takes effect on next restart)
- **No icon.ico?**: Game uses the original Hamsterball icon. Nothing changes.

## Installation (Hamsterball Plus required)

1. Place `_mknp_icon_winname.dll` in the game's `Mods\` folder.
2. Launch the game. The mod auto-loads via HB+ and auto-generates `_mknp_icon_winname.txt` next to the DLL.
3. To change the icon: place a file named `icon.ico` in the game folder (next to `Hamsterball.exe`).
4. To change the window title: edit `_mknp_icon_winname.txt` next to the DLL in `Mods\`.

## Configuration

Edit `_mknp_icon_winname.txt` (next to the DLL in `Mods\`, NOT the game root):

```ini
# Custom window title (shown in title bar)
window_name = Hamsterball

# To change the game icon, place a file named
# icon.ico in the game root folder (next to Hamsterball.exe)
```

**window_name**: Any text string. Default: `Hamsterball`. Read from the DLL-side txt every run.

**Icon**: No config option — just drop `icon.ico` next to `Hamsterball.exe`. If the file exists, it replaces both the runtime window icon and the permanent .exe icon. If it doesn't exist, the original icon is used.

### Permanent .exe icon update

The .exe is locked while the game runs, so the mod:
1. Copies `Hamsterball.exe` → `Hamsterball.exe.tmp`
2. Updates RT_ICON resources on the temp copy via `UpdateResourceA`
3. Schedules replacement via `MoveFileExA(MOVEFILE_DELAY_UNTIL_REBOOT)` (Windows replaces on next restart)
4. If no admin: writes `update_icon.bat` — double-click after closing the game

The state file `.icon_state.txt` (game root) tracks whether the .exe was already updated, so it only runs once.

## Files

- `_mknp_icon_winname.dll` — compiled HB+ mod
- `_mknp_icon_winname.cpp` — source code (MinGW, nocrt, manual 17-entry vtable)
- `_mknp_icon_winname.txt` — auto-generated config file (lives next to the DLL)

## Building

- **MinGW (Linux):** `cd source && ./build.sh` → `_mknp_icon_winname.dll`
- Uses `nocrt` + manual 17-entry vtable + `hbplus_api.h` (MinGW-safe pattern, same as mknp_entities_plus).

## Testing note

The HB+ loading chain (scanning `Mods\`, calling `CreateModInstance`, firing `Initialize()`) cannot be verified under Wine/hbtestd (that environment uses a plain bass proxy, not the HB+ framework). Only real Windows with HB+ installed can confirm the full load + in-game behavior.

## Author

MAKYUNI / Hamsterbot

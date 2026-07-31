# icon_and_windowname

Custom icon and window title mod for Hamsterball.

## Features

- **Custom window title**: Set any text as the window title bar name via config.
- **Custom icon**: Place `icon.ico` in the game root folder. The mod automatically:
  - Applies it at runtime via `WM_SETICON` (immediate effect)
  - Permanently updates `Hamsterball.exe`'s RT_ICON resources (takes effect on next restart)
- **No icon.ico?**: Game uses the original Hamsterball icon. Nothing changes.

## Installation

1. Rename your original `bass.dll` to `bass_real.dll`
2. Copy the modded `bass.dll` into the game folder
3. Run the game once — it will auto-generate `icon_and_windowname.txt`
4. To change the icon: place a file named `icon.ico` in the game folder (next to `Hamsterball.exe`)
5. To change the window title: edit `icon_and_windowname.txt`

## Configuration

Edit `icon_and_windowname.txt`:

```ini
# Custom window title (shown in title bar)
window_name = Hamsterball

# To change the game icon, place a file named
# icon.ico in the game root folder (next to Hamsterball.exe)
```

**window_name**: Any text string. Default: `Hamsterball`

**Icon**: No config option — just drop `icon.ico` next to `Hamsterball.exe`. If the file exists, it replaces both the runtime window icon and the permanent .exe icon. If it doesn't exist, the original icon is used.

### Permanent .exe icon update

The .exe is locked while the game runs, so the mod:
1. Copies `Hamsterball.exe` → `Hamsterball.exe.tmp`
2. Updates RT_ICON resources on the temp copy via `UpdateResourceA`
3. Schedules replacement via `MoveFileExA(MOVEFILE_DELAY_UNTIL_REBOOT)` (Windows replaces on next restart)
4. If no admin: writes `update_icon.bat` — double-click after closing the game

The state file `.icon_state.txt` tracks whether the .exe was already updated, so it only runs once.

## Files

- `bass.dll` — compiled mod proxy
- `icon_and_windowname.c` — source code
- `icon_and_windowname.txt` — auto-generated config file

## Crash Test

Passed: 38.9s, no crash (Wine/Xvfb, hbtestd).

# icon_and_windowname

Custom icon and window title mod for Hamsterball.

## Features

- **Custom window title**: Replace the "Hamsterball" text in the window title bar with any custom name.
- **Custom icon**: Load a custom `.ico` file to replace the game's default icon.
  - **Runtime**: Icon appears immediately via `SendMessage(WM_SETICON)`
  - **Permanent**: The `.exe` file's RT_ICON resources are rewritten using `UpdateResourceA`, so the new icon shows in Explorer/taskbar permanently. Only runs once per unique icon path.

## Installation

1. Rename your original `bass.dll` to `bass_real.dll`
2. Copy the modded `bass.dll` into the game folder
3. Run the game once — it will auto-generate `icon_and_windowname.txt` next to `bass.dll`
4. Edit `icon_and_windowname.txt` to set your custom values
5. Restart the game

## Configuration

Edit `icon_and_windowname.txt`:

```ini
# Custom window title (shown in title bar)
window_name = Hamsterball

# Path to a custom .ico file to replace the game icon
# Use full path or relative path (relative to the game .exe)
icon_path = C:\icons\my_icon.ico
```

- **window_name**: Any text string. Default: `Hamsterball`
- **icon_path**: Full or relative path to a `.ico` file. Leave empty to keep the default icon.

### How the permanent .exe icon update works

1. On launch, the mod checks `.icon_state.txt` (next to `bass.dll`) for the last icon path written.
2. If `icon_path` in the config differs from the state file (or no state file exists), the mod:
   - Parses the `.ico` file (ICONDIR/ICONDIRENTRY format)
   - Extracts each image size and writes it as RT_ICON resources (IDs 1..N) via `UpdateResourceA`
   - Rebuilds the RT_GROUP_ICON "MAINICON" resource to reference them
   - Commits the update via `EndUpdateResourceA`
   - Saves the new path to `.icon_state.txt`
3. If the `icon_path` hasn't changed since last run, the `.exe` update is skipped (already done).
4. The runtime `WM_SETICON` call still happens for the current session.

**Note**: On Windows, `UpdateResourceA` requires write access to the `.exe` file. If the game is running from a read-only directory or under UAC protection, the permanent update will silently fail — the runtime `WM_SETICON` still works for the current session.

## RE Details

| Component | Address | Description |
|----------|---------|-------------|
| Window name string | `0x4D39A8` | "Hamsterball" |
| Window name store | `0x42AEE6` | `mov [esi+0x20], imm32` — stores string pointer |
| CreateWindowExA call | `0x46BA69` | Uses `[esi+0x20]` as lpWindowName |
| LoadIconA call | `0x46D1FA` | Loads MAINICON during RegisterClassA |
| RegisterClassA call | `0x46D224` | Registers "AthenaWindow" class |
| Icon resource (RT_ICON) | `0x538210` | First icon data entry (.rsrc) |
| Icon resource (RT_ICON) | `0x538AB8` | Second icon data entry (.rsrc) |
| App struct HWND field | `App+0x08` | Window handle set after CreateWindowExA |

## Crash Test

Passed: 38.8s, no crash (Wine/Xvfb, hbtestd).

## Files

- `bass.dll` — compiled mod proxy
- `icon_and_windowname.c` — source code
- `icon_and_windowname.txt` — auto-generated config file

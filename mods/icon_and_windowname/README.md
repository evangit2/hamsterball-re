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

The .exe is locked while the game is running, so `BeginUpdateResourceA` on the live .exe fails silently. The mod uses this approach instead:

1. **Copy** `Hamsterball.exe` → `Hamsterball.exe.tmp`
2. **Update resources** on the temp copy via `UpdateResourceA`
3. **Schedule replacement** via `MoveFileExA(MOVEFILE_DELAY_UNTIL_REBOOT)` — Windows replaces the .exe on next restart (requires admin)
4. **Fallback**: If `MoveFileEx` fails (no admin), writes `update_icon.bat` next to the game — double-click after closing the game to apply the icon change

When you change `icon_path` in the config:
- On next launch, the mod detects the path changed (via `.icon_state.txt`)
- It creates the temp copy with updated resources
- Schedules the swap (or writes the .bat)
- On **restart**, the new icon appears in Explorer/taskbar permanently

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

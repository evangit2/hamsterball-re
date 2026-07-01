# icon_and_windowname

Custom icon and window title mod for Hamsterball.

## Features

- **Custom window title**: Replace the "Hamsterball" text in the window title bar with any custom name.
- **Custom icon**: Load a custom `.ico` file to replace the game's default icon in the taskbar and title bar.

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

## How It Works

### Window Name
The game stores the window title string pointer at `App+0x20`. At address `0x42AEE6`, the code does:
```
mov dword [esi+0x20], 0x4D39A8  ; "Hamsterball"
```
This mod patches the 4-byte immediate value to point to a user-configurable string buffer, so when `CreateWindowExA` reads `App+0x20` as the `lpWindowName` parameter, it uses the custom name.

### Icon
The game loads its icon via `LoadIconA(hInstance, "MAINICON")` during `RegisterClassA`. After the game window is created (detected by polling `App+0x08` for the HWND), the mod calls `SendMessage(hwnd, WM_SETICON, ICON_BIG/ICON_SMALL, hIcon)` with an icon loaded from the user-specified `.ico` file path via `LoadImageA`.

## RE Details

| Component | Address | Description |
|----------|---------|-------------|
| Window name string | `0x4D39A8` | "Hamsterball" |
| Window name store | `0x42AEE6` | `mov [esi+0x20], imm32` — patches the pointer |
| CreateWindowExA call | `0x46BA69` | Uses `[esi+0x20]` as lpWindowName |
| LoadIconA call | `0x46D1FA` | Loads MAINICON during RegisterClassA |
| RegisterClassA call | `0x46D224` | Registers "AthenaWindow" class |
| Icon resource (RT_ICON) | `0x538210` | First icon data entry (.rsrc) |
| Icon resource (RT_ICON) | `0x538AB8` | Second icon data entry (.rsrc) |
| App struct HWND field | `App+0x08` | Window handle set after CreateWindowExA |

## Crash Test

Passed: 38.9s, no crash (Wine/Xvfb, hbtestd).

## Files

- `bass.dll` — compiled mod proxy
- `icon_and_windowname.c` — source code
- `icon_and_windowname.txt` — auto-generated config file

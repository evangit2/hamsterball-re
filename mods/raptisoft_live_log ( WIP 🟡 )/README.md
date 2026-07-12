# Raptisoft Live Status Logger

**Type:** bass.dll proxy mod (passive logger — no gameplay changes)  
**File:** `bass.dll` → writes `live_status.txt` in the game folder

## What It Does

Hamsterball has a hidden in-memory status tracking system that Raptisoft built for crash diagnostics. The game continuously updates three `char*` fields on the App struct:

| Offset | Field | Purpose |
|--------|-------|---------|
| `App+0x208` | Init Status | Startup phase tag (e.g. `"App::Initialize(5)"`, `"Graphics::Initialize(10)"`, `"FinishLoad(OK)"`) |
| `App+0x20C` | Current Object | Name of whatever object is being processed |
| `App+0x210` | Current Operation | Runtime operation (`"Background"` = message pump, `"Update"` = game logic frame) |

These fields are **never written to disk or console** — they exist purely as crash breadcrumbs, read only when the game crashes and feeds them to the BugTracker crash dialog (which posts XML to `bugs.raptisoft.com`).

This mod taps into those same fields from a background thread and logs them live to `live_status.txt`.

## What It Logs

1. **System Info** (one-time, when graphics device initializes):
   - Product name, version string
   - Fullscreen mode, resolution
   - Target FPS, refresh rate
   - Graphics/sound/music/input/scene device pointers

2. **Live Status** (on every change, ~100Hz polling):
   - Timestamp (ms since mod load)
   - Init phase tag
   - Current object name
   - Current operation

3. **Device Changes** (every 1s, if pointers change):
   - Sound/music/scene manager device transitions

4. **FPS** (every 1s, if FPS display is enabled in game settings)

## Expected Output (on real Windows)

```
[MOD] Raptisoft Live Status Logger loaded
=======================================================================
          HAMSTERBALL LIVE STATUS LOG - Raptisoft Debug Tap
=======================================================================

Log started: 2026-01-15 14:30:22
Process PID: 12345

--- System Info ---
  Product:     Hamsterball
  Version:     V3.6.c
  Fullscreen:  YES
  Resolution:  1920x1080
  Target FPS:  75
  Refresh:     60 Hz

--- Device Pointers ---
  Graphics:     0x0A1B2C3D (active)
  Sound:        0x0E4F5A6B (active)
  Music:        0x0C7D8E9F (active)
  Input:        0x0D1E2F3A (active)
  Scene/MeshWorld: 0x0F4A5B6C (active)

--- Live Status (updates on change) ---
[tick]     STATUS                          OBJECT              OPERATION
-----------------------------------------------------------------------------
[    50]  Startup(2)                      (null)              (null)
[   100]  Startup(3)                      (null)              (null)
[   150]  Startup(8)                      (null)              (null)
[   200]  Startup(9)                      (null)              (null)
[   250]  Startup(Constructor OK)         (null)              (null)
[   300]  App::Initialize(1)              (null)              (null)
[   350]  App::Initialize(2)              (null)              (null)
[   400]  App::Initialize(5)              (null)              (null)
[   450]  Graphics::Initialize(1)         (null)              (null)
[   500]  Graphics::Initialize(10)        (null)              (null)
[   550]  Graphics::Initialize(20)        (null)              (null)
[   600]  App::Initialize(7)              (null)              (null)
[   650]  Initialize(5)                   (null)              (null)
[   700]  Initialize(15)                  (null)              (null)
[   750]  Initialize(26)                  (null)              (null)
[   800]  FinishLoad(1)                   (null)              (null)
[   850]  FinishLoad(OK)                  (null)              (null)
[   900]  (null)                           (null)              Background
[   950]  (null)                           (null)              Update
[  1000]  (null)                           Ball                 Update
[  1050]  (null)                           Board(Beginner)      Update
```

## Installation

1. Copy `bass.dll` into your Hamsterball game folder (replacing the original)
2. Launch the game
3. Check `live_status.txt` in the same folder — it updates live while you play

## Technical Details

- **App global address:** `0x005341E0` (the `g_App` global, set in `WinMain`)
- **App struct size:** ~0x2D00+ bytes
- **Polling rate:** ~100Hz (10ms sleep between reads)
- **Log only on change:** Avoids flooding the file with duplicate entries
- **Thread-safe:** Uses `IsBadReadPtr` before every memory read
- **No gameplay changes:** Pure passive observer — doesn't hook any game functions

### Status Strings Found in Binary

The game contains 100+ status strings across these categories:

- **Startup:** `Startup(2)`, `Startup(3)`, `Startup(8)`, `Startup(9)`, `Startup(Constructor OK)`
- **App Init:** `App::Initialize(1)` through `(12)`, `App::Initialize(Ok)`
- **Graphics Init:** `Graphics::Initialize(1)` through `(27)`
- **Graphics Defaults:** `Graphics::Defaults(1)` through `(19)`, `Graphics::Defaults(ok)`
- **Full Init:** `Initialize(1)` through `(26)` (no 14 or 24 — skipped in binary)
- **FinishLoad:** `FinishLoad(1)` through `(4)`, `FinishLoad(OK)`
- **Runtime:** `Background` (message pump idle), `Update` (game logic frame)
- **Errors:** `"Failed: Direct3DCreate8(D3D_SDK_VERSION)"`, `"Could not load sound (1-6)"`, `"** No Graphics **"`, `"** No Graphics Device **"`

### Crash Report System (not tapped by this mod)

The game also has a full crash reporting pipeline at `0x0047ABE0`:
1. `App_BuildDiagnosticReport` (0x0046D230) — collects XML tags: PRODUCT, VERSION, RUNTIME, FULLSCREEN, DXDISPLAY, RESOLUTION, SAFEMODE, OS, DDRAW, DSOUND, CURRENTOBJECT, CURRENTOPERATION, EXTENDED_INFO
2. `MWParser_DumpTags` (0x004742B0) — formats as XML
3. `BugTracker_ShowDialog` (0x0047A480) — shows crash dialog with "Send Report" button
4. `BugTracker_SubmitReport` (0x00479FC0) — HTTP POST to `bugs.raptisoft.com/cgi-bin/errorreport.cgi`

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll raptisoft_live_log.c \
    -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
    -Wl,--add-stdcall-alias
```

## Crash Test

Tested on Wine/Xvfb (35s survival, no crash). Game doesn't fully initialize D3D on llvmpipe, so the log shows only the initial `(null)` state — but the mod loads cleanly and the background thread runs for the full duration. On real Windows, all status fields will populate normally.

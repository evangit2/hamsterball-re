# Widescreen UI Fix

A Hamsterball Plus mod that corrects UI stretching when running the game in widescreen resolutions. Fixes in-game HUD (timer, position indicator, scores, ready/set/go) in all game modes while keeping the 3D environment full widescreen. Menus are intentionally left untouched.

## Problem

Hamsterball's UI uses `D3DFVF_XYZRHW` (pre-transformed) vertices. The X coordinates are computed by `Gfx_TransformY` (0x453e90) using a scale factor derived from the backbuffer dimensions. On widescreen resolutions, this scale factor stretches all UI elements horizontally. The projection matrix is **not** involved — UI vertices bypass it entirely.

## Solution

Hook `Gfx_TransformY` directly and apply a linear transform to its return value:

```
corrected = result * scaleFactor + margin
```

Where:
- `scaleFactor = (4/3) / screenAspect` — compression ratio
- `margin = (bbWidth - bbHeight * 4/3) / 2` — pillarbox offset

This preserves screen center (centered elements like the timer blot stay centered) while compressing left/right-aligned elements into a 4:3 region.

## Game Mode Support

| Mode | Fixed? | Notes |
|------|--------|-------|
| Tournament (1P) | ✅ | Scene_Render → SetViewport(0,0) → ArenaBoard_Render |
| Time Trial (1P) | ✅ | Same path |
| Party Race (2P) | ✅ | Split-screen: per-player viewports use non-zero params, UI pass uses (0,0) |
| Rodent Rumble (arena) | ✅ | ArenaBoard has its OWN vtable — does NOT use Scene_Render |
| Menus | ❌ (intentional) | Board pointer is null, transform stays off |

## How It Works

Three hooks work together:

1. **`onGameUpdate` (App_FrameUpdate, 0x46C170)** — resets `g_inUIPass = false` every frame. Prevents the transform from leaking into pause menus and post-match screens, especially in Rodent Rumble where Scene_Render never fires.

2. **`Graphics_SetViewport` (0x454f10) hook** — after the original runs, if params are `(0,0)` and an active game board exists (App→Profile→Board chain), sets `g_inUIPass = true` and computes the scale factor and margin from backbuffer dimensions. The board-existence check skips menus.

3. **`Gfx_TransformY` (0x453e90) hook** — when `g_inUIPass` is true, transforms the return value to pillarbox the UI. Safe because `Gfx_TransformY` is only called by `Sprite_DrawRect` and similar UI-only functions — 3D rendering never goes through it.

4. **`Scene_Render` (0x41a2e0) hook** — resets `g_inUIPass` before and after the render call for race modes. Not strictly needed since `onGameUpdate` handles the reset, but kept as a safety net.

### Why ArenaBoard is special

ArenaBoard (Rodent Rumble) has its own vtable (e.g. 0x4D1680 for Dizzy Arena) that does **not** contain `Scene_Render` (0x41A2E0). The arena render path is completely separate — it calls `ArenaBoard_Render` (vtable[28] = 0x421910) and the fade overlay (vtable[27] = 0x41B710) directly, without going through `Scene_Render`. This is why the Scene_Render hook alone never fires in Rodent Rumble.

## Controls

Toggle in Options menu: **Widescreen UI Fix** (default: ON)

## Build

### Visual Studio (primary)
Compile `WidescreenUIFix.cpp` as a 32-bit DLL, place in game's `Mods\` folder.

### MinGW cross-compile (alternative)
```bash
i686-w64-mingw32-g++ -shared -o WidescreenUIFix.dll WidescreenUIFix_MinGW.cpp nocrt.cpp \
  -I. -O2 -msse2 -mfpmath=sse -mwindows \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -fno-asynchronous-unwind-tables -fno-unwind-tables \
  -nostdlib -nostartfiles \
  -lkernel32 -luser32 \
  -Wl,-e,_DllMain@12 -Wl,--enable-stdcall-fixup \
  -Wl,--image-base,0x10000000 -Wl,--gc-sections \
  -ffunction-sections -fdata-sections \
  -fpermissive -fno-builtin \
  -Wl,--exclude-symbols,_strcmp -Wl,--exclude-symbols,_strlen \
  -Wl,--exclude-symbols,_memcpy -Wl,--exclude-symbols,_memset \
  -Wl,--exclude-symbols,_malloc -Wl,--exclude-symbols,_free
```

The MinGW build requires three fixes (all included):
1. **nocrt** (`nocrt.h`/`nocrt.cpp`) — eliminates msvcrt.dll dependency
2. **Manual 16-entry vtable** — fixes MinGW/MSVC ABI mismatch
3. **`hbplus_api.h` wrapper** — fixes IModAPI vtable dispatch

## Key Addresses

| Address | Function | Purpose |
|---------|----------|---------|
| 0x453e90 | Gfx_TransformY | UI X coordinate transform (hooked) |
| 0x454f10 | Graphics_SetViewport | Viewport + projection setup (hooked) |
| 0x41a2e0 | Scene_Render | Race mode render dispatcher (hooked) |
| 0x421910 | ArenaBoard_Render | Arena UI render (called by arena vtable[28]) |
| 0x5341E0 | App (global) | Board existence check via App→Profile→Board |
| gfx+0x5c | presentParams ptr | → +0x15c=bbWidth, +0x160=bbHeight |

## Version History

- **v12** — Per-frame reset via `onGameUpdate` to stop arena UI transform leaking into pause menus
- **v11** — Board-existence check to skip menus (App→Profile→Board chain)
- **v10** — Removed Scene_Render guard to fix Rodent Rumble (arena has separate vtable)
- **v9** — Fixed Party Race/Rodent Rumble split-screen (any (0,0) call, not just 2nd)
- **v8** — Hook Gfx_TransformY directly instead of modifying global scale factors
- **v1-v7** — Failed approaches: projection matrix override, scale factor writes, offset modification

## Author

BookwormKevin

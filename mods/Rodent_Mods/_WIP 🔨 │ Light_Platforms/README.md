# Light Platforms Mod

Hijacks Neon Race's NeonPlatform appear/disappear system. Instead of the native timer driving platform visibility, an external flag controls whether platforms are appearing (visible) or disappearing (hidden).

## How it works

NeonPlatform objects (vtable 0x004D5A10) are spawned by the Neon Race board and stored in the dynamic objects list (board+0x2578). Each frame, their update function (vtable[11] = 0x0043E260) moves the platform up (appearing) or down (disappearing) based on:

- `obj+0x10E5` = direction flag (0=appearing, 1=disappearing)
- `obj+0x0439` = active flag (1=updating, 0=idle)

This mod scans the dynamic objects list for NeonPlatform objects and writes the direction flag based on an external `g_platforms_visible` flag.

When merged with Electric Lights: `g_platforms_visible = (charge > 0)`.

## Current behavior (standalone testing)

Toggles every 10 seconds (600 frames at 60fps) for testing. When merged with Electric Lights, this will be driven by charge level instead.

## Hook

Hooks `Graphics_RenderScene` entry (0x454BC0) — runs before board update so the direction flag is set before platforms update.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll light_platforms.c \
  -I"../_WIP 🔨 │ shared" -lwinmm -Wl,--enable-stdcall-fixup -O2 \
  -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

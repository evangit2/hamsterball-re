# sharp_textures

Force sharp (point/nearest) texture filtering for all game textures.

## What It Does

Hamsterball uses D3D8 `SetTextureStageState` with `D3DTEXF_LINEAR` (smooth/blurry) by default. This mod hooks `Graphics_BeginFrame` (0x453B50) — called every frame — and forces `D3DTEXF_POINT` (sharp/nearest) filtering on all 8 texture stages. This makes floor textures, checker patterns, and UI textures pixel-sharp instead of blurred.

## Installation

1. Rename original `bass.dll` to `bass_real.dll` in your Hamsterball folder
2. Copy this mod's `bass.dll` into the game folder
3. Run the game

## Config

A `sharp_textures.txt` file is auto-generated next to `bass.dll` on first run:

```
# Sharp Textures config
# Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic

MAGFILTER = 1
MINFILTER = 1
MIPFILTER = 1
```

Change values to `2` for linear (smooth/blurry, the game's default) or `3` for anisotropic filtering.

## Technical Details

- **Hook target:** `Graphics_BeginFrame` at 0x453B50 (7-byte prologue detour)
- **D3D8 device:** Retrieved from `Graphics+0x154` each frame
- **API call:** `IDirect3DDevice8::SetTextureStageState` (vtable[63], offset 0xFC)
- **States controlled:** `D3DTSS_MAGFILTER` (16), `D3DTSS_MINFILTER` (17), `D3DTSS_MIPFILTER` (18)
- **Stages:** All 8 D3D8 texture stages (0-7)
- **Safe for Android/Wine:** No IAT hooks, no background threads, just a detour + per-frame vtable calls

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll sharp_textures.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

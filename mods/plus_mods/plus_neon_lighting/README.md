# Neon Lighting (HB+ Mod)

Recreates the Neon Race darkness + glow lighting effect in all levels.

## How It Works

The mod applies three D3D8 rendering changes every frame when enabled:

1. **Black fog** — D3DRS_FOGENABLE + D3DRS_FOGTABLEMODE=LINEAR with configurable near/far distances. This creates the dark environment where distant geometry fades to black.
2. **D3D lighting** — Enables D3DRS_LIGHTING with a dark ambient color (0x141414), so unlit surfaces are nearly black.
3. **Point light at ball** — A D3DLIGHT_POINT light positioned at the player's ball, with configurable RGB color and range. This creates the "glow" effect where geometry near the ball is lit and everything else fades into darkness.

When toggled off, all original render states are restored.

## Controls

### Toggle
- **Neon Lighting Effect** — ON/OFF

### Sliders
| Slider | Default | Range | Description |
|--------|---------|-------|-------------|
| Neon Fog Start | 150 | 10–3000 | Distance where fog begins (closer = darker) |
| Neon Fog End | 600 | 100–10000 | Distance where fog reaches full black |
| Neon Light Range | 400 | 50–3000 | How far the point light reaches |
| Neon Light Red | 1.0 | 0–2 | Red component of the ball light |
| Neon Light Green | 1.0 | 0–2 | Green component of the ball light |
| Neon Light Blue | 0.0 | 0–2 | Blue component of the ball light |

### Recommended Settings
- **Classic Neon (yellow glow)**: R=1.0, G=1.0, B=0.0, Fog=150/600
- **Red Neon**: R=1.0, G=0.0, B=0.0, Fog=100/500
- **Blue Neon**: R=0.0, G=0.0, B=1.0, Fog=200/800
- **Green Neon**: R=0.0, G=1.0, B=0.0, Fog=150/600

## Technical Details

- **D3D device access**: App(0x5341E0) → +0x174 (graphics) → +0x154 (IDirect3DDevice8*)
- **Render states**: FOGENABLE(28), FOGCOLOR(34), FOGTABLEMODE(35), FOGSTART(36), FOGEND(37), LIGHTING(137), AMBIENT(139), FOGVERTEXMODE(140)
- **D3D8 vtable**: SetRenderState=[50], SetLight=[44], LightEnable=[46], SetMaterial=[42], GetRenderState=[51]
- **Calling convention**: All D3D8 device methods are __stdcall (COM), not __thiscall
- **State management**: Original render states saved on first frame after enable, restored on disable or scene end
- **Multi-player**: If P2 is present, a second point light is placed at P2's position

## Build

```bash
i686-w64-mingw32-g++ -shared -o NeonLighting.dll NeonLighting.cpp nocrt.cpp \
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

## Installation

Place `NeonLighting.dll` in the HB+ `Mods\` folder.

## Author
Hamsterbot (reverse engineering by Hamsterbot)

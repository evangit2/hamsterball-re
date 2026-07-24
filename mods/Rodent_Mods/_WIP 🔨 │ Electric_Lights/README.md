# Electric Lights Mod

A light/dark level mod for Hamsterball Physicus. The ball is a D3D point light source powered by an electric charge that drains over time.

## Phase 1: Charge System + Ball Glow

### What it does

- Hijacks the native Neon Race D3D light system (same technique as XRow's GlobalNeon CEA)
- Writes a D3D point light at slot 2 every frame via `SetLight` + `LightEnable`
- Light position follows the ball (read from gfx+0x854/858/85C)
- Light range scales with charge level (full charge = 400.0, zero charge = 0.0)
- Overrides ambient to dark (gfx+0x730)
- Ball color multiplier scales with charge for visual glow feedback

### How it works

Hooks `Graphics_RenderScene` entry (0x454BC0) — same hook point as GlobalNeon CEA:
- ECX = gfx struct pointer at entry
- gfx+0x154 = IDirect3DDevice8*
- gfx+0x854/858/85C = ball position (set by Scene_Render each frame)

Each frame:
1. Override ambient to dark
2. Enable D3D lighting
3. Read ball position from gfx struct
4. Update D3DLIGHT8 position + range (scaled by charge)
5. Call SetLight(slot, &light) + LightEnable(slot, TRUE)
6. Scale ball color multiplier by charge

### Configuration

Edit `electric_lights.c` and recompile:

| Define | Default | Description |
|--------|---------|-------------|
| `CHARGE_DRAIN_RATE` | 0.0008 | Charge lost per frame (~0.05/sec at 60fps) |
| `LIGHT_FULL_RANGE` | 400.0 | Max light range (same as Neon Race) |
| `LIGHT_Y_OFFSET` | 30.0 | Light height above ball |
| `GLOW_R/G/B` | 0.8/0.95/1.0 | Ball glow color (white-cyan) |
| `LIGHT_SLOT` | 2 | D3D light slot (0-7; Neon uses 0-1) |
| `DARK_AMBIENT` | 0x000C0C14 | Dark ambient color (0x00RRGGBB) |

### Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll electric_lights.c \
  -I"../_WIP 🔨 │ shared" -lwinmm -Wl,--enable-stdcall-fixup -O2 \
  -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

### Install

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game folder

### Status

- Phase 1: Implemented, crash-tested (35s Wine)
- Phase 2 (Recharge Pads): Future separate mod
- Phase 3 (Light/Dark Collision): Future separate mod
- Phase 4 (Energy Gates): Future separate mod

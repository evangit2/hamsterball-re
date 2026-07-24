# Electric Lights Mod

A light/dark level mod for Hamsterball Physicus. The ball is a D3D point light source powered by an electric charge that drains over time.

## Phase 1: Charge System + Ball Glow

### What it does

- Creates a D3D point light (SceneObject) that follows the ball every frame
- Light range scales with charge level (full charge = 400.0 range, zero charge = dark)
- Ball color multiplier scales with charge for visual glow feedback
- Charge drains continuously at a configurable rate

### How it works

Uses the native Neon Race SceneObject light system:
1. At level start: allocates a 0xD4-byte SceneObject via `operator_new` + `SceneObject_ctor`
2. Registers it via `Scene_RegisterObject` to get a D3D light slot (index 2)
3. Every frame in Ball_Update hook: calls `SetPosition(ball.x, ball.y+30, ball.z)` to follow the ball
4. Scales `obj+0xCC` (light range) based on charge: `charge * 400.0`
5. Writes ball color multiplier (`ball+0x2AC/0x2B0/0x2B4`) scaled by charge

### Native Neon Light System (Ghidra-verified)

The neon light follow mechanism is in Neon board vtable[19] (0x00424790):
- Called per-frame from Board_Update
- Reads ball position from App+0x5DC
- Calls SceneObject_SetPosition with ball.x+20, ball.y+30, ball.z-20

Graphics_RenderScene (0x00454BC0) iterates 8 light slots at gfx+0x710 and calls vtable[3] (RefreshLight) on each, which calls D3D SetLight + LightEnable.

### Configuration

Edit `electric_lights.c` and recompile:

| Define | Default | Description |
|--------|---------|-------------|
| `CHARGE_DRAIN_RATE` | 0.0008 | Charge lost per frame (~0.05/sec at 60fps) |
| `LIGHT_FULL_RANGE` | 400.0 | Max light range (same as Neon Race) |
| `LIGHT_Y_OFFSET` | 30.0 | Light height above ball |
| `GLOW_R/G/B` | 0.8/0.95/1.0 | Ball glow color (white-cyan) |
| `LIGHT_SLOT` | 2 | D3D light slot (0-7; Neon uses 0-1) |

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

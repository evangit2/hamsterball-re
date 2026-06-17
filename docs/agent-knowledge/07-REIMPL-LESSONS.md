# 07 - Reimplementation Lessons Learned

The project produced a working D3D8 reimplementation and learned a great deal about what does/does not match the original. This file captures those lessons so future work does not repeat the same mistakes.

## 1. Rendering is D3D8 Only

- The original uses `Direct3DCreate8`, `IDirect3DDevice8`, `DrawPrimitiveUP`, `CreateVertexBuffer`.
- Any OpenGL/SDL2 files in the old repo are leftovers, not the active code.
- Cross-compile with MinGW i686-w64-mingw32-gcc -m32.

## 2. Wine + llvmpipe Workaround

Original EXE runs under Wine with a d3d8to9 proxy plus software llvmpipe:

```bash
# Build/copy d3d8to9 d3d8.dll next to Hamsterball.exe
Xvfb :99 -screen 0 800x600x16 &
DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 wine Hamsterball.exe
```

This is useful for screenshot/testing but has known rendering bugs:
- `D3DRS_LIGHTING=TRUE + D3DFVF_NORMAL` ignores normals → flat surfaces.
- Textures may not render even with correct API usage.

## 3. Lighting Target-Match Approach

Instead of relying on D3D8 material/lighting interpolation (which behaves incorrectly under llvmpipe), define lit/shadow target colors directly:

| Surface | Lit | Shadow |
|---------|-----|--------|
| Wall | `(0.72,0.90,1.0)` | `(0.24,0.35,0.49)` |
| Floor | `(0.96,0.96,1.0)` | `(0.70,0.78,1.0)` |
| Sky | `(85,120,215)` | - |

Formula:
```cpp
float lit_factor = pow(max(dot(normal, light_dir), 0.0f), 0.35f);
Vec3 color = shadow + (lit - shadow) * lit_factor;
```

## 4. Camera Sign Bug

Original `Scene_SetCamera` computes:
```cpp
orbit_dir = (cos_a, 0.9, sin_a);  // camera-to-target direction
eye = target + normalize(orbit_dir) * orbit_dist;
```

A naive reimplementation placing the eye with the opposite sign (`target - dir * dist`) works for some levels (Level3) but breaks others (Level1, Level2). Use an adaptive heuristic or match the original sign per level.

## 5. Spawn / Collision Placement

- Ball spawn Y must be derived from actual geometry (probe downward from START object), not from the START object's raw Y coordinate.
- Level3 `START1-1` is at Y=-85.4; the track surface is nearby, but blindly spawning at START.y + radius places the ball in empty space.

## 6. Per-Geometry vs Per-Level Colors

WarmUp (Level1) uses per-geometry pink diffuse on platform faces. Arena levels use Section 1 PLATFORM diffuse colors. Do not hardcode one color scheme globally.

## 7. Controls

`Ball_GetInputForce` reads exactly **4 directional DIK codes** at `InputDevice+0x50C..0x518`. There is NO brake key and NO player-jump; jumping is triggered by `E:JUMP` collision objects only.

## 8. Avoid Backgrounding Wine Processes

Wine will hang the agent for 50+ minutes if backgrounded. Use `timeout 10s`, or launch and kill in the same command.

## 9. Cross-Compiling DSound8

When compiling DSound code with MinGW, include `<mmeapi.h>` (or `<mmreg.h>`) before `<dsound.h>` to get `LPWAVEFORMATEX`/`LPCWAVEFORMATEX`.

## 10. WASM BoxedWine Dead End

BoxedWine WASM cannot run D3D8/D3D9 games in the browser. The WASM port requires a native SDL2/WebGL reimplementation, not the original EXE.

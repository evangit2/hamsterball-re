# Global Neon Effect

Darkens all Hamsterball races and arenas to match Neon Race's dark aesthetic.

## Current Version: v22

### What It Does
- **Disables all 8 D3D directional lights** via `LightEnable(i, FALSE)` — removes directional light contribution entirely
- **Sets ambient to dim gray** (`0xFF202020`) — geometry barely visible (~12% brightness)
- **Disables fog** — prevents scene from being fogged out (camera is 800-1200 units from geometry)
- **Patches light loop** — prevents light objects from re-applying per-frame

### Why v21 Failed
v21 set ambient to pure black (`0xFF000000`) but didn't touch directional lights. The D3D8 lighting formula has three terms:

```
vertex_color = ambient × material.ambient
             + Σ(light[i].diffuse × material.diffuse × NdotL)   ← STILL ACTIVE
             + material.emissive
```

The directional lights (stored as object pointers at `gfx+0x710`, 8 slots) are applied in `Graphics_RenderScene`'s light loop at `0x454C7B`–`0x454C95`. Each non-null pointer calls `vtable[0xC]` which internally calls `SetLight` + `LightEnable` on the D3D device. These fully illuminate geometry regardless of ambient.

**Result:** Black background/void but bright geometry — "lighting still bright."

### How v22 Fixes It
1. **Disable all 8 D3D lights** at `Graphics_RenderScene` entry → no directional contribution
2. **Dim gray ambient** (`0xFF202020`) instead of pure black → geometry barely visible
3. **Patch light loop** (`JZ→JMP` at `0x454C8A`) → prevents per-frame light re-application
4. **Fog disabled** (same as v21)

### Ball Visibility
The ball stays visible because:
- `Ball_Render` (0x402DE0) checks `ball+0xC80` (glow flag). When set:
  - `Gfx_PackColorRGB(1,1,1)` sets emissive material = white
  - Ball sprite rendering (shadow, aura, stars) bypasses D3D lighting entirely

### Hook Points
| Address | Function | Purpose |
|---------|----------|---------|
| `0x454BC0` | `Graphics_RenderScene` entry | Hook to disable lights + set ambient |
| `0x454C8A` | Light loop JZ | Patch to JMP (skip light Apply calls) |

### gfx Struct Offsets
| Offset | Field | Value Set |
|--------|-------|-----------|
| `+0x154` | D3D device wrapper | (read) |
| `+0x710` | Light pointer array (8 × 4) | (disabled via LightEnable) |
| `+0x730` | D3DRS_AMBIENT (cached) | `0xFF202020` (dim gray) |
| `+0x734` | D3DRS_FOGENABLE (cached) | `0` (disabled) |
| `+0x738` | D3DRS_FOGCOLOR (cached) | `0xFF000000` (black) |

### D3D vtable Offsets
| Offset | Method | Convention |
|--------|--------|-----------|
| `+0xB4` | `LightEnable(index, BOOL)` | `__stdcall` (clobbers EAX/ECX/EDX) |
| `+0xC8` | `SetRenderState(state, value)` | `__stdcall` |

### Tuning
- **Darker:** Change `0xFF202020` to `0xFF101010` or `0xFF000000`
- **Brighter:** Change `0xFF202020` to `0xFF303030` or `0xFF404040`
- Line with `mov dword ptr [ebx+0x730], 0FF202020h`

### Version History
- **v22** — Disable all 8 D3D lights + dim gray ambient + patch light loop
- **v21** — Ambient black + fog disabled (failed: lights still illuminated scene)
- **v20** — Fog enabled with FOGSTART=0, FOGEND=500 (failed: fogged out everything)
- **v19** — Initial fog + ambient attempt

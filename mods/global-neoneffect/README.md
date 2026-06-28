# Global Neon Effect

Makes all race levels look like Neon Race (LevelDark) — dark with fog.

## Versions

| Version | File | Approach | Status |
|---------|------|----------|--------|
| v25 | `GlobalNeonEffect.CEA` | Black bg + high fog, hook BeginFrame | **CURRENT — test this** |
| v24 | `GlobalNeonEffect_v24_lights.CEA` | Disable lights (corrected vtable), hook BeginFrame | Alternative |

## v25 — Black Background + High Fog

**Root cause of v22-v24 failure:** `Graphics_RenderScene` (0x454BC0) is NOT called per-frame! It's only called on D3D device loss/recovery. `PresentOrEnd` checks `Present()` return for `D3DERR_DEVICELOST` — normally SKIPS `RenderScene` entirely. So hooking inside RenderScene = hook never executes.

**v25 hook:** `Graphics_BeginFrame` (0x453B50) — called EVERY frame before scene rendering. Hook at 0x453B63 (inside BeginFrame, after EBX=gfx is set).

**What it does:**
1. Enables D3D8 table fog (LINEAR mode)
2. Fog start=0, end=300 (very dense)
3. Fog color = pure black
4. Ambient = pure black
5. Lights still illuminate, fog fades geometry to black

**Adjusting fog density:** Change `push 43960000` to:
- `43FA0000` = 500.0f (dense)
- `44480000` = 800.0f (moderate)  
- `447A0000` = 1000.0f (light)

## Per-frame render pipeline (from App_Run 0x46BD80)

```
Graphics_BeginFrame(gfx, timer)   ← HOOK HERE (every frame)
(*App->vtable[0x20])()            ← Scene update
(*App->vtable[0x24])()           ← Scene_Render
(*App->vtable[0x28])()           ← 2D overlay
(*App->vtable[0x2C])()           ← UI
Graphics_PresentOrEnd(gfx, 1)    ← Present (only calls RenderScene on device lost)
```

## D3D8 vtable offsets (verified from d3d8.h)

```
[37] +0x94 = SetTransform
[42] +0xA8 = SetMaterial
[44] +0xB0 = SetLight
[45] +0xB4 = GetLight      ← v22 WRONGLY used this
[46] +0xB8 = LightEnable   ← v24 uses this (CORRECT)
[50] +0xC8 = SetRenderState
```

## D3D8 render states (from d3d8types.h)

```
0x1C = D3DRS_FOGENABLE (28)
0x22 = D3DRS_FOGCOLOR (34)
0x23 = D3DRS_FOGTABLEMODE (35)
0x24 = D3DRS_FOGSTART (36)
0x25 = D3DRS_FOGEND (37)
0x8B = D3DRS_AMBIENT (139)
0x8C = D3DRS_FOGVERTEXMODE (140)
```

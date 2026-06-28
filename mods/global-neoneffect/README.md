# Global Neon Effect

Makes all race levels look like Neon Race (LevelDark) — dark with fog.

## Versions

| Version | File | Approach | Status |
|---------|------|----------|--------|
| v23 | `GlobalNeonEffect.CEA` | Black background + high fog (300 units) | **CURRENT — test this first** |
| v24 | `GlobalNeonEffect_v24_lights.CEA` | Disable all 8 D3D lights (corrected vtable offset) | Alternative — test if v23 isn't dark enough |

## v23 — Black Background + High Fog

**User insight:** Neon Race works by setting fog really high (geometry disappears at short distances) and making the background color black.

**Hook point:** `0x454CEE` — mid-function in `Graphics_RenderScene`, AFTER `Graphics_SetupLights()` and the light loop have run, BEFORE fog color is applied.

**What it does:**
1. Enables D3D8 table fog (LINEAR mode)
2. Sets fog start=0, end=300 (very dense — geometry fully fogged at 300 units)
3. Sets fog color to pure black
4. Sets ambient to pure black
5. Lights still illuminate, but fog quickly fades everything to black
6. Ball stays visible via emissive material + sprite rendering

**Adjusting fog density:**
Change the `push 43960000` value in the script:
- `43960000` = 300.0f (very dense, default)
- `43FA0000` = 500.0f (dense)
- `44480000` = 800.0f (moderate)
- `447A0000` = 1000.0f (light)

## v24 — Disable Lights (Corrected vtable offset)

**v22 bug:** Used `vtable+0xB4` = `GetLight` (read-only!) instead of `vtable+0xB8` = `LightEnable`. GetLight with bogus args just returned an error — lights stayed on, nothing changed.

**v24 fix:** Uses correct `vtable+0xB8` for `LightEnable(index, FALSE)`.

**What it does:**
1. Disables all 8 D3D lights via `LightEnable(i, FALSE)` using CORRECT vtable+0xB8
2. Sets ambient to dim gray (0xFF202020, ~12% brightness)
3. Patches light loop at 0x454C8A (JZ→JMP) to prevent re-enabling
4. Ball stays visible via emissive material

**D3D8 vtable verification (from d3d8.h):**
```
[44] +0xB0 = SetLight
[45] +0xB4 = GetLight      ← v22 WRONGLY used this
[46] +0xB8 = LightEnable   ← v24 uses this (CORRECT)
[50] +0xC8 = SetRenderState
```

## Why v22 didn't work (root cause analysis)

1. **Wrong vtable offset:** v22 called `vtable+0xB4` which is `GetLight` (read-only), not `LightEnable` (vtable+0xB8). GetLight with NULL pointer just returns D3DERR_INVALIDCALL — lights stay on.

2. **Wrong hook point:** v22 hooked at function ENTRY (0x454BC0). `Graphics_SetupLights()` is called FIRST inside RenderScene, which OVERWRITES all render states. Our changes were immediately overwritten.

3. **Wrong approach:** User says Neon Race = high fog + black background, NOT disabling lights. v22 tried to disable lights and set ambient — neither produced the Neon Race look.

## RenderScene pipeline (verified from disassembly)

```
0x454BC0: SUB ESP,0xC0          ← function entry
0x454BCC: CALL Graphics_SetupLights  ← sets ALL default render states
0x454C7B: LEA ESI,[EBP+0x710]   ← light array start
0x454C86: MOV ECX,[ESI]         ← load light pointer
0x454C88: TEST ECX,ECX          ← null check
0x454C8A: JZ skip               ← 74 05 (v24 patches to EB 05)
0x454C8C: MOV EAX,[ECX]         ← vtable
0x454C8E: CALL [EAX+0x0C]      ← light->Apply()
0x454C91: ADD ESI,4; DEC EDI; JNZ loop
0x454C97: MOV ESI,[EBP+0x730]  ← read cached AMBIENT
0x454CAC: SetRenderState(AMBIENT=0x8B, gfx+0x730)
0x454CB2: MOV BL,[EBP+0x734]   ← read cached FOGENABLE
0x454CCD: SetRenderState(FOGENABLE=0x1C, gfx+0x734)
0x454CE9: CALL Graphics_SetViewportZ  ← sets FOGSTART/FOGEND
0x454CEE: MOV ESI,[EBP+0x738]  ← read cached FOGCOLOR ← HOOK POINT
0x454D00: SetRenderState(FOGCOLOR=0x22, gfx+0x738)
```

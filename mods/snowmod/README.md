# SnowMod v2 — "Snow Storm" Visibility Mod

Makes all races look like a blinding snow storm with heavy fog, white-grayish background, and white-grayish ambient lighting.

## What It Does

1. **Heavy fog** — Forces FOGVERTEXMODE=LINEAR, FOGSTART=10, FOGEND=200. The ball fades into white fog just 200 units away.
2. **White-grayish background** — Background clear color set to `FFE0E0E0` (light gray, like snow clouds).
3. **White-grayish ambient light** — Ambient render state (D3DRS_AMBIENT=0x8B) set to `FFE0E0E0`.

## v2 Fix: Why v1 Fog Didn't Work

The game's `FUN_004539A0` has **three conditional branches** based on `gfx+0xA4` flags. On most hardware, **none** of the branches match, so FOGVERTEXMODE and FOGTABLEMODE are left at 0 (NONE). Fog is "enabled" but has no mode = **invisible fog**.

v2 fixes this by hooking **after** all game fog calls and directly calling `SetRenderState` to force FOGVERTEXMODE=LINEAR, FOGSTART, and FOGEND — bypassing all conditionals.

## How to Use

1. Open Cheat Engine, attach to `Hamsterball.exe`
2. Load `SnowMod.CEA`
3. Enable the script
4. Start any race — the world will be foggy and white-gray

## Adjustable Parameters

| Symbol | Type | Default | Description |
|--------|------|---------|-------------|
| `snow_fog_start` | Float | 10.0 | Distance where fog begins. 0 = fog at camera. |
| `snow_fog_end` | Float | 200.0 | Distance where fog is total. Lower = denser storm. |
| `snow_fog_color` | D3DCOLOR | `FFE0E0E0` | Fog + background color (`0xAARRGGBB`). |
| `snow_ambient_color` | D3DCOLOR | `FFE0E0E0` | Ambient light color (`0xAARRGGBB`). |

### Tuning Tips

- **Blizzard**: Set `snow_fog_end` to 80.0 — you'll barely see 80 units ahead
- **Light snow**: Set `snow_fog_end` to 800.0
- **Pure white**: Set both colors to `FFFFFFFF`
- **Darker storm**: Set both colors to `FF808080`

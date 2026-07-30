# plus_level_warp — Level Warp Mod (HB+)

HB+ port of the `level_warp` bass.dll proxy mod.

## What it does

Scans S1 ref points for `WARP(LevelName)` entries. When the player ball enters
a warp zone, the mod runs a multi-phase warp effect:

1. **Rumble** — ball turns purple, camera jitters, pause is blocked
2. **Flash** — screen flashes white, ball vanishes, timer freezes
3. **Hold** — white screen, ball invisible
4. **Fade** — screen fades to white
5. **Load** — loads the target level via `App_StartPracticeRace`
6. **Reveal** — fades back in from white

The mod preserves tournament score/time when warping and supports same-level
warps in Time Trial mode.

## Usage

1. Add S1 ref points named `WARP(LevelName)` to a MESHWORLD level.
2. Accepted level names: `warmup`, `beginner`, `intermediate`, `dizzy`, `tower`,
   `up`, `neon`, `expert`, `odd`, `toob`, `wobbly`, `glass`, `sky`, `master`,
   `impossible`, or `level1` through `level15`.
3. Drive the player ball into the warp zone.

## Example

```
WARP(tower)    -> warp to Tower Race
WARP(level5)   -> warp to Expert Race
```

## Installation

Copy `plus_level_warp.dll` into your HB+ `Mods/` folder and enable it in the
HB+ mod menu.

## Files

- `WarpMod.cpp` — Source code
- `plus_level_warp.dll` — Compiled HB+ mod
- `README.md` — This file

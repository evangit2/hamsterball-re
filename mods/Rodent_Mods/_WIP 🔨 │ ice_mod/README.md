# ice_mod v1 — N:ICE / Water Phase-Shift System

Part of **Hamsterball Physicus** (Water/Ice level). Adds glass-like ice
surfaces that swap with water via a global freeze/thaw phase flag.

## What it does

### N:ICE reference objects

Place **S1 ref points named `ICE`** (or `ICEn`) where ice surfaces should be.
The mod spawns a solid, reflective object at each ref position using the same
lifecycle as Neon Race's DFLOOR objects:

- `Stands_ctor` clones collision spatial trees from a template level → instantly solid
- Render object registered into the board render list (DFLOOR-style show/hide)
- Glass slipperiness: touching N:ICE sets the same slip timer (`ball+0xC5C=15`)
  that the Glass Race dispatcher sets on N:GLASS touches → input force ×0.20
- Glass shine: while cold, the ball renders with specular enabled
  (the same `ball+0x280` flag Glass Race uses)

### WATER reference objects

Place **S1 ref points named `WATER`** for the liquid counterpart. These are
NEW placeholder objects (solid, opaque blue-tintable surfaces) — they are NOT
related to water_mod's E:WATER planes. You'll tell me later what actually
disappears/appears with them; the lifecycle machinery is already wired.

### Phase flag: E:FREEZE / E:THAW

Place invisible event planes named `E:FREEZE` or `E:THAW`. Touching them flips
the global phase:

1. **Cross-fade** — outgoing surface opacity slides 1→0 while the incoming
   surface fades 0→1 over `FadeSeconds` (default 1s)
2. **Then the swap** — render list membership AND collision list membership
   switch. Ice becomes solid, water stops colliding (and vice versa).

Level starts in WATER phase by default.

### Cold mechanic (touch-duration based)

While touching an N:ICE surface:

- A linear ramp builds over `ColdSeconds` (default 5s)
- Ball tints toward ice-blue (via ball color multipliers, lava-mod style)
- Input force scales smoothly from 100% down to `InputScale` (default 50%)
  via a detour inside `Ball_ApplyForceV2` — no velocity/position writes

After leaving ice, the ramp unwinds linearly over `WarmupSeconds` (default 5s).
Nothing is instantaneous — no cheese by rolling on/off in bursts.

## Level authoring quick reference

| Ref point / plane | Purpose |
|-------------------|---------|
| `ICE` (S1 ref)    | Ice surface spawn position |
| `WATER` (S1 ref)  | Water surface spawn position |
| `E:FREEZE`        | Event plane: fade to ice |
| `E:THAW`          | Event plane: fade back to water |

## Configuration (auto-generated as `ice_mod.ini` next to the DLL)

```ini
ColdSeconds=5.00     # seconds touching ice until fully cold
WarmupSeconds=5.00   # seconds to warm back up
InputScale=0.50      # input force multiplier at full cold
TintBlue=0.35        # how blue the ball gets (smaller = bluer)
FadeSeconds=1.00     # ice/water cross-fade duration
```

## Debug log

`ice_mod.log` appears next to the DLL. It logs hook installation, every
ICE/WATER ref found at level load, phase transitions, and swaps. If something
doesn't work, check there first.

## Technical notes (all Ghidra-verified)

| Mechanic | How the original does it | How we replicate |
|----------|--------------------------|------------------|
| Glass slip | GlassRace vtable[29] (0x417760): `ball+0xC5C=15` on N:GLASS; ApplyForceV2 ×0.20 while nonzero; Ball_Update decrements/frame | DCE hook sets the same field on N:ICE touch — global, any level |
| Glass shine | Scene_LoadLevelGlass sets `ball+0x280=1`; Ball_Render wraps draw in SPECULARENABLE on/off | Same flag set/cleared with the cold ramp |
| DFLOOR lifecycle | Template MeshWorlds in board slots; Stands_ctor clones trees; render obj in board+0x8B0→+0x18; show/hide via Append/Remove | Same ctor chain + same lists; collision add/remove on sceneobj+0x18 |
| Cross-fade | Material arrays per MeshBuffer (stride 0x50, ambient/diffuse alpha) | Alpha written per-frame during transition |

Hooks: DispatchCollisionEvents trampoline (0x40C5D0), ApplyForceV2 input-scale
detour (0x40174A, byte-pattern verified), GameUpdate frame epilogue tick host
(0x46C1F1). All gated on App+0x159 (quit flag) / board+0x874 (pause).

Crash-tested on Wine/Xvfb: 45+ seconds alive with all hooks installed,
clean log, clean exit.

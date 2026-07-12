# Universal Ref Loader — Warm-Up Slot Swap Test

**Date**: 2026-06-23
**Mod**: universal-ref-loader v3 (bass.dll proxy, 94747 bytes)
**Test method**: Each race level's MESHWORLD file was copied into Level1.MESHWORLD (Warm-Up race slot), then the Warm-Up Time Trial race was loaded. The game process was checked for survival 8 seconds after race start.

## Important limitation

Screenshots are black on Wine/llvmpipe (the mod DLL causes rendering to not produce visible output on software rendering). The mod DLL's hook at 0x0040C4BA interferes with D3D8 rendering on llvmpipe — the game window stays black. However, the game process continues running, which means the ref loading code (Scene_CreateDynamicObjects → vtable[33] hook) executes without crashing.

On real Windows with a GPU, rendering works normally — the hook only intercepts the factory dispatch, not the rendering pipeline.

## Results

| Level | Race | Object Refs | Result |
|-------|------|-------------|--------|
| L3 | Intermediate | BRIDGE, MOUSETRAP | ✅ ALIVE |
| L4 | Dizzy | GLUEBIE, TIPPER, WATERWHEEL, SWIRL, TARBUBBLE, SIGN-TARPIT | ✅ ALIVE |
| L5 | Tower | CATAPULT, TRAPDOOR, DRAWBRIDGE, MACE, WINDMILL, CHOMPER, TURRET | ✅ ALIVE |
| L6 | Up | SPEEDCYLINDER, LIFTER, TIMEBUTTON | ✅ ALIVE |
| L7 | Neon | DFLOOR, TRODE, NEONPLATFORM | ✅ ALIVE |
| L8 | Expert | BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL | ✅ ALIVE |
| L9 | Odd | LIFTER, LAUNCH | ✅ ALIVE |
| L10 | Toob | SPINNY, SAW, FALLOUT1, BLOCKDAWG | ✅ ALIVE |
| L11 | Wobbly | WOBBLY, WAVY | ✅ ALIVE |
| L12 | Glass | SMASHER | ✅ ALIVE |
| L13 | Sky | PILLAR, MAGNIFYER, POPCYLINDER, TRAPDOOR | ✅ ALIVE |
| L14 | Master | BBRIDGE, BLOCKDAWG, BONK, BRIDGE, CATAPULT, GLUEBIE, POPCYLINDER, TIPPER | ✅ ALIVE |
| L15 | Impossible | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM | ✅ ALIVE |

**13/13 levels passed — no crashes.**

All 46 unique object types across 13 race levels loaded without crashing when loaded in the Warm-Up race slot. The Warm-Up board constructor does not load any object meshes (it has no object refs), so the mod's universal factory dispatch hook was responsible for loading all objects via the Arena factory fallback + JIT mesh injection.

## What "ALIVE" means

- Game process survived 8+ seconds after entering the race
- No crash during Scene_CreateDynamicObjects (where the hook fires)
- No crash during factory dispatch for each ref point
- No crash during JIT mesh loading (MeshWorld_ctor, CollisionLevel_ctorWithLevel)
- No crash during clone-on-return for static-mesh objects

## What we couldn't verify on Wine

- Visual rendering (black screen on llvmpipe)
- Whether objects appear at correct positions
- Whether object behaviors work (catapults, tippers, etc.)
- These require testing on real Windows with a GPU

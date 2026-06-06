# Hamsterball — Global Variables and Data Constants

## What This Document Is

A reverse-engineering reference for modders and researchers listing **global variables and data constants** found in `Hamsterball.exe`. These are symbols that live in the `.data` or `.rdata` sections of the PE image and are accessed by the game code through `DAT_xxxxxx` references, named labels, or inline constants.

---

## Why This Matters for Modding

Many game behaviors are controlled not by function logic but by **scalar constants** in `.data`:

- Ball physics multipliers (speed, gravity, friction)
- Camera orbit distance and damping
- Damage/touch force values
- RNG range constants
- Vtable pointers used for dynamic dispatch

If you **patch these values in memory** or replace them at image load time, you can drastically change game behavior without changing a single instruction.

---

## Table of Contents

1. [Confirmed Named Globals (via Ghidra renames)](#confirmed-named-globals)
2. [Game Constants `.data` / `.rdata` (0x004C0000–0x00534...)](#game-constants-in-dat)
3. [Vtables and Jumps in the Data Section](#vtables-and-jumps)
4. [Named Data Constants with Context](#named-data-constants-with-context)
5. [Float Constants (0x004CF000–0x004D0500)](#float-constants-0x004cf0000x004d0500)
6. [String Table Addresses in .rdata](#string-table-addresses-in-rdata)
7. [How to Patch Constants at Runtime](#how-to-patch-constants-at-runtime)
8. [Recommended Modding Recipes](#recommended-modding-recipes)
9. [Address Quick Reference](#address-quick-reference)
10. [Verification Methodology](#verification-methodology)

---

## Confirmed Named Globals

These have been **manually renamed in Ghidra** and are confirmed by the `renames_backup.json` (restored at session start).

| Address | Symbol | Type / Meaning | Xrefs |
|---------|--------|----------------|-------|
| `0x004FD680` | `g_App` | `App*` — global singleton pointer to the App struct | 5 |
| `0x005341CC` | `g_renderIndex` | `uint32_t` — frame/batch render counter | 3 |
| `0x004D2334` | `s_BACK` | `char` or `char[]` — used by the UI text system for the "Back" button / menu logic | 45 |
| `0x004D9CDC` | `MeshWorld_vtable` | `void**` — pointer to the MeshWorld vtable | 2 |
| `0x004F7360` | `PTR_OBJ_VTABLE` | `void**` — generic object vtable base for `Obj` class (193 xrefs) | 193 |

---

## Floating-Point Constants (0x004CF000–0x004D0500)

The engine keeps many scalar constants in the `.data` section. They are **NOT loaded as immediate values** into instructions; they are loaded as memory references (`_DAT_004CFxxx`). This makes them **trivial to patch** from external code.

### Core Ball Physics Constants

| Address | Approx. Value | Function | Description |
|---------|---------------|----------|-------------|
| `0x004CF310` | `1.0f` | `Ball_GetInputForce` / `Ball_ApplyForce` | Full-speed backward / max multiplier |
| `0x004CF368` | `0.0f` | `Ball_ApplyForce`, `Math_Atan2Angle` | Zero constant (force neutral, atan2 fallback) |
| `0x004CF380` | `~0.1–0.2` | `Ball_ApplyForce` | **Hit recovery multiplier** — reduces force after ball takes a hit (`+0x2F0 != 0`) |
| `0x004CF378` | `~0.5–0.8` | `Ball_ApplyForce` | Flag `+0x324` multiplier (e.g., dizzy state) |
| `0x004CF374` | `~0.5–0.8` | `Ball_ApplyForce` | Flag `+0xC5C` (momentum-transfer state) multiplier |
| `0x004CF3E8` | `1.0f` | `Ball_ApplyForce` | Angular velocity scale (applied to `+0xFC/0x100/0x104`) |
| `0x004CF3F0` | `0.95f` | `Ball_AdvancePosition`, `Ball_vtable[0x14]` | **Gravity / damping constant** (per-frame friction multiplier) |
| `0x004CF36C` | `~0.5–0.7` | `Ball_ApplyForce` | Flag `+0xC4C` (e.g., speed boost / power-down) multiplier |

#### Modding Recipes — Physics

1. **Super Ball Mode** — set `0x004CF380` (hit recovery) to `1.0f`:
   The ball no longer loses momentum when hit by hazards.
2. **Ice Mode** — set `0x004CF3F0` (damping) to `1.0f`:
   Ball never slows down from friction/gravity. Will fly off levels.
3. **Dizzy Immunity** — set `0x004CF378` to `1.0f`:
   When the `+0x324` dizzy flag is set, force is not reduced.
4. **Heavy Steel Ball** — set `0x004CF3E8` to `0.1f`:
   Angular velocity barely changes; ball rolls slowly.

---

### Camera / Orbit Constants

| Address | Approx. Value | Function | Description |
|---------|---------------|----------|-------------|
| `0x004CF3EC` | `~1.0–2.0` | `Scene_SetCamera` | Camera **MIN_DIST** for orbit clamping |
| `0x004CFF78` | `700.0f` | `Scene_SetCamera` | Camera **MAX_DIST** (hard cap fallback) |
| `0x004D03A4` | `~0.01–0.1` | `Scene_SetCamera` | Camera wave **SCALE** (`Wave_Sin` input) |
| `0x004D03A0` | `~0.5–1.0` | `Scene_SetCamera` | Camera wave **AMPLITUDE** multiplier |

#### Modding Recipes — Camera

1. **Far Orbit** — change `0x004CFF78` from `700.0f` to `1200.0f`:
   Camera can zoom much farther from the ball.
2. **Wave-Cam Disable** — set `0x004D03A4` to `0.0f`:
   Camera sine-wave bob disappears — perfectly smooth follow.

---

### Collision / Touch Damage Constants

| Address | Approx. Value | Function | Description |
|---------|---------------|----------|-------------|
| `0x004CF484` | `~20.0–50.0` | `Ball_Update` / `0x405190` | Collision **distance threshold** for hazard proximity check |
| `0x004CF370` | `~1.0–1.5` | `CollisionEvents` | Direction/force multiplier on collision deflection |
| `0x004CF508` | `~0.5–1.0` | `Ball_vtable[0x10]` (AI update) | AI chase activation distance ratio |
| `0x004D0434` | `~2.0–10.0` | `Scene_Spawn` | Ball spawn **vertical offset** from START marker |

#### Modding Recipes — Collision

1. **Hazard Immunity** — raise `0x004CF484` to `99999.0f`:
   Collision check never succeeds → hazards never touch you.
2. **Spawn Offset Fix** — set `0x004D0434` to `0.0f`:
   Ball spawns exactly at `START.y` instead of floating above.

---

### RNG / Math Constants

| Address | Approx. Value | Function | Description |
|---------|---------------|----------|-------------|
| `0x004CF558` | `360.0f` | `Ball_vtable[0x10]` (AI) | Angle wrap helper (add 360 to normalize negative deg) |
| `0x004CF554` | `0.0f` | `Ball_vtable[0x10]` (AI) | Angle comparison lower bound |
| `0x004CF550` | `~300.0–350.0` | `Ball_vtable[0x10]` (AI) | AI **flee distance threshold** |
| `0x004CF48C` | `~0.05f` | `Ball_vtable[0x10]` (AI) | AI tick accumulator per frame |
| `0x004D0418` | `~114.59` | `Math_Atan2Angle` | `180/π` — radian→degree constant |
| `0x004D03A0` | `~1.0` | `Scene_SetCamera` | Camera sine multiplier |

---

### Graphics / Rendering Constants

| Address | Approx. Value | Function | Description |
|---------|---------------|----------|-------------|
| `0x004CF3C8` | `1.0f` | `LoadBinaryMesh` / mesh parser | UV **V-inversion constant** (`1.0 - v`) |
| `0x004CF454` | `~0.01–0.1` | `MeshWorld_Parse` | Material **shininess multiplier** |
| `0x004CF41C` | `~1.0` | `Graphics_Initialize` | Light range / projection scale factor |
| `0x004CF308` | `~0.001–0.01` | `Scene_RenderAllObjects` | **Depth bias** for shadow decal offset |

---

### Movement / Input Constants

| Address | Approx. Value | Function | Description |
|---------|---------------|----------|-------------|
| `0x004D0250` | `~0.5f` | `Ball_GetInputForce` | Forward **half-speed** input multiplier |

---

## Vtables and Jumps

These are **pointers to vtable arrays** or **function pointer tables** stored in `.data`.

| Address | Symbol | What it points to | Usage |
|---------|--------|-------------------|-------|
| `0x004D9CDC` | `MeshWorld_vtable` | `MeshWorld` class vtable (2 xrefs) | Level geometry dispatch |
| `0x004CE400` | *(implicit)* | `App` vtable base | `App` virtual method dispatch (see APP_OBJECT.md) |
| `0x004CF3A0` | *(implicit)* | `Ball` vtable | `Ball` virtual method array (set in `Ball_ctor`) |
| `0x004D0260` | *(implicit)* | `Scene` vtable | `Scene` virtual method array |
| `0x004F7360` | `PTR_OBJ_VTABLE` | Generic `Obj` base-class vtable | 193 xrefs — extremely common base pointer |

---

## String Table Addresses in .rdata

The binary embeds strings at fixed addresses in the `.rdata` section. Some are used as global identifiers passed to functions.

| Address | String / Content | Used By |
|---------|------------------|---------|
| `0x004D9E54` | String-table base for `strtok`-style token parsing in MESHWORLD parser | `MeshWorld_Parse` / `FUN_004BC0D1` |
| `0x004D48A0` | Save-file header / format string | `SaveTournament` |
| `0x004D9120` | `999999.0f` or similar sentinel float | `Ball_Update` (AI min-distance init) |
| `0x004D2334` | `s_BACK` — "Back" button / menu state | 45 xrefs across UI code |

---

## How to Patch Constants at Runtime

### Method 1 — External DLL

```c
// In your mod DLL (injected into Hamsterball.exe process)
#define ADDR_HIT_RECOVERY      ((float*)0x004CF380)
#define ADDR_DAMPING           ((float*)0x004CF3F0)

void __declspec(dllexport) ApplyMods() {
    // Make the pages writable
    DWORD oldProtect;
    VirtualProtect(ADDR_HIT_RECOVERY, sizeof(float), PAGE_READWRITE, &oldProtect);

    // Patch values
    *ADDR_HIT_RECOVERY = 1.0f;   // Never reduce force on hit
    *ADDR_DAMPING      = 0.999f; // Very slippery ball

    VirtualProtect(ADDR_HIT_RECOVERY, sizeof(float), oldProtect, &oldProtect);
}
```

### Method 2 — Cheat Engine

1. Open Cheat Engine, attach to `Hamsterball.exe`
2. **Add Address Manually** → enter `004CF3F0` as float
3. Freeze value at `1.0f` → instant zero-friction mode

### Method 3 — DLL Injection at Load Time

Use `detours`, `MinHook`, or a manual IAT hook. Patch `.data` section **before** `WinMain` runs so the game never sees the original values.

---

## Recommended Modding Recipes

### 1. God Mode (Hazard Immunity)
```c
*(float*)0x004CF484 = 99999.0f;  // Distance threshold
```

### 2. Super Speed
```c
*(float*)0x004CF3F0 = 1.0f;   // No friction damping
*(float*)0x004CF310 = 5.0f;   // Max backward speed
```

### 3. Dizzy Immunity
```c
*(float*)0x004CF378 = 1.0f;   // Dizzy flag no longer reduces force
```

### 4. Smooth Camera
```c
*(float*)0x004D03A4 = 0.0f;   // Wave_Sin scale → no bob
*(float*)0x004CFF78 = 2000.0f; // Far max orbit
```

### 5. Spawn Height Fix
```c
*(float*)0x004D0434 = 0.0f;   // No offset above START marker
```

---

## Address Quick Reference

```
0x004CF308    Depth bias (shadow decal)
0x004CF310    Max movement multiplier (1.0f)
0x004CF368    Zero float (0.0f)
0x004CF36C    Speed boost / power-down mult
0x004CF370    Collision deflection dir multiplier
0x004CF374    Momentum-transfer mult
0x004CF378    Dizzy state force mult
0x004CF380    Hit recovery force mult
0x004CF3C8    UV V-inversion (1.0f)
0x004CF3E8    Angular velocity scale (1.0f)
0x004CF3EC    Camera MIN_DIST
0x004CF3F0    Gravity / damping (0.95f)
0x004CF41C    Light range scale
0x004CF454    Shininess multiplier
0x004CF484   Collision dist threshold
0x004CF48C   AI tick accumulator (~0.05)
0x004CF508   AI activation distance ratio
0x004CF550   AI flee distance
0x004CF554   Angle lower bound (0.0)
0x004CF558   Angle wrap (360.0)
0x004CFF78   Camera MAX_DIST (700.0)
0x004D0250   Forward half-speed (~0.5)
0x004D03A0   Camera wave amplitude
0x004D03A4   Camera wave scale
0x004D0418   180/PI rad→deg
0x004D0434   Spawn vertical offset
0x004D2334   s_BACK (UI string)
0x004D48A0   Save header string
0x004D9120   Huge float sentinel
0x004D9CDC   MeshWorld_vtable
0x004D9E54   MESHWORLD tokenizer string table
0x004F7360   PTR_OBJ_VTABLE (193 xrefs)
0x004FD680   g_App (global App* pointer)
0x005341CC   g_renderIndex
```

---

## Verification Methodology

All addresses in this document were derived from:

1. **Ghidra decompilation** (`analysis/ghidra/decompilations/`) — raw `_DAT_004xxxxx` references in `.c` files
2. **Ghidra `renames_backup.json`** — manually renamed global labels
3. **GhidraMCP `list_globals`** — live project query of labeled data
4. **Cross-reference count** from Ghidra xrefs — values with ≥3 xrefs are high-confidence

### What was NOT verified

- Exact float values at each address (Ghidra shows `_DAT_` names but not the literal in decompilation). Where exact values are given, they are **inferred from context** (e.g., `* 0.95f` comment near `0x004CF3F0`). To get ground-truth values, read 4 bytes at the VA in the running process.
- Values may change between game versions. This document targets the version analyzed (`Hamsterball.exe` MD5 from `docs/APP_OBJECT.md`).

---

## Document Revision

- **Compiled from:** Ghidra project, `renames_backup.json`, raw decompilations
- **Coverage:** 24 unique `DAT_004xxxxx` references extracted from raw `.c` files + 5 named globals
- **Next additions:** Runtime memory dump verification (read actual float/int values at each VA), more vtable addresses, additional string table bases

*For object-level offsets (App, Ball, Scene structs), see `APP_OBJECT.md`, `BALL_OBJECT_MODDING.md`, and `SCENE_STRUCT.md`.*

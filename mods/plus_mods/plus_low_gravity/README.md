# Low Gravity — Hamsterball Plus Mod

**Author:** Hamsterbot  
**API:** Hamsterball Plus v1  
**File:** `LowGravity.cpp`

## What It Does

Adds a "LOW GRAVITY" toggle button to the options menu. When enabled, reduces the player's ball gravity to 25% of normal, making the ball floaty and allowing longer air time.

## How It Works

- **Toggle button** (`CHEAT_LOWGRAV`) registered via `api->CreateToggleButton()`
- On each `onBallUpdate`, writes `0.125f` to `ball+0x278` (gravity_scale, default `0.5f`)
- Also writes to the physics object's gravity accumulator at `physics+0x1C0` to prevent the first-frame full-gravity spike

## Key Offsets

| Offset | Field | Default | Mod Value |
|--------|-------|---------|-----------|
| `ball+0x278` | gravity_scale | 0.5 | 0.125 |
| `ball+0x1A4` | physics_object ptr | — | dereferenced |
| `physics+0x1C0` | gravity Y accumulator | 0.5 | 0.125 |

## Building

1. Open Visual Studio with the Hamsterball Plus mod template
2. Replace `MainModFile.cpp` with `LowGravity.cpp`
3. Build as DLL (output name doesn't matter, the API loads by export)
4. Place the compiled `.dll` in the `Mods\` folder

## Controls

- **Toggle:** Options menu → "LOW GRAVITY" → YES/NO

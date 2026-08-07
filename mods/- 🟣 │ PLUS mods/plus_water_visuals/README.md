# Water Visuals Mod (HB+ v2.1)

Visual companion to the Water Mod (`plus_water_mod`) — handles the *look and sound* of water using the game's native **TarBubble** round-sprite bubble object. Fully separate from the physics mod, so nothing about water physics changes.

## Features

- **Entry splash**: on `E:WATER` contact, spawn a burst of native round bubbles at the ball's position.
- **Speed-tiered splash sound**: two distinct effects switched by entry speed —
  - fast entry → more bubbles + `dropin` sound
  - slow entry → fewer bubbles + `dropinshort` sound
- **Float to equilibrium, freeze, random pop**: each bubble floats up at constant size to the water's equilibrium surface (where the ball floats), holds there, then pops after a random 0.5–1.5s. So bubbles rise to the surface line and burst on a slight delay — no instant vanish, no premature shrink.
- **Sparse bubbles while submerged AND moving**: after the splash, bubbles appear at a **random 1–1.5s rate** while the ball stays in water **and is moving** — if the ball goes idle, spawns pause until it moves again. ("Few and far between", organic timing.)
- **Random splash count**: entry splash count varies by ±1 every entry.
- **Native bubbles**: uses the game's own TarBubble object (vtable `0x4D6E48`), a round textured sprite — no custom sprites or mesh construction.
- **Self-drive on any board**: the bubble list (`board+0x3B00`) is natively iterated+rendered only by Dizzy (`0x41D512`) and Master (`0x420DA0`) boards. On every other board this mod drives the rise/freeze/pop animation (in `onGameUpdate`) and renders (in `onRenderApply`), gated on the pause flag (`board+0x874`) so bubbles freeze in the ESC menu.
- **Correct lifecycle**: frees popped bubbles (dtor `0x44FD40` with flag 1) and cleans up all bubbles + animation state on level unload.

## HB+ Options

| Control | ID | Default | Range | Description |
|---|---|---|---|---|
| Toggle | WATER_VIS | ON | - | Master switch for all visuals |
| Slider | WVIS_BURST | 10 | 0-30 | Bubbles in a fast-entry splash (slow ≈ ⅓), random ±1 |

## Level Setup

Same as the physics mod: place `E:WATER` collision objects in custom levels. Contact triggers the splash; staying submerged produces sparse bubbles; rising clearly above the surface exits water and stops spawning.

## Differences from the physics mod

| Aspect | plus_water_mod (physics) | plus_water_visuals (this) |
|---|---|---|
| Purpose | drag/buoyancy/flow forces | bubbles + splash sound |
| E:WATER handling | `onEventPlaneCollide` → water state | same callback → splash + state |
| Per-frame | `apply_water_physics()` forces | `apply_visuals()` sparse bubbles |
| Bubble source | none | native TarBubble |
| List ownership | n/a | self-drive on non-Dizzy/Master boards |

## Building

- **MinGW (Linux):** `./build.sh` → produces `plus_water_visuals.dll` (nocrt + manual 17-entry vtable + hbplus_api.h). Verified: `CreateModInstance` exported, KERNEL32-only import, 17 non-zero vtable slots, no msvcrt.
- **Visual Studio (Windows):** create an HB+ project from `HBmodTemplate.zip`, add `WaterVisualsMod.cpp` + `nocrt.cpp`/`nocrt.h`, build as x86 DLL.

Drop the resulting `.dll` in the game's `Mods\` folder alongside `plus_water_mod.dll`. Neither mod depends on the other.

## Testing note

Same as the physics mod: the HB+ loading chain (scanning `Mods\`, calling `CreateModInstance`, firing `Initialize()`) cannot be verified under Wine/hbtestd — that env uses a plain bass proxy, not the HB+ framework. Only real Windows with HB+ installed can confirm the full load + in-game behavior.

Author: RodentRacer / Hamsterbot

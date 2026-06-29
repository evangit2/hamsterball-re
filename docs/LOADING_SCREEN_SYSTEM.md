# Hamsterball Loading Screen System

## Overview

When Hamsterball starts, it displays a loading screen showing a ball sprite that fills in from bottom to top. This is driven by a **LoaderGadget** scene object that loads one asset per frame, making loading time directly proportional to frame count rather than wall-clock time. Additionally, the unregistered (demo) version injects artificial `Sleep(10)` delays via "DELAY" entries in the asset list.

**Key finding:** At low framerates, loading takes significantly longer because the game processes exactly one asset per frame — halving your FPS doubles the loading time. This is a design choice, not a bug.

---

## LoaderGadget Class

### Vtable

| Slot | Offset | Address | Function | Description |
|------|--------|---------|----------|-------------|
| 0 | 0x00 | 0x0042DE30 | LoaderGadget_Dtor | Destructor |
| 1 | 0x04 | 0x0042CE50 | LoaderGadget_Update | Per-frame update (calls Tick + UI logic) |
| 2 | 0x08 | 0x0042D270 | LoaderGadget_Render | Draw loading screen visuals |
| 3 | 0x0C | 0x004692F0 | (inherited) | Input/mouse handler |
| 5 | 0x14 | 0x0042D020 | (inherited) | Menu handler |
| 7 | 0x1C | 0x00409D90 | (inherited) | — |
| 16 | 0x40 | 0x00419740 | (inherited) | — |
| 18 | 0x48 | 0x00475020 | MeshWorld_AddTexture | Add TEX entry to load list |
| 19 | 0x4C | 0x00475130 | AddMesh | Add MESH entry to load list |
| 20 | 0x50 | 0x004751D0 | AddWMesh | Add WMESH entry to load list |
| 21 | 0x54 | 0x00475270 | AddBCMesh | Add BCMESH entry to load list |
| 22 | 0x58 | 0x004752F0 | AddSprite | Add SPRITE entry to load list |
| 23 | 0x5C | 0x00475390 | AddFont | Add FONT entry to load list |
| 24 | 0x60 | 0x00475430 | AddSound | Add SOUND entry to load list |
| 25 | 0x64 | 0x004750C0 | AddDelay | Add DELAY entry (demo-only Sleep) |
| 26 | 0x68 | 0x004750C0 | (completion) | Calls LoaderGadget_OK when done |
| 27 | 0x6C | 0x00474AE0 | ProcessAsset | Load one asset from the list |

**Vtable location:** `0x004D3C78` (set in constructor at `0x0042C910`)

A second vtable at `0x004DA09C` also contains LoaderGadget_Tick at slot 1 and AddDelay at slot 25 — this is the MeshWorld's loader interface used during level loading.

### Constructor

**`LoaderGadget_Ctor`** at `0x0042C910`

Creates the LoaderGadget scene object. Key actions:
1. Calls `Menu_Ctor` (base class)
2. Sets vtable to `0x004D3C78`
3. Creates four sprites:
   - `Loader.png` (colored ball, fills as loading progresses) → `this+0x2D6C`
   - `Loader(Grey).png` (grey ball background) → `this+0x2D70`
   - `Loadingswirl.png` (spinning swirl) → `this+0x2D74`
   - `loaderbkg.png` (background image) → `this+0x2D7C`
4. Creates `demo.png` sprite → `this+0x2D78`
5. Plays "Loading" music at 2.0 speed
6. Sets `this+0x3624` = `0x41200000` (10.0f) — swirl rotation speed
7. If unregistered (`app+0x200 == 0` and `app+0x914 < 1`), sets `this+0x3208 = 1` (demo mode flag)
8. If demo mode, inserts nag messages: "REGISTER HAMSTERBALL FOR FASTER LOADING!", "LOADING TOO SLOWLY?", etc.

### Struct Layout (LoaderGadget)

All offsets are byte offsets from `this`. Field names are inferred from decompilation.

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x2D54 | void* | load_list_ptr | Current AthenaList being populated |
| +0x2D5C | float | progress | Loading progress (0.0 to 1.0) |
| +0x2D68 | int* | app_ptr | Pointer to App singleton |
| +0x2D6C | Sprite* | loader_sprite | "Loader.png" (colored ball) |
| +0x2D70 | Sprite* | grey_sprite | "Loader(Grey).png" (grey background ball) |
| +0x2D74 | Sprite* | swirl_sprite | "Loadingswirl.png" (spinning swirl) |
| +0x2D78 | Sprite* | demo_sprite | "demo.png" |
| +0x2D7C | Sprite* | bkg_sprite | "loaderbkg.png" (background) |
| +0x2D80 | float | swirl_speed | Swirl rotation speed (from CPUID check) |
| +0x2D84 | byte | done_flag | Set to 1 when loading complete |
| +0x2E24 | int | asset_index | Current asset being loaded (index into list) |
| +0x2E24+4 | AthenaList* | asset_list | The list of assets to load |
| +0x2E24+8 | int | asset_count | Total count from AthenaList_GetSize |
| +0x2E24+0xC | float | fill_ratio | Progress as integer ratio (index/total) |
| +0x2E24+0x10 | int | frame_counter | Total frames since loading started |
| +0x2E24+0x14 | byte | tick_enabled | Gate flag — set by external timer to allow one asset per frame |

---

## Loading Mechanism

### One Asset Per Frame

**`LoaderGadget_Update`** (`0x0042CE50`, vtable slot 1) is called once per frame by the scene update loop. It:

1. Clamps `this+0x3624` (swirl speed) to a minimum of 0.0 (`0x004CF368`)
2. Calls **`LoaderGadget_Tick`** (`0x00474A30`) — processes one asset
3. Updates swirl rotation: `this+0x2D80` = `GetTickCount() / 200`
4. Rotates nag messages every 300 frames (5 seconds at 60fps)
5. Checks if progress ≥ 1.0 (`0x3F800000` at `0x004CF310`) — if so, calls `LoaderGadget_OK`

**`LoaderGadget_Tick`** (`0x00474A30`):

```c
void LoaderGadget_Tick(int *this) {
    if (this->tick_enabled) {          // this+0x2E24+0x14
        int idx = this->asset_index;   // this+0x2E24
        this->frame_counter++;         // this+0x2E24+0x10
        this->tick_enabled = 0;        // clear gate (one-shot per frame)

        // Load the asset at current index
        if (idx >= 0 && idx < this->asset_count) {
            void *asset = this->asset_array[idx];
            if (asset != NULL) {
                this->vtable[0x6C](asset);  // ProcessAsset
            }
        }

        this->asset_index++;

        // Update progress ratio
        int total = AthenaList_GetSize(this->asset_list);
        if (total != 0) {
            this->fill_ratio = (float)this->asset_index / (float)total;
        }

        // Check if all assets loaded
        if (this->asset_index == total) {
            this->vtable[0x68]();  // LoaderGadget_OK
        }
    }
}
```

**`ProcessAsset`** (`0x00474AE0`, vtable slot 0x6C) dispatches based on the entry's type string:

| Type String | Action |
|-------------|--------|
| `TEX` | Load texture via `FUN_00455C50` (D3D texture creation) |
| `DELAY` | **`Sleep(10)`** — only if unregistered (`app+0x200 == 0`) |
| `MESH` | Create `MeshNode` via `MeshNode_ctor` (alloc 0x18) |
| `WMESH` | Create `MeshWorld` via `MeshWorld_ctor` (alloc 0x10D0) |
| `BCMESH` | Create `CollisionLevel` via `CollisionLevel_ctorWithLevel` (alloc 0x10D0) |
| `SPRITE` | Create `Sprite` via `Sprite_ctor` (alloc 0xD4) |
| `FONT` | Load font via `LoadFont` (alloc 0x182C) |
| `SOUND` | Load sound via `Sound_LoadAndAppend` |

### Visual Fill Effect

**`LoaderGadget_Render`** (`0x0042D270`, vtable slot 2) draws the loading screen each frame:

1. Draws `loaderbkg.png` background (600×600 at position 100, 0)
2. Draws logo sprites if registered
3. Draws `Loader(Grey).png` (grey ball) at fixed position (336, 386)
4. Draws `Loader.png` (colored ball) **clipped vertically** based on progress:
   - Source Y start = `_DAT_004D3EC0 - (sprite_height * progress)`
   - Source Y end = `1.0 - progress` (remaining unfilled portion)
   - This creates the "filling from bottom to top" effect
5. If progress < 1.0: draws spinning `Loadingswirl.png` at (400, 450) with rotation = `GetTickCount() / 200`
6. If progress ≥ 1.0: shows "CLICK HERE TO PLAY!" text (or "CLICK HERE TO EXIT" if demo expired)
7. If unregistered and progress is slow: draws nag messages ("LOADING TOO SLOWLY?", "REMOVE THIS DELAY!", etc.)

### Completion

**`LoaderGadget_OK`** (`0x00474900`) is called when all assets are loaded:
1. Sets `app+0x208` = `"LoaderGadget::OK"` (status string for crash handler)
2. If `this+0x2D84` (demo flag) is set, calls `app->vtable[0x44]` (transition to next screen)
3. Calls `this->vtable[0x40]` (cleanup/transition)

### Progress Threshold

The completion check compares `this+0x2D5C` (progress float) against `_DAT_004CF310` = `0x3F800000` (**1.0f**). When progress reaches 1.0, loading is complete.

---

## Artificial Delays

### DELAY Entries (Demo Version Only)

**`AddDelay`** (`0x004750C0`, vtable slot 0x64) creates a load list entry with type string `"DELAY"` and appends it to the asset list. When `ProcessAsset` encounters a DELAY entry:

```asm
; At 0x00474BC1
0x00474BC1: 75 15           jnz  +0x15       ; skip if not "DELAY"
0x00474BC3: 8B 45 14        mov  eax,[ebp+0x14]  ; load app pointer
0x00474BC6: 8A 88 00 02 00 00  mov cl,[eax+0x200]  ; registered flag
0x00474BCC: 84 C9           test cl, cl
0x00474BCE: 75 08           jnz  +8          ; skip Sleep if registered
0x00474BD0: 6A 0A           push 10         ; 10 milliseconds
0x00474BD2: FF 15 8C F0 4C 00  call [Sleep]  ; Sleep(10)
```

**The `Sleep(10)` only fires when `app+0x200` (registered flag) is 0.** Registered versions skip the Sleep entirely.

DELAY entries are added via `vtable[0x64]` calls from:
- `Scene_Render` (`0x0041A2E0`) — two call sites at `0x0041A324` and `0x0041A4DD`
- App initialization (near `0x0040AA7E`)

The render function also shows taunting messages when loading is slow and plays remaining < 20:
- "LOADING TOO SLOWLY?"
- "THE MORE YOU PLAY THE DEMO, THE SLOWER THE GAME LOADS!"
- "BUY HAMSTERBALL TO REMOVE THIS DELAY!"

### One-Asset-Per-Frame Bottleneck (All Versions)

Even without DELAY entries, the fundamental architecture loads **one asset per frame**. The `tick_enabled` flag (`this+0x2E24+0x14`) is set by an external timer (likely the frame loop) and cleared after each asset is processed. This means:

- At 60 FPS with 200 assets: ~3.3 seconds loading time
- At 30 FPS with 200 assets: ~6.7 seconds loading time
- At 15 FPS with 200 assets: ~13.3 seconds loading time

This is why low framerates make loading take much longer — it's frame-count-dependent, not time-dependent.

---

## Patches to Remove Fake Loading

All addresses verified against the original Hamsterball.exe (V3.6.c, md5=`7d25019366b8d7f55906325bd630d7fe`).

### Patch A: Skip Sleep(10) — 2 bytes (Recommended)

Change the conditional jump to unconditional, so `Sleep(10)` is always skipped regardless of registration status:

```
Address: 0x00474BCE  (file offset: 0x74BCE)
Original: 75 08       (jnz +8)
Patched:  EB 08       (jmp +8)
```

This is the smallest, safest patch. Only affects the DELAY handler.

### Patch B: NOP the Sleep call — 8 bytes

Remove the `push 10` and `call [Sleep]` entirely:

```
Address: 0x00474BD0  (file offset: 0x74BD0)
Original: 6A 0A FF 15 8C F0 4C 00  (push 10; call [Sleep IAT @ 0x4CF08C])
Patched:  90 90 90 90 90 90 90 90  (NOP × 8)
```

### Patch C: Multi-load per frame (Code Cave)

To eliminate the framerate dependency entirely, modify `LoaderGadget_Tick` to process ALL remaining assets in a loop instead of one per frame. This requires a code cave that:
1. Removes the `tick_enabled` gate check
2. Loops through all assets in one call
3. Falls through to `LoaderGadget_OK` when done

This would make loading complete in a single frame, limited only by disk I/O speed.

### Effect of Patches A/B

Patches A and B remove the artificial `Sleep(10)` delay from DELAY entries. Loading is still framerate-dependent due to the one-asset-per-frame design, but each DELAY entry no longer adds 10ms of sleep. The visual fill animation will still play, just faster.

### Effect of Patch C

Patch C eliminates both the Sleep delay AND the framerate dependency. Loading completes as fast as the disk can read files. The loading screen may flash too quickly to see — this is expected.

---

## Key Addresses Summary

| Address | Function | Description |
|---------|----------|-------------|
| 0x0042C910 | LoaderGadget_Ctor | Constructor — creates sprites, sets up load list |
| 0x0042CE50 | LoaderGadget_Update | Per-frame update (vtable slot 1) |
| 0x0042D270 | LoaderGadget_Render | Render fill effect + nag messages (vtable slot 2) |
| 0x00474A30 | LoaderGadget_Tick | Process one asset per frame |
| 0x00474AE0 | ProcessAsset | Dispatch by type (TEX/DELAY/MESH/WMESH/etc.) |
| 0x00474BCE | (patch point) | `jnz +8` — skip Sleep if registered |
| 0x00474BD0 | (patch point) | `push 10; call [Sleep]` — Sleep(10) call |
| 0x00474900 | LoaderGadget_OK | Completion handler |
| 0x004750C0 | AddDelay | Add DELAY entry to load list (vtable slot 0x64) |
| 0x004D3C78 | — | LoaderGadget vtable |
| 0x004DA09C | — | Second vtable (MeshWorld loader interface) |
| 0x004CF08C | — | Sleep IAT entry |
| 0x004CF310 | — | Float 1.0f — completion threshold |
| 0x004CF368 | — | Float 0.0f — minimum swirl speed clamp |
| 0x004CF3D0 | — | Float 0.05f — swirl speed decay rate |

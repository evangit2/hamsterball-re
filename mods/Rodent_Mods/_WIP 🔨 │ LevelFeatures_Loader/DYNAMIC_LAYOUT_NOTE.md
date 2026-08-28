# DYNAMIC_LAYOUT_NOTE — LevelFeatures_Loader Structural Rewrite (Option B)

> Reference for RodentRacer during rewrite. Replaces fixed `UNION_SIZE 0xAB00` layout with S1-driven extension heap.

## 1. Why `UNION_SIZE 0xAB00` is removed

- **Memory waste:** `0xAB00` (~43 KB) union padded onto every board alloc via 15 patched `Tournament_AdvanceRace (0x427080)` sites. Allocates max of all features even when level uses none (e.g. WarmUp pays for Dizzy swirl + Tower lists).
- **Cannot handle file-swapped levels:** `g_levelData[level]` / `LevelFeatures.txt [OBJECTS]` per-level lists are index-based, not file-based. Dropping `Level3.MESHWORLD` (Dizzy) into `levels\level1.meshworld` (WarmUp slot) does **not** spawn its objects — source of truth is wrong.
- **Fragile overlap model:** `UNI_*` offsets share a single static range (`0x6500–0xA880`). Adding a new object type forces manual re-packing; collision = silent corruption.
- **Goal:** 100% file customizability — S1 refs inside the `.MESHWORLD` are the config, no `.txt` edit required.

## 2. Design: S1-driven, file-is-config

- **Source of truth = S1 refs in MESHWORLD**, not `g_levelData[level]` or `[OBJECTS]` lists.
  - S1 = mesh/texture string refs parsed by `Level_MeshWorldCtor (0x61510)`. Every dynamic object type has a distinct S1 string prefix.
- **Registry:** `objectType -> { s1Prefix, meshCtor, objectCtor, needsFeatureBit }`
  - e.g. `WATERWHEEL -> "Levels\\Level3-WaterWheel"`, `TIPPER -> "Levels\\Level3-Tipper"`, `SWIRL -> "Levels\\Level3-Swirl"`, `GLUEBIE -> "Levels\\Level3-Gluebie"`, `BRIDGE -> "Levels\\Level2-Bridge"`, `CATAPULT / MACE / WINDMILL / POPCYLINDER` etc.
- **Scan point:** After `Level_MeshWorldCtor` returns, walk `board+0x8AC -> Level* +0x480 -> AthenaList*` (S1 list), `AthenaList_GetSize / GetIterator`, string-compare prefixes, **count** occurrences per type.
  - Counts drive heap size calc. No `level` index used. Works for any file in any slot.
- **No per-level lists in txt:** `[OBJECTS]` / `g_levelData[].meshes[]` become deprecated for spawns. Keep only global fallback / debug.

## 3. Implementation: Option B — Extension Heap

- **Base board stays vanilla size.** Do **not** patch alloc size. Only patch the 15 ctors to init ext pointer to NULL / handle NULL safely.
- **Ext alloc:** `HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalSize)` after scan, where:
  ```
  totalSize = sum(count[type] * sizeof(Object)) + sum(listSlots * 0x410) + fixedFieldBlock
  ```
  - Fixed block = bumpers `lit` + `ehVector` tail + mesh ptrs + rotation floats (see OFF_* below). Always included; dynamic part = lists/objects.
- **EXT_PTR storage:** `*(void**)(board + 0xAB00) = ext` — beyond vanilla max `0x6498`, stable, board-relative, no global map needed. `0xAB00` no longer alloc size, just a pointer slot.
- **Offsets are fixed inside ext, dynamic heap size:** `OFF_*` are constant ext-relative offsets (mirror of `UNI_*` block, see §4). Code never does `board+UNI_*`; always `ext+OFF_*`.
- **Feature enable:** `g_extFeat[board]` (or `ext->featBits`) auto-set per board instance when handler succeeds — not `g_updateFeatures[level]`. Each ext carries its own `featMask` so `Feature_SwirlZones / Windmill / BridgeAnimation` gate on `ext`. Survives file swaps, multiple boards.
- **Lifetime:** Free on next `AdvanceRace` alloc / board dtor (`HeapFree`). Null-check `EXT_PTR` everywhere. On level reload, re-scan, re-alloc.

## 4. Layout: `OFF_*` Map (ext-relative, fixed)

Mirrors current `UNI_*` block but ext-relative. Keep 4-byte alignment; floats = 4B, ptrs = 4B, AthenaList = `0x410`, `ehVector` elem = `0x418`.

```c
// ext layout — fixed offsets, dynamic heap size
#define OFF_TIPPER_MESH      0x00  // void*  (was UNI_MESH_0 slice)
#define OFF_TIPPER_RENDER    0x04  // void*  render obj for tipper
#define OFF_WATER_MESH       0x08  // void*  MeshNode WaterWheel
#define OFF_WATER_ROT_X      0x0C  // float  S1 x2 (pitch)
#define OFF_WATER_ROT_Y      0x10  // float  S1 y2 (yaw)
#define OFF_WATER_ROT_Z      0x14  // float  S1 z2 (roll)
#define OFF_SWIRL_MESH       0x18  // void*  Swirl mesh 1 (UNI_MESH_6)
#define OFF_SWIRL_MESH2      0x1C  // void*  Swirl mesh 2 (UNI_MESH_11)
#define OFF_SWIRL_POS1_X     0x20  // float
#define OFF_SWIRL_POS1_Y     0x24
#define OFF_SWIRL_POS1_Z     0x28
#define OFF_SWIRL_ANGLE1     0x2C
#define OFF_SWIRL_SPEED1     0x30
#define OFF_SWIRL_POS2_X     0x34
#define OFF_SWIRL_POS2_Y     0x38
#define OFF_SWIRL_POS2_Z     0x3C
#define OFF_SWIRL_ANGLE2     0x40
// ... (UNI_MESH_0..15 sequential block preserved as OFF_MESH_0..15 if needed)
#define OFF_BUMPER_LIT       0x50  // 8*4 = 32B  (was UNI_BUMPER_LIT 0x85C0)
#define OFF_EHVECTOR         0x70  // 8*0x418 = 0x20C0 (was UNI_EHVECTOR 0x6500) — keep 4-aligned, place after fixed fields or at ext+0x100 for alignment
#define OFF_BRIDGE_ANGLE     0x2130 // float 45.0 (UNI_BRIDGE_ANGLE)
#define OFF_BRIDGE_STATE     0x2134 // int
#define OFF_BRIDGE_COUNTER   0x2138
#define OFF_WINDMILL_X       0x213C // float
#define OFF_WINDMILL_Y       0x2140
#define OFF_WINDMILL_Z       0x2144
#define OFF_WINDMILL_ANGLE   0x2148
#define OFF_WINDMILL_SPEED   0x214C
#define OFF_WINDMILL_STATE   0x2150
#define OFF_WINDMILL_COUNTER 0x2154
#define OFF_WINDMILL_DECAY   0x2158
// Render-data mirror (was 0xA880..):
#define OFF_REND_GLASS_S1    0x2160 // 34B glass block
#define OFF_REND_TOWER       0x2184 // 12B tower block
#define OFF_REND_SKY         0x2190 // 20B sky block
#define OFF_LIST_0           0x21C0 // AthenaList 0x410 each — dynamic count
#define OFF_LIST_1           0x25D0 // (OFF_LIST_0 + 0x410)
#define OFF_LIST_2           0x29E0
// ... OFF_LIST_3..7 as needed
// OFF_BONK_STORE / OFF_BELL_OBJ / OFF_SAW*_OBJ etc. remain as fixed OFF_* in the 0x2130 region if used
```

> Align `OFF_EHVECTOR` on 4B (or 8B) boundary; pad `OFF_BUMPER_LIT` block first. Exact hex above is illustrative — keep sequential packing from `OFF_TIPPER_MESH`, then pad to `OFF_EHVECTOR`, then lists last so `totalSize = OFF_LIST_0 + nLists*0x410`.

**Example heap sizes (fixed+dynamic):**

- `WarmUp + bumpers` → `lit (32B) + ehVector (0x20C0) + 16 mesh ptrs (64B) + render-data (~68B) + 1 list header` ≈ **0x200–0x600** if bumpers kept; pure vanilla ≈ `0x200` without ehVector pre-alloc (lazy).
- `Dizzy + wheel + 3 tippers + swirl + gluebies` → fixed block + `1 wheel mesh + 3 tipper pairs + swirl meshes + gluebie list + 2 swirl lists` ≈ **0x2000** (old union was `0xAB00` always).
- `Master + bridge + all` → similar `0x2000–0x3000` vs `0xAB00` waste.

- Base `board` alloc untouched (vanilla `boardSize` per level, max `0x6498`). Only ext is variable. Free-on-load keeps tournament loop clean.

## 5. Callouts to fix during rewrite

- **`UniversalCreateDynamicObjects` WATERWHEEL handler:** currently takes only `x/y/z`, **discards `x2/y2/z2` (S1 rot)**. Fix: store all three to `ext+OFF_WATER_ROT_X/Y/Z` (floats). Handler sig is `(board, ext, x,y,z, x2,y2,z2)`.
- **TIPPER handler:** same bug — discards rot (`x2/y2/z2`) passed from S1. Fix identically (store `OFF_TIPPER_ROT_*` or reuse `OFF_WATER_ROT_*` pattern per tipper instance).
- **`Feature_SwirlZones` / `Feature_Windmill` / `Feature_BridgeAnimation`:** replace every `board+UNI_*` with `ext+OFF_*` (resolve `ext = *(void**)(board+0xAB00)` once per tick, early-out if NULL).
- **Waterwheel rotation math:** currently hardcoded Y-only (`RotY(angle)`). Fix: build `YawPitchRoll( storedRotY, storedRotX, storedRotZ ) * RotY(spinAngle)` — use stored `OFF_WATER_ROT_*` as base orientation, spin = Y-axis local.
- **Bridge / bumpers / lists:** audit all `UNI_EHVECTOR / UNI_BUMPER_LIT / UNI_LIST_* / UNI_MESH_*` consumers; redirect via `OFF_*` macros. Grep for `0x65` / `0x85` / `0xA8` board offsets.
- **Render features:** `REND_*` reads must also use `ext+OFF_REND_*`, not `board+0xA880`. Same NULL-ext guard.

## 6. File-swap example (verifies goal)

```bat
copy levels\Level3.MESHWORLD levels\level1.meshworld
:: launch game, start WarmUp slot (level 1)
:: Expected: WarmUp board now spawns Dizzy waterwheel + tippers + swirl + gluebies
:: No LevelFeatures.txt edit, no g_levelData[level] change — scan finds S1 refs in swapped file.
```

- Demonstrates **100% file customizability**: any `.MESHWORLD` dropped into any slot brings its own objects. Registry + scan is slot-agnostic.
- Reverse: dropping WarmUp meshworld into Dizzy slot shrinks ext to `≈0x200` — no wasted `0xAB00`.

---
*Next steps for RodentRacer: implement S1 scanner after `Level_MeshWorldCtor`, wire `HeapAlloc`/`HeapFree`, add `OFF_*` header, migrate handlers + feature ticks to `ext+OFF_*`.*

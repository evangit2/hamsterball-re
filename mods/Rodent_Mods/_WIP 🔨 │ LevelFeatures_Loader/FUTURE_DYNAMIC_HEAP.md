# Future: Dynamic Per-Level Heap (parked 2026-08-28)

RodentRacer confirmed: fixed 48KB ext heap is correct for *now* (safer to test), but custom levels need **multiple instances** of the same object type (e.g. 3 waterwheels, 5 tippers) which fixed single-drawer offsets can't handle.

## Why dynamic is needed
Current ext has one fixed slot per type:
- `UNI_MESH_0` = one waterwheel mesh
- `UNI_BONK_STORE` = one bridge mesh
- `UNI_WINDMILL_X` = one windmill

Three waterwheels would fight for the same slot — only one works. Etched in `LevelFeatures.c` as fixed `UNI_*` constants.

## Design (from DYNAMIC_LAYOUT_NOTE.md)
On level load, after `Level_MeshWorldCtor`:
1. Scan `meshWorld+0x480` S1 refs, **count** per type (`WATERWHEEL: 3`, `TIPPER: 5`, `SWIRL: 2`, etc.)
2. `totalSize = fixedFields + sum(count[type] * sizeof(Object|Mesh|List))` where `fixedFields` = bumpers `lit` + `ehVector` tail + rotation floats + bridge/windmill state, and `List` = `0x410` per AthenaList.
3. `HeapAlloc(totalSize)` instead of fixed `0xC000`. Offsets become `OFF_*` *relative* but base shifts per count — or keep `OFF_*` fixed for singletons and allocate arrays after `OFF_LIST_0`.
4. Per-level feat bits already done (`OrBoardFeat` from S1 scan), so dynamic heap naturally gives WarmUp ≈ 0.5KB vs Dizzy ≈ 8KB vs Master ≈ 12KB, no waste, no limit.

## Parked until after Windows crash-free confirmation
Don't implement until `a5088c9e` + heap/OFF fixes are proven on real Windows (no `0x452783`). This note is the queue for the next step.

# LevelFeatures_Loader — HB+ Port Design Notes (parked 2026-08-28)

RodentRacer requested this be documented for a future HB+ twin. No code built yet.

## Goal if we go HB+
Make `LevelFeatures_Loader` an HB+ mod (`Mods/LevelFeatures_HBPlus.dll`) that still does everything the bass proxy does (universal board/sciene ctor, vtable slots 1/19/29/33, ext heap `0xC000` + `g_extMap`, S1 scan, `RaceFiles.txt`), plus becomes a *meta-loader* for per-level HB+ mods.

## Per-level HB+ mod auto-load
When a level is loaded from a folder, also load any HB+ DLLs in that same folder.

* **Source of truth:** `g_levelDir` already set in `UniversalConstructor` from resolved `RaceFiles.txt` path (e.g. `levels/MyLevel/MyLevel` → `levels/MyLevel/`). File fallback hook already uses this dir for textures/sounds/sub-MESHWORLDs via `CreateFileA/W` IAT.
* **Scan point:** Right after `g_levelDir` is set, before `Level_MeshWorldCtor` sub-loads, scan `g_levelDir\*.dll`.
* **Load:** For each `dll` found:
  * `HMODULE h = LoadLibraryEx(trial, NULL, 0)` (trial = `g_levelDir` + `basename`)
  * `auto init = GetProcAddress(h, "Initialize")` // HB+ ModAPI entry; also try `ModInitialize` fallback
  * `init(HBPlus_GetAPI())` and register in `g_perLevelMods[32]` handles + `HMODULE` list
  * If init returns vtable, push to HB+ `g_Mods` vector so `onEventPlaneCollide`/`onGameUpdate` dispatch covers it
* **Unload:** In `Hook_AdvanceRace` after calling original and after sweeping `g_extMap` stale boards, iterate `g_perLevelMods`, call `Shutdown`/`onLevelUnload` if exported, `FreeLibrary`, clear list. Next `UniversalConstructor` loads next level's DLLs fresh.

### Result
* Level bundle = self-contained: `MyLevel.MESHWORLD` + `textures/*.png` + `sounds/*.ogg` + `Meshes/*.MESH` (already via file fallback) **plus** `MyLevelLogic.dll` that only runs while that level is loaded. No `Mods\` shuffle, no global `LevelFeatures.txt` edit. Dropping `levels/Volcano/` into any race slot just works.
* Unloading prevents Volcano lava logic from leaking into next race's `g_extMap`.

## Compatibility fixes needed for HB+ port
* `DispatchCollisionEvents 0x40C5D0` is already hooked by HB+ for `onEventPlaneCollide`. Make `InstallHook` chain-aware: if `orig[0]==0xE9` hook the existing trampoline instead of bailing.
* `CreateFileA/W` IAT: same chaining — if thunk already points to `Hook_CreateFileA`, save it as `g_orig` so fallback chains through HB+'s loader.
* Init timing: HB+ loads after game start, so 15 ctor patches + vtable patches must be re-applied late and handle already-created WarmUp board.
* `HamsterballAPI.h` packing: MinGW `static_assert` offsets need `nocrt` shim + 17-entry vtable fixups, `VirtualQuery` for config path (not `GetModuleFileNameA(NULL)`).

## Deferred
Do not build until `bass.dll` proxy is stable on vanilla (current priority). Tower/Dizzy/Intermediate verified, other races + file-fallback still need Windows testing.

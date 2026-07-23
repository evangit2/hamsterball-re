# LevelFeatures_Loader — Crash Debugging Session

## Current Status
- **Crash address:** `0x00452783` (RVA `0x00052783`) — middle of a PUSH instruction at `0x45277F`
- **Reproduces on:** ALL races, not just Warm-Up
- **Trigger:** Clicking to start a race from Time Trial Menu
- **Crash context:** CURRENTOBJECT=Time Trial Menu, CURRENTOPERATION=MouseDown, EXTENDED_INFO=FinishLoad(OK)
- **Runtime:** 9-46 seconds (varies by how long user waits before clicking)

## What's Been Ruled Out
1. **UniversalRender (vtable slot 24) is NOT the cause** — disabled slot 24 patching, crash persists identically
2. **Not a vtable dispatch** — `0x452783` does not appear in any of the 15 level vtables
3. **Not in our DLL** — crash is in Hamsterball.exe's .text section
4. **Not allocation patches** — alloc patch RVAs (0x27xxx) are nowhere near 0x52xxx
5. **Not a pre-existing modded-game crash** — user is testing on clean install at `C:\Program Files (x86)\Raptisoft\Hamsterball\`

## Crash Analysis
- `0x452783` is byte 4 of `PUSH 0x3F800000` (float 1.0) at `0x45277F`
- EIP jumped to middle of instruction → corrupted function pointer or return address
- The function at `0x452770` is a color/material init: pushes 3× 1.0f (white RGB), calls `Matrix_Scale4x4` (0x453150), then FLD+FADD
- Ghidra has NO function identified at this address (gap between FUN_00451df0 ending at 0x45215E and Rect_ContainsPoint at 0x453100)

## Log Analysis (lfdebug.log)
The log ends at `"Extra meshes loaded"` and does NOT show:
- `"UniversalPostSetup done"`
- `"Calling Board_Setup (vtable+0x80)..."`
- `"Board_Setup done"`
- `"UniversalBoardUpdate #1"` etc.

**This means the crash happens DURING steps 9a-9g** (between "Extra meshes loaded" and "UniversalPostSetup"), or the constructor never returns.

Steps 9a-9g previously had NO logging. Latest commit (`c713ca7e`) adds DebugLog after EACH step 9a-9g. The next test should reveal exactly which step crashes.

## Steps 9a-9g Code (lines ~1716-1787 of LevelFeatures.c)
For WarmUp (raceIndex=1), LevelData has:
- meshCount=0, meshPaths={} (empty)
- athenaListOffsets: {UNI_LIST_0..UNI_LIST_7} (all 8)
- ehVectorOffset=UNI_EHVECTOR (0x6500), count=8, stride=0x418
- zeroFillOffsets={0} (none)
- assignTexOffsets={0} (none)
- soundChannelOffset=0 (none)
- bridgeParamOffset=0 (none)
- specialByteOffset=0, specialDwordOffset=0 (none)

So for WarmUp, steps 9d/9e/9f/9g should be no-ops. The likely crash points are:
- **9a: AthenaList_Init** (initializes 8 lists at UNI_LIST_0..7)
- **9b: ehVector constructor** (initializes bumper array at UNI_EHVECTOR)
- **9c: zero-fills** (should be empty for WarmUp)

## Current Build State
- **Slot 24 (UniversalRender): DISABLED** — commented out in InstallVtablePatches
- **Slot 24 backup:** Original per-level render functions still in vtables
- **Backups:** `bass.dll.bak` and `LevelFeatures.c.bak` in mod folder (from commit `ff8dd7a5` with slot 24 enabled)
- **Latest commit:** `c713ca7e` — has granular step 9a-9g logging
- **Wine crash test:** PASS (exit 124, 35s survival)

## Key Commits (most recent first)
1. `c713ca7e` — Add granular logging to steps 9a-9g
2. `9d495710` — Disable slot 24 + add diagnostic logging
3. `ff8dd7a5` — Fix Glass render type mismatch + N:TENBONUS1 offset bug
4. `9586fc18` — Dedicated render data section (REND_* offsets)
5. `71032deb` — Dynamic render features (no hardcoded defaults)
6. `dac47e55` — UniversalRender (vtable slot 24)
7. `4f83c9e1` — memset zero-fill old per-level offsets (0x4300-0x6500)

## Next Steps
1. **User tests latest build** → sends new lfdebug.log
2. **Check which step 9a-9g crashes** (or if constructor completes)
3. If constructor completes but crash persists → issue is in Board_Setup, UniversalPostSetup, or the game's own race-start code reading zeroed offsets
4. **Possible culprit: the memset** — zeroing board+0x4300..0x6500 may corrupt data the game needs during FinishLoad/race-start. The game's own color init function (at ~0x452770) may read from those offsets.
5. If step 9a or 9b crashes → AthenaList_Init or ehVector might be writing outside bounds. Check if the unified offsets overlap with game-internal structures.

## File Locations
- Source: `~/hamsterball-re/mods/_WIP 🔨 │ LevelFeatures_Loader/LevelFeatures.c`
- Backup: `~/hamsterball-re/mods/_WIP 🔨 │ LevelFeatures_Loader/LevelFeatures.c.bak`
- DLL backup: `~/hamsterball-re/mods/_WIP 🔨 │ LevelFeatures_Loader/bass.dll.bak`
- Zip: `~/hamsterball-re/mods/_WIP 🔨 │ LevelFeatures_Loader/LevelFeatures_Loader.zip`
- Config: `~/hamsterball-re/mods/_WIP 🔨 │ LevelFeatures_Loader/LevelFeatures.txt`
- Game dir (Wine): `~/hamsterball-re/originals/installed/extracted/`
- GhidraMCP: running on localhost:8089, program=Hamsterball.exe

## User Testing Environment
- **OS:** Windows NT 6.2 (Build 9200) — Windows 8/10/11
- **Game path:** `C:\Program Files (x86)\Raptisoft\Hamsterball\Hamsterball.exe`
- **Display:** D3DFMT_X8R8G8B8, 1024x768:16, windowed (FULLSCREEN: false)
- **SAFEMODE: true** — game running in safe mode
- **D3D8:** 10.0.26100.8115, D3D9: 10.0.26100.8737

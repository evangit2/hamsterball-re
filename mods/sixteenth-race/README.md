# Sixteenth Race Mod ("Test Race")

Adds a 16th race to Hamsterball by cloning the Warm-Up Race board constructor and scene setup.

## What It Does

1. **Tournament_AdvanceRace** (0x00427080): Extends the 15-case switch to 16 cases. Case 15 allocates a board (same size as Warm-Up: 0x436C bytes) and calls `BoardLevel1_WarmUp_Ctor` (0x0041CA40), then overwrites the board name strings to "TEST RACE" / "Board (Test)".

2. **TourneyMenu_CreateBoard** (0x00426780): Extends the 15-case arena switch to 16 cases. Case 15 allocates an arena board (0x47E0 bytes) and calls `ArenaBoard_Warmup_Ctor` (0x004224A0), then overwrites the name strings.

3. **GetLevelPath** (0x0040D1C0): Hooks the hardcoded `"levels\level1"` string push. When the board's race name starts with "TEST", redirects to `"levels\leveltest"` instead. This makes the Test Race load `LevelTest.MESHWORLD`.

4. **PracticeMenu** (0x0042EA30): Injects a 16th `UIList_AddItem` call after the Impossible Race entry, before the separator. Uses the Warm-Up preview image and "Test Race" as the display name.

5. **Level File**: On DLL load, copies `Levels\Level1.MESHWORLD` to `Levels\LevelTest.MESHWORLD` if it doesn't already exist.

## Patch Points (all verified against original EXE)

| Address | Original | Patch | Description |
|---------|----------|-------|-------------|
| 0x004270FB | 0x0E | 0x0F | Tournament switch CMP (14→15 max index) |
| 0x00427658 | 90909090 | cave addr | Tournament jump table entry 15 |
| 0x0042679F | 0x0E | 0x0F | TourneyMenu switch CMP (14→15 max index) |
| 0x00426AEC | 90909090 | cave addr | TourneyMenu jump table entry 15 |
| 0x0040D202 | 68 E0 F8 4C 00 | E9 rel32 | GetLevelPath string push redirect |
| 0x0042F4F7 | 6A 0A 8B CE E8 | E9 rel32 | Practice menu separator hook |
| 0x0041A619 | 0F 85 9D 00 00 00 | 90×6 | Race-end JNZ→NOP: tournament falls through to TourneyMenu |

## Installation

1. Rename the original `bass.dll` to `bass_real.dll` in the game directory.
2. Copy the modded `bass.dll` to the game directory.
3. The mod auto-creates `Levels\LevelTest.MESHWORLD` from `Level1.MESHWORLD` on first launch.
4. To use a custom level, replace `LevelTest.MESHWORLD` with your own file.

**Why rename?** The modded `bass.dll` is a proxy — it loads `bass_real.dll` (the original) on first call and forwards all audio functions to it. Without the real BASS library, the game crashes during startup because `BASS_MusicLoad` returns NULL and the game dereferences the invalid music handle.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll sixteenth_race.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

- **Board alloc size**: 0x436C (same as Warm-Up Race)
- **Arena board alloc size**: 0x47E0 (same as Warm-Up Arena)
- **Race name**: "TEST RACE" (stored at board+0x29B4)
- **Board name**: "Board (Test)" (stored at board+0x868)
- **MESHWORLD path**: `levels\leveltest` (loaded via GetLevelPath hook)
- **Menu entry**: "Test Race" with ID "15", using Warm-Up preview image
- **No new unlock flag needed**: Test Race appears always unlocked in the practice menu
- **Tournament progression**: After winning Impossible (race 15), the NOP at 0x41A619 makes tournament mode fall through to the TourneyMenu (between-races screen) instead of ending. PLAY! calls Tournament_AdvanceRace → case 15 → Test Race board. After Test Race, the tournament ends naturally (Test Race board does not set the "last race" flag at +0x4348).

## Crash Test

Passed hbtestd crash test: 38.8s runtime, no crash, process alive.

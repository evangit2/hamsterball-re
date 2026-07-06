# Ghost Triggers Mod

## What It Does

Scans S1 ref points in level MESHWORLD data for entries prefixed `GT:`. When the **ghost ball** (Time Trial ghost replay) enters a trigger's proximity sphere, logs the event to `ghost_triggers_log.txt`.

## How To Use

1. Rename your original `bass.dll` to `bass_real.dll`
2. Copy this mod's `bass.dll` into your game folder
3. Add `GT:` ref points to level MESHWORLD files (see below)
4. Run the game in Time Trial mode with a ghost replay active

### MESHWORLD S1 Ref Point Format

Add S1 entries to the first section of your level's MESHWORLD file with the name `GT:EventName`:

```
GT:Checkpoint1     pos:(0, 200, 0)     rot:(0, 0, 50)
GT:Halfway         pos:(500, 200, 500)  rot:(0, 0, 75)
GT:FinishLine      pos:(0, 200, 1000)   rot:(0, 0, 100)
```

**Field meanings:**
- **Name**: `GT:` prefix + event name (e.g. `GT:Checkpoint1`)
- **Position**: X, Y, Z world coordinates of the trigger center
- **rot_y** (3rd rotation value): Repurposed as the **trigger radius**. If 0 or unset, defaults to 50.0

### How It Works

- **S1 scan**: On level load, scans the MESHWORLD's S1 AthenaList for `GT:` prefixed entries
- **Per-frame check**: After ghost ball playback position is set each frame, computes squared distance from ghost ball to each trigger center
- **Trigger**: Fires when ghost ball enters the trigger sphere (was outside last frame, now inside)
- **Cooldown**: 60 frames (1 second) between re-triggers of the same zone

### Log Output

```
TRIGGER FIRED: 'Checkpoint1' — ghost ball entered zone at (12.3, 200.5, 8.1), dist=45.2, radius=50.0, frame=1234
```

## Technical Details

| Component | Address/Offset |
|-----------|---------------|
| Frame epilogue hook | 0x46C1F1 (POP ESI / ADD ESP,8 / RET) |
| App pointer | 0x5341E0 |
| Scene pointer | App+0x178 |
| Ghost ball | Scene+0x361C |
| Ball position | Ball+0x164/0x168/0x16C |
| S1 AthenaList | Scene+0x8AC → Level+0x480 → MW+0x894 (embedded) |
| S1 count | MW+0x898 |
| S1 array | MW+0xCA0 |

## Limitations

- Only detects the **ghost ball** (not the player ball)
- Currently logs only — does not trigger game events (future: DCE injection)
- No trigger visualization (future: render markers)
- Requires a ghost replay to be active (Time Trial mode with existing best time)

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll ghost_triggers.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

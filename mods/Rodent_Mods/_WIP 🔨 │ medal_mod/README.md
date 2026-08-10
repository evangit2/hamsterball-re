# Diamond Weasel — 5th Secret Medal (bass.dll proxy mod)

Adds a **fifth, secret medal** per race to Hamsterball.

## What it does

When you finish a race faster than the secret threshold for that race:

### After any race, the results screen lays out the medals like this:

- **Golden weasel** (top-right) — fastest tier
- **Bronze, Silver, Gold** — a diagonal row of the earned standard medals
- **Diamond weasel** — the 5th secret medal, drawn **directly over the golden weasel** (same spot, top-right) when you beat the secret time

When you beat a race's secret time, the **golden weasel stays** and the
**diamond weasel renders directly over it** at the same location.

### Time-Trial menu (standings screen)

Each race's earned medal mini-icons are shown there. When you've earned the
diamond for a race, a **diamond mini-icon** appears **to the right of the
golden weasel** mini-icon for that race.
- The unlock is **persisted** per race (so it shows up on later visits).

## Files

| File | Purpose |
|------|---------|
| `bass.dll` | The mod (drop into the game folder next to `Hamsterball.exe`) |
| `diamond_weasel_config.txt` | Per-race secret times (edit freely) |
| `diamondweasel.png` | **You provide this** — put it in the game's `Textures\` folder |
| `diamondweasel-icon.png` | **You provide this** — the mini icon for the TT menu, in `Textures\` |

## Required icons

The golden weasel uses `Textures\goldenweasel.png` (results) and
`Textures\goldenweasel-icon.png` (TT menu). This mod loads:

- **`Textures\diamondweasel.png`** — results screen (same style as `goldenweasel.png`)
- **`Textures\diamondweasel-icon.png`** — TT-menu mini icon (same style as `goldenweasel-icon.png`)

Drop both (filename: same as the golden weasel ones, but "diamond" instead
of "golden").

## Install

1. Close Hamsterball.
2. Copy `bass.dll` over the one in your game folder (back it up first).
3. Copy `diamondweasel.png` and `diamondweasel-icon.png` into the game's `Textures\` folder.
4. Edit `diamond_weasel_config.txt` to set each race's secret time.
5. Launch Hamsterball.

If you don't have a `bass_real.dll` in the game folder, the proxy will still
load (it looks for `bass_real.dll` next to itself).

## Config

`diamond_weasel_config.txt`:

```
ICON=diamondweasel.png
MINIICON=diamondweasel-icon.png

[BEGINNER]
SECRET=30.0

[INTERMEDIATE]
SECRET=45.0
...
```

- `SECRET=<seconds>` — beat this time to earn the diamond weasel for that race.
- `ICON=<filename>` — optional override for the results-screen icon filename.
- `MINIICON=<filename>` — optional override for the TT-menu mini icon filename.

## How it works (for the curious)

The mod is a BASS proxy DLL (forwards all `BASS_*` calls to `bass_real.dll`).
It hooks two game functions:

1. **Results-screen gold draw (0x44EFD2)** — after the gold medal is drawn,
   if the player's time beats the secret threshold for the current race, the
   diamond sprite is drawn at the golden weasel's location (0x208, 0x63),
   directly over it (the diamond is the 5th medal, layered on top of the
   golden weasel).
2. **TT-menu golden-weasel append (0x42F927)** — when the diamond is unlocked
   for a race, appends a diamond mini-icon entry to the standings medal list
   right after the golden weasel, so it lays out to the right of it.
3. **Icon load** — loads `diamondweasel.png` and `diamondweasel-icon.png` via
   the game's own sprite loader (lazily, on first use).

The unlock flag is persisted to `diamond_weasel_unlocks.dat` (15 bytes, one
per race) next to the DLL.

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll diamond_weasel_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

## Credit

Built for RodentRacer. Reverse-engineered from `Hamsterball.exe` (medal
award + results display + sprite loader).
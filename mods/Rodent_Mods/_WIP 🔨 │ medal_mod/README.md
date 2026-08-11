# Diamond Weasel — 5th Secret Medal (bass.dll proxy mod)

Adds a **fifth, secret medal** per race to Hamsterball.

## What it does

When you finish a race faster than the secret threshold for that race:

### After any race, the results screen lays out the medals like this:

- **Golden weasel** (top-right) — fastest tier
- **Bronze, Silver, Gold** — a diagonal row of the earned standard medals
- **Diamond weasel** — the 5th secret medal, drawn **directly over the golden weasel** (same spot, top-right) when you beat the secret time

When you beat a race's secret time, the **golden weasel stays** and the
**diamond weasel** appears a moment later (3 medal-gaps after the **gold**
medal — around 165 frames, ~5.5s, 3× the game's standard gap) **directly over
the golden weasel** at the same location. You can tune this with
`DIAMOND_DELAY`.

### Time-Trial menu (standings screen)

Each race's earned medal mini-icons are shown there. When you've earned the
diamond for a race, a **diamond mini-icon** appears **to the right of the
golden weasel** mini-icon for that race.
- The unlock is **persisted** per race (so it shows up on later visits).

## Files

| File | Purpose |
|------|---------|
| `bass.dll` | The mod (drop into the game folder next to `Hamsterball.exe`) |
| `diamond_weasel_config.txt` | Optional per-race manual overrides (see below) |
| `diamondweasel.png` | **You provide this** — put it in the game's `Textures\` folder |
| `diamondweasel-icon.png` | **You provide this** — the mini icon for the TT menu, in `Textures\` |
| `diamond_weasel_unlocks.dat` | Created by the mod, per-race unlock flags (don't edit) |

## Required icons

The golden weasel uses `Textures\goldenweasel.png` (results) and
`Textures\goldenweasel-icon.png` (TT menu). This mod loads:

- **`Textures\diamondweasel.png`** — results screen (same style as `goldenweasel.png`)
- **`Textures\diamondweasel-icon.png`** — TT-menu mini icon (same style as `goldenweasel-icon.png`)

Drop both (filename: same as the golden weasel ones, but "diamond" instead
of "golden").

## Install

1. Close Hamsterball.
2. Make sure `bass_real.dll` is in the game folder — this is the original
   BASS DLL the mod forwards to. Without it the game can't load music/sound
   and will crash. (If you don't have one, install the latest BASS from
   un4seen.com and drop the 32-bit `bass.dll` renamed to `bass_real.dll`.)
3. Copy `bass.dll` (the mod) over the one in your game folder (back up the
   original first).
4. Copy `diamondweasel.png` and `diamondweasel-icon.png` into the game's `Textures\` folder.
5. (Optional) Edit `diamond_weasel_config.txt` to add per-race manual overrides.
6. Launch Hamsterball.

## How the diamond time is chosen

The diamond time for each race comes from **three sources, in priority order**:

1. **`racedata.xml`** — the mod reads the *same* `racedata.xml` the game uses
   (in the game folder). If a race's existing block contains a `<DIAMOND>`
   element, that value is used — exactly the way the game reads its other
   medal times (`<WEASEL>`, `<GOLD>`, etc.). Add one like this to set a race:
   ```xml
   <BEGINNERRACE>
       <TIME>60</TIME>
       <PAR>47.0</PAR>
       <WEASEL>6.6</WEASEL>
       <DIAMOND>4.2</DIAMOND>
       ...
   </BEGINNERRACE>
   ```
2. **DLL defaults** — if `racedata.xml` has no `<DIAMOND>` for a race (or the
   file is missing), the time baked into the DLL is used:
   Warm-Up **3.5**, Beginner **12.3**, Intermediate **14.0**, Dizzy **23.0**,
   Tower **24.0**, Up **20.0**, Neon **28.0**, Expert **29.0**, Odd **12.0**,
   Toob **25.0**, Wobbly **23.0**, Glass **30.0**, Sky **32.0**, Master **40.0**,
   Impossible **26.0**.
3. **Config `SECRET=`** — an explicit per-race override in
   `diamond_weasel_config.txt` beats both of the above.

## Config

`diamond_weasel_config.txt`:

```
ICON=diamondweasel.png
MINIICON=diamondweasel-icon.png

[BEGINNER]
;SECRET=12.3
```

- `SECRET=<seconds>` — optional manual override (highest priority). Leave it
  commented out to use the `racedata.xml` `<DIAMOND>` value (or the DLL default
  if there is none).
- `ICON=<filename>` — optional override for the results-screen icon filename.
- `MINIICON=<filename>` — optional override for the TT-menu mini icon filename.
- `DIAMOND_DELAY=<frames>` — optional. How many frames after the **gold** medal
  appears before the diamond appears (default `165`, 3× the game's ~55-frame
  stagger between the other medals). Set `0` to draw it on the same frame as gold.

## How it works (for the curious)

The mod is a BASS proxy DLL (forwards all `BASS_*` calls to `bass_real.dll`).
It hooks three things:

1. **Results-screen gold draw (0x44EFD2)** — this runs a genuine **5th medal
   block**, mirroring exactly how the game draws each medal. After the gold
   medal is drawn, it waits `DIAMOND_DELAY` frames (default 165 = 3× the
   game's own bronze→silver→gold gap of 55), then if the player's time beats the secret
   threshold for the current race, the diamond sprite is drawn at the golden
   weasel's location (0x208, 0x63), directly over it.
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
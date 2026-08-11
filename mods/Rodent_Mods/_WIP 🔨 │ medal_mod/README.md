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
the golden weasel** at the same location.

### Time-Trial menu (standings screen)

Each race's earned medal mini-icons are shown there. When you've earned the
diamond for a race, a **diamond mini-icon** appears **to the right of the
golden weasel** mini-icon for that race.
- The unlock is **persisted** per race (so it shows up on later visits).

## Files

| File | Purpose |
|------|---------|
| `bass.dll` | The mod (drop into the game folder next to `Hamsterball.exe`) |
| `diamond_weasel_unlocks.dat` | Created by the mod, per-race unlock flags (don't edit) |

There is **no config file** — the diamond times come from the game's own
`racedata.xml` (see below), with DLL-baked fallbacks.

## The diamond art is fully in-memory (not on disk)

The diamond weasel medal and mini-icon **do not exist as files anywhere** —
not in the game folder, not even inside the DLL as a recognizable image. The
pixels are embedded in the DLL as XOR-encrypted raw data and decrypted + built
into a D3D texture entirely in memory at runtime, then seeded straight into
the game's texture cache. The game's file loader never runs for them, and no
temporary file is ever written. Nothing named `diamond` can be found on disk.

## Required icons

Only the game's own golden-weasel icons are used (`Textures\\goldenweasel.png`
and `Textures\\goldenweasel-icon.png`) — the diamond versions are derived from
the embedded in-memory art. **You do not need to provide any diamond icon
files.**

## Install

1. Close Hamsterball.
2. Make sure `bass_real.dll` is in the game folder — this is the original
   BASS DLL the mod forwards to. Without it the game can't load music/sound
   and will crash. (If you don't have one, install the latest BASS from
   un4seen.com and drop the 32-bit `bass.dll` renamed to `bass_real.dll`.)
3. Copy `bass.dll` (the mod) over the one in your game folder (back up the
   original first).
4. Launch Hamsterball. No icon files needed.

## How the diamond time is chosen

The diamond time for each race comes from **two sources, in priority order**:

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

(Note: the game's race *names* and the `racedata.xml` *block names* are offset
by two slots — e.g. the "Warm-Up" race loads the `BEGINNERRACE` block, and the
"Beginner" race loads `CASCADERACE`. The mod matches each race to the block the
game actually uses.)

## How it works (for the curious)

The mod is a BASS proxy DLL (forwards all `BASS_*` calls to `bass_real.dll`).
It hooks three things:

1. **Results-screen gold draw (0x44EFD2)** — this runs a genuine **5th medal
   block**, mirroring exactly how the game draws each medal. After the gold
   medal is drawn, it waits 165 frames (3× the game's own bronze→silver→gold
   gap of 55), then if the player's time beats the secret threshold for the
   current race, the diamond sprite is drawn at the golden weasel's location
   (0x208, 0x63), directly over it.
2. **TT-menu golden-weasel append (0x42F927)** — when the diamond is unlocked
   for a race, appends a diamond mini-icon entry to the standings medal list
   right after the golden weasel, so it lays out to the right of it.
3. **Icon load (in-memory)** — the diamond art is embedded in the DLL as
   XOR-encrypted raw pixels. On first use the mod decrypts the pixels in
   memory, builds a D3D8 texture via the game's device (CreateTexture +
   LockRect + copy + UnlockRect), constructs a texture-cache object in the
   game's exact layout, and seeds it into the game's texture cache under the
   request name — so the game's own sprite loader resolves it from memory and
   the file loader never runs. No `diamond*.png` file exists anywhere.

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
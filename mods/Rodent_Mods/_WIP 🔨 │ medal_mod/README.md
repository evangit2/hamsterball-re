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

There is **no config file** — the diamond times come from the game's own
`racedata.xml` (see below), with DLL-baked fallbacks.

## Unlocks are stored in the registry (like the game's own medals)

The diamond unlock flags live in the **same registry key the game uses** for
its medal and unlock state:

```
HKEY_CURRENT_USER\Software\Raptisoft\Hamsterball
    DiamondMedals   REG_BINARY   (15 bytes, one flag per race)
```

This is the exact key the game itself writes (its own medal flags live in the
`Medals` value there, plus per-race/arena unlock booleans). The `DiamondMedals`
value is created by the mod **only at the same moment it writes the diamond
PNG assets** (first unlock), so editing the registry the same way you'd edit
the game's medals will also let you grant/revoke diamond unlocks. No `.dat`
file is used.

## The diamond art is write-on-first-unlock (not on disk)

Before you earn your first diamond, `diamondweasel.png` and
`diamondweasel-icon.png` **do not exist** — nothing named `diamond` is on disk
to spoil the secret. The PNG bytes are embedded in the DLL as XOR-encrypted
data. The instant you beat a secret time, the mod decrypts them and writes
both files into `Textures\`, then they're loaded by the game through its
normal file path — exactly like the golden weasel.

After the first unlock the PNGs persist on disk so they render normally on
subsequent visits.

**You do not need to provide any diamond icon files.**

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
3. **Icon load (write-on-first-unlock)** — the diamond PNG bytes are embedded
   in the DLL as XOR-encrypted data (not extractable from the DLL). On the
   *first* time a diamond is awarded, the mod decrypts them and writes
   `diamondweasel.png` + `diamondweasel-icon.png` into `Textures\`, then the
   game loads them through its normal file path. Before any unlock, no
   `diamond*.png` exists on disk.

The unlock flag is persisted as the `DiamondMedals` registry value (15 bytes,
one flag per race) in `HKCU\Software\Raptisoft\Hamsterball`, written at the
same moment the PNG assets are created.

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll diamond_weasel_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

## Credit

Built for RodentRacer. Reverse-engineered from `Hamsterball.exe` (medal
award + results display + sprite loader).
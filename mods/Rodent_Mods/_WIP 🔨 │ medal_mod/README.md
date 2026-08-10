# Diamond Weasel — 5th Secret Medal (bass.dll proxy mod)

Adds a **fifth, secret medal** per race to Hamsterball.

## What it does

When you finish a race faster than the secret threshold for that race:

### After any race, the results screen lays out the medals like this:

- **Golden weasel** (top-right) — fastest tier
- **Bronze, Silver, Gold** — a diagonal row of the earned standard medals
- **Diamond weasel** — the 5th secret medal, appearing **after gold** (continuing the diagonal) when you beat the secret time

When you beat a race's secret time, the **golden weasel stays** and the
**diamond weasel appears as a 5th medal after gold**. The normal weasel
"burst of stars" still plays on the gold medal, and the diamond appears
alongside it.
- The unlock is **persisted** per race (so it shows up on later visits).

## Files

| File | Purpose |
|------|---------|
| `bass.dll` | The mod (drop into the game folder next to `Hamsterball.exe`) |
| `diamond_weasel_config.txt` | Per-race secret times (edit freely) |
| `diamondweasel.png` | **You provide this** — put it in the game's `Textures\` folder |

## Required icon

The golden weasel uses `Textures\goldenweasel.png`. This mod loads
**`Textures\diamondweasel.png`** (same name, "diamond" instead of "golden").
Drop your provided PNG there. It should be the same size/style as the golden
weasel icon (32×32).

## Install

1. Close Hamsterball.
2. Copy `bass.dll` over the one in your game folder (back it up first).
3. Copy `diamondweasel.png` into the game's `Textures\` folder.
4. Edit `diamond_weasel_config.txt` to set each race's secret time.
5. Launch Hamsterball.

If you don't have a `bass_real.dll` in the game folder, the proxy will still
load (it looks for `bass_real.dll` next to itself).

## Config

`diamond_weasel_config.txt`:

```
ICON=diamondweasel.png

[BEGINNER]
SECRET=30.0

[INTERMEDIATE]
SECRET=45.0
...
```

- `SECRET=<seconds>` — beat this time to earn the diamond weasel for that race.
- `ICON=<filename>` — optional override for the icon filename.

## How it works (for the curious)

The mod is a BASS proxy DLL (forwards all `BASS_*` calls to `bass_real.dll`).
It hooks two game functions:

1. **Results-screen gold draw (0x44EFD2)** — after the gold medal is drawn,
   if the player's time beats the secret threshold for the current race, the
   diamond sprite is drawn at a position continuing the medal diagonal
   (below-right of gold).
2. **Icon load** — loads `diamondweasel.png` via the game's own sprite loader
   (lazily, on first use), so no extra game systems are touched.

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
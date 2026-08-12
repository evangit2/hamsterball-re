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

## The unlock is atomic (all-or-nothing)

Unlocking a diamond requires writing **everything it needs** — the
`DiamondMedals` registry value *and* the two PNG assets (plus any future
effects/assets). If **any** of those writes fails, the mod treats the diamond
as **never earned**: it does not draw, does not show on the TT menu, applies
no effects, and does not persist. No partial unlock can ever exist — you can't
end up with a saved flag but a missing image, or vice versa. If the environment
becomes writable later, re-earning the diamond will then succeed normally.

## Medal-award effects (pop sound + star ring)

When the diamond is first unlocked, the mod runs the **same award effects the
game uses for its other medals** (verified in `FUN_0044df70`). On the first
earn it (1) plays the medal **pop sound** through the game's own medal sound
channel, and (2) spawns a **ring of star particles** — 18 `ArenaScoreParticle`
objects arcing out from the diamond's spot, exactly like a medal reveal. These
fire only on the genuine first-earn for a race (gated by the atomic unlock), so
re-visiting an already-earned diamond playback does not repeat them.

**Golden-weasel white-fade** (rebased to the gold medal award)

The unlock sequence is **timed relative to the moment the gold medal is
awarded** — not from the moment you touch the goal. The game awards gold when
the results frame counter reaches the gold gate (`results+0x4c`, frame 400 for
the race results object). The mod computes a *sequence frame* since that instant
(`results+0x10 − results+0x4c`), so the whole reveal plays off gold:

- **55 frames after gold** — the golden weasel starts turning white
- **150 frames after gold** — it is fully white (saturating multiplier 4.0)
- **240 frames after gold** — the diamond trophy reveal

The white-fade drives the game's native color-multiplier
(`Graphics_SetColorMultiplier` + gfx+0x7A8) up to a saturating 4.0, so the
sprite blows out to pure white. Applied only around the weasel draw and cleared
immediately after, so no other on-screen draw is tinted.

**Golden-weasel suction vortex**

While the weasel is turning white it is surrounded by a **suction vortex**:
small white **thin rectangles** fade in at random angles and radii around the
trophy, then pull straight inward toward its center (no swirl), disappear behind
it, and the cycle repeats for ~100 result-frames. Each streak has a soft
**alpha gradient**: it fades in at both ends and is most opaque in its middle
(subdivided into `VORTEX_SEGS=8` segments with a tapered per-vertex alpha). It is
drawn with the game's D3D8 device directly (`DrawPrimitiveUP`, screen-space
`D3DFVF_TLVERTEX` quads) **before** the trophy sprite draw, so the streaks render
behind the golden weasel. Each streak also fades out as its inner tip nears the
trophy center, reaching fully transparent exactly when the tip reaches the
center — so the streak never visibly sticks out past the trophy before
despawning. Its center is the trophy's true center (sprite top-left + half of
its width/height), and center tracking replicates the game's own
`Gfx_TransformX/Y` world→screen math so it stays anchored at any resolution.

After the ~100-frame active cycle, a ~30-frame tail plays: no new streaks spawn,
the weasel stays white, and any remaining streaks drift inward and fade to
nothing. A single one-shot **whoosh** (`sounds\\whoosh`, via the mod's real-BASS
layer) plays once when the vortex begins at the 55-frame mark — it is not
looped — and the stream is freed when the cycle ends.

The white trophy then **holds for another 55 frames** with no particles, before
**reverting to its normal golden color.**

## The reveal at gold + 240

At **240 frames after the gold medal is awarded** the golden weasel **stops
rendering entirely** and the **diamond trophy replaces it in that same frame**,
firing the reveal effects (medal pop + star ring) on the first frame of the swap.
This is the *only* diamond reveal — the old gold-gap 5th-medal reveal is
disabled; the swap *is* the reveal, moved to land exactly at gold + 240.

After the first unlock the PNGs persist on disk so they render normally on
subsequent visits.

**Click/keypress skip is blocked while the diamond reveal is pending.** Normally
clicking or pressing a key on the results screen fast-forwards through the medal
awards. To make sure you don't accidentally miss the diamond, the mod stops the
skip from engaging while the diamond time was met for the current race, the race
hasn't already been earned, and the reveal (gold + 240) hasn't played yet — the
reveal plays out at normal speed. The skip behaves exactly as normal when the
diamond was **not** achieved, and it is re-enabled after the reveal so you aren't
stuck watching the white hold. On replays of an already-earned race the skip
works normally (no lockout).

**Pausing is also blocked while the diamond reveal is pending (first earn).**
The game's own ESC-pause path is already suppressed at goal (it sets
an internal latch), but the two other pause entry points — right-clicking, and
the Win32 ESC key handler — would still open the pause menu and interrupt the
reveal. The mod intercepts both of those for the same window (diamond achieved
+ not yet earned + reveal not yet played), so you cannot pause away the diamond.
Pause works normally during gameplay, on already-earned replays, and after the
reveal.

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
It hooks:

1. **Golden-weasel draw (0x44E139)** — this cave (a) draws the **suction vortex**
   behind the trophy (raw `DrawPrimitiveUP` screen-space streaks), (b) drives the
   golden weasel's white-fade via the game's color-multiplier, and (c) at
   **gold + 240** swaps the trophy: the golden weasel stops rendering and the
   **diamond** is drawn in its place at (0x208, 0x63), firing the reveal effects
   (medal pop + star ring) on the first swap frame. All timing is offset from the
   gold-medal award frame (`results+0x4c`): before gold+55 the gold draws
   normally; after gold+240 the white-hold ends and it reverts to gold unless the
   diamond earned a swap.
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
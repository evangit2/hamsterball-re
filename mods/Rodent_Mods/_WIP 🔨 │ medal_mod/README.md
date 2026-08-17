# Diamond Weasel — 5th Secret Medal (bass.dll proxy mod)

Adds a **fifth, secret medal** per race to Hamsterball.

## What it does

When you finish a race faster than the secret threshold for that race:

### After any race, the results screen lays out the medals like this:

- **Golden weasel** (top-right) — fastest tier
- **Bronze, Silver, Gold** — a diagonal row of the earned standard medals
- **Diamond weasel** — the 5th secret medal, drawn **directly over the golden weasel** (same spot, top-right) when you beat the secret time

When you beat a race's secret time, the **golden weasel stays** and the
**diamond weasel** appears a moment later (240 frames after the **gold** medal
is awarded — ~8s, per the reveal timeline below) **directly over
the golden weasel** at the same location.

### Time-Trial menu (standings screen)

Each race's earned medal mini-icons are shown there. When you've earned the
diamond for a race, a **diamond mini-icon** appears **to the right of the
golden weasel** mini-icon for that race.
- The unlock is **persisted** per race (so it shows up on later visits).
- The standings mini-icons **only activate once you've earned at least one
  diamond**. On a fresh profile nothing is touched (no code cave installed),
  which is what keeps the menu from crashing. The **moment you earn your first
  diamond** the cave is installed on the spot, so the mini-icons show **in
  that same session** — no restart needed — for exactly the races you've
  unlocked. Diamond assets are written the moment they're first earned, so the
  mini-icon file only loads when it actually exists.

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

**Golden-weasel white-fade** (first earn only)

The unlock sequence — white-fade, vortex, and the dramatic reveal — plays **only
the first time you earn a race's diamond**. On an already-earned replay the
golden weasel renders normal gold and the diamond simply swaps in at gold + 240
with no buildup.

When it does play (first earn), it is **timed relative to the moment the gold
medal is awarded** — not from the moment you touch the goal. The game awards
gold when the results frame counter reaches the gold gate (`results+0x4c`, frame
400 for the race results object). The mod computes a *sequence frame* since that
instant (`results+0x10 − results+0x4c`), so the whole reveal plays off gold:

- **55 frames after gold** — the golden weasel starts turning white
- **150 frames after gold** — it is fully white (saturating additive blend)
- **240 frames after gold** — the diamond trophy reveal

The white-fade **whitens the golden-weasel sprite itself** from the award-screen
update host (`0x44D760`, vtable[1] — the only place that actually renders on the
award screen). The **consolidated procedural-composite** (see below) now does the
white-out *and* the vortex in one mechanism — no code cave, no present hook.

**Golden-weasel suction vortex + white-fade** (first earn only) — consolidated
into ONE procedurally-generated texture

Instead of drawing raw D3D primitives (which crashed `d3d8.dll` on real Windows
at frame ~57 from a mis-slot) or juggling device blend/material state, the mod
**renders the entire effect through the game's own sprite renderer**:

1. At the start of the reveal it **captures the golden weasel's real pixels**
   (a read `LockRect` of the weasel texture at `sprite+0x50`).
2. Every reveal frame it generates a **composite texture**: the weasel RGB
   **lerped toward white** by the reveal-frame t, plus the **suction streaks**
   painted into the annulus just *outside* the weasel disk (so the weasel stays
   1:1 and undistorted underneath them).
3. It uploads that composite via `CreateTexture` + `LockRect`-write into a
   scratch texture, binds it to a sprite `+0x50`, and **swaps that sprite into
   `ctx+0x37C`** — the same slot the game's own medal renderer already draws.
   The game's single `Sprite_DrawRect` then composites it.

Because **we control every pixel**, the white-out and the vortex happen together
in one texture and one draw call. Streaks fade in at random angles/radii, pull
straight inward (no swirl), cluster toward the center, and re-spawn over the
`VORTEX_FRAMES` active window; a ~30-frame tail lets remaining streaks finish
with no new spawns. Each streak fades out as its inner tip nears the trophy's
center so it never sticks out past it.

**Bigger canvas, same weasel spot:** the composite canvas is **2× the weasel's
texture** with the weasel drawn 1:1 centered in it, giving a wide annulus for
the streaks. To keep the **full-size weasel exactly where it was**, the sprite
box is scaled 2× (so the centered weasel maps to the original footprint) and
the medal-draw anchor (the two `push` immediates at `0x44E132`/`0x44E134`,
`0x63`/`0x208`) is shifted by `-origBox/2`. The anchor is a pure constant read
every frame — patched when the vortex swaps in and restored before the diamond
trophy swap, so neither the weasel nor the diamond moves.

The old white-out mechanisms (weasel material-diffuse ramp + device additive
blend) are **subsumed** — they would double-tint the texture, so they no longer
run during the reveal (restored defensively at +240 to avoid any leaked state).

A single one-shot **whoosh** (`sounds\\whoosh`, via the mod's real-BASS layer)
plays once when the vortex begins at the 55-frame mark — it is not looped — and
the stream is freed when the cycle ends. The composite sprite/texture/capture
are freed when the reveal finishes.

## How the diamond trophy renders (results screen)

The trophy swap works by **redirecting the game's own medal renderer**, not by
drawing on top of it. The golden-weasel medal is composited by the award-screen
**render** function (award vtable slot[2] = `0x44DF70`), at `0x44E12C`:

```asm
mov eax,[esi+0xC]      ; eax = display context
mov ecx,[eax+0x37C]    ; ctx+0x37C = the golden-weasel sprite slot
push 0x63; push 0x208; call 0x42c7c0   ; draw it at (0x208,0x63)
```

Earlier builds tried to draw the diamond from the award **update** host (vtable
slot[1] = `0x44D760`) by calling `Sprite_DrawRect` directly — but that host does
**not** composite to screen (the frame draws in the render function), so the
gold weasel rendered and the diamond never appeared.

**The fix (2026-08-16):** instead of drawing from the update host, the mod
swaps the sprite pointer at `ctx+0x37C` (ctx = `*(results+0xC)`) to the diamond
sprite from the safe update host. The game's own render function then draws the
diamond at the weasel spot. No heap cave is introduced in the render/SEH path —
the render function runs 100% original.

The swap is armed only while the reveal is active for the *current* results
object. When the results screen changes (or a run no longer qualifies), the mod
restores `ctx+0x37C` to the original golden-weasel sprite so the diamond never
leaks onto another screen or medal row.

## The reveal at gold + 240

At **240 frames after the gold medal is awarded** the golden weasel **stops
rendering entirely** and the **diamond trophy replaces it in that same frame**
(every time the diamond time is met — including replays). On the first earn the
swap fires the reveal effects (medal pop + star ring), and it is preceded by the
white-out + vortex buildup. On an already-earned replay the diamond swaps in
plainly (no white-out, no vortex, no pop) so the player can skip past it.

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
reveal. The mod blocks both for the same window (diamond achieved + not yet
earned + reveal not yet played), so you cannot pause away the diamond. Pause
works normally during gameplay, on already-earned replays, and after the
reveal.

**Pause-blocking rewrite (2026-08-14):** earlier versions hooked the *deep*
pause call sites (`0x4130c3` / `0x40b40f`) and routed the not-blocked path
back toward `0x40a920` (Scene_CreateGameOverMenu) through hand-rolled
heap-cave memory. On **real Windows** merely having mod bytes at those sites —
even a pure passthrough — crashed a normal pause (`CRASH_ADDRESS
0001:FFFFFFFF`, `MouseDown`, ~10s) because control flowed back into
`0x40a920`'s SEH frame from written memory. Wine tolerates this so it slipped
past crash tests.

The pause block now uses the **same proven approach as the level-warp mod
(option B)**: it patches the *single branch-decider byte* at each shallow
decision point *before* the pause function is reached, turning it into an
unconditional skip:
- right-click `0x4130B5` (`74`→`EB`), Win32 ESC `0x40B405` (`75`→`EB`),
  DI-ESC `0x419D5B` (`74`→`EB`).

No code cave, no jump-to-`0x40a920`, no heap memory in the pause path at all.
When blocked, the game **never reaches** `0x40a920`; when unblocked, the
original byte is restored verbatim so the game runs its own native pause path.
apply/remove are driven around the reveal (armed when a diamond is pending,
restored the instant it ends), so normal gameplay pause is 100% original —
exactly how level_warp's conditional blocker works.

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
It hooks only **results-screen-only** addresses — **none of them ever execute
during the boot loading screen**. This is the whole trick: the reveal runs
inline from the game's own medal-award draw, never from a per-frame
present/GameUpdate hook. That is why it no longer crashes at boot on real
Windows (the earlier present-hook versions did). It hooks:

1. **Golden-weasel reveal (full-wrapper SEH replacement of 0x44D760)** — the
   reveal does NOT hook `0x44E139` (interior of the SEH-protected award-DRAW
   `0x44DF70`), `0x44CBAA`/`0x44CC3F`/`0x44CB90` (the "click to continue"
   continuation, never fires per-frame), nor a present hook (boot crash), nor
   even a post-SEH patch. **Every prior host crashed real Windows with the
   same signature: heap EIP `C0000005` inside the award-update SEH frame.** The
   root cause is running mod heap code inside an SEH frame the GAME built —
   whether through a mid-frame redirect or a post-prologue patch. Fix: replace
   the **entire** award-update function `0x44D760` with a wrapper that:
   - re-implements the game's **exact** SEH prologue byte-for-byte (installs a
     byte-identical `FS:[0]` frame using the game's real scope table
     `0x4CC77C`, so exceptions route to the game's own handler),
   - runs `diamond_reveal_draw(results)` inside **our own** authentic frame,
   - then jumps into the game's untouched body at `0x44D77B` and lets its
     epilogue restore `FS:[0]` and return.
   Because the wrapper owns a frame identical to the game's, the exception
   chain is never patched mid-flight — nothing to corrupt. `diamond_reveal_draw`:
   - immediately checks `diamond_first_earn`; if not active it does
     **absolutely nothing** (no I/O, no D3D, no sound, no color-mult),
   - only a genuine first-earn reveal draws the **suction vortex**, fades the
     weasel to **white** (frames 55 → white at 150, hold to 240), then at
     gold+240 draws the **diamond** + fires the reveal effects (pop + star
     ring) and persists the unlock.
   All register state is preserved via `pushad`/`popad`.

   **CRASH-SAFE LOGGING (important):** the award-screen update runs inside a
   live SEH frame. Calling CRT file I/O (`fopen`/`fprintf`/`fflush`) inside
   that frame crashes real Windows with `C0000005` (Wine tolerates it) —
   this bit us when per-frame `diag_logf` calls were added. ALL frame-path
   tracing (`CAVE-FIRED`, `CAVE-ENTER`, `FIRST-EARN`, `weasel white`,
   `SWAP-GATE`) therefore writes to an **in-memory ring buffer** under a
   spinlock (`trace_logf`), and a dedicated background thread
   (`diamond_flusher_thread`) drains it to the log file. Nothing in the
   frame path touches the filesystem.
2. **TT-menu golden-weasel append (0x42F927)** — when the diamond is unlocked
   for a race, appends a diamond mini-icon entry to the standings medal list
   right after the golden weasel, so it lays out to the right of it. Also
   results-screen-only.
3. **Skip-latch (0x44CBAA)** — blocks the results/click "skip" when the
   diamond was achieved so the full reveal plays out (it keeps the skip latch
   from being set until frame 240). Results-screen-only. *(Re-enabled 2026-08-16:
   it had been dropped when the reveal host moved to the vtable override — the
   "countdown starts too early" bug was the click-to-continue fast-forwarding
   the frame counter 10x past the gold+240 reveal.)*
4. **Icon load (write-on-first-unlock)** — the diamond PNG bytes are embedded
   in the DLL as XOR-encrypted data (not extractable from the DLL). On the
   *first* time a diamond is awarded, the mod decrypts them and writes
   `diamondweasel.png` + `diamondweasel-icon.png` into `Textures\\`, then the
   game loads them through its normal file path. Before any unlock, no
   `diamond*.png` exists on disk.

All four hooks are installed from a **background init thread that first
`Sleep(2000)`ms** — the exact pattern proven by ghost_triggers/warp. Nothing
is patched in DllMain (VirtualAlloc/VirtualProtect under the Windows loader
lock crashes real Windows at RUNTIME 0-1s).

> **Why no arm/present hook?** Earlier builds added a "store-only arm" cave at
> `0x44D778` (inside the SEH-protected award update `0x44D760`) to flag the
> results screen, plus a per-frame reveal hook. Both caused real-Windows
> crashes: the arm cave patched **inside an SEH exception frame** (a `fs:0`
> SEH handler is installed right before `0x44D778`), so whenever the award
> screen raised any exception, unwinding through my redirected bytes
> corrupted the exception chain → heap-fault at the award vtable during
> `Board Update`. It also did nothing (it was a debug-only flag with no
> reader). The weasel-draw cave itself (`0x44E139`) is inside another SEH
> function (`0x44DF70`) but is byte-for-byte identical to the crash-free
> stable era, so it is safe — only the redundant, later-added arm cave broke.
> Removed it entirely; the reveal needs no arming signal because it reads
> `diamond_first_earn` directly from the results object on the award screen.

> **Why no present/per-frame hook at all?** The white-fade needs a draw on the
> award screen. Earlier builds drove it from a `Graphics_PresentOrEnd`
> (`0x455A90`) or `GameUpdate` frame-epilogue (`0x46C1F1`) hook, installed cold
> from DllMain. Two independent problems killed that approach, and it has been
> **removed from the mod entirely**:
>
> 1. **Boot crash.** Installing a JMP→heap redirect at boot fires during the
>    LoadingScreen (RUNTIME 00:00:01, `fonts\\\\showcardgothic28`, `CRASH_ADDRESS
>    0001:0000284F`, primary EIP=heap C0000005). Wine tolerates it; real Windows
>    does not. No amount of gate-on-globals or gate-on-flag in the tick changes
>    that — the redirect itself executing during boot is the vector.
> 2. **The award screen doesn't run the GameUpdate epilogue.** The medal-award
>    screen is a modal object driven by its own vtable[1] update (`0x44D760`)
>    and draw (`0x44DF70`); the GameUpdate frame epilogue where `0x46C1F1` lives
>    is **not** on that screen's loop. A present-hook overlay therefore could
>    never draw during the award — the `0x455A90`/`0x46C1F1` tick simply never
>    fired there (verified: zero `present-tick FIRES` for a full reveal).
>
> The white-fade instead runs from the **award-update host that actually
> renders** (`diamond_weasel_mult` + `diamond_set_add`, both called from the
> `0x44D760` vtable[1] override) — a safe, already-proven host, no code cave, no
> boot-resident redirect. This is strictly better: it removes the boot crash
> vector permanently AND actually draws on the screen that matters.

The unlock flag is persisted as the `DiamondMedals` registry value (15 bytes,
one flag per race) in `HKCU\Software\Raptisoft\Hamsterball`, written at the
same moment the PNG assets are created.

## Build

**IMPORTANT:** the results-screen trophy and the TT-menu mini icon are two
separate mechanisms, enabled by separate `-D` flags. Both must be present or
one of them silently disappears:

- `-DDIAMOND_TT_WRAPPER` → the TT-menu diamond mini-icons (clones
  `TimeTrialMenu_ctor` 0x42F810 with a 5th in-flow append).
- `-DDIAMOND_VTABLE_OVERRIDE` → the results-screen diamond trophy reveal
  (patches award vtable slot[1]=0x4D6CF4) + the render-path sprite swap into
  `ctx+0x37C`.
- *(optional)* `-DVORTEX_OFF` → compiles the procedural-composite vortex out
  (white-fade + diamond swap stay enabled, but no vortex/white-lerp). The
  consolidated vortex is safe (it draws through the game's own sprite
  renderer, not raw D3D), so the normal build omits it.

Use both `TT_WRAPPER` + `VTABLE_OVERRIDE` together — building only one removes
the corresponding feature.

```
i686-w64-mingw32-gcc -shared -o bass.dll diamond_weasel_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse \
  -DDIAMOND_TT_WRAPPER -DDIAMOND_VTABLE_OVERRIDE
```

## Credit

Built for RodentRacer. Reverse-engineered from `Hamsterball.exe` (medal
award + results display + sprite loader).
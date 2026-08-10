# Medal Time Mod (bass.dll proxy)

Overrides the per-race medal threshold times (WEASEL / GOLD / SILVER / BRONZE)
that Hamsterball loads from `Data/RaceData.xml` at startup.

This lets you make the secret **golden weasel** medal reachable (or any medal's
par time harder/easier) per race, without editing the game's data file.

## What it does

The game reads one medal-par block per race from `RaceData.xml` into memory
(App+0x2990..0x29A8) when a race board is constructed. The Time-Trial results
screen awards a medal when your finish time is **at or below** the threshold:

- **Golden Weasel** — `time <= WEASEL`
- **Gold** — `time <= GOLD`
- **Silver** — `time <= SILVER`
- **Bronze** — `time <= BRONZE`

This mod hooks the parser after it runs and overwrites the four thresholds for
the current race with the values from `medal_config.txt`. Everything downstream
(the award logic, the in-race target time, the medal icons) reads the same
memory slots, so the overrides take effect automatically.

## Installation

1. Close Hamsterball.
2. Put `bass.dll` in the game folder (where `Hamsterball.exe` lives).
3. Put `medal_config.txt` in the same folder.
4. Launch the game.

> The mod forwards all BASS audio calls to the game's real BASS. If you are
> also running another bass.dll proxy mod, only one `bass.dll` can be active
> at a time — keep a backup of the one you currently use.

## Configuration

Edit `medal_config.txt`. It has one section per race, in tournament order:

```
[WARMUP]       ; race 0  — has no medal block, ignored
[BEGINNER]     ; race 1
[INTERMEDIATE] ; race 2
[DIZZY]        ; race 3
[TOWER]        ; race 4
[UP]           ; race 5
[NEON]         ; race 6
[EXPERT]       ; race 7
[ODD]          ; race 8
[TOOB]         ; race 9
[WOBBLY]       ; race 10
[GLASS]        ; race 11
[SKY]          ; race 12
[MASTER]       ; race 13
[IMPOSSIBLE]   ; race 14
```

Within a section, set any of:

```
WEASEL=6.6
GOLD=7.6
SILVER=10.3
BRONZE=15.0
```

Values are in **seconds**. A section you omit (or a field you leave out) keeps
the game's original value for that race. Lines starting with `#` or `;` are
ignored.

### Example: make the Golden Weasel easy on every race

```
[WARMUP]
[BEGINNER]
WEASEL=60.0
[INTERMEDIATE]
WEASEL=60.0
...
```

## Logging

The mod writes `medal_mod_log.txt` next to `bass.dll`. It records:

- whether the config loaded and the hooks installed, and
- the race name captured when the parser runs, plus a confirmation that
  overrides were applied.

If you don't see `parser called for race: X` in the log after starting a race,
the mediator parser wasn't reached (e.g. another bass.dll proxy is active).

## Build (for developers)

```
i686-w64-mingw32-gcc -shared -o bass.dll medal_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

## Source layout

- `medal_mod.c` — the mod (single file).
- `bass.dll` — built proxy.
- `medal_config.txt` — config template with the game's original values
  commented out for reference.
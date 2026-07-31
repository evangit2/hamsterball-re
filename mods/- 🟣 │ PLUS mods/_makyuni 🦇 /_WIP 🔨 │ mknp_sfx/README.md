# Custom SFX + SFX Path (HB+ Mod)

Replace any sound effect in Hamsterball with custom audio files, and redirect all sounds to a per-level subfolder.

## Installation

1. Place `mkn_plus_sfx.dll` in the `Mods\` folder (HB+ mod loader directory)
2. Launch the game — `custom_sfx.txt` will be auto-generated next to the DLL

## Features

### 1. Individual Sound Replacement

Edit `custom_sfx.txt` to replace individual sounds. Change the filename after `=`:

```
; Default:
collide = collide

; Replace with your sound:
collide = my_collision
```

Your file must be `.ogg` or `.wav` format, placed in the `Sounds\` folder:
- `Sounds\my_collision.ogg` (preferred)
- `Sounds\my_collision.wav` (fallback)

The game tries `.ogg` first, then `.wav`.

### 2. Per-Level SFX Path (NEW)

Add `<SFX_PATH>` tags to `RaceData.XML` to redirect all sounds to a subfolder for a specific level:

```xml
<GLASSRACE>
    <TIME>25</TIME>
    <PAR>10</PAR>
    <SFX_PATH>Newsfx</SFX_PATH>
    <WEASEL>36.0</WEASEL>
    <GOLD>43.5</GOLD>
    <SILVER>52.1</SILVER>
    <BRONZE>65.0</BRONZE>
    <CAM>0.71</CAM>
</GLASSRACE>
```

When Glass Race starts, ALL sounds will load from `Sounds\Newsfx\` instead of `Sounds\`. Place your custom sound files (same names as originals) in `Sounds\Newsfx\`:

```
Sounds\Newsfx\Collide.ogg
Sounds\Newsfx\Roll.ogg
Sounds\Newsfx\Bumper.ogg
... (all 61 sounds)
```

If no `<SFX_PATH>` tag exists in a level's XML block, that level loads sounds normally from `Sounds\`.

### Supported RaceData.XML Tags

| Tag | Level | Scene Name |
|-----|-------|------------|
| CASCADERACE | Warm-Up Race | Board (Warm-Up) |
| BEGINNERRACE | Beginner Race | Board (Beginner) |
| INTERMEDIATERACE | Intermediate Race | Board (Intermediate) |
| DIZZYRACE | Dizzy Race | Board (Dizzy) |
| TOWERRACE | Tower Race | Board (Tower) |
| UPRACE | Up Race | Board (Up) |
| NEONRACE | Neon Race | Board (Dark) |
| EXPERTRACE | Expert Race | Board (Expert) |
| ODDRACE | Odd Race | Board (Odd) |
| TOOBRACE | Toob Race | Board (Toob) |
| WOBBLYRACE | Wobbly Race | Board (Wobbly) |
| GLASSRACE | Glass Race | Board (Glass) |
| SKYRACE | Sky Race | Board (Sky) |
| MASTERRACE | Master Race | Board (Master) |
| IMPOSSIBLERACE | Impossible Race | Board (Impossible) |

Arena tags (WARMUPARENA, BEGINNERARENA, etc.) are also supported.

### Priority

1. Individual sound replacement (from `custom_sfx.txt`) takes priority over SFX_PATH
2. If both are set, the individual replacement is used
3. SFX_PATH applies to all sounds that don't have an individual replacement

## How It Works

The mod hooks `Sound_LoadOggOrWav` (0x459660) — the function that builds the file path for every sound effect. When the game tries to load `sounds\collide.ogg`, the hook checks:

1. First, `custom_sfx.txt` for an individual replacement name
2. If none, checks if `g_activeSfxPath` is set (from RaceData.XML)
3. If SFX_PATH is active, redirects to `sounds\<subfolder>\collide.ogg`

On level start (`onLevelStart`), the mod detects the current level via HB+ API `GetScene()` and checks RaceData.XML for a matching `<SFX_PATH>`. If found, it reloads all 61 sounds from the new subfolder. On scene end, sounds are restored to the default folder.

## Sound System Architecture

- **SFX**: DirectSound (DSOUND.dll) — 61 sounds, loaded via `Sound_LoadOggOrWav`
- **Music**: BASS.dll — plays .mod/.xm/.it/.mo3 files (not affected by this mod)
- **SoundDevice**: Created at `App+0x178`, master volume at `+0x838`
- **Sound Channels**: 61 slots at `App+0x43C` through `App+0x52C` (4-byte stride)

## File List

| File | Description |
|------|-------------|
| `mkn_plus_sfx.dll` | Compiled HB+ mod DLL |
| `custom_sfx.txt` | Auto-generated config (individual sound replacements) |
| `source/mknp_sfx_MinGW.cpp` | Source code (MinGW) |
| `source/nocrt.h` | Minimal CRT replacement header |
| `source/nocrt.cpp` | CRT implementation (no msvcrt) |
| `source/hbplus_api.h` | HB+ IModAPI vtable wrapper |
| `source/HamsterballAPI.h` | HB+ API definitions |
| `source/build.sh` | Cross-compilation build script |

## All 61 Sounds

| # | Name | Buffers | Entity/Event |
|---|------|---------|-------------|
| 1 | collide | 10 | Ball-wall collision, Bumper hit |
| 2 | roll | 10 | Ball rolling on surfaces |
| 3 | whistle | 1 | Race start whistle, menu select |
| 4 | bumper | 10 | Bumper collision (Warm-Up, Beginner, Arena) |
| 5 | ballbreak | 5 | Ball shatter (full break) |
| 6 | ballbreaksmall | 5 | Ball partial break |
| 7 | thwomp | 2 | Bonkbash slam, heavy impact |
| 8 | snap | 2 | BreakBridge snap |
| 9 | popup | 2 | Bonk popup, UI popup |
| 10 | dropin | 2 | Ball drop-in at race start |
| 11 | dropinshort | 2 | Short drop-in |
| 12 | popout | 2 | Ball pop out (pipe exit) |
| 13 | pipebump1 | 10 | Pipe bump variant 1 |
| 14 | pipebump2 | 10 | Pipe bump variant 2 |
| 15 | pipebump3 | 10 | Pipe bump variant 3 |
| 16 | gearclank | 20 | Gear clank (Rotator collision) |
| 17 | bridgeslam | 2 | Bridge slam, Catapult launch |
| 18 | platformtick | 5 | Platform tick (moving platform) |
| 19 | gluestuck | 5 | Gluebie stuck sound |
| 20 | bubble1 | 5 | Tar bubble 1 |
| 21 | bubble2 | 5 | Tar bubble 2 |
| 22 | wheelcreak | 2 | Wheel creak (Spinner) |
| 23 | catapult | 2 | Catapult wind-up |
| 24 | trapdoor | 2 | Trapdoor activate, Rotator start |
| 25 | fwing | 2 | E:JUMP event, fan wing flap |
| 26 | clink | 3 | Breaker extend, metal clink |
| 27 | whoosh | 3 | Whoosh (air movement) |
| 28 | chomp | 1 | Chomp (Mousetrap) |
| 29 | fan-start | 10 | Fan startup |
| 30 | fan-blow | 10 | Fan blowing (continuous) |
| 31 | crack | 2 | Glass crack (Glass Race) |
| 32 | crumble | 2 | BreakBridge crumble |
| 33 | sawstartup | 2 | Saw startup (alert sound) |
| 34 | sawcut | 2 | Saw cutting (contact) |
| 35 | minipop | 5 | Mini pop (small collision) |
| 36 | bell | 3 | Bell ring (Bonus Bell) |
| 37 | zip | 2 | Zip (speed boost) |
| 38 | ting | 20 | Ting (Rotator gear tick) |
| 39 | shrink | 3 | Ball shrink (power-down) |
| 40 | grow | 3 | Ball grow (power-up) |
| 41 | tweet | 3 | Tweet (bird/secret unlock) |
| 42 | creakyplatform | 20 | Creaky platform (Tower) |
| 43 | wubba | 5 | Wubba (Wobbly Race platform) |
| 44 | saw | 2 | Saw idle hum |
| 45 | sawspeedy | 2 | Saw speedy (fast cutting) |
| 46 | dawgstep1 | 10 | Blockdawg footstep 1 |
| 47 | dawgstep2 | 10 | Blockdawg footstep 2 |
| 48 | dawgsmash | 10 | Blockdawg smash (fall impact) |
| 49 | sizzle | 2 | Sizzle (Neon Race) |
| 50 | explode | 3 | Explode (Impossible Race) |
| 51 | vac-o-sux | 3 | Vac-O-Sux vacuum suction |
| 52 | speedcylinder | 2 | Speed Cylinder boost, Pendulum hit |
| 53 | bonuspop | 5 | Bonus pop (arena score) |
| 54 | buzzbonus | 1 | Buzz bonus (Rotator trigger) |
| 55 | breakbridge | 1 | Break bridge collapse |
| 56 | unlock | 1 | Level/feature unlock |
| 57 | NeonRide | 1 | Neon Ride (Neon Race enter) |
| 58 | NeonFlicker | 50 | Neon Flicker (Neon Race ambient) |
| 59 | ZoopDown | 2 | Zoop Down (Glass Race) |
| 60 | LightsOff | 2 | Lights Off (Glass Race) |
| 61 | GlassBonus | 2 | Glass Bonus (Glass Race) |

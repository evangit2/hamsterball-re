# Custom SFX Mod

Replace any sound effect in Hamsterball with custom audio files.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` to the game folder
3. Launch the game — `custom_sfx.txt` will be auto-generated

## Usage

Edit `custom_sfx.txt` to replace sounds. Change the filename after `=`:

```
; Default:
collide = collide

; Replace with your sound:
collide = my_collision
```

Your file must be `.ogg` or `.wav` format, placed in the `sounds\` folder:
- `sounds\my_collision.ogg` (preferred)
- `sounds\my_collision.wav` (fallback)

The game tries `.ogg` first, then `.wav`.

### Supported Formats

| Format | Supported | Notes |
|--------|-----------|-------|
| OGG (Vorbis) | ✅ | Preferred — better compression |
| WAV (PCM) | ✅ | Fallback if .ogg missing |

### Properties Explained

- **Buffers**: Max concurrent playback instances. Higher = more overlapping (e.g., `collide` = 10 allows 10 simultaneous collision sounds).
- **Volume**: Dynamic, calculated from 3D distance between ball and sound source. Range: 0.0 (silent) to 1.0 (full). Master volume at `SoundDevice+0x838`.
- **Pitch**: Not per-sound. Uses DirectSound frequency (default sample rate). Can be modified at runtime via DSound `SetFrequency`.
- **Loop**: All SFX are one-shot. No looping sound effects in Hamsterball.
- **3D Attenuation**: Min distance = 0.0, Max distance = 6000.0. Sounds beyond max distance are inaudible.

## How It Works

The mod hooks `Sound_LoadOggOrWav` (0x459660) — the function that builds the file path for every sound effect. When the game tries to load `sounds\collide.ogg`, the hook checks `custom_sfx.txt` for a replacement name, and if found, redirects to `sounds\<replacement>.ogg` instead.

The hook is installed at DLL load time (before the game initializes), so all 61 sounds are intercepted during `TimerDisplay` (0x4298C0) scene initialization.

## Sound System Architecture

- **SFX**: DirectSound (DSOUND.dll) — 61 sounds, loaded via `Sound_LoadOggOrWav`
- **Music**: BASS.dll — plays .mod/.xm/.it/.mo3 files (not affected by this mod)
- **SoundDevice**: Created at `App+0x178`, master volume at `+0x838`
- **Sound Channels**: 61 slots at `Board+0x43C` through `Board+0x52C` (4-byte stride)
- **Buffer Clones**: Each sound has N DSound buffer clones for overlapping playback

## File List

| File | Description |
|------|-------------|
| `custom_sfx.c` | Source code |
| `bass.dll` | Compiled mod DLL |
| `custom_sfx.txt` | Auto-generated config (created on first run) |

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

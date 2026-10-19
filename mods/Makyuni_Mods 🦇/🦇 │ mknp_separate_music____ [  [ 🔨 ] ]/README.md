# Separate Music Mod (mknp_separate_music, HB+)

One .mo3 file per song instead of all songs in `Music.mo3`.
Map any game song to its own file, e.g. `Fight! = Level8.mo3`.

## Installation

1. Place `mknp_separate_music.dll` in the `Mods\` folder
2. Launch a race — `mknp_separate_music.txt` is auto-made next to the DLL
3. Put your .mo3 files in the game's `Music\` folder

> HB+ mod. Do NOT rename to `bass.dll`. Game root `bass.dll`
> must stay the HB+ proxy.

## Usage

Edit `mknp_separate_music.txt`:

```
Fight! = Level8.mo3
```

Empty value = play original `Music.mo3` (safe).
Missing file = original plays, note in `separate_music.log`.
Edits apply live (no restart).

| Key        | Value    | Result          |
|------------|----------|-----------------|
| Track name | file.mo3 | plays that file |
| Track name | (empty)  | plays original  |

## Defaults

| Race       | Song          | File        |
|------------|---------------|-------------|
| Warm-Up    | Cascade Race  | Level1.mo3  |
| Beginner   | Hamster Nation| Level2.mo3  |
| Intermed.  | Gerbil Groove | Level3.mo3  |
| Dizzy      | Dizzy!        | Level4.mo3  |
| Tower      | Happy Rush    | Level5.mo3  |
| Up         | Up Race       | Level6.mo3  |
| Dark       | Neon Theme    | Level7.mo3  |
| Expert     | Fight!        | Level8.mo3  |
| Odd        | Ninja Hamster | Level9.mo3  |
| Toob       | Rodenthood    | Level10.mo3 |
| Wobbly     | Hamster Chase | Level11.mo3 |
| Glass      | Glass Theme   | Level12.mo3 |
| Sky        | Bucky Break   | Level13.mo3 |
| Master     | Master Theme  | Level14.mo3 |
| Impossible | Imposs. Theme | Level15.mo3 |

Menus default to original.

## Files

| File                    | Note               |
|-------------------------|--------------------|
| mknp_separate_music.dll | into Mods\ as-is   |
| mknp_separate_music.txt | auto-made, edit me |
| separate_music.log      | auto-made debug log|
| README.md               | this file          |

## How it works

`Music.mo3` holds all songs as orders. `Audio_PlayMusic`
(0x46A310) plays one by name (`Audio_PlayMusicAtSpeed`
0x46A440 calls it internally). This mod hooks that function:
mapped name + file present -> load file via BASS, play it,
skip original. Else original runs. Entry bytes verified
before patching, never blind.

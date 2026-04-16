# Hamsterball Audio Renders

Pre-rendered songs from `Music.mo3` (23 subsongs as WAV files).

## Regenerate

```bash
# Install openmpt123:
#   apt install libopenmpt0  (or build from source)

MO3_PATH=../../originals/installed/extracted/Music/Music.mo3
OUT_DIR=.

for idx in $(seq 0 22); do
  SUBSONG=$(printf "%02d" $idx)
  openmpt123 --batch \
    -o "${OUT_DIR}/${SUBSONG}_song.wav" \
    "$MO3_PATH" --subsong $idx
done
```

## Songs

| Subsong | Name |
|---------|------|
| 0  | 00_main_theme.wav |
| 1  | 01_main_theme_no_intro.wav |
| 2  | 02_hamster_nation.wav |
| 3  | 03_hamster_nation_no_intro.wav |
| 4  | 04_cascade_race.wav |
| 5  | 05_cascade_race_no_intro.wav |
| 6  | 06_gerbil_groove.wav |
| 7  | 07_gerbil_groove_no_intro.wav |
| 8  | 08_dizzy.wav |
| 9  | 09_dizzy_no_intro.wav |
| 10 | 10_happy_rush.wav |
| 11 | 11_happy_rush_no_intro.wav |
| 12 | 12_up_race.wav |
| 13 | 13_up_race_no_intro.wav |
| 14 | 14_neon_theme.wav |
| 15 | 15_fight.wav |
| 16 | 16_fight_no_intro.wav |
| 17 | 17_ninja_hamster.wav |
| 18 | 18_ninja_hamster_no_intro.wav |
| 19 | 19_rodenthood.wav |
| 20 | 20_rodenthood_no_intro.wav |
| 21 | 21_hamster_chase.wav |
| 22 | 22_hamster_chase_no_intro.wav |

Total: 23 subsongs, ~18:47 duration.

## Original Audio Files

- Music: `originals/installed/extracted/Music/Music.mo3` (23 subsongs, IT/MO3 tracker)
- Sounds: `originals/installed/extracted/Sounds/` (63 OGG files)

#!/usr/bin/env python3
"""Render all 23 subsongs from Music.mo3 to WAV files using openmpt123."""
import subprocess, os, sys

MO3_PATH = os.path.join(os.path.dirname(__file__), "..", "..", "originals", "installed", "extracted", "Music", "Music.mo3")
OUT_DIR  = os.path.join(os.path.dirname(__file__), "..", "songs")

SONGS = [
    (0,  "00_main_theme.wav"),
    (1,  "01_main_theme_no_intro.wav"),
    (2,  "02_hamster_nation.wav"),
    (3,  "03_hamster_nation_no_intro.wav"),
    (4,  "04_cascade_race.wav"),
    (5,  "05_cascade_race_no_intro.wav"),
    (6,  "06_gerbil_groove.wav"),
    (7,  "07_gerbil_groove_no_intro.wav"),
    (8,  "08_dizzy.wav"),
    (9,  "09_dizzy_no_intro.wav"),
    (10, "10_happy_rush.wav"),
    (11, "11_happy_rush_no_intro.wav"),
    (12, "12_up_race.wav"),
    (13, "13_up_race_no_intro.wav"),
    (14, "14_neon_theme.wav"),
    (15, "15_fight.wav"),
    (16, "16_fight_no_intro.wav"),
    (17, "17_ninja_hamster.wav"),
    (18, "18_ninja_hamster_no_intro.wav"),
    (19, "19_rodenthood.wav"),
    (20, "20_rodenthood_no_intro.wav"),
    (21, "21_hamster_chase.wav"),
    (22, "22_hamster_chase_no_intro.wav"),
]

os.makedirs(OUT_DIR, exist_ok=True)

for idx, fname in SONGS:
    out_path = os.path.join(OUT_DIR, fname)
    if os.path.exists(out_path):
        print(f"SKIP {fname} (exists)")
        continue
    print(f"Rendering subsong {idx}: {fname}")
    rc = subprocess.call([
        "openmpt123", "--batch", "-o", out_path, MO3_PATH, "--subsong", str(idx)
    ])
    if rc == 0 and os.path.exists(out_path):
        sz = os.path.getsize(out_path) / 1024 / 1024
        print(f"  OK: {sz:.1f} MB")
    else:
        print(f"  FAILED (rc={rc})")

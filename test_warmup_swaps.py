#!/usr/bin/env python3
"""
Warm-Up slot swap test for universal-ref-loader v3.
Swaps each race level into Level1.MESHWORLD, starts the game, 
navigates to Warm-Up race, screenshots, reads the ref_loader_log.
"""

import subprocess
import os
import time
import shutil
import hashlib

GAME_DIR = os.path.expanduser("~/hamsterball-re/originals/installed/extracted")
LEVELS_DIR = os.path.join(GAME_DIR, "Levels")
BACKUP = os.path.join(LEVELS_DIR, "Level1.MESHWORLD.ORIGINAL_BACKUP")
LOG_FILE = os.path.join(GAME_DIR, "ref_loader_log.txt")
SCREENSHOT_DIR = os.path.expanduser("~/hamsterball-re/test_screenshots")

os.makedirs(SCREENSHOT_DIR, exist_ok=True)

# All 14 race levels (excluding Level1=Warm-Up itself)
LEVELS = [
    ("Level2",          "Beginner"),
    ("Level3",          "Intermediate"),
    ("LevelCascade",   "Dizzy"),
    ("Level4",          "Tower"),
    ("LevelUp",         "Up"),
    ("LevelDark",       "Neon"),
    ("Level5",          "Expert"),
    ("Level9",          "Odd"),
    ("Level8",          "Toob"),
    ("Level7",          "Wobbly"),
    ("LevelGlass",      "Glass"),
    ("Level10",         "Sky"),
    ("LevelImpossible", "Impossible"),
    # Also include Master arena? No, these are race levels only.
]

def run(cmd, timeout=30):
    """Run a command and return (exit_code, stdout, stderr)."""
    try:
        r = subprocess.run(cmd, shell=True, timeout=timeout, 
                          capture_output=True, text=True)
        return r.returncode, r.stdout.strip(), r.stderr.strip()
    except subprocess.TimeoutExpired:
        return -1, "", "TIMEOUT"

def kill_game():
    run("pkill -9 wine", 5)
    run("pkill -9 wineserver", 5)
    time.sleep(2)

def start_game():
    """Start the game on Xvfb display :99."""
    env = os.environ.copy()
    env["DISPLAY"] = ":99"
    env["LIBGL_ALWAYS_SOFTWARE"] = "1"
    proc = subprocess.Popen(
        ["wine", "Hamsterball.exe"],
        cwd=GAME_DIR,
        env=env,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    return proc

def is_game_alive():
    r = run("pgrep -x wine", 5)
    return r[0] == 0 and len(r[1]) > 0

def navigate_to_warmup():
    """Navigate to Warm-Up race: Enter (title), Enter (menu), Enter (select first race)."""
    # Title screen: wait for load
    time.sleep(12)
    # Press Enter to advance title
    run("DISPLAY=:99 xte 'key Return'", 5)
    time.sleep(2)
    run("DISPLAY=:99 xte 'key Return'", 5)
    time.sleep(2)
    # Down once to get to Time Trial? No - just select the first race
    run("DISPLAY=:99 xte 'key Return'", 5)
    time.sleep(3)
    # Select Warm-Up (first option)
    run("DISPLAY=:99 xte 'key Return'", 5)
    time.sleep(3)
    # Confirm
    run("DISPLAY=:99 xte 'key Return'", 5)
    time.sleep(5)

def take_screenshot(name):
    """Take a screenshot using xwd + ffmpeg."""
    screenshot_path = os.path.join(SCREENSHOT_DIR, f"{name}.png")
    run(f"DISPLAY=:99 xwd -root -silent | ffmpeg -y -i - -f image2 {screenshot_path} 2>/dev/null", 10)
    return screenshot_path

def read_log():
    """Read the ref_loader_log.txt file."""
    try:
        with open(LOG_FILE, "r", errors="replace") as f:
            return f.read()
    except FileNotFoundError:
        return "(no log file)"

def delete_cached():
    """Delete all .cached files to force re-reading MESHWORLD."""
    for f in os.listdir(LEVELS_DIR):
        if f.endswith(".cached"):
            os.remove(os.path.join(LEVELS_DIR, f))

def swap_level(level_filename):
    """Copy a level's MESHWORLD into Level1.MESHWORLD."""
    src = os.path.join(LEVELS_DIR, f"{level_filename}.MESHWORLD")
    dst = os.path.join(LEVELS_DIR, "Level1.MESHWORLD")
    shutil.copy2(src, dst)

def restore_level1():
    """Restore original Level1.MESHWORLD."""
    shutil.copy2(BACKUP, os.path.join(LEVELS_DIR, "Level1.MESHWORLD"))

# Main test loop
results = []

for i, (level_file, level_name) in enumerate(LEVELS):
    print(f"\n{'='*60}")
    print(f"Test {i+1}/{len(LEVELS)}: {level_name} ({level_file}.MESHWORLD → Level1)")
    print(f"{'='*60}")
    
    # Kill previous game
    kill_game()
    
    # Clear log
    try:
        os.remove(LOG_FILE)
    except FileNotFoundError:
        pass
    
    # Swap level
    swap_level(level_file)
    delete_cached()
    
    # Start game
    proc = start_game()
    
    # Navigate to Warm-Up
    navigate_to_warmup()
    
    # Check if alive
    alive = is_game_alive()
    print(f"Game alive: {alive}")
    
    # Take screenshot
    screenshot_path = take_screenshot(f"{level_file}_in_WarmUp")
    if os.path.exists(screenshot_path):
        size = os.path.getsize(screenshot_path)
        print(f"Screenshot: {screenshot_path} ({size} bytes)")
    else:
        print("Screenshot: FAILED")
        screenshot_path = None
    
    # Read log
    log = read_log()
    print(f"Log contents:")
    print(log)
    
    # Parse log for ref results
    ref_lines = [l for l in log.split('\n') if l.startswith('REFLOAD')]
    ok_orig = [l for l in ref_lines if 'OK_ORIG' in l]
    ok_jit = [l for l in ref_lines if 'OK_JIT' in l]
    failed = [l for l in ref_lines if 'FAIL' in l]
    
    result = {
        "level": level_name,
        "file": level_file,
        "alive": alive,
        "ok_orig": len(ok_orig),
        "ok_jit": len(ok_jit),
        "failed": len(failed),
        "failed_refs": [l.split('\t')[1] if len(l.split('\t')) > 1 else l for l in failed],
        "screenshot": screenshot_path,
        "log": log,
    }
    results.append(result)
    
    # Kill game
    kill_game()

# Restore original Level1
restore_level1()
delete_cached()

# Print summary
print(f"\n{'='*60}")
print("SUMMARY")
print(f"{'='*60}")
print(f"{'Level':<15} {'Alive':<6} {'OKOrig':<7} {'OKJIT':<6} {'Fail':<5} Failed refs")
print("-" * 80)
for r in results:
    print(f"{r['level']:<15} {'YES' if r['alive'] else 'NO':<6} {r['ok_orig']:<7} {r['ok_jit']:<6} {r['failed']:<5} {', '.join(r['failed_refs'])}")

print(f"\nScreenshots in: {SCREENSHOT_DIR}")

#!/usr/bin/env python3
"""Navigate the original Hamsterball game menu to start a tournament race.

Uses xdotool keyboard input to navigate through:
  Title → Space → LET'S PLAY → Return → CHOOSE A GAME → Tournament
  → Return → CONTINUE? → Right+Return(NO) → Difficulty → Return → PLAY!

Usage:
  python3 navigate_to_race.py [--window-id WID] [--delay SEC] [--start-delay SEC]
  python3 navigate_to_race.py --full-auto   # Wait 25s for game load, then navigate

The game must already be running on the Xvfb display (:99 by default).
"""
import argparse
import os
import subprocess
import sys
import time


def run_xdotool(cmd_args, window_id=None):
    """Run an xdotool command with the correct DISPLAY."""
    env = os.environ.copy()
    env["DISPLAY"] = os.environ.get("HBTESTD_DISPLAY", ":99")
    cmd = ["xdotool"]
    if window_id:
        # For key commands, use --window
        if cmd_args[0] in ("key", "keydown", "keyup"):
            cmd += ["key", "--window", window_id, "--delay", "200"] + cmd_args[1:]
        elif cmd_args[0] == "click":
            cmd += cmd_args
        elif cmd_args[0] == "mousemove":
            cmd += cmd_args
        else:
            cmd += cmd_args
    else:
        cmd += cmd_args
    return subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=10)


def find_hamsterball_window():
    """Find the window ID for the Hamsterball game."""
    env = os.environ.copy()
    env["DISPLAY"] = os.environ.get("HBTESTD_DISPLAY", ":99")
    # Try Wine desktop first
    for search_term in ["Wine desktop", "Hamsterball", "hamsterball"]:
        result = subprocess.run(
            ["xdotool", "search", "--name", search_term],
            env=env, capture_output=True, text=True, timeout=5
        )
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout.strip().splitlines()[0]
    return ""


def take_screenshot(output_path="/tmp/hb_screenshot.png"):
    """Take a screenshot of the Xvfb display."""
    env = os.environ.copy()
    display = env.get("HBTESTD_DISPLAY", ":99")
    subprocess.run(
        ["ffmpeg", "-f", "x11grab", "-video_size", "800x600",
         "-i", display, "-frames:v", "1", output_path, "-y"],
        env=env, capture_output=True, timeout=10
    )
    return output_path


def navigate_to_race(window_id, delay=1.5):
    """Navigate through the menu to start a tournament race.

    Verified navigation sequence for Hamsterball v1.5.2:
    1. Space: Title screen → Main menu (LET'S PLAY, HIGH SCORES, etc.)
    2. Return: Select LET'S PLAY → CHOOSE A GAME! menu
    3. Return: Select TOURNAMENT → CONTINUE TOURNAMENT? dialog
    4. Right+Return: Select NO → CHOOSE A DIFFICULTY! menu
    5. Down+Return: Select NORMAL → Level intro (NEXT UP: WARM-UP RACE)
    6. Return: Select PLAY! → Gameplay starts!
    """
    steps = [
        # Step 1: Get past title screen
        ("space", "Title screen → Main menu"),
        (None, 2.0),  # Wait for animation

        # Step 2: Select LET'S PLAY! (first/top option)
        ("Return", "Select LET'S PLAY! → CHOOSE A GAME"),
        (None, 2.0),

        # Step 3: TOURNAMENT is highlighted by default
        ("Return", "Select TOURNAMENT → Continue tournament dialog"),
        (None, 2.0),

        # Step 4: Select NO (don't continue saved tournament)
        ("Right", "Navigate to NO button"),
        (None, 0.5),
        ("Return", "Select NO → Difficulty select"),
        (None, 2.0),

        # Step 5: Select difficulty - PIPSQUEAK is default, Down to NORMAL
        ("Down", "Navigate to NORMAL difficulty"),
        (None, 0.5),
        ("Return", "Select NORMAL → Level intro screen"),
        (None, 2.0),

        # Step 6: Start the race
        ("Return", "Select PLAY! → Start race"),
    ]

    for step in steps:
        if step[0] is None:
            # It's a wait step
            wait_time = step[1]
            print(f"  Waiting {wait_time:.1f}s...")
            time.sleep(wait_time)
        else:
            key, description = step
            run_xdotool(["key", key], window_id)
            print(f"  Pressed {key}: {description}")
            time.sleep(delay)

    print("\nNavigation complete! The race should be starting.")
    return True


def main():
    parser = argparse.ArgumentParser(
        description="Navigate Hamsterball menu to start a race"
    )
    parser.add_argument(
        "--window-id", default="",
        help="X11 window ID (auto-detected if omitted)"
    )
    parser.add_argument(
        "--delay", type=float, default=1.5,
        help="Delay between key presses (seconds, default: 1.5)"
    )
    parser.add_argument(
        "--start-delay", type=float, default=0,
        help="Initial delay before starting navigation (seconds)"
    )
    parser.add_argument(
        "--full-auto", action="store_true",
        help="Wait 25s for game to load, then navigate automatically"
    )
    parser.add_argument(
        "--screenshot", default="",
        help="Take a screenshot after navigation (specify output path)"
    )
    args = parser.parse_args()

    if args.full_auto:
        print("Full auto mode: waiting 25s for game to load...")
        time.sleep(25)

    wid = args.window_id or find_hamsterball_window()
    if not wid:
        print("ERROR: Could not find Hamsterball window. Is the game running?",
              file=sys.stderr)
        sys.exit(1)

    print(f"Using window ID: {wid}")
    print(f"Delay between keys: {args.delay}s")

    if args.start_delay > 0:
        print(f"Waiting {args.start_delay}s before starting...")
        time.sleep(args.start_delay)

    navigate_to_race(wid, delay=args.delay)

    # Wait a moment for the race to load
    time.sleep(5)

    if args.screenshot:
        path = take_screenshot(args.screenshot)
        print(f"Screenshot saved to: {path}")


if __name__ == "__main__":
    main()

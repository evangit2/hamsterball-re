#!/usr/bin/env python3
"""Navigate the original Hamsterball game menu to start a tournament race.

Uses xdotool keyboard input to navigate through:
  Title → LET'S PLAY → TOURNAMENT → NO (new tournament) → PLAY!

Usage:
  python3 navigate_to_race.py [--window-id WID] [--delay SECONDS]

The game must already be running on the Xvfb display.
"""
import argparse
import os
import subprocess
import sys
import time


def xdotool(window_id: str, *args: str) -> subprocess.CompletedProcess:
    env = os.environ.copy()
    env["DISPLAY"] = os.environ.get("HBTESTD_DISPLAY", ":99")
    cmd = ["xdotool"]
    if window_id:
        cmd += ["key", "--window", window_id] + list(args)
    else:
        cmd += ["key"] + list(args)
    return subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=10)


def find_hamsterball_window() -> str:
    """Find the Wine desktop window ID for Hamsterball."""
    env = os.environ.copy()
    env["DISPLAY"] = os.environ.get("HBTESTD_DISPLAY", ":99")
    result = subprocess.run(
        ["xdotool", "search", "--name", "Wine desktop"],
        env=env, capture_output=True, text=True, timeout=5
    )
    if result.returncode == 0 and result.stdout.strip():
        return result.stdout.strip().splitlines()[0]
    # Fallback: try Hamsterball window directly
    result = subprocess.run(
        ["xdotool", "search", "--name", "Hamsterball"],
        env=env, capture_output=True, text=True, timeout=5
    )
    if result.returncode == 0 and result.stdout.strip():
        return result.stdout.strip().splitlines()[0]
    return ""


def main():
    parser = argparse.ArgumentParser(description="Navigate Hamsterball menu to start a race")
    parser.add_argument("--window-id", default="", help="X11 window ID (auto-detected if omitted)")
    parser.add_argument("--delay", type=float, default=0.5, help="Delay between key presses (seconds)")
    parser.add_argument("--start-delay", type=float, default=0, help="Initial delay before starting navigation")
    args = parser.parse_args()

    wid = args.window_id or find_hamsterball_window()
    if not wid:
        print("ERROR: Could not find Hamsterball window. Is the game running?", file=sys.stderr)
        sys.exit(1)

    print(f"Using window ID: {wid}")
    print(f"Delay between keys: {args.delay}s")

    if args.start_delay > 0:
        print(f"Waiting {args.start_delay}s before starting...")
        time.sleep(args.start_delay)

    # Navigate: Title screen → Main menu → LET'S PLAY → CHOOSE A GAME → TOURNAMENT
    # → CONTINUE TOURNAMENT? → NO → WARM-UP RACE → PLAY!
    steps = [
        ("Return", "Title → Main menu"),
        ("space", "LET'S PLAY → CHOOSE A GAME"),
        ("space", "TOURNAMENT selected → CONTINUE TOURNAMENT? dialog"),
        ("Right", "Navigate to NO"),
        ("space", "Select NO → Tournament bracket"),
        ("space", "PLAY! → Start race"),
    ]

    for key, description in steps:
        xdotool(wid, key)
        print(f"  Pressed {key}: {description}")
        time.sleep(args.delay)

    print("\nDone! The race should be loading now.")


if __name__ == "__main__":
    main()

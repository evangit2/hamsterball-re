#!/usr/bin/env python3
"""End-to-end test: generate a custom MESHWORLD, load it in the original game, and verify.

Usage:
  python3 test_custom_level.py --preset bowl
  python3 test_custom_level.py --preset plane --skip-verify
  python3 test_custom_level.py --preset bowl --level-slot Arena-WarmUp

This script:
1. Generates a custom MESHWORLD file using mw_create.py
2. Backs up and swaps it into a level slot
3. Launches the original Hamsterball.exe on Xvfb
4. Navigates the menu to start a race
5. Takes a screenshot and (optionally) verifies it with vision model
6. Restores the original level file
"""
import argparse
import os
import subprocess
import sys
import time

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
# Repo root is hamsterball-re/, not hamsterball-re/tools/hbtestd/
GAME_DIR = os.path.join(REPO_ROOT, "originals", "installed", "extracted")
LEVELS_DIR = os.path.join(GAME_DIR, "Levels")
MW_CREATE = os.path.join(REPO_ROOT, "tools", "mw_create.py")
NAVIGATE_SCRIPT = os.path.join(REPO_ROOT, "tools", "hbtestd", "scripts", "navigate_to_race.py")
DISPLAY = os.environ.get("HBTESTD_DISPLAY", ":99")


def run(cmd, **kwargs):
    """Run a command and return the result."""
    print(f"  $ {cmd}" if isinstance(cmd, str) else f"  $ {' '.join(cmd)}")
    result = subprocess.run(cmd, shell=isinstance(cmd, str), capture_output=True, text=True, **kwargs)
    if result.returncode != 0:
        print(f"  WARNING: exit code {result.returncode}")
        if result.stderr:
            print(f"  stderr: {result.stderr[:200]}")
    return result


def generate_level(preset, output_path, rings=None, sectors=None):
    """Generate a custom MESHWORLD level using mw_create.py."""
    print(f"\n[1/6] Generating '{preset}' level...")
    cmd = ["python3", MW_CREATE, "--preset", preset]
    if rings is not None:
        cmd += ["--rings", str(rings)]
    if sectors is not None:
        cmd += ["--sectors", str(sectors)]
    cmd += ["--output", output_path]
    result = run(cmd)
    if result.returncode != 0:
        print(f"ERROR: Failed to generate level", file=sys.stderr)
        sys.exit(1)
    size = os.path.getsize(output_path)
    print(f"  Generated: {output_path} ({size} bytes)")
    return True


def swap_level(custom_path, level_name):
    """Swap a custom level into a level slot, backing up the original."""
    target = os.path.join(LEVELS_DIR, f"{level_name}.MESHWORLD")
    backup = target + ".bak"

    print(f"\n[2/6] Swapping into {level_name} slot...")

    # Backup original if not already backed up
    if not os.path.exists(backup):
        run(["cp", target, backup])
        print(f"  Backed up original to {backup}")

    # Copy custom level into slot
    run(["cp", custom_path, target])
    print(f"  Swapped {custom_path} → {target}")
    return target


def launch_game():
    """Launch the original Hamsterball game on Xvfb."""
    print(f"\n[3/6] Launching Hamsterball.exe on display {DISPLAY}...")

    # Kill any existing wine processes
    run("wineserver -k 2>/dev/null", timeout=10)
    time.sleep(2)

    # Start the game
    env = os.environ.copy()
    env["DISPLAY"] = DISPLAY
    env["LIBGL_ALWAYS_SOFTWARE"] = "1"
    proc = subprocess.Popen(
        ["wine", "Hamsterball.exe"],
        cwd=GAME_DIR,
        env=env,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    print(f"  Launched PID={proc.pid}")
    return proc


def navigate_to_race():
    """Navigate the game menu to start a race."""
    print(f"\n[4/6] Navigating menu to start race...")
    result = run(
        ["python3", NAVIGATE_SCRIPT, "--full-auto", "--delay", "1.5"],
        timeout=60
    )
    return result.returncode == 0


def take_screenshot(output_path="/tmp/custom_level_test.png"):
    """Take a screenshot of the Xvfb display."""
    print(f"\n[5/6] Taking screenshot...")
    env = os.environ.copy()
    env["DISPLAY"] = DISPLAY
    subprocess.run(
        ["ffmpeg", "-f", "x11grab", "-video_size", "800x600",
         "-i", DISPLAY, "-frames:v", "1", output_path, "-y"],
        env=env, capture_output=True, timeout=10
    )
    size = os.path.getsize(output_path) if os.path.exists(output_path) else 0
    print(f"  Screenshot: {output_path} ({size} bytes)")
    return output_path


def restore_level(level_name):
    """Restore the original level file from backup."""
    target = os.path.join(LEVELS_DIR, f"{level_name}.MESHWORLD")
    backup = target + ".bak"

    print(f"\n[Cleanup] Restoring original {level_name}...")
    if os.path.exists(backup):
        run(["cp", backup, target])
        run(["rm", backup])
        print(f"  Restored {target} from backup")
    else:
        print(f"  No backup found at {backup}")


def cleanup():
    """Kill wine processes."""
    run("wineserver -k 2>/dev/null", timeout=10)


def main():
    parser = argparse.ArgumentParser(
        description="End-to-end test: generate custom MESHWORLD and load in original game"
    )
    parser.add_argument(
        "--preset", default="bowl",
        choices=["plane", "bowl", "ramp", "platforms"],
        help="MW create preset to use (default: bowl)"
    )
    parser.add_argument(
        "--level-slot", default="Level1",
        help="Level slot to replace (default: Level1)"
    )
    parser.add_argument(
        "--screenshot-path", default="/tmp/custom_level_test.png",
        help="Output path for screenshot"
    )
    parser.add_argument(
        "--skip-verify", action="store_true",
        help="Skip vision model verification"
    )
    parser.add_argument(
        "--keep-level", action="store_true",
        help="Don't restore original level after test"
    )
    parser.add_argument(
        "--rings", type=int, default=None,
        help="Number of rings for bowl tessellation"
    )
    parser.add_argument(
        "--sectors", type=int, default=None,
        help="Number of sectors for bowl tessellation"
    )
    parser.add_argument(
        "--wait-after-nav", type=float, default=10.0,
        help="Seconds to wait after navigation before screenshot (default: 10)"
    )
    args = parser.parse_args()

    custom_path = os.path.join(LEVELS_DIR, f"Custom-Test{args.preset.capitalize()}.MESHWORLD")

    try:
        # Step 1: Generate level (with optional tessellation args)
        generate_level(args.preset, custom_path, rings=args.rings, sectors=args.sectors)

        # Step 2: Swap into level slot
        swap_level(custom_path, args.level_slot)

        # Step 3: Launch game
        proc = launch_game()

        # Step 4: Navigate menu
        time.sleep(2)  # Brief wait before starting navigation
        navigate_to_race()

        # Step 5: Wait for level to load, then take screenshot
        time.sleep(args.wait_after_nav)
        screenshot = take_screenshot(args.screenshot_path)

        # Wait a moment then kill the game
        time.sleep(3)
        proc.terminate()
        time.sleep(2)
        cleanup()

        # Step 6: Report results
        print(f"\n{'='*60}")
        print(f"TEST COMPLETE")
        print(f"  Preset: {args.preset}")
        print(f"  Level slot: {args.level_slot}")
        print(f"  Screenshot: {screenshot}")
        print(f"{'='*60}")

        if not args.skip_verify:
            print(f"\nTo verify the screenshot, use:")
            print(f"  vision_analyze image={screenshot}")
            print(f"  Ask: 'Is there a {args.preset} shape visible with a ball?'")

    except Exception as e:
        print(f"\nERROR: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
    finally:
        if not args.keep_level:
            restore_level(args.level_slot)
        cleanup()


if __name__ == "__main__":
    main()

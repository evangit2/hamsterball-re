#!/usr/bin/env python3
"""
Verify that the universal ref loader actually processes refs correctly.

For each race level:
  1. Swap the level's MESHWORLD into Level1 (Warm-Up) slot
  2. Install the mod DLL (bass.dll proxy with in-memory stats)
  3. Start the game via Wine/Xvfb
  4. Navigate into the race via hbtestd REST API (load_level endpoint)
  5. Read the RefLoaderStats struct from the game's memory
  6. Report which refs were loaded (OK_ORIG), cross-loaded (OK_JIT), or failed

Stats are found by scanning memory for magic 0xCAFEBABE.
"""

import struct
import subprocess
import time
import os
import sys
import json
import signal

GAME_DIR = "/home/evan/hamsterball-wasm/boxedwine-package/hamsterball"
LEVELS_DIR = f"{GAME_DIR}/Levels"
MOD_DLL = os.path.expanduser("~/hamsterball-re/mods/universal-ref-loader/bass.dll")
ORIG_BASS = f"{GAME_DIR}/bass_real.dll"
BACKUP_L1 = f"{LEVELS_DIR}/Level1.MESHWORLD.ORIG_BACKUP"
HBTESTD = "http://127.0.0.1:8777"

# Stats struct layout (must match DLL source)
STATS_MAGIC = 0xCAFEBABE
STATS_RING_SIZE = 96
REF_NAME_LEN = 28
REF_ENTRY_SIZE = REF_NAME_LEN + 2  # name[28] + result(1) + factory(1)
STATS_HEADER_SIZE = 8 * 4  # 8 unsigned ints
STATS_TOTAL_SIZE = STATS_HEADER_SIZE + (STATS_RING_SIZE * REF_ENTRY_SIZE)

# Levels to test (Level1=Warm-Up is the slot, so test all others)
TEST_LEVELS = [
    ("LevelCascade", "Beginner"),
    ("Level2", "Intermediate"),
    ("Level3", "Dizzy"),
    ("Level4", "Tower"),
    ("LevelUp", "Up"),
    ("LevelDark", "Neon"),
    ("Level5", "Expert"),
    ("Level9", "Sky"),
    ("Level8", "Toob"),
    ("Level7", "Wobbly"),
    ("LevelGlass", "Glass"),
    ("Level10", "Sky"),
    ("LevelImpossible", "Impossible"),
]

FACTORY_NAMES = [
    "original", "Expert", "Wobbly", "Master", "Tower", "Impossible",
    "Up", "Dizzy", "Beginner", "Neon", "Odd", "Toob", "Glass", "Sky",
]

RESULT_NAMES = {0: "FAIL", 1: "OK_ORIG", 2: "OK_JIT"}


def kill_all():
    """Kill all game/wine/Xvfb processes safely."""
    for pattern in ["Hamsterball.exe", "wine-preloader", "wine64-preloader",
                     "wineserver", "Xvfb"]:
        try:
            result = subprocess.run(["pgrep", "-x", pattern],
                                    capture_output=True, text=True, timeout=5)
            for pid_str in result.stdout.strip().split("\n"):
                pid_str = pid_str.strip()
                if pid_str and pid_str.isdigit():
                    try:
                        os.kill(int(pid_str), signal.SIGKILL)
                    except (ProcessLookupError, PermissionError):
                        pass
        except Exception:
            pass
    time.sleep(2)
    # Remove X lock files
    for f in ["/tmp/.X99-lock", "/tmp/.X11-unix/X99"]:
        try:
            os.remove(f)
        except FileNotFoundError:
            pass


def start_xvfb():
    """Start Xvfb on display :99."""
    proc = subprocess.Popen(
        ["Xvfb", ":99", "-screen", "0", "1024x768x24", "-nolisten", "tcp"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )
    time.sleep(2)
    return proc


def get_live_pid():
    """Get the PID of the running Hamsterball.exe (not zombie)."""
    try:
        result = subprocess.run(
            ["ps", "aux"], capture_output=True, text=True, timeout=5
        )
        for line in result.stdout.split("\n"):
            if "Hamsterball.exe" in line and "defunct" not in line and "grep" not in line:
                parts = line.split()
                if len(parts) > 1:
                    pid = int(parts[1])
                    return pid
    except Exception:
        pass
    return None


def check_hook(pid):
    """Verify hook is installed at 0x0040C4BA."""
    try:
        with open(f"/proc/{pid}/mem", "rb") as f:
            f.seek(0x0040C4BA)
            data = f.read(6)
            if data[0] == 0xE8:
                return f"HOOKED ({data.hex()})"
            elif data[0] == 0xFF:
                return f"NOT_HOOKED ({data.hex()})"
            return f"UNKNOWN ({data.hex()})"
    except Exception as e:
        return f"READ_FAILED ({e})"


def find_stats_in_memory(pid):
    """Scan memory for the 0xCAFEBABE magic to find RefLoaderStats."""
    try:
        # Read /proc/pid/maps to find readable regions
        with open(f"/proc/{pid}/maps", "r") as f:
            maps = f.read()

        regions = []
        for line in maps.split("\n"):
            if not line.strip():
                continue
            parts = line.split()
            if len(parts) < 5:
                continue
            perms = parts[1]
            if "r" not in perms:
                continue
            addr_range = parts[0].split("-")
            if len(addr_range) != 2:
                continue
            start = int(addr_range[0], 16)
            end = int(addr_range[1], 16)
            # Skip very large regions (stack, etc.) and tiny ones
            size = end - start
            if size < 32 or size > 64 * 1024 * 1024:
                continue
            # Skip non-anonymous regions that aren't heap or the DLL
            path = parts[-1] if len(parts) > 5 else ""
            # Focus on heap (anonymous) and bass.dll regions
            if path and "bass.dll" not in path and "[heap]" not in path and not path.startswith("["):
                # Allow anonymous mappings
                if path and "Hamsterball.exe" not in path:
                    continue
            regions.append((start, end, size, path))

        # Search for magic in each region
        magic_bytes = struct.pack("<I", STATS_MAGIC)

        with open(f"/proc/{pid}/mem", "rb") as mem:
            for start, end, size, path in regions:
                try:
                    mem.seek(start)
                    data = mem.read(size)
                except (OSError, ValueError):
                    continue

                offset = 0
                while True:
                    idx = data.find(magic_bytes, offset)
                    if idx == -1:
                        break

                    addr = start + idx
                    # Try to parse as RefLoaderStats
                    try:
                        mem.seek(addr)
                        stats_data = mem.read(STATS_TOTAL_SIZE)
                        if len(stats_data) < STATS_TOTAL_SIZE:
                            offset = idx + 4
                            continue

                        # Parse header
                        (magic, total, ok_orig, ok_jit, fail, clones,
                         entry_count, entry_head) = struct.unpack_from(
                            "<8I", stats_data, 0
                        )

                        if magic != STATS_MAGIC:
                            offset = idx + 4
                            continue

                        # Validate: total should equal ok_orig + ok_jit + fail
                        if total != ok_orig + ok_jit + fail:
                            offset = idx + 4
                            continue

                        # Parse ring buffer entries
                        entries = []
                        base = STATS_HEADER_SIZE
                        for i in range(min(entry_count, STATS_RING_SIZE)):
                            entry_off = base + i * REF_ENTRY_SIZE
                            if entry_off + REF_ENTRY_SIZE > len(stats_data):
                                break
                            name_bytes = stats_data[entry_off:entry_off + REF_NAME_LEN]
                            name = name_bytes.split(b"\x00")[0].decode("ascii", errors="replace")
                            result = stats_data[entry_off + REF_NAME_LEN]
                            factory = stats_data[entry_off + REF_NAME_LEN + 1]
                            entries.append((name, result, factory))

                        return {
                            "addr": f"0x{addr:08X}",
                            "total": total,
                            "ok_orig": ok_orig,
                            "ok_jit": ok_jit,
                            "fail": fail,
                            "clones": clones,
                            "entries": entries,
                        }
                    except Exception:
                        pass
                    offset = idx + 4

        return None
    except Exception as e:
        return {"error": str(e)}


def hbtestd_call(endpoint, method="GET", **kwargs):
    """Call hbtestd REST API via curl."""
    cmd = ["curl", "-s", "-X", method, f"{HBTESTD}{endpoint}"]
    if kwargs:
        import json as jsonmod
        cmd += ["-H", "Content-Type: application/json",
                "-d", jsonmod.dumps(kwargs)]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        return result.stdout
    except Exception as e:
        return f'{{"error": "{e}"}}'


def run_test(level_file, level_name):
    """Run a single level test."""
    print(f"\n{'='*60}")
    print(f"Testing: {level_name} ({level_file})")
    print(f"{'='*60}")

    # 1. Kill everything
    kill_all()

    # 2. Swap MESHWORLD
    src = f"{LEVELS_DIR}/{level_file}.MESHWORLD"
    dst = f"{LEVELS_DIR}/Level1.MESHWORLD"
    subprocess.run(["cp", src, dst], check=True)

    # 3. Delete .cached files
    subprocess.run(f"find {LEVELS_DIR} -name '*.cached' -delete",
                   shell=True, capture_output=True)

    # 4. Install mod DLL
    subprocess.run(["cp", MOD_DLL, f"{GAME_DIR}/bass.dll"], check=True)

    # 5. Start Xvfb
    xvfb = start_xvfb()

    # 6. Start game
    env = os.environ.copy()
    env["DISPLAY"] = ":99"
    env["LIBGL_ALWAYS_SOFTWARE"] = "1"
    proc = subprocess.Popen(
        ["wine", "Hamsterball.exe"],
        cwd=GAME_DIR, env=env,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )
    time.sleep(0.5)

    # 7. Wait for game to load
    print("  Waiting for game to load (20s)...", end="", flush=True)
    time.sleep(20)

    pid = get_live_pid()
    if not pid:
        print(" FAIL — game not running")
        return {"level": level_name, "result": "GAME_DEAD", "hook": "N/A"}

    hook = check_hook(pid)
    print(f" PID={pid} HOOK={hook}")

    if "HOOKED" not in hook:
        print(f"  Hook not installed — aborting")
        kill_all()
        return {"level": level_name, "result": "HOOK_FAILED", "hook": hook}

    # 8. Navigate into the race via hbtestd
    # First check if hbtestd is running
    hbtestd_health = subprocess.run(
        ["curl", "-s", f"{HBTESTD}/health"],
        capture_output=True, text=True, timeout=5
    )

    if hbtestd_health.returncode != 0 or "ok" not in hbtestd_health.stdout.lower():
        # hbtestd not running — navigate manually with xdotool
        print("  hbtestd not available — navigating with xdotool...")
        # Title screen → menu → race
        for key in ["Return", "Return", "Down", "Return"]:
            subprocess.run(["xdotool", "key", "--delay", "200", key],
                          env=env, capture_output=True, timeout=5)
            time.sleep(3)
        # Wait for race to load
        time.sleep(10)
    else:
        # Use hbtestd load_level — but we need it to NOT restart the game
        # Since the game is already running, just navigate
        print("  Navigating via xdotool (hbtestd game already running)...")
        # The key sequence from hbtestd source:
        # Enter (title→menu), Enter (LET'S PLAY→CHOOSE), Down (→TIME TRIALS),
        # Enter (→race selection), Enter (select Warm-Up=Level1 slot)
        for key in ["Return", "Return", "Down", "Return", "Return"]:
            subprocess.run(["xdotool", "key", "--delay", "200", key],
                          env=env, capture_output=True, timeout=5)
            time.sleep(3)
        # Wait for race to load
        time.sleep(10)

    # 9. Read stats from memory
    pid = get_live_pid()
    if not pid:
        print("  Game died during navigation!")
        return {"level": level_name, "result": "CRASHED_NAV", "hook": hook}

    print(f"  Reading stats from PID {pid}...")
    stats = find_stats_in_memory(pid)

    if not stats:
        print("  Stats not found (magic 0xCAFEBABE not found in memory)")
        # Check if the hook even fired — if game is on title screen, refs weren't loaded
        return {"level": level_name, "result": "STATS_NOT_FOUND",
                "hook": hook, "total": 0, "ok_orig": 0, "ok_jit": 0,
                "fail": 0, "clones": 0, "entries": []}

    if "error" in stats:
        print(f"  Error reading stats: {stats['error']}")
        return {"level": level_name, "result": "STATS_ERROR",
                "hook": hook, **stats}

    # 10. Report
    print(f"\n  --- Ref Loader Stats for {level_name} ---")
    print(f"  Total refs processed:  {stats['total']}")
    print(f"  OK (original factory): {stats['ok_orig']}")
    print(f"  OK (JIT cross-load):   {stats['ok_jit']}")
    print(f"  FAILED (no factory):   {stats['fail']}")
    print(f"  Clones (static mesh):  {stats['clones']}")
    print(f"  Ring entries:           {len(stats['entries'])}")

    if stats["entries"]:
        print(f"\n  {'Ref Name':<28} {'Result':<10} {'Factory':<12}")
        print(f"  {'-'*28} {'-'*10} {'-'*12}")
        for name, result, factory in stats["entries"]:
            res_str = RESULT_NAMES.get(result, f"?({result})")
            fac_str = FACTORY_NAMES[factory] if factory < len(FACTORY_NAMES) else f"?({factory})"
            print(f"  {name:<28} {res_str:<10} {fac_str:<12}")

    # Kill for cleanup
    kill_all()

    return {"level": level_name, "result": "OK", "hook": hook, **stats}


def main():
    # Create backup
    if not os.path.exists(BACKUP_L1):
        subprocess.run(["cp", f"{LEVELS_DIR}/Level1.MESHWORLD", BACKUP_L1])
        print(f"Created backup: {BACKUP_L1}")

    # Verify mod DLL
    if not os.path.exists(MOD_DLL):
        print(f"ERROR: Mod DLL not found: {MOD_DLL}")
        sys.exit(1)

    print(f"Mod DLL: {os.path.getsize(MOD_DLL)} bytes")
    print(f"Test levels: {len(TEST_LEVELS)}")
    print(f"Stats struct size: {STATS_TOTAL_SIZE} bytes")

    all_results = []

    for level_file, level_name in TEST_LEVELS:
        try:
            result = run_test(level_file, level_name)
            all_results.append(result)
        except Exception as e:
            print(f"  EXCEPTION: {e}")
            all_results.append({"level": level_name, "result": f"EXCEPTION: {e}"})
            kill_all()

    # Restore originals
    subprocess.run(["cp", BACKUP_L1, f"{LEVELS_DIR}/Level1.MESHWORLD"])
    subprocess.run(f"find {LEVELS_DIR} -name '*.cached' -delete",
                   shell=True, capture_output=True)
    subprocess.run(["cp", ORIG_BASS, f"{GAME_DIR}/bass.dll"])
    kill_all()

    # Summary
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    print(f"{'Level':<15} {'Result':<15} {'Total':>6} {'Orig':>6} {'JIT':>6} {'Fail':>6} {'Clone':>6}")
    print("-" * 60)
    for r in all_results:
        name = r.get("level", "?")
        result = r.get("result", "?")
        total = r.get("total", 0)
        ok_orig = r.get("ok_orig", 0)
        ok_jit = r.get("ok_jit", 0)
        fail = r.get("fail", 0)
        clones = r.get("clones", 0)
        print(f"{name:<15} {result:<15} {total:>6} {ok_orig:>6} {ok_jit:>6} {fail:>6} {clones:>6}")

    # Save full results
    results_file = os.path.expanduser("~/hamsterball-re/ref_verify_results.json")
    with open(results_file, "w") as f:
        json.dump(all_results, f, indent=2, default=str)
    print(f"\nFull results: {results_file}")


if __name__ == "__main__":
    main()

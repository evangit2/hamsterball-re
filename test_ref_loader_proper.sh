#!/bin/bash
# Proper crash-test: restart game between EVERY level swap.
# Installs mod DLL, swaps MESHWORLD, starts game fresh via Wine/Xvfb,
# waits 35s, checks alive + hook installed, kills everything, repeats.

set -e

GAME_DIR="/home/evan/hamsterball-wasm/boxedwine-package/hamsterball"
LEVELS_DIR="$GAME_DIR/Levels"
MOD_DLL="$HOME/hamsterball-re/mods/universal-ref-loader/bass.dll"
ORIG_BASS="$GAME_DIR/bass_real.dll"
BACKUP_L1="$LEVELS_DIR/Level1.MESHWORLD.ORIG_BACKUP"
RESULTS="/tmp/ref_loader_proper_results.txt"

# Levels to test (excluding Level1=Warm-Up itself)
TEST_LEVELS=(
    "LevelCascade:Beginner"
    "Level2:Intermediate"
    "Level3:Dizzy"
    "Level4:Tower"
    "LevelUp:Up"
    "LevelDark:Neon"
    "Level5:Expert"
    "Level9:Odd"
    "Level8:Toob"
    "Level7:Wobbly"
    "LevelGlass:Glass"
    "Level10:Sky"
    "LevelImpossible:Impossible"
)

# Kill ALL wine/game processes and clean up
kill_all() {
    # Kill Hamsterball.exe processes (use exact match to avoid killing our own shell)
    for pid in $(pgrep -x "Hamsterball.exe" 2>/dev/null); do
        kill -9 "$pid" 2>/dev/null || true
    done
    # Also catch wine-preloader and wine64-preloader
    for pid in $(pgrep -x "wine-preloader" 2>/dev/null); do
        kill -9 "$pid" 2>/dev/null || true
    done
    for pid in $(pgrep -x "wine64-preloader" 2>/dev/null); do
        kill -9 "$pid" 2>/dev/null || true
    done
    sleep 1
    # Kill wineserver
    for pid in $(pgrep -x "wineserver" 2>/dev/null); do
        kill -9 "$pid" 2>/dev/null || true
    done
    sleep 1
    # Kill Xvfb
    for pid in $(pgrep -x "Xvfb" 2>/dev/null); do
        kill -9 "$pid" 2>/dev/null || true
    done
    sleep 2
    # Remove X lock files
    rm -f /tmp/.X99-lock /tmp/.X11-unix/X99 2>/dev/null || true
    # Reap any zombie processes
    wait 2>/dev/null || true
}

# Start Xvfb fresh
start_xvfb() {
    Xvfb :99 -screen 0 1024x768x24 -nolisten tcp &
    sleep 2
    # Verify it's running
    if ! pgrep -f "Xvfb :99" > /dev/null 2>&1; then
        echo "  ERROR: Xvfb failed to start"
        return 1
    fi
}

# Check if hook is installed by reading 6 bytes at 0x0040C4BA
check_hook() {
    local pid=$1
    if [ -z "$pid" ]; then
        echo "NO_PID"
        return
    fi
    # Read 6 bytes at the hook address
    local bytes=$(dd if=/proc/$pid/mem bs=1 skip=$((0x0040C4BA)) count=6 2>/dev/null | xxd -p 2>/dev/null)
    if [[ "$bytes" == "e8"* ]]; then
        echo "HOOKED ($bytes)"
    elif [ -n "$bytes" ]; then
        echo "NOT_HOOKED ($bytes)"
    else
        echo "READ_FAILED"
    fi
}

# Verify mod DLL is actually installed (not overwritten by hbtestd)
verify_dll() {
    local mod_md5=$(md5sum "$MOD_DLL" 2>/dev/null | awk '{print $1}')
    local installed_md5=$(md5sum "$GAME_DIR/bass.dll" 2>/dev/null | awk '{print $1}')
    if [ "$mod_md5" != "$installed_md5" ]; then
        echo "  WARNING: bass.dll mismatch! mod=$mod_md5 installed=$installed_md5 — reinstalling"
        cp "$MOD_DLL" "$GAME_DIR/bass.dll"
        return 1
    fi
    return 0
}

# Create backup of original Level1 if not exists
if [ ! -f "$BACKUP_L1" ]; then
    cp "$LEVELS_DIR/Level1.MESHWORLD" "$BACKUP_L1"
    echo "Created backup of original Level1.MESHWORLD"
fi

# Verify mod DLL exists
if [ ! -f "$MOD_DLL" ]; then
    echo "ERROR: Mod DLL not found at $MOD_DLL"
    exit 1
fi

echo "=== Universal Ref Loader v3 — Proper Restart Test ===" | tee "$RESULTS"
echo "Date: $(date)" | tee -a "$RESULTS"
echo "Mod DLL: $(md5sum "$MOD_DLL" | awk '{print $1}')" | tee -a "$RESULTS"
echo "Original bass_real.dll: $(md5sum "$ORIG_BASS" | awk '{print $1}')" | tee -a "$RESULTS"
echo "" | tee -a "$RESULTS"

PASS_COUNT=0
FAIL_COUNT=0

for entry in "${TEST_LEVELS[@]}"; do
    LEVEL_FILE="${entry%%:*}"
    LEVEL_NAME="${entry##*:}"

    echo "=== Testing: $LEVEL_NAME ($LEVEL_FILE) ===" | tee -a "$RESULTS"

    # Step 1: Kill everything
    kill_all

    # Step 2: Swap MESHWORLD file
    cp "$LEVELS_DIR/$LEVEL_FILE.MESHWORLD" "$LEVELS_DIR/Level1.MESHWORLD"

    # Step 3: Delete all .cached files
    find "$LEVELS_DIR" -name "*.cached" -delete 2>/dev/null || true

    # Step 4: Install mod DLL (overwrite bass.dll with our proxy)
    cp "$MOD_DLL" "$GAME_DIR/bass.dll"

    # Step 5: Start Xvfb
    start_xvfb || continue

    # Step 6: Start the game with Wine (disown so shell signal doesn't propagate)
    cd "$GAME_DIR"
    DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 wine Hamsterball.exe >/dev/null 2>&1 &
    WINE_PID=$!
    disown $WINE_PID 2>/dev/null || true

    # Step 7: Wait 20s for game to load
    sleep 20

    # Step 8: Check if game process is alive (filter out zombies/defunct)
    GAME_PID=$(ps aux | grep -i "Hamsterball.exe" | grep -v grep | grep -v defunct | grep -v zombie | awk '{print $2}' | head -1)

    if [ -n "$GAME_PID" ]; then
        ALIVE_20="YES"
        HOOK_STATUS=$(check_hook "$GAME_PID")
        DLL_OK=$(verify_dll 2>&1)
        [ -n "$DLL_OK" ] && echo "  $DLL_OK"
    else
        ALIVE_20="NO"
        HOOK_STATUS="N/A"
    fi

    # Step 9: Wait 15 more seconds (35s total crash test)
    sleep 15

    # Step 10: Final alive check (filter out zombies)
    GAME_PID2=$(ps aux | grep -i "Hamsterball.exe" | grep -v grep | grep -v defunct | grep -v zombie | awk '{print $2}' | head -1)
    if [ -n "$GAME_PID2" ]; then
        ALIVE_35="YES"
        FINAL_HOOK=$(check_hook "$GAME_PID2")
    else
        ALIVE_35="NO"
        FINAL_HOOK="N/A"
    fi

    # Determine result
    if [ "$ALIVE_35" = "YES" ]; then
        RESULT="PASS"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        RESULT="FAIL (crash)"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi

    echo "  20s: PID=$GAME_PID ALIVE=$ALIVE_20 HOOK=$HOOK_STATUS" | tee -a "$RESULTS"
    echo "  35s: PID=$GAME_PID2 ALIVE=$ALIVE_35 HOOK=$FINAL_HOOK" | tee -a "$RESULTS"
    echo "  Result: $RESULT" | tee -a "$RESULTS"
    echo "" | tee -a "$RESULTS"

    # Kill for next iteration
    kill_all
done

# Restore original files
cp "$BACKUP_L1" "$LEVELS_DIR/Level1.MESHWORLD"
find "$LEVELS_DIR" -name "*.cached" -delete 2>/dev/null || true
cp "$ORIG_BASS" "$GAME_DIR/bass.dll"

echo "=== SUMMARY ===" | tee -a "$RESULTS"
echo "PASS: $PASS_COUNT / $((PASS_COUNT + FAIL_COUNT))" | tee -a "$RESULTS"
echo "FAIL: $FAIL_COUNT / $((PASS_COUNT + FAIL_COUNT))" | tee -a "$RESULTS"
echo "Results saved to: $RESULTS"

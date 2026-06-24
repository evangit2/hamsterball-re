#!/bin/bash
# Crash-test universal-ref-loader v3 on all 13 levels (excluding Warm-Up)
# For each level: swap MESHWORLD, install mod DLL, start game, wait 35s, check alive+hook

GAME_DIR="/home/evan/hamsterball-wasm/boxedwine-package/hamsterball"
LEVELS="$GAME_DIR/Levels"
MOD_DLL="$HOME/hamsterball-re/mods/universal-ref-loader/bass.dll"
BACKUP="$LEVELS/Level1.MESHWORLD.ORIGINAL_BACKUP"
HBTESTD_URL="http://127.0.0.1:8777"
RESULTS_FILE="/tmp/warmup_swap_results.txt"

# Ensure backup exists
if [ ! -f "$BACKUP" ]; then
    cp "$LEVELS/Level1.MESHWORLD" "$BACKUP"
fi

# Levels to test (excluding Level1=Warm-Up itself, and Level14=Master which doesn't have a standalone race level file)
LEVELS_TO_TEST=(
    "Level2:Beginner"
    "Level3:Intermediate"
    "LevelCascade:Dizzy"
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

kill_all() {
    pkill -9 -f Hamsterball 2>/dev/null
    pkill -9 -f wine 2>/dev/null
    pkill -9 -f wineserver 2>/dev/null
    pkill -9 -f Xvfb 2>/dev/null
    sleep 3
    rm -f /tmp/.X99-lock /tmp/.X11-unix/X99 2>/dev/null
}

delete_cached() {
    find "$LEVELS" -name "*.cached" -delete 2>/dev/null
}

check_hook() {
    local pid=$1
    if [ -z "$pid" ]; then
        echo "NO_PID"
        return
    fi
    # Read 6 bytes at 0x0040C4BA from process memory
    local bytes=$(dd if=/proc/$pid/mem bs=1 skip=$((0x0040C4BA)) count=6 2>/dev/null | xxd -p)
    if [[ "$bytes" == "e8"* ]]; then
        echo "HOOKED"
    else
        echo "NOT_HOOKED ($bytes)"
    fi
}

echo "=== Universal Ref Loader v3 Warm-Up Slot Swap Test ===" > "$RESULTS_FILE"
echo "Date: $(date)" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

for entry in "${LEVELS_TO_TEST[@]}"; do
    LEVEL_FILE="${entry%%:*}"
    LEVEL_NAME="${entry##*:}"
    
    echo "Testing: $LEVEL_NAME ($LEVEL_FILE)"
    echo "--- $LEVEL_NAME ($LEVEL_FILE) ---" >> "$RESULTS_FILE"
    
    # Kill everything
    kill_all
    
    # Swap level
    cp "$LEVELS/$LEVEL_FILE.MESHWORLD" "$LEVELS/Level1.MESHWORLD"
    
    # Install mod DLL
    cp "$MOD_DLL" "$GAME_DIR/bass.dll"
    
    # Delete cached
    delete_cached
    
    # Restart game via hbtestd REST API
    curl -s "$HBTESTD_URL/restart_game" > /dev/null 2>&1
    
    # Wait for game to start and load
    sleep 20
    
    # Check if game is alive
    PID=$(pgrep -f "Hamsterball.exe" | head -1)
    if [ -n "$PID" ]; then
        ALIVE="YES"
        HOOK_STATUS=$(check_hook $PID)
    else
        ALIVE="NO"
        HOOK_STATUS="N/A"
    fi
    
    # Wait a bit more (crash test - 35s total)
    sleep 15
    
    # Re-check alive
    PID2=$(pgrep -f "Hamsterball.exe" | head -1)
    if [ -n "$PID2" ]; then
        FINAL_ALIVE="YES"
    else
        FINAL_ALIVE="NO"
    fi
    
    echo "  Start: PID=$PID ALIVE=$ALIVE HOOK=$HOOK_STATUS" >> "$RESULTS_FILE"
    echo "  35s:   PID=$PID2 ALIVE=$FINAL_ALIVE" >> "$RESULTS_FILE"
    echo "  Result: $([ "$FINAL_ALIVE" = "YES" ] && echo "PASS ✅" || echo "FAIL ❌")" >> "$RESULTS_FILE"
    echo "" >> "$RESULTS_FILE"
    
    echo "  Start: ALIVE=$ALIVE HOOK=$HOOK_STATUS → 35s: ALIVE=$FINAL_ALIVE"
    
    # Kill for next test
    kill_all
done

# Restore original Level1
cp "$BACKUP" "$LEVELS/Level1.MESHWORLD"
delete_cached

# Restore original bass.dll
cp "$GAME_DIR/bass_real.dll" "$GAME_DIR/bass.dll"

echo ""
echo "=== RESULTS ==="
cat "$RESULTS_FILE"

#!/bin/bash
# Test all 15 race levels with the universal ref loader mod
# Checks if the game crashes during each level's object loading

GAME_DIR="/home/evan/hamsterball-wasm/boxedwine-package/hamsterball"
MOD_DLL="/home/evan/hamsterball-re/mods/universal-ref-loader/bass.dll"
RESULTS_FILE="/home/evan/hamsterball-re/mods/universal-ref-loader/test_results.txt"

# Level navigation indices (0=Warm-Up through 14=Impossible)
LEVEL_NAMES=("Warm-Up" "Beginner" "Intermediate" "Dizzy" "Tower" "Up" "Neon" "Expert" "Odd" "Toob" "Wobbly" "Glass" "Sky" "Master" "Impossible")
LEVEL_DESCS=("L1-no-refs" "L2-no-refs" "L3-BRIDGE-MOUSETRAP" "L4-GLUEBIE-TIPPER-WATERWHEEL-SWIRL" "L5-CATAPULT-TRAPDOOR-DRAWBRIDGE-MACE-WINDMILL-CHOMPER-TURRET" "L6-SPEEDCYLINDER-LIFTER-TIMEBUTTON" "L7-DFLOOR-TRODE-NEONPLATFORM" "L8-BONK-FAN-SAWBLADE-BRIDGE-JUDGE-BELL" "L9-LIFTER-LAUNCH" "L10-SPINNY-SAW-FALLOUT1-BLOCKDAWG" "L11-WOBBLY-WAVY" "L12-SMASHER" "L13-PILLAR-MAGNIFYER-POPCYLINDER-TRAPDOOR" "L14-BBRIDGE-BLOCKDAWG-BONK-BRIDGE-CATAPULT-GLUEBIE-POPCYLINDER-TIPPER" "L15-LOOPER-GEAR-BIGGEAR-ROTATOR-PENDULUM")

echo "=== Universal Ref Loader Level Test ===" > "$RESULTS_FILE"
echo "Started: $(date)" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

stop_game() {
    wineserver -k 2>/dev/null
    sleep 1
    pkill -9 -f Hamsterball 2>/dev/null
    pkill -9 Xvfb 2>/dev/null
    sleep 1
}

start_game() {
    # Start Xvfb
    Xvfb :99 -screen 0 1024x768x24 &
    sleep 1
    
    # Ensure mod DLL is in place
    cp "$MOD_DLL" "$GAME_DIR/bass.dll"
    
    # Launch game
    DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 wine "$GAME_DIR/Hamsterball.exe" &
    GAME_PID=$!
    echo "$GAME_PID"
}

navigate_to_race() {
    local nav_count=$1
    local d=100
    
    # Wait for title screen
    sleep 12
    
    # Title → main menu (double Enter for reliability)
    xdotool key --delay $d Return
    sleep 1.5
    xdotool key --delay $d Return
    sleep 1
    
    # LET'S PLAY → CHOOSE A GAME
    xdotool key --delay $d Return
    sleep 1
    
    # TOURNAMENT → TIME TRIALS
    xdotool key --delay $d Down
    sleep 0.3
    xdotool key --delay $d Return
    sleep 1
    
    # Navigate race list
    for i in $(seq 1 $nav_count); do
        xdotool key --delay $d Down
        sleep 0.2
    done
    
    # Start race
    xdotool key --delay $d Return
    sleep 0.5
    xdotool key --delay $d Return
}

for i in $(seq 0 14); do
    LEVEL_NAME="${LEVEL_NAMES[$i]}"
    LEVEL_DESC="${LEVEL_DESCS[$i]}"
    
    echo "Testing $LEVEL_DESC..."
    
    stop_game
    GAME_PID=$(start_game)
    sleep 1
    
    navigate_to_race $i
    
    # Wait for race to load
    sleep 6
    
    # Check if process is still alive
    if kill -0 "$GAME_PID" 2>/dev/null; then
        STATUS="PASS"
        echo "  $LEVEL_DESC: PASS (pid=$GAME_PID alive)"
    else
        STATUS="CRASH"
        echo "  $LEVEL_DESC: CRASH (pid=$GAME_PID dead)"
    fi
    
    echo "$STATUS — $LEVEL_DESC" >> "$RESULTS_FILE"
done

stop_game
echo "" >> "$RESULTS_FILE"
echo "Finished: $(date)" >> "$RESULTS_FILE"
echo "DONE" >> "$RESULTS_FILE"
echo "All tests complete!"

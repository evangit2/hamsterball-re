#!/bin/bash
# screenshot_level.sh — Run hamsterball level, wait for render, screenshot
# Args: [LEVEL_NAME] [OUT_PNG]
set -e
LEVEL="${1:-Level1}"
OUT="${2:-/tmp/hb_test_${LEVEL}.png}"
EXE="${3:-./hamsterball.exe}"
DELAY="${4:-12}"  # seconds to wait for Wine + llvmpipe to render

cd /home/evan/hamsterball-re/reimpl

# Ensure Xvfb
if ! pgrep -x Xvfb > /dev/null; then
  Xvfb :99 -screen 0 1024x768x24 &>/dev/null &
  sleep 0.5
fi

# Clean previous wine
killall wine wineserver hamsterball.exe 2>/dev/null || true
sleep 0.5

# Run game with logging
LOG="/tmp/hb_${LEVEL}.log"
DISPLAY=:99 wine "$EXE" -level "$LEVEL" &> "$LOG" &
echo "Game PID: $!"
sleep "$DELAY"

# Click center to dismiss any splash / start menu
DISPLAY=:99 xdotool click 1 2>/dev/null || true
sleep 1

# Screenshot
DISPLAY=:99 scrot "$OUT"
echo "Screenshot saved: $OUT"

# Kill game gracefully
killall -TERM hamsterball.exe 2>/dev/null || true
sleep 0.5
killall -9 hamsterball.exe 2>/dev/null || true
killall -9 wine wineserver 2>/dev/null || true

# Show last log lines
echo "--- Last 20 log lines ---"
tail -20 "$LOG"

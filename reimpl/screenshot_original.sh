#!/bin/bash
# screenshot_original.sh — Run original hamsterball, capture screenshot
# Args: LEVEL_NAME OUT_PNG [DELAY_SECONDS]
set -e
LEVEL="${1:-Level1}"
OUT="${2:-/tmp/hb_orig_${LEVEL}.png}"
DELAY="${3:-25}"  # originals need longer for menu + load

HAMSTERBALL_DIR="/home/evan/hamsterball-re/originals/installed/extracted"
cd "$HAMSTERBALL_DIR"

if ! pgrep -x Xvfb > /dev/null; then
  Xvfb :99 -screen 0 1024x768x24 &>/dev/null &
  sleep 0.5
fi

killall wine wineserver Hamsterball.exe 2>/dev/null || true
sleep 0.5

DISPLAY=:99 wine Hamsterball.exe &> /dev/null &
PID=$!
echo "Original PID: $PID"

sleep "$DELAY"

# Click center to start default level (WarmUp=Level1)
DISPLAY=:99 xdotool click 1 2>/dev/null || true
sleep 3

# For levels other than Level1, use keyboard navigation
if [ "$LEVEL" != "Level1" ]; then
  sleep 1
  DISPLAY=:99 xdotool key Return || true  # start game
  sleep 2
  # Navigate to tournament, choose level... (simplified: click through)
fi

DISPLAY=:99 scrot "$OUT"
echo "Screenshot: $OUT"

killall -9 Hamsterball.exe wine wineserver 2>/dev/null || true

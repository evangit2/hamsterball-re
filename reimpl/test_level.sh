#!/bin/bash
# test_level.sh — build, run specific level, screenshot. Args: LEVEL_NAME OUT_PNG
set -e
LEVEL="${1:-Level1}"
OUT="${2:-/tmp/hb_test_${LEVEL}.png}"
cd /home/evan/hamsterball-re/reimpl

# Rebuild
 i686-w64-mingw32-gcc -std=c11 -m32 -O2 -D_D3D8 -Iinclude \
  src/core/win32_main.c src/level/meshworld_parser.c src/level/mesh_parser.c \
  src/graphics/texture.c -o hamsterball.exe \
  -ld3d8 -ldinput8 -ldsound -ldxguid -lole32 -lwinmm -mwindows || exit 1

# Start Xvfb if needed
if ! pgrep -x Xvfb > /dev/null; then
  Xvfb :99 -screen 0 1024x768x24 &>/dev/null &
  sleep 0.5
fi

# Kill any existing wine processes
timeout 3 wine taskmgr &>/dev/null || true
killall wine wineserver hamsterball.exe 2>/dev/null || true

# Run with level arg, wait for window to appear
DISPLAY=:99 timeout 10 wine ./hamsterball.exe -level "$LEVEL" &>/dev/null || true
sleep 4

# Click center to dismiss any splash/intro
DISPLAY=:99 xdotool click 1 || true
sleep 1
DISPLAY=:99 scrot "$OUT"
echo "screenshot: $OUT"

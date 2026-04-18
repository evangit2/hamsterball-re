#!/bin/bash
# Quick test: build, run, screenshot. NEVER runs wine in background.
# Usage: ./test_screenshot.sh [output_path]
OUT="${1:-/tmp/hamsterball_test.png}"
cd "$(dirname "$0")"
i686-w64-mingw32-gcc -std=c11 -m32 -O2 -D_D3D8 -Iinclude \
  src/core/win32_main.c src/level/meshworld_parser.c src/level/mesh_parser.c \
  src/graphics/texture.c -o hamsterball.exe \
  -ld3d8 -ldinput8 -ldsound -ldxguid -lole32 -lwinmm -mwindows 2>&1 || exit 1
# Ensure Xvfb is up
if ! pgrep -x Xvfb > /dev/null; then
  Xvfb :99 -screen 0 1024x768x24 &>/dev/null &
  sleep 0.5
fi
# Run with strict timeout, then screenshot
DISPLAY=:99 timeout 5 wine ./hamsterball.exe &>/dev/null
DISPLAY=:99 scrot "$OUT" 2>/dev/null
echo "screenshot: $OUT"
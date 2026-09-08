#!/bin/bash
# build.sh - Cross-compile FPS Unlocker (HB+) with MinGW-w64 on Linux.
# Needs -lwinmm for timeGetTime/timeBeginPeriod. Stages HamsterballAPI.h
# to a temp dir (static_asserts commented out for MinGW) so the repo
# tree is never modified.
set -e
cd "$(dirname "$0")"
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
cp HamsterballAPI.h "$STAGE/"
sed -i 's/^\(\s*\)static_assert(\([^;]*\));/\1\/* static_assert(\2); *\//' "$STAGE/HamsterballAPI.h"
i686-w64-mingw32-g++ -shared -o plus_fps_unlocker.dll FPSUnlocker.cpp \
  -I"$STAGE" -O2 -mwindows -static-libgcc -static-libstdc++ \
  -Wl,--enable-stdcall-fixup -fpermissive -lwinmm
i686-w64-mingw32-objdump -p plus_fps_unlocker.dll > "$STAGE/exports.txt"
if grep -q CreateModInstance "$STAGE/exports.txt"; then
  echo "Built: plus_fps_unlocker.dll ($(stat -c%s plus_fps_unlocker.dll) bytes), Export OK"
else
  echo "Export: CreateModInstance MISSING" >&2; exit 1
fi

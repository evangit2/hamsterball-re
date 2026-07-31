#!/bin/bash
# build.sh - Cross-compile TimeWarpMod for HB+ with MinGW
set -e

cd "$(dirname "$0")"

# Disable static_asserts in HamsterballAPI.h that fail under MinGW
cp HamsterballAPI.h HamsterballAPI.h.bak
sed -i 's/static_assert(/\/\/ static_assert(/g' HamsterballAPI.h

i686-w64-mingw32-g++ -shared -o TimeWarpMod.dll TimeWarpMod.cpp \
  -I. -O2 -msse2 -mfpmath=sse -mwindows \
  -static-libgcc -static-libstdc++ \
  -Wl,--enable-stdcall-fixup \
  -Wl,--image-base,0x10000000 \
  -fpermissive \
  -ldinput8 -lwinmm \
  -lm

# Restore original header
mv HamsterballAPI.h.bak HamsterballAPI.h

echo "Build complete: TimeWarpMod.dll"
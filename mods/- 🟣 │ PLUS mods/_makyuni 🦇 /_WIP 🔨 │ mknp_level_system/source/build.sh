#!/bin/bash
# Build: mkn_level_system bass.dll proxy
# Reads mkn_level_system.txt config to swap/insert per-level setup functions
cd "$(dirname "$0")"
i686-w64-mingw32-gcc -shared -o bass.dll mkn_level_system.c \
  -I../shared -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
echo "Built bass.dll for mkn_level_system"

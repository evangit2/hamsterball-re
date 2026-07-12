#!/bin/bash
# Build: "draw-bridge doesn't open (tower race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "draw-bridge doesn't open (tower race)""

#!/bin/bash
# Build: "Flipped Camera (2?)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Flipped Camera (2?)""

#!/bin/bash
# Build: "Slippery 8balls"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Slippery 8balls""

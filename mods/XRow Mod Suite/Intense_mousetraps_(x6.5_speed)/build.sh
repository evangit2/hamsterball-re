#!/bin/bash
# Build: "Intense mousetraps (x6.5 speed)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Intense mousetraps (x6.5 speed)""

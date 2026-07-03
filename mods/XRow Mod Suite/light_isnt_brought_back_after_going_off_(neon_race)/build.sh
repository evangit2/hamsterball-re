#!/bin/bash
# Build: "light isn't brought back after going off (neon race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "light isn't brought back after going off (neon race)""

#!/bin/bash
# Build: "light is always on (neon race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "light is always on (neon race)""

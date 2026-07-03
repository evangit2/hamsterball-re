#!/bin/bash
# Build: "Weaker mousetraps (x0 speed)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Weaker mousetraps (x0 speed)""

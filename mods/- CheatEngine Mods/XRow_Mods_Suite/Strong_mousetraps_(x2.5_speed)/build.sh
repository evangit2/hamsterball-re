#!/bin/bash
# Build: "Strong mousetraps (x2.5 speed)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Strong mousetraps (x2.5 speed)""

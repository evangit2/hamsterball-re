#!/bin/bash
# Build: "Mid-Strong mousetraps (x3.75 speed)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Mid-Strong mousetraps (x3.75 speed)""

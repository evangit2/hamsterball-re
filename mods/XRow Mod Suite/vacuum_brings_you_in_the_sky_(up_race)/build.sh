#!/bin/bash
# Build: "vacuum brings you in the sky (up race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "vacuum brings you in the sky (up race)""

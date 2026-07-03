#!/bin/bash
# Build: "remove the flickring (neon arena)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "remove the flickring (neon arena)""

#!/bin/bash
# Build: "giant circle stands platforms (Arenas)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "giant circle stands platforms (Arenas)""

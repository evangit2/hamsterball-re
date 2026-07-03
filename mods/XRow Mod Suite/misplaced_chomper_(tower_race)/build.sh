#!/bin/bash
# Build: "misplaced chomper (tower race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "misplaced chomper (tower race)""

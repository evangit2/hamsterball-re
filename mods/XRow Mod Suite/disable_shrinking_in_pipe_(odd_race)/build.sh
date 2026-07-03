#!/bin/bash
# Build: "disable shrinking in pipe (odd race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "disable shrinking in pipe (odd race)""

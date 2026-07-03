#!/bin/bash
# Build: "gravity doesnt return after changing (odd race, odd arena)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "gravity doesnt return after changing (odd race, odd arena)""

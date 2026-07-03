#!/bin/bash
# Build: "disable gravity on walls (odd race and odd arena)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "disable gravity on walls (odd race and odd arena)""

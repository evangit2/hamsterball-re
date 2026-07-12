#!/bin/bash
# Build: "grow hamster after shrink (odd race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "grow hamster after shrink (odd race)""

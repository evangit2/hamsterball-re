#!/bin/bash
# Build: "de-activate catapults (tower race, tower arena and master race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "de-activate catapults (tower race, tower arena and master race)""

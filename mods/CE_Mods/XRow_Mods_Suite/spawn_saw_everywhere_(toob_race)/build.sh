#!/bin/bash
# Build: "spawn saw everywhere (toob race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "spawn saw everywhere (toob race)""

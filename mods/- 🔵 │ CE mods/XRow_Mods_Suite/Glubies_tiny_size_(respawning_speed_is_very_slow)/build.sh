#!/bin/bash
# Build: "Glubies tiny size (respawning speed is very slow)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Glubies tiny size (respawning speed is very slow)""

#!/bin/bash
# Build: "Glubies small size (respawning speed is slow)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Glubies small size (respawning speed is slow)""

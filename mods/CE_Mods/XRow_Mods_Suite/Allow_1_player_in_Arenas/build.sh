#!/bin/bash
# Build: "Allow 1 player in Arenas"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Allow 1 player in Arenas""

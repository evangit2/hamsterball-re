#!/bin/bash
# Build: "spawn platforms everywhere (Arenas)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "spawn platforms everywhere (Arenas)""

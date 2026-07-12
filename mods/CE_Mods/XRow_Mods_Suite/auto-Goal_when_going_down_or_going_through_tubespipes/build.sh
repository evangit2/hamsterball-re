#!/bin/bash
# Build: "auto-Goal when going down or going through tubes/pipes"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "auto-Goal when going down or going through tubes/pipes""

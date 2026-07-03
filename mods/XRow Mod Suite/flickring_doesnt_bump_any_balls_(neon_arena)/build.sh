#!/bin/bash
# Build: "flickring doesnt bump any balls (neon arena)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "flickring doesnt bump any balls (neon arena)""

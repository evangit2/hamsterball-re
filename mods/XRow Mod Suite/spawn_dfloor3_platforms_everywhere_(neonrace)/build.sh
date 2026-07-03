#!/bin/bash
# Build: "spawn dfloor3 platforms everywhere (neonrace) "
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "spawn dfloor3 platforms everywhere (neonrace) ""

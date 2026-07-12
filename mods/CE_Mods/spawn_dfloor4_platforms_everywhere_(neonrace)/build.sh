#!/bin/bash
# Build: "spawn dfloor4 platforms everywhere (neonrace) "
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "spawn dfloor4 platforms everywhere (neonrace) ""

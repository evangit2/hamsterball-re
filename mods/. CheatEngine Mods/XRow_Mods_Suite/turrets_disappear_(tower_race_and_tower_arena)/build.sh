#!/bin/bash
# Build: "turrets disappear (tower race and tower arena)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "turrets disappear (tower race and tower arena)""

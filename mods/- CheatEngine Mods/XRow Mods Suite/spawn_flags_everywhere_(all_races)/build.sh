#!/bin/bash
# Build: "spawn flags everywhere (all races)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "spawn flags everywhere (all races)""

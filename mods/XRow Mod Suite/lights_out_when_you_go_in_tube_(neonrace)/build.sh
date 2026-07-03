#!/bin/bash
# Build: "lights out when you go in tube (neonrace)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "lights out when you go in tube (neonrace)""

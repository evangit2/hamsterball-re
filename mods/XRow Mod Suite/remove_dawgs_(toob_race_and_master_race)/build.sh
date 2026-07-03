#!/bin/bash
# Build: "remove dawgs (toob race and master race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "remove dawgs (toob race and master race)""

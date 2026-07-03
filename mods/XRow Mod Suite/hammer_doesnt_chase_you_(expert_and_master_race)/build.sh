#!/bin/bash
# Build: "hammer doesnt chase you (expert and master race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "hammer doesnt chase you (expert and master race)""

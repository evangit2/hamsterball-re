#!/bin/bash
# Build: "remove neon bridges (neon race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "remove neon bridges (neon race)""

#!/bin/bash
# Build: "remove 8balls/badballs"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "remove 8balls/badballs""

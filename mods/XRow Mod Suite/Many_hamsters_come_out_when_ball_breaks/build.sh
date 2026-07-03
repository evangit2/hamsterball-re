#!/bin/bash
# Build: "Many hamsters come out when ball breaks"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Many hamsters come out when ball breaks""

#!/bin/bash
# Build: "remove fan-storms (expert race, crashes in expert arena)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "remove fan-storms (expert race, crashes in expert arena)""

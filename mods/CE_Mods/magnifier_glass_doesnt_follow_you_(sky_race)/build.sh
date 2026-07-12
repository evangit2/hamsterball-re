#!/bin/bash
# Build: "magnifier glass doesnt follow you (sky race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "magnifier glass doesnt follow you (sky race)""

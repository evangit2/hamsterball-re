#!/bin/bash
# Build: "remove tipper floors (dizzy and master race)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "remove tipper floors (dizzy and master race)""

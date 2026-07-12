#!/bin/bash
# Build: "spinning floor doesn't move any balls (dizzy race, dizzy arena)"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "spinning floor doesn't move any balls (dizzy race, dizzy arena)""

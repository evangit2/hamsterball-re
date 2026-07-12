#!/bin/bash
# Build: "Press M to spawn mini 8ball"
# NOTE: This is a complex CEA script that needs manual C translation.
# The CEA source is in script.CEA and embedded in mod.c as comments.
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Press M to spawn mini 8ball""

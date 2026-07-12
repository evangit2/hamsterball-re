#!/bin/bash
# Build: "Adjust arena time with UP arrow and DOWN arrow "
# NOTE: This is a complex CEA script that needs manual C translation.
# The CEA source is in script.CEA and embedded in mod.c as comments.
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Adjust arena time with UP arrow and DOWN arrow ""

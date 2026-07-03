#!/bin/bash
# Build: "Camera FOV very far"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Camera FOV very far""

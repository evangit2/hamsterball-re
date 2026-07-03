#!/bin/bash
# Build: "Camera Angle change 2"
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Camera Angle change 2""

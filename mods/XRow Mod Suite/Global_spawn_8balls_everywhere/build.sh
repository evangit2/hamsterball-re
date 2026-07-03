#!/bin/bash
# Build: "Global spawn 8balls everywhere "
i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
echo "Built bass.dll for: "Global spawn 8balls everywhere ""

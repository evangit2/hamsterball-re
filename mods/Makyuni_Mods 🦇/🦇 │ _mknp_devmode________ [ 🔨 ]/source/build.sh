#!/bin/bash
# build.sh - Cross-compile mknp_devmode (bass.dll proxy) with MinGW
set -e
cd "$(dirname "$0")"

i686-w64-mingw32-gcc -shared -o _mknp_devmode.dll mknp_devmode.c bass.def \
  -O2 -static -static-libgcc \
  -Wl,--kill-at -Wl,--enable-stdcall-fixup \
  -msse2 -mfpmath=sse -mwindows \
  -luser32 -lwinmm -lkernel32

cp _mknp_devmode.dll ../_mknp_devmode.dll
rm -f mknp_devmode.dll

echo "=== Build complete: _mknp_devmode.dll (also copied to mod root) ==="
echo "=== Exports (must be UNDECORATED BASS_* names) ==="
i686-w64-mingw32-objdump -p _mknp_devmode.dll | grep "BASS_Init$\|BASS_SetConfig$\|BASS_MusicPlayEx$" | head -n 5 || true
echo "=== DLL dependencies ==="
i686-w64-mingw32-objdump -p _mknp_devmode.dll | grep "DLL Name" || true

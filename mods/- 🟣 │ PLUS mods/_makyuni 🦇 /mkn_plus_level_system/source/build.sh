#!/bin/bash
set -e
cd "$(dirname "$0")"
cp HamsterballAPI.h HamsterballAPI.h.bak
sed -i 's/static_assert(/\/\/ static_assert(/g' HamsterballAPI.h
i686-w64-mingw32-g++ -shared -o mkn_plus_level_system.dll LevelSystem_MinGW.cpp \
  -I. -O2 -msse2 -mfpmath=sse -mwindows \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -lkernel32 -luser32 \
  -Wl,-e,_DllMain@12 -Wl,--enable-stdcall-fixup \
  -Wl,--image-base,0x10000000 -Wl,--gc-sections \
  -ffunction-sections -fdata-sections \
  -fpermissive
mv HamsterballAPI.h.bak HamsterballAPI.h
echo "Build complete: mkn_plus_level_system.dll"

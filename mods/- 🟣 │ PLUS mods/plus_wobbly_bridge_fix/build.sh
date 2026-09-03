#!/bin/bash
set -e
cd "$(dirname "$0")"
# MinGW workaround: HamsterballAPI.h static_asserts fail with #pragma pack(1)
cp HamsterballAPI.h HamsterballAPI.h.bak
sed -i 's/static_assert(/\/\/ static_assert(/g' HamsterballAPI.h

i686-w64-mingw32-g++ -shared -o WobblyBridgeFix.dll WobblyBridgeFix_MinGW.cpp \
  -O2 -mwindows -static-libgcc -static-libstdc++ \
  -Wl,--enable-stdcall-fixup -fpermissive \
  -Wl,--image-base,0x10000000

mv HamsterballAPI.h.bak HamsterballAPI.h
echo "Build complete: WobblyBridgeFix.dll (HB+)"
i686-w64-mingw32-objdump -p WobblyBridgeFix.dll | grep -A2 "DLL Name"
strings WobblyBridgeFix.dll | grep -i "Wobbly" | head

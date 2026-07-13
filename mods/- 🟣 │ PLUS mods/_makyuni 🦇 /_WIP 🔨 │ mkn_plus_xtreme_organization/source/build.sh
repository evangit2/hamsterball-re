#!/bin/bash
# build.sh — Cross-compile XtremeOrganization mod with MinGW
set -e

cd "$(dirname "$0")"

i686-w64-mingw32-g++ -shared -o mkn_plus_xtreme_organization.dll XtremeOrganization_MinGW.cpp nocrt.cpp \
  -I. -O2 -msse2 -mfpmath=sse -mwindows \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -fno-asynchronous-unwind-tables -fno-unwind-tables \
  -fno-stack-protector -mno-stack-arg-probe \
  -nostdlib -nostartfiles \
  -lkernel32 -luser32 \
  -Wl,-e,_DllMain@12 -Wl,--enable-stdcall-fixup \
  -Wl,--image-base,0x10000000 -Wl,--gc-sections \
  -ffunction-sections -fdata-sections \
  -fpermissive -fno-builtin \
  -Wl,--exclude-symbols,_strcmp -Wl,--exclude-symbols,_strlen \
  -Wl,--exclude-symbols,_memcpy -Wl,--exclude-symbols,_memset \
  -Wl,--exclude-symbols,_malloc -Wl,--exclude-symbols,_free

echo "Build complete: mkn_plus_xtreme_organization.dll"

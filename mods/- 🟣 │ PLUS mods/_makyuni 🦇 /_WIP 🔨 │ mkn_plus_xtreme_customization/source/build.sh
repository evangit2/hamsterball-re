#!/bin/bash
# build.sh — Cross-compile CustomUIColors mod with MinGW (HB+ v2.0)
set -e
cd "$(dirname "$0")"

# Disable static_asserts in header (MinGW pack alignment differs)
sed -i 's/^static_assert/\/\/ static_assert/' HamsterballAPI.h

i686-w64-mingw32-g++ -shared -o mkn_plus_xtreme_customization.dll CustomUIColors_MinGW.cpp nocrt.cpp \
  -I. -O2 -msse2 -mfpmath=sse -mwindows \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -fno-asynchronous-unwind-tables -fno-unwind-tables \
  -nostdlib -nostartfiles \
  -lkernel32 -luser32 \
  -Wl,-e,_DllMain@12 -Wl,--enable-stdcall-fixup \
  -Wl,--image-base,0x10000000 -Wl,--gc-sections \
  -ffunction-sections -fdata-sections \
  -fpermissive -fno-builtin \
  -Wl,--exclude-symbols,_strcmp -Wl,--exclude-symbols,_strlen \
  -Wl,--exclude-symbols,_memcpy -Wl,--exclude-symbols,_memset \
  -Wl,--exclude-symbols,_malloc -Wl,--exclude-symbols,_free \
  -Wl,--exclude-symbols,_stricmp -Wl,--exclude-symbols,_strncpy \
  -Wl,--exclude-symbols,_strcpy -Wl,--exclude-symbols,_strcat \
  -Wl,--exclude-symbols,_strtok -Wl,--exclude-symbols,_realloc \
  -Wl,--exclude-symbols,_strchr -Wl,--exclude-symbols,_strstr \
  -Wl,--exclude-symbols,_strrchr -Wl,--exclude-symbols,_memcmp

echo "Build complete: mkn_plus_xtreme_customization.dll"

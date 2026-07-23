#!/bin/bash
# Build script for mkn_plus_neon_custom_colors
set -e
cd "$(dirname "$0")/source"
i686-w64-mingw32-g++ -shared -o ../mkn_plus_neon_custom_colors.dll \
    NeonCustomColors_MinGW.cpp nocrt.cpp \
    -I. -O2 -msse2 -mfpmath=sse -mwindows \
    -fno-exceptions -fno-rtti -fno-threadsafe-statics \
    -fno-asynchronous-unwind-tables -fno-unwind-tables \
    -nostdlib -nostartfiles \
    -lkernel32 -luser32 \
    -Wl,-e,_DllMain@12 -Wl,--enable-stdcall-fixup \
    -Wl,--image-base,0x10000000 -Wl,--gc-sections \
    -ffunction-sections -fdata-sections \
    -fpermissive -fno-builtin
echo "Build complete: $(ls -la ../mkn_plus_neon_custom_colors.dll)"

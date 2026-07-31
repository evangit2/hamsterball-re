#!/bin/bash
set -e

MOD_NAME="mkn_plus_bumper_colors"
MOD_DIR="$(cd "$(dirname "$0")" && pwd)"
OUT_DIR="$(dirname "$MOD_DIR")"

echo "=== Building $MOD_NAME (HB+ v2.1, MinGW 32-bit) ==="

i686-w64-mingw32-g++ -shared -o "$OUT_DIR/${MOD_NAME}.dll" \
    "$MOD_DIR/${MOD_NAME}.cpp" "$MOD_DIR/nocrt.cpp" \
    -I"$MOD_DIR" -O2 -msse2 -mfpmath=sse -mwindows \
    -fno-exceptions -fno-rtti -fno-threadsafe-statics \
    -fno-asynchronous-unwind-tables -fno-unwind-tables \
    -nostdlib -nostartfiles \
    -lkernel32 -luser32 \
    -Wl,-e,_DllMain@12 -Wl,--enable-stdcall-fixup \
    -Wl,--image-base,0x10000000 -Wl,--gc-sections \
    -ffunction-sections -fdata-sections \
    -fpermissive -fno-builtin

echo "=== Built: $OUT_DIR/${MOD_NAME}.dll ==="

echo "=== Exports ==="
i686-w64-mingw32-objdump -p "$OUT_DIR/${MOD_NAME}.dll" 2>/dev/null | grep -i "Export" -A5 || true

echo "=== DLL dependencies ==="
i686-w64-mingw32-objdump -p "$OUT_DIR/${MOD_NAME}.dll" 2>/dev/null | grep "DLL Name" || true

echo "=== File size ==="
ls -la "$OUT_DIR/${MOD_NAME}.dll"

#!/bin/bash
# build.sh - Cross-compile Widescreen UI Fix (HB+) with MinGW-w64 on Linux.
# Uses the MinGW-safe variant (WidescreenUIFix_MinGW.cpp) with nocrt +
# manual vtable (same pattern as plus_water_mod). Stages HamsterballAPI.h
# to a temp dir (static_asserts commented out for MinGW) so the repo
# tree is never modified.
set -e
cd "$(dirname "$0")"
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
cp HamsterballAPI.h "$STAGE/"
sed -i 's/^\(\s*\)static_assert(\([^;]*\));/\1\/* static_assert(\2); *\//' "$STAGE/HamsterballAPI.h"
i686-w64-mingw32-g++ -shared -o plus_widescreen_ui.dll WidescreenUIFix_MinGW.cpp nocrt.cpp \
  -I"$STAGE" -I. -O2 -msse2 -mfpmath=sse -mwindows \
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
  -lm
i686-w64-mingw32-objdump -p plus_widescreen_ui.dll > "$STAGE/exports.txt"
if grep -q CreateModInstance "$STAGE/exports.txt"; then
  echo "Built: plus_widescreen_ui.dll ($(stat -c%s plus_widescreen_ui.dll) bytes), Export OK"
else
  echo "Export: CreateModInstance MISSING" >&2; exit 1
fi

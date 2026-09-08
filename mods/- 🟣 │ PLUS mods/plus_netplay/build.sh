#!/bin/bash
# build.sh - Cross-compile Netplay Mod (HB+) with MinGW-w64 on Linux.
# Links nocrt.cpp (NetplayMod.cpp redefines memcpy/memset/strlen/strcmp
# to nc_*). Stages HamsterballAPI.h to a temp dir (static_asserts
# commented out for MinGW) so the repo tree is never modified.
set -e
cd "$(dirname "$0")"
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
cp HamsterballAPI.h "$STAGE/"
sed -i 's/^\(\s*\)static_assert(\([^;]*\));/\1\/* static_assert(\2); *\//' "$STAGE/HamsterballAPI.h"
i686-w64-mingw32-g++ -shared -o plus_netplay.dll NetplayMod.cpp nocrt.cpp \
  -I"$STAGE" -I. -O2 -mwindows -static-libgcc -static-libstdc++ \
  -Wl,--enable-stdcall-fixup -fpermissive
i686-w64-mingw32-objdump -p plus_netplay.dll > "$STAGE/exports.txt"
if grep -q CreateModInstance "$STAGE/exports.txt"; then
  echo "Built: plus_netplay.dll ($(stat -c%s plus_netplay.dll) bytes), Export OK"
else
  echo "Export: CreateModInstance MISSING" >&2; exit 1
fi

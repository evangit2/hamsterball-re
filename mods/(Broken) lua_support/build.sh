#!/bin/bash
# Build script for lua_support mod
# Compiles Lua 5.1.5 source + mod code into bass.dll

set -e

cd "$(dirname "$0")"

CC=i686-w64-mingw32-gcc
OUT=bass.dll

# Lua source files (exclude lua.c/luac.c which are standalone executables)
LUA_SRCS=$(ls lua/src/l*.c 2>/dev/null | grep -v 'lua\.c\|luac\.c' | tr '\n' ' ')

# Include paths
INCLUDES="-I lua/src"

# Compiler flags
CFLAGS="-O2 -Wall -shared -DNO_LUA_STANDALONE"
LDFLAGS="-lwinmm -Wl,--enable-stdcall-fixup -static -static-libgcc -Wl,--add-stdcall-alias -lm"

echo "Building lua_support mod..."
echo "  CC: $CC"
echo "  Lua sources: $(echo $LUA_SRCS | wc -w) files"
echo ""

$CC $CFLAGS $INCLUDES -o $OUT lua_support.c $LUA_SRCS $LDFLAGS

echo ""
echo "Build complete: $OUT ($(du -h $OUT | cut -f1))"

# Verify all 10 BASS exports are present
echo ""
echo "Verifying BASS exports..."
BASS_EXPORTS=$(i686-w64-mingw32-objdump -p $OUT 2>/dev/null | grep -E "^\t\[" | sed 's/.*\] //' | grep '^BASS_' | sort)
EXPORT_COUNT=$(echo "$BASS_EXPORTS" | wc -l)
echo "  Found $EXPORT_COUNT BASS exports"

REQUIRED="BASS_ChannelSetAttributes BASS_ChannelStop BASS_ErrorGetCode BASS_Free BASS_Init BASS_MusicLoad BASS_MusicPlayEx BASS_SetConfig BASS_Start BASS_Stop"
MISSING=0
for fn in $REQUIRED; do
    if ! echo "$BASS_EXPORTS" | grep -q "$fn"; then
        echo "  MISSING: $fn"
        MISSING=1
    fi
done

if [ $MISSING -eq 0 ]; then
    echo "  All 10 required BASS exports present!"
else
    echo "  WARNING: Missing exports!"
fi

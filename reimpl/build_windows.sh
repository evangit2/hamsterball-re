#!/bin/bash
# build_windows.sh - Cross-compile Hamsterball for Windows x64
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-win64"
MINGW_LIBS="$SCRIPT_DIR/mingw-libs"

# SDL2 paths
SDL2_DIR="$MINGW_LIBS/SDL2-2.32.0/x86_64-w64-mingw32"
SDL2_IMG_DIR="$MINGW_LIBS/SDL2_image-2.8.4/x86_64-w64-mingw32"
SDL2_MIX_DIR="$MINGW_LIBS/SDL2_mixer-2.8.0/x86_64-w64-mingw32"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "=== Cross-compiling Hamsterball for Windows x64 ==="

# Compile all source files
SOURCES=$(find "$SCRIPT_DIR/src" -name "*.c" ! -name "level_viewer*.c" ! -name "mesh_test*.c" | sort)

x86_64-w64-mingw32-gcc \
    -std=c11 -O2 -Wall \
    -D_WIN32 -DGLEW_STATIC \
    -I"$SCRIPT_DIR/include" \
    -I"$SDL2_DIR/include" \
    -I"$SDL2_DIR/include/SDL2" \
    -I"$SDL2_IMG_DIR/include" \
    -I"$SDL2_IMG_DIR/include/SDL2" \
    -I"$SDL2_MIX_DIR/include" \
    -I"$SDL2_MIX_DIR/include/SDL2" \
    $SOURCES \
    -L"$SDL2_DIR/lib" \
    -L"$SDL2_IMG_DIR/lib" \
    -L"$SDL2_MIX_DIR/lib" \
    -lmingw32 -lSDL2main -lSDL2 \
    -lSDL2_image \
    -lSDL2_mixer \
    -lopengl32 -lglu32 \
    -lsetupapi -lole32 -loleaut32 -limm32 -lversion -lwinmm \
    -lm \
    -static \
    -mwindows \
    -o hamsterball.exe

echo "=== Build successful: $BUILD_DIR/hamsterball.exe ==="
ls -la hamsterball.exe

# Copy DLLs next to the exe
for dir in "$SDL2_DIR/bin" "$SDL2_IMG_DIR/bin" "$SDL2_MIX_DIR/bin"; do
    if [ -d "$dir" ]; then
        cp "$dir"/*.dll "$BUILD_DIR/" 2>/dev/null || true
    fi
done

echo "=== DLLs copied ==="
ls -la *.dll 2>/dev/null || echo "(no DLLs needed - fully static)"
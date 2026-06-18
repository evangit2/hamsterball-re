# Makefile for Hamsterball Water Physics Mod
# Cross-compiles the bass.dll proxy on Linux with MinGW.

CC = i686-w64-mingw32-gcc
CFLAGS = -O2 -Wall -m32 -static -static-libgcc
LDFLAGS = -shared -lwinmm -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias

SRC = hamsterball_water_mod.c
DEF = bass_exports.def
OUT = bass.dll

.PHONY: all clean package

all: $(OUT)

$(OUT): $(SRC) $(DEF)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(DEF)

clean:
	rm -f $(OUT) *_water_mod.log

# Creates a zip ready to drop into the Hamsterball game folder on Windows.
package: $(OUT)
	zip hamsterball-water-mod.zip $(OUT) hamsterball_water.ini install_water_mod.bat uninstall_water_mod.bat README_WATER_MOD.md

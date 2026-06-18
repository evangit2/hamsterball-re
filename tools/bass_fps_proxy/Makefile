# Makefile for Hamsterball FPS Mod
# Cross-compiles the bass.dll proxy on Linux with MinGW.

CC = i686-w64-mingw32-gcc
CFLAGS = -O2 -Wall -m32 -static -static-libgcc
LDFLAGS = -shared -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias
LDLIBS = -lwinmm

SRC = hamsterball_fps_mod.c
DEF = bass_exports.def
OUT = bass.dll

.PHONY: all clean package

all: $(OUT)

$(OUT): $(SRC) $(DEF)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(DEF) $(LDLIBS)

clean:
	rm -f $(OUT) *_fps_mod.log

# Creates a zip ready to drop into the Hamsterball game folder on Windows.
package: $(OUT)
	zip hamsterball-fps-mod.zip $(OUT) hamsterball_fps.ini install_fps_mod.bat uninstall_fps_mod.bat README_FPS_MOD.md

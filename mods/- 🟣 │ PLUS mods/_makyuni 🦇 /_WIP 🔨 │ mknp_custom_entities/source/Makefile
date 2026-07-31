CC = i686-w64-mingw32-gcc
CFLAGS = -O2 -Wall -m32 -static -static-libgcc -I.
LDFLAGS = -shared -lwinmm -lshlwapi -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias
SRC = custom_entities.c
DEF = bass.def
OUT = bass.dll

all: $(OUT)

$(OUT): $(SRC) $(DEF)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(DEF) -lshlwapi

clean:
	rm -f $(OUT)

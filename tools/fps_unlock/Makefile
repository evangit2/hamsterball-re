# Hamsterball FPS Unlock — Makefile
CC = i686-w64-mingw32-gcc
CFLAGS = -O2 -Wall -Wextra -std=gnu99
LDFLAGS = -shared -Wl,--enable-stdcall-fixup

.PHONY: all clean

all: fps_unlock.dll

fps_unlock.dll: fps_unlock.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ fps_unlock.c

clean:
	rm -f fps_unlock.dll

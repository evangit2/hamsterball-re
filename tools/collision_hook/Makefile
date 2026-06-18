# Hamsterball Collision Hook — Makefile
# Cross-compile from Linux with MinGW

CC = i686-w64-mingw32-gcc
CFLAGS = -O2 -Wall -Wextra -std=gnu99
LDFLAGS_DLL = -shared -Wl,--enable-stdcall-fixup -Wl,--out-implib,collision_hook.lib
LDFLAGS_EXE = -lpsapi

.PHONY: all clean

all: collision_hook.dll injector.exe

collision_hook.dll: collision_hook.c
	$(CC) $(CFLAGS) $(LDFLAGS_DLL) -o $@ collision_hook.c

injector.exe: injector.c
	$(CC) $(CFLAGS) $(LDFLAGS_EXE) -o $@ injector.c

clean:
	rm -f collision_hook.dll injector.exe collision_hook.lib

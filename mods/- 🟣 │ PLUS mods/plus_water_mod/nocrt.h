/*
 * nocrt.h — Minimal CRT replacement for HB+ mods compiled with MinGW.
 * All functions prefixed with nc_ to avoid conflicts with <cstdio>/<cstdlib>.
 *
 * Build: -nostdlib -nostartfiles -fno-exceptions -fno-rtti -mwindows
 *        -lkernel32 -luser32 -lws2_32 -Wl,-e,_DllMain@12
 */
#ifndef NOCRT_H
#define NOCRT_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// Memory
void* nc_malloc(size_t size);
void nc_free(void* ptr);
void* nc_memcpy(void* dst, const void* src, size_t count);
void* nc_memset(void* dst, int val, size_t count);

// String
size_t nc_strlen(const char* s);
int nc_strcmp(const char* a, const char* b);
int nc_strncmp(const char* a, const char* b, size_t n);
int nc_memcmp(const void* a, const void* b, size_t count);
char* nc_strncpy(char* dst, const char* src, size_t n);
int nc_snprintf(char* buf, size_t size, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// C++ operator new/delete using our malloc
#endif

#endif // NOCRT_H

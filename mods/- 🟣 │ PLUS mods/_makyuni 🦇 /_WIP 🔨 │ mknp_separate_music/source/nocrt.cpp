/*
 * nocrt.cpp — Implementation of minimal CRT functions using Win32 only.
 * No msvcrt.dll dependency.
 */
#include "nocrt.h"

extern "C" {

void* nc_malloc(size_t size) {
    return HeapAlloc(GetProcessHeap(), 0, size ? size : 1);
}

void nc_free(void* ptr) {
    if (ptr) HeapFree(GetProcessHeap(), 0, ptr);
}

void* nc_memcpy(void* dst, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    while (count--) *d++ = *s++;
    return dst;
}

void* nc_memset(void* dst, int val, size_t count) {
    unsigned char* p = (unsigned char*)dst;
    while (count--) *p++ = (unsigned char)val;
    return dst;
}

size_t nc_strlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int nc_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

char* nc_strncpy(char* dst, const char* src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = '\0';
    return dst;
}

int nc_snprintf(char* buf, size_t size, const char* fmt, ...) {
    typedef int (WINAPI *wvsprintfA_t)(LPSTR, LPCSTR, va_list);
    static wvsprintfA_t pWvsprintfA = NULL;
    if (!pWvsprintfA) {
        HMODULE h = GetModuleHandleA("user32.dll");
        if (h) pWvsprintfA = (wvsprintfA_t)GetProcAddress(h, "wvsprintfA");
    }
    if (!pWvsprintfA) return 0;
    va_list args;
    va_start(args, fmt);
    char temp[512];
    int result = pWvsprintfA(temp, fmt, args);
    va_end(args);
    if (result > 0) {
        if ((size_t)result >= size) result = (int)size - 1;
        nc_memcpy(buf, temp, result);
        buf[result] = '\0';
    }
    return result;
}

} // extern "C"

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    return TRUE;
}

extern "C" void* __cdecl memset(void* dst, int val, size_t count) {
    return nc_memset(dst, val, count);
}
extern "C" void* __cdecl memcpy(void* dst, const void* src, size_t count) {
    return nc_memcpy(dst, src, count);
}
extern "C" size_t __cdecl strlen(const char* s) {
    return nc_strlen(s);
}
extern "C" int __cdecl strcmp(const char* a, const char* b) {
    return nc_strcmp(a, b);
}
extern "C" char* __cdecl strncpy(char* dst, const char* src, size_t n) {
    return nc_strncpy(dst, src, n);
}
extern "C" void* __cdecl malloc(size_t size) {
    return nc_malloc(size);
}
extern "C" void __cdecl free(void* ptr) {
    nc_free(ptr);
}

void* __cdecl operator new(unsigned int size) {
    return nc_malloc(size);
}
void __cdecl operator delete(void* ptr) {
    nc_free(ptr);
}

extern "C" void __chkstk_ms(void) {}

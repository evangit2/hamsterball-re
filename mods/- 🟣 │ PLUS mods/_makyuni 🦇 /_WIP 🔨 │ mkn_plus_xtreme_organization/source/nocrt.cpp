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
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

int nc_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int nc_stricmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return (int)(unsigned char)ca - (int)(unsigned char)cb;
        a++; b++;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

char* nc_strncpy(char* dst, const char* src, size_t n) {
    size_t i = 0;
    while (i < n && src[i]) { dst[i] = src[i]; i++; }
    if (i < n) dst[i] = '\0';
    return dst;
}

char* nc_strrchr(char* s, char c) {
    char* last = NULL;
    while (*s) {
        if (*s == c) last = s;
        s++;
    }
    return last;
}

int nc_snprintf(char* buf, size_t size, const char* fmt, ...) {
    // Minimal snprintf: only supports %s and %d
    va_list args;
    va_start(args, fmt);
    size_t pos = 0;
    while (*fmt && pos < size - 1) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                const char* str = va_arg(args, const char*);
                while (*str && pos < size - 1) { buf[pos++] = *str++; }
            } else if (*fmt == 'd') {
                int val = va_arg(args, int);
                char tmp[16];
                int neg = 0;
                if (val < 0) { neg = 1; val = -val; }
                int len = 0;
                if (val == 0) tmp[len++] = '0';
                while (val > 0) { tmp[len++] = '0' + (val % 10); val /= 10; }
                if (neg && pos < size - 1) buf[pos++] = '-';
                while (len > 0 && pos < size - 1) buf[pos++] = tmp[--len];
            }
            fmt++;
        } else {
            buf[pos++] = *fmt++;
        }
    }
    buf[pos] = '\0';
    va_end(args);
    return (int)pos;
}

}

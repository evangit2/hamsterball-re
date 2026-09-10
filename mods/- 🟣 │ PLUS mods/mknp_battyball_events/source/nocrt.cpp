/*
 * nocrt.cpp — Minimal CRT replacement for HB+ mods compiled with MinGW.
 *
 * Provides the small set of CRT functions the water visuals mod needs,
 * implemented with only Win32 APIs (no msvcrt dependency). This keeps the
 * resulting DLL ship-safe: the only import is KERNEL32.dll.
 */
#include "nocrt.h"

#include <stdarg.h>
#include <stdint.h>

extern "C" {

// ── Memory ────────────────────────────────────────────────────────────────

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
    unsigned char* d = (unsigned char*)dst;
    while (count--) *d++ = (unsigned char)val;
    return dst;
}

// ── String ────────────────────────────────────────────────────────────────

size_t nc_strlen(const char* s) {
    const char* p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

int nc_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int nc_strncmp(const char* a, const char* b, size_t n) {
    while (n > 0 && *a && (*a == *b)) { a++; b++; n--; }
    if (n == 0) return 0;
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int nc_memcmp(const void* a, const void* b, size_t count) {
    const unsigned char* pa = (const unsigned char*)a;
    const unsigned char* pb = (const unsigned char*)b;
    while (count--) {
        if (*pa != *pb) return (int)*pa - (int)*pb;
        pa++; pb++;
    }
    return 0;
}

char* nc_strncpy(char* dst, const char* src, size_t n) {
    size_t i = 0;
    while (i < n && src[i]) { dst[i] = src[i]; i++; }
    while (i < n) { dst[i] = '\0'; i++; }
    return dst;
}

// ── Minimal snprintf (subset: %d %u %x %s %c %f %%) ───────────────────────

static void nc_utoa(unsigned int v, char* out, int base, int upper) {
    char tmp[12];
    int i = 0;
    if (v == 0) { tmp[i++] = '0'; }
    while (v > 0) {
        int dig = v % base;
        tmp[i++] = (dig < 10) ? ('0' + dig) : ((upper ? 'A' : 'a') + (dig - 10));
        v /= base;
    }
    while (i > 0) *out++ = tmp[--i];
    *out = '\0';
}

static void nc_ftoa(double v, char* out, int prec) {
    char tmp[32];
    if (v < 0) { *out++ = '-'; v = -v; }
    // Integer part
    unsigned long long ip = (unsigned long long)v;
    // Round fractional to prec
    double frac = v - (double)ip;
    for (int i = 0; i < prec; i++) frac *= 10.0;
    unsigned long long fp = (unsigned long long)(frac + 0.5);
    // Handle rounding carry
    if (fp >= 10ULL) {
        for (int i = 0; i < prec; i++) fp /= 10;
        ip += 1;
        for (int i = 0; i < prec; i++) fp *= 10;
        fp = 0;
    }
    char t[32];
    int ti = 0;
    if (ip == 0) { t[ti++] = '0'; }
    while (ip > 0) { t[ti++] = '0' + (ip % 10); ip /= 10; }
    while (ti > 0) *out++ = t[--ti];
    if (prec > 0) {
        *out++ = '.';
        for (int i = 0; i < prec; i++) {
            *out++ = '0' + (fp % 10);
            fp /= 10;
        }
    }
    *out = '\0';
}

int nc_snprintf(char* buf, size_t size, const char* fmt, ...) {
    if (!buf || size == 0) return 0;
    va_list args;
    va_start(args, fmt);
    char* out = buf;
    size_t left = size;
    const char* p = fmt;
    while (*p && left > 1) {
        if (*p != '%') {
            *out++ = *p++;
            left--;
            continue;
        }
        p++;
        if (*p == '%') { *out++ = '%'; p++; left--; continue; }
        // flags/width (minimal: support %- and width digits for %d/%s)
        int pad_left = 0;
        while (*p == '-') { pad_left = 1; p++; }
        int width = 0;
        while (*p >= '0' && *p <= '9') { width = width * 10 + (*p - '0'); p++; }
        char tmp[64];
        int tmp_len = 0;
        if (*p == 'd' || *p == 'u' || *p == 'x' || *p == 'X') {
            unsigned int v;
            if (*p == 'd') {
                int sv = va_arg(args, int);
                if (sv < 0) { tmp[tmp_len++] = '-'; v = (unsigned int)(-sv); }
                else v = (unsigned int)sv;
            } else {
                v = va_arg(args, unsigned int);
            }
            char num[16];
            nc_utoa(v, num, (*p == 'x' || *p == 'X') ? 16 : 10, (*p == 'X'));
            for (char* q = num; *q; q++) tmp[tmp_len++] = *q;
            p++;
        } else if (*p == 's') {
            const char* s = va_arg(args, const char*);
            if (!s) s = "(null)";
            while (*s) tmp[tmp_len++] = *s++;
            p++;
        } else if (*p == 'c') {
            tmp[tmp_len++] = (char)va_arg(args, int);
            p++;
        } else if (*p == 'f') {
            double v = va_arg(args, double);
            int prec = 6;
            if (p[1] == '.') {
                p += 2;
                prec = 0;
                while (*p >= '0' && *p <= '9') { prec = prec * 10 + (*p - '0'); p++; }
                if (prec > 9) prec = 9;
            }
            nc_ftoa(v, tmp, prec);
            tmp_len = (int)nc_strlen(tmp);
            p++;
        } else {
            tmp[tmp_len++] = '%';
            tmp[tmp_len++] = *p;
            p++;
        }
        int pad = width - tmp_len;
        if (pad > 0 && !pad_left) {
            while (pad > 0 && left > 1) { *out++ = ' '; left--; pad--; }
        }
        for (int i = 0; i < tmp_len && left > 1; i++) { *out++ = tmp[i]; left--; }
        if (pad > 0 && pad_left) {
            while (pad > 0 && left > 1) { *out++ = ' '; left--; pad--; }
        }
    }
    va_end(args);
    *out = '\0';
    return (int)(out - buf);
}

} // extern "C"

// ── C++ operator new/delete (required before any C++ new/delete) ───────────
void* __cdecl operator new(unsigned int size) {
    return nc_malloc(size);
}
void __cdecl operator delete(void* ptr) {
    nc_free(ptr);
}
void* __cdecl operator new[](unsigned int size) {
    return nc_malloc(size);
}
void __cdecl operator delete[](void* ptr) {
    nc_free(ptr);
}

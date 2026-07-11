/*
 * nocrt.cpp — Implementation of minimal CRT functions using Win32 only.
 * No msvcrt.dll dependency.
 *
 * Extended for local_mods: adds nc_stricmp, nc_strcpy, nc_strcat, nc_strtok
 * for full string parsing without CRT dependencies.
 */
#include "nocrt.h"

extern "C" {

void* nc_malloc(size_t size) {
    return HeapAlloc(GetProcessHeap(), 0, size ? size : 1);
}

void* nc_realloc(void* ptr, size_t size) {
    if (!ptr) return HeapAlloc(GetProcessHeap(), 0, size ? size : 1);
    return HeapReAlloc(GetProcessHeap(), 0, ptr, size ? size : 1);
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

int nc_stricmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

char* nc_strncpy(char* dst, const char* src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = '\0';
    return dst;
}

char* nc_strcpy(char* dst, const char* src) {
    char* d = dst;
    while ((*d++ = *src++));
    return dst;
}

char* nc_strcat(char* dst, const char* src) {
    char* d = dst;
    while (*d) d++;
    while ((*d++ = *src++));
    return dst;
}

static char* g_strtok_pos = NULL;

char* nc_strtok(char* str, const char* delim) {
    if (str) g_strtok_pos = str;
    if (!g_strtok_pos || !*g_strtok_pos) return NULL;

    // Skip leading delimiters
    char* start = g_strtok_pos;
    while (*start) {
        bool isDelim = false;
        for (const char* d = delim; *d; d++) {
            if (*start == *d) { isDelim = true; break; }
        }
        if (!isDelim) break;
        start++;
    }
    if (!*start) { g_strtok_pos = start; return NULL; }

    // Find end of token
    char* end = start;
    while (*end) {
        bool isDelim = false;
        for (const char* d = delim; *d; d++) {
            if (*end == *d) { isDelim = true; break; }
        }
        if (isDelim) break;
        end++;
    }

    if (*end) {
        *end = '\0';
        g_strtok_pos = end + 1;
    } else {
        g_strtok_pos = end;
    }
    return start;
}

int nc_snprintf(char* buf, size_t size, const char* fmt, ...) {
    // Use wvsprintfA from user32.dll — no float support but no msvcrt needed
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

// DllMain — minimal, no CRT init
BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    return TRUE;
}

// Standard-named wrappers — MUST be static-linked into our DLL only.
// If these become global exports they override the game's msvcrt versions
// and cause crashes.
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
extern "C" int __cdecl stricmp(const char* a, const char* b) {
    return nc_stricmp(a, b);
}
extern "C" char* __cdecl strncpy(char* dst, const char* src, size_t n) {
    return nc_strncpy(dst, src, n);
}
extern "C" char* __cdecl strcpy(char* dst, const char* src) {
    return nc_strcpy(dst, src);
}
extern "C" char* __cdecl strcat(char* dst, const char* src) {
    return nc_strcat(dst, src);
}
extern "C" char* __cdecl strtok(char* str, const char* delim) {
    return nc_strtok(str, delim);
}
extern "C" int __cdecl memcmp(const void* a, const void* b, size_t count) {
    const unsigned char* pa = (const unsigned char*)a;
    const unsigned char* pb = (const unsigned char*)b;
    while (count--) {
        if (*pa != *pb) return (int)*pa - (int)*pb;
        pa++; pb++;
    }
    return 0;
}
extern "C" char* __cdecl strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == 0) ? (char*)s : NULL;
}
extern "C" char* __cdecl strrchr(const char* s, int c) {
    const char* last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    if (c == 0) return (char*)s;
    return (char*)last;
}
extern "C" char* __cdecl strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return NULL;
}
extern "C" void* __cdecl malloc(size_t size) {
    return nc_malloc(size);
}
extern "C" void* __cdecl realloc(void* ptr, size_t size) {
    return nc_realloc(ptr, size);
}
extern "C" void __cdecl free(void* ptr) {
    nc_free(ptr);
}

// C++ operator new/delete
void* __cdecl operator new(unsigned int size) {
    return nc_malloc(size);
}
void __cdecl operator delete(void* ptr) {
    nc_free(ptr);
}
void __cdecl operator delete(void* ptr, unsigned int) {
    nc_free(ptr);
}
void* __cdecl operator new[](unsigned int size) {
    return nc_malloc(size);
}
void __cdecl operator delete[](void* ptr) {
    nc_free(ptr);
}

// __chkstk_ms — required by MinGW when stack frame > 4096 bytes
extern "C" void __chkstk_ms(void) {
}

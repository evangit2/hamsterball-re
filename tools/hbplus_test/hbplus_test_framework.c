
#include "bass_proxy.h"
#include <stdbool.h>
#include <stdarg.h>

/* Fake IModAPI — just a raw vtable with no-op functions */
static void log_msg(const char* fmt, ...) {
    FILE* f = fopen("hbplus_test_log.txt", "a");
    if (f) { va_list a; va_start(a, fmt); vfprintf(f, fmt, a); va_end(a); fprintf(f, "\n"); fclose(f); }
}

/* All fake API functions are just no-ops */
static void __attribute__((cdecl)) noOpVoid() {}
static int noOpInt() { return 0; }
static void* noOpPtr() { return 0; }
static float noOpFloat() { return 0.0f; }

typedef void* (*CreateModFunct)();

/* Call a __thiscall function manually */
static void call_thiscall1(void* func, void* thisptr, void* arg) {
    /* __thiscall: this in ECX, arg on stack */
    void* (*wrapper)(void*, void*) = (void*(*)(void*,void*))func;
    /* On MinGW, __thiscall isn't available in C. Use assembly. */
    /* Actually, we can just cast and call — on 32-bit, the first param goes on stack */
    /* But __thiscall puts this in ECX. We need assembly. */
    __asm__ __volatile__(
        "push %1\n"
        "movl %0, %%ecx\n"
        "call *%2\n"
        "addl $4, %%esp\n"
        :
        : "r"(thisptr), "r"(arg), "r"(func)
        : "ecx", "esp", "memory"
    );
}

static const char* call_thiscall_retstr(void* func, void* thisptr) {
    const char* result = 0;
    __asm__ __volatile__(
        "movl %1, %%ecx\n"
        "call *%2\n"
        "movl %%eax, %0\n"
        : "=r"(result)
        : "r"(thisptr), "r"(func)
        : "eax", "ecx", "memory"
    );
    return result;
}

static int call_thiscall_retint(void* func, void* thisptr) {
    int result = 0;
    __asm__ __volatile__(
        "movl %1, %%ecx\n"
        "call *%2\n"
        "movl %%eax, %0\n"
        : "=r"(result)
        : "r"(thisptr), "r"(func)
        : "eax", "ecx", "memory"
    );
    return result;
}

static DWORD WINAPI modLoaderThread(LPVOID param) {
    log_msg("=== HB+ Test Framework Started ===");
    log_msg("Waiting 2s for game init...");
    Sleep(2000);
    log_msg("Scanning Mods/ folder...");

    CreateDirectoryA("Mods", NULL);

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("Mods\\*.dll", &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        log_msg("No mod DLLs found in Mods/ folder");
        log_msg("=== Test Complete ===");
        return 0;
    }

    do {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "Mods\\%s", findData.cFileName);
        log_msg("--- Loading: %s ---", path);

        HMODULE hMod = LoadLibraryA(path);
        if (!hMod) {
            log_msg("  FAILED LoadLibrary error=%d", (int)GetLastError());
            continue;
        }
        log_msg("  LoadLibrary OK handle=%p", hMod);

        CreateModFunct factory = (CreateModFunct)GetProcAddress(hMod, "CreateModInstance");
        if (!factory) {
            log_msg("  FAILED: no CreateModInstance export");
            continue;
        }
        log_msg("  CreateModInstance at %p", factory);

        /* Call CreateModInstance */
        void* mod = factory();
        if (!mod) {
            log_msg("  FAILED: CreateModInstance returned NULL");
            continue;
        }
        log_msg("  CreateModInstance OK mod=%p", mod);

        /* Read vtable pointer */
        void** vtable = *(void***)mod;
        if (vtable) {
            log_msg("  vtable=%p entries[0-5]=%p %p %p %p %p %p",
                vtable, vtable[0], vtable[1], vtable[2], vtable[3], vtable[4], vtable[5]);
        }

        /* Call GetModName — vtable[1] in MSVC layout */
        const char* name = call_thiscall_retstr(vtable[1], mod);
        log_msg("  GetModName(vtable[1]) = '%s'", name ? name : "(null)");

        /* Call GetApiVersion — vtable[3] in MSVC layout */
        int ver = call_thiscall_retint(vtable[3], mod);
        log_msg("  GetApiVersion(vtable[3]) = %d", ver);

        /* Call Initialize — vtable[5] in MSVC layout */
        /* Pass NULL as IModAPI (safe — most mods just store the pointer) */
        log_msg("  Calling Initialize(vtable[5])...");
        call_thiscall1(vtable[5], mod, NULL);
        log_msg("  Initialize OK");

        log_msg("--- SUCCESS ---");

    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    log_msg("=== All mods processed ===");
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        /* Truncate log */
        FILE* f = fopen("hbplus_test_log.txt", "w");
        if (f) fclose(f);
        CreateThread(NULL, 0, modLoaderThread, NULL, 0, NULL);
    }
    return TRUE;
}

/*
 * WobblyBridgeFix.c — Fix invisible wavy bridge in Wobbly race (Level 7)
 *
 * Root cause: Wavy_Update (0x440390) does VertexBuffer::Lock with
 * D3DLOCK_DISCARD (0x2000) at 0x440752: PUSH 0x2000.
 * The VB was created without D3DUSAGE_DYNAMIC, so DISCARD fails on strict
 * drivers (Intel UHD, Wine wined3d/dxvk) → Lock returns error → bridge
 * invisible but collision still works.
 *
 * Fix: patch PUSH 0x2000 → PUSH 0 (flags=0, plain lock). Works on all
 * drivers; negligible perf cost (one 342-vertex buffer every 3 frames).
 *
 * This is a bass.dll proxy — game loads it as bass.dll. Ships as
 * WobblyBridgeFix.dll (same binary, rename allowed).
 */
#include "bass_proxy.h"

#define WAVY_LOCK_PUSH_ADDR 0x00440752
#define WAVY_LOCK_FLAG_BYTE_ADDR 0x00440754

static void fix_log(const char *msg) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)fix_log, &hSelf);
    GetModuleFileNameA(hSelf, path, MAX_PATH);
    char *p = strrchr(path, '\\');
    if (p) strcpy(p+1, "WobblyBridgeFix.log");
    else strcpy(path, "WobblyBridgeFix.log");
    FILE *f = fopen(path, "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

static void apply_wavy_fix(void) {
    BYTE orig = *(BYTE*)WAVY_LOCK_FLAG_BYTE_ADDR;
    if (orig == 0x20) {
        DWORD old;
        VirtualProtect((void*)WAVY_LOCK_PUSH_ADDR, 5, PAGE_EXECUTE_READWRITE, &old);
        *(BYTE*)WAVY_LOCK_FLAG_BYTE_ADDR = 0x00;
        VirtualProtect((void*)WAVY_LOCK_PUSH_ADDR, 5, old, &old);
        FlushInstructionCache(GetCurrentProcess(), (void*)WAVY_LOCK_PUSH_ADDR, 5);
        fix_log("[WobblyBridgeFix] Patched Wavy Lock DISCARD(0x2000)->0 at 0x440752");
    } else if (orig == 0x00) {
        fix_log("[WobblyBridgeFix] Already patched");
    } else {
        char buf[128];
        wsprintfA(buf, "[WobblyBridgeFix] Unexpected byte 0x%02X at 0x440754", orig);
        fix_log(buf);
    }
}

static DWORD WINAPI init_thread(LPVOID param) {
    Sleep(500);
    apply_wavy_fix();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
    }
    return TRUE;
}

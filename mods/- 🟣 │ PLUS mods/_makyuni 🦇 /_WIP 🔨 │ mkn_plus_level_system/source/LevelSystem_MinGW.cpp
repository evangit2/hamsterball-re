/*
 * LevelSystem_MinGW.cpp — Extended Vtables + Custom Dispatch (HB+ port)
 *
 * Ported from mkn_level_system v5 bass.dll proxy mod.
 * Patches all 15 level allocations to MAX_STRUCT_SIZE, creates extended
 * vtables (128 entries), and dispatches custom per-frame functions.
 *
 * Uses standard CRT (not nocrt) due to sscanf/strtoul/atof requirements.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* ============================================================
 * Level table
 * ============================================================ */

#define MAX_STRUCT_SIZE 0x6498

typedef struct {
    int level_num;
    const char* name;
    DWORD ctor_addr;
    DWORD struct_size;
    DWORD push_addr;
    DWORD call_addr;
    DWORD vtable_addr;
    DWORD ctor_vt_patch;
} LevelEntry;

static const LevelEntry g_levels[15] = {
    { 1,  "WarmUp",       0x0041CA40, 0x436C, 0x00427109, 0x0042712C, 0x004D04A8, 0x0041CA73 },
    { 2,  "Beginner",     0x004200E0, 0x644C, 0x00427136, 0x0042715D, 0x004D1098, 0x0042012B },
    { 3,  "Intermediate", 0x0041CB20, 0x438C, 0x00427167, 0x0042718E, 0x004D05A0, 0x0041CB57 },
    { 4,  "Dizzy",        0x0041D060, 0x4BE0, 0x00427198, 0x004271BF, 0x004D0890, 0x0041D09B },
    { 5,  "Tower",        0x0041E340, 0x5418, 0x004271C9, 0x004271F0, 0x004D0A08, 0x0041E379 },
    { 6,  "Up",           0x00420390, 0x4790, 0x004271FA, 0x00427221, 0x004D11A0, 0x004203CC },
    { 7,  "Neon",         0x00424440, 0x4394, 0x0042722B, 0x00427252, 0x004D1DF0, 0x00424481 },
    { 8,  "Expert",       0x0041EA40, 0x4FD8, 0x0042725C, 0x00427283, 0x004D0B00, 0x0041EA7B },
    { 9,  "Odd",          0x0041ED80, 0x43B0, 0x0042728D, 0x004272B4, 0x004D0BC0, 0x0041EDB1 },
    { 10, "Toob",         0x0041F4B0, 0x646C, 0x004272BE, 0x004272E5, 0x004D0E78, 0x0041F4FB },
    { 11, "Wobbly",       0x0041F110, 0x4388, 0x004272EF, 0x00427316, 0x004D0D38, 0x0041F144 },
    { 12, "Glass",        0x00424A90, 0x4390, 0x00427320, 0x00427347, 0x004D1F90, 0x00424AD3 },
    { 13, "Sky",          0x0041F930, 0x47F8, 0x00427351, 0x00427374, 0x004D0FC8, 0x0041F96A },
    { 14, "Master",       0x004206D0, 0x6498, 0x0042737B, 0x0042739E, 0x004D12B0, 0x0042071C },
    { 15, "Impossible",   0x00424C20, 0x4380, 0x004273A5, 0x004273C8, 0x004D21C0, 0x00424C5D },
};

#define VTABLE_SIZE 512
#define VTABLE_ENTRIES 128
#define VTABLE_NATIVE_SLOTS 36
#define VTABLE_EXT_SLOTS 92

static DWORD g_custom_vtables[15] = {0};
static int g_vtables_initialized = 0;
static DWORD g_orig_vtable[15][36] = {{0}};
static int g_vtable_saved = 0;

static void* g_storedApi = NULL;

/* Patch helpers */
static void patch_byte(DWORD addr, BYTE val) {
    DWORD old;
    VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)addr = val;
    VirtualProtect((void*)addr, 1, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, 1);
}

static void patch_bytes(DWORD addr, const void* data, SIZE_T len) {
    DWORD old;
    VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy((void*)addr, data, len);
    VirtualProtect((void*)addr, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, len);
}

static void patch_dword(DWORD addr, DWORD val) {
    patch_bytes(addr, &val, 4);
}

static void patch_float(DWORD addr, float val) {
    patch_bytes(addr, &val, 4);
}

static void patch_jmp(DWORD patchAddr, DWORD target) {
    DWORD offset = target - (patchAddr + 5);
    patch_byte(patchAddr, 0xE9);
    patch_dword(patchAddr + 1, offset);
}

static void patch_nop(DWORD addr, SIZE_T count) {
    for (SIZE_T i = 0; i < count; i++)
        patch_byte(addr + i, 0x90);
}

#define GLOBAL_APP_PTR 0x005341E0

/* ============================================================
 * Custom vtable initialization
 * ============================================================ */

static void init_custom_vtables(void) {
    if (g_vtables_initialized) return;
    for (int i = 0; i < 15; i++) {
        DWORD addr = (DWORD)VirtualAlloc(NULL, VTABLE_SIZE,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!addr) continue;
        if (!IsBadReadPtr((void*)g_levels[i].vtable_addr, 144)) {
            memcpy((void*)addr, (void*)g_levels[i].vtable_addr, 144);
        }
        if (g_vtable_saved == 0) {
            memcpy(g_orig_vtable[i], (void*)g_levels[i].vtable_addr, 144);
        }
        g_custom_vtables[i] = addr;
    }
    g_vtable_saved = 1;
    g_vtables_initialized = 1;
}

/* Patch struct sizes */
static void patch_struct_sizes(void) {
    for (int i = 0; i < 15; i++) {
        if (g_levels[i].struct_size >= MAX_STRUCT_SIZE) continue;
        DWORD patchAddr = g_levels[i].push_addr + 1;
        patch_dword(patchAddr, MAX_STRUCT_SIZE);
    }
}

/* Patch ctor vtable pointers */
static void patch_ctor_vtables(void) {
    for (int i = 0; i < 15; i++) {
        if (!g_custom_vtables[i]) continue;
        patch_dword(g_levels[i].ctor_vt_patch + 2, g_custom_vtables[i]);
    }
}

/* ============================================================
 * Vtable slot access
 * ============================================================ */

static int set_vtable_entry(int level, int slot, DWORD addr) {
    if (level < 1 || level > 15) return 0;
    if (slot < 0 || slot >= VTABLE_ENTRIES) return 0;
    if (!g_custom_vtables[level-1]) return 0;
    DWORD* vt = (DWORD*)g_custom_vtables[level-1];
    vt[slot] = addr;
    return 1;
}

static DWORD get_vtable_entry(int level, int slot) {
    if (level < 1 || level > 15) return 0;
    if (slot < 0 || slot >= VTABLE_ENTRIES) return 0;
    if (!g_custom_vtables[level-1]) return 0;
    return ((DWORD*)g_custom_vtables[level-1])[slot];
}

static void swap_vtable_entry(int levelA, int levelB, int slot) {
    DWORD a = get_vtable_entry(levelA, slot);
    DWORD b = get_vtable_entry(levelB, slot);
    set_vtable_entry(levelA, slot, b);
    set_vtable_entry(levelB, slot, a);
}

static int lookup_level(const char* name) {
    if (!name) return 0;
    for (int i = 0; i < 15; i++) {
        if (_stricmp(name, g_levels[i].name) == 0) return i + 1;
    }
    return 0;
}

/* ============================================================
 * Config parser (simplified — VTABLE commands only)
 * ============================================================ */

static void parse_config_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        if (_strnicmp(p, "VTABLE ", 7) == 0) {
            char tokLevel[64] = "", tokSlot[64] = "", tokFunc[128] = "";
            if (sscanf(p + 7, "%63s %63s %127s", tokLevel, tokSlot, tokFunc) >= 2) {
                int a = lookup_level(tokLevel);
                int slot = atoi(tokSlot);
                if (a && slot >= 0 && slot < VTABLE_ENTRIES) {
                    DWORD addr = 0;
                    if (tokFunc[0] == '0' && (tokFunc[1] == 'x' || tokFunc[1] == 'X')) {
                        addr = (DWORD)strtoul(tokFunc, NULL, 16);
                    } else if (tokFunc[0] >= '0' && tokFunc[0] <= '9') {
                        addr = (DWORD)strtoul(tokFunc, NULL, 10);
                    }
                    if (addr) set_vtable_entry(a, slot, addr);
                }
            }
        }
        else if (_strnicmp(p, "SWAPGEO ", 8) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(p + 8, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    swap_vtable_entry(a, b, 18);
                }
            }
        }
    }
    fclose(f);
}

static void load_config(void) {
    /* Config file next to DLL */
    char dllPath[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&load_config, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, dllPath, MAX_PATH) > 0) {
        char* last = strrchr(dllPath, '\\');
        if (last) {
            strcpy(last + 1, "mkn_level_functions.txt");
            parse_config_file(dllPath);
        }
    }
    /* Also try game directory */
    parse_config_file("mkn_level_functions.txt");
}

/* ============================================================
 * Dispatch (runs in onGameUpdate)
 * ============================================================ */

static int g_lastBoard = 0;

static void run_dispatch(void) {
    if (!g_vtables_initialized) return;
    if (IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) return;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000) return;
    if (IsBadReadPtr((void*)(app + 0x178), 4)) return;
    DWORD board = *(DWORD*)(app + 0x178);
    if (!board || board < 0x10000) return;
    if (IsBadReadPtr((void*)board, 4)) return;
    DWORD board_vt = *(DWORD*)board;
    if (!board_vt || board_vt < 0x10000) return;

    /* Find which level's custom vtable matches */
    for (int i = 0; i < 15; i++) {
        if (g_custom_vtables[i] == board_vt) {
            /* Call extended slots 36-127 */
            DWORD* vt = (DWORD*)g_custom_vtables[i];
            for (int s = VTABLE_NATIVE_SLOTS; s < VTABLE_ENTRIES; s++) {
                if (vt[s] != 0 && !IsBadReadPtr((void*)vt[s], 4)) {
                    typedef void (__thiscall *SlotFunc)(DWORD);
                    ((SlotFunc)vt[s])(board);
                }
            }
            break;
        }
    }
}

/* ============================================================
 * HB+ Vtable
 * ============================================================ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Level System"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;

    init_custom_vtables();
    patch_struct_sizes();
    patch_ctor_vtables();
    load_config();
}

static void __thiscall game_update_impl(void*) {
    run_dispatch();
}

static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall scene_end_impl(void*) {}
static void __thiscall level_start_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor, (void*)get_mod_name, (void*)get_author, (void*)get_version,
    (void*)get_contributors, (void*)init_impl, (void*)ball_update_impl,
    (void*)render_apply_impl, (void*)button_toggle_impl, (void*)slider_change_impl,
    (void*)cycle_change_impl, (void*)game_update_impl, (void*)event_collide_impl,
    (void*)text_render_impl, (void*)ball_bump_impl, (void*)scene_end_impl,
    (void*)level_start_impl,
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(GetModuleHandle(NULL));
    return TRUE;
}

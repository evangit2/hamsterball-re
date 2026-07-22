/*
 * mkn_plus_bumper_quantity.cpp
 *
 * Increases the hardcoded bumper count limit for each level that has bumpers.
 *
 * The game hardcodes bumper limits in level-init functions as:
 *   CMP EAX, <count>   (83 F8 <byte>)
 * followed by JL to continue the loop.
 *
 * The loop iterates i=0..N-1, calling AthenaString_Format("N:BUMPER%d", i)
 * and Scene_CollectByNameFilter to find the meshbuffer. The collision handler
 * uses atol(name+8) to get the bumper number and writes a flag at
 * board[flagOffset + num*4]. There is NO bounds check in the collision
 * handler — it trusts the level init to have found all meshbuffers.
 *
 * Patch points (verified from binary, bytes 83 F8 XX):
 *
 *   Level             Address     Current  Patch Byte
 *   ─────────────────────────────────────────────────
 *   Beginner Race     0x004111C5    8       config (default 16)
 *   Toob Race         0x0040FB06    8       config (default 16)
 *   Master Race       0x00412047    4       config (default 16)
 *   Arena Beginner    0x00413DC6    4       config (default 16)
 *   Arena Toob        0x00414FE6    5       config (default 16)
 *
 * Only the IMMEDIATE BYTE (offset+2) is patched. The CMP opcode (83 F8) stays.
 * Max value: 127 (signed byte range for CMP EAX, imm8).
 *
 * Config file: mkn_plus_bumper_quantity.txt next to DLL
 * Format: one number per line (the new bumper limit, applied to all levels)
 * Default: 16
 *
 * HB+ v2.1, MinGW manual vtable (17 entries).
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* ---- Patch points ---- */
typedef struct {
    DWORD address;       /* address of the immediate byte (CMP EAX, imm8 + 2) */
    BYTE  originalValue;  /* original limit */
    const char* name;
} BumperPatch;

static BumperPatch g_patches[] = {
    { 0x004111C5, 8, "Beginner Race" },
    { 0x0040FB06, 8, "Toob Race" },
    { 0x00412047, 4, "Master Race" },
    { 0x00413DC6, 4, "Arena Beginner" },
    { 0x00414FE6, 5, "Arena Toob" },
};
static const int NUM_PATCHES = 5;

/* ---- State ---- */
static void* g_storedApi = NULL;
static int  g_bumperLimit = 16;      /* default */
static bool g_patchesApplied = false;
static char g_configPath[MAX_PATH] = "";
static bool g_pathReady = false;

/* Forward declaration for VirtualQuery */
static void* __thiscall sc_dtor(void* thisptr, int flags);

/* ---- Config loader ---- */
static void buildConfigPath(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)sc_dtor, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) {
                if (*p == '\\' || *p == '/') last = p;
                p++;
            }
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_configPath, dllPath, MAX_PATH - 1);
                nc_strncpy(g_configPath + nc_strlen(g_configPath),
                           "mkn_plus_bumper_quantity.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "mkn_plus_bumper_quantity.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_pathReady = true;
}

static void reloadConfig(void) {
    if (!g_pathReady) return;
    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    char buf[256];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    /* Skip BOM */
    char* p = buf;
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) p += 3;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p == '#') return; /* comment-only */
    if (*p == '\0') return;

    int val = 0;
    int hadDigits = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
        hadDigits = 1;
    }
    if (!hadDigits) return;
    if (val < 1) val = 1;
    if (val > 127) val = 127; /* CMP EAX, imm8 max */
    g_bumperLimit = val;
}

/* ---- Patching ---- */
static void applyPatches(void) {
    if (g_patchesApplied) return;
    for (int i = 0; i < NUM_PATCHES; i++) {
        DWORD addr = g_patches[i].address;
        DWORD oldProtect = 0;
        if (VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *(BYTE*)addr = (BYTE)g_bumperLimit;
            VirtualProtect((void*)addr, 1, oldProtect, &oldProtect);
        }
    }
    g_patchesApplied = true;
}

static void revertPatches(void) {
    if (!g_patchesApplied) return;
    for (int i = 0; i < NUM_PATCHES; i++) {
        DWORD addr = g_patches[i].address;
        DWORD oldProtect = 0;
        if (VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *(BYTE*)addr = g_patches[i].originalValue;
            VirtualProtect((void*)addr, 1, oldProtect, &oldProtect);
        }
    }
    g_patchesApplied = false;
}

/* ---- Vtable functions ---- */
static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Bumper Quantity"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    buildConfigPath();
    reloadConfig();
}

/* onGameUpdate: apply patches once, re-read config for live editing */
static void __thiscall game_update_impl(void* thisptr) {
    /* Re-read config every frame for live editing */
    int oldLimit = g_bumperLimit;
    reloadConfig();

    /* If limit changed and patches were applied, revert and re-apply */
    if (g_patchesApplied && oldLimit != g_bumperLimit) {
        revertPatches();
    }

    if (!g_patchesApplied) {
        applyPatches();
    }
}

/* onLevelStart: nothing special, patches persist */
static void __thiscall level_start_impl(void*) {}

/* onSceneEnd: nothing special */
static void __thiscall scene_end_impl(void*) {}

/* No-op callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

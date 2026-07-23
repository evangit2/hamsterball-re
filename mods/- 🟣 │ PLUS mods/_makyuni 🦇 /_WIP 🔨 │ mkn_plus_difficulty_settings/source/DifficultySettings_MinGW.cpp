/*
 * DifficultySettings_MinGW.cpp — Difficulty-based level file redirect (HB+ v2.1, MinGW)
 *
 * Hooks LoadMeshWorld (0x0045DE30) to redirect level files based on difficulty.
 * NOPs difficulty gates in Board_Setup so entities always spawn.
 *
 * Ported from bass.dll proxy mod to HB+ format.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#define APP_DIFFICULTY_OFFSET  0x23C
#define APP_PROFILE_OFFSET     0x220
#define PROFILE_PARTY_FLAG     0x10
#define PROFILE_PRACTICE_FLAG  0x11
#define LOADMESHWORLD_ADDR     0x0045DE30
#define GATE_CREATEBADBALLS    0x0041C9E4
#define GATE_CREATEMOUSETRAP   0x0041CA05

static void* g_storedApi = NULL;

/* Tournament detection */
static int is_tournament_mode(void) {
    if (!g_storedApi) return 0;
    HBPlusAPI hb = { g_storedApi };
    App* app = hb.GetApp();
    if (!app) return 0;
    DWORD appAddr = (DWORD)app;
    if (IsBadReadPtr((void*)(appAddr + APP_PROFILE_OFFSET), 4)) return 0;
    DWORD profile = *(DWORD*)(appAddr + APP_PROFILE_OFFSET);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_PARTY_FLAG), 1)) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_PRACTICE_FLAG), 1)) return 0;
    if (*(BYTE*)(profile + PROFILE_PARTY_FLAG) != 0) return 0;
    if (*(BYTE*)(profile + PROFILE_PRACTICE_FLAG) != 0) return 0;
    return 1;
}

static int get_difficulty(void) {
    if (!g_storedApi) return 1;
    HBPlusAPI hb = { g_storedApi };
    App* app = hb.GetApp();
    if (!app) return 1;
    DWORD appAddr = (DWORD)app;
    if (IsBadReadPtr((void*)(appAddr + APP_DIFFICULTY_OFFSET), 4)) return 1;
    int diff = *(int*)(appAddr + APP_DIFFICULTY_OFFSET);
    if (diff < 0 || diff > 2) return 1;
    return diff;
}

static int redirect_level_name(const char *original, char *outBuf, int bufSize) {
    int diff = get_difficulty();
    int tournament = is_tournament_mode();
    const char *suffix = NULL;

    if (tournament) {
        switch (diff) {
            case 0:  suffix = "-easy"; break;
            case 1:  suffix = NULL;   break;
            case 2:  suffix = "-hard"; break;
            default: suffix = NULL;   break;
        }
    } else {
        suffix = "-hard";
    }
    if (!suffix) return 0;

    int origLen = nc_strlen(original);
    int suffixLen = nc_strlen(suffix);
    if (origLen + suffixLen + 1 > bufSize) return 0;

    if (origLen >= 5) {
        const char* end = original + origLen - 5;
        if (nc_strcmp(end, "-easy") == 0) return 0;
        if (nc_strcmp(end, "-hard") == 0) return 0;
    }

    /* Skip arena files */
    if (origLen >= 12) {
        char lower[13];
        for (int i = 0; i < 12 && original[i]; i++) {
            lower[i] = (original[i] >= 'A' && original[i] <= 'Z') ? original[i] + 32 : original[i];
        }
        lower[12] = '\0';
        if (nc_strcmp(lower, "levels\\arena") == 0 || nc_strcmp(lower, "levels/arena") == 0) return 0;
    }

    /* Build redirected name */
    nc_memcpy(outBuf, original, origLen);
    nc_memcpy(outBuf + origLen, suffix, suffixLen + 1);

    /* Check if redirected .meshworld file exists */
    char checkPath[MAX_PATH];
    nc_snprintf(checkPath, MAX_PATH, "%s.meshworld", outBuf);
    checkPath[MAX_PATH - 1] = '\0';
    if (GetFileAttributesA(checkPath) == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    return 1;
}

/* Detour hook for LoadMeshWorld */
typedef void (__fastcall *LoadMeshWorld_t)(void *this_, void *edx, char *levelName);
static LoadMeshWorld_t g_origLoadMeshWorld = NULL;
static char g_redirectedName[MAX_PATH];

void __fastcall hook_LoadMeshWorld(void *this_, void *edx, char *levelName) {
    char redirected[MAX_PATH];
    if (redirect_level_name(levelName, redirected, MAX_PATH)) {
        nc_strncpy(g_redirectedName, redirected, MAX_PATH - 1);
        g_redirectedName[MAX_PATH - 1] = '\0';
        levelName = g_redirectedName;
    }
    g_origLoadMeshWorld(this_, edx, levelName);
}

static void install_loadmeshworld_hook(void) {
    DWORD target = LOADMESHWORLD_ADDR;
    unsigned char *src = (unsigned char*)target;
    void *trampMem = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!trampMem) return;
    unsigned char *tramp = (unsigned char*)trampMem;

    DWORD old;
    VirtualProtect(src, 16, PAGE_EXECUTE_READWRITE, &old);
    nc_memcpy(tramp, src, 8);
    tramp[8] = 0xE9;
    DWORD back = (target + 8) - ((DWORD)(tramp + 8) + 5);
    *(DWORD*)(tramp + 9) = back;
    VirtualProtect(src, 16, old, &old);
    FlushInstructionCache(GetCurrentProcess(), src, 16);

    g_origLoadMeshWorld = (LoadMeshWorld_t)tramp;

    VirtualProtect(src, 8, PAGE_EXECUTE_READWRITE, &old);
    src[0] = 0xE9;
    DWORD hookAddr = (DWORD)hook_LoadMeshWorld;
    DWORD offset = hookAddr - (target + 5);
    *(DWORD*)(src + 1) = offset;
    src[5] = 0x90; src[6] = 0x90; src[7] = 0x90;
    VirtualProtect(src, 8, old, &old);
    FlushInstructionCache(GetCurrentProcess(), src, 8);
}

static void patch_difficulty_gates(void) {
    DWORD old;
    VirtualProtect((void*)GATE_CREATEBADBALLS, 2, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)GATE_CREATEBADBALLS = 0x90;
    *(BYTE*)(GATE_CREATEBADBALLS + 1) = 0x90;
    VirtualProtect((void*)GATE_CREATEBADBALLS, 2, old, &old);

    VirtualProtect((void*)GATE_CREATEMOUSETRAP, 2, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)GATE_CREATEMOUSETRAP = 0x90;
    *(BYTE*)(GATE_CREATEMOUSETRAP + 1) = 0x90;
    VirtualProtect((void*)GATE_CREATEMOUSETRAP, 2, old, &old);

    FlushInstructionCache(GetCurrentProcess(), (void*)GATE_CREATEBADBALLS, 2);
    FlushInstructionCache(GetCurrentProcess(), (void*)GATE_CREATEMOUSETRAP, 2);
}

/* HB+ Vtable */
static void* __thiscall sc_dtor(void* thisptr, int flags) { if (flags & 1) nc_free(thisptr); return thisptr; }
static const char* __thiscall get_mod_name(void*) { return "Difficulty Settings"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    patch_difficulty_gates();
    install_loadmeshworld_hook();
}

static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall game_update_impl(void*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor, (void*)get_mod_name, (void*)get_author, (void*)get_version,
    (void*)get_contributors, (void*)init_impl, (void*)ball_update_impl,
    (void*)render_apply_impl, (void*)button_toggle_impl, (void*)slider_change_impl,
    (void*)cycle_change_impl, (void*)game_update_impl, (void*)event_collide_impl,
    (void*)text_render_impl, (void*)ball_bump_impl, (void*)scene_end_impl,
    (void*)level_start_impl,
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

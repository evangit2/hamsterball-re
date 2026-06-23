/*
 * universal_ref_loader.c — Hamsterball DLL Mod v2
 *
 * Hooks the vtable[33] factory dispatch in Scene_CreateDynamicObjects.
 * When the original Board factory doesn't recognize a ref name, tries all
 * other Arena factories in sequence. Features:
 *
 *   - Clone-on-return for static-mesh objects (WATERWHEEL, SWIRL, BRIDGE-base)
 *     via Level_CloneTree (0x466060) for multi-instance support
 *   - Difficulty gate bypass (temporarily sets App+0x23C = 1)
 *   - Board slot safety checks (prevents crashes from unloaded meshes)
 *
 * Hook point: 0x0040C4BA (CALL [EAX+0x84] → CALL universal_factory)
 *   Original: FF 90 84 00 00 00  (6 bytes)
 *   Patched:  E8 XX XX XX XX 90  (CALL rel32 + NOP)
 *
 * All addresses verified via GhidraMCP decompilation + vtable reads (June 2026).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll universal_ref_loader.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ============================================================
 * Game constants (all verified via GhidraMCP)
 * ============================================================ */

#define HOOK_CALL_SITE     0x0040C4BA
#define HOOK_CALL_SITE_LEN 6

#define LEVEL_CLONE_TREE   0x00466060
#define APP_SINGLETON      0x004FD680
#define APP_DIFFICULTY    0x23C
#define BOARD_APP_PTR     0x878

/* Arena factory addresses (verified via vtable[33] reads) */
#define FACTORY_MASTER      0x004121D0  /* BRIDGE,TIPPER,BONK,BBRIDGE1-2,POPCYLINDER,BLOCKDAWG1-2,CATAPULT,GLUEBIE */
#define FACTORY_TOWER       0x0040D7C0  /* CATAPULT,MACE,DRAWBRIDGE,WINDMILL,TRAPDOOR,CHOMPER,TURRET,BONK,FAN,SAWBLADE,BRIDGE,JUDGE,BELL */
#define FACTORY_IMPOSSIBLE  0x00417FE0  /* LOOPER,GEAR,BIGGEAR,ROTATOR,PENDULUM */
#define FACTORY_EXPERT      0x0040E250  /* BONK,FAN,SAWBLADE,BRIDGE,JUDGE,BELL (self-loading, no mesh deps) */
#define FACTORY_UP          0x004117B0  /* LIFTER,SPEEDCYLINDER,TIMEBUTTON */
#define FACTORY_DIZZY       0x0040A5F0  /* TIPPER,WATERWHEEL,SWIRL,GLUEBIE */
#define FACTORY_BEGINNER    0x0040A550  /* BRIDGE (base, static return) */
#define FACTORY_WOBBLY      0x0040AD80  /* SMASHER1,SMASHER2 (configuring) */
#define FACTORY_NEON        0x00416910  /* NEONPLATFORM,DFLOOR,TRODE,etc */
#define FACTORY_ODD         0x0040EC40  /* WOBBLY,WAVY,SPINNY,etc */
#define FACTORY_TOOB        0x0040FB30  /* SPINNY,FALLOUT,BLOCKDAWG,PILLAR,etc */
#define FACTORY_GLASS       0x0040F420  /* similar to Odd */
#define FACTORY_SKY         0x00410AD0  /* POPCYLINDER,TRAPDOOR,N:BUMPER,etc */

/* ============================================================
 * Types
 * ============================================================ */

typedef void (__thiscall *FactoryFunc)(void* board, char* refName,
    void** outObj, void** outCol, int* refEntry);

typedef void* (__thiscall *CloneTreeFn)(void* mesh, int board);

/* Factory entry: function pointer + board offsets that must be non-NULL */
typedef struct {
    FactoryFunc func;
    int slots[14];  /* board+0x4xxx offsets that must be non-NULL, 0 = end */
    const char* name;
} SafeFactory;

/* ============================================================
 * Globals
 * ============================================================ */

static BYTE  g_origBytes[6];
static BOOL  g_hooked = FALSE;
static CloneTreeFn g_cloneTree = (CloneTreeFn)LEVEL_CLONE_TREE;

/* ============================================================
 * Factory table
 *
 * Each entry lists ALL board+0x4xxx offsets the factory READS.
 * If any slot is NULL, the factory is skipped (safety check).
 * Factories that don't read any slots (self-loading) have empty lists.
 *
 * Order: most inclusive factories first (handle the most ref types).
 * Expert and Wobbly have no mesh deps, so they're tried early.
 * ============================================================ */

static SafeFactory g_factories[] = {
    /* Expert Arena: BONK,FAN,SAWBLADE,BRIDGE,JUDGE,BELL — all self-loading or procedural */
    { (FactoryFunc)FACTORY_EXPERT,     {0}, "Expert" },

    /* Wobbly Arena: SMASHER1,SMASHER2 — configuring only, no mesh reads */
    { (FactoryFunc)FACTORY_WOBBLY,     {0}, "Wobbly" },

    /* Master Arena: BRIDGE,TIPPER,BONK,BBRIDGE1-2,POPCYLINDER,BLOCKDAWG1-2,CATAPULT,GLUEBIE */
    { (FactoryFunc)FACTORY_MASTER,     {0x436C, 0x4370, 0x4394, 0x4398,
                                       0x5410, 0x5414, 0x5420,
                                       0x5840, 0x5844, 0x5848, 0x607C, 0}, "Master" },

    /* Tower Arena: CATAPULT,MACE,DRAWBRIDGE,WINDMILL,TRAPDOOR,CHOMPER,TURRET */
    { (FactoryFunc)FACTORY_TOWER,      {0x436C, 0x4370, 0x4378, 0x437C, 0x43B4, 0}, "Tower" },

    /* Impossible Arena: LOOPER,GEAR,BIGGEAR,ROTATOR,PENDULUM */
    { (FactoryFunc)FACTORY_IMPOSSIBLE, {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, 0}, "Impossible" },

    /* Up Arena: LIFTER,SPEEDCYLINDER,TIMEBUTTON */
    { (FactoryFunc)FACTORY_UP,         {0x4784, 0x4788, 0x478C, 0}, "Up" },

    /* Dizzy Arena: TIPPER,WATERWHEEL,SWIRL,GLUEBIE */
    { (FactoryFunc)FACTORY_DIZZY,      {0x436C, 0x4370, 0x4374,
                                       0x4BA8, 0x4BAC, 0x4BC4, 0x4BC8, 0}, "Dizzy" },

    /* Beginner Arena: BRIDGE (base, static return) */
    { (FactoryFunc)FACTORY_BEGINNER,   {0x436C, 0x4370, 0}, "Beginner" },

    /* Neon Arena: NEONPLATFORM,DFLOOR,TRODE,etc */
    { (FactoryFunc)FACTORY_NEON,       {0x4374, 0x4378, 0x437C, 0x4380, 0x4384, 0x4388, 0}, "Neon" },

    /* Odd Arena: WOBBLY,WAVY,SPINNY,etc */
    { (FactoryFunc)FACTORY_ODD,        {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, 0}, "Odd" },

    /* Toob Arena: SPINNY,FALLOUT,BLOCKDAWG,PILLAR,etc */
    { (FactoryFunc)FACTORY_TOOB,       {0x436C, 0x4370, 0x4374, 0x4378, 0x437C,
                                       0x4380, 0x4384, 0}, "Toob" },

    /* Glass Arena: similar to Odd */
    { (FactoryFunc)FACTORY_GLASS,      {0x436C, 0x4370, 0x4374, 0x4378, 0x437C,
                                       0x4380, 0x4384, 0}, "Glass" },

    /* Sky Arena: POPCYLINDER,TRAPDOOR,N:BUMPER,etc */
    { (FactoryFunc)FACTORY_SKY,        {0x436C, 0x4374, 0x4378, 0x437C, 0x4380,
                                       0x438C, 0x4390, 0}, "Sky" },
};

#define NUM_FACTORIES (sizeof(g_factories) / sizeof(g_factories[0]))

/* ============================================================
 * Static-mesh detection
 *
 * These objects return a board slot pointer directly (no alloc,
 * no constructor). Multiple refs all point to the same object,
 * so only one renders (at the last position). We clone them
 * via Level_CloneTree for multi-instance support.
 *
 * Verified from decompiled factory code:
 *   WATERWHEEL → returns board+0x4BA8 (Dizzy Arena)
 *   SWIRL      → returns board+0x4BC4 (Dizzy Arena)
 *   BRIDGE     → returns board+0x436C (Beginner/Master Arena)
 *
 * WINDMILL also returns a static mesh (board+0x437C) but additionally
 * creates a CollisionLevel and attaches it — more complex, skip for now.
 * CHOMPER and SMASHER1/2 don't return objects (configuring only).
 * ============================================================ */

static int is_static_mesh_object(const char* refName)
{
    if (_strnicmp(refName, "WATERWHEEL", 10) == 0) return 1;
    if (_strnicmp(refName, "SWIRL", 5) == 0) return 1;
    if (_strnicmp(refName, "BRIDGE", 6) == 0) return 1;
    /* WINDMILL: returns static mesh + creates collision. Complex — skip for now. */
    return 0;
}

/* ============================================================
 * Difficulty-gated object detection
 *
 * These objects only spawn when App+0x23C != 0 (Normal or Frenzied).
 * The universal factory temporarily sets App+0x23C = 1 to bypass the gate.
 *
 * Verified from decompiled factory code:
 *   TIPPER:     *(int*)(App+0x23C) != 0  (Master Arena, Dizzy Arena)
 *   BONK:       *(int*)(App+0x23C) != 0  (Master Arena, Expert Arena)
 *   BLOCKDAWG1: *(int*)(App+0x23C) != 0  (Master Arena)
 *   BLOCKDAWG2: *(int*)(App+0x23C) != 0  (Master Arena)
 *   GLUEBIE:    *(int*)(App+0x23C) != 0  (Master Arena, Dizzy Arena)
 *   FAN:        *(int*)(App+0x23C) != 0  (Expert Arena)
 *   SAWBLADE:   *(int*)(App+0x23C) != 0  (Expert Arena)
 *   MACE:       *(int*)(App+0x23C) != 0  (Tower Arena)
 * ============================================================ */

static int is_difficulty_gated(const char* refName)
{
    if (_strnicmp(refName, "TIPPER",     6) == 0) return 1;
    if (_strnicmp(refName, "BONK",       4) == 0) return 1;
    if (_strnicmp(refName, "BLOCKDAWG",  9) == 0) return 1;
    if (_strnicmp(refName, "GLUEBIE",    7) == 0) return 1;
    if (_strnicmp(refName, "FAN",        3) == 0) return 1;
    if (_strnicmp(refName, "SAWBLADE",   8) == 0) return 1;
    if (_strnicmp(refName, "MACE",       4) == 0) return 1;
    return 0;
}

/* ============================================================
 * Safety check: verify all required board slots are non-NULL
 * ============================================================ */

static int factory_slots_safe(void* board, const SafeFactory* sf)
{
    int j;
    for (j = 0; sf->slots[j] != 0; j++) {
        void* slot_val = *(void**)((char*)board + sf->slots[j]);
        if (slot_val == NULL) return 0;
    }
    return 1;
}

/* ============================================================
 * Universal Factory Dispatch
 *
 * Called instead of the original vtable[33] dispatch.
 * The calling convention is __thiscall:
 *   ECX = board (this pointer)
 *   Stack: refName, &outObj, &outCol, refEntry
 *
 * Flow:
 *   1. Call the original factory (from board's vtable[33])
 *   2. If it returned an object, clone if static-mesh, return
 *   3. If not, try each Arena factory in order (with safety checks)
 *   4. For difficulty-gated objects, temporarily set App+0x23C = 1
 *   5. If a factory returned an object, clone if static-mesh, return
 *   6. Return NULL if nothing handled it
 * ============================================================ */

static void __thiscall universal_factory(
    void* board, char* refName,
    void** outObj, void** outCol, int* refEntry)
{
    void** vtable;
    FactoryFunc original;
    int i;
    int savedDiff = 0;
    int needDiffBypass = 0;

    /* Initialize outputs */
    *outObj = NULL;
    *outCol = NULL;

    /* Step 1: Try the original factory first */
    vtable = *(void***)board;
    original = (FactoryFunc)vtable[33]; /* offset 0x84 / 4 = slot 33 */

    original(board, refName, outObj, outCol, refEntry);

    if (*outObj != NULL) {
        /* Original factory handled it — clone if static-mesh */
        if (is_static_mesh_object(refName)) {
            *outObj = g_cloneTree(*outObj, (int)board);
        }
        return;
    }

    /* Step 2: Original factory didn't handle it — try all Arena factories */
    needDiffBypass = is_difficulty_gated(refName);

    if (needDiffBypass) {
        int* app = *(int**)((char*)board + BOARD_APP_PTR);
        if (app) {
            savedDiff = *(int*)((char*)app + APP_DIFFICULTY);
            *(int*)((char*)app + APP_DIFFICULTY) = 1; /* force Normal */
        }
    }

    for (i = 0; i < (int)NUM_FACTORIES; i++) {
        SafeFactory* sf = &g_factories[i];

        /* Skip if this is the same as the original factory */
        if (sf->func == original)
            continue;

        /* Safety check: verify all required board slots are non-NULL */
        if (!factory_slots_safe(board, sf))
            continue;

        /* Clear outputs before calling */
        *outObj = NULL;
        *outCol = NULL;

        /* Call the factory */
        sf->func(board, refName, outObj, outCol, refEntry);

        if (*outObj != NULL) {
            /* This factory handled it — clone if static-mesh */
            if (is_static_mesh_object(refName)) {
                *outObj = g_cloneTree(*outObj, (int)board);
            }

            /* Restore difficulty if we changed it */
            if (needDiffBypass) {
                int* app = *(int**)((char*)board + BOARD_APP_PTR);
                if (app)
                    *(int*)((char*)app + APP_DIFFICULTY) = savedDiff;
            }
            return;
        }
    }

    /* No factory handled this ref — return NULL */
    *outObj = NULL;
    *outCol = NULL;

    /* Restore difficulty if we changed it */
    if (needDiffBypass) {
        int* app = *(int**)((char*)board + BOARD_APP_PTR);
        if (app)
            *(int*)((char*)app + APP_DIFFICULTY) = savedDiff;
    }
}

/* ============================================================
 * Hook installation
 *
 * Patches the CALL at 0x0040C4BA:
 *   Original:  FF 90 84 00 00 00  (CALL dword ptr [EAX + 0x84])
 *   Patched:   E8 XX XX XX XX 90  (CALL rel32 + NOP)
 *
 * The calling convention is already set up by the caller:
 *   ECX = board, stack has 4 args (refName, &outObj, &outCol, refEntry)
 * ============================================================ */

static void install_hook(void)
{
    BYTE* callSite = (BYTE*)HOOK_CALL_SITE;
    BYTE patch[6];
    DWORD rel32;
    DWORD oldProt;

    /* Save original bytes */
    memcpy(g_origBytes, callSite, 6);

    /* Calculate relative offset: target - (callSite + 5) */
    rel32 = (DWORD)universal_factory - (DWORD)(callSite + 5);

    /* Build patch: E8 rel32 90 (CALL rel32 + NOP) */
    patch[0] = 0xE8;
    *(DWORD*)(patch + 1) = rel32;
    patch[5] = 0x90; /* NOP */

    /* Write patch */
    VirtualProtect(callSite, 6, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy(callSite, patch, 6);
    VirtualProtect(callSite, 6, oldProt, &oldProt);

    /* Flush instruction cache */
    FlushInstructionCache(GetCurrentProcess(), callSite, 6);

    g_hooked = TRUE;
}

static void remove_hook(void)
{
    BYTE* callSite = (BYTE*)HOOK_CALL_SITE;
    DWORD oldProt;

    if (!g_hooked) return;

    VirtualProtect(callSite, 6, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy(callSite, g_origBytes, 6);
    VirtualProtect(callSite, 6, oldProt, &oldProt);

    FlushInstructionCache(GetCurrentProcess(), callSite, 6);
    g_hooked = FALSE;
}

/* ============================================================
 * BASS Proxy (v3 lazy loader pattern)
 *
 * Loads bass_real.dll on first BASS call (NOT in DllMain).
 * If bass_real.dll exists, all calls forwarded.
 * If missing, stubs return 0 (no audio, but no crash).
 *
 * CRITICAL: Each function's param count MUST match the real
 * BASS function's stack cleanup. See skill hamsterball-dll-modding.
 * ============================================================ */

static HMODULE g_realBass = NULL;
static BOOL g_bassTriedLoad = FALSE;

static void load_real_bass(void)
{
    g_bassTriedLoad = TRUE;
    g_realBass = LoadLibraryA("bass_real.dll");
}

#define DEFINE_BASS_FORWARDED(name, ret_type, params, args, stub_ret) \
    typedef ret_type (__stdcall *name##_t) params; \
    static name##_t real_##name = NULL; \
    __declspec(dllexport) ret_type __stdcall name params { \
        if (!g_bassTriedLoad) load_real_bass(); \
        if (g_realBass && !real_##name) \
            real_##name = (name##_t)GetProcAddress(g_realBass, #name); \
        if (real_##name) return real_##name args; \
        return stub_ret; \
    }

/* The 10 BASS functions the game imports.
 * Param counts verified from working bass.dll mods.
 * BASS_MusicLoad has 6 params (QWORD offset counts as 2 DWORDs = 8 bytes
 * but BASS 32-bit packs it differently — 6 params works, 4 doesn't).
 */
DEFINE_BASS_FORWARDED(BASS_Init,              int,     (int a, int b, int c, HWND d, void* e), (a,b,c,d,e), 0)
DEFINE_BASS_FORWARDED(BASS_Free,              int,     (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_GetVersion,        DWORD,   (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_MusicLoad,          void*,   (void* a, void* b, DWORD c, DWORD d, DWORD e, DWORD f), (a,b,c,d,e,f), 0)
DEFINE_BASS_FORWARDED(BASS_StreamCreateFile,  void*,   (void* a, void* b, DWORD c, DWORD d, DWORD e), (a,b,c,d,e), 0)
DEFINE_BASS_FORWARDED(BASS_SampleLoad,        void*,   (void* a, void* b, DWORD c, DWORD d, DWORD e, DWORD f), (a,b,c,d,e,f), 0)
DEFINE_BASS_FORWARDED(BASS_SetConfig,         int,     (DWORD a, DWORD b), (a,b), 0)
DEFINE_BASS_FORWARDED(BASS_GetConfig,         DWORD,   (DWORD a), (a), 0)
DEFINE_BASS_FORWARDED(BASS_SetVolume,         int,     (float a), (a), 0)
DEFINE_BASS_FORWARDED(BASS_GetVolume,         float,   (void), (), 0)

/* Additional BASS functions the game may call */
DEFINE_BASS_FORWARDED(BASS_ChannelPlay,        int,     (DWORD a, int b), (a,b), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelStop,        int,     (DWORD a), (a), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelSetAttribute, int,     (DWORD a, DWORD b, float c), (a,b,c), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelGetAttribute, int,     (DWORD a, DWORD b, float* c), (a,b,c), 0)
DEFINE_BASS_FORWARDED(BASS_SampleCreate,       void*,   (DWORD a, DWORD b, DWORD c, DWORD d, DWORD e), (a,b,c,d,e), 0)

/* ============================================================
 * DLL Entry Point
 * ============================================================ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        /* Do NOT load bass_real.dll here — use lazy loading on first BASS call.
         * Loading in DllMain deadlocks on real Windows (loader lock). */
        install_hook();
        break;

    case DLL_PROCESS_DETACH:
        remove_hook();
        if (g_realBass) {
            FreeLibrary(g_realBass);
            g_realBass = NULL;
        }
        break;
    }

    return TRUE;
}

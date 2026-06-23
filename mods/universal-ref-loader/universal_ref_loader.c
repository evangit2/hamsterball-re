/*
 * universal_ref_loader.c — Hamsterball DLL Mod
 * 
 * Loads ANY ref type into ANY level by hooking the vtable[33] dispatch
 * in Scene_CreateDynamicObjects. When the original Board factory doesn't
 * recognize a ref name, the hook tries all other level factories in sequence.
 *
 * Hook point: 0x0040C4BA (CALL dword ptr [EAX + 0x84])
 *   Original: indirect call through board vtable slot 33
 *   Patched:  direct call to our universal_factory()
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll universal_ref_loader.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * The mod hooks as a bass.dll proxy (lazy loader pattern).
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <mmeapi.h>
#include <dsound.h>

/* ============================================================
 * Game constants (from reverse engineering)
 * ============================================================ */

#define GAME_BASE 0x400000

/* Function addresses (RVA from game base) */
#define SCENE_CREATE_DYNAMIC_OBJECTS  0x0040C4BA  /* call site */
#define HOOK_CALL_SITE                0x0040C4BA
#define HOOK_CALL_SITE_LEN            6           /* FF 90 84 00 00 00 */

/* Original factory function addresses
 * 
 * ARENA Board factories (0x40Axxx-0x417xxx) — handle full ref set
 * RACE Board factories (0x413xxx-0x418xxx) — handle subset of refs
 * 
 * The universal factory should try ALL Arena factories to maximize
 * the number of ref types that can be created.
 */

/* ARENA factories */
#define FACTORY_ARENA_NOOP        0x00419750  /* Arena L1 (WarmUp) */
#define FACTORY_ARENA_BEGINNER    0x0040A550  /* Arena L2 */
#define FACTORY_ARENA_INTERMED    0x0040A5F0  /* Arena L3 */
#define FACTORY_ARENA_DIZZY       0x0040D7C0  /* Arena L4 */
#define FACTORY_ARENA_TOWER       0x004117B0  /* Arena L5 */
#define FACTORY_ARENA_UP          0x00416910  /* Arena L6 */
#define FACTORY_ARENA_EXPERT      0x0040E250  /* Arena L7 */
#define FACTORY_ARENA_ODD         0x0040EC40  /* Arena L8 */
#define FACTORY_ARENA_TOOB        0x0040FB30  /* Arena L9 */
#define FACTORY_ARENA_GLASS      0x0040F420  /* Arena L10 */
#define FACTORY_ARENA_WOBBLY     0x0040AD80  /* Arena L11 */
#define FACTORY_ARENA_SKY        0x00410AD0  /* Arena L12 */
#define FACTORY_ARENA_MASTER     0x004121D0  /* Arena L13 (most inclusive) */
#define FACTORY_ARENA_IMPOSSIBLE 0x00417FE0  /* Arena L14 */

/* RACE factories (for race mode) */
#define FACTORY_RACE_BASE         0x004133E0  /* Race L1,2,3,9,10,12,14 */
#define FACTORY_RACE_DIZZY        0x004143D0  /* Race L4 */
#define FACTORY_RACE_TOWER        0x00414680  /* Race L5 */
#define FACTORY_RACE_UP           0x00414A20  /* Race L6 */
#define FACTORY_RACE_NEON         0x004173B0  /* Race L7 */
#define FACTORY_RACE_EXPERT       0x00414BD0  /* Race L8 */
#define FACTORY_RACE_WOBBLY       0x00415460  /* Race L11 */
#define FACTORY_RACE_SKY          0x00415A30  /* Race L13 */
#define FACTORY_RACE_IMPOSSIBLE   0x00418760  /* Race L15 */

/* Board struct offsets */
#define BOARD_VTABLE             0x000       /* *(void***)board = vtable */
#define BOARD_SCENE              0x878       /* board+0x878 = Scene* */
#define SCENE_QUALITY            0x23C       /* scene+0x23C = quality flag */
#define BOARD_GENERAL_LIST       0x2578      /* board+0x2578 = AthenaList of all objects */

/* ============================================================
 * Types
 * ============================================================ */

typedef void (__thiscall *FactoryFunc)(void* board, char* refName, 
    void** outObj, void** outCol, int* refEntry);

typedef void* (__cdecl *operator_new_func)(size_t size);

/* ============================================================
 * Globals
 * ============================================================ */

static BYTE g_origBytes[6];  /* original bytes at hook site */
static BOOL g_hooked = FALSE;
static DWORD g_oldProtect = 0;

/* The original vtable[33] pointer saved before hooking */
static FactoryFunc g_origFactory = NULL;

/* operator_new and MeshWorld_ctor for runtime mesh loading */
static operator_new_func g_operator_new = NULL;
static DWORD g_operator_new_addr = 0x00449E70;  /* operator new in Hamsterball.exe */

/* ============================================================
 * Universal Factory
 * 
 * Called instead of the original vtable[33] dispatch.
 * Tries the original factory first, then falls through to
 * all other level factories until one succeeds.
 * ============================================================ */

/* Sub-mesh slot dependency table: which slots each factory accesses.
 * Factories do NOT null-check these slots — if a slot is NULL, the factory
 * will crash with an access violation. We must check before calling.
 */
typedef struct {
    FactoryFunc func;
    int slots[9];  /* board offsets that must be non-NULL, -1 = end */
    const char* name;
} SafeFactory;

static SafeFactory safe_factories[] = {
    { (FactoryFunc)FACTORY_ARENA_MASTER,     {0x436C, 0x4370, 0x4394, 0x4398, -1}, "Master" },
    { (FactoryFunc)FACTORY_ARENA_DIZZY,      {0x436C, 0x4370, 0x4378, 0x437C, 0x4390, 0x43A4, 0x43B0, 0x43B4, -1}, "Dizzy" },
    { (FactoryFunc)FACTORY_ARENA_IMPOSSIBLE, {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, -1}, "Impossible" },
    { (FactoryFunc)FACTORY_ARENA_EXPERT,     {0x436C, 0x4370, 0x4374, -1}, "Expert" },
    { (FactoryFunc)FACTORY_ARENA_TOWER,     {0x436C, 0x4370, 0x4374, 0x4378, 0x4394, 0x4398, -1}, "Tower" },
    { (FactoryFunc)FACTORY_ARENA_GLASS,     {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, 0x4380, 0x4384, -1}, "Glass" },
    { (FactoryFunc)FACTORY_ARENA_TOOB,      {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, 0x4380, 0x4384, -1}, "Toob" },
    { (FactoryFunc)FACTORY_ARENA_UP,        {0x4374, 0x4378, 0x437C, 0x4380, 0x4384, 0x4388, 0x438C, 0x4390, -1}, "Neon" },
    { (FactoryFunc)FACTORY_ARENA_ODD,       {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, -1}, "Odd" },
    { (FactoryFunc)FACTORY_ARENA_SKY,       {0x436C, 0x4374, 0x4378, 0x437C, 0x4380, 0x438C, 0x4390, -1}, "Sky" },
    { (FactoryFunc)FACTORY_ARENA_BEGINNER,  {0x436C, 0x4370, 0x4374, -1}, "Beginner" },
    { (FactoryFunc)FACTORY_ARENA_INTERMED,  {0x436C, 0x4370, 0x4374, -1}, "Intermediate" },
    { (FactoryFunc)FACTORY_ARENA_WOBBLY,   {0x4344, -1}, "Wobbly" },
    { (FactoryFunc)FACTORY_RACE_BASE,      {-1}, "RaceBase" },  /* no sub-mesh slots needed */
};

static void __thiscall universal_factory(
    void* board, char* refName, 
    void** outObj, void** outCol, int* refEntry)
{
    int numFactories = sizeof(safe_factories) / sizeof(safe_factories[0]);
    int i, j;
    
    /* Initialize outputs to null */
    *outObj = NULL;
    *outCol = NULL;
    
    /* Try each factory in sequence, but only if its sub-mesh slots are loaded */
    for (i = 0; i < numFactories; i++) {
        SafeFactory* sf = &safe_factories[i];
        
        /* Check if all required sub-mesh slots are non-NULL */
        BOOL safe = TRUE;
        for (j = 0; sf->slots[j] != -1; j++) {
            void* slot_val = *(void**)((char*)board + sf->slots[j]);
            if (slot_val == NULL) {
                safe = FALSE;
                break;
            }
        }
        
        if (!safe) {
            /* Skip this factory — its sub-mesh slots aren't loaded */
            continue;
        }
        
        /* Call the factory */
        *outObj = NULL;
        *outCol = NULL;
        sf->func(board, refName, outObj, outCol, refEntry);
        
        /* If it returned a non-null object, we're done */
        if (*outObj != NULL) {
            return;
        }
    }
    
    /* No factory handled this ref — return null (original behavior for unknown refs) */
    *outObj = NULL;
    *outCol = NULL;
}

/* ============================================================
 * Hook installation
 * 
 * The call site at 0x0040C4BA is:
 *   FF 90 84 00 00 00    CALL dword ptr [EAX + 0x84]
 * 
 * We replace it with:
 *   E8 XX XX XX XX       CALL universal_factory  (relative call)
 *   90                   NOP (padding)
 * 
 * But wait — the original is an indirect call through EAX+0x84, where
 * EAX = *board (the vtable pointer), and the calling convention has
 * already set up ECX=board, and pushed the 4 arguments.
 * 
 * Our universal_factory needs to be a __thiscall with ECX=board.
 * The existing stack setup already has the right args pushed, so we
 * just need to change the indirect call to a direct call to our function.
 * 
 * However, the original CALL is 6 bytes (FF 90 84 00 00 00) and a
 * relative CALL is 5 bytes (E8 XX XX XX XX) + 1 NOP.
 * ============================================================ */

static void install_hook(void)
{
    BYTE* callSite = (BYTE*)HOOK_CALL_SITE;
    BYTE patch[6];
    DWORD rel32;
    DWORD oldProt;
    
    /* Save original bytes */
    memcpy(g_origBytes, callSite, 6);
    
    /* Calculate relative offset for CALL instruction */
    /* rel32 = target - (callSite + 5) */
    rel32 = (DWORD)universal_factory - (DWORD)(callSite + 5);
    
    /* Build patch: E8 rel32 90 (CALL rel32 + NOP) */
    patch[0] = 0xE8;
    *(DWORD*)(patch + 1) = rel32;
    patch[5] = 0x90;  /* NOP */
    
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
 * BASS Proxy (lazy loader pattern v3)
 * ============================================================ */

/* BASS function typedefs */
typedef void* HPLUGIN;
typedef void* HSTREAM;
typedef void* HMUSIC;
typedef void* HSAMPLE;
typedef void* HCHANNEL;

static HMODULE g_realBass = NULL;

/* Forward declarations for BASS functions we proxy */
#define BASS_PROXY(name, rettype, callconv, args, argnames) \
    typedef rettype (callconv *name##_t) args; \
    static name##_t name##_real = NULL; \
    rettype callconv name args { \
        if (!name##_real) { \
            if (!g_realBass) return 0; \
            name##_real = (name##_t)GetProcAddress(g_realBass, #name); \
            if (!name##_real) return 0; \
        } \
        return name##_real argnames; \
    }

/* Minimal BASS proxy set */
BASS_PROXY(BASS_Init, int, __stdcall, (int a, int b, int c, HWND d, void* e), (a,b,c,d,e))
BASS_PROXY(BASS_Free, int, __stdcall, (void), ())
BASS_PROXY(BASS_GetVersion, DWORD, __stdcall, (void), ())
BASS_PROXY(BASS_MusicLoad, HMUSIC, __stdcall, (void* a, void* b, DWORD c, DWORD d, DWORD e, DWORD f), (a,b,c,d,e,f))
BASS_PROXY(BASS_StreamCreateFile, HSTREAM, __stdcall, (void* a, void* b, DWORD c, DWORD d, DWORD e), (a,b,c,d,e))
BASS_PROXY(BASS_SampleLoad, HSAMPLE, __stdcall, (void* a, void* b, DWORD c, DWORD d, DWORD e, DWORD f), (a,b,c,d,e,f))
BASS_PROXY(BASS_SetConfig, BOOL, __stdcall, (DWORD opt, DWORD val), (opt,val))
BASS_PROXY(BASS_GetConfig, DWORD, __stdcall, (DWORD opt), (opt))
BASS_PROXY(BASS_SetVolume, BOOL, __stdcall, (float vol), (vol))
BASS_PROXY(BASS_GetVolume, float, __stdcall, (void), ())
BASS_PROXY(BASS_ChannelPlay, BOOL, __stdcall, (DWORD handle, BOOL restart), (handle,restart))
BASS_PROXY(BASS_ChannelStop, BOOL, __stdcall, (DWORD handle), (handle))
BASS_PROXY(BASS_ChannelSetAttribute, BOOL, __stdcall, (DWORD handle, DWORD attr, float val), (handle,attr,val))
BASS_PROXY(BASS_ChannelGetAttribute, BOOL, __stdcall, (DWORD handle, DWORD attr, float* val), (handle,attr,val))
BASS_PROXY(BASS_SampleCreate, HSAMPLE, __stdcall, (DWORD freq, DWORD chans, DWORD flags, DWORD max, DWORD extra), (freq,chans,flags,max,extra))

/* Stub for any other BASS function — returns 0 */
void __stdcall BASS_Stub(void) { return; }

/* ============================================================
 * DLL Entry Point
 * ============================================================ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        /* Try to load real bass.dll for audio */
        g_realBass = LoadLibraryA("bass_real.dll");
        
        /* Install the universal ref loader hook */
        install_hook();
        
        break;
    }
    
    case DLL_PROCESS_DETACH:
    {
        remove_hook();
        if (g_realBass) {
            FreeLibrary(g_realBass);
            g_realBass = NULL;
        }
        break;
    }
    }
    
    return TRUE;
}

/* Export table — must match bass.dll exports for proxy behavior */
/* The linker with --add-stdcall-alias will handle the rest */

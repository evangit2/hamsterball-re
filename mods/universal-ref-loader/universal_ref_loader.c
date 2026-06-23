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

/* Original factory function addresses */
#define FACTORY_NOOP             0x00419750  /* WarmUp, Beginner */
#define FACTORY_BRIDGE           0x0040A550  /* Intermediate */
#define FACTORY_DIZZY            0x0040A5F0  /* Dizzy */
#define FACTORY_TOWER            0x0040D7C0  /* Tower */
#define FACTORY_UP               0x004117B0  /* Up (SpeedCylinder) */
#define FACTORY_NEON             0x00416910  /* Neon */
#define FACTORY_EXPERT           0x0040E250  /* Expert (Sawblade) */
#define FACTORY_ODD              0x0040EC40  /* Odd (Lifter) */
#define FACTORY_TOOB             0x0040FB30  /* Toob */
#define FACTORY_WOBBLY           0x0040F420  /* Wobbly */
#define FACTORY_GLASS            0x0040AD80  /* Glass (Smasher) */
#define FACTORY_SKY              0x00410AD0  /* Sky */
#define FACTORY_MASTER           0x004121D0  /* Master (CreateLevelObjects) */
#define FACTORY_IMPOSSIBLE       0x00417FE0  /* Impossible (CreateMechanicalObjects) */

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

static void __thiscall universal_factory(
    void* board, char* refName, 
    void** outObj, void** outCol, int* refEntry)
{
    FactoryFunc factories[] = {
        /* Most inclusive factories first */
        (FactoryFunc)FACTORY_MASTER,        /* BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT, GLUEBIE */
        (FactoryFunc)FACTORY_IMPOSSIBLE,    /* LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM */
        (FactoryFunc)FACTORY_TOWER,         /* CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, TURRET */
        (FactoryFunc)FACTORY_EXPERT,        /* BONK, SAWBLADE, BRIDGE, JUDGE, BELL */
        (FactoryFunc)FACTORY_TOOB,          /* SPINNY, SAW, SAW2, FALLOUT1, BLOCKDAWG1-3 */
        (FactoryFunc)FACTORY_UP,            /* LIFTER, SPEEDCYLINDER, TIMEBUTTON */
        (FactoryFunc)FACTORY_DIZZY,         /* TIPPER, WATERWHEEL, SWIRL, GLUEBIE */
        (FactoryFunc)FACTORY_NEON,          /* NEONPLATFORM, DFLOOR1-4, TRODE */
        (FactoryFunc)FACTORY_WOBBLY,        /* WOBBLY1-7, WAVY1 */
        (FactoryFunc)FACTORY_SKY,           /* POPCYLINDER, TRAPDOOR */
        (FactoryFunc)FACTORY_BRIDGE,        /* BRIDGE */
        (FactoryFunc)FACTORY_ODD,           /* LIFTER */
        (FactoryFunc)FACTORY_GLASS,         /* SMASHER1-2 */
    };
    int numFactories = sizeof(factories) / sizeof(factories[0]);
    int i;
    
    /* Initialize outputs to null */
    *outObj = NULL;
    *outCol = NULL;
    
    /* Try each factory in sequence */
    for (i = 0; i < numFactories; i++) {
        *outObj = NULL;
        *outCol = NULL;
        
        /* Call the factory */
        factories[i](board, refName, outObj, outCol, refEntry);
        
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

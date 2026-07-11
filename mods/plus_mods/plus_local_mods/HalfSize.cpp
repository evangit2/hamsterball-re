/*
 * HalfSize.cpp — HB+ API port of half_size_balls.c
 *
 * Halves all ball sizes by patching 3 sites:
 *   1. Ball_ctor2 default radius:  27.0 -> 13.5  (MOV immediate patch)
 *   2. Player ball spawn radius:   26.0 -> 13.0  (MOV immediate patch)
 *   3. CreateBadBall SIZE tag:     halve FPU value via code cave
 *
 * Original: bass.dll proxy mod (half_size_balls.c)
 * Ported to HB+ API v2.0 for use with local_mods loader.
 *
 * Author: Hamsterbot
 */
#include <windows.h>
#include "HamsterballAPI.h"
#include "nocrt.h"
#include "hbplus_api.h"

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define snprintf nc_snprintf

// ============================================================================
// Patch Constants
// ============================================================================

#define IMAGE_BASE 0x00400000

/* Patch 1: Ball_ctor2 default radius 27.0 -> 13.5 */
#define PATCH1_ADDR      0x00403C91
#define PATCH1_ORIGINAL  "\x00\x00\xD8\x41"   /* 27.0f */
#define PATCH1_PATCHED   "\x00\x00\x58\x41"   /* 13.5f */
#define PATCH1_LEN       4

/* Patch 2: Player ball spawn radius 26.0 -> 13.0 */
#define PATCH2_ADDR      0x0041C8B0
#define PATCH2_ORIGINAL  "\x00\x00\xD0\x41"   /* 26.0f */
#define PATCH2_PATCHED   "\x00\x00\x50\x41"   /* 13.0f */
#define PATCH2_LEN       4

/* Patch 3: CreateBadBall SIZE handler — code cave */
#define PATCH3_ADDR      0x0040BE74
#define PATCH3_ORIGINAL  "\xD9\x9E\x84\x02\x00\x00"   /* FSTP [ESI+0x284] */
#define PATCH3_LEN       6

/* Address of 0.5f float constant in .text */
#define HALF_FLOAT_ADDR   0x0041C89C

// ============================================================================
// Memory Patching Helpers
// ============================================================================

static int patch_bytes(BYTE *addr, const BYTE *expected, const BYTE *replacement, SIZE_T len) {
    DWORD oldProtect;
    if (memcmp(addr, expected, len) != 0) return 0;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) return 0;
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

static int write_bytes(BYTE *addr, const BYTE *data, SIZE_T len) {
    DWORD oldProtect;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) return 0;
    memcpy(addr, data, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

static void *allocate_code_cave(SIZE_T size) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    SIZE_T alloc_size = ((size + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;
    DWORD_PTR addr = (IMAGE_BASE + 0xF8000);
    void *cave = VirtualAlloc((void*)addr, alloc_size, MEM_COMMIT | MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);
    if (cave) return cave;
    return VirtualAlloc(NULL, alloc_size, MEM_COMMIT | MEM_RESERVE,
                       PAGE_EXECUTE_READWRITE);
}

static int make_rel32_call(BYTE *call_addr, void *target) {
    DWORD_PTR src = (DWORD_PTR)call_addr + 5;
    DWORD_PTR dst = (DWORD_PTR)target;
    ptrdiff_t rel = (ptrdiff_t)(dst - src);
    if (rel > 0x7FFFFFFF || rel < (ptrdiff_t)0x80000000) return 0;
    int32_t rel32 = (int32_t)rel;
    memcpy(call_addr + 1, &rel32, 4);
    return 1;
}

// ============================================================================
// Code Cave (halves CreateBadBall SIZE via FMUL 0.5f)
// ============================================================================

static unsigned char code_cave[] = {
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* FSTP [ESI+0x284] */
    0xD9, 0x86, 0x84, 0x02, 0x00, 0x00,   /* FLD  [ESI+0x284] */
    0xD8, 0x0D, 0x9C, 0xC8, 0x41, 0x00,   /* FMUL dword [0x0041C89C] */
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* FSTP [ESI+0x284] */
    0xC3                                    /* RET */
};

// ============================================================================
// Patch Application
// ============================================================================

static int g_patchesApplied = 0;
static bool g_initDone = false;

static void applyPatches() {
    if (g_initDone) return;
    g_initDone = true;

    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    BYTE *base = (BYTE*)hExe;

    BYTE *p1 = base + (PATCH1_ADDR - IMAGE_BASE);
    BYTE *p2 = base + (PATCH2_ADDR - IMAGE_BASE);
    BYTE *p3 = base + (PATCH3_ADDR - IMAGE_BASE);

    g_patchesApplied = 0;

    /* Patch 1: Ball_ctor2 default radius 27.0 -> 13.5 */
    g_patchesApplied += patch_bytes(p1, (const BYTE*)PATCH1_ORIGINAL,
                                    (const BYTE*)PATCH1_PATCHED, PATCH1_LEN);

    /* Patch 2: Player ball spawn radius 26.0 -> 13.0 */
    g_patchesApplied += patch_bytes(p2, (const BYTE*)PATCH2_ORIGINAL,
                                    (const BYTE*)PATCH2_PATCHED, PATCH2_LEN);

    /* Patch 3: CreateBadBall SIZE — code cave to halve FPU value */
    if (memcmp(p3, PATCH3_ORIGINAL, PATCH3_LEN) == 0) {
        void *cave = allocate_code_cave(sizeof(code_cave));
        if (cave && write_bytes((BYTE*)cave, code_cave, sizeof(code_cave))) {
            BYTE call_nop[6];
            call_nop[0] = 0xE8;  /* CALL rel32 */
            call_nop[5] = 0x90;  /* NOP */
            if (make_rel32_call(call_nop, cave) && write_bytes(p3, call_nop, 6)) {
                g_patchesApplied++;
            }
        }
    }
}

// ============================================================================
// Manual Vtable (17-entry MSVC layout for HB+ v2.0)
// ============================================================================

static IModAPI* g_api = NULL;

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Half Size"; }
static const char* __thiscall get_author(void*) { return "Hamsterbot"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return ""; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    /* Apply patches in Initialize — the code section is already loaded */
    applyPatches();
}

static void __thiscall ball_update_impl(void* thisptr, Ball* ball) {
    /* No per-frame logic needed — patches are permanent */
}

static void __thiscall render_apply_impl(void* thisptr, void* this_ptr, float* viewMatrix) {}
static void __thiscall button_toggle_impl(void* thisptr, const char* id, bool state) {}
static void __thiscall slider_change_impl(void* thisptr, const char* id, float val) {}
static void __thiscall cycle_option_impl(void* thisptr, const char* id, const char* opt) {}
static void __thiscall game_update_impl(void* thisptr) {}
static void __thiscall event_collide_impl(void* thisptr, Ball* ball, char* id) {}
static void __thiscall text_render_impl(void* thisptr) {}
static void __thiscall ball_bump_impl(void* thisptr, Ball* b1, Ball* b2) {}
static void __thiscall scene_end_impl(void* thisptr) {}

static void __thiscall level_start_impl(void* thisptr) {
    /* Re-apply patches on level start in case game re-initializes */
    /* (the code cave allocation is one-time, but the byte patches
       might get overwritten by the game's own init) */
    if (!g_initDone) applyPatches();
}

static void* g_vtable[17] = {
    (void*)sc_dtor,             // [0]  scalar deleting destructor
    (void*)get_mod_name,        // [1]  GetModName
    (void*)get_author,          // [2]  GetAuthorName
    (void*)get_version,         // [3]  GetApiVersion
    (void*)get_contributors,    // [4]  GetContributors
    (void*)init_impl,           // [5]  Initialize
    (void*)ball_update_impl,    // [6]  onBallUpdate
    (void*)render_apply_impl,   // [7]  onRenderApply
    (void*)button_toggle_impl,  // [8]  onButtonToggle
    (void*)slider_change_impl,  // [9]  onSliderChange
    (void*)cycle_option_impl,   // [10] onCycleOptionChange (v2.0)
    (void*)game_update_impl,    // [11] onGameUpdate
    (void*)event_collide_impl,  // [12] onEventPlaneCollide
    (void*)text_render_impl,    // [13] onTextRenderLoop
    (void*)ball_bump_impl,      // [14] onBallBump
    (void*)scene_end_impl,      // [15] onSceneEnd
    (void*)level_start_impl,    // [16] onLevelStart
};

// ============================================================================
// Export
// ============================================================================

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

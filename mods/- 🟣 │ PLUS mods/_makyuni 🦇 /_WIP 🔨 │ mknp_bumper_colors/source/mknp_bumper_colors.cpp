/*
 * mkn_plus_bumper_colors.cpp — Bumper flash colors from MESHWORLD emissive (HB+ v2.1)
 *
 * When a ball hits a bumper, instead of flashing hardcoded white (1,1,1),
 * the bumper flashes its own emissive color from the level MESHWORLD material.
 *
 * The mod patches Scene_RenderReflectiveObjects4 (0x00412DC0) at the point
 * where it writes emissive RGBA to the RenderContext. A code cave reads
 * the bumper's material emissive color from the MeshWorld material array
 * and writes it to the RenderContext instead of hardcoded 1.0.
 *
 * Access path:
 *   level_ptr = *(board + 0x8AC)
 *   material_array = *(level_ptr + 0x28)
 *   bumper_sceneobj = board + 0x439C + index * 0x418
 *   material_index = *(bumper_sceneobj + 0x04)
 *   material = material_array + material_index * 0x50
 *   emissive_R = *(float*)(material + 0x34)
 *   emissive_G = *(float*)(material + 0x38)
 *   emissive_B = *(float*)(material + 0x3C)
 *   emissive_A = *(float*)(material + 0x40)
 *
 * Hook point: 0x00412EA6 (first MOV for Emissive.R)
 * Replaces: 0x00412EA6 to 0x00412ECB (37 bytes → JMP + 32 NOPs)
 * Returns to: 0x00412ECB (MOV BYTE [ESP+0x60], 1)
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.0/v2.1.
 * Uses PatchMemory from HB+ API (no VirtualAlloc needed — the API does it).
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* ===== Globals ===== */
static void* g_api = NULL;
static bool g_patched = false;

/* ===== Code cave data ===== */
/* The code cave is written to VirtualAlloc'd memory at runtime. */
/* We store the cave address here so onButtonToggle can re-patch if needed. */
static DWORD g_caveAddr = 0;

/* Original bytes at hook site (for unpatch support) */
static unsigned char g_origBytes[37];

/* ===== Code cave assembly (machine code) ===== */
/*
 * At hook point (0x00412EA6):
 *   EDI = board
 *   EBP = bumper SceneObject
 *   ESP+0x14 = RenderContext
 *   Stack offsets for emissive: ESP+0x48(R), +0x4C(G), +0x50(B), +0x54(A)
 *
 * Code cave:
 *   PUSH EDX                        ; 52
 *   PUSH ECX                        ; 51
 *   PUSH EAX                        ; 50
 *   ; After 3 pushes, ESP is 12 bytes lower
 *   ; ESP+0x48 → ESP+0x54, ESP+0x4C → ESP+0x58, etc.
 *
 *   MOV EDX, [EDI + 0x8AC]          ; 8B 97 AC 08 00 00  (level ptr)
 *   TEST EDX, EDX                    ; 85 D2
 *   JZ fallback                      ; 74 XX
 *   MOV EDX, [EDX + 0x28]           ; 8B 52 28  (material_array)
 *   TEST EDX, EDX                    ; 85 D2
 *   JZ fallback                      ; 74 XX
 *   MOV ECX, [EBP + 0x04]           ; 8B 4D 04  (material_index)
 *   IMUL ECX, ECX, 0x50             ; 6B C9 50  (offset = index * 0x50)
 *   ADD EDX, ECX                    ; 01 CA  (EDX = material ptr)
 *
 *   ; Read emissive RGBA
 *   MOV EAX, [EDX + 0x34]           ; 8B 42 34  (R)
 *   MOV [ESP + 0x54], EAX           ; 89 44 24 54  (→ orig ESP+0x48)
 *   MOV EAX, [EDX + 0x38]           ; 8B 42 38  (G)
 *   MOV [ESP + 0x58], EAX           ; 89 44 24 58  (→ orig ESP+0x4C)
 *   MOV EAX, [EDX + 0x3C]           ; 8B 42 3C  (B)
 *   MOV [ESP + 0x5C], EAX           ; 89 44 24 5C  (→ orig ESP+0x50)
 *   MOV EAX, [EDX + 0x40]           ; 8B 42 40  (A)
 *   MOV [ESP + 0x60], EAX           ; 89 44 24 60  (→ orig ESP+0x54)
 *   JMP done                         ; EB XX
 *
 * fallback:
 *   MOV DWORD [ESP + 0x54], 0x3F800000  ; C7 44 24 54 00 00 80 3F (R=1.0)
 *   MOV DWORD [ESP + 0x58], 0x3F800000  ; C7 44 24 58 00 00 80 3F (G=1.0)
 *   MOV DWORD [ESP + 0x5C], 0x3F800000  ; C7 44 24 5C 00 00 80 3F (B=1.0)
 *   MOV DWORD [ESP + 0x60], 0x3F800000  ; C7 44 24 60 00 00 80 3F (A=1.0)
 *
 * done:
 *   POP EAX                          ; 58
 *   POP ECX                          ; 59
 *   POP EDX                          ; 5A
 *   FNSTSW AX                        ; DF E0  (original instruction at 0x412EAE)
 *   TEST AH, 0x44                    ; F6 C4 44  (original instruction at 0x412EC0)
 *   JMP 0x00412ECB                   ; E9 XX XX XX XX  (back to MOV BYTE [ESP+0x60], 1)
 */

/* Build the code cave bytes at runtime */
static void buildCodeCave(unsigned char* cave, DWORD caveAddr) {
    int i = 0;

    /* PUSH EDX, PUSH ECX, PUSH EAX */
    cave[i++] = 0x52;  /* PUSH EDX */
    cave[i++] = 0x51;  /* PUSH ECX */
    cave[i++] = 0x50;  /* PUSH EAX */

    /* MOV EDX, [EDI + 0x8AC] */
    cave[i++] = 0x8B; cave[i++] = 0x97;
    cave[i++] = 0xAC; cave[i++] = 0x08; cave[i++] = 0x00; cave[i++] = 0x00;

    /* TEST EDX, EDX */
    cave[i++] = 0x85; cave[i++] = 0xD2;

    /* JZ fallback (we'll patch the offset later) */
    int jz1_offset = i;
    cave[i++] = 0x74; cave[i++] = 0x00; /* placeholder */

    /* MOV EDX, [EDX + 0x28] */
    cave[i++] = 0x8B; cave[i++] = 0x52; cave[i++] = 0x28;

    /* TEST EDX, EDX */
    cave[i++] = 0x85; cave[i++] = 0xD2;

    /* JZ fallback */
    int jz2_offset = i;
    cave[i++] = 0x74; cave[i++] = 0x00; /* placeholder */

    /* MOV ECX, [EBP + 0x04] */
    cave[i++] = 0x8B; cave[i++] = 0x4D; cave[i++] = 0x04;

    /* IMUL ECX, ECX, 0x50 */
    cave[i++] = 0x6B; cave[i++] = 0xC9; cave[i++] = 0x50;

    /* ADD EDX, ECX */
    cave[i++] = 0x01; cave[i++] = 0xCA;

    /* MOV EAX, [EDX + 0x34]  (Emissive.R) */
    cave[i++] = 0x8B; cave[i++] = 0x42; cave[i++] = 0x34;
    /* MOV [ESP + 0x54], EAX */
    cave[i++] = 0x89; cave[i++] = 0x44; cave[i++] = 0x24; cave[i++] = 0x54;

    /* MOV EAX, [EDX + 0x38]  (Emissive.G) */
    cave[i++] = 0x8B; cave[i++] = 0x42; cave[i++] = 0x38;
    /* MOV [ESP + 0x58], EAX */
    cave[i++] = 0x89; cave[i++] = 0x44; cave[i++] = 0x24; cave[i++] = 0x58;

    /* MOV EAX, [EDX + 0x3C]  (Emissive.B) */
    cave[i++] = 0x8B; cave[i++] = 0x42; cave[i++] = 0x3C;
    /* MOV [ESP + 0x5C], EAX */
    cave[i++] = 0x89; cave[i++] = 0x44; cave[i++] = 0x24; cave[i++] = 0x5C;

    /* MOV EAX, [EDX + 0x40]  (Emissive.A) */
    cave[i++] = 0x8B; cave[i++] = 0x42; cave[i++] = 0x40;
    /* MOV [ESP + 0x60], EAX */
    cave[i++] = 0x89; cave[i++] = 0x44; cave[i++] = 0x24; cave[i++] = 0x60;

    /* JMP done */
    int jmp_done_offset = i;
    cave[i++] = 0xEB; cave[i++] = 0x00; /* placeholder */

    /* fallback: */
    int fallback_pos = i;

    /* MOV DWORD [ESP + 0x54], 0x3F800000  (R = 1.0) */
    cave[i++] = 0xC7; cave[i++] = 0x44; cave[i++] = 0x24; cave[i++] = 0x54;
    cave[i++] = 0x00; cave[i++] = 0x00; cave[i++] = 0x80; cave[i++] = 0x3F;

    /* MOV DWORD [ESP + 0x58], 0x3F800000  (G = 1.0) */
    cave[i++] = 0xC7; cave[i++] = 0x44; cave[i++] = 0x24; cave[i++] = 0x58;
    cave[i++] = 0x00; cave[i++] = 0x00; cave[i++] = 0x80; cave[i++] = 0x3F;

    /* MOV DWORD [ESP + 0x5C], 0x3F800000  (B = 1.0) */
    cave[i++] = 0xC7; cave[i++] = 0x44; cave[i++] = 0x24; cave[i++] = 0x5C;
    cave[i++] = 0x00; cave[i++] = 0x00; cave[i++] = 0x80; cave[i++] = 0x3F;

    /* MOV DWORD [ESP + 0x60], 0x3F800000  (A = 1.0) */
    cave[i++] = 0xC7; cave[i++] = 0x44; cave[i++] = 0x24; cave[i++] = 0x60;
    cave[i++] = 0x00; cave[i++] = 0x00; cave[i++] = 0x80; cave[i++] = 0x3F;

    /* done: */
    int done_pos = i;

    /* POP EAX, POP ECX, POP EDX */
    cave[i++] = 0x58;  /* POP EAX */
    cave[i++] = 0x59;  /* POP ECX */
    cave[i++] = 0x5A;  /* POP EDX */

    /* FNSTSW AX */
    cave[i++] = 0xDF; cave[i++] = 0xE0;

    /* TEST AH, 0x44 */
    cave[i++] = 0xF6; cave[i++] = 0xC4; cave[i++] = 0x44;

    /* JMP 0x00412ECB (rel32) */
    DWORD returnAddr = 0x00412ECB;
    DWORD jmpTarget = returnAddr - (caveAddr + i + 5);
    cave[i++] = 0xE9;
    cave[i++] = (jmpTarget) & 0xFF;
    cave[i++] = (jmpTarget >> 8) & 0xFF;
    cave[i++] = (jmpTarget >> 16) & 0xFF;
    cave[i++] = (jmpTarget >> 24) & 0xFF;

    /* Patch JZ offsets (short jumps = relative to next instruction) */
    cave[jz1_offset + 1] = (unsigned char)(fallback_pos - (jz1_offset + 2));
    cave[jz2_offset + 1] = (unsigned char)(fallback_pos - (jz2_offset + 2));
    cave[jmp_done_offset + 1] = (unsigned char)(done_pos - (jmp_done_offset + 2));
}

/* Apply the patch using PatchMemory from HB+ API */
static void applyPatch(HBPlusAPI hb) {
    if (g_patched) return;

    DWORD gameBase = hb.GetGameBaseAddress();
    /* Hook address is relative to game base (0x400000) */
    DWORD hookAddr = gameBase + 0x0012EA6;

    /* Save original bytes */
    /* We can't read game memory directly from the DLL without the API.
       Instead, we use VirtualAlloc + memcpy to create the code cave,
       then use PatchMemory to write the JMP at the hook site. */

    /* Allocate executable memory for code cave */
    DWORD caveSize = 256;
    unsigned char* cave = (unsigned char*)VirtualAlloc(
        NULL, caveSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!cave) return;

    g_caveAddr = (DWORD)(uintptr_t)cave;

    /* Build the code cave */
    buildCodeCave(cave, g_caveAddr);

    /* Now patch the hook site: JMP to code cave + NOPs */
    unsigned char patch[37];
    DWORD jmpOffset = g_caveAddr - (hookAddr + 5);
    patch[0] = 0xE9;  /* JMP rel32 */
    patch[1] = jmpOffset & 0xFF;
    patch[2] = (jmpOffset >> 8) & 0xFF;
    patch[3] = (jmpOffset >> 16) & 0xFF;
    patch[4] = (jmpOffset >> 24) & 0xFF;
    for (int k = 5; k < 37; k++) patch[k] = 0x90; /* NOP */

    /* Use PatchMemory to write the hook */
    hb.PatchMemory(hookAddr, (const char*)patch, 37);

    g_patched = true;
}

/* ===== HB+ vtable ===== */
static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Bumper Colors"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_api = modApi;
}

static void __thiscall game_update_impl(void* thisptr) {
    if (g_patched) return;
    if (!g_api) return;

    /* Check if toggle is enabled */
    HBPlusAPI hb = { g_api };
    bool enabled = hb.GetButtonState("mkn_bumper_colors_enabled");
    if (!enabled) return;

    /* Apply patch once */
    applyPatch(hb);
}

static void __thiscall button_toggle_impl(void* thisptr, const char* buttonId, bool newState) {
    if (g_api && buttonId) {
        /* Compare button ID manually (no strcmp in nocrt) */
        const char* id = "mkn_bumper_colors_enabled";
        const char* p = buttonId;
        while (*id && *p && *id == *p) { id++; p++; }
        if (*id == 0 && *p == 0) {
            if (newState && !g_patched) {
                HBPlusAPI hb = { g_api };
                applyPatch(hb);
            }
            /* Note: unpatching is not supported in this version.
               Once patched, the code cave stays active. */
        }
    }
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

/* 17-entry vtable for HB+ v2.0/v2.1 */
static void* g_vtable[17] = {
    (void*)sc_dtor,
    (void*)get_mod_name,
    (void*)get_author,
    (void*)get_version,
    (void*)get_contributors,
    (void*)init_impl,
    (void*)ball_update_impl,
    (void*)render_apply_impl,
    (void*)button_toggle_impl,
    (void*)slider_change_impl,
    (void*)cycle_change_impl,
    (void*)game_update_impl,
    (void*)event_collide_impl,
    (void*)text_render_impl,
    (void*)ball_bump_impl,
    (void*)scene_end_impl,
    (void*)level_start_impl,
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

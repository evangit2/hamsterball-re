/*
 * light_platforms.c — Light Platforms mod for Hamsterball
 *
 * Hijacks Neon Race's NeonPlatform appear/disappear system.
 * Instead of the timer driving platform visibility, an external flag controls it.
 * When merged with Electric Lights, the flag = "ball lights are on" (charge > 0).
 *
 * NeonPlatform update (vtable[11] = 0x0043E260, __thiscall):
 *   obj+0x10E4 = active flag (byte: 1=updating, 0=idle) — CHECKED FIRST
 *   obj+0x10E5 = direction  (byte: 0=appearing, 1=disappearing)
 *   obj+0x10DC = Y position  (float, moved ±4.0/frame)
 *   obj+0x10E8 = tick counter (int, +4 appearing / -4 disappearing, resets at 300/0)
 *   obj+0x10D8 = X position  (float, ball proximity check)
 *   obj+0x10E0 = Z position  (float, ball proximity check)
 *   obj+0x10D0 = board pointer
 *   Ball proximity radius = 22.0 units (double at 0x4D5D18)
 *   When disappearing + ball within 22 units X+Z: ball+0x168 (Y vel) -= 4.0
 *   When tick resets: Y += 100.0, direction flips, tick += 100
 *
 * NeonPlatform vtable = 0x004D5A10
 * NeonPlatform objects stored in board+0x2578 AthenaList
 *
 * Hook: Graphics_RenderScene entry (0x454BC0) — same as Electric Lights.
 *   Runs before board update, so direction flag is set before platforms update.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll light_platforms.c \
 *     -I"../_WIP 🔨 │ shared" -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *     -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include "bass_proxy.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Addresses
 * ═══════════════════════════════════════════════════════════════════════════ */

#define RENDER_SCENE_HOOK    0x00454BC0
#define RENDER_SCENE_ORIG    0x00454BC6
#define RENDER_SCENE_ORIG_BYTES 6

#define NEONPLATFORM_VTABLE   0x004D5A10

/* Board offsets */
#define BOARD_DYNOBJ_LIST    0x2578   /* AthenaList of dynamic objects */
#define ATHENALIST_COUNT     0x04     /* count at +0x04 (inline) */
#define ATHENALIST_ITEMS     0x40C    /* items array at +0x40C */

/* NeonPlatform offsets — VERIFIED via disassembly of 0x0043E260 */
#define NP_ACTIVE             0x10E4   /* byte: 1=updating, 0=idle (checked first) */
#define NP_DIRECTION          0x10E5   /* byte: 0=appearing, 1=disappearing */

/* Scene pointer for getting board */
#define SCENE_PTR             0x005341E4
#define SCENE_LEVEL_PTR       0x8AC    /* scene+0x8AC = level ptr */
#define LEVEL_BOARD_PTR       0x08     /* level+0x08 = board ptr... actually */

/* Actually: scene+0x8AC = level, but we need board.
   Board = scene itself for race levels. Or use App.
   Let me use App: App+0x178 = board */
#define APP_PTR               0x005341E0
#define APP_BOARD             0x178

/* ═══════════════════════════════════════════════════════════════════════════
 * Configuration
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Toggle period for testing (in frames at 60fps) */
#define TOGGLE_PERIOD_FRAMES  600   /* 10 seconds on, 10 seconds off */

/* ═══════════════════════════════════════════════════════════════════════════
 * Mod State
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE  g_orig_bytes[6];
static int   g_frame_count = 0;

/* External flag: 1=platforms visible, 0=platforms hidden
   When merged with Electric Lights, this = (charge > 0) */
static int   g_platforms_visible = 1;

/* ═══════════════════════════════════════════════════════════════════════════
 * Update NeonPlatform Objects
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_platforms(void) {
    /* Get board from App */
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x200)) return;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board || IsBadReadPtr((void*)board, 0x2600)) return;

    /* Get dynamic objects AthenaList at board+0x2578 */
    DWORD listBase = board + BOARD_DYNOBJ_LIST;
    if (IsBadReadPtr((void*)listBase, 0x410)) return;

    /* AthenaList: count at +0x04, items at +0x40C */
    int count = *(int*)(listBase + ATHENALIST_COUNT);
    if (count < 1 || count > 500) return;

    DWORD items = *(DWORD*)(listBase + ATHENALIST_ITEMS);
    if (!items || IsBadReadPtr((void*)items, count * 4)) return;

    /* Iterate all dynamic objects, find NeonPlatforms */
    for (int i = 0; i < count; i++) {
        DWORD obj = *(DWORD*)(items + i * 4);
        if (!obj || IsBadReadPtr((void*)obj, 4)) continue;

        DWORD vtable = *(DWORD*)obj;
        if (vtable != NEONPLATFORM_VTABLE) continue;

        /* Found a NeonPlatform — set direction based on flag */
        if (IsBadReadPtr((void*)(obj + NP_DIRECTION), 1)) continue;

        if (g_platforms_visible) {
            /* Make platform appear: direction=0 (appearing) */
            *(BYTE*)(obj + NP_DIRECTION) = 0;
            /* Ensure active flag is set so update runs */
            *(BYTE*)(obj + NP_ACTIVE) = 1;
        } else {
            /* Make platform disappear: direction=1 (disappearing) */
            *(BYTE*)(obj + NP_DIRECTION) = 1;
            *(BYTE*)(obj + NP_ACTIVE) = 1;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-Frame Hook
 * ═══════════════════════════════════════════════════════════════════════════ */

extern void hook_cave_asm(void);

/* At hook entry: ECX = gfx (Graphics_RenderScene this pointer)
   Original instruction: SUB ESP, 0xC0 (6 bytes) */
__asm__(
    ".global _hook_cave_asm\n"
    "_hook_cave_asm:\n"
    "    pushal\n"
    "    pushfl\n"
    "    call _on_render_scene\n"
    "    popfl\n"
    "    popal\n"
    "    subl $0xc0, %esp\n"    /* Original instruction */
    "    jmp 0x00454BC6\n"       /* Jump back */
);

void on_render_scene(void) {
    /* Toggle flag for testing */
    g_frame_count++;
    if ((g_frame_count / TOGGLE_PERIOD_FRAMES) % 2 == 0) {
        g_platforms_visible = 1;
    } else {
        g_platforms_visible = 0;
    }

    update_platforms();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_hooks(void) {
    memcpy(g_orig_bytes, (void*)RENDER_SCENE_HOOK, RENDER_SCENE_ORIG_BYTES);
    install_jmp_hook_nop(RENDER_SCENE_HOOK, (DWORD)hook_cave_asm, RENDER_SCENE_ORIG_BYTES);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        install_hooks();
    }
    return TRUE;
}

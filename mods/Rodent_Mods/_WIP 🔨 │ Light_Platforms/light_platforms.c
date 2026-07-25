/*
 * light_platforms.c — Light Platforms mod for Hamsterball
 *
 * Replaces ArenaStands (DFLOOR1-4) blink timer with the Neon light state.
 * When the Neon Race light is ON, platforms stay visible.
 * When the light is OFF, platforms stay invisible.
 *
 * ArenaStands (DFLOOR) — vtable 0x4D5A70, ctor 0x43E450, size 0x1104
 *   +0x10DC = state (0-3: 0,1=visible / 2,3=invisible)
 *   +0x10E0 = timer (75 frames per state, 0x4B)
 *   +0x10E4 = board pointer
 *   +0x10E8 = render object (Level_RenderCtor result)
 *   +0x1100 = needs_readd flag (1=should be added back to render list)
 *
 * Native state machine (vtable[11] = 0x4373E0):
 *   State 0 (visible): timer-- → 0: state=1, ADD to render list, sound
 *   State 1 (visible): timer-- → 0: state=2, REMOVE from render list
 *   State 2 (invisible): timer-- → 0: state=3, ADD to render list, sound
 *   State 3 (invisible): timer-- → 0: state=0
 *
 * Mod approach: override state+timer every frame so the timer never reaches 0.
 *   Light ON  → state=0, timer=75 (visible, never transitions)
 *   Light OFF → state=2, timer=75 (invisible, never transitions)
 *
 * Light object at board+0x436C (SceneObject, 0xD4 bytes)
 *   +0x88 = visible flag (1=on, 0=off)
 *   Set by E:LIGHTSON (→1) / E:LIGHTSOFF (→0) collision events
 *
 * Hook: Graphics_RenderScene entry (0x454BC0)
 *   Runs AFTER Board_UpdateRaceState (which calls vtable[11]).
 *   So we override state AFTER the native timer tick — next frame
 *   vtable[11] decrements from 75 to 74, never reaches 0, no transition.
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

#define RENDER_SCENE_HOOK      0x00454BC0
#define RENDER_SCENE_ORIG      0x00454BC6
#define RENDER_SCENE_ORIG_BYTES 6

#define ARENASTANDS_VTABLE     0x004D5A70

/* Board offsets */
#define BOARD_DYNOBJ_LIST      0x2578   /* AthenaList of dynamic objects */
#define BOARD_LIGHT_P0         0x436C   /* Light SceneObject pointer (player 0) */
#define ATHENALIST_COUNT       0x04
#define ATHENALIST_ITEMS       0x40C

/* ArenaStands struct offsets */
#define AS_STATE               0x10DC   /* int: 0-3 */
#define AS_TIMER               0x10E0   /* int: countdown (75=0x4B per state) */
#define AS_BOARD               0x10E4   /* DWORD: board pointer */
#define AS_RENDER_OBJ          0x10E8   /* DWORD: Level_RenderCtor result */
#define AS_NEEDS_READD         0x1100   /* byte: 1=should be re-added to render list */

/* Light SceneObject offsets */
#define LIGHT_VISIBLE          0x88     /* byte: 1=on, 0=off */

/* Scene/board access */
#define APP_PTR                 0x005341E0
#define APP_BOARD               0x178

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-Frame Update
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_platforms(void) {
    /* Get board from App */
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x200)) return;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;

    /* Read light state: board+0x436C → SceneObject+0x88 */
    DWORD light = *(DWORD*)(board + BOARD_LIGHT_P0);
    if (!light || IsBadReadPtr((void*)light, 0xD4)) return;
    BYTE light_on = *(BYTE*)(light + LIGHT_VISIBLE);

    /* Iterate dynamic objects AthenaList at board+0x2578 */
    DWORD listBase = board + BOARD_DYNOBJ_LIST;
    if (IsBadReadPtr((void*)listBase, 0x410)) return;

    int count = *(int*)(listBase + ATHENALIST_COUNT);
    if (count < 1 || count > 500) return;

    DWORD items = *(DWORD*)(listBase + ATHENALIST_ITEMS);
    if (!items || IsBadReadPtr((void*)items, count * 4)) return;

    for (int i = 0; i < count; i++) {
        DWORD obj = *(DWORD*)(items + i * 4);
        if (!obj || IsBadReadPtr((void*)obj, 4)) continue;

        DWORD vtable = *(DWORD*)obj;
        if (vtable != ARENASTANDS_VTABLE) continue;

        /* Found an ArenaStands object — override state machine */
        if (IsBadReadPtr((void*)(obj + AS_STATE), 4)) continue;

        if (light_on) {
            /* Light is ON → keep platform visible
             * State 0 = visible (in render list), timer=75 prevents transition */
            *(int*)(obj + AS_STATE) = 0;
            *(int*)(obj + AS_TIMER) = 0x4B;  /* 75 */
        } else {
            /* Light is OFF → keep platform invisible
             * State 2 = invisible (removed from render list), timer=75 prevents transition */
            *(int*)(obj + AS_STATE) = 2;
            *(int*)(obj + AS_TIMER) = 0x4B;  /* 75 */
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
    update_platforms();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_hooks(void) {
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

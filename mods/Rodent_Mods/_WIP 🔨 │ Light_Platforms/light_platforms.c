/*
 * light_platforms.c — Light Platforms mod for Hamsterball
 *
 * Replaces ArenaStands (DFLOOR1-4) blink timer with the Neon light state.
 * When the Neon Race light is ON, platforms stay visible.
 * When the light is OFF, platforms stay invisible.
 *
 * FLICKER: When transitioning between visible/invisible, routes through
 * the native flicker states (1 and 3) instead of jumping directly.
 * The ToggleTimer at obj+0x10EC controls the visual flicker by toggling
 * a visible flag every 100 frames. During states 1 and 3, the render
 * function (vtable slot 18 = 0x437560) skips rendering when the flag is 0.
 * The object stays in the render list during flicker, so collision
 * remains active (platforms are "physically tangible" while flickering).
 *
 * ArenaStands (DFLOOR) — vtable 0x4D5A70, ctor 0x43E450, size 0x1104
 *   +0x10DC = state (0-3)
 *     0 = solid visible (stable)
 *     1 = flicker before disappearing (→ 2)
 *     2 = invisible (stable, removed from render list)
 *     3 = flicker after reappearing (→ 0)
 *   +0x10E0 = timer (75 frames per state, 0x4B)
 *   +0x10E4 = board pointer
 *   +0x10E8 = render object (Level_RenderCtor result)
 *   +0x10EC = ToggleTimer struct (20 bytes)
 *     +0x10F0 = visible flag (byte, toggles every `period` frames)
 *     +0x10F4 = period (int, init 100)
 *     +0x10F8 = counter (int)
 *     +0x10FC = just_toggled (byte)
 *   +0x1100 = needs_readd flag (1=should be added back to render list)
 *
 * Native state machine (vtable[11] = 0x4373E0):
 *   State 0: timer-- → 0: state=1, ADD to render list (if needs_readd), sound
 *   State 1: timer-- → 0: state=2, REMOVE from render list, set needs_readd=1
 *   State 2: timer-- → 0: state=3, ADD to render list (if needs_readd), sound
 *   State 3: timer-- → 0: state=0 (wraps), no render list change
 *
 * Render function (vtable slot 18 = 0x437560):
 *   Skips D3DXSkinMesh_CopyStripData when:
 *   - state == 2 (fully invisible)
 *   - state == 1 or 3 AND ToggleTimer.visible == 0 (flicker)
 *
 * Mod approach (normal — DFLOOR1, 2, 4):
 *   Light ON  + state 2: set state=3, timer=75 (start flicker → reappear)
 *   Light ON  + state 0: pin timer=75 (stay visible, no transition)
 *   Light ON  + state 3: let it run (flicker in progress)
 *   Light OFF + state 0: set state=1, timer=75 (start flicker → disappear)
 *   Light OFF + state 2: pin timer=75 (stay invisible, no transition)
 *   Light OFF + state 1: let it run (flicker in progress)
 *
 * Mod approach (INVERTED — DFLOOR3 only, NO flicker):
 *   Light OFF + state 2: set state=3, timer=1, TT.visible=1 (instant reappear)
 *   Light OFF + state 0: pin timer=75 (stay visible, no transition)
 *   Light OFF + state 3: force TT.visible=1 (no flicker, fast transition)
 *   Light ON  + state 0: set state=1, timer=1, TT.visible=1 (instant disappear)
 *   Light ON  + state 2: pin timer=75 (stay invisible, no transition)
 *   Light ON  + state 1: force TT.visible=1 (no flicker, fast transition)
 *
 * DFLOOR3 identified by mesh pointer match: board+0x4380 holds DFLOOR3's
 * mesh Level. Stands_ctor copies *(mesh+0x08) (MeshWorld ptr) to obj+0x08.
 * We compare obj+0x08 against *(*(board+0x4380)+0x08) to detect DFLOOR3.
 *
 * Light object at board+0x436C (SceneObject, 0xD4 bytes)
 *   +0x88 = visible flag (1=on, 0=off)
 *   Set by E:LIGHTSON (→1) / E:LIGHTSOFF (→0) collision events
 *
 * Hook: Graphics_RenderScene entry (0x454BC0)
 *   Runs AFTER Board_UpdateRaceState (which calls vtable[11]).
 *   So we override state AFTER the native timer tick.
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
#define AS_TOGGLE_TIMER        0x10EC   /* ToggleTimer struct (20 bytes) */
#define AS_TT_VISIBLE          0x10F0   /* byte: ToggleTimer visible flag */
#define AS_TT_PERIOD           0x10F4   /* int: toggle period (100) */
#define AS_TT_COUNTER          0x10F8   /* int: counter */
#define AS_TT_JUST_TOGGLED     0x10FC   /* byte: just toggled this frame */
#define AS_NEEDS_READD         0x1100   /* byte: 1=should be re-added to render list */

/* Light SceneObject offsets */
#define LIGHT_VISIBLE          0x88     /* byte: 1=on, 0=off */

/* Scene/board access */
#define APP_PTR                 0x005341E0
#define APP_BOARD               0x178

/* State machine constants */
#define STATE_SOLID             0   /* stable visible */
#define STATE_FLICKER_DOWN      1   /* flicker before disappearing */
#define STATE_INVISIBLE         2   /* stable invisible */
#define STATE_FLICKER_UP        3   /* flicker after reappearing */
#define TIMER_FULL              0x4B /* 75 frames */

/* DFLOOR3 identified by matching its mesh against board+0x4380 (DFLOOR3 mesh slot).
 * Each DFLOOR uses a different mesh Level stored at board+0x4378+N.
 * ArenaStands_ctor passes the mesh to Stands_ctor, which copies
 * *(mesh+0x08) (MeshWorld ptr) to obj+0x08. We compare obj+0x08
 * against *(*(board+0x4380)+0x08) to identify DFLOOR3. */

/* Board mesh slots — each DFLOOR's mesh Level object */
#define BOARD_DFLOOR3_MESH     0x4380   /* board+0x4380 = DFLOOR3 mesh Level ptr */

/* ═══════════════════════════════════════════════════════════════════════════
 * N:DISCHARGE — Energy Gate
 * ═══════════════════════════════════════════════════════════════════════════
 * When the ball touches an N:DISCHARGE mesh AND charge > 0 (light is on):
 *   1. Zero the charge (turn off neon light)
 *   2. Set g_discharged flag
 *   3. Force ALL DFloors to instant transition (no flicker)
 *      Light DFloors (1,2,4) → invisible (state 2)
 *      Dark DFloor (3) → visible (state 0)
 * If charge == 0 (light already off): does nothing.
 *
 * MeshBuffer name scanning: MeshWorld+0x2C → AthenaList → each MB+0x864 = name.
 * Ball proximity check against MB position (+0x868/86C/870).
 */

/* Ball access */
#define APP_BALL               0x5DC    /* App+0x5DC = player 1 ball pointer */
#define BALL_POS_X             0x164
#define BALL_POS_Y             0x168
#define BALL_POS_Z             0x16C

/* MeshWorld access */
#define BOARD_LEVEL            0x8AC    /* board+0x8AC = Level ptr */
#define LEVEL_MESHWORLD        0x08     /* Level+0x08 = MeshWorld ptr */
#define MW_MESHBUFFER_LIST     0x2C     /* MeshWorld+0x2C = AthenaList of MBs */
#define MB_NAME                0x864    /* MeshBuffer+0x864 = char* name */
#define MB_POS_X               0x868
#define MB_POS_Y               0x86C
#define MB_POS_Z               0x870

#define DISCHARGE_PROXIMITY    80.0f    /* ball radius 26, 80 = generous touch */

/* Discharge state */
static int g_discharged = 0;             /* 1 = discharge has fired */
static int g_discharge_cooldown = 0;     /* frames until ball leaves mesh */

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-Frame Update
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Check if ball is touching any N:DISCHARGE mesh.
 * Returns 1 if touching, 0 if not. */
static int check_discharge_collision(DWORD board, DWORD app) {
    DWORD ball = *(DWORD*)(app + APP_BALL);
    if (!ball || IsBadReadPtr((void*)ball, 0x200)) return 0;

    float bx = *(float*)(ball + BALL_POS_X);
    float by = *(float*)(ball + BALL_POS_Y);
    float bz = *(float*)(ball + BALL_POS_Z);

    /* Get MeshWorld: board+0x8AC → Level → Level+0x08 = MeshWorld */
    DWORD level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || IsBadReadPtr((void*)level, 0x10)) return 0;
    DWORD mw = *(DWORD*)(level + LEVEL_MESHWORLD);
    if (!mw || IsBadReadPtr((void*)mw, 0x40)) return 0;

    /* Get MeshBuffer AthenaList at MeshWorld+0x2C */
    DWORD mbList = mw + MW_MESHBUFFER_LIST;
    if (IsBadReadPtr((void*)mbList, 0x410)) return 0;
    int mbCount = *(int*)(mbList + ATHENALIST_COUNT);
    if (mbCount < 1 || mbCount > 500) return 0;
    DWORD mbItems = *(DWORD*)(mbList + ATHENALIST_ITEMS);
    if (!mbItems || IsBadReadPtr((void*)mbItems, mbCount * 4)) return 0;

    for (int i = 0; i < mbCount; i++) {
        DWORD mb = *(DWORD*)(mbItems + i * 4);
        if (!mb || IsBadReadPtr((void*)mb, 0x874)) continue;

        char *name = *(char**)(mb + MB_NAME);
        if (!name || IsBadReadPtr(name, 12)) continue;

        /* Check for N:DISCHARGE prefix */
        if (_strnicmp(name, "N:DISCHARGE", 12) != 0) continue;

        /* Found a discharge mesh — check ball proximity */
        float mx = *(float*)(mb + MB_POS_X);
        float my = *(float*)(mb + MB_POS_Y);
        float mz = *(float*)(mb + MB_POS_Z);
        float dx = bx - mx, dy = by - my, dz = bz - mz;
        float dist2 = dx*dx + dy*dy + dz*dz;

        if (dist2 < DISCHARGE_PROXIMITY * DISCHARGE_PROXIMITY)
            return 1;
    }

    return 0;
}

static void update_platforms(void) {
    /* Get board from App */
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x200)) return;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;

    /* Read light state: board+0x436C -> SceneObject+0x88 */
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

        /* Found an ArenaStands object */
        if (IsBadReadPtr((void*)(obj + AS_STATE), 4)) continue;

        int state = *(int*)(obj + AS_STATE);

        /* Check if this is DFLOOR3 (inverted logic — solid in dark) */
        /* DFLOOR3's mesh is at board+0x4380. Stands_ctor copies *(mesh+0x08)
         * to obj+0x08. Compare to identify DFLOOR3. */
        DWORD dfloor3_mesh = *(DWORD*)(board + BOARD_DFLOOR3_MESH);
        DWORD dfloor3_mw = 0;
        if (dfloor3_mesh && !IsBadReadPtr((void*)dfloor3_mesh, 0x10))
            dfloor3_mw = *(DWORD*)(dfloor3_mesh + 0x08);

        DWORD obj_mw = *(DWORD*)(obj + 0x08);
        int is_inverted = (dfloor3_mw && obj_mw == dfloor3_mw);

        /* For inverted platforms, flip the light state */
        BYTE want_visible = is_inverted ? !light_on : light_on;

        /* If discharge just fired, force ALL DFloors to instant transition
         * (no flicker) — same as DFLOOR3's no-flicker approach. */
        int force_instant = g_discharged;

        /* DFLOOR3 (inverted): skip flicker entirely.
         * Use timer=1 so the flicker state lasts only 1 frame, and force
         * ToggleTimer.visible=1 during that frame so the render function
         * draws the object normally — no visible flicker at all.
         * The native state machine still handles render list add/remove. */
        int flicker_timer = (is_inverted || force_instant) ? 1 : TIMER_FULL;

        if (want_visible) {
            /* Want platform visible (normal: light on, inverted: light off) */
            switch (state) {
            case STATE_INVISIBLE:
                /* Currently invisible — start flicker to reappear */
                *(int*)(obj + AS_STATE) = STATE_FLICKER_UP;  /* state 3 */
                *(int*)(obj + AS_TIMER) = flicker_timer;
                if (is_inverted || force_instant)
                    *(BYTE*)(obj + AS_TT_VISIBLE) = 1;  /* no visual flicker */
                break;
            case STATE_FLICKER_UP:
                /* Flicker in progress — for inverted/instant, force visible + let it
                 * transition quickly. For normal, let it run naturally. */
                if (is_inverted || force_instant) {
                    *(BYTE*)(obj + AS_TT_VISIBLE) = 1;
                }
                break;
            case STATE_SOLID:
            default:
                /* Stable visible — pin timer to prevent transition */
                *(int*)(obj + AS_TIMER) = TIMER_FULL;
                break;
            }
        } else {
            /* Want platform invisible (normal: light off, inverted: light on) */
            switch (state) {
            case STATE_SOLID:
                /* Currently visible — start flicker to disappear */
                *(int*)(obj + AS_STATE) = STATE_FLICKER_DOWN;  /* state 1 */
                *(int*)(obj + AS_TIMER) = flicker_timer;
                if (is_inverted || force_instant)
                    *(BYTE*)(obj + AS_TT_VISIBLE) = 1;  /* no visual flicker */
                break;
            case STATE_FLICKER_DOWN:
                /* Flicker in progress — for inverted/instant, force visible + let it
                 * transition quickly. For normal, let it run naturally. */
                if (is_inverted || force_instant) {
                    *(BYTE*)(obj + AS_TT_VISIBLE) = 1;
                }
                break;
            case STATE_INVISIBLE:
            default:
                /* Stable invisible — pin timer to prevent transition */
                *(int*)(obj + AS_TIMER) = TIMER_FULL;
                break;
            }
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
    /* Check N:DISCHARGE collision */
    DWORD app = *(DWORD*)APP_PTR;
    if (app && !IsBadReadPtr((void*)app, 0x200)) {
        DWORD board = *(DWORD*)(app + APP_BOARD);
        if (board && !IsBadReadPtr((void*)board, 0x4400)) {
            int touching = check_discharge_collision(board, app);

            if (touching && g_discharge_cooldown > 0)
                g_discharge_cooldown--;

            if (touching && g_discharge_cooldown == 0) {
                /* Ball is touching a discharge mesh */
                DWORD light = *(DWORD*)(board + BOARD_LIGHT_P0);
                if (light && !IsBadReadPtr((void*)light, 0xD4)) {
                    BYTE light_on = *(BYTE*)(light + LIGHT_VISIBLE);
                    if (light_on) {
                        /* Charge > 0 — DISCHARGE! */
                        *(BYTE*)(light + LIGHT_VISIBLE) = 0;  /* turn off light */
                        g_discharged = 1;
                        g_discharge_cooldown = 300;  /* 5s cooldown */
                    }
                }
            }

            /* Reset discharge flag once all platforms have settled */
            /* (g_discharged stays 1 until ball leaves mesh, then clears) */
            if (!touching && g_discharge_cooldown > 0)
                g_discharge_cooldown--;
            if (!touching && g_discharge_cooldown == 0 && g_discharged) {
                /* Ball left the discharge zone — keep flag set so platforms
                 * stay in their discharged state until light turns back on */
                /* g_discharged stays 1 — it's only cleared by external
                 * charge restoration (Phase 2 recharge pads, or future
                 * Electric Lights integration). For now, it's permanent. */
            }
        }
    }

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

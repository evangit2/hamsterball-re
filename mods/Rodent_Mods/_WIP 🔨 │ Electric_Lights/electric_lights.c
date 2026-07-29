/*
 * electric_lights.c — Electric Lights + Light Platforms merged mod
 * v6: Fixed DFLOOR4 handling + ball color restoration on level change/unload
 *
 * Phase 1+2: Charge system + ball glow + D3D light + platform flicker
 *
 * CHARGE SYSTEM:
 *   Ball starts with full charge (1.0). Charge drains over time.
 *   Ball collision with N:CHARGE mesh restores charge to full.
 *   Ball collision with N:DISCHARGE mesh zeros charge instantly.
 *   D3D point light follows the ball, range scales with charge.
 *   Ball color multiplier scales with charge for visual glow.
 *
 * PLATFORM FLICKER:
 *   ArenaStands (DFLOOR) platforms flicker based on charge level:
 *   - Charge drops below FLICKER_OUT_THRESHOLD -> start flicker DOWN
 *     (flicker for 75 frames, then platform becomes invisible)
 *     Threshold is set so flicker completes before charge hits 0.
 *   - Ball touches N:CHARGE (charge restored) -> start flicker UP
 *     (flicker for 75 frames, then platform becomes solid)
 *   - During flicker: native ToggleTimer controls visual on/off.
 *     Object stays in render list -> collision active during flicker.
 *
 * ARENASTANDS STATE MACHINE:
 *   State 0: solid visible (stable)
 *   State 1: flicker before disappearing (-> 2 after 75 frames)
 *   State 2: invisible (stable, removed from render list)
 *   State 3: flicker after reappearing (-> 0 after 75 frames)
 *
 * COLLISION DETECTION (v5):
 *   Hooks the two call sites where Ball_Update calls vtable[29]
 *   (DispatchCollision) — at 0x40728F and 0x408B85.
 *   Original bytes at both sites: 52 56 FF 50 74
 *     (PUSH EDX=collObj; PUSH ESI=ball; CALL [EAX+0x74])
 *   The cave saves registers, calls C handler with (ball, collObj),
 *   restores, re-executes the original 5 bytes, jumps back.
 *   No SEH, no trampoline — we hook the CALLER, not the callee.
 *
 * Hook: Graphics_RenderScene entry (0x454BC0)
 *   ECX = gfx struct pointer at entry
 *   gfx+0x154 = IDirect3DDevice8*
 *   gfx+0x854/858/85C = ball position (set by Scene_Render)
 *
 * v4 CHANGES:
 *   - Fixed ArenaStands render-list re-add: setting state=3 directly from
 *     state=2 skipped the native 2→3 transition that ADDS the object back
 *     to the render+collision list. Now sets timer=1 in state 2 so the
 *     native state machine does the ADD itself.
 *
 * v5 CHANGES:
 *   - Replaced proximity-based N:CHARGE/N:DISCHARGE scanning with real
 *     collision dispatch hooks at 0x40728F + 0x408B85.
 *   - N:CHARGE and N:DISCHARGE now fire as actual collision events,
 *     not distance checks.
 *   - Removed scan_charge_pads(), check_discharge_collision(), and
 *     all proximity constants.
 *
 * v6 CHANGES:
 *   - FIXED: DFLOOR4 was missing from platform flicker. Unlike DFLOOR1-3+TRODE
 *     which are in board+0x2578 AthenaList, DFLOOR4 is stored separately at
 *     board+0x438C. Added dedicated handling for it.
 *   - FIXED: Ball color was not restored on level change or DLL unload.
 *     Added restore_ball_color() — called on board change (new race) and
 *     DLL_PROCESS_DETACH. Restores original color multiplier + alpha=1.0.
 *   - NOTE: Ball becoming invisible at charge=0 is INTENTIONAL (feature,
 *     not bug). The ball alpha is tied to charge level by design.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll electric_lights.c \
 *     -I"../../_WIP 🔨 │ shared" -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *     -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include "bass_proxy.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Addresses
 * ═══════════════════════════════════════════════════════════════════════════ */

#define RENDER_SCENE_HOOK    0x00454BC0
#define RENDER_SCENE_ORIG    0x00454BC6
#define RENDER_SCENE_ORIG_BYTES 6

/* Collision dispatch call sites in Ball_Update.
 * Both are identical: 52 56 FF 50 74
 *   PUSH EDX (collObj); PUSH ESI (ball); CALL [EAX+0x74] (vtable[29])
 * At hook entry: ESI=ball, EDX=collObj, EAX=board vtable ptr.
 * Mesh name is at: [collObj+4] -> sceneobj+0x864 */
#define COLLISION_HOOK_1     0x0040728F
#define COLLISION_RETURN_1   0x00407294
#define COLLISION_HOOK_2     0x00408B85
#define COLLISION_RETURN_2   0x00408B8A
#define COLLISION_ORIG_BYTES 5

/* gfx struct offsets */
#define GFX_D3D_DEVICE       0x154
#define GFX_AMBIENT          0x730
#define GFX_BALL_X           0x854
#define GFX_BALL_Y           0x858
#define GFX_BALL_Z           0x85C

/* D3D8 device vtable offsets */
#define D3D_SetLight         0xB0    /* vtable[44] */
#define D3D_LightEnable      0xB8    /* vtable[46] */
#define D3D_SetRenderState   0xC8    /* vtable[50] */

/* D3D render state IDs */
#define D3DRS_LIGHTING       0x89
#define D3DRS_AMBIENT        0x8B

/* Ball field offsets */
#define BALL_COLOR_R         0x2AC
#define BALL_COLOR_G         0x2B0
#define BALL_COLOR_B         0x2B4
#define BALL_ALPHA           0x2FC

/* App pointer */
#define GLOBAL_APP_PTR       0x005341E0
#define APP_BALL_P1          0x5DC
#define APP_BOARD            0x178
#define APP_QUIT_FLAG        0x159

/* Board offsets */
#define BOARD_DYNOBJ_LIST    0x2578
#define BOARD_DFOOR4_PTR     0x438C  /* DFLOOR4 stored separately, not in AthenaList */

/* AthenaList offsets */
#define ATHENALIST_COUNT      0x04
#define ATHENALIST_ITEMS      0x40C

/* collObj layout: [0]=MeshBuffer, [4]=sceneobj (has name at +0x864) */
#define COLLOBJ_SCENEOBJ     0x04
#define SCENEOBJ_NAME        0x864

/* ArenaStands struct offsets */
#define ARENASTANDS_VTABLE   0x004D5A70
#define AS_STATE             0x10DC
#define AS_TIMER             0x10E0
#define AS_NEEDS_READD       0x1100

/* State machine constants */
#define STATE_SOLID          0
#define STATE_FLICKER_DOWN   1
#define STATE_INVISIBLE      2
#define STATE_FLICKER_UP     3
#define TIMER_FULL           0x4B  /* 75 frames */

/* ═══════════════════════════════════════════════════════════════════════════
 * Configuration
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CHARGE_DRAIN_RATE    0.001667f  /* ~10sec to deplete at 60fps */
#define CHARGE_MAX           1.0f
#define CHARGE_MIN           0.0f

/* When charge drops below this, platforms start flickering down.
 * Flicker takes 75 frames. At drain rate 0.001667/frame, charge at
 * flicker start = 0.001667 * 75 = 0.125. Use 0.15 for a small buffer
 * so flicker finishes slightly before charge hits 0. */
#define FLICKER_OUT_THRESHOLD 0.15f

#define LIGHT_FULL_RANGE     400.0f
#define LIGHT_Y_OFFSET       30.0f

/* Electric blue (#03fff2) */
#define GLOW_R               (3.0f / 255.0f)
#define GLOW_G               (255.0f / 255.0f)
#define GLOW_B               (242.0f / 255.0f)

#define LIGHT_SLOT           2

/* ═══════════════════════════════════════════════════════════════════════════
 * D3DLIGHT8 Structure (104 bytes)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    DWORD  Type;
    float  DiffuseR, DiffuseG, DiffuseB, DiffuseA;
    float  SpecularR, SpecularG, SpecularB, SpecularA;
    float  AmbientR, AmbientG, AmbientB, AmbientA;
    float  PositionX, PositionY, PositionZ;
    float  DirectionX, DirectionY, DirectionZ;
    float  Range, Falloff, Attenuation0, Attenuation1, Attenuation2;
    float  Theta, Phi;
} D3DLIGHT8;

/* ═══════════════════════════════════════════════════════════════════════════
 * Mod State
 * ═══════════════════════════════════════════════════════════════════════════ */

static float    g_charge = CHARGE_MAX;
static BYTE     g_orig_bytes[6];

/* Saved original ball color for restoration */
static float    g_orig_color_r = 1.0f;
static float    g_orig_color_g = 1.0f;
static float    g_orig_color_b = 1.0f;
static int      g_color_saved = 0;

/* Track ball pointer — reset color state when ball changes (new level) */
static DWORD    g_last_ball = 0;

/* Track board pointer — reset charge when board changes (new race) */
static DWORD    g_last_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Ball Color Restoration
 *
 * Called on level change (board change) and DLL unload to restore the
 * ball's original color multiplier and alpha values. Without this, the
 * ball keeps the mod's glow colors after the level changes or the DLL
 * is unloaded, which could make it permanently invisible (alpha=0)
 * or wrong-colored.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void restore_ball_color(void) {
    if (!g_color_saved) return;

    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x200)) return;

    DWORD ball = *(DWORD*)(app + APP_BALL_P1);
    if (!ball || IsBadReadPtr((void*)(ball + BALL_COLOR_R), 16)) return;

    *(float*)(ball + BALL_COLOR_R) = g_orig_color_r;
    *(float*)(ball + BALL_COLOR_G) = g_orig_color_g;
    *(float*)(ball + BALL_COLOR_B) = g_orig_color_b;
    *(float*)(ball + BALL_ALPHA)   = 1.0f;  /* fully visible */

    g_color_saved = 0;
    g_last_ball = 0;
}

/* N:DISCHARGE state */
static int      g_discharged = 0;          /* 1 = discharge has fired */

/* D3D light struct (pre-filled, position+range updated per frame) */
static D3DLIGHT8 g_light = {
    1,                          /* Type = D3DLIGHT_POINT */
    3.0f / 255.0f,             /* DiffuseR */
    255.0f / 255.0f,           /* DiffuseG */
    242.0f / 255.0f,           /* DiffuseB */
    0.0f,                       /* DiffuseA */
    3.0f / 255.0f,             /* SpecularR */
    255.0f / 255.0f,           /* SpecularG */
    242.0f / 255.0f,           /* SpecularB */
    0.0f,                       /* SpecularA */
    0.0f, 0.0f, 0.0f, 0.0f,    /* Ambient */
    0.0f, 0.0f, 0.0f,          /* Position */
    0.0f, 0.0f, 0.0f,          /* Direction */
    400.0f,                     /* Range */
    0.0f,                       /* Falloff */
    0.0f,                       /* Attenuation0 */
    0.04f,                      /* Attenuation1 */
    0.0f,                       /* Attenuation2 */
    0.0f, 0.0f                  /* Theta, Phi */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Shutdown Guard
 * ═══════════════════════════════════════════════════════════════════════════ */

static int game_is_quitting(void) {
    if (IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) return 1;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000) return 1;
    if (IsBadReadPtr((void*)(app + APP_QUIT_FLAG), 1)) return 1;
    return *(BYTE*)(app + APP_QUIT_FLAG) != 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Collision Dispatch Handler
 *
 * Called from both Ball_Update collision dispatch call sites.
 * At hook entry: ESI=ball, EDX=collObj.
 *   collObj+0 = MeshBuffer (collision mesh data)
 *   collObj+4 = sceneobj (sceneobj+0x864 = mesh name string)
 *
 * We read the mesh name and fire N:CHARGE / N:DISCHARGE events.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Non-static: called from inline assembly via _on_collision_dispatch symbol */
void on_collision_dispatch(DWORD ball, DWORD collObj) {
    if (game_is_quitting()) return;
    if (!ball || !collObj) return;
    if (IsBadReadPtr((void*)collObj, 8)) return;

    /* sceneobj = collObj+4, mesh name at sceneobj+0x864 */
    DWORD sceneobj = *(DWORD*)(collObj + COLLOBJ_SCENEOBJ);
    if (!sceneobj || IsBadReadPtr((void*)(sceneobj + SCENEOBJ_NAME), 4)) return;

    DWORD namePtr = *(DWORD*)(sceneobj + SCENEOBJ_NAME);
    if (!namePtr || IsBadReadPtr((void*)namePtr, 13)) return;

    const char *name = (const char*)namePtr;
    if (!name[0]) return;

    if (_strnicmp(name, "N:CHARGE", 8) == 0) {
        /* Recharge to full */
        g_charge = CHARGE_MAX;
    } else if (_strnicmp(name, "N:DISCHARGE", 12) == 0) {
        /* Zero charge + force instant platform transitions */
        if (g_charge > 0.0f) {
            g_charge = 0.0f;
            g_discharged = 1;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * D3D Light Update
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_light(DWORD gfx) {
    if (!gfx || IsBadReadPtr((void*)gfx, 0x860)) return;

    DWORD device = *(DWORD*)(gfx + GFX_D3D_DEVICE);
    if (!device || IsBadReadPtr((void*)device, 4)) return;
    DWORD vtable = *(DWORD*)device;
    if (!vtable || IsBadReadPtr((void*)vtable, 0x100)) return;

    /* Read ball position from gfx+0x854/858/85C */
    float bx = *(float*)(gfx + GFX_BALL_X);
    float by = *(float*)(gfx + GFX_BALL_Y);
    float bz = *(float*)(gfx + GFX_BALL_Z);

    if (bx == 0.0f && by == 0.0f && bz == 0.0f) return;

    /* Enable D3D lighting */
    {
        DWORD fn = *(DWORD*)(vtable + D3D_SetRenderState);
        __asm__ __volatile__(
            "pushl $1\n"
            "pushl $0x89\n"
            "movl %0, %%ecx\n"
            "call *%1\n"
            :
            : "r"(device), "r"(fn)
            : "eax", "edx", "ecx", "memory"
        );
    }

    /* Update light position to follow ball */
    g_light.PositionX = bx;
    g_light.PositionY = by + LIGHT_Y_OFFSET;
    g_light.PositionZ = bz;

    /* Scale light range based on charge */
    g_light.Range = g_charge * LIGHT_FULL_RANGE;

    /* device->SetLight(LIGHT_SLOT, &g_light) */
    {
        DWORD fn = *(DWORD*)(vtable + D3D_SetLight);
        __asm__ __volatile__(
            "pushl %0\n"
            "pushl %1\n"
            "movl %2, %%ecx\n"
            "call *%3\n"
            :
            : "r"(&g_light), "r"((DWORD)LIGHT_SLOT), "r"(device), "r"(fn)
            : "eax", "edx", "ecx", "memory"
        );
    }

    /* device->LightEnable(LIGHT_SLOT, TRUE) */
    {
        DWORD fn = *(DWORD*)(vtable + D3D_LightEnable);
        __asm__ __volatile__(
            "pushl $1\n"
            "pushl %0\n"
            "movl %1, %%ecx\n"
            "call *%2\n"
            :
            : "r"((DWORD)LIGHT_SLOT), "r"(device), "r"(fn)
            : "eax", "edx", "ecx", "memory"
        );
    }

    /* Update ball color multiplier + alpha for glow effect */
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (app && !IsBadReadPtr((void*)(app + APP_BALL_P1), 4)) {
        DWORD ball = *(DWORD*)(app + APP_BALL_P1);
        if (ball && !IsBadReadPtr((void*)(ball + BALL_COLOR_R), 16)) {
            /* Detect ball pointer change (new level/race).
             * When the ball object is recreated, save its original colors
             * so we don't write stale saved values from a freed ball. */
            if (ball != g_last_ball) {
                g_orig_color_r = *(float*)(ball + BALL_COLOR_R);
                g_orig_color_g = *(float*)(ball + BALL_COLOR_G);
                g_orig_color_b = *(float*)(ball + BALL_COLOR_B);
                g_color_saved = 1;
                g_last_ball = ball;
            }

            *(float*)(ball + BALL_COLOR_R) = GLOW_R * g_charge;
            *(float*)(ball + BALL_COLOR_G) = GLOW_G * g_charge;
            *(float*)(ball + BALL_COLOR_B) = GLOW_B * g_charge;
            *(float*)(ball + BALL_ALPHA) = g_charge;
        }
    }

    /* Drain charge — only when board is valid (active gameplay) */
    if (app && !IsBadReadPtr((void*)(app + APP_BOARD), 4)) {
        DWORD board = *(DWORD*)(app + APP_BOARD);
        if (board && !IsBadReadPtr((void*)board, 0x100)) {
            g_charge -= CHARGE_DRAIN_RATE;
            if (g_charge < CHARGE_MIN) g_charge = CHARGE_MIN;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Platform Flicker Update
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_platforms(void) {
    /* Determine desired platform state from charge */
    int want_visible = (g_charge >= FLICKER_OUT_THRESHOLD);

    /* If discharge fired, force all platforms to transition instantly */
    int force_instant = g_discharged;
    int flicker_timer = force_instant ? 1 : TIMER_FULL;

    /* Get board from App */
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x200)) return;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;

    /* Detect board change -> reset charge for new race */
    if (board != g_last_board) {
        /* Restore ball color from the OLD ball before switching */
        restore_ball_color();
        g_charge = CHARGE_MAX;
        g_last_board = board;
        /* Ball will change too — reset color tracking */
        g_color_saved = 0;
        g_last_ball = 0;
        /* Reset discharge state for new race */
        g_discharged = 0;
    }

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

        if (IsBadReadPtr((void*)(obj + AS_STATE), 4)) continue;

        int state = *(int*)(obj + AS_STATE);

        if (want_visible) {
            /* Charge is high enough — want platforms visible */
            switch (state) {
            case STATE_INVISIBLE:
                /* Was invisible — let native 2->3 transition fire so the
                 * object gets ADDED back to the render+collision list.
                 * Set timer=1 for fast transition. Do NOT set state=3
                 * directly — that skips the native ADD. */
                *(int*)(obj + AS_TIMER) = 1;
                break;
            case STATE_FLICKER_DOWN:
                /* Was flickering towards invisible — reverse course.
                 * Let native 1->2->3 path run by accelerating timer.
                 * The 1->2 transition does REMOVE (already done), then
                 * 2->3 transition does ADD. Set timer=1 to speed it up. */
                *(int*)(obj + AS_TIMER) = 1;
                break;
            case STATE_FLICKER_UP:
                /* Already flickering up — don't restart countdown */
                break;
            case STATE_SOLID:
            default:
                /* Already solid — pin timer to stay visible */
                *(int*)(obj + AS_TIMER) = TIMER_FULL;
                break;
            }
        } else {
            /* Charge is low — want platforms invisible.
             * Start flicker-down early so it finishes before charge hits 0. */
            switch (state) {
            case STATE_SOLID:
                /* Currently visible — start flicker to disappear. */
                *(int*)(obj + AS_STATE) = STATE_FLICKER_DOWN;
                *(int*)(obj + AS_TIMER) = flicker_timer;
                break;
            case STATE_FLICKER_DOWN:
                /* Flicker in progress — let native state machine run */
                break;
            case STATE_INVISIBLE:
            default:
                /* Stable invisible — pin timer to prevent transition */
                *(int*)(obj + AS_TIMER) = TIMER_FULL;
                break;
            }
        }
    }

    /* DFLOOR4 is NOT in the board+0x2578 AthenaList — it's stored separately
     * at board+0x438C. Neon_CreateDynamicObjects sets it to state=2 (invisible)
     * at spawn. We must handle it independently. */
    if (!IsBadReadPtr((void*)(board + BOARD_DFOOR4_PTR), 4)) {
        DWORD dfloor4 = *(DWORD*)(board + BOARD_DFOOR4_PTR);
        if (dfloor4 && !IsBadReadPtr((void*)dfloor4, 4) &&
            *(DWORD*)dfloor4 == ARENASTANDS_VTABLE &&
            !IsBadReadPtr((void*)(dfloor4 + AS_STATE), 4)) {

            int state = *(int*)(dfloor4 + AS_STATE);

            if (want_visible) {
                switch (state) {
                case STATE_INVISIBLE:
                    *(int*)(dfloor4 + AS_TIMER) = 1;
                    break;
                case STATE_FLICKER_DOWN:
                    *(int*)(dfloor4 + AS_TIMER) = 1;
                    break;
                case STATE_FLICKER_UP:
                    break;
                case STATE_SOLID:
                default:
                    *(int*)(dfloor4 + AS_TIMER) = TIMER_FULL;
                    break;
                }
            } else {
                switch (state) {
                case STATE_SOLID:
                    *(int*)(dfloor4 + AS_STATE) = STATE_FLICKER_DOWN;
                    *(int*)(dfloor4 + AS_TIMER) = flicker_timer;
                    break;
                case STATE_FLICKER_DOWN:
                    break;
                case STATE_INVISIBLE:
                default:
                    *(int*)(dfloor4 + AS_TIMER) = TIMER_FULL;
                    break;
                }
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Graphics_RenderScene Hook Cave
 * ═══════════════════════════════════════════════════════════════════════════ */

extern void hook_cave_asm(void);

/* At hook entry: ECX = gfx (this pointer for Graphics_RenderScene)
   Original instruction: SUB ESP, 0xC0 (81 EC C0 00 00 00, 6 bytes) */
__asm__(
    ".global _hook_cave_asm\n"
    "_hook_cave_asm:\n"
    "    pushal\n"
    "    pushfl\n"
    "    pushl %ecx\n"           /* gfx pointer */
    "    call _on_render_scene\n"
    "    addl $4, %esp\n"
    "    popfl\n"
    "    popal\n"
    "    subl $0xc0, %esp\n"    /* Original instruction */
    "    jmp 0x00454BC6\n"       /* jump back */
);

void on_render_scene(DWORD gfx) {
    /* Shutdown guard — game frees board/ball before DLL_PROCESS_DETACH.
     * Without this, the hook accesses freed memory on exit -> crash. */
    if (game_is_quitting()) return;

    update_light(gfx);

    /* Clear discharge flag when charge is restored (N:CHARGE collision) */
    if (g_discharged && g_charge >= CHARGE_MAX)
        g_discharged = 0;

    update_platforms();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Collision Dispatch Hook Caves
 *
 * Both call sites are: 52 56 FF 50 74
 *   PUSH EDX (collObj); PUSH ESI (ball); CALL [EAX+0x74]
 * The cave saves all registers, calls C handler, restores, re-executes
 * the original 5 bytes, then jumps back.
 * ═══════════════════════════════════════════════════════════════════════════ */

extern void collision_cave_1(void);
extern void collision_cave_2(void);

/* Call site 1: 0x40728F */
__asm__(
    ".global _collision_cave_1\n"
    "_collision_cave_1:\n"
    "    pushal\n"
    "    pushfl\n"
    "    pushl %edx\n"            /* collObj (2nd param) */
    "    pushl %esi\n"            /* ball (1st param) */
    "    call _on_collision_dispatch\n"
    "    addl $8, %esp\n"        /* clean up 2 params */
    "    popfl\n"
    "    popal\n"
    "    pushl %edx\n"            /* original: PUSH EDX (52) */
    "    pushl %esi\n"            /* original: PUSH ESI (56) */
    "    call *0x74(%eax)\n"      /* original: CALL [EAX+0x74] (FF 50 74) */
    "    jmp 0x00407294\n"        /* return to original+5 */
);

/* Call site 2: 0x408B85 */
__asm__(
    ".global _collision_cave_2\n"
    "_collision_cave_2:\n"
    "    pushal\n"
    "    pushfl\n"
    "    pushl %edx\n"            /* collObj (2nd param) */
    "    pushl %esi\n"            /* ball (1st param) */
    "    call _on_collision_dispatch\n"
    "    addl $8, %esp\n"        /* clean up 2 params */
    "    popfl\n"
    "    popal\n"
    "    pushl %edx\n"            /* original: PUSH EDX (52) */
    "    pushl %esi\n"            /* original: PUSH ESI (56) */
    "    call *0x74(%eax)\n"      /* original: CALL [EAX+0x74] (FF 50 74) */
    "    jmp 0x00408B8A\n"        /* return to original+5 */
);

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_hooks(void) {
    /* RenderScene hook (6-byte instruction -> JMP + NOP) */
    memcpy(g_orig_bytes, (void*)RENDER_SCENE_HOOK, RENDER_SCENE_ORIG_BYTES);
    install_jmp_hook_nop(RENDER_SCENE_HOOK, (DWORD)hook_cave_asm, RENDER_SCENE_ORIG_BYTES);

    /* Collision dispatch call site 1 (5 bytes -> JMP, exact fit) */
    install_jmp_hook(COLLISION_HOOK_1, (DWORD)collision_cave_1);

    /* Collision dispatch call site 2 (5 bytes -> JMP, exact fit) */
    install_jmp_hook(COLLISION_HOOK_2, (DWORD)collision_cave_2);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        install_hooks();
    } else if (reason == DLL_PROCESS_DETACH) {
        /* Restore ball color before the DLL is unloaded.
         * Without this, the ball keeps mod colors (possibly alpha=0
         * = invisible) after the mod is removed. */
        restore_ball_color();
    }
    return TRUE;
}

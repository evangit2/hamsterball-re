/*
 * electric_lights.c — Electric Lights + Light Platforms merged mod
 * v3: Fixed SEH trampoline crash, shutdown guard, color restoration, charge gating
 * v4: Fixed ArenaStands render-list re-add bug (state 2→3 skip), added N:DISCHARGE
 *
 * Phase 1+2: Charge system + ball glow + D3D light + platform flicker
 *
 * CHARGE SYSTEM:
 *   Ball starts with full charge (1.0). Charge drains over time.
 *   Ball proximity to N:CHARGE meshes restores charge to full.
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
 * Hook: Graphics_RenderScene entry (0x454BC0)
 *   ECX = gfx struct pointer at entry
 *   gfx+0x154 = IDirect3DDevice8*
 *   gfx+0x854/858/85C = ball position (set by Scene_Render)
 *
 * v3 CHANGES:
 *   - Replaced DispatchCollisionEvents trampoline hook (SEH crash) with
 *     proximity-based N:CHARGE mesh scan from RenderScene hook
 *   - Added shutdown guard (App+0x159 quit flag) to prevent use-after-free
 *   - Ball color now properly tracked: resets when ball pointer changes
 *   - Charge only drains during active gameplay (board valid + not quitting)
 *   - Charge resets to full on new race (board pointer change detection)
 *   - Removed dead g_flicker_down_triggered code
 *
 * v4 CHANGES:
 *   - Fixed ArenaStands render-list re-add: setting state=3 directly from
 *     state=2 skipped the native 2→3 transition that ADDS the object back
 *     to the render+collision list. Now sets timer=1 in state 2 so the
 *     native state machine does the ADD itself.
 *   - Added N:DISCHARGE proximity detection (ported from light_platforms.c):
 *     when ball touches N:DISCHARGE mesh and charge > 0, zeros charge and
 *     forces instant platform transitions (no flicker).
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
#define BOARD_LEVEL_PTR      0x8AC
#define ATHENALIST_COUNT      0x04
#define ATHENALIST_ITEMS      0x40C

/* Level/MeshWorld offsets */
#define LEVEL_MESHWORLD_PTR  0x08
#define MW_MESHBUFFER_LIST   0x2C

/* MeshBuffer offsets */
#define MESHBUFFER_NAME      0x864
#define MESHBUFFER_POS_X     0x868
#define MESHBUFFER_POS_Y     0x86C
#define MESHBUFFER_POS_Z     0x870

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

/* Charge pad proximity threshold (ball radius 26 + tolerance) */
#define CHARGE_PAD_RADIUS    50.0f
#define CHARGE_PAD_RADIUS_SQ (CHARGE_PAD_RADIUS * CHARGE_PAD_RADIUS)

/* N:DISCHARGE proximity threshold (ball radius 26, 80 = generous touch) */
#define DISCHARGE_RADIUS     80.0f
#define DISCHARGE_RADIUS_SQ  (DISCHARGE_RADIUS * DISCHARGE_RADIUS)
#define DISCHARGE_COOLDOWN   300   /* frames (5s at 60fps) */

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

/* N:DISCHARGE state */
static int      g_discharged = 0;          /* 1 = discharge has fired */
static int      g_discharge_cooldown = 0;   /* frames until ball leaves mesh */

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
 * Charge Pad Proximity Scan (replaces DCE trampoline hook)
 *
 * DispatchCollisionEvents (0x40C5D0) uses SEH in its prologue. Building a
 * manual trampoline on it corrupts the exception chain and crashes. Instead,
 * we scan the MeshWorld for N:CHARGE meshes and check ball proximity from
 * the RenderScene hook. This is safe — no SEH, no trampoline.
 *
 * Chain: App -> board+0x178 -> level+0x8AC -> meshworld+0x08 -> MW+0x2C
 * AthenaList: count at +0x04, items at +0x40C
 * MeshBuffer: name at +0x864, position at +0x868/86C/870
 * ═══════════════════════════════════════════════════════════════════════════ */

static void scan_charge_pads(float bx, float by, float bz) {
    if (bx == 0.0f && by == 0.0f && bz == 0.0f) return;

    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x200)) return;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board || IsBadReadPtr((void*)board, 0x8B0)) return;

    DWORD level = *(DWORD*)(board + BOARD_LEVEL_PTR);
    if (!level || IsBadReadPtr((void*)level, 0x10)) return;
    DWORD meshworld = *(DWORD*)(level + LEVEL_MESHWORLD_PTR);
    if (!meshworld || IsBadReadPtr((void*)meshworld, 0x420)) return;

    /* AthenaList embedded at MeshWorld+0x2C */
    DWORD listBase = meshworld + MW_MESHBUFFER_LIST;
    int count = *(int*)(listBase + ATHENALIST_COUNT);
    if (count < 1 || count > 5000) return;
    DWORD items = *(DWORD*)(listBase + ATHENALIST_ITEMS);
    if (!items || IsBadReadPtr((void*)items, count * 4)) return;

    for (int i = 0; i < count; i++) {
        DWORD mb = *(DWORD*)(items + i * 4);
        if (!mb || IsBadReadPtr((void*)mb, 0x874)) continue;

        DWORD namePtr = *(DWORD*)(mb + MESHBUFFER_NAME);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 9)) continue;

        const char *name = (const char*)namePtr;
        if (!name[0]) continue;

        if (_strnicmp(name, "N:CHARGE", 8) != 0) continue;

        /* Found a charge pad — check ball proximity */
        float mx = *(float*)(mb + MESHBUFFER_POS_X);
        float my = *(float*)(mb + MESHBUFFER_POS_Y);
        float mz = *(float*)(mb + MESHBUFFER_POS_Z);

        float dx = bx - mx;
        float dy = by - my;
        float dz = bz - mz;
        float dist_sq = dx*dx + dy*dy + dz*dz;

        if (dist_sq < CHARGE_PAD_RADIUS_SQ) {
            g_charge = CHARGE_MAX;
            return; /* Recharged — no need to check more pads */
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

    /* Scan for N:CHARGE proximity pads (replaces DCE hook) */
    scan_charge_pads(bx, by, bz);

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
 * N:DISCHARGE — Energy Gate
 *
 * When the ball touches an N:DISCHARGE mesh AND charge > 0:
 *   1. Zero the charge (turns off light + platforms)
 *   2. Set g_discharged flag (forces instant platform transitions, no flicker)
 *   3. Cooldown prevents repeat triggers while ball stays on mesh
 *
 * Uses same MeshBuffer name scanning as charge pads.
 * ═══════════════════════════════════════════════════════════════════════════ */

static int check_discharge_collision(float bx, float by, float bz) {
    if (bx == 0.0f && by == 0.0f && bz == 0.0f) return 0;

    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x200)) return 0;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board || IsBadReadPtr((void*)board, 0x8B0)) return 0;

    DWORD level = *(DWORD*)(board + BOARD_LEVEL_PTR);
    if (!level || IsBadReadPtr((void*)level, 0x10)) return 0;
    DWORD meshworld = *(DWORD*)(level + LEVEL_MESHWORLD_PTR);
    if (!meshworld || IsBadReadPtr((void*)meshworld, 0x420)) return 0;

    DWORD listBase = meshworld + MW_MESHBUFFER_LIST;
    int count = *(int*)(listBase + ATHENALIST_COUNT);
    if (count < 1 || count > 5000) return 0;
    DWORD items = *(DWORD*)(listBase + ATHENALIST_ITEMS);
    if (!items || IsBadReadPtr((void*)items, count * 4)) return 0;

    for (int i = 0; i < count; i++) {
        DWORD mb = *(DWORD*)(items + i * 4);
        if (!mb || IsBadReadPtr((void*)mb, 0x874)) continue;

        DWORD namePtr = *(DWORD*)(mb + MESHBUFFER_NAME);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 13)) continue;

        const char *name = (const char*)namePtr;
        if (!name[0]) continue;

        if (_strnicmp(name, "N:DISCHARGE", 12) != 0) continue;

        float mx = *(float*)(mb + MESHBUFFER_POS_X);
        float my = *(float*)(mb + MESHBUFFER_POS_Y);
        float mz = *(float*)(mb + MESHBUFFER_POS_Z);

        float dx = bx - mx;
        float dy = by - my;
        float dz = bz - mz;
        float dist_sq = dx*dx + dy*dy + dz*dz;

        if (dist_sq < DISCHARGE_RADIUS_SQ)
            return 1;
    }

    return 0;
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
        g_charge = CHARGE_MAX;
        g_last_board = board;
        /* Ball will change too — reset color tracking */
        g_color_saved = 0;
        g_last_ball = 0;
        /* Reset discharge state for new race */
        g_discharged = 0;
        g_discharge_cooldown = 0;
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
                /* Was invisible — let native 2→3 transition fire so the
                 * object gets ADDED back to the render+collision list.
                 * Set timer=1 for fast transition. Do NOT set state=3
                 * directly — that skips the native ADD. */
                *(int*)(obj + AS_TIMER) = 1;
                break;
            case STATE_FLICKER_DOWN:
                /* Was flickering towards invisible — reverse course.
                 * Let native 1→2→3 path run by accelerating timer.
                 * The 1→2 transition does REMOVE (already done), then
                 * 2→3 transition does ADD. Set timer=1 to speed it up. */
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

    /* Check N:DISCHARGE collision using ball position from gfx */
    float bx = *(float*)(gfx + GFX_BALL_X);
    float by = *(float*)(gfx + GFX_BALL_Y);
    float bz = *(float*)(gfx + GFX_BALL_Z);

    int touching = check_discharge_collision(bx, by, bz);

    if (touching && g_discharge_cooldown == 0 && g_charge > 0.0f) {
        /* Discharge! Zero the charge and set flag for instant transitions */
        g_charge = 0.0f;
        g_discharged = 1;
        g_discharge_cooldown = DISCHARGE_COOLDOWN;
    }

    if (g_discharge_cooldown > 0)
        g_discharge_cooldown--;

    /* Clear discharge flag when charge is restored (touching N:CHARGE pad) */
    if (g_discharged && g_charge >= CHARGE_MAX)
        g_discharged = 0;

    update_platforms();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_hooks(void) {
    memcpy(g_orig_bytes, (void*)RENDER_SCENE_HOOK, RENDER_SCENE_ORIG_BYTES);
    install_jmp_hook_nop(RENDER_SCENE_HOOK, (DWORD)hook_cave_asm, RENDER_SCENE_ORIG_BYTES);
    /* DCE trampoline hook REMOVED — was crashing due to SEH prologue.
     * N:CHARGE detection now uses proximity scan in update_light. */
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

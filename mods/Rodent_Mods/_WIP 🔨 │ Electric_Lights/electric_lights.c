/*
 * electric_lights.c — Electric Lights + Light Platforms merged mod
 *
 * Phase 1+2: Charge system + ball glow + D3D light + platform flicker
 *
 * CHARGE SYSTEM:
 *   Ball starts with full charge (1.0). Charge drains over time.
 *   Touching N:CHARGE collision planes restores charge to full.
 *   D3D point light follows the ball, range scales with charge.
 *   Ball color multiplier scales with charge for visual glow.
 *
 * PLATFORM FLICKER:
 *   ArenaStands (DFLOOR) platforms flicker based on charge level:
 *   - Charge drops below FLICKER_OUT_THRESHOLD → start flicker DOWN
 *     (flicker for 75 frames, then platform becomes invisible)
 *     Threshold is set so flicker completes before charge hits 0.
 *   - Ball touches N:CHARGE (charge restored) → start flicker UP
 *     (flicker for 75 frames, then platform becomes solid)
 *   - During flicker: native ToggleTimer controls visual on/off.
 *     Object stays in render list → collision active during flicker.
 *
 * ARENASTANDS STATE MACHINE:
 *   State 0: solid visible (stable)
 *   State 1: flicker before disappearing (→ 2 after 75 frames)
 *   State 2: invisible (stable, removed from render list)
 *   State 3: flicker after reappearing (→ 0 after 75 frames)
 *
 * Hook: Graphics_RenderScene entry (0x454BC0)
 *   ECX = gfx struct pointer at entry
 *   gfx+0x154 = IDirect3DDevice8*
 *   gfx+0x854/858/85C = ball position (set by Scene_Render)
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

/* Board offsets */
#define BOARD_DYNOBJ_LIST    0x2578
#define ATHENALIST_COUNT      0x04
#define ATHENALIST_ITEMS      0x40C

/* ArenaStands struct offsets */
#define ARENASTANDS_VTABLE   0x004D5A70
#define AS_STATE             0x10DC
#define AS_TIMER             0x10E0
#define AS_NEEDS_READD       0x1100

/* Collision dispatch hook */
#define DISPATCH_COLLISION_EVENTS 0x0040C5D0
#define MESHBUFFER_NAME      0x864

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

/* Track whether we've triggered the flicker-down so we don't retrigger */
static int      g_flicker_down_triggered = 0;

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
 * D3D Light Update
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_light(DWORD gfx) {
    if (!gfx || IsBadReadPtr((void*)gfx, 0x860)) return;

    DWORD device = *(DWORD*)(gfx + GFX_D3D_DEVICE);
    if (!device || IsBadReadPtr((void*)device, 4)) return;
    DWORD vtable = *(DWORD*)device;
    if (!vtable || IsBadReadPtr((void*)vtable, 0x100)) return;

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

    /* Read ball position from gfx+0x854/858/85C */
    float bx = *(float*)(gfx + GFX_BALL_X);
    float by = *(float*)(gfx + GFX_BALL_Y);
    float bz = *(float*)(gfx + GFX_BALL_Z);

    if (bx == 0.0f && by == 0.0f && bz == 0.0f) return;

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
            if (!g_color_saved) {
                g_orig_color_r = *(float*)(ball + BALL_COLOR_R);
                g_orig_color_g = *(float*)(ball + BALL_COLOR_G);
                g_orig_color_b = *(float*)(ball + BALL_COLOR_B);
                g_color_saved = 1;
            }
            *(float*)(ball + BALL_COLOR_R) = GLOW_R * g_charge;
            *(float*)(ball + BALL_COLOR_G) = GLOW_G * g_charge;
            *(float*)(ball + BALL_COLOR_B) = GLOW_B * g_charge;
            *(float*)(ball + BALL_ALPHA) = g_charge;
        }
    }

    /* Drain charge */
    g_charge -= CHARGE_DRAIN_RATE;
    if (g_charge < CHARGE_MIN) g_charge = CHARGE_MIN;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Platform Flicker Update
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_platforms(void) {
    /* Determine desired platform state from charge */
    /* "want_visible" = charge is high enough to keep platforms solid */
    int want_visible = (g_charge >= FLICKER_OUT_THRESHOLD);

    /* Reset flicker-down trigger when charge is restored */
    if (g_charge > FLICKER_OUT_THRESHOLD) {
        g_flicker_down_triggered = 0;
    }

    /* Get board from App */
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x200)) return;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;

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
                /* Currently invisible — start flicker to reappear.
                 * This fires when N:CHARGE restores charge. */
                *(int*)(obj + AS_STATE) = STATE_FLICKER_UP;
                *(int*)(obj + AS_TIMER) = TIMER_FULL;
                break;
            case STATE_FLICKER_UP:
                /* Flicker in progress — let native state machine run */
                break;
            case STATE_SOLID:
            default:
                /* Stable visible — pin timer to prevent transition */
                *(int*)(obj + AS_TIMER) = TIMER_FULL;
                break;
            }
        } else {
            /* Charge is low — want platforms invisible.
             * Start flicker-down early so it finishes before charge hits 0. */
            switch (state) {
            case STATE_SOLID:
                /* Currently visible — start flicker to disappear.
                 * Only trigger once per charge depletion cycle. */
                *(int*)(obj + AS_STATE) = STATE_FLICKER_DOWN;
                *(int*)(obj + AS_TIMER) = TIMER_FULL;
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
    update_light(gfx);
    update_platforms();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Collision Event Hook — N:CHARGE recharge pads
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE g_dce_original[8];
static void *g_dce_trampoline = NULL;
static BYTE *g_dce_stub = NULL;

static void build_dce_trampoline(void) {
    BYTE *code = (BYTE*)alloc_executable(16);
    memcpy(code, g_dce_original, 8);
    code[8] = 0xE9;
    *(DWORD*)(code + 9) = (DWORD)(DISPATCH_COLLISION_EVENTS + 8) - (DWORD)(code + 13);
    g_dce_trampoline = code;
}

void __cdecl dce_handler(DWORD board, DWORD ball, DWORD collEntry) {
    if (!collEntry) return;

    DWORD *pair = (DWORD*)collEntry;
    if (IsBadReadPtr(pair, 8)) return;
    if (!pair[1]) return;
    if (IsBadReadPtr((void*)pair[1], 0x868)) return;

    DWORD namePtr = *(DWORD*)((BYTE*)pair[1] + MESHBUFFER_NAME);
    if (!namePtr || IsBadReadPtr((void*)namePtr, 8)) return;
    const char *eventName = (const char*)namePtr;
    if (!eventName[0]) return;

    /* Check for N:CHARGE */
    if (_strnicmp(eventName, "N:CHARGE", 8) != 0) return;

    /* Reset charge to max — this triggers platform flicker-up */
    g_charge = CHARGE_MAX;
    g_flicker_down_triggered = 0;
}

static void build_dce_stub(void) {
    BYTE *code = (BYTE*)alloc_executable(64);
    int i = 0;
    code[i++] = 0x60;  /* pushad */
    code[i++] = 0x9C;  /* pushfd */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C; /* push [esp+0x2C] (collEntry) */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C; /* push [esp+0x2C] (ball) */
    code[i++] = 0x51;  /* push ecx (board) */
    code[i++] = 0xE8;  /* call dce_handler */
    *(DWORD*)(code + i) = (DWORD)&dce_handler - (DWORD)(code + i + 4);
    i += 4;
    code[i++] = 0x83; code[i++] = 0xC4; code[i++] = 0x0C; /* add esp, 12 */
    code[i++] = 0x9D;  /* popfd */
    code[i++] = 0x61;  /* popad */
    code[i++] = 0xB8;  /* mov eax, addr */
    *(DWORD*)(code + i) = (DWORD)g_dce_trampoline;
    i += 4;
    code[i++] = 0xFF; code[i++] = 0xE0; /* jmp eax */

    g_dce_stub = code;
}

static void install_dce_hook(void) {
    memcpy(g_dce_original, (void*)DISPATCH_COLLISION_EVENTS, 8);
    build_dce_trampoline();
    build_dce_stub();

    BYTE patch[8];
    patch[0] = 0xE9;
    *(DWORD*)(patch + 1) = (DWORD)g_dce_stub - DISPATCH_COLLISION_EVENTS - 5;
    patch[5] = 0x90;
    patch[6] = 0x90;
    patch[7] = 0x90;
    patch_bytes(DISPATCH_COLLISION_EVENTS, patch, 8);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_hooks(void) {
    memcpy(g_orig_bytes, (void*)RENDER_SCENE_HOOK, RENDER_SCENE_ORIG_BYTES);
    install_jmp_hook_nop(RENDER_SCENE_HOOK, (DWORD)hook_cave_asm, RENDER_SCENE_ORIG_BYTES);
    install_dce_hook();
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

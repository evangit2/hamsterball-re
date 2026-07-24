/*
 * electric_lights.c — Electric Lights mod for Hamsterball Physicus
 *
 * Phase 1: Charge system + ball glow
 *
 * The ball is a D3D point light source that drains over time.
 * Uses the native Neon Race SceneObject light system:
 *   - Creates a SceneObject (0xD4 bytes) at level start
 *   - Registers it via Scene_RegisterObject to get a D3D light slot
 *   - Updates its position to follow the ball every frame
 *   - Scales light range (obj+0xCC) based on charge level
 *   - Scales ball color multiplier for visual glow feedback
 *
 * Hook: Ball_Update at 0x405E22 (same as all RodentRacer mods)
 *   ESI = ball pointer at hook point
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll electric_lights.c \
 *     -I"../_WIP 🔨 │ shared" -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *     -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include "bass_proxy.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Addresses
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BALL_UPDATE_HOOK     0x00405E22
#define BALL_UPDATE_ORIG     0x00405E28    /* jump back target */
#define BALL_UPDATE_ORIG_BYTES 6

#define ADDR_operator_new    0x004BA57B
#define ADDR_SceneObject_ctor 0x0046B4F0
#define ADDR_Scene_RegisterObject 0x00453BD0

/* SceneObject vtable entries */
#define SCENEOBJ_VTABLE      0x004D934C
#define VTABLE_SETPosition   0x04    /* vtable[1] */
#define VTABLE_REFRESH       0x0C    /* vtable[3] */

/* SceneObject field offsets */
#define SO_PARENT            0x04    /* gfx ptr */
#define SO_POS_X             0x08    /* light position (set by SetPosition) */
#define SO_POS_Y             0x0C
#define SO_POS_Z             0x10
#define SO_LIGHT_TYPE        0x34    /* 1 = point light */
#define SO_VISIBLE           0x88    /* visible flag */
#define SO_LIGHT_INDEX      0x8C    /* D3D light index (set by RegisterObject) */
#define SO_RANGE             0xCC    /* light range (float, 400.0 default) */
#define SO_D3DTYPE           0xD0    /* D3DLIGHT8.Type (3 = POINT) */

/* Ball field offsets */
#define BALL_APP             0x10    /* App pointer (NOT board) */
#define BALL_BOARD           0x14    /* board pointer */
#define BALL_POS_X           0x164
#define BALL_POS_Y           0x168
#define BALL_POS_Z           0x16C
#define BALL_COLOR_R         0x2AC   /* runtime color multiplier */
#define BALL_COLOR_G         0x2B0
#define BALL_COLOR_B         0x2B4

/* App field offsets */
#define APP_GFX              0x174   /* gfx/scene pointer */

/* Board field offsets */
#define BOARD_APP            0x878   /* App pointer */

/* gfx field offsets */
#define GFX_D3D_DEVICE       0x154
#define GFX_LIGHT_SLOTS      0x710   /* SceneObject* slots[8] */
#define GFX_AMBIENT          0x730   /* ambient color (DWORD) */

/* ═══════════════════════════════════════════════════════════════════════════
 * Configuration
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CHARGE_DRAIN_RATE    0.0008f   /* charge lost per frame (~0.05/sec at 60fps) */
#define CHARGE_MAX           1.0f
#define CHARGE_MIN           0.0f

#define LIGHT_FULL_RANGE     400.0f    /* max light range (same as Neon Race) */
#define LIGHT_Y_OFFSET       30.0f     /* light hovers above ball */

/* Glow color when fully charged (white-cyan, like Neon Race) */
#define GLOW_R               0.8f
#define GLOW_G               0.95f
#define GLOW_B               1.0f

/* D3D light slot to use (0-7; Neon Race uses 0 for P1, 1 for P2) */
#define LIGHT_SLOT           2   /* use slot 2 to avoid conflicts with Neon Race */

/* ═══════════════════════════════════════════════════════════════════════════
 * Mod State
 * ═══════════════════════════════════════════════════════════════════════════ */

static float  g_charge = CHARGE_MAX;
static DWORD  g_light_obj = 0;       /* SceneObject pointer */
static int    g_light_created = 0;   /* one-shot flag */
static BYTE   g_orig_bytes[6];      /* original bytes at hook point */

/* Saved original ball color for restoration */
static float  g_orig_color_r = 1.0f;
static float  g_orig_color_g = 1.0f;
static float  g_orig_color_b = 1.0f;
static int    g_color_saved = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * __thiscall wrappers (MinGW doesn't support __thiscall keyword)
 * Use inline asm for functions with >1 stack param
 * ═══════════════════════════════════════════════════════════════════════════ */

/* SceneObject_ctor: __thiscall(this, gfx_ptr) — 1 stack param, RET 4 */
static DWORD call_sceneobject_ctor(DWORD this_ptr, DWORD gfx_ptr) {
    /* __fastcall with dummy EDX works for 1-param __thiscall */
    typedef DWORD (__fastcall *ctor_t)(DWORD, DWORD, DWORD);
    ctor_t fn = (ctor_t)ADDR_SceneObject_ctor;
    return fn(this_ptr, 0, gfx_ptr);
}

/* Scene_RegisterObject: __thiscall(scene, slot, obj) — 2 stack params, RET 8 */
static void call_register_object(DWORD scene, int slot, DWORD obj) {
    /* Need inline asm for 2-param __thiscall */
    DWORD fn = ADDR_Scene_RegisterObject;
    __asm__ __volatile__(
        "push %2\n"
        "push %1\n"
        "movl %0, %%ecx\n"
        "call *%3\n"
        : 
        : "r"(scene), "r"(slot), "r"(obj), "r"(fn)
        : "eax", "edx", "ecx", "memory"
    );
}

/* SceneObject_SetPosition: __thiscall(this, x, y, z) — 3 stack params, RET 0xC */
static void call_set_position(DWORD obj, float x, float y, float z) {
    /* Read vtable[1] and call it */
    DWORD vtable = *(DWORD*)obj;
    DWORD setpos_fn = *(DWORD*)(vtable + VTABLE_SETPosition);
    
    __asm__ __volatile__(
        "push %3\n"
        "push %2\n"
        "push %1\n"
        "movl %0, %%ecx\n"
        "call *%4\n"
        :
        : "r"(obj), "m"(x), "m"(y), "m"(z), "r"(setpos_fn)
        : "eax", "edx", "ecx", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Light Creation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void create_light(DWORD ball) {
    if (g_light_created) return;
    
    /* Get gfx pointer: ball+0x14 → board, board+0x878 → App, App+0x174 → gfx */
    if (IsBadReadPtr((void*)(ball + BALL_BOARD), 4)) return;
    DWORD board = *(DWORD*)(ball + BALL_BOARD);
    if (!board || IsBadReadPtr((void*)(board + BOARD_APP), 4)) return;
    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)(app + APP_GFX), 4)) return;
    DWORD gfx = *(DWORD*)(app + APP_GFX);
    if (!gfx || IsBadReadPtr((void*)gfx, 4)) return;
    
    /* Allocate SceneObject (0xD4 bytes) */
    operator_new_t op_new = (operator_new_t)ADDR_operator_new;
    DWORD obj = (DWORD)op_new(0xD4);
    if (!obj) return;
    
    /* SceneObject_ctor(obj, gfx) */
    call_sceneobject_ctor(obj, gfx);
    
    /* Set light type = 1 (point light) */
    *(DWORD*)(obj + SO_LIGHT_TYPE) = 1;
    
    /* Set D3DLIGHT8.Type = 3 (D3DLIGHT_POINT) */
    *(DWORD*)(obj + SO_D3DTYPE) = 3;
    
    /* Set range = 400.0 */
    *(float*)(obj + SO_RANGE) = LIGHT_FULL_RANGE;
    
    /* Set visible = 1 */
    *(BYTE*)(obj + SO_VISIBLE) = 1;
    
    /* Register with scene to get a D3D light slot */
    call_register_object(gfx, LIGHT_SLOT, obj);
    
    g_light_obj = obj;
    g_light_created = 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-Frame Update
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_light(DWORD ball) {
    if (!g_light_obj) return;
    if (IsBadReadPtr((void*)g_light_obj, 0xD4)) return;
    
    /* Read ball position */
    if (IsBadReadPtr((void*)(ball + BALL_POS_X), 12)) return;
    float bx = *(float*)(ball + BALL_POS_X);
    float by = *(float*)(ball + BALL_POS_Y);
    float bz = *(float*)(ball + BALL_POS_Z);
    
    /* Update light position to follow ball */
    call_set_position(g_light_obj, bx, by + LIGHT_Y_OFFSET, bz);
    
    /* Scale light range based on charge */
    *(float*)(g_light_obj + SO_RANGE) = g_charge * LIGHT_FULL_RANGE;
    
    /* Scale ball color multiplier for glow effect */
    if (!g_color_saved) {
        g_orig_color_r = *(float*)(ball + BALL_COLOR_R);
        g_orig_color_g = *(float*)(ball + BALL_COLOR_G);
        g_orig_color_b = *(float*)(ball + BALL_COLOR_B);
        g_color_saved = 1;
    }
    
    /* Lerp from glow color to dark based on charge */
    float glow_r = GLOW_R * g_charge;
    float glow_g = GLOW_G * g_charge;
    float glow_b = GLOW_B * g_charge;
    
    *(float*)(ball + BALL_COLOR_R) = glow_r;
    *(float*)(ball + BALL_COLOR_G) = glow_g;
    *(float*)(ball + BALL_COLOR_B) = glow_b;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Ball_Update Hook Cave
 * ═══════════════════════════════════════════════════════════════════════════ */

static void __attribute__((used)) hook_cave(void) {
    /* This function is never called directly — its address is used for the
       JMP hook. The actual code is in the assembly below. */
}

/* The hook cave: called every frame from Ball_Update
   At entry: ESI = ball pointer (preserved by PUSHAD/POPAD) */

extern void hook_cave_asm(void);

__asm__(
    ".global _hook_cave_asm\n"
    "_hook_cave_asm:\n"
    "    pushal\n"
    "    movl %esi, %eax\n"        /* ball pointer */
    "    pushl %eax\n"
    "    call _on_ball_update\n"
    "    addl $4, %esp\n"
    "    popal\n"
    "    movl 0xc5c(%esi), %eax\n" /* Original instruction */
    "    jmp 0x00405E28\n"          /* jump back */
);

/* C function called from the asm cave */
void on_ball_update(DWORD ball) {
    /* Create light on first frame */
    if (!g_light_created) {
        create_light(ball);
    }
    
    /* Drain charge */
    g_charge -= CHARGE_DRAIN_RATE;
    if (g_charge < CHARGE_MIN) g_charge = CHARGE_MIN;
    
    /* Update light position and intensity */
    update_light(ball);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_hooks(void) {
    /* Save original bytes */
    memcpy(g_orig_bytes, (void*)BALL_UPDATE_HOOK, BALL_UPDATE_ORIG_BYTES);
    
    /* Install JMP to our asm cave */
    install_jmp_hook_nop(BALL_UPDATE_HOOK, (DWORD)hook_cave_asm, BALL_UPDATE_ORIG_BYTES);
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

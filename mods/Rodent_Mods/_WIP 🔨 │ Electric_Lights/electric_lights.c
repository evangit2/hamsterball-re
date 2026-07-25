/*
 * electric_lights.c — Electric Lights mod for Hamsterball Physicus
 *
 * Phase 1: Charge system + ball glow
 *
 * Hijacks the native Neon Race D3D light system. Instead of creating a
 * SceneObject, we write directly to a D3D light slot every frame via
 * SetLight + LightEnable — same technique as XRow's GlobalNeon CEA.
 *
 * Light range scales with charge level. Ball color multiplier scales
 * with charge for visual glow feedback.
 *
 * Hook: Graphics_RenderScene entry (0x454BC0) — same hook point as GlobalNeon.
 *   ECX = gfx struct pointer at entry
 *   gfx+0x154 = IDirect3DDevice8*
 *   gfx+0x854/858/85C = ball position (set by Scene_Render)
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

#define RENDER_SCENE_HOOK    0x00454BC0
#define RENDER_SCENE_ORIG    0x00454BC6    /* jump back after 6-byte SUB ESP,0xC0 */
#define RENDER_SCENE_ORIG_BYTES 6

/* gfx struct offsets */
#define GFX_D3D_DEVICE       0x154
#define GFX_AMBIENT          0x730   /* ambient color (DWORD) */
#define GFX_BALL_X           0x854   /* ball position (set by Scene_Render) */
#define GFX_BALL_Y           0x858
#define GFX_BALL_Z           0x85C

/* D3D8 device vtable offsets */
#define D3D_SetLight         0xB0    /* vtable[44] */
#define D3D_LightEnable      0xB8    /* vtable[46] */
#define D3D_SetRenderState   0xC8    /* vtable[50] */

/* D3D render state IDs */
#define D3DRS_LIGHTING       0x89
#define D3DRS_AMBIENT        0x8B

/* Ball field offsets (for color multiplier + alpha) */
#define BALL_COLOR_R         0x2AC
#define BALL_COLOR_G         0x2B0
#define BALL_COLOR_B         0x2B4
#define BALL_ALPHA           0x2FC   /* float: 0.0=invisible, 1.0=opaque */

/* App pointer for getting ball */
#define GLOBAL_APP_PTR       0x005341E0
#define APP_BALL_P1          0x5DC   /* player 1 ball pointer */

/* Collision dispatch hook */
#define DISPATCH_COLLISION_EVENTS 0x0040C5D0
#define MESHBUFFER_NAME     0x864   /* char* pointer to event name */

/* ═══════════════════════════════════════════════════════════════════════════
 * Configuration
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CHARGE_DRAIN_RATE    0.001667f  /* charge lost per frame (~10sec to deplete at 60fps) */
#define CHARGE_MAX           1.0f
#define CHARGE_MIN           0.0f

#define LIGHT_FULL_RANGE     400.0f    /* max light range (same as Neon Race) */
#define LIGHT_Y_OFFSET       30.0f     /* light hovers above ball */

/* Electric blue (#03fff2) — same color as the warp mod */
#define GLOW_R               (3.0f / 255.0f)    /* 0.0118 */
#define GLOW_G               (255.0f / 255.0f)   /* 1.0 */
#define GLOW_B               (242.0f / 255.0f)   /* 0.9490 */

/* D3D light slot to use (0-7; Neon Race uses 0 for P1, 1 for P2) */
#define LIGHT_SLOT           2

/* ═══════════════════════════════════════════════════════════════════════════
 * D3DLIGHT8 Structure (104 bytes)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    DWORD  Type;           /* 0: D3DLIGHT_POINT = 1 */
    float  DiffuseR;        /* 4 */
    float  DiffuseG;        /* 8 */
    float  DiffuseB;        /* 12 */
    float  DiffuseA;        /* 16 */
    float  SpecularR;       /* 20 */
    float  SpecularG;       /* 24 */
    float  SpecularB;       /* 28 */
    float  SpecularA;       /* 32 */
    float  AmbientR;        /* 36 */
    float  AmbientG;        /* 40 */
    float  AmbientB;        /* 44 */
    float  AmbientA;        /* 48 */
    float  PositionX;       /* 52 */
    float  PositionY;       /* 56 */
    float  PositionZ;       /* 60 */
    float  DirectionX;      /* 64 */
    float  DirectionY;      /* 68 */
    float  DirectionZ;      /* 72 */
    float  Range;           /* 76 */
    float  Falloff;         /* 80 */
    float  Attenuation0;    /* 84 */
    float  Attenuation1;    /* 88 */
    float  Attenuation2;    /* 92 */
    float  Theta;           /* 96 */
    float  Phi;             /* 100 */
} D3DLIGHT8;                /* total: 104 bytes */

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

/* D3D light struct (pre-filled, position+range updated per frame) */
/* Diffuse color = electric blue #03fff2 */
static D3DLIGHT8 g_light = {
    1,                          /* Type = D3DLIGHT_POINT */
    3.0f / 255.0f,             /* DiffuseR = 0.0118 */
    255.0f / 255.0f,           /* DiffuseG = 1.0 */
    242.0f / 255.0f,           /* DiffuseB = 0.9490 */
    0.0f,                       /* DiffuseA */
    3.0f / 255.0f,             /* SpecularR */
    255.0f / 255.0f,           /* SpecularG */
    242.0f / 255.0f,           /* SpecularB */
    0.0f,                       /* SpecularA */
    0.0f, 0.0f, 0.0f, 0.0f,    /* Ambient (none) */
    0.0f, 0.0f, 0.0f,          /* Position (updated per frame) */
    0.0f, 0.0f, 0.0f,          /* Direction (unused for point light) */
    400.0f,                     /* Range */
    0.0f,                       /* Falloff */
    0.0f,                       /* Attenuation0 (no constant) */
    0.04f,                      /* Attenuation1 (linear falloff, same as Neon Race) */
    0.0f,                       /* Attenuation2 */
    0.0f,                       /* Theta (spot only) */
    0.0f                        /* Phi (spot only) */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-Frame Light Update
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_light(DWORD gfx) {
    if (!gfx || IsBadReadPtr((void*)gfx, 0x860)) return;
    
    /* Get D3D device */
    DWORD device = *(DWORD*)(gfx + GFX_D3D_DEVICE);
    if (!device || IsBadReadPtr((void*)device, 4)) return;
    DWORD vtable = *(DWORD*)device;
    if (!vtable || IsBadReadPtr((void*)vtable, 0x100)) return;
    
    /* Enable D3D lighting */
    /* device->SetRenderState(D3DRS_LIGHTING, TRUE) */
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
    
    /* Check if position is valid (not all zeros = no ball) */
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
            *(float*)(ball + BALL_ALPHA) = g_charge;  /* invisible at 0 charge */
        }
    }
    
    /* Drain charge */
    g_charge -= CHARGE_DRAIN_RATE;
    if (g_charge < CHARGE_MIN) g_charge = CHARGE_MIN;
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

/* C function called from the asm cave */
void on_render_scene(DWORD gfx) {
    update_light(gfx);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_hooks(void) {
    /* Save original bytes */
    memcpy(g_orig_bytes, (void*)RENDER_SCENE_HOOK, RENDER_SCENE_ORIG_BYTES);
    
    /* Install JMP to our asm cave */
    install_jmp_hook_nop(RENDER_SCENE_HOOK, (DWORD)hook_cave_asm, RENDER_SCENE_ORIG_BYTES);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Collision Event Hook — N:CHARGE recharge pads
 * ═══════════════════════════════════════════════════════════════════════════ */

/* DispatchCollisionEvents is __thiscall(board, ball, collEntry), RET 0x8
   collEntry is a pair of DWORDs: [0]=self, [1]=meshbuffer
   MeshBuffer+0x864 = char* event name
   We hook with a trampoline: call original first, then check for N:CHARGE */

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

/* Called after original DispatchCollisionEvents runs.
   Checks if the collision was with an N:CHARGE mesh — if so, reset charge. */
void __cdecl dce_handler(DWORD board, DWORD ball, DWORD collEntry) {
    if (!collEntry) return;

    DWORD *pair = (DWORD*)collEntry;
    if (IsBadReadPtr(pair, 8)) return;
    if (!pair[1]) return;
    if (IsBadReadPtr((void*)pair[1], 0x868)) return;

    /* Get event name from MeshBuffer+0x864 */
    DWORD namePtr = *(DWORD*)((BYTE*)pair[1] + MESHBUFFER_NAME);
    if (!namePtr || IsBadReadPtr((void*)namePtr, 8)) return;
    const char *eventName = (const char*)namePtr;
    if (!eventName[0]) return;

    /* Check for N:CHARGE */
    if (_strnicmp(eventName, "N:CHARGE", 8) != 0) return;

    /* Reset charge to max */
    g_charge = CHARGE_MAX;
}

static void build_dce_stub(void) {
    /* At entry: ECX=board, [ESP+4]=ball, [ESP+8]=collEntry, RET 0x8
       pushad (32) + pushfd (4) = 36 bytes on stack
         [ESP+36] = return addr
         [ESP+40] = ball
         [ESP+44] = collEntry */
    BYTE *code = (BYTE*)alloc_executable(64);
    int i = 0;
    code[i++] = 0x60;  /* pushad */
    code[i++] = 0x9C;  /* pushfd */
    /* push [esp+0x2C] (collEntry at offset 44=0x2C) */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C;
    /* push [esp+0x2C] (ball was at 40=0x28, now shifted by 4 to 0x2C) */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C;
    /* push ecx (board) */
    code[i++] = 0x51;
    /* call dce_handler (relative) */
    code[i++] = 0xE8;
    *(DWORD*)(code + i) = (DWORD)&dce_handler - (DWORD)(code + i + 4);
    i += 4;
    /* add esp, 12 (cleanup 3 pushed params) */
    code[i++] = 0x83; code[i++] = 0xC4; code[i++] = 0x0C;
    /* popfd */
    code[i++] = 0x9D;
    /* popad */
    code[i++] = 0x61;
    /* jmp to trampoline: mov eax, addr; jmp eax */
    code[i++] = 0xB8;
    *(DWORD*)(code + i) = (DWORD)g_dce_trampoline;
    i += 4;
    code[i++] = 0xFF; code[i++] = 0xE0;

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
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        install_hooks();
        install_dce_hook();
    }
    return TRUE;
}

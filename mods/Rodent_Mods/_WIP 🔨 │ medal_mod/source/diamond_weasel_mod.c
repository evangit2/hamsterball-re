/*
 * diamond_weasel_mod.c — "Diamond Weasel" 5th secret medal (bass.dll proxy)
 * For RodentRacer. Adds a fifth, secret medal per race to Hamsterball.
 *
 *  - Results screen: when the player beats a per-race SECRET time threshold,
 *    the golden weasel medal icon is REPLACED by a "diamond weasel" icon at
 *    the same spot (0x208, 0x63). The normal weasel star-burst still plays
 *    because beating the secret implies beating the weasel par.
 *  - Time-Trial menu: a diamond mini-icon appears just right of the golden
 *    weasel mini-icon once the secret is unlocked.
 *
 *  Config:  diamond_weasel_config.txt   ([RACE] SECRET=<seconds>)
 *  Unlocks: diamond_weasel_unlocks.dat  (persisted per-race flags)
 *  Icon:    diamondweasel.png           (player-provided, in Textures\)
 *
 * Build (MinGW):
 *   i686-w64-mingw32-gcc -shared -o bass.dll diamond_weasel_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

/* ================================================================
 * BASS Proxy Layer (proven from time_warp)
 * ================================================================ */
typedef unsigned long long QWORD;
typedef DWORD HSTREAM;
typedef DWORD HMUSIC;
typedef DWORD HFX;

static HMODULE g_hRealBass = NULL;

typedef BOOL (__stdcall *BASS_Init_t)(int, int, DWORD, HWND, const void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) BOOL __stdcall BASS_Init(int device, int freq, DWORD flags, HWND win, const void *dsguid) {
    if (real_BASS_Init) return real_BASS_Init(device, freq, flags, win, dsguid);
    return TRUE;
}
typedef void (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) void __stdcall BASS_Free(void) {
    if (real_BASS_Free) real_BASS_Free();
}
typedef HSTREAM (__stdcall *BASS_StreamCreateFile_t)(BOOL, const void*, QWORD, QWORD, DWORD);
static BASS_StreamCreateFile_t real_BASS_StreamCreateFile = NULL;
__declspec(dllexport) HSTREAM __stdcall BASS_StreamCreateFile(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags) {
    if (real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(mem, file, offset, length, flags);
    return 0;
}
typedef BOOL (__stdcall *BASS_StreamFree_t)(HSTREAM);
static BASS_StreamFree_t real_BASS_StreamFree = NULL;
__declspec(dllexport) BOOL __stdcall BASS_StreamFree(HSTREAM handle) {
    if (real_BASS_StreamFree) return real_BASS_StreamFree(handle);
    return FALSE;
}
typedef HMUSIC (__stdcall *BASS_MusicLoad_t)(BOOL, const void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) HMUSIC __stdcall BASS_MusicLoad(BOOL mem, const void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(mem, file, offset, length, flags, freq);
    return (HMUSIC)1;
}
typedef BOOL (__stdcall *BASS_MusicFree_t)(HMUSIC);
static BASS_MusicFree_t real_BASS_MusicFree = NULL;
__declspec(dllexport) BOOL __stdcall BASS_MusicFree(HMUSIC handle) {
    if (real_BASS_MusicFree) return real_BASS_MusicFree(handle);
    return FALSE;
}
typedef int (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}
typedef int (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
typedef int (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
typedef int (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
typedef int (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
typedef int (__stdcall *BASS_ChannelPlay_t)(DWORD, BOOL);
static BASS_ChannelPlay_t real_BASS_ChannelPlay = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}
typedef DWORD (__stdcall *BASS_ChannelGetData_t)(DWORD, void*, DWORD);
static BASS_ChannelGetData_t real_BASS_ChannelGetData = NULL;
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD handle, void *buffer, DWORD length) {
    if (real_BASS_ChannelGetData) return real_BASS_ChannelGetData(handle, buffer, length);
    return 0;
}
typedef HFX (__stdcall *BASS_ChannelSetFX_t)(DWORD, DWORD, int);
static BASS_ChannelSetFX_t real_BASS_ChannelSetFX = NULL;
__declspec(dllexport) HFX __stdcall BASS_ChannelSetFX(DWORD handle, DWORD type, int priority) {
    if (real_BASS_ChannelSetFX) return real_BASS_ChannelSetFX(handle, type, priority);
    return 0;
}
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) { (void)a; }
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { (void)a; return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) { (void)a; }
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { (void)a; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) {
    if (real_BASS_ChannelPlay) return real_BASS_ChannelPlay(a, b);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { (void)a; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { (void)a;(void)b; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { (void)a; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { (void)a;(void)b; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { (void)a;(void)b; return 0; }

static void load_real_bass(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (g_hRealBass) {
        real_BASS_Init = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_StreamCreateFile = (BASS_StreamCreateFile_t)GetProcAddress(g_hRealBass, "BASS_StreamCreateFile");
        real_BASS_StreamFree = (BASS_StreamFree_t)GetProcAddress(g_hRealBass, "BASS_StreamFree");
        real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_MusicFree = (BASS_MusicFree_t)GetProcAddress(g_hRealBass, "BASS_MusicFree");
        real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Start = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
        real_BASS_ChannelPlay = (BASS_ChannelPlay_t)GetProcAddress(g_hRealBass, "BASS_ChannelPlay");
        if (!real_BASS_ChannelPlay)
            real_BASS_ChannelPlay = (BASS_ChannelPlay_t)GetProcAddress(g_hRealBass, "_BASS_ChannelPlay@8");
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        if (!real_BASS_ChannelSetAttributes)
            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttribute");
        real_BASS_ChannelGetData = (BASS_ChannelGetData_t)GetProcAddress(g_hRealBass, "BASS_ChannelGetData");
        real_BASS_ChannelSetFX = (BASS_ChannelSetFX_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetFX");
    }
}

/* ================================================================
 * Game addresses (Hamsterball.exe, image base 0x400000)
 * ================================================================ */
#define EXE_BASE           0x00400000
#define APP_PTR            0x005341E0
#define APP_PROFILE        0x220
#define PROFILE_RACE       0x8
#define APP_BOARD          0x178
#define BOARD_TIME         0x1C
#define APP_MGR            0x22C

/* Route 3: write-on-first-unlock diamond art. The PNG bytes are embedded as
 * XOR-encrypted data (so the image cannot be extracted from the DLL before a
 * real unlock). On first unlock the mod decrypts + writes them to the game's
 * Textures\ folder, then the game loads them through its NORMAL file path —
 * no manual D3D texture building, no cache seeding. */
#include "diamond_png_data.h"

#define ICON_LOAD_HOOK     0x42A304
#define GOLD_DRAW_HOOK     0x44EFD2   /* call 0x42c7c0 (gold draw) */
#define WEASEL_DRAW_HOOK   0x44E139   /* call 0x42c7c0 (golden weasel draw) */
#define WEASEL_RET         0x44E13E   /* next instr after 0x44E139 */
#define TT_WEASEL_APPEND   0x42F927   /* call 0x44abf0 (TT menu golden weasel) */
#define SKIP_LATCH_HOOK    0x44CBAA   /* movb $1,0x25(esi)  (skip latch set) */
#define SKIP_LATCH_RET     0x44CBB1   /* next instr after the 5 patched bytes */
#define RESULT_UPDATE_1    0x44CB90   /* results update (vtable 0x4D6CFC slot 1): push ebx;push esi;mov esi,ecx;mov eax,[esi+0x1c] */
#define RESULT_UPDATE_1_RET 0x44CB97  /* next instr after the 7 patched bytes */
#define RESULT_UPDATE_2    0x44B860   /* results update (vtable 0x4D6C00 slot 1): mov eax,[ecx+0x10];mov edx,[ecx+0x14] */
#define RESULT_UPDATE_2_RET 0x44B866  /* next instr after the 6 patched bytes */
#define PAUSE_RCLICK_HOOK  0x4130C9   /* right-click: call 0x40a920 (5B) */
#define PAUSE_RCLICK_RET   0x4130CE   /* next instr after the 5 patched bytes */
#define PAUSE_ESCMSG_HOOK  0x40B40F   /* Win32 ESC: jmp 0x40a920 (5B) */
#define PAUSE_ESCMSG_RET   0x40B414   /* next instr after the 5 patched bytes */
#define SPRITE_DRAW        0x42C7C0
#define ABF0_APPEND        0x44ABF0   /* medal list append (__stdcall, ret 8) */
#define STR_FMT_D          0x4D03F8   /* "%d" */
#define STR_BUF            0x4F7448   /* AthenaString buffer */

/* Graphics_PresentOrEnd (0x455A90): the per-frame Present/EndScene entry,
 * __thiscall(ecx=gfx). Runs AFTER all game logic + rendering and right before
 * the backbuffer is presented. Drawing from here (not from an inline Draw
 * cave) avoids re-entering the game's draw loop. */
#define PRESENT_HOOK       0x455A90   /* mov al,[esp+4]; sub esp,0x20 (7B prologue) */
#define PRESENT_RET        0x455A97   /* next instr after the 7 patched bytes */


/* Golden-weasel white-fade (result-frame keyed). The results screen
 * frame counter is at results+0x10 (incremented each frame by the update
 * fn FUN_0044cb90). The user wants the golden weasel to start turning white
 * ~55 frames into the results screen and be FULLY white by ~150 frames.
 * We drive the game's native color-multiplier (Graphics_SetColorMultiplier,
 * gfx+0x7A8 enable + gfx+0x7B0..0x7BC RGBA scale) up from 1.0 to a saturating
 * value so the sprite blows out to white. Applied only around the weasel draw
 * (cave at 0x44E139 sets it, re-emits the draw, then clears it). */
#define WEASEL_WHITE_START   55
#define WEASEL_WHITE_END     150
#define WEASEL_WHITE_MULT    4.0f      /* saturating color multiplier for pure white */
#define WEASEL_WHITE_HOLD    55        /* frames the white trophy holds with no particles */
/* Total result-frame at which the white hold ends and the trophy reverts to
 * normal gold: (white start) + (active spawns) + (fade tail) + (hold). */
#define WEASEL_WHITE_TOTAL   (WEASEL_WHITE_START + (int)VORTEX_FRAMES + (int)VORTEX_TAIL + WEASEL_WHITE_HOLD)
#define APP_GFX             0x174      /* App+0x174 = gfx ptr */
#define SPRITE_GFX          0x04       /* sprite+4 = gfx ptr (Sprite_DrawRect uses) */
#define SPRITE_WEAEL_APP    0x37C      /* App+0x37C = goldenweasel.png sprite */
#define GFX_MULT_ENABLE     0x7A8      /* gfx+0x7A8 = color-mult enable byte */
#define GFX_MULT_R          0x7B0      /* gfx+0x7B0..0x7BC = RGBA scale */


/* Native medal-award effects (verified 0x44daf8-0x44dc10 in FUN_0044df70):
 * when a medal is FIRST earned the game (1) plays the medal pop sound via
 * Sound_PlayChannel(0x4597b0) on the channel at App+0x50C, and (2) spawns a
 * ring of ArenaScoreParticle objects (ctor 0x44ad50, alloc 0x28) appended to
 * the results-screen particle list ([results+0x94]). The ring is 18 particles
 * spaced 20deg (=0x14) around center (429,317) at radius 30, positions =
 * fcos(angle*PI)*30+429 / fsin(angle*PI)*30+317. */
#define Sound_PlayChannel  0x4597B0
#define SND_CHANNEL_MEDAL  0x50C      /* App+0x50C = medal pop channel */
#define RESULT_PARTICLES   0x94       /* results+0x94 = particle AthenaList */
#define PARTICLE_CTOR      0x44AD50
#define PARTICLE_SIZE      0x28
#define ARENA_SCORE_LIST   0x4F7188   /* wave-const struct (+4 = PI), arg to Wave_Cos/Sin */
#define WAVE_COS           0x457DC0   /* Wave_Cos(0x4F7188, angle) */
#define WAVE_SIN           0x457DA0   /* Wave_Sin(0x4F7188, angle) */
#define PART_ANGLE_MULT    0x4CF528   /* *30.0f radius */
#define PART_CENTER_X      0x4D6D84   /* +429.0f */
#define PART_CENTER_Y      0x4D6D80   /* +317.0f */
#define PART_ANGLE_INC     0x14       /* +20 degrees per particle */
#define PART_ANGLES_END    0x168      /* 360 degrees total -> 18 particles */
#define operator_new_      0x4BA57B

/* ================================================================
 * D3D8 device + render-state constants (verified references:
 * hamsterball-re/references/d3d8-device-vtable-layout.md,
 * hamsterball-sound-system/references/d3d8-screen-rect-overlay.md)
 * Device chain: App(0x5341E0) --+0x174--> gfx --+0x154--> device
 * ================================================================ */
#define APP_DEV_OFFSET     0x174      /* App+0x174 = gfx ptr */
#define GFX_DEV_OFFSET     0x154      /* gfx+0x154 = IDirect3DDevice8* */
#define GFX_VIEW_W         0x15C      /* gfx+0x15C = viewport width (float) */
#define GFX_VIEW_H         0x160      /* gfx+0x160 = viewport height */
#define D3D_DEV_SETRENDERSTATE     0xC8
#define D3D_DEV_SETTEXTURESTAGE    0xFC
#define D3D_DEV_DRAWPRIMITIVEUP    0x120
#define D3D_DEV_SETVERTEXSHADER    0x130   /* vtable[76] SetVertexShader (set FVF) */
#define D3D_DEV_GETVERTEXSHADER    0x134   /* vtable[77] GetVertexShader (get FVF) */
#define D3DRS_SRCBLEND             19
#define D3DRS_DESTBLEND            20
#define D3DRS_ALPHABLENDENABLE     27
#define D3DRS_FOGENABLE            28
#define D3DBLEND_SRCALPHA          5
#define D3DBLEND_INVSRCALPHA       6
#define D3DTSS_COLOROP             0
#define D3DTSS_COLORARG2           2
#define D3DTSS_ALPHAOP             3
#define D3DTSS_ALPHAARG2           5
#define D3DTOP_SELECTARG2          3
#define D3DTA_DIFFUSE              0
#define D3DFVF_XYZRHW              0x001
#define D3DFVF_DIFFUSE             0x040
#define D3DFVF_TLVERTEX            (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

/* Screen-space transformed vertex (D3DFVF_TLVERTEX) — 20 bytes. */
typedef struct {
    float x, y, z, rhw;
    DWORD color;   /* 0xAARRGGBB */
} Diamond_TLVertex;

/* Custom "suction vortex" around the golden weasel during the white-out.
 * Small white streaks fade in randomly around the trophy, spiral inward,
 * converge behind it, then vanish; the cycle runs ~100 result-frames.
 * Drawn by Modulo the game's native sprite draw (before the weasel) via
 * DrawPrimitiveUP so it renders BEHIND the trophy icon. Screen-space math.
 */
#define VORTEX_MAX     20            /* streaks at a time */
#define VORTEX_FRAMES  100           /* ~100-frame active cycle */
#define VORTEX_TAIL    30            /* +30-frame tail: no new spawns, streaks fade */
#define VORTEX_GIFTSHOT_ANG  0.55f   /* random-in-circle start radius multiplier */
#define VORTEX_MINR    6.0f          /* center kill radius (px, screen) */
#define VORTEX_STRETCH 22.0f         /* streak length (px) */
#define VORTEX_SEGS    8             /* subdivisions along the streak length (gradient) */
/* Triangles per streak = SEGS segments * 2 tris (one quad per segment). The
 * DrawPrimitiveUP call is in triangle-units, but the vertex array is sized in
 * VERTICES (6 per triangle). Keep the two quantities explicit and independent
 * so editing VORTEX_SEGS/MAX can never silently index past the array. */
#define VORTEX_TRIS_MAX  (VORTEX_MAX * VORTEX_SEGS * 2)      /* 320 */
#define VORTEX_VERTS_MAX (VORTEX_TRIS_MAX * 6)               /* 1920 */
#define VORTEX_CENTER_FADE 12.0f   /* fade-to-0 as the streak's inner tip nears center */

typedef struct {
    float ax, ay;       /* angle + angular velocity */
    float r, vr;        /* current radius (px) + radial speed (inward) */
    int   born;         /* life counter (all share global cycle time) */
    BYTE  alpha;        /* per-streak opacity 0..255 */
    unsigned char active;
} Diamond_VortexP;

static Diamond_VortexP g_vortex[VORTEX_MAX];
static int g_vortexActive = 0;       /* 1 while the cycle runs */
static int g_vortexFrame = 0;        /* cycle time 0..VORTEX_FRAMES */
static int g_vortexSeeded = 0;
static DWORD g_vortexResults = 0;    /* results obj of the CURRENT vortex session.
                                        Detecting a change lets us reset state + stop
                                        the whoosh when a results screen is exited
                                        mid-cycle (frame never reaches the tail). */
static int g_revealArmed = 0;        /* 1 once a results screen has GENUINELY
                                        appeared (armed by the skip-latch cave).
                                        Gates diamond_present_tick so it NEVER
                                        reads game state during a loading screen
                                        (warp's g_whiteAlpha early-return pattern). */

/* ================================================================
 * Mod globals
 * ================================================================ */
static DWORD g_diamondSprite = 0;
static DWORD g_diamondMiniSprite = 0;
static int   g_secret_cs[15] = {0};   /* per-race DIAMOND threshold in CENTISECONDS (int) */
static int   g_hasSecret[15] = {0};
static BYTE  g_won[15]       = {0};
static int   g_anyDiamond    = 0;    /* 1 once ANY diamond has been earned (gates TT-menu cave) */
static char  g_iconFile[64]  = "diamondweasel.png";
static char  g_miniIconFile[64] = "diamondweasel-icon.png";
static int   g_iconLoaded    = 0;
static int   g_miniIconLoaded = 0;
static int   g_configLoaded  = 0;
static char  g_fmtDiamond[]  = "%dD";
static int   g_diamondDelay  = 165;  /* frames after the gold medal appears that the
                                        diamond appears (3x the game's bronze->
                                        silver (55) and silver->gold (55) gaps) */

/* Hardcoded DIAMOND fallback times (seconds), by tournament slot index 0-14.
 * These are baked into the DLL and used ONLY when racedata.xml has no <DIAMOND>
 * element for that race's block. Listed in the user's requested order. */
static const float g_default_diamond_s[15] = {
    3.5f,   /* WARM-UP     -> BEGINNERRACE   block */
    12.3f,  /* BEGINNER    -> CASCADERACE    block */
    14.0f,  /* INTERMEDIATE-> INTERMEDIATERACE block */
    23.0f,  /* DIZZY       -> DIZZYRACE      block */
    24.0f,  /* TOWER       -> TOWERRACE      block */
    20.0f,  /* UP          -> UPRACE         block */
    28.0f,  /* NEON        -> NEONRACE       block */
    29.0f,  /* EXPERT      -> EXPERTRACE     block */
    12.0f,  /* ODD         -> ODDRACE        block */
    25.0f,  /* TOOB        -> TOOBRACE       block */
    23.0f,  /* WOBBLY      -> WOBBLYRACE     block */
    30.0f,  /* GLASS       -> GLASSRACE      block */
    32.0f,  /* SKY         -> SKYRACE        block */
    40.0f,  /* MASTER      -> MASTERRACE     block */
    26.0f,  /* IMPOSSIBLE  -> IMPOSSIBLERACE block */
};

/* RaceData.xml block name that the game's board ctor loads for each
 * tournament slot (verified against the binary: the jump table at 0x42761c
 * maps slot -> board ctor -> parser call -> the *RACE block name). */
static const char *g_xml_block[15] = {
    "BEGINNERRACE",   /* slot 0  WARM-UP */
    "CASCADERACE",    /* slot 1  BEGINNER */
    "INTERMEDIATERACE",/* slot 2  INTERMEDIATE */
    "DIZZYRACE",      /* slot 3  DIZZY */
    "TOWERRACE",      /* slot 4  TOWER */
    "UPRACE",         /* slot 5  UP */
    "NEONRACE",       /* slot 6  NEON */
    "EXPERTRACE",     /* slot 7  EXPERT */
    "ODDRACE",        /* slot 8  ODD */
    "TOOBRACE",       /* slot 9  TOOB */
    "WOBBLYRACE",     /* slot 10 WOBBLY */
    "GLASSRACE",      /* slot 11 GLASS */
    "SKYRACE",        /* slot 12 SKY */
    "MASTERRACE",     /* slot 13 MASTER */
    "IMPOSSIBLERACE", /* slot 14 IMPOSSIBLE */
};

static char  g_logPath[MAX_PATH] = {0};
static FILE *g_log = NULL;

static unsigned char *g_iconCave = NULL;
static unsigned char *g_dispCave = NULL;
static unsigned char *g_ttCave = NULL;
static unsigned char *g_weaselCave = NULL;
static unsigned char *g_skipCave = NULL;
static unsigned char *g_pauseCave = NULL;
static unsigned char *g_presentCave = NULL;
static unsigned char *g_armCave = NULL;

/* ================================================================
 * Logging + path helpers
 * ================================================================ */
static void diag_log(const char *msg) {
    if (!g_log) { if (g_logPath[0]) g_log = fopen(g_logPath, "a"); }
    if (g_log) { fprintf(g_log, "%s\n", msg); fflush(g_log); }
}
static void diag_logf(const char *fmt, ...) {
    char buf[512]; va_list ap;
    if (!g_log) { if (g_logPath[0]) g_log = fopen(g_logPath, "a"); }
    if (!g_log) return;
    va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    fprintf(g_log, "%s\n", buf); fflush(g_log);
}
static void get_own_dir(char *out, DWORD cap) {
    HMODULE hm = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&diag_log, &hm);
    GetModuleFileNameA(hm, out, cap);
    char *s = strrchr(out, '\\');
    if (s) *s = 0;
}

/* ================================================================
 * Write-on-first-unlock art
 * ================================================================ */
/* Decrypt an embedded PNG byte buffer into a malloc'd buffer. */
static unsigned char *diamond_decrypt_png(const unsigned char *enc, DWORD size,
                                          DWORD key) {
    unsigned char *out = (unsigned char*)malloc(size);
    if (!out) return NULL;
    for (DWORD i = 0; i < size; i++) out[i] = enc[i] ^ key;
    return out;
}

/* Write a decrypted PNG to `Textures\name`. Returns 1 on success (or if the
 * file already exists). The game dir is the DLL's own dir (bass.dll sits next
 * to Hamsterball.exe); Textures\ is under that. */
static int diamond_write_png(const char *name, const unsigned char *enc,
                             DWORD size, DWORD key) {
    char dir[MAX_PATH], path[MAX_PATH];
    unsigned char *dec;
    FILE *f;
    get_own_dir(dir, sizeof(dir));
    snprintf(path, sizeof(path), "%s\\Textures\\%s", dir, name);
    dec = diamond_decrypt_png(enc, size, key);
    if (!dec) return 0;
    f = fopen(path, "wb");
    if (!f) { free(dec); return 0; }
    fwrite(dec, 1, size, f);
    fclose(f);
    free(dec);
    diag_logf("[diamond] wrote %s (%u bytes)", path, size);
    return 1;
}

/* Materialize both diamond PNGs into Textures\ if they don't already exist.
 * Called when the first diamond is unlocked. Returns 1 if both are present
 * afterward. */
static int diamond_materialize_pngs(void) {
    char dir[MAX_PATH], path[MAX_PATH];
    DWORD attr;
    int medal = 1, mini = 1;
    get_own_dir(dir, sizeof(dir));
    snprintf(path, sizeof(path), "%s\\Textures\\diamondweasel.png", dir);
    attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES)
        medal = diamond_write_png("diamondweasel.png",
                                  diamondweasel_png_data, diamondweasel_png_SIZE,
                                  diamondweasel_png_XORKEY);
    snprintf(path, sizeof(path), "%s\\Textures\\diamondweasel-icon.png", dir);
    attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES)
        mini = diamond_write_png("diamondweasel-icon.png",
                                 diamondweasel_icon_png_data,
                                 diamondweasel_icon_png_SIZE,
                                 diamondweasel_icon_png_XORKEY);
    return (medal && mini) ? 1 : 0;
}

/* ================================================================
 * ATOMIC UNLOCK — all-or-nothing commit gate
 * ================================================================
 * Provisioning a diamond unlock performs SEVERAL writes (registry flag,
 * PNG assets, and any future effects/assets). We only commit the unlock if
 * EVERY one of them succeeds. If any write fails, we return 0 and the caller
 * treats the diamond as if it was never earned this run — no draw, no
 * persistence, no effects. This keeps the unlock atomic: a partial unlock
 * (e.g. flag saved but PNG failed, or vice versa) can never happen.
 *
 * Add every new effect/asset write here so the whole unlock stays all-or-nothing.
 */
static int save_unlocks_reg(void);   /* defined below (registry save) */

static int diamond_provision_unlock(int race) {
    int ok_pngs, ok_reg;
    /* 1. Materialize the PNG assets FIRST. If this fails, nothing has touched
     *    the registry yet — the unlock stays fully uncommitted (no partial
     *    state: no flag persisted without its icon). */
    ok_pngs = diamond_materialize_pngs();
    if (!ok_pngs) {
        /* g_won[race] is still 0 here — never set it, never touch reg. */
        diag_logf("[diamond] provision FAILED for race %d (pngs=%d) — unlock not committed",
                  race, ok_pngs);
        return 0;
    }
    /* 2. Only now set the in-memory flag and persist it to the registry. */
    g_won[race] = 1;
    g_anyDiamond = 1;   /* a diamond now exists -> TT-menu cave eligible next launch */
    ok_reg = save_unlocks_reg();
    if (!ok_reg) {
        g_won[race] = 0;   /* roll back — flag would exist in memory but not on disk */
        diag_logf("[diamond] provision FAILED for race %d (reg=%d) — unlock rolled back",
                  race, ok_reg);
        return 0;
    }
    diag_logf("[diamond] provision OK for race %d", race);
    return 1;
}

/* ================================================================
 * Config
 * ================================================================ */
/* Read a <DIAMOND> time from racedata.xml the same way the game reads its
 * other medal times. The game opens exactly "racedata.xml" (relative to the
 * working directory, string at 0x4cf5d0) and parser 0x40A120 matches each
 * <*RACENAME> block by the block name its board ctor passes. We mirror that:
 * for each tournament slot we look up the block name the game actually loads
 * (g_xml_block[slot]) and, if that block contains a <DIAMOND> element, use it
 * as the diamond threshold for that race. Blocks with no <DIAMOND> keep the
 * hardcoded DLL default. */
static void load_racedata_xml(void) {
    FILE *f = fopen("racedata.xml", "r");
    char line[512];
    int cur = -1;   /* currently-open XML block index, or -1 none */
    if (!f) { diag_log("[diamond] racedata.xml not found (using DLL defaults)"); return; }
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p == 0) continue;
        /* Race-block opener: <BEGINNERRACE>, <CASCADERACE>, ... Only these switch
         * the current block. Field tags inside a block (<DIAMOND>, <WEASEL>, ...)
         * are NOT block openers and must not disturb `cur`. */
        if (*p == '<' && p[1] != '/') {
            char *end = strchr(p, '>');
            if (end) {
                int is_block = 0;
                for (int i = 0; i < 15; i++) {
                    if (_strnicmp(p + 1, g_xml_block[i], strlen(g_xml_block[i])) == 0) {
                        cur = i; is_block = 1; break;
                    }
                }
                if (is_block) continue;   /* switched block; skip to next line */
            }
        }
        if (cur < 0 || cur > 14) continue;
        /* <DIAMOND>4.2</DIAMOND>  (whole thing on one line, like every other field) */
        if (_strnicmp(p, "<DIAMOND>", 9) == 0) {
            char *val = p + 9;
            char *end = strstr(val, "</DIAMOND>");
            if (end) {
                *end = 0;
                double v = atof(val);
                g_secret_cs[cur] = (int)(v * 100.0);
                g_hasSecret[cur] = 1;
                diag_logf("[diamond] racedata.xml race %d <DIAMOND>=%.2f", cur, v);
            }
        }
    }
    fclose(f);
}

/* Seed per-race diamond thresholds: start from the hardcoded DLL defaults,
 * then let racedata.xml <DIAMOND> elements override them (same file the game
 * reads, matched the same way). No config file anymore. */
static void init_thresholds(void) {
    int i;
    for (i = 0; i < 15; i++) {
        g_secret_cs[i] = (int)(g_default_diamond_s[i] * 100.0f);
        g_hasSecret[i] = 1;
    }
    load_racedata_xml();
    g_configLoaded = 1;
}
#define REG_KEY        "Software\\Raptisoft\\Hamsterball"
#define REG_VAL_MEDALS "DiamondMedals"
#define DIAMOND_REGWIN  HKEY_CURRENT_USER

static int save_unlocks_reg(void) {
    HKEY hk;
    DWORD disp, len = 15;
    if (RegCreateKeyExA(DIAMOND_REGWIN, REG_KEY, 0, NULL, 0, KEY_WRITE,
                        NULL, &hk, &disp) != ERROR_SUCCESS) {
        diag_log("[diamond] reg create key failed");
        return 0;
    }
    if (RegSetValueExA(hk, REG_VAL_MEDALS, 0, REG_BINARY, g_won, len) != ERROR_SUCCESS) {
        RegCloseKey(hk);
        diag_log("[diamond] reg set value failed");
        return 0;
    }
    RegCloseKey(hk);
    diag_logf("[diamond] DiamondMedals registry value written (%u bytes)", (unsigned)len);
    return 1;
}
static void load_unlocks(void) {
    HKEY hk;
    DWORD type, size = 15;
    if (RegOpenKeyExA(DIAMOND_REGWIN, REG_KEY, 0, KEY_READ, &hk) != ERROR_SUCCESS) {
        diag_logf("[diamond] no DiamondMedals registry key");
        return;
    }
    if (RegQueryValueExA(hk, REG_VAL_MEDALS, NULL, &type, g_won, &size) == ERROR_SUCCESS
        && type == REG_BINARY && size <= 15) {
        diag_logf("[diamond] loaded %u diamond unlock flags from registry", size);
    } else {
        diag_log("[diamond] no DiamondMedals registry value (or unexpected)");
    }
    RegCloseKey(hk);
    /* has the player EVER earned a diamond? (gates the TT-menu mini-icons) */
    for (int i = 0; i < 15; i++) if (g_won[i]) { g_anyDiamond = 1; break; }
}

/* ================================================================
 * Runtime helpers
 * ================================================================ */
static DWORD get_app(void) {
    if (IsBadReadPtr((void*)APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return 0;
    return app;
}
static int get_race_index(void) {
    DWORD app = get_app();
    if (!app) return -1;
    if (IsBadReadPtr((void*)(app + APP_PROFILE), 4)) return -1;
    DWORD prof = *(DWORD*)(app + APP_PROFILE);
    if (!prof) return -1;
    if (IsBadReadPtr((void*)(prof + PROFILE_RACE), 4)) return -1;
    return *(int*)(prof + PROFILE_RACE);
}
/* board+0x1C is an INTEGER centisecond counter (native medal code compares
 * it with CMP at 0x44d932 / 0x44d958), NOT a float. Reading it as float
 * reinterprets e.g. 3000 (30s) as a denormal ~4e-42, which is always
 * "beats the secret" -> silent unlock on every race. */
static int get_player_time_cs(DWORD app) {
    if (IsBadReadPtr((void*)(app + APP_BOARD), 4)) return 0;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_TIME), 4)) return 0;
    return *(int*)(board + BOARD_TIME);
}

/* ================================================================
 * Core logic (called from caves)
 * ================================================================ */
__attribute__((used)) void diamond_load_icon_impl(DWORD app);
__attribute__((used)) void diamond_spawn_medal_effects(DWORD results, DWORD app);

/* Arming signal for the present hook: set g_revealArmed=1. Called from a
 * cave at the results screen's per-frame update (vtable slot 1, 0x44CB90),
 * which runs EVERY FRAME the results screen is live — and never during a
 * loading screen. The present tick refuses to read game state until this
 * flag is set, so a half-built board (title screen's Level4-Trapdoor2
 * background load) can never be dereferenced. */
__attribute__((used)) void diamond_arm_reveal(void) {
    g_revealArmed = 1;
}

static int diamond_seq_frame(DWORD results);   /* frames since gold award */

/* BLOCK the results-screen click/keypress skip when the diamond time was
 * achieved for the current race. The skip latch (results+0x25) drives the
 * frame counter forward at 10x/frame (multiplier computed at 0x44cbcf),
 * which would blast past the diamond's frame-240 reveal before the player
 * can see it. Returning 1 forces the multiplier to 0 (normal speed) so the
 * reveal plays out; returning 0 leaves skip fully functional (diamond NOT
 * achieved this race -> skip exactly as before). Only blocks while the
 * reveal could still be missed: after the reveal frame (240) the skip is
 * allowed again so the player isn't stuck watching the white hold.
 */
__attribute__((used)) int diamond_block_skip(DWORD results) {
    int frame, race, cs, thr;
    DWORD app;
    if (!results) return 0;
    if (IsBadReadPtr((void*)(results + 0x10), 4)) return 0;   /* RESULT_FRAME */
    frame = diamond_seq_frame(results);          /* frames since gold award */
    /* After the reveal (gold + 240) the skip is safe — allow it. */
    if (frame >= WEASEL_WHITE_TOTAL) return 0;
    /* Only relevant while the results screen is awarding medals. */
    if (IsBadReadPtr((void*)(results + 0x0C), 4)) return 0;   /* RESULT_APP */
    app = *(DWORD*)(results + 0x0C);
    if (!app || !g_configLoaded) return 0;
    race = get_race_index();
    if (race < 0 || race > 14) return 0;
    if (!g_hasSecret[race]) return 0;
    if (g_won[race]) return 0;                 /* only on the FIRST earn */
    cs = get_player_time_cs(app);
    thr = g_secret_cs[race];
    /* Block skip iff the diamond time was met (and the reveal hasn't passed). */
    return (cs > 0 && cs <= thr) ? 1 : 0;
}

/* BLOCK the PAUSE menu while the diamond reveal is pending, mirroring the
 * skip-latch blocker above. The player must not be able to pause and interrupt
 * the diamond reveal (goal touch -> frame 240). Returning 1 suppresses the
 * pause menu entirely; returning 0 lets it open normally.
 *
 * Called from the pause call-site caves with the SCENE object (ecx/esi). The
 * results object is created at goal-touch and lives in the scene's results
 * list at scene+0x8B8 (AthenaList: count +4, items +0x40C). We locate it,
 * confirm it's a genuine results object (vtable 0x4d6cfc), and block pause
 * only while its frame counter is before the reveal (WEASEL_WHITE_TOTAL=240)
 * AND the diamond was actually earned for the current race. If anything is
 * unreadable or the scene isn't holding a results screen, we allow pause
 * (return 0) so normal gameplay pause is never affected.
 */
#define SCENE_RESULTS_LIST   0x8B8   /* scene+0x8B8 = results AthenaList */
#define SCENE_LIST_COUNT     0x04    /* AthenaList count */
#define SCENE_LIST_ITEMS     0x40C   /* AthenaList items ptr */
#define RESULTS_VTABLE       0x4D6CFC
__attribute__((used)) int diamond_pause_blocked(DWORD scene) {
    DWORD list, items, results, app, vt;
    int count, frame, race, cs, thr;
    if (!scene) return 0;
    if (IsBadReadPtr((void*)(scene + SCENE_RESULTS_LIST), 4)) return 0;
    list = scene + SCENE_RESULTS_LIST;
    if (IsBadReadPtr((void*)(list + SCENE_LIST_COUNT), 4)) return 0;
    count = *(int*)(list + SCENE_LIST_COUNT);
    if (count <= 0) return 0;                       /* no results screen -> allow */
    if (IsBadReadPtr((void*)(list + SCENE_LIST_ITEMS), 4)) return 0;
    items = *(DWORD*)(list + SCENE_LIST_ITEMS);
    if (!items || IsBadReadPtr((void*)items, 4)) return 0;
    results = *(DWORD*)items;                        /* first item = results object */
    if (!results || IsBadReadPtr((void*)(results + 0x10), 4)) return 0;   /* RESULT_FRAME */
    /* Confirm it's really a results object (vtable matches the ctor-set one). */
    if (IsBadReadPtr((void*)results, 4)) return 0;
    vt = *(DWORD*)results;
    if (vt != RESULTS_VTABLE && vt != 0x4D6C00) return 0;
    frame = diamond_seq_frame(results);          /* frames since gold award */
    if (frame < 0 || frame >= WEASEL_WHITE_TOTAL) return 0;   /* reveal done -> allow */
    /* Only block when the diamond was actually earned for this race. */
    if (IsBadReadPtr((void*)(results + 0x0C), 4)) return 0;   /* RESULT_APP */
    app = *(DWORD*)(results + 0x0C);
    if (!app || !g_configLoaded) return 0;
    race = get_race_index();
    if (race < 0 || race > 14) return 0;
    if (!g_hasSecret[race]) return 0;
    if (g_won[race]) return 0;                 /* only on the FIRST earn */
    cs = get_player_time_cs(app);
    thr = g_secret_cs[race];
    return (cs > 0 && cs <= thr) ? 1 : 0;
}

/* Spawn the native medal-award effects (pop sound + star ring) for the
 * diamond, mirroring FUN_0044df70's first-earn block (0x44daf8-0x44dc10).
 * Called once, on the frame the diamond is first unlocked+shown for a race.
 *   results = results-screen object (particle list at +0x94)
 *   app     = App ptr (medal pop channel at +0x50C)
 */
__attribute__((used)) void diamond_spawn_medal_effects(DWORD results, DWORD app) {
    int angle, i;
    DWORD channel, plist, part;
    if (!results || !app) return;
    if (IsBadReadPtr((void*)(results + RESULT_PARTICLES), 4)) return;
    plist = results + RESULT_PARTICLES;
    /* (1) Play the medal pop on the medal channel (App+0x50C). */
    if (IsBadReadPtr((void*)(app + SND_CHANNEL_MEDAL), 4)) return;
    channel = *(DWORD*)(app + SND_CHANNEL_MEDAL);
    if (channel) {
        /* Sound_PlayChannel is __fastcall(param_1=ecx) — the single real arg.
         * (Native also pushes a stray 1.0, but the callee only reads ecx; we
         * omit it to keep our own stack perfectly balanced.) */
        __asm__ volatile(
            "movl %1, %%ecx\n\t"     /* channel (ecx = this) */
            "call *%0\n\t"           /* Sound_PlayChannel(channel) */
            : : "r"((void*)Sound_PlayChannel), "r"(channel)
            : "eax", "ecx", "edx", "memory");
    }
    /* (2) Spawn the ring of 18 ArenaScoreParticle objects (one per 20deg). */
    for (angle = 0, i = 0; angle < PART_ANGLES_END; angle += PART_ANGLE_INC, i++) {
        /* operator_new(PARTICLE_SIZE) */
        __asm__ volatile(
            "pushl %1\n\t"
            "call *%2\n\t"
            "addl $4, %%esp\n\t"
            : "=a"(part)
            : "r"(PARTICLE_SIZE), "r"((void*)operator_new_)
            : "ecx", "edx", "memory");
        if (!part) continue;
        /* ArenaScoreParticle_ctor(part, app): __thiscall(ecx=part, stack arg app),
         * ret $4 — callee pops the stack arg, so NO addl esp after. */
        __asm__ volatile(
            "movl %2, %%ecx\n\t"
            "pushl %1\n\t"
            "call *%0\n\t"
            : : "r"((void*)PARTICLE_CTOR), "r"(app), "r"(part)
            : "eax", "ecx", "edx", "memory");
        /* Native layout (verified): part+0x08..0x10 = POSITION vec3,
         * part+0x14..0x1C = VELOCITY/trajectory vec3 (radial dir, no center
         * offset), part+0x20 = 1.0 scale (set by ctor).
         *   pos.x = cos(a)*r + 429   pos.y = sin(a)*r + 317
         *   vel.x = cos(a)*r         vel.y = sin(a)*r        */
        {
            float rad = (float)angle * 3.14159265f / 180.0f;
            float c = cosf(rad), s = sinf(rad);
            float cx = 429.0f, cy = 317.0f, r = 30.0f;
            *(float*)(part + 0x08) = c * r + cx;
            *(float*)(part + 0x0C) = s * r + cy;
            *(float*)(part + 0x10) = 0.0f;
            *(float*)(part + 0x14) = c * r;
            *(float*)(part + 0x18) = s * r;
            *(float*)(part + 0x1C) = 0.0f;
        }
        /* AthenaList_Append(&results.particles, part): __thiscall(ecx=list,
         * arg on stack), ret $4 — callee pops the arg, so NO addl esp after. */
        __asm__ volatile(
            "movl %2, %%ecx\n\t"
            "pushl %1\n\t"
            "call *%0\n\t"
            : : "r"((void*)0x453810), "r"(part), "r"(plist)
            : "eax", "ecx", "edx", "memory");
    }
    diag_logf("[diamond] medal effects spawned (pop + %d star particles) for race", i);
}

/* RESULT_OBJ offsets used by the weasel white-fade + diamond 5th-medal. */
#define RESULT_FRAME   0x10   /* frame counter [esi+0x10] */
#define RESULT_GOLD    0x4c   /* gold medal awarded when frame == [esi+0x4c];
                                 drawn when frame > [esi+0x4c] (cmp 0x44e113).
                                 NOTE: was mislabeled RESULT_GATE_GOLD 0x74 —
                                 0x74 is a DIFFERENT medal's gate (0x44dd67). */
#define RESULT_APP     0x0C   /* App ptr [esi+0xc] */

/* Frames SINCE the gold medal was awarded: sequence frame = results+0x10
 * (frame counter) minus results+0x4c (the gold-award gate). The unlock
 * sequence (white-out, vortex, reveal) is rebased to this so it starts the
 * moment gold is awarded, per the user's design: reveal lands at gold+240.
 * Returns the raw frame counter if +0x4c is unreadable (defensive), and
 * clamps below 0 in case the counter is read before the gate is set. */
__attribute__((used)) static int diamond_seq_frame(DWORD results) {
    int frame, gold;
    if (!results) return 0;
    if (IsBadReadPtr((void*)(results + RESULT_FRAME), 4)) return 0;
    frame = *(int*)(results + RESULT_FRAME);
    if (IsBadReadPtr((void*)(results + RESULT_GOLD), 4)) return frame;
    gold = *(int*)(results + RESULT_GOLD);
    if (gold < 0) gold = 0;
    return (frame < gold) ? 0 : (frame - gold);
}

/* True only while the diamond's FIRST-EARN reveal is running for the current
 * race: the diamond time was met AND the race hasn't been earned before.
 * The white-out, vortex, and trophy-swap reveal all gate on this so they play
 * only on the genuine first earn — on a replay the golden weasel renders
 * normal gold, then the diamond just swaps in at gold+240 with no buildup. */
__attribute__((used)) static int diamond_first_earn(DWORD results) {
    int race, cs, thr;
    DWORD app;
    if (!results) return 0;
    if (!g_configLoaded) return 0;
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return 0;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app) return 0;
    race = get_race_index();
    if (race < 0 || race > 14) return 0;
    if (!g_hasSecret[race]) return 0;
    if (g_won[race]) return 0;                 /* already earned -> replay */
    cs = get_player_time_cs(app);
    thr = g_secret_cs[race];
    return (cs > 0 && cs <= thr) ? 1 : 0;
}

/* Set the golden-weasel sprite's color-multiplier so it renders white,
 * phased over result-frames [55,150]. Sets gfx+0x7A8=1 and the RGBA scale at
 * gfx+0x7B0..0x7BC to (m,m,m,1). gfx comes from the weasel sprite (sprite+4).
 */
__attribute__((used)) void diamond_weasel_mult(DWORD results) {
    int frame;
    float m, *sc;
    DWORD app, sprite, gfx;
    if (!results) return;
    if (IsBadReadPtr((void*)(results + RESULT_FRAME), 4)) return;
    if (!diamond_first_earn(results)) return;   /* replay -> no white-out */
    frame = diamond_seq_frame(results);          /* frames since gold award */
    if (frame <= WEASEL_WHITE_START) return;      /* not fading yet */
    if (frame >= WEASEL_WHITE_TOTAL) return;      /* white hold over -> normal gold */
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app || IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) return;
    sprite = *(DWORD*)(app + SPRITE_WEAEL_APP);
    if (!sprite || IsBadReadPtr((void*)(sprite + SPRITE_GFX), 4)) return;
    gfx = *(DWORD*)(sprite + SPRITE_GFX);
    if (!gfx || IsBadReadPtr((void*)(gfx + GFX_MULT_R), 4)) return;
    if (frame >= WEASEL_WHITE_END) m = WEASEL_WHITE_MULT;
    else m = 1.0f + (WEASEL_WHITE_MULT - 1.0f) *
            ((float)(frame - WEASEL_WHITE_START) / (float)(WEASEL_WHITE_END - WEASEL_WHITE_START));
    *(volatile unsigned char*)(gfx + GFX_MULT_ENABLE) = 1;
    sc = (float*)(gfx + GFX_MULT_R);
    sc[0] = m; sc[1] = m; sc[2] = m; sc[3] = 1.0f;
    diag_logf("[diamond] weasel white frame=%d mult=%.2f", frame, m);
}

/* Clear the weasel color-multiplier back to identity (all 1.0) so
 * subsequent draws are unaffected. */
__attribute__((used)) void diamond_weasel_mult_clear(DWORD results) {
    DWORD app, sprite, gfx;
    float *sc;
    if (!results) return;
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app || IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) return;
    sprite = *(DWORD*)(app + SPRITE_WEAEL_APP);
    if (!sprite || IsBadReadPtr((void*)(sprite + SPRITE_GFX), 4)) return;
    gfx = *(DWORD*)(sprite + SPRITE_GFX);
    if (!gfx || IsBadReadPtr((void*)(gfx + GFX_MULT_R), 4)) return;
    *(volatile unsigned char*)(gfx + GFX_MULT_ENABLE) = 0;
    sc = (float*)(gfx + GFX_MULT_R);
    sc[0] = sc[1] = sc[2] = sc[3] = 1.0f;
}

/* ================================================================
 * Golden-weasel "suction vortex" (raw D3D8 screen-space streaks)
 *
 * Small white streaks fade in at random angles/radii around the trophy,
 * spiral inward toward its center, converge behind it, then vanish. The
 * whole cycle runs ~VORTEX_FRAMES (100) result-frames, in sync with the
 * white-out (started at WEASEL_WHITE_START, ends at WEASEL_WHITE_END).
 *
 * RENDERING BEHIND THE TROPHY: the vortex is drawn by the weasel cave
 * BEFORE the weasel sprite draw, using DrawPrimitiveUP in screen-space
 * (D3DFVF_TLVERTEX quads). Because it is issued first (and the trophy
 * writes Z / draws opaque on top), the streaks render underneath it.
 *
 * Coordinate space: the trophy sits at world (0x208,0x63); its on-screen
 * center is obtained by calling Gfx_TransformX/Y(0x453e90/0x453eb0) once
 * per cycle and cached, so the vortex tracks the trophy at any resolution.
 * ================================================================ */
#define GFX_TRANSFORM_X   0x453E90    /* __thiscall(ecx=gfx, float x)->FPU */
#define GFX_TRANSFORM_Y   0x453EB0
#define VORTEX_WORLD_X    0x208       /* golden weasel world pos X */
#define VORTEX_WORLD_Y    0x63
static float g_vortexCx = 0.0f, g_vortexCy = 0.0f;   /* trophy screen center */

/* Seed rand once. */
static void vortex_seed(void) {
    if (!g_vortexSeeded) { srand(12345); g_vortexSeeded = 1; }
}

/* Random float in [0,1). */
static float vortex_frand(void) {
    return (float)(rand() & 0x7FFF) / 32768.0f;
}

/* Cached trophy screen center — replicate Gfx_TransformX/Y (0x453e90/0x453eb0)
 * arithmetic directly in C (verified from decompile):
 *   screenX = worldX * *(gfx+0x5C)->0x1F8 + *(int*)(gfx+0x798)
 *   screenY = worldY * *(gfx+0x5C)->0x1FC + *(int*)(gfx+0x79C)
 * The golden-weasel draw passes the sprite's TOP-LEFT world corner (0x208,0x63).
 * The sprite spans [sprite+0xC8] x [sprite+0xCC] (width/height, world units),
 * so its true center is top-left + half dims. We transform that center to
 * screen so the vortex converges on the middle of the trophy.
 * This avoids fragile inline-asm __thiscall + FPU-return calling. */
static void vortex_compute_center(DWORD gfx, DWORD sprite) {
    DWORD scale = 0;
    float cxw, cyw, sx = 0.0f, sy = 0.0f;
    float w = 0.0f, h = 0.0f;
    if (!gfx) return;
    if (sprite) {
        if (!IsBadReadPtr((void*)(sprite + 0xC8), 8)) {
            w = *(float*)(sprite + 0xC8);
            h = *(float*)(sprite + 0xCC);
        }
    }
    cxw = (float)VORTEX_WORLD_X + w * 0.5f;
    cyw = (float)VORTEX_WORLD_Y + h * 0.5f;
    if (IsBadReadPtr((void*)(gfx + 0x5C), 4)) return;
    scale = *(DWORD*)(gfx + 0x5C);
    if (!scale || IsBadReadPtr((void*)(scale + 0x1FC + 4), 4)) return;
    sx = cxw * *(float*)(scale + 0x1F8) + (float)(*(int*)(gfx + 0x798));
    sy = cyw * *(float*)(scale + 0x1FC) + (float)(*(int*)(gfx + 0x79C));
    if (sx > -5000.0f && sx < 5000.0f && sy > -5000.0f && sy < 5000.0f) {
        g_vortexCx = sx; g_vortexCy = sy;
    }
}

/* Reset the vortex cycle. Called by the weasel cave at WEASEL_WHITE_START. */
static void vortex_sound_start(void);
static void vortex_sound_stop(void);
static void vortex_start_cycle(DWORD gfx, DWORD sprite) {
    int i;
    if (!gfx) return;
    vortex_seed();
    vortex_compute_center(gfx, sprite);
    g_vortexFrame = 0;
    g_vortexActive = 1;
    for (i = 0; i < VORTEX_MAX; i++) {
        g_vortex[i].active = 0;
        g_vortex[i].alpha = 0;
        g_vortex[i].r = 0.0f;
        g_vortex[i].born = 0;
    }
    vortex_sound_start();   /* start looping whoosh for the vortex window */
}

/* Advance one vortex streak (spiral-in + fade). Called every frame the
 * cycle is active, for every streak slot.
 * Alpha timeline (by the streak's own age p->born):
 *   born 0..15           -> ease in (0..255)
 *   active window        -> 255 (sustain)
 *   tail (frame>=VORTEX_FRAMES) -> fade to nothing across VORTEX_TAIL frames */
static void vortex_update_streak(Diamond_VortexP *p, int frame) {
    float a;
    if (!p->active) return;
    /* angle (curl) */
    a = p->ax + p->ay * p->born;
    /* gradual inward pull */
    p->r -= p->vr;
    /* The rectangle's inner tip is at r - VORTEX_STRETCH. Kill the particle once
     * that tip reaches the center (r - STRETCH <= 0) so the streak's leading end
     * disappears exactly at the center rather than passing through it and
     * sticking out the far side. */
    if (p->r - VORTEX_STRETCH <= 0.0f || p->r > 4000.0f) { p->active = 0; return; }
    /* ease-in over first 15 frames of this streak's life */
    if (p->born < 15) {
        p->alpha = (BYTE)((p->born * 17) & 0xFF);
    } else if (frame >= (int)VORTEX_FRAMES) {
        /* tail: fade to nothing across the VORTEX_TAIL frames */
        int tail_elapsed = frame - (int)VORTEX_FRAMES;
        int rem = (int)VORTEX_TAIL - tail_elapsed;
        if (rem <= 0) p->alpha = 0;
        else p->alpha = (BYTE)((rem * 255) / (int)VORTEX_TAIL);
    } else {
        p->alpha = 255;
    }
    /* Fade out as the inner tip approaches the center, reaching fully
     * transparent exactly when the tip hits 0 (center). This prevents the
     * streak from visibly sticking out the far side before it despawns. */
    {
        float tip = p->r - VORTEX_STRETCH;            /* inner tip radius (0 = at center) */
        if (tip < VORTEX_CENTER_FADE) {
            float f = tip / VORTEX_CENTER_FADE;        /* 1..0 as tip -> center */
            if (f <= 0.0f) p->alpha = 0;
            else p->alpha = (BYTE)((int)p->alpha * (f * f));
        }
    }
    p->born++;
    (void)a;
}

/* Draw all active streaks around the trophy center as white screen-space
 * quads (2 triangles each). Runs on the main thread inside the weasel cave
 * (PUSHAD/POPAD), so D3D calls are safe. gfx is the graphics object. */
static void vortex_draw(DWORD gfx) {
    int i, n = 0;                    /* triangles emitted */
    int vc = 0;                      /* vertices filled (independent cursor) */
    DWORD device, vt;
    Diamond_TLVertex verts[VORTEX_VERTS_MAX];
    Diamond_TLVertex *v;
    float cx, cy, s, rr;
    float ang, cang, sang;
    BYTE a;
    /* resolve device */
    if (!g_vortexActive) return;
    if (!gfx || IsBadReadPtr((void*)(gfx + GFX_DEV_OFFSET), 4)) return;
    device = *(DWORD*)(gfx + GFX_DEV_OFFSET);
    if (!device || IsBadReadPtr((void*)device, 4)) return;
    vt = *(DWORD*)device;
    if (!vt || IsBadReadPtr((void*)(vt + D3D_DEV_DRAWPRIMITIVEUP), 4)) return;
    cx = g_vortexCx; cy = g_vortexCy;
    /* Build each streak as VORTEX_SEGS short rectangles stacked along the
     * inward direction. Vertex alpha ramps 0 (outer tip) -> peak (middle) -> 0
     * (inner tip): a triangular "tent" profile giving a soft gradient that
     * fades to nothing at both ends of the streak. */
    for (i = 0; i < VORTEX_MAX && n < VORTEX_TRIS_MAX; i++) {
        Diamond_VortexP *p = &g_vortex[i];
        if (!p->active || p->alpha == 0) continue;
        ang = p->ax + p->ay * p->born;
        cang = (float)cos(ang); sang = (float)sin(ang);
        rr = p->r;
        s  = 0.5f;                    /* rectangle half-thickness (px) — thin */
        a  = p->alpha;
        {
            float px_ = -sang, py_ = cang;              /* perpendicular */
            float outer = rr;                            /* outer radius */
            float inner = rr - VORTEX_STRETCH;           /* inner radius */
            int seg;
            for (seg = 0; seg < VORTEX_SEGS; seg++) {
                /* positions along the length [0..1] for this segment's edges */
                float t0 = (float)seg              / (float)VORTEX_SEGS;
                float t1 = (float)(seg + 1)        / (float)VORTEX_SEGS;
                /* axial position: outer edge t=0, inner edge t=1 */
                float r0 = outer + (inner - outer) * t0;   /* this segment's outer radius */
                float r1 = outer + (inner - outer) * t1;   /* this segment's inner radius */
                /* tent alpha profile: peak at t=0.5, 0 at both ends (t=0, t=1).
                 * a_tip alpha at outer(0) and inner(1) = 0; peak ~ a at 0.5. */
                float mid = 0.5f;
                float f0 = (t0 <= mid) ? (t0 / mid) : ((1.0f - t0) / (1.0f - mid));
                float f1 = (t1 <= mid) ? (t1 / mid) : ((1.0f - t1) / (1.0f - mid));
                int a0 = (int)(a * (f0 * f0));   /* smoother falloff (quadratic) */
                int a1 = (int)(a * (f1 * f1));
                if (vc + 6 > VORTEX_VERTS_MAX) break;   /* hard guard, independent of n */
                /* corners of this quad (A outer-left, B outer-right, C inner-right, D inner-left) */
                v = &verts[vc];
                v[0].x = cx + cang*r0 + px_*s;  v[0].y = cy + sang*r0 + py_*s;  /* A */
                v[1].x = cx + cang*r0 - px_*s;  v[1].y = cy + sang*r0 - py_*s;  /* B */
                v[2].x = cx + cang*r1 - px_*s;  v[2].y = cy + sang*r1 - py_*s;  /* C */
                v[3].x = cx + cang*r0 + px_*s;  v[3].y = cy + sang*r0 + py_*s;  /* A */
                v[4].x = cx + cang*r1 - px_*s;  v[4].y = cy + sang*r1 - py_*s;  /* C */
                v[5].x = cx + cang*r1 + px_*s;  v[5].y = cy + sang*r1 + py_*s;  /* D */
                for (int k = 0; k < 6; k++) {
                    int ka = (k == 0 || k == 3) ? a0 : a1;   /* outer pair -> a0, inner pair -> a1 */
                    v[k].z = 0.0f;
                    v[k].rhw = 1.0f;
                    v[k].color = (DWORD)(ka << 24) | 0x00FFFFFF;  /* white, gradient alpha */
                }
                vc += 6;
                n += 2;   /* this segment = 2 triangles */
            }
        }
    }
    if (n == 0) return;
    /* --- render setup (see d3d8-screen-rect-overlay recipe) ---
     * Use proper C function pointers to the device vtable slots. All D3D8
     * device methods are __stdcall, so the callee cleans the stack. */
    {
        typedef HRESULT (__stdcall *PFN_SetRenderState)(void*, int, DWORD);
        typedef HRESULT (__stdcall *PFN_SetTextureStageState)(void*, int, int, DWORD);
        typedef HRESULT (__stdcall *PFN_DrawPrimitiveUP)(void*, DWORD, DWORD, const void*, DWORD);
        typedef HRESULT (__stdcall *PFN_SetVertexShader)(void*, DWORD);
        typedef HRESULT (__stdcall *PFN_GetVertexShader)(void*, DWORD*);
        PFN_SetRenderState      SetRenderState  = (PFN_SetRenderState)      (*(void**)(vt + D3D_DEV_SETRENDERSTATE));
        PFN_SetTextureStageState SetTextureStageState = (PFN_SetTextureStageState)(*(void**)(vt + D3D_DEV_SETTEXTURESTAGE));
        PFN_DrawPrimitiveUP     DrawPrimitiveUP = (PFN_DrawPrimitiveUP)     (*(void**)(vt + D3D_DEV_DRAWPRIMITIVEUP));
        PFN_SetVertexShader     SetVertexShader = (PFN_SetVertexShader)     (*(void**)(vt + D3D_DEV_SETVERTEXSHADER));
        PFN_GetVertexShader     GetVertexShader = (PFN_GetVertexShader)     (*(void**)(vt + D3D_DEV_GETVERTEXSHADER));
        void *dev = (void*)device;
        DWORD savedFVF = 0;

        /* Save the game's FVF, set our TL-vertex format. Without this the
         * DrawPrimitiveUP reads the game's last-frame FVF (e.g. textured
         * 3D vertex with extra fields) and walks past our 20-byte verts. */
        if (GetVertexShader && SetVertexShader) {
            GetVertexShader(dev, &savedFVF);
            SetVertexShader(dev, D3DFVF_TLVERTEX);
        }

        /* enable alpha blend (soft streaks) */
        SetRenderState(dev, D3DRS_ALPHABLENDENABLE, 1);
        SetRenderState(dev, D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
        SetRenderState(dev, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        SetRenderState(dev, D3DRS_FOGENABLE, 0);
        /* texture stage: use vertex diffuse for color (SELECTARG2 / diffuse) */
        SetTextureStageState(dev, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
        SetTextureStageState(dev, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        SetTextureStageState(dev, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        SetTextureStageState(dev, 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        /* draw n streak triangles */
        DrawPrimitiveUP(dev, 4 /*D3DPT_TRIANGLELIST*/, (DWORD)n, verts,
                        sizeof(Diamond_TLVertex));
        /* restore render states + FVF a prior draw may rely on */
        if (SetVertexShader) SetVertexShader(dev, savedFVF);
        SetRenderState(dev, D3DRS_ALPHABLENDENABLE, 0);
        SetRenderState(dev, D3DRS_FOGENABLE, 1);
    }
}

/*
 * Vortex whoosh sound — a looping BASS stream of the game's Whoosh.ogg that
 * plays for as long as the vortex cycle runs (frame 55 -> ~frame 185, = the
 * 100-frame active window + 30-frame tail). Uses the mod's already-loaded
 * real BASS.dll so it needs no new DirectSound/heap plumbing and shares the
 * game's music output safely (BASS plays the .mod music + this stream
 * simultaneously). Falls back silently if bass_real.dll/a pointer is absent.
 */
#define VORTEX_SND_STREAM  0
static HSTREAM g_vortex_snd = VORTEX_SND_STREAM;
#define WHOOSH_SND_PATH   "sounds\\whoosh.ogg"

static void vortex_sound_start(void) {
    if (g_vortex_snd != VORTEX_SND_STREAM) return;   /* already playing */
    if (!real_BASS_StreamCreateFile || !real_BASS_ChannelPlay) return;
    /* StreamCreateFile(mem=FALSE, file, off=0, len=0, flags=0) — NO loop flag:
     * the whoosh is a single one-shot sound played once. */
    g_vortex_snd = real_BASS_StreamCreateFile(FALSE, WHOOSH_SND_PATH, 0, 0, 0);
    if (g_vortex_snd != VORTEX_SND_STREAM) real_BASS_ChannelPlay(g_vortex_snd, TRUE);
}
static void vortex_sound_stop(void) {
    if (g_vortex_snd == VORTEX_SND_STREAM) return;
    if (real_BASS_ChannelStop) real_BASS_ChannelStop(g_vortex_snd);
    if (real_BASS_StreamFree) real_BASS_StreamFree(g_vortex_snd);
    g_vortex_snd = VORTEX_SND_STREAM;
}

/* Main vortex tick: called from the weasel cave each frame the weasel is
 * drawn. Starts the cycle at WEASEL_WHITE_START, advances streaks, and
 * draws them behind the trophy. Clears at WEASEL_WHITE_END. results = the
 * results-screen object (READ ONLY for frame + to find gfx via the weasel
 * sprite, same as the white-fade). */
__attribute__((used)) void diamond_vortex_tick(DWORD results) {
    int frame, i;
    DWORD app, sprite, gfx;
    if (!results) return;
    if (IsBadReadPtr((void*)(results + RESULT_FRAME), 4)) return;
    if (!diamond_first_earn(results)) {          /* replay -> no vortex */
        g_vortexResults = results;               /* keep session anchor fresh */
        return;                                  /* (no cycle to tear down) */
    }
    /* Fresh results session (pointer changed) -> any leftover cycle from a
     * previous results screen that was exited mid-cycle is torn down now:
     * stop the whoosh and reset the active flag so the new session starts
     * clean. (Without this, exiting the results screen before frame ~185
     * leaves the whoosh handle live and marks the cycle active, so the next
     * results screen would never restart it.) */
    if (results != g_vortexResults) {
        if (g_vortexActive) { g_vortexActive = 0; vortex_sound_stop(); }
        g_vortexResults = results;
    }
    frame = diamond_seq_frame(results);          /* frames since gold award */
    /* start the cycle a touch after the white-fade begins; active spawn
     * window is [START, START+VORTEX_FRAMES); the tail extends to
     * [START+VORTEX_FRAMES, START+VORTEX_FRAMES+VORTEX_TAIL) during which no
     * new streaks spawn but existing ones keep moving + fade away. */
    if (frame >= WEASEL_WHITE_START && frame < WEASEL_WHITE_START + (int)(VORTEX_FRAMES + VORTEX_TAIL)) {
        if (!g_vortexActive) {
            /* find gfx the same way the white-fade does */
            if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
            app = *(DWORD*)(results + RESULT_APP);
            if (!app || IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) return;
            sprite = *(DWORD*)(app + SPRITE_WEAEL_APP);
            if (sprite && IsBadReadPtr((void*)(sprite + SPRITE_GFX), 4)==0)
                gfx = *(DWORD*)(sprite + SPRITE_GFX);
            else gfx = 0;
            vortex_start_cycle(gfx, sprite);
        }
    } else {
        /* outside the window -> cycle ended */
        if (g_vortexActive) { g_vortexActive = 0; vortex_sound_stop(); }
        return;
    }
    g_vortexFrame = frame - WEASEL_WHITE_START;
    /* --- spawn streak (active window only; suppress during the tail) --- */
    for (i = 0; i < VORTEX_MAX; i++) {
        Diamond_VortexP *p = &g_vortex[i];
        if (!p->active) {
            if (g_vortexFrame >= (int)VORTEX_FRAMES) { continue; }  /* tail: no new spawn */
            /* random delay before a slot's first spawn (fade-in at random) */
            int wait = (int)(vortex_frand() * 24.0f);
            if (g_vortexFrame >= wait) {
                p->active = 1; p->born = 0;
                p->ax = vortex_frand() * 6.2832f;          /* random start angle (fixed: no curl) */
                p->ay = 0.0f;                              /* no angular motion — straight inward pull */
                /* start at a random radius on the outer ring */
                p->r = 34.0f + vortex_frand()*40.0f;
                p->vr = 4.4f + vortex_frand()*1.6f;      /* suck-in speed (uniform ~4.4-6.0) */
                p->alpha = 0;
            } else { continue; }
        } else {
            vortex_update_streak(p, g_vortexFrame);
        }
    }
    /* resolve gfx (for drawing) — reused from above if available */
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app || IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) return;
    sprite = *(DWORD*)(app + SPRITE_WEAEL_APP);
    if (!sprite || IsBadReadPtr((void*)(sprite + SPRITE_GFX), 4)) return;
    gfx = *(DWORD*)(sprite + SPRITE_GFX);
    if (!gfx) return;
    vortex_draw(gfx);
}



/* Genuine 5th-medal block: draws the diamond icon one "medal gap" AFTER gold.
 * Mirrors the native gold block exactly (frame gate -> threshold check ->
 * sprite draw), except the gate is (gold_gate + delay) and the threshold is
 * the mod's per-race SECRET. Called from the code cave right after the gold
 * draw re-emit. standalone -> no dependence on the current draw frame.
 */
__attribute__((used)) void diamond_render_after(DWORD results) {
    int frame, gold_gate, race, cs, thr;
    DWORD app;
    if (!results) return;
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app || !g_configLoaded) return;
    /* frame gate: only after gold + delay (mirrors native frame gate) */
    if (IsBadReadPtr((void*)(results + RESULT_GOLD), 4)) return;
    gold_gate = *(int*)(results + RESULT_GOLD);
    if (IsBadReadPtr((void*)(results + RESULT_FRAME), 4)) return;
    frame = *(int*)(results + RESULT_FRAME);
    if (frame <= gold_gate + g_diamondDelay) return;
    /* threshold check */
    race = get_race_index();
    if (race < 0 || race > 14) return;
    if (!g_hasSecret[race]) return;
    cs = get_player_time_cs(app);
    thr = g_secret_cs[race];
    if (!(cs > 0 && cs <= thr)) return;
    /* ATOMIC UNLOCK COMMIT: if this race hasn't been won yet, provision ALL
     * required writes (registry flag + PNG assets + future effects). If ANY
     * write fails, treat the diamond as never earned — no draw, no effects,
     * no persistence — so no partial state can exist. */
    if (!g_won[race]) {
        if (!diamond_provision_unlock(race)) {
            diag_logf("[diamond] unlock aborted for race %d (could not write everything)", race);
            return;   /* as if the diamond time was not met at all */
        }
        /* g_won[race] committed (to 1) inside diamond_provision_unlock on success.
         * First earn: play the native medal pop + spawn the star ring, exactly
         * like the game does for the other medals on their first award. */
        diamond_spawn_medal_effects(results, app);
    }
    /* Now that the diamond is genuinely unlocked (and its assets exist),
     * load + draw it. Load lazily (retries if a provision path raced). */
    if (!g_iconLoaded) diamond_load_icon_impl(app);
    if (!g_diamondSprite) return;   /* not loadable -> nothing to draw */
    /* draw the diamond over the golden weasel spot (0x208, 0x63) */
    __asm__ volatile(
        "movl %2, %%ecx\n\t"          /* ecx = g_diamondSprite */
        "pushl $0x63\n\t"             /* y */
        "pushl $0x208\n\t"            /* x */
        "call *%1\n\t"                /* 0x42C7C0(sprite,x,y) -- ret $8 */
        : : "r"(0), "r"(SPRITE_DRAW), "r"(g_diamondSprite)
        : "eax", "ecx", "edx", "memory"
    );
}

/* Trophy swap at the end of the white hold: at frame >= WEASEL_WHITE_TOTAL
 * (= 240), the golden weasel stops rendering and the diamond trophy appears in
 * its place at the trophy spot (0x208, 0x63), firing the reveal effects on the
 * first frame of the swap. Called from the weasel cave in place of the
 * unconditional gold draw. Returns 1 to SKIP the gold draw (diamond shown), 0
 * to draw gold as normal.
 */
__attribute__((used)) int diamond_trophy_swap(DWORD results) {
    int frame, race, cs, thr;
    DWORD app;
    if (!results) return 0;
    if (IsBadReadPtr((void*)(results + RESULT_FRAME), 4)) return 0;
    frame = diamond_seq_frame(results);          /* frames since gold award */
    if (frame < WEASEL_WHITE_TOTAL) return 0;    /* before hold end -> gold */
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return 0;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app || !g_configLoaded) return 0;
    /* threshold check: only swap if the diamond time was met for this race */
    race = get_race_index();
    if (race < 0 || race > 14) return 0;
    if (!g_hasSecret[race]) return 0;
    cs = get_player_time_cs(app);
    thr = g_secret_cs[race];
    if (!(cs > 0 && cs <= thr)) return 0;
    /* Atomic unlock commit on first reveal (mirrors diamond_render_after). */
    if (!g_won[race]) {
        if (!diamond_provision_unlock(race)) {
            diag_logf("[diamond] trophy swap unlock aborted for race %d", race);
            return 0;
        }
        diamond_spawn_medal_effects(results, app);   /* pop + star ring */
    }
    /* Ensure the diamond icon is loaded, then draw it at the trophy spot. */
    if (!g_iconLoaded) diamond_load_icon_impl(app);
    if (!g_diamondSprite) return 0;                  /* not loadable -> gold stays */
    __asm__ volatile(
        "movl %2, %%ecx\n\t"          /* ecx = g_diamondSprite */
        "pushl $0x63\n\t"             /* y */
        "pushl $0x208\n\t"            /* x */
        "call *%1\n\t"                /* 0x42C7C0(sprite,x,y) -- ret $8 */
        : : "r"(0), "r"(SPRITE_DRAW), "r"(g_diamondSprite)
        : "eax", "ecx", "edx", "memory"
    );
    return 1;                                          /* skip the gold draw */
}

__attribute__((used)) void diamond_load_icon_impl(DWORD app) {
    DWORD mgr, vt, load;
    if (g_iconLoaded) return;
    if (!app || !g_configLoaded) return;
    /* The diamond PNG is loaded through the game's NORMAL file path (it was
     * written to Textures\ on first unlock). No manual texture building. */
    mgr = *(DWORD*)(app + APP_MGR);
    if (!mgr) return;
    if (IsBadReadPtr((void*)mgr, 4)) return;
    vt = *(DWORD*)mgr;
    if (!vt) return;
    if (IsBadReadPtr((void*)(vt + 0x58), 4)) return;
    load = *(DWORD*)(vt + 0x58);
    if (!load) return;
    /* __thiscall: ecx=mgr, push <str> FIRST, then push &slot, call [vt+0x58].
         * Native call site (0x42a2f8): push $0x4d31c0 (str); push %edx (&slot);
         * call *0x58(%eax). The loader is __stdcall `ret $8` (cleans its own two
         * args) — its caller does NOT `add esp,8` afterwards. */
        __asm__ volatile(
                    "pushl %3\n\t"        /* g_iconFile (str) — pushed FIRST */
                    "pushl %2\n\t"        /* &g_diamondSprite (slot) */
                    "movl %0, %%ecx\n\t"  /* mgr */
                    "call *%1\n\t"        /* load — ret $8, no add esp */
                    : : "r"(mgr), "r"(load), "r"(&g_diamondSprite), "r"(g_iconFile)
                    : "eax", "ecx", "edx", "memory"
                );
    /* Only mark the icon loaded if it actually resolved. If the PNG wasn't
     * written yet (first unlock provisions it just before this load), leave
     * g_iconLoaded=0 so a later frame retries. */
    if (g_diamondSprite) {
        g_iconLoaded = 1;
        diag_logf("[diamond] icon loaded: %s -> %08X", g_iconFile, g_diamondSprite);
    }
}
__attribute__((used)) void diamond_load_mini_icon_impl(DWORD app) {
    DWORD mgr, vt, load;
    if (g_miniIconLoaded) return;
    if (!app || !g_configLoaded) return;
    mgr = *(DWORD*)(app + APP_MGR);
    if (!mgr) return;
    if (IsBadReadPtr((void*)mgr, 4)) return;
    vt = *(DWORD*)mgr;
    if (!vt) return;
    if (IsBadReadPtr((void*)(vt + 0x58), 4)) return;
    load = *(DWORD*)(vt + 0x58);
    if (!load) return;
    __asm__ volatile(
        "pushl %3\n\t"        /* g_miniIconFile (str) — pushed FIRST */
        "pushl %2\n\t"        /* &g_diamondMiniSprite (slot) */
        "movl %0, %%ecx\n\t"  /* mgr */
        "call *%1\n\t"        /* load — ret $8, no add esp */
        : : "r"(mgr), "r"(load), "r"(&g_diamondMiniSprite), "r"(g_miniIconFile)
        : "eax", "ecx", "edx", "memory"
    );
    if (g_diamondMiniSprite) {
        g_miniIconLoaded = 1;
        diag_logf("[diamond] mini icon loaded: %s -> %08X", g_miniIconFile, g_diamondMiniSprite);
    }
}

/* TT-menu: append a diamond medal entry to the standings list.
 * Called from the TT cave. standings = the standings screen object (esi),
 * race = the loop counter edi, which is 1-INDEXED into race (edi=0 shows
 * race 1 BEGINNER; edi=13 shows race 14 IMPOSSIBLE; edi=14 indexes the
 * free App tail 0x8F8 = phantom all-zero flags, so it must be skipped).
 * 0x44abf0 is __stdcall(ecx=this, name, sprite) with ret 8.
 */
__attribute__((used)) void diamond_tt_append(DWORD standings, int race) {
    static char namebuf[16];
    int r = race + 1;                 /* edi is 1-indexed into race */
    if (r < 0 || r > 14) return;      /* guard: edi=14 -> r=15 (phantom tail) */
    if (!g_won[r]) return;
    /* lazy-load mini icon (manager valid during TT menu) */
    if (!g_miniIconLoaded) {
        DWORD app = get_app();
        if (app) diamond_load_mini_icon_impl(app);
    }
    if (!g_diamondMiniSprite) return;
    /* format a distinct name "%dD" for the diamond entry (must differ from
     * the weasel's "%d" so 0x44abf0 creates a NEW list entry) */
    sprintf(namebuf, g_fmtDiamond, r);
    /* 0x44abf0(ecx=standings, arg1=name, arg2=sprite) __stdcall ret 8 */
    __asm__ volatile(
        "pushl %1\n\t"        /* sprite (arg2) */
        "pushl %0\n\t"        /* name (arg1) */
        "movl %2, %%ecx\n\t"  /* standings */
        "call *%3\n\t"
        : : "r"(namebuf), "r"(g_diamondMiniSprite), "r"(standings), "r"(ABF0_APPEND)
        : "eax", "ecx", "edx", "memory"
    );
    diag_logf("[diamond] TT diamond appended for race %d", r);
}

/* ================================================================
 * Present-hook reveal driver
 * ================================================================
 * The white-out + vortex + diamond trophy-swap used to run from an INLINE
 * cave at 0x44E139 (the game's own golden-weasel draw call site), which
 * re-emitted the gold draw (call 0x42c7c0) and re-entered the sprite loader
 * from inside the game's Draw loop. That re-entrancy crashed on real Windows
 * (Wine tolerates it). The whole reveal now runs from the Graphics_PresentOrEnd
 * hook (0x455A90), which fires ONCE PER FRAME at a safe boundary — after all
 * game logic + rendering, immediately before the backbuffer is presented. The
 * game's own weasel draw at 0x44E139 is left 100% original; we draw the
 * white-out weasel and the diamond ON TOP of it from present-time, so the
 * layering is: gold (native) -> white weasel (55..240) -> diamond (240+).
 */

/* Locate the active results-screen object (scene+0x8B8 results list) or 0. */
static DWORD find_results_object(void) {
    DWORD app, scene, list, items, results, vt;
    int count;
    app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_BOARD), 4)) return 0;
    scene = *(DWORD*)(app + APP_BOARD);
    if (!scene) return 0;
    if (IsBadReadPtr((void*)(scene + SCENE_RESULTS_LIST), 4)) return 0;
    list = scene + SCENE_RESULTS_LIST;
    if (IsBadReadPtr((void*)(list + SCENE_LIST_COUNT), 4)) return 0;
    count = *(int*)(list + SCENE_LIST_COUNT);
    if (count <= 0) return 0;
    if (IsBadReadPtr((void*)(list + SCENE_LIST_ITEMS), 4)) return 0;
    items = *(DWORD*)(list + SCENE_LIST_ITEMS);
    if (!items || IsBadReadPtr((void*)items, 4)) return 0;
    results = *(DWORD*)items;
    if (!results || IsBadReadPtr((void*)results, 4)) return 0;
    vt = *(DWORD*)results;
    if (vt != RESULTS_VTABLE && vt != 0x4D6C00) return 0;
    return results;
}

/* Scoped white-out: tint the weasel sprite white over the trophy. Instead of
 * tinting via the cave (set mult -> game draws -> clear), we set the color
 * multiplier, draw the weasel sprite OURSELVES through the game's draw fn
 * (0x42c7c0, tinted white by the multiplier), then clear it. Because this runs
 * at present-time (not mid-draw), the multiplier only affects OUR draw, not the
 * whole medal row. The game's native gold weasel stays underneath. */
static void whiteout_tick(DWORD results) {
    int frame;
    DWORD app, sprite, gfx;
    float m, *sc;
    if (!results) return;
    if (IsBadReadPtr((void*)(results + RESULT_FRAME), 4)) return;
    if (!diamond_first_earn(results)) return;   /* replay -> no white-out */
    frame = diamond_seq_frame(results);
    if (frame <= WEASEL_WHITE_START || frame >= WEASEL_WHITE_TOTAL) return;
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app || IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) return;
    sprite = *(DWORD*)(app + SPRITE_WEAEL_APP);
    if (!sprite || IsBadReadPtr((void*)(sprite + SPRITE_GFX), 4)) return;
    gfx = *(DWORD*)(sprite + SPRITE_GFX);
    if (!gfx || IsBadReadPtr((void*)(gfx + GFX_MULT_R), 4)) return;
    if (frame >= WEASEL_WHITE_END) m = WEASEL_WHITE_MULT;
    else m = 1.0f + (WEASEL_WHITE_MULT - 1.0f) *
            ((float)(frame - WEASEL_WHITE_START) / (float)(WEASEL_WHITE_END - WEASEL_WHITE_START));
    *(volatile unsigned char*)(gfx + GFX_MULT_ENABLE) = 1;
    sc = (float*)(gfx + GFX_MULT_R);
    sc[0] = m; sc[1] = m; sc[2] = m; sc[3] = 1.0f;
    /* draw the weasel sprite tinted white over the trophy spot (0x208, 0x63) */
    __asm__ volatile(
        "movl %1, %%ecx\n\t"          /* ecx = sprite */
        "pushl $0x63\n\t"             /* y */
        "pushl $0x208\n\t"            /* x */
        "call *%0\n\t"                /* 0x42C7C0(sprite,x,y) -- ret $8 */
        : : "r"((void*)SPRITE_DRAW), "r"(sprite)
        : "eax", "ecx", "edx", "memory"
    );
    /* restore identity multiplier so the rest of the frame is unaffected */
    *(volatile unsigned char*)(gfx + GFX_MULT_ENABLE) = 0;
    sc[0] = sc[1] = sc[2] = sc[3] = 1.0f;
}

/* Per-frame reveal driver, called from the present hook (ecx=gfx). Safe at the
 * frame boundary: no re-entrancy into the game's draw loop. Derives everything
 * itself (app -> scene -> results), so it takes no arguments. */
__attribute__((used)) void diamond_present_tick(void) {
    DWORD results;
    /* EARLY-EXIT GATE (warp's g_whiteAlpha pattern): this hook fires EVERY
     * frame, including during the title screen's Level4-Trapdoor2 background
     * load where app+0x178 (board) / scene+0x8B8 (results list) point at a
     * half-built board. IsBadReadPtr is NOT enough protection against those
     * live-but-half-built objects (the pointer is valid but its vtable is
     * garbage), so refuse to read ANY game state until a results screen has
     * genuinely armed us via the skip-latch cave. */
    if (!g_revealArmed) return;
    results = find_results_object();
    if (!results) {
        /* results screen gone -> disarm so the NEXT loading screen is safe,
         * and tear down any leftover vortex cycle */
        g_revealArmed = 0;
        if (g_vortexActive) { g_vortexActive = 0; vortex_sound_stop(); }
        return;
    }
    diamond_vortex_tick(results);   /* advances + draws vortex (raw D3D) */
    whiteout_tick(results);         /* scoped white-out weasel draw */
    diamond_trophy_swap(results);   /* commit unlock + draw diamond at 240+ */
}

/* ================================================================
 * Patch helpers + code caves
 * ================================================================ */
static void write_jmp(unsigned char *at, DWORD target) {
    at[0] = 0xE9;
    *(DWORD*)(at + 1) = target - (DWORD)at - 5;
}
static void patch_bytes(void *addr, const void *data, DWORD size) {
    DWORD old;
    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, data, size);
    VirtualProtect(addr, size, old, &old);
    FlushInstructionCache(GetCurrentProcess(), addr, size);
}

/* Cave A: icon load at 0x42A304 (covers 9 bytes through 0x42A30C).
 *   original: call [eax+0x58] ; mov ecx,[esi+0x22C]
 *   cave: call [eax+0x58]; mov ecx,[esi+0x22C]; pushad; push esi;
 *         call diamond_load_icon_impl; add esp,4; popad; jmp 0x42A30D
 *   CRITICAL ordering: the re-emitted `call [eax+0x58]` and `mov ecx,[esi+0x22C]`
 *   MUST run BEFORE `pushad`. The loader is __stdcall `ret $8` and reads the game's
 *   two args (&slot, str) off the stack at this depth — if it runs after pushad,
 *   those args are buried under 32 bytes of saved registers, so it reads the pushed
 *   register values as args (garbage) and ret $8 pops the pushad saves -> stack &
 *   register corruption -> crash at startup icon load (Initialize(25), MODULE: @).
 *   And `mov ecx,[esi+0x22C]` must precede pushad so popad restores ecx=mgr for the
 *   continuation at 0x42A30D (which reads [ecx] again).
 */
/* install_icon_cave is DISABLED (no-op).
 *
 * Root-cause finding for the real-Windows startup crash (Initialize(25),
 * MODULE: @, far-out heap fault address):
 *
 *   The icon cave at 0x42A304 was the ONLY hook that fired during startup —
 *   the other six (TT-menu 0x42F927, weasel-white 0x44E139, skip-latch
 *   0x44CBAA, pause 0x4130C9/0x40B40F, medal-award) only run on the
 *   results screen / TT menu / pause, none of which execute at Initialize(25).
 *
 *   The cave's job was to PRE-WARM the diamond icon at startup by calling the
 *   game's sprite loader ([vt+0x58]) with "diamondweasel.png" — a file that
 *   does NOT exist until the player first unlocks a diamond. It also fired
 *   re-entrantly inside the game's own icon-load function during audio init,
 *   which is an unsafe time to call back into the sprite system.
 *
 *   The pre-warm is REDUNDANT: the icon is loaded lazily, on demand, right
 *   before it is drawn, from diamond_render_after() (line ~1298) and
 *   diamond_trophy_swap() (line ~1344), both of which call
 *   diamond_load_icon_impl() once the diamond assets actually exist (they are
 *   provisioned by diamond_provision_unlock() before the first draw). So the
 *   startup pre-warm adds no capability — only a startup crash vector.
 *
 *   Removing it keeps the icon cave ORIGINAL code intact (we patch nothing at
 *   0x42A304), leaves the single 9-byte `call*0x58` + `mov ecx,[esi+0x22C]`
 *   untouched, and offloads all icon loading to the on-demand path.
 */
static void install_icon_cave(void) {
    /* no-op: icon loads lazily on first draw. Do NOT patch 0x42A304. */
    diag_log("[diamond] icon cave DISABLED (icon loads lazily on first draw)");
}

/* Cave E: block the results-screen click/keypress skip when the diamond was
 * achieved (so the player sees the frame-240 reveal). We stop the SKIP LATCH
 * from being set rather than hacking the multiplier — the latch (results+0x25)
 * is the single source of truth for skip; if it never gets set, the frame
 * counter simply advances at 1x/frame and the reveal plays out.
 *
 * Hook at 0x44CBAA: `movb $1,0x25(%esi)` (set latch, 4 bytes) + the first
 * byte of the next instruction (`cmp %bl,0x24(%esi)` = 38 5e 24) = 5 bytes,
 * ret 0x44CBB1. The cave:
 *   push esi; call diamond_block_skip; add esp,4; test eax,eax
 *   jnz block_skip          ; block == achieved -> do NOT set the latch
 *   movb $1,0x25(esi)       ; re-emit original latch-set (not achieved)
 * block_skip:
 *   cmp %bl,0x24(esi)       ; re-emit the borrowed cmp byte
 *   jmp 0x44CBB1
 * esi = results object is preserved (helper is callee-saved for esi/edi/ebx).
 */
static void install_skip_cave(void) {
    DWORD patchAddr = EXE_BASE + (SKIP_LATCH_HOOK - EXE_BASE);
    DWORD retAddr   = EXE_BASE + (SKIP_LATCH_RET - EXE_BASE);
    g_skipCave = (unsigned char*)VirtualAlloc(NULL, 48, MEM_COMMIT|MEM_RESERVE,
                                              PAGE_EXECUTE_READWRITE);
    if (!g_skipCave) return;
    unsigned char *p = g_skipCave;
    /* push esi (arg: results) */
    p[0]=0x56; p+=1;
    /* call diamond_block_skip (cdecl) */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_block_skip-(DWORD)(p+5); p+=5;
    /* add esp,4 (cdecl caller cleans) */
    p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;
    /* test eax,eax */
    p[0]=0x85; p[1]=0xC0; p+=2;
    /* jnz block_skip (rel=4: skip the 4-byte movb) */
    p[0]=0x75; p[1]=0x04; p+=2;
    /* movb $1,0x25(esi) — original latch-set (only when NOT blocked) */
    p[0]=0xC6; p[1]=0x46; p[2]=0x25; p[3]=0x01; p+=4;
    /* block_skip: cmp %bl,0x24(esi) — re-emit the borrowed byte's instr */
    p[0]=0x38; p[1]=0x5E; p[2]=0x24; p+=3;
    /* jmp back */
    write_jmp(p, retAddr); p+=5;
    unsigned char patch[5];
    memset(patch, 0x90, 5);
    write_jmp(patch, (DWORD)g_skipCave);
    patch_bytes((void*)patchAddr, patch, 5);
    diag_log("[diamond] skip-latch cave installed at 0x44CBAA");
}

/* Cave G: ARM the reveal from the results screen's per-frame update (vtable
 * slot 1). Both results objects — base "Click to continue" (vtable 0x4D6CFC,
 * update 0x44CB90) and final medal screen (vtable 0x4D6C00, update 0x44B860)
 * — run a per-frame update; arming from BOTH ensures the reveal arms whichever
 * phase is live. This is the ONLY thing that lets diamond_present_tick read
 * game state: without it the tick early-returns, so a half-built board (the
 * title screen's Level4-Trapdoor2 preview load) can never be dereferenced
 * (crash 5, CRASH_ADDRESS 0000:001AEED8 at 2s). These update fns only run
 * while a results screen is genuinely live, never during a loading screen, so
 * the caves are installed synchronously (no deferral needed). */
static void install_arm_cave(void) {
    unsigned char *c1, *c2;
    g_armCave = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT|MEM_RESERVE,
                                              PAGE_EXECUTE_READWRITE);
    if (!g_armCave) return;
    c1 = g_armCave;        /* first cave (0x44CB90) */
    c2 = g_armCave + 32;   /* second cave (0x44B860) */

    /* cave 1: 0x44CB90 — push ebx;push esi;mov esi,ecx;mov eax,[esi+0x1c] (7B) */
    {
        unsigned char *p = c1;
        p[0]=0x51; p+=1;                                   /* push ecx (this) */
        p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_arm_reveal-(DWORD)(p+5); p+=5;
        p[0]=0x59; p+=1;                                   /* pop ecx */
        p[0]=0x53; p+=1;                                   /* push ebx (re-emit) */
        p[0]=0x56; p+=1;                                   /* push esi (re-emit) */
        p[0]=0x8B; p[1]=0xF1; p+=2;                        /* mov esi,ecx */
        p[0]=0x8B; p[1]=0x46; p[2]=0x1C; p+=3;             /* mov eax,[esi+0x1c] */
        write_jmp(p, EXE_BASE + (RESULT_UPDATE_1_RET - EXE_BASE)); p+=5;
        unsigned char patch[7];
        memset(patch, 0x90, 7);
        write_jmp(patch, (DWORD)c1);
        patch_bytes((void*)(EXE_BASE + (RESULT_UPDATE_1 - EXE_BASE)), patch, 7);
    }

    /* cave 2: 0x44B860 — mov eax,[ecx+0x10];mov edx,[ecx+0x14] (6B) */
    {
        unsigned char *p = c2;
        p[0]=0x51; p+=1;                                   /* push ecx (this) */
        p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_arm_reveal-(DWORD)(p+5); p+=5;
        p[0]=0x59; p+=1;                                   /* pop ecx */
        p[0]=0x8B; p[1]=0x41; p[2]=0x10; p+=3;             /* mov eax,[ecx+0x10] */
        p[0]=0x8B; p[1]=0x51; p[2]=0x14; p+=3;             /* mov edx,[ecx+0x14] */
        write_jmp(p, EXE_BASE + (RESULT_UPDATE_2_RET - EXE_BASE)); p+=5;
        unsigned char patch[6];
        memset(patch, 0x90, 6);
        write_jmp(patch, (DWORD)c2);
        patch_bytes((void*)(EXE_BASE + (RESULT_UPDATE_2 - EXE_BASE)), patch, 6);
    }

    diag_log("[diamond] reveal-arm cave installed (results update, both vtables)");
}

/* Cave F: block the PAUSE menu while the diamond reveal is pending (goal
 * touch -> frame-240 trophy swap). The game's OWN DirectInput-ESC pause path
 * (Scene_Update, Path 1) is already suppressed at goal because the game sets
 * scene+0x880=1 there; that check runs at 0x419d50 and skips the pause call.
 * But the OTHER two pause entry points do NOT consult scene+0x880, so they
 * can still open the pause menu mid-reveal and interrupt it:
 *
 *   Path 2 (right-click): 0x4130c9 `call 0x40a920` (5B) — vtable[5] thunk.
 *   Path 3 (Win32 ESC):   0x40b40f `jmp  0x40a920` (5B) — vtable[8] thunk.
 *
 * Both funnel into Scene_CreateGameOverMenu (0x40a920, which has an SEH
 * prologue, so we patch the CALL SITES, not the function entry). Each cave
 * calls diamond_pause_blocked(scene) and, when it returns nonzero (diamond
 * earned + reveal not yet passed + results object present), skips the pause
 * while keeping the stack balanced for the surrounding thunk.
 *
 * ecx = scene object at both call sites (preserved to esi, callee-saved).
 * Path 2 cave ends with `add esp,4; jmp ret` (the pushed $1 arg to the
 * skipped pause call is manually popped; the thunk's own ret $0xC rebalances
 * the rest). Path 3 cave ends with `jmp 0x40b414` whose `ret $4` pops both
 * the return address and the handler's arg (identical to the non-ESC path).
 */
static void install_pause_caves(void) {
    DWORD rcAddr = EXE_BASE + (PAUSE_RCLICK_HOOK - EXE_BASE);
    DWORD rcRet  = EXE_BASE + (PAUSE_RCLICK_RET  - EXE_BASE);
    DWORD esAddr = EXE_BASE + (PAUSE_ESCMSG_HOOK - EXE_BASE);
    DWORD esRet  = EXE_BASE + (PAUSE_ESCMSG_RET  - EXE_BASE);
    DWORD menu   = EXE_BASE + (0x40A920 - EXE_BASE);
    unsigned char *p;
    unsigned char patch[5];

    /* -- Path 2 (right-click) cave -- */
    g_pauseCave = (unsigned char*)VirtualAlloc(NULL, 160, MEM_COMMIT|MEM_RESERVE,
                                               PAGE_EXECUTE_READWRITE);
    if (g_pauseCave) {
        p = g_pauseCave;
        p[0]=0x8B; p[1]=0xF1; p+=2;                 /* mov esi,ecx (save scene) */
        p[0]=0x56; p+=1;                            /* push esi (arg: scene) */
        p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_pause_blocked-(DWORD)(p+5); p+=5;
        p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;      /* add esp,4 (pop arg) */
        p[0]=0x85; p[1]=0xC0; p+=2;                 /* test eax,eax */
        p[0]=0x8B; p[1]=0xCE; p+=2;                 /* mov ecx,esi (restore scene) */
        p[0]=0x75; p[1]=0x0A; p+=2;                 /* jnz blocked (rel=10) */
        p[0]=0xE8; *(DWORD*)(p+1)=menu-(DWORD)(p+5); p+=5;   /* call 0x40a920 */
        p[0]=0xE9; *(DWORD*)(p+1)=rcRet-(DWORD)(p+5); p+=5;  /* jmp 0x4130ce */
        /* blocked: */
        p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;      /* add esp,4 (pop $1 arg) */
        p[0]=0xE9; *(DWORD*)(p+1)=rcRet-(DWORD)(p+5); p+=5;  /* jmp 0x4130ce */
        memset(patch, 0x90, 5);
        write_jmp(patch, (DWORD)g_pauseCave);
        patch_bytes((void*)rcAddr, patch, 5);
        diag_log("[diamond] pause-block cave installed at 0x4130C9 (right-click)");
    }

    /* -- Path 3 (Win32 ESC message) cave -- (single 160B buf holds both) */
    {
        unsigned char *c2 = g_pauseCave + 80;       /* second half of the buffer */
        p = c2;
        p[0]=0x8B; p[1]=0xF1; p+=2;                 /* mov esi,ecx */
        p[0]=0x56; p+=1;                            /* push esi */
        p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_pause_blocked-(DWORD)(p+5); p+=5;
        p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;      /* add esp,4 */
        p[0]=0x85; p[1]=0xC0; p+=2;                 /* test eax,eax */
        p[0]=0x8B; p[1]=0xCE; p+=2;                 /* mov ecx,esi */
        p[0]=0x75; p[1]=0x05; p+=2;                 /* jnz blocked (rel=5) */
        p[0]=0xE9; *(DWORD*)(p+1)=menu-(DWORD)(p+5); p+=5;  /* jmp 0x40a920 */
        /* blocked: */
        p[0]=0xE9; *(DWORD*)(p+1)=esRet-(DWORD)(p+5); p+=5;  /* jmp 0x40b414 (ret $4) */
        memset(patch, 0x90, 5);
        write_jmp(patch, (DWORD)c2);
        patch_bytes((void*)esAddr, patch, 5);
        diag_log("[diamond] pause-block cave installed at 0x40B40F (Win32 ESC)");
    }
}

/* Cave B: results-screen DIAMOND draw as a genuine 5th medal block, one
 * medal-gap AFTER gold. Hook at 0x44EFD2 (the gold `call 0x42c7c0`, 5 bytes).
 *   cave:     call 0x42c7c0            ; re-emit gold draw (existing stack args)
 *             pushad
 *             push esi                 ; results object (diamond_render_after arg1)
 *             call diamond_render_after ; full 5th-medal block (frame gate +
 *                                       ;  secret check + draw, its own delay)
 *             add esp,4
 *             popad
 *             jmp 0x44EFD7
 * The diamond appears (gold_gate + g_diamondDelay) frames after gold, i.e.
 * one medal gap later, at the golden weasel's spot (0x208, 0x63).
 */
/* Cave B (DISABLED 2026-08-12): the diamond reveal now fires at the frame-240
 * trophy swap (install_weasel_cave -> diamond_trophy_swap). This cave used to
 * draw the diamond as a 5th medal one gold-gap after gold; kept only for
 * reference/re-support. Not installed (see install_hooks). */
static void install_disp_cave(void) __attribute__((unused));
static void install_disp_cave(void) {
    DWORD patchAddr = EXE_BASE + (GOLD_DRAW_HOOK - EXE_BASE);
    DWORD retAddr = patchAddr + 5;   /* 0x44EFD7 */
    g_dispCave = (unsigned char*)VirtualAlloc(NULL, 160, MEM_COMMIT|MEM_RESERVE,
                                              PAGE_EXECUTE_READWRITE);
    if (!g_dispCave) return;
    unsigned char *p = g_dispCave;
    /* call 0x42c7c0 (re-emit gold draw) — E8 rel32 */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(SPRITE_DRAW)-(DWORD)(p+5); p+=5;
    /* pushad */
    p[0]=0x60; p+=1;
    /* push esi (results object) — 56 */
    p[0]=0x56; p+=1;
    /* call diamond_render_after */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_render_after-(DWORD)(p+5); p+=5;
    /* add esp,4 */
    p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;
    /* popad */
    p[0]=0x61; p+=1;
    /* jmp retAddr */
    write_jmp(p, retAddr); p+=5;
    unsigned char patch[5];
    memset(patch, 0x90, 5);
    write_jmp(patch, (DWORD)g_dispCave);
    patch_bytes((void*)patchAddr, patch, 5);
    diag_log("[diamond] diamond 5th-medal cave installed at 0x44EFD2");
}

/* Cave C: TT-menu (standings) diamond mini-icon after golden weasel.
 * Hook at 0x42F927 (call 0x44abf0, the golden-weasel append, 5 bytes).
 *   cave:     mov ecx,esi          ; re-emit weasel append (stack has name+sprite)
 *             call 0x44abf0
 *             pushad
 *             push esi             ; standings (diamond_tt_append arg1)
 *             push edi             ; race (diamond_tt_append arg2)
 *             call diamond_tt_append
 *             add esp,8
 *             popad
 *             jmp 0x42F92C         ; inc edi (original next instruction)
 */
static void install_tt_cave(void) {
    /* Gate the TT-menu cave on the player having EARNED at least one diamond.
     *
     * If no diamond has ever been unlocked (fresh profile: g_anyDiamond==0),
     * we install NOTHING at 0x42F927 — the game's own golden-weasel append
     * runs untouched, the mini-icon code is never entered, and the TT menu
     * cannot crash from this cave. (The 0x42F927 crash only ever reproduced
     * on real Windows, not on the Wine test-harness, which is why it slipped
     * past the crash test.)
     *
     * Once the player earns their first diamond (g_anyDiamond becomes 1, set
     * in diamond_provision_unlock), the cave activates on the NEXT launch so
     * the mini diamond can show in the standings for exactly the races the
     * player has unlocked (diamond_tt_append still guards each race on
     * g_won[r]). Diamond assets are written at the moment they're first
     * earned, so the mini-icon file only loads when it actually exists.
     */
    if (!g_anyDiamond) {
        diag_log("[diamond] TT-menu cave NOT installed (no diamonds earned yet)");
        return;
    }
    DWORD patchAddr = EXE_BASE + (TT_WEASEL_APPEND - EXE_BASE);
    DWORD retAddr = patchAddr + 5;   /* 0x42F92C */
    g_ttCave = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE,
                                            PAGE_EXECUTE_READWRITE);
    if (!g_ttCave) return;
    unsigned char *p = g_ttCave;
    /* mov ecx, esi */
    p[0]=0x8B; p[1]=0xCE; p+=2;
    /* call 0x44abf0 (re-emit weasel append) */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(ABF0_APPEND)-(DWORD)(p+5); p+=5;
    /* pushad */
    p[0]=0x60; p+=1;
    /* push esi */
    p[0]=0x56; p+=1;
    /* push edi */
    p[0]=0x57; p+=1;
    /* call diamond_tt_append */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_tt_append-(DWORD)(p+5); p+=5;
    /* add esp,8 */
    p[0]=0x83; p[1]=0xC4; p[2]=0x08; p+=3;
    /* popad */
    p[0]=0x61; p+=1;
    /* jmp retAddr */
    write_jmp(p, retAddr); p+=5;
    unsigned char patch[5];
    memset(patch, 0x90, 5);
    write_jmp(patch, (DWORD)g_ttCave);
    patch_bytes((void*)patchAddr, patch, 5);
    diag_log("[diamond] TT-menu cave installed (diamond(s) earned)");
}

/* Cave D (DISABLED 2026-08-12): the golden-weasel white-fade + suction vortex
 * USED to run from an inline cave at 0x44E139 (the game's own weasel draw call
 * site), re-emitting `call 0x42c7c0` and re-entering the sprite loader from
 * inside the game's Draw loop. That re-entrancy crashed on real Windows with
 * CRASH_ADDRESS 0000:00000000 (CURRENTOPERATION: Draw) — the third crash of
 * this class (startup icon cave, TT-menu cave, now this). Wine tolerates it,
 * which is why hbtestd never reproduced it.
 *
 * The white-out + vortex + diamond trophy-swap now runs from the
 * Graphics_PresentOrEnd hook (install_present_cave -> diamond_present_tick),
 * which fires once per frame at a safe boundary. 0x44E139 is left 100%
 * original (the game draws the gold weasel natively; our white-out weasel and
 * diamond draw OVER it at present time). */
static void install_weasel_cave(void) {
    diag_log("[diamond] weasel white-fade cave DISABLED (reveal moved to present hook)");
}

/* Present-hook cave: Graphics_PresentOrEnd (0x455A90), __thiscall(ecx=gfx).
 * Original prologue (7 bytes): 8A 44 24 04  MOV AL,[ESP+4]
 *                              83 EC 20     SUB ESP,0x20
 * Detour: save ecx/edx (the game reads ecx=gfx at 0x455A9A `mov esi,ecx` right
 * after this prologue), call diamond_present_tick (cdecl, no args), restore
 * ecx/edx, then re-emit the 7-byte prologue and jump back to 0x455A97. */
static void install_present_cave(void) {
    DWORD patchAddr = EXE_BASE + (PRESENT_HOOK - EXE_BASE);
    DWORD retAddr   = EXE_BASE + (PRESENT_RET - EXE_BASE);
    unsigned char *p;
    unsigned char patch[7];
    g_presentCave = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT|MEM_RESERVE,
                                                 PAGE_EXECUTE_READWRITE);
    if (!g_presentCave) return;
    p = g_presentCave;
    /* save ecx (gfx) + edx across the helper call */
    p[0]=0x51; p+=1;                                  /* push ecx */
    p[0]=0x52; p+=1;                                  /* push edx */
    /* call diamond_present_tick (cdecl, no args) */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_present_tick-(DWORD)(p+5); p+=5;
    /* restore edx, ecx */
    p[0]=0x5A; p+=1;                                  /* pop edx */
    p[0]=0x59; p+=1;                                  /* pop ecx */
    /* re-emit original 7-byte prologue: MOV AL,[ESP+4]; SUB ESP,0x20 */
    p[0]=0x8A; p[1]=0x44; p[2]=0x24; p[3]=0x04; p+=4;
    p[0]=0x83; p[1]=0xEC; p[2]=0x20; p+=3;
    /* jump back */
    write_jmp(p, retAddr); p+=5;
    /* 7-byte patch at 0x455A90: JMP rel32 (5 bytes) + 2 NOPs */
    memset(patch, 0x90, 7);
    write_jmp(patch, (DWORD)g_presentCave);
    patch_bytes((void*)patchAddr, patch, 7);
    diag_log("[diamond] present-hook reveal cave installed at 0x455A90");
}

/* ================================================================
 * Install
 * ================================================================ */
static void install_hooks(void) {
    install_icon_cave();
    /* install_disp_cave() DISABLED: the diamond reveal now happens at the
     * frame-240 trophy swap (diamond_present_tick via the present hook),
     * replacing the old gold-gap 5th-medal reveal. Keeping Cave B would
     * draw+pop the diamond too early at ~frame 165 AND again at 240. */
    install_tt_cave();
    install_weasel_cave();   /* no-op — reveal moved to present hook */
    /* install_present_cave() is DEFERRED to a background thread (see
     * present_install_thread) that installs it once the game is past the
     * loading screen. Installing it here (synchronously, in DllMain) made it
     * live during the boot loading screen, where diamond_present_tick ran on
     * an uninitialized board -> crash on real Windows (Initialize(26)). The
     * warp mod installs its present hook the same way (2s-late thread). */
    install_skip_cave();
    install_arm_cave();      /* arm the present reveal from the results update */
    install_pause_caves();
    diag_log("[diamond] hooks installed");
}

/* ================================================================
 * Deferred present-hook install
 * ================================================================
 * The Graphics_PresentOrEnd hook (0x455A90) fires EVERY FRAME, including
 * during the boot loading screen (CURRENTOBJECT "LoadingScreen Gadget",
 * Initialize(26)). Installing it synchronously in DllMain made it live
 * during that screen, where diamond_present_tick ran against a half-built
 * board and crashed on real Windows (CRASH_ADDRESS 0006:00000000). The
 * proven-working warp mod installs the SAME hook 2 seconds late from a
 * background thread — after the loading screen is done. We do the same. */
static DWORD WINAPI present_install_thread(LPVOID param) {
    (void)param;
    Sleep(2000);                 /* let the loading screen + boot finish */
    install_present_cave();      /* hook live only once the game is past boot */
    return 0;
}

/* ================================================================
 * DllMain
 * ================================================================ */
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
        load_real_bass();
        get_own_dir(g_logPath, sizeof(g_logPath));
        snprintf(g_logPath,    sizeof(g_logPath), "%s\\diamond_weasel_mod.log", g_logPath);
        diag_log("=== DIAMOND WEASEL MOD LOADED ===");
        init_thresholds();
        load_unlocks();
        install_hooks();
        CreateThread(NULL, 0, present_install_thread, NULL, 0, NULL);
    }
    return TRUE;
}
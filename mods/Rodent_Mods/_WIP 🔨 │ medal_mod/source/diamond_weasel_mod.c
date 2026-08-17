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
/* The medal-award screen's per-frame UPDATE is 0x44D760 (award vtable
 * 0x4D6CF0 slot[1]) — the ONLY function that runs every frame the award
 * screen is live. It is SEH-protected: its prologue pushes a frame onto the
 * FS:[0] exception chain (`mov fs:[0]=esp` at 0x44D76E). Hooking its ENTRY
 * (0x44D760) breaks that chain, and 5-byte-JMP-to-heap from inside its frame
 * (e.g. the 0x44E139 draw site) corrupts the chain too. The SAFE host is the
 * first instruction AFTER the prologue installs the frame: 0x44D77B
 * (`lea ecx,[esi+0x4C4]`, 6 bytes -> JMP + 1 NOP). At that point FS:[0] is
 * valid, esi = results object, and it runs every award frame. We contain the
 * mod call in our OWN nested FS:[0] frame so nothing we raise reaches the
 * game's frame. */
#define AWARD_UPDATE_ENTRY    0x44D760   /* entry — patched with 5-byte JMP to wrapper */
#define AWARD_UPDATE_BODY     0x44D77B   /* jump-IN point: game body after its prologue */
#define AWARD_UPDATE_SCOPE    0x4CC77C   /* game's real SEH scope table (handler) for this fn */
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
/* 0x44E139 golden-weasel draw is NO LONGER hooked (proven to crash real
 * Windows); the reveal runs from the 0x44CBAA update cave instead. */
#define TT_WEASEL_APPEND   0x42F927   /* call 0x44abf0 (TT menu golden weasel) */
#define SKIP_LATCH_HOOK    0x44CBAA   /* movb $1,0x25(esi)  (skip latch set) */
#define SKIP_LATCH_RET     0x44CBB1   /* next instr after the 5 patched bytes */
#define RESULT_UPDATE_1    0x44CB90   /* MEDAL-AWARD / "click to continue" screen update
                                           (vtable 0x4D6CF0 slot 4 AND vtable 0x4D6CFC slot 1).
                                           NON-SEH, plain prologue `push ebx; push esi; mov esi,ecx;
                                           mov eax,[esi+0x1c]` (53 56 8B F1 8B 46 1C) = 7 bytes,
                                           RET 0x44CB97. Runs every frame the award screen is live.
                                           IMPORTANT: do NOT hook 0x44D760 (vtable 0x4D6CF0 slot 1)
                                           — that fn sets up an SEH frame (44d760) and calling mod
                                           C logic from inside it corrupts the exception chain on
                                           real Windows (cascading heap faults, crash before arm
                                           can log). 0x44CB90 is the non-SEH twin and is safe. */
#define RESULT_UPDATE_1_RET 0x44CB97  /* next instr after the 7 patched bytes at 0x44CB90 */
#define RESULT_UPDATE_EPILOGUE 0x44CC3F /* 0x44CB90's epilogue: pop edi;pop esi;pop ebx;ret (5F 5E 5B C3), runs EVERY
                                           award-update call with esi=results still live — the per-frame reveal host */
#define RESULT_UPDATE_2    0x44B860   /* results update (vtable 0x4D6C00 slot 1): mov eax,[ecx+0x10];mov edx,[ecx+0x14] */
#define RESULT_UPDATE_2_RET 0x44B866  /* next instr after the 6 patched bytes */
#define SPRITE_DRAW        0x42C7C0
#define ABF0_APPEND        0x44ABF0   /* medal list append (__stdcall, ret 8) */
#define STR_FMT_D          0x4D03F8   /* "%d" */
#define STR_BUF            0x4F7448   /* AthenaString buffer */

/* Inline reveal cave at 0x44E139 — fires INSTEAD of the game's own golden
 * weasel draw (`call 0x42c7c0`). Draws the vortex + white-fade trophy +
 * diamond swap from a results-screen-only context. NEVER runs during boot,
 * safe to install from the init thread. See install_weasel_cave(). */


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
/* Lead-in dead zone before the reveal countdown begins. The gold gate is at
 * results+0x4c (0x1e0=480), but some results screens reach the reveal state
 * earlier than the user expects — add a fixed no-op window up front so the
 * white-fade/vortex/trophy don't start too soon. Subtracted in
 * diamond_seq_frame so every phase (white start 55, total 240) shifts later
 * together. */
#define REVEAL_LEAD_IN      400
/* Total result-frame at which the white hold ends and the trophy reverts to
 * normal gold: (white start) + (active spawns) + (fade tail) + (hold). */
#define WEASEL_WHITE_TOTAL   (WEASEL_WHITE_START + (int)VORTEX_FRAMES + (int)VORTEX_TAIL + WEASEL_WHITE_HOLD)
#define APP_GFX             0x174      /* App+0x174 = gfx ptr */
#define SPRITE_GFX          0x04       /* sprite+4 = gfx ptr (Sprite_DrawRect uses) */
#define SPRITE_WEAEL_APP    0x37C      /* App+0x37C = goldenweasel.png sprite */
#define GFX_MULT_ENABLE     0x7A8      /* gfx+0x7A8 = color-mult enable byte */
#define GFX_MULT_R          0x7B0      /* gfx+0x7B0..0x7BC = RGBA scale */


/* Native medal-award effects. Each medal has its own frame-gated award block
 * in FUN_0044df70 that (1) plays the medal pop via Sound_PlayChannel(0x4597b0)
 * on the channel at App+0x50C and (2) spawns a ring of ArenaScoreParticle
 * objects (ctor 0x44ad50, alloc 0x28) appended to the results-screen particle
 * list ([results+0x94]). The diamond REPLACES the golden weasel, whose native
 * ring (block at 0x44d9a0, gated by frame == [esi+0x4C]) is 18 particles
 * spaced 20deg (=0x14) around center (227,648) at radius 74 — constants at
 * 0x4d6d8c/0x4d6d88/0x4d6d90. (The bronze/silver/gold rings use radius 30 at
 * centers (429,317)/(465,338)/(501,363) — the diamond must NOT use those.) */
#define Sound_PlayChannel  0x4597B0
#define SND_CHANNEL_MEDAL  0x50C      /* App+0x50C = medal pop channel */
#define RESULT_PARTICLES   0x94       /* results+0x94 = particle AthenaList */
#define PARTICLE_CTOR      0x44AD50
#define PARTICLE_SIZE      0x28
#define PARTICLE_VTABLE    0x4D6BF4   /* native override applied after the ctor */
#define ARENA_SCORE_LIST   0x4F7188   /* wave-const struct (+4 = PI), arg to Wave_Cos/Sin */
#define WAVE_COS           0x457DC0   /* Wave_Cos(0x4F7188, angle) */
#define WAVE_SIN           0x457DA0   /* Wave_Sin(0x4F7188, angle) */
#define PART_ANGLE_MULT    0x4D6D90   /* *74.0f radius (golden-weasel ring) */
#define PART_CENTER_X      0x4D6D8C   /* +227.0f (golden-weasel ring) */
#define PART_CENTER_Y      0x4D6D88   /* +648.0f (golden-weasel ring) */
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
#define D3DTA_TEXTURE              2
#define D3DTOP_SELECTARG1          2
#define D3DTOP_MODULATE            4
#define D3DTSS_COLOROP             0
#define D3DTSS_COLORARG1           1
#define D3DTSS_ALPHAARG1           4
#define D3D_DEV_SETTEXTURE         0xF4   /* vtable[61] SetTexture(dev, stage, tex) */
#define D3DFVF_XYZRHW              0x001
#define D3DFVF_DIFFUSE             0x040
#define D3DFVF_TEX1                0x100
#define D3DFVF_TLVERTEX            (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
#define D3DFVF_TLVERTEX_TEX1       (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

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
volatile int g_caveProbe = 0;        /* VEH-reading probe (0 = cave path untouched) */

/* ---- CONSOLIDATED PROCEDURAL-COMPOSITE VORTEX (Option A, 2026-08-17) ----
 * Replaces the raw-DrawPrimitiveUP vortex AND the separate white-out
 * mechanisms with a SINGLE procedurally-generated texture: we capture the
 * golden weasel's real pixels once, then each reveal frame build a composite
 * (weasel lerped to white + suction streaks in the annulus), upload it to a
 * CreateTexture'd scratch, bind it to a sprite +0x50, and swap that sprite
 * into ctx+0x37C so the game's own renderer draws it (ONE draw call).
 *
 * D3D8 slot constants (VERIFIED vs MinGW d3d8.h):
 *   IDirect3DDevice8::CreateTexture  = slot 20 / 0x50
 *   IDirect3DTexture8::GetLevelDesc  = slot 14 / 0x38
 *   IDirect3DTexture8::LockRect      = slot 16 / 0x40
 *   IDirect3DTexture8::UnlockRect    = slot 17 / 0x44
 */
#define D3D_DEV_CREATETEXTURE   0x50
#define D3D_TEX_GETLEVELDESC    0x38
#define D3D_TEX_LOCKRECT        0x40
#define D3D_TEX_UNLOCKRECT      0x44
#define D3DFMT_A8R8G8B8         0x15
#define D3DFMT_X8R8G8B8         0x16
#define D3DPOOL_MANAGED         0x01
#define D3DLOCK_READONLY        0x10
#define VORTEX_TEX_MIN          64
#define VORTEX_TEX_MAX          1024
/* The composite canvas is VORTEX_CANVAS_SCALE x the weasel's texture, with the
 * weasel drawn 1:1 centered in it. The sprite box stays the weasel's original
 * footprint (Sprite_DrawRect positions + maps the full canvas onto it), so the
 * centered weasel lands exactly where it was and only the annulus grows. */
#define VORTEX_CANVAS_SCALE     2.0f
/* Texture-relative streak geometry (fractions of captured tex width). */
#define VORTEX_TEX_STRETCH      0.16f
#define VORTEX_TEX_CENTER_FADE  0.04f
#define VORTEX_TEX_WEASEL_R     0.26f   /* weasel clear-disk radius (frac of w) */
static DWORD g_vortexDevice = 0;        /* device we created our texture with */
static BYTE  *g_vortexCaptured = NULL;  /* weasel RGBA capture (w*h*4) */
static int    g_vortexCaptW = 0, g_vortexCaptH = 0;   /* canvas = weasel x VORTEX_CANVAS_SCALE */
static int    g_vortexWeaselW = 0, g_vortexWeaselH = 0;/* real weasel texture pixel size */
static DWORD  g_vortexTex = 0;          /* our scratch texture */
static int    g_vortexTexW = 0, g_vortexTexH = 0;
static DWORD  g_vortexSprite = 0;       /* our composite sprite */
static DWORD  g_vortexOrigTex = 0;      /* the weasel texture we captured */
/* ctx+0x37C swap state for the composite vortex (SEPARATE from the trophy
 * swap so the two don't fight: trophy swap is the glassy/swap at +240; this
 * is the during-white window). */
static int    g_vortexSwapActive = 0;
static DWORD  g_vortexSwapCtx = 0;
static DWORD  g_vortexSwapOrig = 0;     /* orig sprite (weasel) at ctx+0x37C */
static int    g_vortexSwapResults = 0;
/* Anchor shift for the composite (see below). The medal draw pins the sprite
 * at a fixed top-left (0x208,0x63) via two immediates at 0x44E132/0x44E134.
 * To keep the FULL-SIZE centered weasel exactly in place while the canvas (and
 * box) grow 2x around it, we shift the anchor by -origBox/2. These are pure
 * constant reads in the render fn - a safe data write, restored on disarm
 * BEFORE the diamond swap so the diamond isn't shifted too. */
static int    g_vortexAnchorPatched = 0;
#define ANCHOR_PATCH_X   0x44E134   /* push $0x208 (imm32) */
#define ANCHOR_PATCH_Y   0x44E132   /* push $0x63 (imm8) */
#define ANCHOR_ORIG_X    0x208
#define ANCHOR_ORIG_Y    0x63
/* The original weasel sprite's DRAW BOX (world units, +0xC8/+0xCC) — the
 * composite sprite copies these so it renders in the exact same spot/size.
 * Our texture (g_vortexCaptW x g_vortexCaptH) maps 0..1 UV onto this box. */
static float  g_vortexBoxW = 0.0f, g_vortexBoxH = 0.0f;

/* ================================================================
 * Mod globals
 * ================================================================ */
static DWORD g_diamondSprite = 0;
static DWORD g_diamondMiniSprite = 0;

/* Star-ring center, computed at trophy-swap time from the ACTUAL sprite that
 * is (or will be) in ctx+0x37C — the diamond trophy's true on-screen center.
 * Set by diamond_trophy_swap before diamond_spawn_medal_effects, so the ring
 * never depends on hardcoded/guessed coordinates. Defaults to the draw top-left
 * (0x208,0x63) + half the sprite's live width/height (sprite+0xC8/+0xCC). */
static float g_ringCx = 0.0f, g_ringCy = 0.0f;
/* ---- RESULTS-SCREEN BIG DIAMOND TROPHY (render-path fix, 2026-08-16) ----
 * The golden-weasel medal is composited by the game's RENDER fn (award vtable
 * slot[2]=0x44DF70, draw at 0x44E12C):  mov ecx,[ctx+0x37C]; Sprite_DrawRect
 * (ctx+0x37C = golden-weasel sprite slot, ctx = *(results+0xC)). Drawing the
 * diamond from the UPDATE host never composites. Instead we redirect the
 * game's own renderer: swap the sprite pointer at ctx+0x37C to g_diamondSprite
 * (qualification is decided from the safe update host), restore it when the
 * reveal is no longer active. No heap cave in the render/SEH path. */
#define RESULT_CTX  0x0C    /* results+0xC = display context (see ctor 0x44C8B4) */
#define CTX_WEASEL  0x37C   /* ctx+0x37C = golden-weasel sprite slot (renderer reads this) */
static DWORD g_trophySwapCtx  = 0;   /* ctx whose +0x37C we redirected */
static DWORD g_trophySwapOrig = 0;   /* original golden-weasel sprite at ctx+0x37C */
static DWORD g_trophySwapResults = 0;/* results obj the swap belongs to (cross-screen guard) */
static int   g_trophySwapActive = 0; /* 1 while the renderer is showing the diamond here */
static unsigned char *g_skipCave = NULL;  /* results-screen skip-latch cave (0x44CBAA) */
static int   g_secret_cs[15] = {0};   /* per-race DIAMOND threshold in CENTISECONDS (int) */
static int   g_hasSecret[15] = {0};
static BYTE  g_won[15]       = {0};
static int   g_ttAppendCount = 0;   /* bumped by the TT wrapper diamond append (diagnostic) */
static int   g_anyDiamond    = 0;    /* 1 once ANY diamond has been earned (gates TT-menu cave) */
static char  g_iconFile[64]  = "diamondweasel.png";
static char  g_miniIconFile[64] = "diamondweasel-icon.png";
static int   g_iconLoaded    = 0;
static int   g_miniIconLoaded = 0;
static int   g_configLoaded  = 0;
static char  g_fmtDiamond[]  = "%dD";

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

/* ---- Crash-safe trace logging (no file I/O inside the game's SEH frame) ----
 * The award-screen update (0x44D760) installs a live SEH frame via FS:[0], and
 * our reveal helpers run INSIDE that frame every frame the results screen is up.
 * Calling fopen/fprintf/fflush there crashes real Windows with C0000005 (CRT
 * file I/O is not safe from inside a live exception frame during dispatch) —
 * Wine tolerates it, Windows does not. So ALL frame-path tracing writes to an
 * in-memory ring buffer under a spinlock, and a dedicated background thread
 * drains it to disk. Nothing in the frame path ever touches the filesystem. */
#define RING_ROWS    256
#define RING_LEN     160
static char    g_ring[RING_ROWS][RING_LEN];
static volatile int  g_ringW = 0;   /* next write slot (monotonic) */
static volatile LONG g_ringLock = 0;/* spinlock */
static int g_flusherStarted = 0;    /* ensure the flusher thread starts once */

/* Lock-free-ish spinlock helpers (no CRT, no SEH). */
static void ring_lock(void)   { while (InterlockedCompareExchange(&g_ringLock,1,0)) Sleep(0); }
static void ring_unlock(void) { InterlockedExchange(&g_ringLock,0); }

/* Thread-safe in-memory trace. NEVER does file I/O. Safe to call from inside
 * the game's live SEH frame. This is the crash-safe replacement for per-frame
 * diag_logf calls in the award/reveal path. */
static void trace_logf(const char *fmt, ...) {
    char buf[256]; va_list ap;
    va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    ring_lock();
    int slot = (g_ringW % RING_ROWS);
    strncpy(g_ring[slot], buf, RING_LEN-1);
    g_ring[slot][RING_LEN-1] = 0;
    g_ringW++;
    ring_unlock();
}

static unsigned char *g_iconCave = NULL;
static unsigned char *g_ttCave = NULL;
static int   g_ttInstalled = 0;   /* has cherry TT cave been patched in? Start 0;
                                   * set 1 after install so it's idempotent whether
                                   * called at startup (g_anyDiamond already 1) or
                                   * lazily from diamond_provision_unlock. */
static unsigned char *g_presentStub = NULL;    /* award-update post-SEH cave stub */

/* ================================================================
 * Logging + path helpers
 * ================================================================ */
static void diag_log(const char *msg) {
    if (!g_log) { if (g_logPath[0]) g_log = fopen(g_logPath, "a"); }
    if (g_log) { fprintf(g_log, "%s\n", msg); fflush(g_log); }
}
static void diag_logf(const char *fmt, ...);  /* defined below */

/* Throttled cave-phase tracer. Logs the cave-helper NAME the moment the
 * helper is entered — BEFORE any game-state reads — so a real-Windows crash
 * that happens during the first read still identifies which helper it was.
 * Throttled to once per observed results-session pointer.
 * CRASH-SAFE: writes to the in-memory ring (trace_logf), never file I/O, so
 * it is safe to call from inside the award screen's live SEH frame. */
static void cave_enter(DWORD results, const char *phase) {
    static DWORD last_results = 0;
    if (results != last_results) {
        last_results = results;
        trace_logf("[diamond] CAVE-ENTER results=%08X -> %s", results, phase);
    }
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
static void install_tt_cave(void);   /* defined below (patch helpers) — called
                                      * lazily when the first diamond is earned
                                      * so the TT mini-icon shows this session. */

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
    ok_reg = save_unlocks_reg();
    if (!ok_reg) {
        g_won[race] = 0;   /* roll back — flag would exist in memory but not on disk */
        diag_logf("[diamond] provision FAILED for race %d (reg=%d) — unlock rolled back",
                  race, ok_reg);
        return 0;
    }
    /* 3. Only after the registry commit succeeds: a diamond now exists -> the
     *    TT-menu cave becomes eligible. Normally install_tt_cave() runs at
     *    startup (a diamond was already earned on a prior save), but if this
     *    is the player's FIRST diamond ever, g_anyDiamond was 0 at startup and
     *    the cave is NOT yet installed — so install it NOW so the mini-icon
     *    shows in the standings THIS session, not only on the next launch. */
    g_anyDiamond = 1;
    install_tt_cave();
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
    /* LOG the actual g_won bytes so we can see WHICH races are flagged (esp.
     * Warm-Up=0 / Odd=8) and whether the load is byte-aligned to our indices. */
    {
        char buf[96]; int n=0;
        n += sprintf(buf+n, "[diamond] g_won[0..14] =");
        for (int i=0;i<15;i++) n += sprintf(buf+n, " %d", g_won[i]);
        diag_log(buf);
    }
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
    int idx = *(int*)(prof + PROFILE_RACE);
    /* The game's tournament race slot (Profile+0x8) is 1-INDEXED: slot 1 =
     * WARM-UP, ..., slot 9 = ODD, slot 10 = TOOB (verified in
     * Tournament_AdvanceRace 0x427080 switch). Our per-race arrays
     * (g_secret_cs, g_default_diamond_s, g_xml_block) are 0-indexed. Convert:
     * game slot N -> our index N-1. Without this, Odd (game slot 9) hit
     * our [9]=TOOB default (25.0s) instead of Odd's 12.0s. */
    if (idx >= 1) idx -= 1;
    return idx;
}
/* board+0x1C is an INTEGER centisecond counter (native medal code compares
 * it with CMP at 0x44d932 / 0x44d958), NOT a float. Reading it as float
 * reinterprets e.g. 3000 (30s) as a denormal ~4e-42, which is always
 * "beats the secret" -> silent unlock on every race. */
static int get_player_time_cs(DWORD app) {
    /* The AUTHORITATIVE player-1 finish time is App+0x5E8 (centiseconds),
     * NOT board+0x1C. board+0x1C is a transient/partial value read as time=0
     * even on a diamond-qualifying run (and read as 61 on a non-qualifying
     * run) — it is not the frozen race time. App+0x5E8 is the player-1 timer
     * slot (stride 0xA0); the N:GOAL handler reads it (0x40CF38) and freezes
     * it into the BTT tracker at the goal moment — the authoritative finish
     * time in centiseconds (threshold 2500 = 25.00s). */
    if (IsBadReadPtr((void*)(app + 0x5E8), 4)) return 0;
    return *(int*)(app + 0x5E8);
}

/* ================================================================
 * Core logic (called from caves)
 * ================================================================ */
__attribute__((used)) void diamond_load_icon_impl(DWORD app);
__attribute__((used)) void diamond_spawn_medal_effects(DWORD results, DWORD app);
static void install_reveal_cave(void);   /* defined below (patch helpers) */
static void install_skip_cave(void);     /* defined below (patch helpers) */
static void diamond_set_add(DWORD results, float intens);   /* defined above */
static void vortex_patch_anchor(int on);   /* defined in the procedural-composite block */
/* patch-helper forward decls (defined in the patch-helpers section) */
static void write_jmp(unsigned char *at, DWORD target);
static void patch_bytes(void *addr, const void *data, DWORD size);

static int diamond_seq_frame(DWORD results);   /* frames since gold award */
/* reveal-helper forward decls (defined below, used by diamond_reveal_draw) */
__attribute__((used)) void diamond_vortex_tick(DWORD results);
__attribute__((used)) void diamond_weasel_mult(DWORD results);
__attribute__((used)) void diamond_weasel_mult_clear(DWORD results);
__attribute__((used)) int  diamond_trophy_swap(DWORD results);
__attribute__((used)) void diamond_trophy_restore(DWORD results);
static void vortex_disarm(void);   /* defined in the procedural-composite block */

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
    cave_enter(results, "block_skip");
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
#define RESULTS_VTABLE       0x4D6CB8   /* smaller results/"score updater" screen */
#define RESULTS_VTABLE_OLD_1 0x4D6CFC   /* "click to continue" continuation */
#define RESULTS_VTABLE_OLD_2 0x4D6C00   /* pre-upgrade base results object */
#define RESULTS_VTABLE_AWARD 0x4D6CF0   /* the MEDAL-AWARD screen: vtable[1]=0x44D760
                                           (the per-frame update that awards gold +
                                           runs the reveal). This is the object that
                                           actually shows on "beat a level" with a
                                           medal time — the arm cave MUST hook this. */

/* Spawn the native medal-award effects (pop sound + star ring) for the
 * diamond, mirroring FUN_0044df70's first-earn block (0x44daf8-0x44dc10).
 * Called once, on the frame the diamond is first unlocked+shown for a race.
 *   results = results-screen object (particle list at +0x94)
 *   app     = App ptr (medal pop channel at +0x50C)
 */
__attribute__((used)) void diamond_spawn_medal_effects(DWORD results, DWORD app) {
    int angle, i;
    DWORD channel, plist, part, sprite;
    if (!results || !app) return;
    if (IsBadReadPtr((void*)(results + RESULT_PARTICLES), 4)) return;
    plist = results + RESULT_PARTICLES;
    /* Resolve the ring CENTER from the ACTUAL trophy sprite (not a hardcoded
     * constant). The award renderer draws the gold-weasel/diamond trophy from
     * ctx+0x37C at a world TOP-LEFT of (0x208, 0x63) (Sprite_DrawRect at 0x44E12C:
     * mov ecx,[ctx+0x37C]; push 0x63; push 0x208; call Sprite_DrawRect). The
     * sprite's real display size is sprite+0xC8 (width) / +0xCC (height) in
     * world units (set in Sprite_ctor, and the renderer builds corners from
     * them). So the trophy's true center is top-left + half dims:
     *   cx = 0x208 + sprite[0xC8]/2    cy = 0x63 + sprite[0xCC]/2
     * The ring particles render through the SAME world->screen transform as
     * the sprite (particle vtable 0x44ACE0 -> 0x45d300), so centering on the
     * sprite's world-box center makes the burst coincide with the diamond —
     * regardless of the PNG's aspect (user reports the trophy renders as a
     * ~2:3 rectangle, not the 256x256 PNG). falls back to (227,648) r=74
     * (the native golden-weasel ring) only if the sprite is unreadable. */
    if (IsBadReadPtr((void*)(app + CTX_WEASEL), 4)) return;
    sprite = *(DWORD*)(app + CTX_WEASEL);   /* app here = ctx = *(results+0xC) */
    if (sprite && sprite > 0x10000 && !IsBadReadPtr((void*)(sprite + 0xC8), 8)) {
        float w = *(float*)(sprite + 0xC8), h = *(float*)(sprite + 0xCC);
        if (w > 1.0f && h > 1.0f) {
            g_ringCx = 0x208 + w * 0.5f;
            g_ringCy = 0x63  + h * 0.5f;
            diag_logf("[diamond] ring center from sprite: w=%.1f h=%.1f -> (%.1f, %.1f)",
                      w, h, g_ringCx, g_ringCy);
        }
    }
    if (g_ringCx == 0.0f || g_ringCy == 0.0f) { g_ringCx = 227.0f; g_ringCy = 648.0f; }
    /* (1) Play the medal pop on the medal channel (App+0x50C). */
    if (IsBadReadPtr((void*)(app + SND_CHANNEL_MEDAL), 4)) return;
    channel = *(DWORD*)(app + SND_CHANNEL_MEDAL);
    if (channel) {
        /* Sound_PlayChannel is __fastcall(ecx=channel) AND takes one stack
         * arg (volume, pushed as 1.0f). The function ends in `ret 4` (verified
         * at 0x459804 / 0x4595B0) and EVERY native call site pushes 0x3F800000
         * first — see 0x44DB03, 0x44D991, 0x403805. Omitting the push leaves
         * the callee to pop 4 bytes off OUR frame (stack corruption on return). */
        __asm__ volatile(
            "pushl $0x3F800000\n\t"   /* volume 1.0f (callee ret 4 pops it) */
            "movl %1, %%ecx\n\t"     /* channel (ecx = this) */
            "call *%0\n\t"           /* Sound_PlayChannel(channel, 1.0f) */
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
        /* Native override: the medal ring sets the results-screen vtable
         * (0x4D6BF4) over the ctor's generic vtable (0x4D6AEC). Without it the
         * particles update/destroy through the wrong functions. */
        *(DWORD*)part = PARTICLE_VTABLE;
        /* Native layout (verified): part+0x08..0x10 = POSITION vec3,
         * part+0x14..0x1C = VELOCITY/trajectory vec3 (UNIT radial dir, NO
         * radius factor — the native code applies the radius factor only to
         * position; the velocity block is a bare unit vector).
         *   pos.x = cos(a)*r + cx   pos.y = sin(a)*r + cy
         *   vel.x = cos(a)          vel.y = sin(a)
         * The diamond REPLACES the golden weasel (it swaps into the SAME
         * ctx+0x37C slot the gold weasel trophy draws from), so the ring uses
         * the NATIVE GOLDEN-WEASEL ring constants — verified in the award
         * render block 0x44D980 (flag 0x8BF):
         *   cos(deg) * 0x4d6d90(74) + 0x4d6d8c(227)  -> X
         *   sin(deg) * 0x4d6d90(74) + 0x4d6d88(648)  -> Y
         * angle = loop counter 0,20,..340 deg.
         * (0x4d6d84=429/0x4d6d80=317 are the SILVER ring's, and (429,317)
         * appeared down-left — NOT the weasel's. (227,648) r=74 is correct.) */
        {
            float rad = (float)angle * 3.14159265f / 180.0f;
            float c = cosf(rad), s = sinf(rad);
            /* center computed from the actual ctx+0x37C sprite box above. */
            float cx = g_ringCx, cy = g_ringCy, r = 74.0f;
            *(float*)(part + 0x08) = c * r + cx;
            *(float*)(part + 0x0C) = s * r + cy;
            *(float*)(part + 0x10) = 0.0f;
            *(float*)(part + 0x14) = c;
            *(float*)(part + 0x18) = s;
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
    diag_logf("[diamond] medal effects spawned (pop + %d star particles)", i);
}

/* RESULT_OBJ offsets used by the weasel white-fade + diamond 5th-medal. */
#define RESULT_FRAME   0x10   /* frame counter [esi+0x10] */
#define RESULT_GOLD    0x4c   /* gold medal awarded when frame == [esi+0x4c];
                                 drawn when frame > [esi+0x4c] (cmp 0x44e113).
                                 NOTE: was mislabeled RESULT_GATE_GOLD 0x74 —
                                 0x74 is a DIFFERENT medal's gate (0x44dd67). */
#define RESULT_APP     0x0C   /* App ptr [esi+0xc] */

/* Frames SINCE the start of the reveal countdown. The gold gate is at
 * results+0x4c; the reveal countdown begins REVEAL_LEAD_IN frames after gold
 * (a fixed no-op window so the white-fade/vortex/trophy don't start too soon).
 * Returns the raw frame counter if +0x4c is unreadable (defensive), and
 * clamps below 0 during the lead-in (all phases gate on < threshold so they
 * hold off correctly). */
__attribute__((used)) static int diamond_seq_frame(DWORD results) {
    int frame, gold;
    if (!results) return 0;
    if (IsBadReadPtr((void*)(results + RESULT_FRAME), 4)) return 0;
    frame = *(int*)(results + RESULT_FRAME);
    if (IsBadReadPtr((void*)(results + RESULT_GOLD), 4)) return frame;
    gold = *(int*)(results + RESULT_GOLD);
    if (gold < 0) gold = 0;
    return (frame < (gold + REVEAL_LEAD_IN)) ? 0 : (frame - gold - REVEAL_LEAD_IN);
}

/* True only while the diamond's FIRST-EARN reveal is running for the current
 * race: the diamond time was met AND the race hasn't been earned before.
 * The white-out, vortex, and trophy-swap reveal all gate on this so they play
 * only on the genuine first earn — on a replay the golden weasel renders
 * normal gold, then the diamond just swaps in at gold+240 with no buildup. */
static int g_firstEarnLog[15] = {0};   /* one-shot diagnostic per race */
__attribute__((used)) static int diamond_first_earn(DWORD results) {
    int race = -1, cs = 0, thr = 0;
    DWORD app = 0;
    const char *skip = NULL;   /* first blocking reason, logged once/session */
    if (!results)                         skip = "no-results";
    else if (!g_configLoaded)            skip = "no-config";
    else if (IsBadReadPtr((void*)(results + RESULT_APP), 4))
                                          skip = "app-BAD";
    else if (!(app = *(DWORD*)(results + RESULT_APP)))
                                          skip = "app-null";
    else { race = get_race_index();
           if (race < 0 || race > 14)     skip = "race-OOR";
           else if (!g_hasSecret[race])   skip = "no-secret";
           else if (g_won[race])          skip = "already-won";
    }
    if (skip) { cave_enter(results, skip); return 0; }
    cs = get_player_time_cs(app);
    thr = g_secret_cs[race];
    cave_enter(results, "checks-passed");
    if (!g_firstEarnLog[race]) {
        g_firstEarnLog[race] = 1;
        trace_logf("[diamond] FIRST-EARN race=%d time=%d thr=%d won=%d cfg=%d hasSecr=%d",
                  race, cs, thr, g_won[race], g_configLoaded, g_hasSecret[race]);
    }
    return (cs > 0 && cs <= thr) ? 1 : 0;
}

/* Consolidated reveal draw — the SINGLE helper the 0x44E139 cave calls.
 *
 * On a FAST-but-not-diamond run (weasel medal drawn, no diamond time), and on
 * replays, diamond_first_earn is 0 -> we do NOTHING (no I/O, no D3D, no sound,
 * no color-mult) and return 0 so the cave's gold draw proceeds untouched. Only
 * when a genuine first-earn reveal is active do we run the vortex, white-out,
 * and trophy-swap. This keeps the common golden-weasel draw path completely
 * inert — a key crash fix: earlier caves called logging/vortex/color helpers on
 * every weasel draw, and entering those (which pull in fopen/vsnprintf/D3D/
 * sound calls) from inside the game's embedded award-draw frame crashed real
 * Windows. 
 * Returns 1 to skip gold (diamond drawn), 0 to draw gold. */
__attribute__((used)) int diamond_reveal_draw(DWORD results) {
    int frame;
    /* UNCONDITIONAL one-shot per-run probe: confirms whether the 0x44CC3F
     * cave fires AT ALL (before any gate). 0 = cave not reached. */
    { static int p = 0; if (!p) { p = 1; trace_logf("[diamond] CAVE-FIRED results=%08X", results); } }
    if (!diamond_first_earn(results)) return 0;   /* no reveal -> plain gold */
    frame = diamond_seq_frame(results);
    if (frame < WEASEL_WHITE_TOTAL) {
        /* reveal in progress: the procedural-composite vortex (diamond_vortex_tick)
         * does white-lerp + streaks in ONE texture swapped into ctx+0x37C. The
         * game's render draws it. Gold is still drawn (swap happens at
         * WEASEL_WHITE_TOTAL, i.e. the trophy swap frame). */
        diamond_vortex_tick(results);
        return 0;                        /* draw gold (composite shown over it) */
    }
    /* paste the hold end -> swap to diamond */
    return diamond_trophy_swap(results);  /* draws diamond + returns 1 to skip gold */
}

/* ---- Award-update reveal driver (full-wrapper SEH replacement) ----
 * Replace the ENTIRE award-update function 0x44D760 with our own wrapper that
 * re-implements the EXACT SEH prologue (install a byte-identical FS:[0] frame
 * using the game's REAL scope table 0x4CC77C), runs the reveal inside OUR OWN
 * valid frame, then jumps into the game's untouched body (0x44D77B) and lets
 * ITS epilogue restore FS:[0].
 *
 * WHY THIS IS THE CORRECT FIX (vs every prior host):
 *   - 0x44E139 (inside SEH award-DRAW 0x44DF70): redirecting to heap mid-frame
 *     corrupts the exception chain -> real-Windows crash (proven, d2453e65).
 *   - 0x44D77B post-SEH (patch inside the award-UPDATE SEH frame): even a
 *     balanced no-I/O call from inside the game's live frame CASCADED into the
 *     same heap-EIP C0000005 on real Windows (two identical logs). The problem
 *     is running our heap code inside a frame the game built, not I/O.
 *   - 0x44CB90/0x44CC3F are the continuation (not the per-frame medal driver).
 *   - 0x46C1F1 present hook fires at boot (LoadingScreen) -> startup crash.
 *
 * The full-wrapper REPLACES the frame with a byte-identical one we own, so the
 * exception chain is never patched mid-flight and never corrupts. This is the
 * only host with no dependence on the game's frame timing.
 *
 * Stack math (verified): wrapper replicates prologue -> at jump-in to 0x44D77B,
 * [esp] and FS:[0] match native post-prologue state (frame at E-12, saved
 * handler at [E-12], esp=E-40, esi=results). Game epilogue restores FS:[0]
 * from [E-12] correctly and rets. See /tmp/verify_stack.py.
 */
static void install_reveal_cave(void) {
    DWORD entry = EXE_BASE + (AWARD_UPDATE_ENTRY - EXE_BASE);
    DWORD body  = EXE_BASE + (AWARD_UPDATE_BODY  - EXE_BASE);
    g_presentStub = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE,
                                                 PAGE_EXECUTE_READWRITE);
    if (!g_presentStub) return;
    unsigned char *p = g_presentStub;
    /* --- replicate the game's REAL SEH prologue byte-for-byte --- */
    p[0]=0x64; p[1]=0xA1;  p[2]=0x00; p[3]=0x00; p[4]=0x00; p[5]=0x00; p+=6; /* mov eax,fs:[0] */
    p[0]=0x6A; p[1]=0xFF;                                    p+=2;         /* push -1 */
    p[0]=0x68; *(DWORD*)(p+1) = EXE_BASE+(AWARD_UPDATE_SCOPE-EXE_BASE); p+=5; /* push 0x4CC77C */
    p[0]=0x50;                                                p+=1;         /* push eax */
    p[0]=0x64; p[1]=0x89; p[2]=0x25; p[3]=0x00; p[4]=0x00; p[5]=0x00; p+=6; /* mov fs:[0],esp */
    p[0]=0x83; p[1]=0xEC; p[2]=0x18;                          p+=3;         /* sub esp,0x18 */
    p[0]=0x56;                                                p+=1;         /* push esi */
    p[0]=0x8B; p[1]=0xF1;                                     p+=2;         /* mov esi,ecx */
    /* --- run the reveal inside THIS authentic frame (handle via game's handler) --- */
    p[0]=0x60;                                                p+=1;         /* pushad */
    p[0]=0x56;                                                p+=1;         /* push esi (arg) */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_reveal_draw-(DWORD)(p+5); p+=5; /* call */
    p[0]=0x83; p[1]=0xC4; p[2]=0x04;                          p+=3;         /* add esp,4 */
    p[0]=0x61;                                                p+=1;         /* popad */
    /* --- hand off to the game's untouched body at 0x44D77B --- */
    write_jmp(p, body);                                        p+=5;
    /* Patch the entry: 5-byte JMP to wrapper (6th byte = padding, never exec) */
    unsigned char patch[5];
    write_jmp(patch, (DWORD)g_presentStub);
    patch_bytes((void*)entry, patch, 5);
    diag_log("[diamond] reveal: FULL WRAPPER replaces 0x44D760 (own SEH frame via 0x4CC77C, reveal inside, then jmp 0x44D77B)");
}

/* Set the golden-weasel sprite's color-multiplier so it renders white,
 * phased over result-frames [55,150]. Sets gfx+0x7A8=1 and the RGBA scale at
 * gfx+0x7B0..0x7BC to (m,m,m,1). gfx comes from the weasel sprite (sprite+4).
 *
 * CRITICAL (shared-global fix): gfx+0x7A8 (enable) + gfx+0x7B0..0x7BC (RGBA
 * scale) is a GAME-OWNED SHARED multiplier the whole screen uses. Leaving it
 * set globally (the old m pegged at WEASEL_WHITE_MULT=4.0) bleaches the ENTIRE
 * race render ("the race whited out") and corrupts the next screen. We:
 *   - SAVE the prior enable byte + 4 floats ONCE at the start of the fade,
 *   - only ramp toward white and CAP at 1.0 (a clean white tint; >1.0 blows
 *     out the whole frame),
 *   - RESTORE the saved prior state when the fade ends (the reveal driver
 *     calls diamond_weasel_mult_clear at WEASEL_WHITE_TOTAL) — not force
 *     identity, preserving whatever the game had queued.
 */
static int   g_multSaved = 0;
static BYTE  g_multSaveEnable = 0;
static float g_multSave[4] = {1,1,1,1};
static float g_weaselIntens = 0.0f;   /* fade intensity 0..1 for the additive blend */
__attribute__((used)) void diamond_weasel_mult(DWORD results) {
    int frame;
    float m, t, *sc;
    DWORD app, sprite, gfx;
    cave_enter(results, "weasel_mult");
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
    /* save the prior shared state once */
    if (!g_multSaved) {
        g_multSaveEnable = *(volatile unsigned char*)(gfx + GFX_MULT_ENABLE);
        if (!IsBadReadPtr((void*)(gfx + GFX_MULT_R), 16))
            memcpy(g_multSave, (float*)(gfx + GFX_MULT_R), 16);
        g_multSaved = 1;
    }
    /* Weasel color-multiplier is DISABLED from the shared global path (it
     * bleaches the whole race). Instead EXPERIMENT (2026-08-16, user-approved
     * low-risk data-write): drive the GOLDEN-WEASEL SPRITE's OWN material
     * diffuse color white, per-frame, from this safe update host — no code
     * cave. The sprite material is the D3DMATERIAL8 at [sprite+8]; its Diffuse
     * (D3DCOLORVALUE, 4 floats) is at [sprite+8+4] = [sprite+0x0C]. The sprite
     * draw (0x45D300 -> 0x455110) passes [sprite+8] to Graphics_ApplyMaterial
     * and draws texture x diffuse (MODULATE). We ramp the diffuse from white
     * toward a brighten, so the gold WASHES toward white without touching the
     * shared gfx multiplier. If the renderer honors material diffuse
     * multiplicatively, the trophy whitens on its own; if not, we learn that
     * here and escalate to a minimal self-limited blend hook.
     *
     * NOTE: default sprite diffuse is already (1,1,1,1), so a multiply can't
     * exceed the texture's own max. The REAL lever for a true white-blowout is
     * a per-sprite ADD blend. But per the approved plan we FIRST confirm whether
     * the renderer honors per-sprite material diffuse at all before any render
     * path change. To make the test observable, ramp the diffuse to (1,1,1,1)
     * AND flip the material's alpha-blend flag (+0x4C) so we also learn if the
     * +0x4C byte reaches the SRC/DEST blend decision. */
    t = (frame - WEASEL_WHITE_START) / (float)(WEASEL_WHITE_END - WEASEL_WHITE_START);
    if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
    g_weaselIntens = t;                 /* 0..1 additive blend intensity for the reveal */
    m = 1.0f + t * 1.0f;        /* diffuse 1.0 -> 2.0 (attempt a white push) */
    {   /* write the sprite's own material diffuse + alpha-blend flag */
        float *d = (float*)(sprite + 0x0C);          /* material diffuse RGB(A) */
        if (!IsBadReadPtr(d, 16)) {
            d[0] = m; d[1] = m; d[2] = m;             /* diffuse RGB */
            d[3] = 1.0f;                              /* diffuse alpha */
        }
        *(volatile unsigned char*)(sprite + 0x4C) = 1;  /* material blend flag */
    }
    /* THROTTLED: don't spam the ring every frame during the 55-240 fade. Log
     * only start/end + every 15 frames so a freeze at a specific frame (like
     * the 131-stall seen in the field) is still clearly visible without a
     * ~250-line flood. */
    {   static int lastLogged = -1;
        if (lastLogged < 0 || frame == WEASEL_WHITE_END ||
            frame == WEASEL_WHITE_TOTAL-1 || frame - lastLogged >= 15) {
            trace_logf("[diamond] weasel sprite-diffuse frame=%d m=%.2f sprite=%08X",
                       frame, m, sprite);
            lastLogged = frame;
        }
    }
}

/* Clear the weasel color-multiplier back to identity (all 1.0) so
 * subsequent draws are unaffected. */
__attribute__((used)) void diamond_weasel_mult_clear(DWORD results) {
    DWORD app, sprite, gfx;
    float *sc;
    cave_enter(results, "weasel_mult_clear");
    if (!results) return;
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app || IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) return;
    sprite = *(DWORD*)(app + SPRITE_WEAEL_APP);
    if (!sprite || IsBadReadPtr((void*)(sprite + SPRITE_GFX), 4)) return;
    gfx = *(DWORD*)(sprite + SPRITE_GFX);
    if (!gfx || IsBadReadPtr((void*)(gfx + GFX_MULT_R), 4)) return;
    /* Restore the sprite's own material diffuse + blend flag back to the game's
     * defaults (1,1,1,1 and 0) so the gold-weasel/sprite isn't left modified
     * after the white-out. */
    {   float *d = (float*)(sprite + 0x0C);
        if (!IsBadReadPtr(d, 16)) { d[0]=1.0f; d[1]=1.0f; d[2]=1.0f; d[3]=1.0f; }
        *(volatile unsigned char*)(sprite + 0x4C) = 0;
    }
    /* restore the PRIOR saved shared state (not force identity) so we don't
     * clobber whatever the game had queued for the frame. */
    if (g_multSaved) {
        *(volatile unsigned char*)(gfx + GFX_MULT_ENABLE) = g_multSaveEnable;
        if (!IsBadReadPtr((void*)(gfx + GFX_MULT_R), 16))
            memcpy((float*)(gfx + GFX_MULT_R), g_multSave, 16);
        g_multSaved = 0;
    } else {
        /* no prior save (shouldn't happen) — safe default identity */
        *(volatile unsigned char*)(gfx + GFX_MULT_ENABLE) = 0;
        sc = (float*)(gfx + GFX_MULT_R);
        sc[0] = sc[1] = sc[2] = sc[3] = 1.0f;
    }
}

/* ---- SEH-SAFE additive white-out (2026-08-16) ----
 * The award/medal screen renders ONLY 2D sprites (verified: its render fn
 * 0x44DF70 calls only Sprite_DrawRect/0x42c7c0 + text helpers — it never
 * re-renders the 3D level). So flipping the device SRC/DEST blend to ADDITIVE
 * during the white-out only affects the medal panel's own sprites — the level
 * behind stays normal. This is issued from the SAFE vtable-update host (NOT
 * the SEH render fn; never patch 0x44E139 — that corrupted the exception chain
 * on real Windows, proven). Persistent device render-state: set here before the
 * render fn draws the frame, restored once the fade ends. */
#define D3DBLEND_ONE 2
static int   g_blendSaved = 0;
static DWORD g_blendSaveSrc = 0, g_blendSaveDst = 0;   /* saved game default blend */
static DWORD g_lastBlendDev = 0;                        /* dev we left ADDitive on */
static int   g_addDown = 0;                             /* 1 while we're holding ADD */
/* intens in [0..1]: 0 = normal, >0 ramps the trophy toward additive white. */
static void diamond_set_add(DWORD results, float intens) {
    DWORD app, sprite, gfx, device, vt, src, dst;
    if (!results || intens < 0.0f) return;
    if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
    app = *(DWORD*)(results + RESULT_APP);
    if (!app || IsBadReadPtr((void*)(app + CTX_WEASEL), 4)) return;
    sprite = *(DWORD*)(app + CTX_WEASEL);
    if (!sprite || IsBadReadPtr((void*)(sprite + SPRITE_GFX), 4)) return;
    gfx = *(DWORD*)(sprite + SPRITE_GFX);
    if (!gfx || IsBadReadPtr((void*)(gfx + GFX_DEV_OFFSET), 4)) return;
    device = *(DWORD*)(gfx + GFX_DEV_OFFSET);
    if (!device || IsBadReadPtr((void*)device, 4)) return;
    vt = *(DWORD*)device;
    if (!vt || IsBadReadPtr((void*)(vt + D3D_DEV_SETRENDERSTATE), 4)) return;
    {
        typedef HRESULT (__stdcall *PFN_SetRenderState)(void*, int, DWORD);
        PFN_SetRenderState SetRenderState = (PFN_SetRenderState)(*(void**)(vt + D3D_DEV_SETRENDERSTATE));
        void *dev = (void*)device;
        if (!SetRenderState) return;
        if (intens > 0.001f) {
            /* entering additive for the reveal */
            if (!g_addDown || g_lastBlendDev != device) {
                /* Save the game's intended blend. The medal/sprites render with
                 * SRCALPHA/INVSRCALPHA (the standard alpha blend the game uses
                 * for all sprites — the vortex itself sets exactly these at
                 * its draw). We snapshot via known defaults, NOT a GetRenderState
                 * call: D3D8 GetRenderState is vtable 0xCC, and calling the
                 * wrong slot corrupts the stack (proven — real-Windows crash at
                 * d3d8.dll on the first reveal using vtable 0x18). */
                g_blendSaveSrc = D3DBLEND_SRCALPHA;      /* 5 */
                g_blendSaveDst = D3DBLEND_INVSRCALPHA;   /* 6 */
                g_blendSaved = 1;
                g_lastBlendDev = device;
                /* additive: src=ONE, dst=ONE */
                SetRenderState(dev, D3DRS_SRCBLEND,  D3DBLEND_ONE);
                SetRenderState(dev, D3DRS_DESTBLEND, D3DBLEND_ONE);
                g_addDown = 1;
            }
        } else if (g_addDown) {
            /* leaving additive: restore exactly what the game had */
            src = g_blendSaveSrc; dst = g_blendSaveDst;
            if (!src) src = D3DBLEND_SRCALPHA;      /* 5: defensive default */
            if (!dst) dst = D3DBLEND_INVSRCALPHA;   /* 6 */
            SetRenderState(dev, D3DRS_SRCBLEND,  src);
            SetRenderState(dev, D3DRS_DESTBLEND, dst);
            g_addDown = 0;
            g_blendSaved = 0;      /* invalidate cache so a later reveal re-reads */
            g_lastBlendDev = 0;
        }
    }
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

/* ==================================================================
 * PROCEDURAL-COMPOSITE VORTEX (Option A)
 * We control every pixel, so instead of issuing raw DrawPrimitiveUP calls
 * (which crashed real Windows on a mis-slot) we generate RGBA and let the
 * game's own sprite renderer draw it:
 *   1) Once per reveal: read-capture the golden weasel's real pixels
 *      (LockRect of the weasel texture at sprite+0x50).
 *   2) Each frame: lerp the weasel toward white by reveal-frame t, and paint
 *      the suction streaks into the annulus OUTSIDE the weasel disk.
 *   3) Upload via CreateTexture+LockRect-write into a scratch texture.
 *   4) Bind to our sprite +0x50, swap into ctx+0x37C; the render fn draws it
 *      with its existing single Sprite_DrawRect. No raw D3D in the draw.
 * ================================================================== */
typedef struct { int w, h; DWORD pitch; BYTE *pBits; } Vortex_Locked;
typedef HRESULT (__stdcall *PFN_CreateTexture)(void*, UINT,UINT,UINT,DWORD,DWORD,DWORD,void**);
typedef HRESULT (__stdcall *PFN_TexLockRect)(void*, UINT, Vortex_Locked*, const RECT*, DWORD);
typedef HRESULT (__stdcall *PFN_TexUnlockRect)(void*, UINT);
typedef HRESULT (__stdcall *PFN_TexGetLevelDesc)(void*, UINT, void*);

/* Read the weasel's pixel dims from its texture via GetLevelDesc. Returns
 * something sane (w,h scaled to texture-relative coords) or (0,0). */
static void vortex_weasel_size(DWORD weaselTex, int *outW, int *outH) {
    DWORD vt;
    PFN_TexGetLevelDesc GetLevelDesc;
    unsigned char desc[0x20];
    *outW = 0; *outH = 0;
    if (!weaselTex || IsBadReadPtr((void*)weaselTex, 4)) return;
    vt = *(DWORD*)weaselTex;
    if (!vt || IsBadReadPtr((void*)(vt + D3D_TEX_GETLEVELDESC), 4)) return;
    GetLevelDesc = (PFN_TexGetLevelDesc)(*(void**)(vt + D3D_TEX_GETLEVELDESC));
    if (!GetLevelDesc) return;
    memset(desc, 0, sizeof(desc));
    /* D3DSURFACE_DESC (MinGW d3d8.h): Format(+0) Type(+4) Usage(+8) Pool(+0xC)
     * Size(+0x10) MultiSampleType(+0x14) Width(+0x18) Height(+0x1C). */
    if (GetLevelDesc((void*)weaselTex, 0, desc) == 0 /*S_OK*/) {
        int w = (int)*(DWORD*)(desc + 0x18);
        int h = (int)*(DWORD*)(desc + 0x1C);
        /* Accept any 32-bit or masked format; we read texels as 4-byte ARGB.
         * Even if the format is 16-bit, GetLevelDesc still gives us the true
         * dims (which is all we need for the in-bounds capture buffer). */
        if (w > 0 && h > 0) { *outW = w; *outH = h; }
    }
}

/* Create the scratch texture (recreated if size changed). */
static DWORD vortex_ensure_scratch(DWORD device, int w, int h) {
    DWORD vt;
    PFN_CreateTexture CreateTexture;
    if (g_vortexTex && g_vortexTexW == w && g_vortexTexH == h) return g_vortexTex;
    if (!device || IsBadReadPtr((void*)device, 4)) return 0;
    vt = *(DWORD*)device;
    if (!vt || IsBadReadPtr((void*)(vt + D3D_DEV_CREATETEXTURE), 4)) return 0;
    CreateTexture = (PFN_CreateTexture)(*(void**)(vt + D3D_DEV_CREATETEXTURE));
    if (!CreateTexture) return 0;
    if (g_vortexTex) {   /* release old scratch (Release = slot 2) */
        DWORD *rvt = *(DWORD**)g_vortexTex;
        if (rvt && IsBadReadPtr((void*)(rvt + 0x08), 4)) {
            typedef ULONG (__stdcall *PFN_Release)(void*);
            PFN_Release rel = (PFN_Release)(*(void**)(rvt + 0x08));
            if (rel) rel((void*)g_vortexTex);
        }
        g_vortexTex = 0;
    }
    {
        void *out = NULL;
        HRESULT hr = CreateTexture((void*)device, (UINT)w, (UINT)h,
                                   1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &out);
        if (hr != 0 || !out) return 0;
        g_vortexTex = (DWORD)out;
        g_vortexTexW = w; g_vortexTexH = h;
        trace_logf("[vortex] scratch %dx%d -> %08X", w, h, g_vortexTex);
    }
    return g_vortexTex;
}

/* Upload `w*h*4` RGBA into the scratch texture (LockRect write). */
static int vortex_upload(DWORD tex, int w, int h, const BYTE *rgba) {
    DWORD vt;
    PFN_TexLockRect LockRect; PFN_TexUnlockRect UnlockRect;
    Vortex_Locked lr;
    if (!tex || !rgba || IsBadReadPtr((void*)tex, 4)) return 0;
    vt = *(DWORD*)tex;
    if (!vt || IsBadReadPtr((void*)(vt + D3D_TEX_LOCKRECT), 8)) return 0;
    LockRect = (PFN_TexLockRect)(*(void**)(vt + D3D_TEX_LOCKRECT));
    UnlockRect = (PFN_TexUnlockRect)(*(void**)(vt + D3D_TEX_UNLOCKRECT));
    if (!LockRect || !UnlockRect) return 0;
    memset(&lr, 0, sizeof(lr));
    if (LockRect((void*)tex, 0, &lr, NULL, 0) != 0) return 0;
    if (lr.pBits) {
        int y; const BYTE *s = rgba; BYTE *d = (BYTE*)lr.pBits;
        for (y = 0; y < h; y++) {
            memcpy(d, s, (size_t)w * 4);
            s += (size_t)w * 4; d += lr.pitch;
        }
    }
    UnlockRect((void*)tex, 0);
    return 1;
}

/* Capture the weasel's current texture into g_vortexCaptured (RGBA, w*h*4).
 * Returns 1 on success. */
static int vortex_capture_weasel(DWORD device, DWORD weaselTex, int capW, int capH) {
    DWORD vt;
    PFN_TexLockRect LockRect; PFN_TexUnlockRect UnlockRect;
    Vortex_Locked lr;
    BYTE *buf;
    int x, y;
    if (!device || !weaselTex || !capW || !capH) return 0;
    if (capW > VORTEX_TEX_MAX || capH > VORTEX_TEX_MAX || capW < VORTEX_TEX_MIN || capH < VORTEX_TEX_MIN)
        capW = capH = 128;   /* fall back to a sane square */
    buf = (BYTE*)VirtualAlloc(NULL, (size_t)capW*capH*4, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!buf) return 0;
    if (IsBadReadPtr((void*)weaselTex, 4)) { VirtualFree(buf,0,MEM_RELEASE); return 0; }
    vt = *(DWORD*)weaselTex;
    if (!vt || IsBadReadPtr((void*)(vt + D3D_TEX_LOCKRECT), 8)) { VirtualFree(buf,0,MEM_RELEASE); return 0; }
    LockRect = (PFN_TexLockRect)(*(void**)(vt + D3D_TEX_LOCKRECT));
    UnlockRect = (PFN_TexUnlockRect)(*(void**)(vt + D3D_TEX_UNLOCKRECT));
    if (!LockRect || !UnlockRect) { VirtualFree(buf,0,MEM_RELEASE); return 0; }
    memset(&lr, 0, sizeof(lr));
    /* Read the source texture's level 0.
     *
     * FIX (B, 2026-08-17): D3DLOCK_READONLY is only valid on D3DPOOL_SYSTEMMEM
     * textures. The golden-weasel texture comes from D3DXCreateTextureFromFileEx
     * (0x476770), which creates a D3DPOOL_MANAGED texture. Passing READONLY on a
     * MANAGED texture is not universally honored and can fail or return garbage
     * on some drivers (real-Windows-only, invisible to Wine) — which would
     * capture a black/faded weasel and composite wrong pixels into the vortex.
     * Try the READONLY lock first (cheap when it works), then fall back to a
     * plain read lock (flags=0) if the driver rejects it. A MANAGED texture is
     * CPU-readable, so flags=0 is safe. */
    if (LockRect((void*)weaselTex, 0, &lr, NULL, D3DLOCK_READONLY) != 0) {
        if (LockRect((void*)weaselTex, 0, &lr, NULL, 0) != 0) {
            VirtualFree(buf,0,MEM_RELEASE); return 0;
        }
    }
    if (lr.pBits) {
        const BYTE *s = (const BYTE*)lr.pBits;
        size_t row_w = ((size_t)capW) < lr.pitch ? (size_t)capW*4 : (size_t)lr.pitch;
        int   anyAlpha = 0;
        for (y = 0; y < capH; y++) {
            /* If the source row is narrower than the pitch (format/pad), copy
             * the row's actual bytes; never read past the locked region. */
            memcpy(buf + (size_t)y*capW*4, s, row_w);
            s += lr.pitch;
        }
        /* Validate the capture actually landed: if every pixel is fully
         * transparent, the read lock failed silently (returned empty) — treat
         * as a failed capture and degrade to the safe default instead of
         * compositing transparent garbage over the weasel/near-black streaks. */
        {   const BYTE *p = buf; int n = capW*capH;
            for (x = 0; x < n; x++, p += 4) if (p[3] != 0) { anyAlpha = 1; break; }
        }
        if (!anyAlpha) { VirtualFree(buf,0,MEM_RELEASE); return 0; }
    }
    UnlockRect((void*)weaselTex, 0);
    /* store the REAL weasel pixels (canvas is derived = weasel x scale) */
    if (g_vortexCaptured) VirtualFree(g_vortexCaptured,0,MEM_RELEASE);
    g_vortexCaptured = buf;
    g_vortexWeaselW = capW; g_vortexWeaselH = capH;
    g_vortexOrigTex = weaselTex;
    trace_logf("[vortex] captured weasel %dx%d -> %08X (orig-tex %08X)",
               capW, capH, g_vortexCaptured, weaselTex);
    return 1;
}

/* Smallest power of two >= n (D3D8 texture dims must be a power of two).
 * Returns VORTEX_TEX_MIN if n <= 0. */
static int vortex_next_pot(int n) {
    unsigned v = (unsigned)(n > 0 ? n : 1);
    v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++;
    return (int)v;
}

/* Build a minimal sprite (0xD4 buffer) the renderer draws with +0x50=tex.
 * vtable 0x4D8F84 (all-menu sprite vtable) + defaults Sprite_ctor sets. */
static DWORD vortex_make_sprite(DWORD gfx, DWORD tex, float w, float h) {
    unsigned char *sp;
    if (!gfx || !tex || gfx <= 0x10000) return 0;
    sp = (unsigned char*)VirtualAlloc(NULL, 0xD4, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!sp) return 0;
    memset(sp, 0, 0xD4);
    *(DWORD*)(sp + 0x00) = 0x4D8F84;   /* vtable */
    *(DWORD*)(sp + 0x04) = gfx;        /* +0x04 gfx */
    *(float*)(sp + 0x0C) = 1.0f; *(float*)(sp + 0x10) = 1.0f;  /* diffuse */
    *(float*)(sp + 0x14) = 1.0f; *(float*)(sp + 0x18) = 1.0f;
    *(float*)(sp + 0x1C) = 1.0f; *(float*)(sp + 0x20) = 1.0f;  /* ambient */
    *(float*)(sp + 0x24) = 1.0f; *(float*)(sp + 0x28) = 1.0f;
    *(float*)(sp + 0x2C) = 1.0f; *(float*)(sp + 0x30) = 1.0f;  /* specular */
    *(float*)(sp + 0x34) = 1.0f; *(float*)(sp + 0x38) = 1.0f;
    *(float*)(sp + 0x3C) = 0.0f; *(float*)(sp + 0x40) = 0.0f;  /* emissive */
    *(volatile unsigned char*)(sp + 0x4C) = 0;                 /* blend flag */
    *(volatile unsigned char*)(sp + 0x4D) = 0;                 /* alpha flag */
    *(DWORD*)(sp + 0x50) = tex;                                /* texture */
    *(float*)(sp + 0xC8) = w;                                  /* box w */
    *(float*)(sp + 0xCC) = h;                                  /* box h */
    return (DWORD)sp;
}
/* Call vortex_make_sprite using the ORIGINAL weasel's draw box so our
 * composite renders in exactly the weasel's spot/size.
 *
 * FIX (C, 2026-08-17): derive the sprite box from the ACTUAL canvas↔weasel
 * pixel ratio, not from `weaselBox x scale`. The canvas texture (g_vortexCaptW
 * / g_vortexCaptH) is POT-rounded in vortex_start_cycle, so it can be slightly
 * larger than `weaselW x scale`. If we sized the box by `weaselBox x scale`
 * while the texture held more pixels, the renderer (which maps the whole
 * texture over the whole box) would stretch the extra padding across the
 * original footprint — drifting the centered weasel away from the anchor by
 * the padding fraction. The correct box makes `weaselW` pixels map to exactly
 * `weaselBoxW` world units, centering the weasel precisely on the original
 * footprint regardless of POT padding:
 *     box_w = captW * (weaselBoxW / weaselW)
 * and the anchor (shifted by weaselBoxW/2 in vortex_patch_anchor) then lands
 * the weasel exactly. */
static DWORD vortex_make_sprite_box(DWORD gfx, DWORD tex) {
    float w, h;
    if (g_vortexWeaselW > 0 && g_vortexCaptW > 0 && g_vortexBoxW > 0.0f)
        w = (float)g_vortexCaptW * (g_vortexBoxW / (float)g_vortexWeaselW);
    else
        w = g_vortexBoxW * VORTEX_CANVAS_SCALE;
    if (g_vortexWeaselH > 0 && g_vortexCaptH > 0 && g_vortexBoxH > 0.0f)
        h = (float)g_vortexCaptH * (g_vortexBoxH / (float)g_vortexWeaselH);
    else
        h = g_vortexBoxH * VORTEX_CANVAS_SCALE;
    if (w <= 0.0f || h <= 0.0f) {   /* fallback: canvas pixels (1px=1unit approx) */
        w = (float)g_vortexCaptW; h = (float)g_vortexCaptH;
    }
    /* Box = canvas = weasel x scale. The anchored top-left (0x208,0x63) stays,
     * and the weasel is centered in the canvas => the weasel's on-screen center
     * = anchor + box/2 = anchor + (origBox x scale)/2. With scale=2 the weasel
     * maps to the full original footprint centered at the original center: the
     * canvas grows symmetrically around the weasel, so it stays put with no
     * anchor patch. */
    return vortex_make_sprite(gfx, tex, w, h);
}

/* Composite the current reveal frame into the scratch, then ensure our
 * sprite is swapped into ctx+0x37C. white t in [0,1]. */
static void vortex_composite_render(DWORD results, float t, int frame) {
    int w, h, x, y, cx, cy, i;
    float wr, wr2;
    BYTE *buf;
    if (!g_vortexCaptured || g_vortexCaptW<=0 || g_vortexCaptH<=0) return;
    w = g_vortexCaptW; h = g_vortexCaptH;   /* canvas */
    if (!vortex_ensure_scratch(g_vortexDevice, w, h)) return;
    buf = (BYTE*)VirtualAlloc(NULL, (size_t)w*h*4, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!buf) return;
    memset(buf, 0, (size_t)w*h*4);           /* transparent canvas */
    cx = w/2; cy = h/2;
    wr  = VORTEX_TEX_WEASEL_R * (float)w;
    wr2 = wr*wr;
    /* 1) weasel, lerped to white by t, drawn 1:1 CENTERED in the canvas.
     *    offset = (canvas - weasel)/2 so the sprite box (which maps the full
     *    canvas onto the weasel's footprint) centers the weasel in place. */
    if (g_vortexWeaselW <= 0 || g_vortexWeaselH <= 0) { g_vortexWeaselW = w; g_vortexWeaselH = h; }
    if (g_vortexWeaselW > w || g_vortexWeaselH > h)   { g_vortexWeaselW = w; g_vortexWeaselH = h; }
    {
        int ox = (w - g_vortexWeaselW) / 2, oy = (h - g_vortexWeaselH) / 2;
        int y2;
        for (y2 = 0; y2 < g_vortexWeaselH; y2++) {
            int dstY = oy + y2;
            const BYTE *s = g_vortexCaptured + (size_t)y2*g_vortexWeaselW*4;
            BYTE *d = buf + ((size_t)dstY*w + ox)*4;
            for (x = 0; x < g_vortexWeaselW; x++) {
                BYTE r=s[0], g=s[1], b=s[2], a=s[3];
                if (a) {
                    int dr=(int)((255-r)*t), dg=(int)((255-g)*t), db=(int)((255-b)*t);
                    r=(BYTE)(r+dr); g=(BYTE)(g+dg); b=(BYTE)(b+db);
                }
                d[0]=r; d[1]=g; d[2]=b; d[3]=a;
                s+=4; d+=4;
            }
        }
    }
    /* 2) streaks in the annulus (outside weasel disk), alpha treated as
     *    white additive on top of the base (already-white-lerped) pixels */
    for (i = 0; i < VORTEX_MAX; i++) {
        Diamond_VortexP *p = &g_vortex[i];
        float L, ca, sa, ang;
        int seg;
        if (!p->active || p->alpha == 0) continue;
        L  = VORTEX_TEX_STRETCH * (float)w;
        ang = p->ax + p->ay * p->born;
        ca = (float)cos(ang); sa = (float)sin(ang);
        for (seg = 0; seg < VORTEX_SEGS; seg++) {
            float t0=(float)seg/(float)VORTEX_SEGS, t1=(float)(seg+1)/(float)VORTEX_SEGS;
            float r0 = p->r - L*t0;
            float r1 = p->r - L*t1;
            float mid=0.5f;
            float f0=(t0<=mid)?(t0/mid):((1.0f-t0)/(1.0f-mid));
            float f1=(t1<=mid)?(t1/mid):((1.0f-t1)/(1.0f-mid));
            int a0=(int)(p->alpha*(f0*f0)), a1=(int)(p->alpha*(f1*f1));
            float x0=(float)cx+ca*r0, y0=(float)cy+sa*r0;
            float x1=(float)cx+ca*r1, y1=(float)cy+sa*r1;
            int steps=6, k, ox, oy;
            if (r0 < wr && r1 < wr) continue;   /* fully inside weasel -> skip */
            for (k=0;k<=steps;k++){
                float tt=(float)k/(float)steps;
                float lx=x0+(x1-x0)*tt, ly=y0+(y1-y0)*tt;
                int a=(k<=steps/2)?a0:a1;
                if (a<=0) continue;
                {
                    int ix=(int)lx, iy=(int)ly;
                    for(oy=-1;oy<=1;oy++) for(ox=-1;ox<=1;ox++){
                        int px=ix+ox, py=iy+oy; BYTE *pix;
                        if(px<0||py<0||px>=w||py>=h) continue;
                        /* skip the weasel disk entirely (no streaks over it) */
                        {
                            float dx=(float)(px-cx), dy=(float)(py-cy);
                            if (dx*dx+dy*dy < wr2) continue;
                        }
                        pix = buf + ((size_t)py*w + px)*4;
                        {
                            unsigned aa=(unsigned)a;
                            pix[0]=(BYTE)(pix[0]+((255-pix[0])*aa>>8));
                            pix[1]=(BYTE)(pix[1]+((255-pix[1])*aa>>8));
                            pix[2]=(BYTE)(pix[2]+((255-pix[2])*aa>>8));
                            pix[3]=0xFF;
                        }
                    }
                }
            }
        }
    }
    /* 3) upload + bind into a sprite and swap ctx+0x37C */
    if (vortex_upload(g_vortexTex, w, h, buf)) {
        DWORD ctx = 0, gfx = 0;
        DWORD app;
        /* find gfx + ctx the same way the white-fade finds gfx */
        if (!IsBadReadPtr((void*)(results + RESULT_APP), 4)) {
            app = *(DWORD*)(results + RESULT_APP);
            if (app && !IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) {
                DWORD sp = *(DWORD*)(app + SPRITE_WEAEL_APP);
                if (sp && !IsBadReadPtr((void*)(sp + SPRITE_GFX), 4))
                    gfx = *(DWORD*)(sp + SPRITE_GFX);
            }
        }
        if (!IsBadReadPtr((void*)(results + RESULT_CTX), 4))
            ctx = *(DWORD*)(results + RESULT_CTX);
        if (gfx && ctx && ctx > 0x10000) {
            if (!g_vortexSprite)
                g_vortexSprite = vortex_make_sprite_box(gfx, g_vortexTex);
            if (!g_vortexSwapActive) {
                if (!IsBadReadPtr((void*)(ctx + CTX_WEASEL), 4)) {
                    g_vortexSwapOrig = *(DWORD*)(ctx + CTX_WEASEL);
                    g_vortexSwapCtx = ctx;
                    g_vortexSwapResults = (int)results;
                    g_vortexSwapActive = 1;
                    vortex_patch_anchor(1);   /* shift anchor so the full-size weasel stays put */
                    trace_logf("[vortex] SWAP-ARM ctx=%08X orig=%08X -> our-sprite=%08X frame=%d",
                               ctx, g_vortexSwapOrig, g_vortexSprite, frame);
                }
            }
            if (g_vortexSwapActive && (results == (DWORD)g_vortexSwapResults)) {
                if (g_vortexSprite && g_vortexSprite > 0x10000)
                    *(volatile DWORD*)(ctx + CTX_WEASEL) = g_vortexSprite;
            }
        }
    }
    VirtualFree(buf, 0, MEM_RELEASE);
}


/* Shift (or restore) the medal-draw anchor for the composite window. When
 * `on`, writes anchor = (ANCHOR_ORIG_X - origBoxW/2, ANCHOR_ORIG_Y - origBoxH/2)
 * so the full-size centered weasel lands where the original was. When off,
 * restores the original immediates (must happen BEFORE the diamond swap). */
static void vortex_patch_anchor(int on) {
    DWORD xa = EXE_BASE + (ANCHOR_PATCH_X - EXE_BASE);
    DWORD ya = EXE_BASE + (ANCHOR_PATCH_Y - EXE_BASE);
    if (g_vortexAnchorPatched == on) return;
    if (on) {
        int sx = (int)(g_vortexBoxW * 0.5f);
        int sy = (int)(g_vortexBoxH * 0.5f);
        int nx = ANCHOR_ORIG_X - sx;
        int ny = ANCHOR_ORIG_Y - sy;
        if (nx < 0) nx = 0; if (ny < 0) ny = 0;   /* defensive clamp */
        if (nx > 0x7FFFFFFF) nx = 0x7FFFFFFF;
        if (ny > 0x7F) ny = 0x7F;                  /* imm8 range */
        patch_bytes((void*)xa, &(DWORD){ (DWORD)nx }, 4);   /* X imm32 */
        patch_bytes((void*)ya, &(BYTE){ (BYTE)ny }, 1);     /* Y imm8 */
        g_vortexAnchorPatched = 1;
        trace_logf("[vortex] anchor-shift on: X %d->%d (%d) Y %d->%d (%d)",
                   ANCHOR_ORIG_X, nx, ANCHOR_ORIG_X-nx, ANCHOR_ORIG_Y, ny, ANCHOR_ORIG_Y-(int)ny);
    } else {
        patch_bytes((void*)xa, &(DWORD){ (DWORD)ANCHOR_ORIG_X }, 4);
        patch_bytes((void*)ya, &(BYTE){ (BYTE)ANCHOR_ORIG_Y }, 1);
        g_vortexAnchorPatched = 0;
        trace_logf("[vortex] anchor-shift off (restored 0x%X,0x%X)", ANCHOR_ORIG_X, ANCHOR_ORIG_Y);
    }
}

/* Disarm the composite swap: restore the original weasel sprite at ctx+0x37C
 * and free our texture/sprite/capture. Called at reveal end (+240). MUST
 * restore the anchor BEFORE the diamond trophy swap so the diamond isn't
 * shifted (the diamond draws at the same 0x208,0x63 anchor). */
static void vortex_disarm(void) {
    vortex_patch_anchor(0);   /* restore the medal draw anchor (BEFORE diamond swap) */
    if (g_vortexSwapActive && g_vortexSwapCtx && g_vortexSwapCtx > 0x10000) {
        if (!IsBadReadPtr((void*)(g_vortexSwapCtx + CTX_WEASEL), 4))
            *(volatile DWORD*)(g_vortexSwapCtx + CTX_WEASEL) = g_vortexSwapOrig;
        trace_logf("[vortex] SWAP-DISARM ctx=%08X restore=%08X", g_vortexSwapCtx, g_vortexSwapOrig);
    }
    g_vortexSwapActive = 0; g_vortexSwapCtx = 0; g_vortexSwapOrig = 0;
    g_vortexSwapResults = 0;
    if (g_vortexSprite) { VirtualFree((void*)g_vortexSprite,0,MEM_RELEASE); g_vortexSprite=0; }
    if (g_vortexTex) {
        DWORD *rvt = *(DWORD**)g_vortexTex;
        if (rvt && IsBadReadPtr((void*)(rvt + 0x08), 4)) {
            typedef ULONG (__stdcall *PFN_Release)(void*);
            PFN_Release rel = (PFN_Release)(*(void**)(rvt + 0x08));
            if (rel) rel((void*)g_vortexTex);
        }
        g_vortexTex = 0;
    }
    g_vortexTexW = g_vortexTexH = 0;
    if (g_vortexCaptured) { VirtualFree(g_vortexCaptured,0,MEM_RELEASE); g_vortexCaptured=NULL; }
    g_vortexCaptW = g_vortexCaptH = 0;
    g_vortexWeaselW = g_vortexWeaselH = 0;
    g_vortexOrigTex = 0; g_vortexDevice = 0;
}

/* Reset the vortex cycle. Called by the weasel cave at WEASEL_WHITE_START. */
static void vortex_sound_start(void);
static void vortex_sound_stop(void);
static void vortex_start_cycle(DWORD gfx, DWORD sprite, DWORD results) {
    int i;
    DWORD device = 0, weaselTex = 0;
    int capW = 0, capH = 0;
    if (!gfx) return;
    vortex_seed();
    vortex_compute_center(gfx, sprite);
    if (!IsBadReadPtr((void*)(gfx + GFX_DEV_OFFSET), 4))
        device = *(DWORD*)(gfx + GFX_DEV_OFFSET);
    if (!device) return;
    g_vortexDevice = device;
    /* grab the weasel texture from the sprite the game draws, and its real
     * pixel size (1:1 capture, always in-bounds) + its draw box. */
    if (!IsBadReadPtr((void*)(results + RESULT_APP), 4)) {
        DWORD app = *(DWORD*)(results + RESULT_APP);
        if (app && !IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) {
            DWORD sp = *(DWORD*)(app + SPRITE_WEAEL_APP);
            if (sp && !IsBadReadPtr((void*)(sp + 0x50), 4)) {
                weaselTex = *(DWORD*)(sp + 0x50);
                vortex_weasel_size(weaselTex, &capW, &capH);
            }
            if (sp && !IsBadReadPtr((void*)(sp + 0xC8), 8)) {
                float fw = *(float*)(sp + 0xC8), fh = *(float*)(sp + 0xCC);
                if (fw > 1.0f && fh > 1.0f) { g_vortexBoxW = fw; g_vortexBoxH = fh; }
            }
        }
    }
    if (capW <= 0 || capH <= 0) {   /* didn't resolve from the texture: fall back */
        capW = 128; capH = 128;
    }
    /* clamp to a sane power-of-two range */
    if (capW < VORTEX_TEX_MIN) capW = VORTEX_TEX_MIN;
    if (capH < VORTEX_TEX_MIN) capH = VORTEX_TEX_MIN;
    if (capW > VORTEX_TEX_MAX) capW = VORTEX_TEX_MAX;
    if (capH > VORTEX_TEX_MAX) capH = VORTEX_TEX_MAX;
    if (weaselTex)
        vortex_capture_weasel(device, weaselTex, capW, capH);
    else
        return;   /* no weasel texture -> can't composite; abort cycle */
    /* The composite CANVAS is the weasel x VORTEX_CANVAS_SCALE (weasel drawn
     * 1:1 centered in it). Cap at the texture max and clamp to powers of two.
     *
     * FIX (C, 2026-08-17): D3D8 textures MUST be power-of-two (except the
     * strict-NPOT support rarely present on D3D8-era GPUs). The old code only
     * clamped to [VORTEX_TEX_MIN, VORTEX_TEX_MAX]; a canvas that wasn't a
     * power of two (e.g. weaselW=180 -> canvas=360) made CreateTexture fail
     * or the driver round it up, so the canvas texture pixel count (what the
     * renderer maps over the box) disagreed with the box we derived from the
     * weasel — drifting the centered weasel off-anchor. Round W and H up to
     * the next power of two here, and let vortex_make_sprite_box derive the
     * box from the ACTUAL (POT) canvas dims so the weasel still maps 1:1 onto
     * the original footprint. */
    g_vortexCaptW = (int)((float)g_vortexWeaselW * VORTEX_CANVAS_SCALE);
    g_vortexCaptH = (int)((float)g_vortexWeaselH * VORTEX_CANVAS_SCALE);
    if (g_vortexCaptW > VORTEX_TEX_MAX) g_vortexCaptW = VORTEX_TEX_MAX;
    if (g_vortexCaptH > VORTEX_TEX_MAX) g_vortexCaptH = VORTEX_TEX_MAX;
    g_vortexCaptW = vortex_next_pot(g_vortexCaptW);
    g_vortexCaptH = vortex_next_pot(g_vortexCaptH);
    if (g_vortexCaptW < VORTEX_TEX_MIN) g_vortexCaptW = VORTEX_TEX_MIN;
    if (g_vortexCaptH < VORTEX_TEX_MIN) g_vortexCaptH = VORTEX_TEX_MIN;
    /* guard against an oversized weasel texture overflowing the 16-bit pixel
     * index used internally for the canvas buffer. */
    if (g_vortexCaptW > VORTEX_TEX_MAX) g_vortexCaptW = VORTEX_TEX_MAX;
    if (g_vortexCaptH > VORTEX_TEX_MAX) g_vortexCaptH = VORTEX_TEX_MAX;
    g_vortexFrame = 0;
    g_vortexActive = 1;
    for (i = 0; i < VORTEX_MAX; i++) {
        g_vortex[i].active = 0;
        g_vortex[i].alpha = 0;
        g_vortex[i].r = 0.0f;
        g_vortex[i].born = 0;
    }
    vortex_sound_start();   /* start the whoosh for the vortex window */
}

/* Advance one vortex streak (spiral-in + fade). Radii/stretch in TEXTURE
 * pixels (relative to captured width). Called every frame the cycle runs. */
static void vortex_update_streak(Diamond_VortexP *p, int frame) {
    float L, cf;
    if (!p->active) return;
    p->r -= p->vr;                       /* inward pull */
    L  = VORTEX_TEX_STRETCH * (float)g_vortexCaptW;
    cf = VORTEX_TEX_CENTER_FADE * (float)g_vortexCaptW;
    if (p->r - L <= 0.0f || p->r > 4000.0f) { p->active = 0; return; }
    if (p->born < 15) {
        p->alpha = (BYTE)((p->born * 17) & 0xFF);
    } else if (frame >= (int)VORTEX_FRAMES) {
        int tail_elapsed = frame - (int)VORTEX_FRAMES;
        int rem = (int)VORTEX_TAIL - tail_elapsed;
        if (rem <= 0) p->alpha = 0;
        else p->alpha = (BYTE)((rem * 255) / (int)VORTEX_TAIL);
    } else {
        p->alpha = 255;
    }
    {
        float tip = p->r - L;
        if (tip < cf) {
            float f = tip / cf;
            if (f <= 0.0f) p->alpha = 0;
            else p->alpha = (BYTE)((int)p->alpha * (f * f));
        }
    }
    p->born++;
}

/* Vortex whoosh — a looping BASS stream of the game's Whoosh.ogg that plays
 * for the vortex window. Falls back silently if real BASS is absent. */
#define VORTEX_SND_STREAM  0
static HSTREAM g_vortex_snd = VORTEX_SND_STREAM;
#define WHOOSH_SND_PATH   "sounds\\\\whoosh.ogg"
static void vortex_sound_start(void) {
    if (g_vortex_snd != VORTEX_SND_STREAM) return;
    if (!real_BASS_StreamCreateFile || !real_BASS_ChannelPlay) return;
    g_vortex_snd = real_BASS_StreamCreateFile(FALSE, WHOOSH_SND_PATH, 0, 0, 0);
    if (g_vortex_snd != VORTEX_SND_STREAM) real_BASS_ChannelPlay(g_vortex_snd, TRUE);
}
static void vortex_sound_stop(void) {
    if (g_vortex_snd == VORTEX_SND_STREAM) return;
    if (real_BASS_ChannelStop) real_BASS_ChannelStop(g_vortex_snd);
    if (real_BASS_StreamFree) real_BASS_StreamFree(g_vortex_snd);
    g_vortex_snd = VORTEX_SND_STREAM;
}

/* Main vortex tick: called from the reveal driver each frame the weasel is
 * drawn. Starts the cycle at WEASEL_WHITE_START, advances streaks + emits the
 * composite frame via ctx+0x37C swap. Clears/disarms at the reveal end.
 * results = the results-screen object. */
__attribute__((used)) void diamond_vortex_tick(DWORD results) {
    int frame, i;
    DWORD app, sprite, gfx;
    float t;
    cave_enter(results, "vortex_tick");
    if (!results) return;
    if (IsBadReadPtr((void*)(results + RESULT_FRAME), 4)) return;
    if (!diamond_first_earn(results)) {          /* replay -> no vortex */
        g_vortexResults = results;
        return;
    }
    if (results != g_vortexResults) {            /* fresh session -> teardown */
        if (g_vortexActive) { g_vortexActive = 0; vortex_sound_stop(); vortex_disarm(); }
        g_vortexResults = results;
    }
    frame = diamond_seq_frame(results);
    /* window: [WEASEL_WHITE_START, WEASEL_WHITE_START+VORTEX_FRAMES+VORTEX_TAIL) */
    if (frame >= WEASEL_WHITE_START &&
        frame <  WEASEL_WHITE_START + (int)(VORTEX_FRAMES + VORTEX_TAIL)) {
        if (!g_vortexActive) {
            if (IsBadReadPtr((void*)(results + RESULT_APP), 4)) return;
            app = *(DWORD*)(results + RESULT_APP);
            if (!app || IsBadReadPtr((void*)(app + SPRITE_WEAEL_APP), 4)) return;
            sprite = *(DWORD*)(app + SPRITE_WEAEL_APP);
            gfx = (sprite && IsBadReadPtr((void*)(sprite + SPRITE_GFX), 4)==0)
                    ? *(DWORD*)(sprite + SPRITE_GFX) : 0;
            vortex_start_cycle(gfx, sprite, results);
        }
    } else {
        if (g_vortexActive) { g_vortexActive = 0; vortex_sound_stop(); vortex_disarm(); }
        return;
    }
    if (!g_vortexActive) return;                  /* cycle didn't start */
    g_vortexFrame = frame - WEASEL_WHITE_START;
    /* spawn streaks (no new spawns in the tail) */
    for (i = 0; i < VORTEX_MAX; i++) {
        Diamond_VortexP *p = &g_vortex[i];
        if (!p->active) {
            if (g_vortexFrame >= (int)VORTEX_FRAMES) continue;
            {
                int wait = (int)(vortex_frand() * 24.0f);
                if (g_vortexFrame >= wait) {
                    p->active=1; p->born=0;
                    p->ax = vortex_frand() * 6.2832f;
                    p->ay = 0.0f;                    /* no curl */
                    /* spawn in the visible annulus just outside the weasel disk,
                     * within the canvas (max radius from center = 0.5*captW).
                     * Canvas is 2x the weasel, so the weasel disk is 0.26*captW;
                     * streaks fill 0.30..0.48 of the width and pull inward. */
                    p->r  = (0.30f + vortex_frand()*0.18f) * (float)g_vortexCaptW;
                    p->vr = (4.4f + vortex_frand()*1.6f) * (1.0f/128.0f) * (float)g_vortexCaptW;
                    p->alpha = 0;
                } else continue;
            }
        } else {
            vortex_update_streak(p, g_vortexFrame);
        }
    }
    /* composite frame: white-lerp t ramps across [WHITE_START, WHITE_END] */
    t = (float)(frame - WEASEL_WHITE_START) / (float)(WEASEL_WHITE_END - WEASEL_WHITE_START);
    if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
    vortex_composite_render(results, t, frame);
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
    cave_enter(results, "trophy_swap");
    /* Cross-screen guard: if a previous results screen armed the swap and we
     * are now on a DIFFERENT results object, restore the old ctx before doing
     * anything — the diamond sprite must not leak onto the new screen. */
    if (results != g_trophySwapResults)
        diamond_trophy_restore(results);
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
    /* One-shot diagnostic: report the decision the first time we reach the
     * threshold gate (fast time + reveal passed 240). Logs cs/thr/race so a
     * real-Windows run shows exactly why the diamond did/didn't appear. */
    {   static DWORD lastGateResults = 0;
        if (results != lastGateResults) {
            lastGateResults = results;
            trace_logf("[diamond] SWAP-GATE race=%d time=%d threshold=%d (won=%d)",
                       race, cs, thr, g_won[race]);
        }
    }
    if (!(cs > 0 && cs <= thr)) { diamond_trophy_restore(results); return 0; }
    /* Atomic unlock commit on first reveal. */
    if (!g_won[race]) {
        if (!diamond_provision_unlock(race)) {
            trace_logf("[diamond] trophy swap unlock aborted for race %d", race);
            return 0;
        }
        diamond_spawn_medal_effects(results, app);   /* pop + star ring */
    }
    /* Ensure the diamond icon is loaded, then draw it at the trophy spot.
     * FIX (2026-08-16): the ORIGINAL used the GLOBAL App manager (App+0x22C ->
     * vtable[0x58]) which is NOT the sprite manager at results-time (its
     * vtable[0x58] is 0, proven by diagnostics). The decompiled gold-medal
     * draw shows the CORRECT mechanism: the award screen draws its medals via
     *   Graphics_SetScaleAndPosition( *(*(this+0xC)) + 0x370..0x37C, x, y )
     * where *(this+0xC) is the award's DISPLAY CONTEXT whose +0x370..0x37C are
     * the bronze/silver/gold/weasel SPRITE slots, and whose vtable[0x58] is the
     * AddSprite (FUN_004752F0) that populates a slot. So the correct this for
     * AddSprite is ctx = *(results+0xC), diamond slot = ctx+0x380. */
    /* CORRECTED (2026-08-16, definitive): AddSprite (FUN_004752F0) only creates a
     * 0x48-byte menu-list entry with a filename — NO texture, so it draws
     * nothing. The proper drawable is created by Sprite_ctor (0x45D0C0), which
     * builds the full 0xD4 Sprite AND loads the texture via FUN_00455c50 into
     * sprite+0x50, sets width(+200)/height(+0xCC). gfx for ctor = App+0x174.
     * Reuse a persistent diamond sprite (created once, drawn each reveal frame). */
    if (!g_diamondSprite) {
        DWORD gfx = 0, app2 = (DWORD)get_app();
        if (app2 && !IsBadReadPtr((void*)(app2 + APP_GFX), 4)) gfx = *(DWORD*)(app2 + APP_GFX);
        if (gfx && gfx > 0x10000 && !IsBadReadPtr((void*)gfx, 4)) {
            unsigned char *sp = (unsigned char*)VirtualAlloc(NULL, 0xD4, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
            if (sp) {
                memset(sp, 0, 0xD4);
                __asm__ volatile(
                    "pushl %2\n\t"        /* filename */
                    "pushl %1\n\t"        /* gfx */
                    "movl %0, %%ecx\n\t"  /* sprite */
                    "call *%3\n\t"        /* Sprite_ctor __thiscall RET 0x8? callee cleans 2 */
                    : : "r"(sp), "r"(gfx), "r"(g_iconFile), "r"(0x45D0C0)
                    : "eax", "ecx", "edx", "memory");
                g_diamondSprite = (DWORD)sp;
                g_iconLoaded = 1;
            }
        }
        diag_logf("[diamond] sprite_ctor gfx=%08X -> g_diamondSprite=%08X loaded=%d",
                  gfx, g_diamondSprite, g_iconLoaded);
    }
    if (!g_diamondSprite) return 0;                  /* not loadable -> gold stays */
    /* REDIRECT the game's own renderer instead of drawing here.
     * The golden-weasel medal is composited by the award RENDER fn (slot[2]=
     * 0x44DF70) at 0x44E12C via `mov ecx,[ctx+0x37C]; Sprite_DrawRect(sp,0x208,
     * 0x63)` where ctx = *(results+0xC). Drawing the diamond HERE (from the
     * UPDATE host) does not composite — the frame draws in the render fn.
     * So swap ctx+0x37C (golden-weasel sprite slot) to the diamond sprite;
     * the game's own render draw then composites the diamond at the weasel
     * spot. Restore is handled by diamond_trophy_restore() on frames where the
     * reveal is no longer active. ctx lives past the update frame (it is the
     * results screen's display context), so a swap here is seen by the render. */
    {
        DWORD ctx = 0;
        if (!IsBadReadPtr((void*)(results + RESULT_CTX), 4)) ctx = *(DWORD*)(results + RESULT_CTX);
        if (ctx && ctx > 0x10000 && !IsBadReadPtr((void*)(ctx + CTX_WEASEL), 4)) {
            if (!g_trophySwapActive) {
                g_trophySwapOrig = *(DWORD*)(ctx + CTX_WEASEL);
                g_trophySwapCtx  = ctx;
                g_trophySwapResults = results;
                g_trophySwapActive = 1;
                trace_logf("[diamond] TROPHY-SWAP arm ctx=%08X orig=%08X -> diamond=%08X results=%08X",
                           ctx, g_trophySwapOrig, g_diamondSprite, results);
            }
            *(volatile DWORD*)(ctx + CTX_WEASEL) = g_diamondSprite;
        }
    }
    return 1;                                          /* gold is suppressed by the swap */
}

/* Restore the game's golden-weasel sprite pointer at ctx+0x37C when the
 * reveal is no longer active. Safe to call from the update host on any frame;
 * does nothing if no swap is armed. Prevents the diamond sprite from leaking
 * onto the next results screen / other medal rows. */
__attribute__((used)) void diamond_trophy_restore(DWORD results) {
    (void)results;
    if (!g_trophySwapActive) return;
    if (g_trophySwapCtx && g_trophySwapCtx > 0x10000 &&
        !IsBadReadPtr((void*)(g_trophySwapCtx + CTX_WEASEL), 4)) {
        *(volatile DWORD*)(g_trophySwapCtx + CTX_WEASEL) = g_trophySwapOrig;
    }
    trace_logf("[diamond] TROPHY-SWAP disarm ctx=%08X restore=%08X",
               g_trophySwapCtx, g_trophySwapOrig);
    g_trophySwapCtx = 0; g_trophySwapOrig = 0;
    g_trophySwapResults = 0; g_trophySwapActive = 0;
}
__attribute__((used)) void diamond_load_icon_impl(DWORD app) {
    DWORD mgr, vt, load;
    diag_logf("[diamond] load_icon ENTER app=%08X iconLoaded=%d config=%d", app, g_iconLoaded, g_configLoaded);
    if (g_iconLoaded) { diag_log("[diamond] load_icon: already loaded"); return; }
    if (!app || !g_configLoaded) { diag_logf("[diamond] load_icon: bail app/config (app=%08X config=%d)", app, g_configLoaded); return; }
    /* The diamond PNG is loaded through the game's NORMAL file path (it was
     * written to Textures\ on first unlock). No manual texture building. */
    mgr = *(DWORD*)(app + APP_MGR);
    diag_logf("[diamond] load_icon: mgr=%08X", mgr);
    if (!mgr) { diag_log("[diamond] load_icon: mgr=0"); return; }
    if (IsBadReadPtr((void*)mgr, 4)) { diag_log("[diamond] load_icon: mgr unreadable"); return; }
    vt = *(DWORD*)mgr;
    if (!vt) { diag_log("[diamond] load_icon: vt=0"); return; }
    if (IsBadReadPtr((void*)(vt + 0x58), 4)) { diag_log("[diamond] load_icon: vt+0x58 unreadable"); return; }
    load = *(DWORD*)(vt + 0x58);
    diag_logf("[diamond] load_icon: vt=%08X slots[48]=%08X [4C]=%08X [50]=%08X [54]=%08X [58]=%08X [5C]=%08X",
              vt, *(DWORD*)(vt+0x48), *(DWORD*)(vt+0x4C), *(DWORD*)(vt+0x50), *(DWORD*)(vt+0x54),
              *(DWORD*)(vt+0x58), *(DWORD*)(vt+0x5C));
    if (!load) { diag_log("[diamond] load_icon: load=0"); return; }
    /* __thiscall: ecx=mgr, push <str> FIRST, then push &slot, call [vt+0x58].
         * Native call site (0x42a2f8): push $0x4d31c0 (str); push %edx (&slot);
         * call *0x58(%eax). The loader is __stdcall `ret $8` (cleans its own two
         * args) — its caller does NOT `add esp,8` afterwards. */
        diag_logf("[diamond] load_icon: mgr=%08X vt=%08X load=%08X file=%s config=%d",
                  mgr, vt, load, g_iconFile, g_configLoaded);
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
    /* Pre-load the TT-menu MINI icon at the same safe time (same manager
     * context, NOT during TT render). This is the Option-B fix: the TT
     * append handler must NOT lazily re-enter the sprite loader during the
     * TT standings render (that re-entrancy is the historical TT-crash). */
    if (!g_miniIconLoaded)
        diamond_load_mini_icon_impl(app);
}
__attribute__((used)) void diamond_load_mini_icon_impl(DWORD app) {
    /* FIX (2026-08-16): the ORIGINAL used the vtable[0x58] loader via App+0x22C,
     * which is NOT a valid sprite manager (returns null / garbage at runtime).
     * The PROVEN correct constructor is Sprite_ctor (0x45D0C0) — it builds a
     * full 0xD4 Sprite AND loads the texture via FUN_00455c50 into sprite+0x50,
     * sets width/height. gfx = App+0x174. This is the same fix that made the
     * big diamond sprite load (g_diamondSprite=025B0000). */
    DWORD gfx = 0;
    unsigned char *sp;
    if (g_miniIconLoaded) return;
    if (!app || !g_configLoaded) return;
    if (!IsBadReadPtr((void*)(app + APP_GFX), 4)) gfx = *(DWORD*)(app + APP_GFX);
    if (!gfx || gfx <= 0x10000 || IsBadReadPtr((void*)gfx, 4)) return;
    sp = (unsigned char*)VirtualAlloc(NULL, 0xD4, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!sp) return;
    memset(sp, 0, 0xD4);
    __asm__ volatile(
        "pushl %2\n\t"        /* filename */
        "pushl %1\n\t"        /* gfx */
        "movl %0, %%ecx\n\t"  /* sprite */
        "call *%3\n\t"        /* Sprite_ctor __thiscall (ret 8) */
        : : "r"(sp), "r"(gfx), "r"(g_miniIconFile), "r"(0x45D0C0)
        : "eax", "ecx", "edx", "memory");
    if (sp[0] == 0 && sp[1] == 0) {   /* vtable not set -> ctor may have bailed */
        VirtualFree(sp, 0, MEM_RELEASE);
        return;
    }
    g_diamondMiniSprite = (DWORD)sp;
    g_miniIconLoaded = 1;
    diag_logf("[diamond] mini icon loaded: %s -> %08X (gfx=%08X)", g_miniIconFile, g_diamondMiniSprite, gfx);
}

/* Called from the TT-ctor-clone epilogue AFTER the medal-append loop. Logs
 * whether the diamond append actually fired (g_ttAppendCount) and the mini
 * sprite state — the definitive check that the mini icon is being appended. */
__attribute__((used)) void diamond_tt_post_log(void) {
    diag_logf("[diamond] TT append: count=%d miniSprite=%08X miniLoaded=%d",
              g_ttAppendCount, g_diamondMiniSprite, g_miniIconLoaded);
}

/* TT-menu: append a diamond medal entry to the standings list.
 * Called from the TT cave. standings = the standings screen object (esi),
 * race = the loop counter edi, which is 1-INDEXED into race (edi=0 shows
 * race 1 BEGINNER; edi=13 shows race 14 IMPOSSIBLE; edi=14 indexes the
 * free App tail 0x8F8 = phantom all-zero flags, so it must be skipped).
 * 0x44abf0 is __stdcall(ecx=this, name, sprite) with ret 8.
 */
__attribute__((used)) void diamond_tt_append(DWORD standings, int race1) {
    static char namebuf[16];
    /* race1 = the game's 1-indexed tournament slot (the cave passes edi at the
     * 0x42F927 loop, which iterates game slots 0..14, i.e. edi is the slot as
     * shown in the standings = 1-indexed race). The standings-name key 0x44abf0
     * matches uses the SAME 1-indexed slot as the game's own "%d" entry, so we
     * format the name with race1. Our g_won[] array is 0-indexed, so guard it
     * with race1-1. */
    int r0 = race1 - 1;
    if (race1 < 1 || r0 > 14) return;
    if (!g_won[r0]) return;
    /* Option-B: NO lazy-load here. The mini icon is pre-loaded at the safe
     * time (diamond_load_icon_impl on the results draw). Re-entering the
     * sprite loader during the TT standings render is the historical crash. */
    if (!g_diamondMiniSprite) {
        trace_logf("[diamond] TT diamond SKIPPED race %d (mini sprite null)", race1);
        return;
    }
    /* format a distinct name "%dD" for the diamond entry (must differ from
     * the weasel's "%d" so 0x44abf0 creates a NEW list entry) — use the
     * 1-indexed slot so it keys to the same race row as the game's "%d". */
    sprintf(namebuf, g_fmtDiamond, race1);
    /* 0x44abf0(ecx=standings, arg1=name, arg2=sprite) __stdcall ret 8 */
    __asm__ volatile(
        "pushl %1\n\t"        /* sprite (arg2) */
        "pushl %0\n\t"        /* name (arg1) */
        "movl %2, %%ecx\n\t"  /* standings */
        "call *%3\n\t"
        : : "r"(namebuf), "r"(g_diamondMiniSprite), "r"(standings), "r"(ABF0_APPEND)
        : "eax", "ecx", "edx", "memory"
    );
    trace_logf("[diamond] TT diamond appended for race %d", race1);
}

/* ================================================================
 * Present-hook reveal driver
 * ================================================================
 * The white-out + vortex + diamond trophy-swap used to run from an INLINE
 * cave at 0x44E139 (the game's own golden-weasel draw call site), which
 * re-emitted the gold draw (call 0x42c7c0) and re-entered the sprite loader
 * from inside the game's Draw loop. That re-entrancy crashed on real Windows
 * (Wine tolerates it). The whole reveal now runs from the GameUpdate frame
 * epilogue hook (0x46C1F1), which fires ONCE PER FRAME at a safe boundary —
 * after all game logic + rendering, immediately before the function returns
 * and the frame is presented. The
 * game's own weasel draw at 0x44E139 is left 100% original; we draw the
 * white-out weasel and the diamond ON TOP of it from present-time, so the
 * layering is: gold (native) -> white weasel (55..240) -> diamond (240+).
 */

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
 *   before it is drawn, from diamond_trophy_swap() (line ~1290), which calls
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
/* ================================================================
 * FULL-WRAPPER TT ctor clone (DIAMOND_TT_WRAPPER) — Option 3
 * ================================================================
 * The 0x42F927 detour is PROVEN structurally impossible on real Windows (a
 * minimal 12-byte re-emit probe crashed identically). Option 3 replaces the
 * WHOLE TimeTrialMenu_ctor (0x42F810) with a byte-exact clone in a VirtualAlloc
 * RWX stub that keeps the SAME SEH prologue/setup/epilogue and the SAME 4-medal
 * append loop, plus a 5th in-flow diamond append. The clone is a COMPLETE
 * function (own valid FS:[0] set/teardown; no mod-C running inside the game's
 * frame), so it avoids the crash class. See emit_tt_clone below.
 */
#define TT_CTOR_ORIG     0x42F810   /* TimeTrialMenu_ctor entry (JMP target) */
#define TT_CTOR_PRACTICE 0x42EA30   /* PracticeMenu_ctor (parent, __thiscall) */
#define TT_CTOR_ABF0     0x44ABF0   /* Scene_AddTextureToList (__thiscall ret 8) */
#define TT_CTOR_STRFMT   0x466C70   /* AthenaString_Format */
#define TT_CTOR_STRBUF   0x4F7448   /* AthenaString object */
#define TT_CTOR_FMT      0x4D03F8   /* "%d" format */

/* Emit one in-flow medal append block (exact native byte pattern). The caller
 * provides them; returns bytes written. flag_idx = 0..3 for bronze/silver/gold/
 * weasel. sprite_slot = App+0x380/384/388/38C. */
static unsigned emit_tt_block(unsigned char *p, int flag_idx, DWORD sprite_slot) {
    unsigned char *start = p;
    p[0]=0x8B; p[1]=0x86; *(DWORD*)(p+2)=0x878;      p+=6; /* mov 0x878(%esi),%eax */
    p[0]=0x8A; p[1]=0x8C; p[2]=0xB8;
    *(DWORD*)(p+3)=(0x8C0+flag_idx);                  p+=7; /* mov 0x8C?(%eax,%edi,4),%cl */
    p[0]=0x84; p[1]=0xC9;                             p+=2; /* test %cl,%cl */
    p[0]=0x74; p[1]=0x00;                             p+=2; /* je skip (patched) */
    p[0]=0x8B; p[1]=0x88; *(DWORD*)(p+2)=sprite_slot; p+=6; /* mov 0x38?(%eax),%ecx */
    p[0]=0x51; p+=1;                                       /* push %ecx */
    p[0]=0x57; p+=1;                                       /* push %edi */
    p[0]=0x68; *(DWORD*)(p+1)=TT_CTOR_FMT;        p+=5;    /* push $FMT */
    p[0]=0x68; *(DWORD*)(p+1)=TT_CTOR_STRBUF;     p+=5;    /* push $STRBUF */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(TT_CTOR_STRFMT)-(DWORD)(p+5); p+=5; /* call STRFORMAT */
    p[0]=0x83; p[1]=0xC4; p[2]=0x0C;              p+=3;    /* add $0xc,%esp */
    p[0]=0x50; p+=1;                                       /* push %eax */
    p[0]=0x8B; p[1]=0xCE; p+=2;                            /* mov %esi,%ecx */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(TT_CTOR_ABF0)-(DWORD)(p+5); p+=5; /* call ABF0 */
    /* skip: patch the je displacement. Block layout (offsets from start):
     *   0-5  8B 86 78 08 00 00   mov 0x878(%esi),%eax      (6)
     *   6-12 8A 8C B8 <c0+idx>   mov 0x8C?(%eax,%edi,4),%cl (7)
     *   13-14 84 C9              test %cl,%cl              (2)
     *   15-16 74 ??              je                        (2) disp@start[16]
     *   17+  mov 0x38?(%eax),%ecx ... call ABF0
     * je is a 2-byte instr spanning [15..16]; rel8 displacement is byte[16],
     * and is measured from the END of the je (offset 17). So disp = here-17.
     * (Writing start[9] here corrupted the flag-address immediate and left the
     * real disp at 0 -> je never taken -> all medals appended unconditionally.)
     */
    start[16] = (unsigned char)((p-(start+17)) & 0xFF);
    return (unsigned)(p - start);
}

/* Emit the full clone (SEH prologue + setup + 4-medal loop + diamond 5th +
 * epilogue) into b. Returns size. */
static unsigned emit_tt_clone(unsigned char *b) {
    unsigned char *p = b;
    p[0]=0x6A; p[1]=0xFF;                                  p+=2; /* push -1 */
    p[0]=0x68; *(DWORD*)(p+1)=0x4CB378;                    p+=5; /* push sehscope */
    p[0]=0x64; p[1]=0xA1; p[2]=0;p[3]=0;p[4]=0;p[5]=0;      p+=6; /* mov eax,fs:[0] */
    p[0]=0x50; p+=1;                                              /* push eax */
    p[0]=0x64; p[1]=0x89; p[2]=0x25; p[3]=0;p[4]=0;p[5]=0;p[6]=0; p+=7; /* mov fs:[0],esp */
    p[0]=0x51; p+=1;                                              /* push ecx */
    p[0]=0x8B; p[1]=0x44; p[2]=0x24; p[3]=0x14; p+=4;             /* mov eax,[esp+0x14] */
    p[0]=0x56; p+=1;                                              /* push esi */
    p[0]=0x57; p+=1;                                              /* push edi */
    p[0]=0x8B; p[1]=0xF1; p+=2;                                  /* mov esi,ecx */
    p[0]=0x50; p+=1;                                              /* push eax */
    p[0]=0x89; p[1]=0x74; p[2]=0x24; p[3]=0x0C; p+=4;             /* mov [esp+0xc],esi */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(TT_CTOR_PRACTICE)-(DWORD)(p+5); p+=5; /* call PracticeMenu_ctor */
    /* On the MAIN thread at TT-open, build the diamond MINI sprite via
     * diamond_load_mini_icon_impl (now Sprite_ctor-based). This was the missing
     * piece: the earlier in-flow load crashed because it used the broken
     * vtable[0x58] loader; with Sprite_ctor (0x45D0C0) it now builds a textured
     * sprite in-flow, so the diamond append block below finds g_diamondMiniSprite
     * non-null. push App ; call diamond_load_mini_icon_impl ; add esp,4 */
    p[0]=0xB8; *(DWORD*)(p+1)=APP_PTR; p+=5;                     /* mov eax,<APP_PTR> */
    p[0]=0x8B; p[1]=0x00; p+=2;                                  /* mov eax,[eax] = App */
    p[0]=0x50; p+=1;                                             /* push App */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(diamond_load_mini_icon_impl)-(DWORD)(p+5); p+=5; /* call */
    p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;                       /* add esp,4 */
    p[0]=0xC7; p[1]=0x44; p[2]=0x24; p[3]=0x14; *(DWORD*)(p+4)=0; p+=8; /* movl $0,0x14(%esp) */
    p[0]=0xC7; p[1]=0x06; *(DWORD*)(p+2)=0x4D4670;         p+=6;  /* movl vt1,(%esi) */
    p[0]=0xC7; p[1]=0x86; *(DWORD*)(p+2)=0x868; *(DWORD*)(p+6)=0x4D4660; p+=10;
    p[0]=0xC7; p[1]=0x86; *(DWORD*)(p+2)=0x888; *(DWORD*)(p+6)=0x4D4644; p+=10;
    p[0]=0x33; p[1]=0xFF; p+=2;                                   /* xor edi,edi */
    unsigned char *loop_top = p;
    p += emit_tt_block(p, 0, 0x380);  /* bronze */
    p += emit_tt_block(p, 1, 0x384);  /* silver */
    p += emit_tt_block(p, 2, 0x388);  /* gold */
    p += emit_tt_block(p, 3, 0x38C);  /* weasel */
    /* DIAMOND 5th: read g_won (absolute) indexed by edi, append mini sprite.
     *   mov eax,<g_won_base>
     *   movzx ecx,byte [eax+edi]
     *   test cl,cl ; je skip
     *   push edi ; push FMT ; push STRBUF ; call STRFORMAT ; add esp,0xc
     *   push eax(name) ; mov ecx,<mini> ; push ecx ; mov ecx,esi ; call ABF0
     * skip:
     */
    {
        unsigned char *ds = p;
        /* DIAMOND 5th: read g_won (absolute) indexed by RACE. The standings loop
         * iterates edi=0..14 and each edi DISPLAYS race edi+1 (1-indexed, the
         * loop's App+0x8C0+edi*4 flags are race edi+1; edi=0 shows race 1
         * BEGINNER, edi=14 shows the free tail). So a diamond at display index
         * edi corresponds to g_won[edi+1], NOT g_won[edi]. We therefore index
         * g_won + edi + 1 (g_won[15] past the 15-byte array is the native
         * phantom-tail slot, all-zero, so never triggers). Registers we clobber
         * (edx, eax, ecx) are freely reusable — the diamond block is LAST per
         * loop iteration, and the loop only carries edi/esi across iterations.
         * Push order for f(name,sprite): sprite pushed FIRST (deepest), name
         * LAST (topmost) so [esp]=name,[esp+4]=sprite. */
        p[0]=0xBA; *(DWORD*)(p+1)=(DWORD)g_won; p+=5;   /* mov edx,<g_won> (BA imm32) — NOT lea! 0x8D 0x97 was lea edx,[edi+g_won], adding edi into the address -> read g_won[2*edi] ('reads two bytes'). */
        /* FLAG INDEX FIX: use edi (NOT edi+1). The standings rows are shown in
         * the loop order edi=0..13 and each row displays game race edi+1; g_won
         * is mod-0-indexed (get_race_index subtracts 1 from the game slot), so
         * race (edi+1) earned = g_won[edi]. Using g_won[edi+1] placed Odd's
         * diamond (g_won[8]) one row early at row 7 (which shows Tower). */
        p[0]=0x8B; p[1]=0xCF; p+=2;                               /* mov ecx,edi */
        p[0]=0x0F; p[1]=0xB6; p[2]=0x04; p[3]=0x0A; p+=4;         /* movzx eax,byte[edx+ecx] */
        p[0]=0x84; p[1]=0xC0; p+=2;                               /* test al,al */
        p[0]=0x74; p[1]=0x00; p+=2;                               /* je1 skip */
        /* push edi+1 as the "%d" value (race label matches the game's row name
         * so 0x44ABF0 finds the race's medal row and appends the diamond into
         * it — rendering it in-flow after the weasel. USING "%dD" FAILED: rows
         * are named "%d" (e.g. "8"), so "__stricmp(row->name, "8D")" matched
         * nothing and the append silently no-op'd (count bumped but no icon). */
        /* KEY FIX: the name key must be EDI (the loop index 0-14, matching how
         * the GAME names the standings rows). Native appends `push %edi` then
         * formats "%d" -> row names are the loop index "0".."14". Using edi+1
         * was OFF BY ONE: for edi=7 the row is "7" but we pushed "8" -> no
         * __stricmp match -> silent no-op (count bumped, no icon). */
        p[0]=0x57; p+=1;                                          /* push edi (=row name) */
        p[0]=0x68; *(DWORD*)(p+1)=TT_CTOR_FMT; p+=5;              /* push "%d" fmt */
        p[0]=0x68; *(DWORD*)(p+1)=TT_CTOR_STRBUF;  p+=5;
        p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(TT_CTOR_STRFMT)-(DWORD)(p+5); p+=5;
        p[0]=0x83; p[1]=0xC4; p[2]=0x0C; p+=3;
        /* eax = name (from format). Now push sprite, then name LAST. */
        /* load mini sprite into ecx, push as arg2 (first C arg, deepest) */
        p[0]=0xB9; *(DWORD*)(p+1)=(DWORD)&g_diamondMiniSprite; p+=5;/* mov ecx,&mini */
        p[0]=0x8B; p[1]=0x09; p+=2;                               /* mov ecx,[ecx] */
        p[0]=0x85; p[1]=0xC9; p+=2;                               /* test ecx,ecx */
        p[0]=0x74; p[1]=0x00; p+=2;                               /* je2 skip (null) */
        p[0]=0x51; p+=1;                                          /* push sprite (deepest) */
        p[0]=0x50; p+=1;                                          /* push eax=name (top) */
        /* bump g_ttAppendCount so we can confirm the append fired (diagnostic) */
        p[0]=0xB8; *(DWORD*)(p+1)=(DWORD)&g_ttAppendCount; p+=5;  /* mov eax,&g_ttAppendCount */
        p[0]=0xFF; p[1]=0x00; p+=2;                               /* inc dword[&count] */
        /* now [esp]=name,[esp+4]=sprite, ecx held 0? -> set ecx=this */
        p[0]=0x8B; p[1]=0xCE; p+=2;                               /* mov ecx,esi (this) */
        p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(TT_CTOR_ABF0)-(DWORD)(p+5); p+=5; /* call ABF0(ret 8) */
        /* skip: patch the TWO branch rel8s. Layout from ds (mov edx,<g_won>):
         * ds+0:  BA imm32 (5) mov edx,<g_won>
         * ds+5:  8B CF (2) mov ecx,edi
         * ds+7:  0F B6 04 0A (4) movzx eax,[edx+ecx]
         * ds+11: 84 C0 (2) test al,al
         * ds+13: 74 ?? (2) je1 disp@ds+14
         * ds+15: 57; ds+16:68(5); ds+21:68(5); ds+26:E8(5); ds+31:83C40C(3)
         * ds+34: B9 imm32(5) mov ecx,&mini; ds+39:8B09; ds+41:85C9
         * ds+43: 74 ?? (2) je2 disp@ds+44
         * ds+45: 51; ds+46:50; ds+47:B8(5); ds+52:FF00(2); ds+54:8BCE(2); ds+56:E8(5) call
         * skip (here) = ds+61. rel8 disp at byte X = here-(X+1).
         */
        ds[14] = (unsigned char)((p-(ds+15)) & 0xFF);
        ds[44] = (unsigned char)((p-(ds+45)) & 0xFF);
    }
    /* inc edi; cmp $0xf,edi; jl loop_top */
    p[0]=0x47; p+=1;
    p[0]=0x83; p[1]=0xFF; p[2]=0x0F; p+=3;
    p[0]=0x0F; p[1]=0x8C; *(DWORD*)(p+2)=(DWORD)(loop_top-(p+6)); p+=6; /* jl rel32 */
    /* diagnostic: call diamond_tt_post_log() once per TT construction to log
     * whether the diamond append fired. __cdecl, no args. */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(diamond_tt_post_log)-(DWORD)(p+5); p+=5;
    /* epilogue */
    p[0]=0x8B; p[1]=0x4C; p[2]=0x24; p[3]=0x0C; p+=4;             /* mov ecx,[esp+0xc] */
    p[0]=0x5F; p+=1;
    p[0]=0x8B; p[1]=0xC6; p+=2;
    p[0]=0x5E; p+=1;
    p[0]=0x64; p[1]=0x89; p[2]=0x0D; p[3]=0;p[4]=0;p[5]=0;p[6]=0; p+=7; /* mov fs:[0],ecx */
    p[0]=0x83; p[1]=0xC4; p[2]=0x10; p+=3;
    p[0]=0xC2; p[1]=0x04; p[2]=0x00; p+=3;
    return (unsigned)(p - b);
}

/* Main-thread PIGGYBACK: load the diamond MINI sprite where the game loads its
 * OWN TT icons (goldenweasel-icon at 0x42A2F8-0x42A304, vtable[0x58]) — the
 * proven-safe main-thread App-init icon-load context. This is the correct place
 * to load g_diamondMiniSprite (the init-thread preload has a valid manager but
 * still returns null, and the results-draw path only fires after racing).
 *
 * Patch site: 0x42A307 = `mov 0x22c(%esi),%ecx` (8B 8E 2C 02 00 00, 6 bytes),
 * immediately after the game loads goldenweasel-icon into App+0x38C.
 * Cave (heap RWX) body:
 *     pushad
 *     mov  ecx,[esi+0x22c]      ; manager (mgr)
 *     test ecx,ecx ; jz done
 *     push <&g_diamondMiniSprite>
 *     push <g_miniIconFile>     ; "diamondweasel-icon.png" (str pushed FIRST)
 *     mov  eax,[ecx]
 *     call *0x58(%eax)          ; loader (ret 8) — same as the game uses
 * done:
 *     popad
 *     mov  ecx,[esi+0x22c]      ; re-emit original
 *     jmp  0x42A30D
 * Runs only on the main thread during App-init icon load. This is a SEPARATE
 * call at a natural boundary (NOT re-entering a loader from inside its own
 * frame), so it avoids the historical startup-icone crash class.
 */
static void install_tt_piggyback(void) {
    if (g_miniIconLoaded) return;               /* already loaded (results-draw) */
    if (!g_anyDiamond) return;
    DWORD patchAddr = EXE_BASE + 0x2A307;      /* 0x42A307 */
    DWORD retAddr   = EXE_BASE + 0x2A30D;      /* 0x42A30D */
    unsigned char *cave = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE,
                                                       PAGE_EXECUTE_READWRITE);
    if (!cave) return;
    unsigned char *p = cave;
    p[0]=0x60; p+=1;                                             /* pushad */
    p[0]=0x8B; p[1]=0x8E; *(DWORD*)(p+2)=0x22C; p+=6;            /* mov ecx,[esi+0x22c] */
    p[0]=0x85; p[1]=0xC9; p+=2;                                  /* test ecx,ecx */
    p[0]=0x74; p[1]=0x1E; p+=2;                                  /* jz done (dispatch below) */
    p[0]=0x68; *(DWORD*)(p+1)=(DWORD)&g_diamondMiniSprite; p+=5; /* push &slot */
    p[0]=0x68; *(DWORD*)(p+1)=(DWORD)g_miniIconFile; p+=5;       /* push str */
    p[0]=0x8B; p[1]=0x01; p+=2;                                  /* mov eax,[ecx] */
    p[0]=0xFF; p[1]=0x50; p[2]=0x58; p+=3;                       /* call *0x58(%eax) */
    /* done: */
    p[0]=0x61; p+=1;                                             /* popad */
    p[0]=0x8B; p[1]=0x8E; *(DWORD*)(p+2)=0x22C; p+=6;            /* mov ecx,[esi+0x22c] (orig) */
    write_jmp(p, retAddr); p+=5;                                 /* jmp 0x42A30D */
    /* jz displacement: the jz at cave+8 (2B), target = the "done:" label = here after the call block.
     * done offset: cave[0]=pushad(1) cave[1..6]=mov(6) cave[7..8]=test(2) cave[9..10]=jz(2)
     * push&slot(5)=11..15 push str(5)=16..20 mov eax(2)=21..22 call(3)=23..25
     * done label = cave+26. jz at cave+9, disp = (cave+26)-(cave+11) = 15. */
    cave[10] = 15;
    unsigned char patch[6];
    memset(patch, 0x90, 6);
    write_jmp(patch, (DWORD)cave);
    patch_bytes((void*)patchAddr, patch, 6);
    diag_log("[diamond] TT PIGGYBACK: diamond mini icon load hooked at 0x42A307 (main-thread gold-icon context)");
}

static void install_tt_wrapper(void) {
#ifdef DIAMOND_TT_WRAPPER
    unsigned char *stub = (unsigned char*)VirtualAlloc(NULL, 1024, MEM_COMMIT|MEM_RESERVE,
                                                       PAGE_EXECUTE_READWRITE);
    if (!stub) { diag_log("[diamond] TT WRAPPER: VirtualAlloc failed"); return; }
    unsigned sz = emit_tt_clone(stub);
    FlushInstructionCache(GetCurrentProcess(), stub, sz);
    unsigned char jmp[5] = {0xE9,0,0,0,0};
    DWORD tar = (DWORD)stub;
    *(DWORD*)(jmp+1) = tar - (EXE_BASE + (TT_CTOR_ORIG - EXE_BASE)) - 5;
    patch_bytes((void*)(EXE_BASE + (TT_CTOR_ORIG - EXE_BASE)), jmp, 5);
    diag_logf("[diamond] TT WRAPPER: TimeTrialMenu_ctor cloned+patched (emit %u bytes)", sz);
    diag_logf("[diamond] TT WRAPPER: g_won_base=%p g_diamondMiniSprite=%08X (0=loads lazily on first TT draw)",
              (void*)g_won, g_diamondMiniSprite);
    /* PRELOAD the mini sprite HERE at startup if any diamond is earned, so it
     * is ready whenever the TT menu opens — regardless of whether a race was
     * completed this session (the results-draw preload only fires after racing,
     * which left it 0 and hid every diamond). The sprite manager (App+0x22C)
     * is valid after init; diamond_load_mini_icon_impl guards all pointers.
     * REMOVED 2026-08-16: calling the sprite loader from the init thread returns
     * null (wrong context), and from the TT-ctor it crashes. The loader only
     * works in the results-screen draw context. The mini sprite is therefore
     * loaded only on the results-draw (diamond_load_icon_impl). */
#else
    (void)0;
#endif
}

static void install_tt_cave(void) {
    /* PROBE ISOLATION (DIAMOND_TT_PROBE): minimal cave that does ONLY the bare
     * re-emit the game itself does (mov ecx,esi; call 0x44abf0) then jmp back
     * to 0x42F92C — NO diamond handler, NO pushad, NO extra calls. Build:
     * -DDIAMOND_VTABLE_OVERRIDE -DVORTEX_OFF -DDIAMOND_TT_PROBE.
     * This isolates whether ANY 5-byte detour from inside this SEH frame
     * (TimeTrialMenu_ctor) crashes real Windows, independent of handler
     * content. If this crashes: the detour architecture itself is the problem
     * and no in-flow patch here can work. If it doesn't: the crash is in
     * diamond_tt_append and we hunt that. */
#ifdef DIAMOND_TT_PROBE
    if (g_ttInstalled) return;
    DWORD patchAddr = EXE_BASE + (TT_WEASEL_APPEND - EXE_BASE);
    DWORD retAddr = patchAddr + 5;   /* 0x42F92C */
    g_ttCave = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE,
                                            PAGE_EXECUTE_READWRITE);
    if (!g_ttCave) return;
    unsigned char *p = g_ttCave;
    /* mov ecx, esi */
    p[0]=0x8B; p[1]=0xCE; p+=2;
    /* call 0x44abf0 (re-emit weasel append) — the stack already has the
     * name+sprite args the game pushed before 0x42F927. */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(ABF0_APPEND)-(DWORD)(p+5); p+=5;
    /* jmp retAddr (inc edi) */
    write_jmp(p, retAddr); p+=5;
    unsigned char patch[5];
    memset(patch, 0x90, 5);
    write_jmp(patch, (DWORD)g_ttCave);
    patch_bytes((void*)patchAddr, patch, 5);
    g_ttInstalled = 1;
    diag_log("[diamond] TT PROBE: minimal re-emit cave (no diamond handler) installed");
    return;
#endif

    /* DISABLED — CONCLUSIVELY proved to crash the TT menu on real Windows.
     * THREE byte-identical crashes (2026-08-16) across THREE different cave
     * contents: (1) lazy-loader re-entrant, (2) null-guard, (3) Option B =
     * preloaded icon + pure in-flow 0x44abf0 append (identical to the game's
     * own per-race append). All crash at the SAME trace:
     *   regs esi=0D3xE310 ebp=0D3x1348 esp=001AFB54 edi=0,
     *   stack[2]=0x4D3FDD stack[6]=0x4CB378 stack[8]=0x433E70 (GameSelectionManager)
     *   crash on TT-menu open, CURRENTOBJECT Game Menu / MouseDown.
     * => The crash is the CAVE ARCHITECTURE ITSELF: jumping to a
     * VirtualAlloc'd region from inside GameSelectionManager's active SEH
     * frame corrupts the ESP-relative unwind state on real Windows, regardless
     * of handler content. Punctuated: the game's own in-line 0x44abf0 calls
     * are safe; a mod cave re-entering that code is not.
     * 0x42F927 left 100% original. Per-race earned indicators must be drawn as
     * a NON-REENTRANT overlay (read g_won[], never re-enter 0x44abf0). */
    diag_log("[diamond] TT-menu cave DISABLED (cave architecture crashes TT menu on real Windows)");
}

/* Cave E: block the results-screen click/keypress skip when the diamond was
 * achieved (so the player sees the full gold+240 reveal instead of the frame
 * counter rocketing forward at 10x). We stop the SKIP LATCH from being set
 * rather than hacking the multiplier — the latch (results+0x25) is the single
 * source of truth for skip; if it never gets set, +0x10 advances 1x/frame and
 * the white-fade/vortex/trophy timeline plays out. This was dropped when the
 * reveal host moved to the vtable override (issue #4: "countdown starts much
 * too early" — without it, clicking continue fast-forwards past gold and the
 * reveal fires all at once).
 *
 * Hook at 0x44CBAA: `movb $1,0x25(%esi)` (set latch, 4 bytes) + first byte of
 * next instr (`cmp %bl,0x24(%esi)` = 38 5e 24) = 5 bytes, ret 0x44CBB1. Cave:
 *   push esi; call diamond_block_skip; add esp,4; test eax,eax
 *   jnz block_skip          ; achieved -> do NOT set the latch
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

/* ================================================================
 * Install
 * ================================================================ */

/* PATH-1 PROBE: the untested NON-SEH results function 0x44CB90.
 * Entry bytes: 53 56 8B F1 (push ebx; push esi; mov esi,ecx) — 4 bytes,
 * NO fs:[0] SEH frame. Hook the entry with a 5-byte JMP trampoline that
 * logs entry + the results frame counter [esi+0x1c], throttled to once per
 * unique results object + every ~60 hits, then re-emits the prologue and
 * resumes. Crash-safe (non-SEH host, pure throttled in-memory log).
 * Build: -DDIAMOND_CB90_PROBE. Answers: does 0x44CB90 run per-frame during
 * the results window, and is it a safe host? */
#ifdef DIAMOND_CB90_PROBE
#define CB90_ENTRY   0x44CB90   /* push ebx; push esi; mov esi,ecx (4 bytes) */
#define CB90_RESUME  0x44CB94   /* resume: mov 0x1c(esi),eax */
static int g_cb90Count = 0;
static DWORD g_cb90LastObj = 0;

static void cb90_probe(DWORD obj) {
    DWORD frame = 0;
    if (!obj || IsBadReadPtr((void*)obj, 0x100)) return;
    if (obj != g_cb90LastObj) {
        g_cb90LastObj = obj;
        g_cb90Count = 0;
        trace_logf("[CB90] new results obj=%08X", obj);
    }
    g_cb90Count++;
    if (g_cb90Count <= 1 || (g_cb90Count % 60) == 0) {
        if (IsBadReadPtr((void*)(obj + 0x1c), 4)) frame = 0;
        else frame = *(DWORD*)(obj + 0x1c);
        trace_logf("[CB90] hit #%d frame_cnt=%d obj=%08X", g_cb90Count, frame, obj);
    }
}

static void install_cb90_probe(void) {
    DWORD addr = EXE_BASE + (CB90_ENTRY - EXE_BASE);
    DWORD resume = EXE_BASE + (CB90_RESUME - EXE_BASE);
    unsigned char *stub = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE,
                                                       PAGE_EXECUTE_READWRITE);
    if (!stub) return;
    unsigned char *p = stub;
    p[0]=0x60; p+=1;                              /* pushad */
    p[0]=0x9C; p+=1;                              /* pushfd */
    p[0]=0x56; p+=1;                              /* push esi (results obj) */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)cb90_probe-(DWORD)(p+5); p+=5; /* call */
    p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;        /* add esp,4 */
    p[0]=0x9D; p+=1;                              /* popfd */
    p[0]=0x61; p+=1;                              /* popad */
    p[0]=0x53; p+=1;                              /* re-emit: push ebx */
    p[0]=0x56; p+=1;                              /* re-emit: push esi */
    p[0]=0x8B; p[1]=0xF1; p+=2;                   /* re-emit: mov esi,ecx */
    write_jmp(p, resume); p+=5;                   /* jmp 0x44CB94 */
    unsigned char patch[5];
    write_jmp(patch, (DWORD)stub);
    patch_bytes((void*)addr, patch, 5);
    diag_log("[diamond] CB90 PROBE: trampoline at 0x44CB90 (non-SEH results fn)");
}
#endif /* DIAMOND_CB90_PROBE */


/* APPROACH-D VTABLE OVERRIDE — shared-slot version (DIAMOND_VTABLE_OVERRIDE).
 *
 * Insight: board collision handlers hook fine (board vtable slot 0x1D =
 * DispatchCollisionEvents 0x40C5D0) — a vtable SLOT override is the proven safe
 * pattern, NOT interior-of-SEH code patches / not a per-object copy.
 *
 * Prior attempt (per-object vtable copy via a cave at 0x44C8DE) CRASHED because
 * 0x44C8DE sits INSIDE the SEH-wrapped results-ctor 0x44C880 — running mod C
 * inside any SEH results function corrupts the chain (5th confirmation, user
 * log: stack[4]=0x4CC69F ctor scope, stack[6]=0x409F56 = after call 0x44c880).
 *
 * FIX: patch the SHARED vtable slot 0x4D6CF0+4 directly from the SAFE init
 * thread (no SEH frame there). Point slot[1] at diamond_reveal_update — a
 * legitimate __thiscall(ecx=results) dispatch target, called by the scene's
 * results iterator from OUTSIDE any SEH body (identical to how board
 * vtable[0x1D] handlers are dispatched). No mod code ever runs inside an SEH
 * function. Build: -DDIAMOND_VTABLE_OVERRIDE.
 */
#ifdef DIAMOND_VTABLE_OVERRIDE
#define AWARD_VTABLE       0x4D6CF0   /* results/award object vtable */
#define AWARD_ORIG_UPDATE  0x44D760   /* vtable slot[1] = the SEH award update (called as sub-call) */
#define AWARD_VTABLE_SLOT1 0x4D6CF4   /* shared vtable + 4 = slot[1] */

typedef void (__thiscall *award_update_fn)(void *self);
static award_update_fn g_origAwardUpdate = NULL;
static int g_revealArmedVtbl = 0;

/* The reveal update: __thiscall(ecx=results). Runs the reveal, then calls the
 * original award update (which owns its own valid SEH frame internally).
 *
 * CRITICAL ORDER: call the ORIGINAL award update FIRST. 0x44D760 commits the
 * player race time (board+0x1C) into the results object / board; the reveal
 * reads that time (via get_player_time_cs -> board+0x1C) to decide if the
 * diamond qualified. Running the reveal BEFORE the original read time=0 and
 * silently did nothing (FIRST-EARN time=0 + SWAP-GATE time=0 in the user log).
 * After the original runs, the time is populated and the reveal evaluates
 * correctly.
 *
 * OPTION-1 (VORTEX_OFF): the suction-vortex raw D3D8 draw crashed real Windows
 * at frame ~57 (eax=0 deref inside d3d8.dll). The white-fade + diamond-swap
 * do NOT touch the D3D device, so they're kept. Vortex is compiled out here so
 * we ship a stable reveal; re-enable (undef VORTEX_OFF) after the vortex's
 * DrawPrimitiveUP is made Windows-robust. */
static void __thiscall diamond_reveal_update(void *self) {
    DWORD results = (DWORD)self;
    /* 1) run the ORIGINAL award update FIRST so board+0x1C (player time) is
     *    populated before the reveal reads it to decide qualification. */
    if (g_origAwardUpdate) g_origAwardUpdate(self);
    /* 2) always guard that an armed trophy swap stays bound to THIS results
     *    object. Once the reveal disarms (g_revealArmedVtbl=0) we stop calling
     *    diamond_trophy_swap, so without this the diamond sprite pointer would
     *    leak onto the next screen's ctx+0x37C. Restore if the object changed. */
    if (g_trophySwapActive && results != g_trophySwapResults)
        diamond_trophy_restore(results);
    /* 3) now run the reveal (reads the time correctly). The consolidated
     *    procedural-composite vortex (diamond_vortex_tick) now does EVERYTHING
     *    the reveal needs in ONE texture + one ctx+0x37C swap:
     *      - captures the golden weasel's real pixels once,
     *      - lerps them toward white by the reveal-frame t,
     *      - paints the suction streaks into the annulus outside the weasel,
     *      - uploads via CreateTexture+LockRect, binds to a sprite +0x50,
     *      - swaps that sprite into ctx+0x37C so the game's own renderer
     *        draws it with its single Sprite_DrawRect.
     *    The former white-out mechanisms (weasel material-diffuse ramp
     *    diamond_weasel_mult + additive diamond_set_add) are subsumed: the
     *    white is baked by pixel control, so they would double-tint and fight
     *    the texture. We therefore call ONLY the composite vortex. */
    if (g_revealArmedVtbl) {
#ifndef VORTEX_OFF
        diamond_vortex_tick(results);   /* composite: white-lerp + streaks in ONE texture */
#endif
        if (diamond_seq_frame(results) >= WEASEL_WHITE_TOTAL) {
            /* reveal done: disarm composite (restore ctx+0x37C + free our
             * texture/sprite), then swap in the diamond trophy.
             *
             * FIX (A, 2026-08-17): do NOT set g_revealArmedVtbl=0 here. That
             * permanently latched the reveal OFF after the FIRST earn, so every
             * subsequent race's diamond (and every replay) never revealed —
             * the vtable wrapper stayed installed but the reveal block was
             * gated off forever. All the per-call guards below (diamond_first_
             * earn for the vortex, and diamond_trophy_swap's own frame /
             * threshold / cross-screen checks) already make it safe for the
             * reveal to run every frame the shared vtable is reached. The
             * vortex re-inits on each fresh results object (results change
             * teardown in diamond_vortex_tick) and the trophy swap is
             * bound-by-object, so re-arming naturally per screen. Keep the
             * wrapper armed for the life of the session. */
#ifndef VORTEX_OFF
            vortex_disarm();
#endif
            diamond_trophy_swap(results);
            /* ensure the old white-out mechanisms are fully cleared even if
             * they never ran in this build (defensive restore of the shared
             * mult + default blend so no state leaks to the next frame). */
            diamond_weasel_mult_clear(results);
            diamond_set_add(results, 0.0f);
        }
    }
}

/* Patch the SHARED vtable slot from the safe init thread (no SEH frame). */
static void install_vtable_override(void) {
    DWORD slot = EXE_BASE + (AWARD_VTABLE_SLOT1 - EXE_BASE);
    DWORD origAddr = *(DWORD*)slot;              /* should be 0x44D760 */
    DWORD origUpdate = (DWORD)(EXE_BASE+(AWARD_ORIG_UPDATE-EXE_BASE));
    g_origAwardUpdate = (award_update_fn)origUpdate;
    if (origAddr != origUpdate) {
        diag_logf("[diamond] VTABLE OVERRIDE: unexpected slot1 %08X (expected %08X) — patching anyway", origAddr, origUpdate);
    }
    patch_bytes((void*)slot, &(DWORD){ (DWORD)diamond_reveal_update }, 4);
    g_revealArmedVtbl = 1;
    diag_log("[diamond] VTABLE OVERRIDE: shared slot[1]=0x4D6CF4 -> reveal+orig (patched from init thread, no SEH)");
}
#endif /* DIAMOND_VTABLE_OVERRIDE */

/* ---- 0x46C1F1 frame-epilogue PROBE (DIAMOND_C6F1_PROBE) ----
 * Answers a single question on real Windows: does the GameUpdate frame
 * epilogue (0x46C1F1: pop esi; add esp,8; ret = 5E 83 C4 08 C3) fire while the
 * medal-award screen is live with a diamond reveal armed? Establishes whether a
 * present-hook white overlay is even viable (the earlier "zero present-tick
 * FIRES" could mean the host doesn't run on that screen, OR that my gates bailed
 * — this resolves it).
 *
 * The site is a NON-SEH epilogue (fn 0x46c170: sub esp,8; push esi; ...; ret),
 * so a 5-byte JMP there is a legitimate non-SEH host. Installed from the init
 * thread (NOT DllMain) so no boot redirect. The tick is pure-log: reads
 * g_revealArmedVtbl (set by the reveal driver) + a reversed one-shot, logs
 * exactly one line per reveal if the epilogue actually runs while armed. No
 * D3D, no game-state reads beyond the flag. */
#ifdef DIAMOND_C6F1_PROBE
#define C6F1_EPI   0x46C1F1   /* pop esi; add esp,8; ret (5E 83 C4 08 C3) */
static int g_c6f1Logged = 0;  /* one-shot per session */
static void c6f1_probe(void) {
    /* Only care while a reveal is armed — the window where an overlay draw
     * would happen. Gate on the same flag the reveal driver uses. */
    if (!g_revealArmedVtbl) return;
    if (g_c6f1Logged) return;
    g_c6f1Logged = 1;
    trace_logf("[C6F1] frame-epilogue FIRES while reveal armed (present-hook host viable)");
}
static void install_c6f1_probe(void) {
    DWORD addr = EXE_BASE + (C6F1_EPI - EXE_BASE);
    unsigned char *stub = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE,
                                                       PAGE_EXECUTE_READWRITE);
    if (!stub) return;
    unsigned char *p = stub;
    p[0]=0x60; p+=1;                              /* pushad */
    p[0]=0x9C; p+=1;                              /* pushfd */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)c6f1_probe-(DWORD)(p+5); p+=5; /* call */
    p[0]=0x9D; p+=1;                              /* popfd */
    p[0]=0x61; p+=1;                              /* popad */
    /* re-emit epilogue: pop esi; add esp,8; ret */
    p[0]=0x5E; p+=1;
    p[0]=0x83; p[1]=0xC4; p[2]=0x08; p+=3;
    p[0]=0xC3; p+=1;
    unsigned char patch[5];
    write_jmp(patch, (DWORD)stub);
    patch_bytes((void*)addr, patch, 5);
    diag_log("[diamond] C6F1 PROBE: epilogue trampoline at 0x46C1F1 (non-SEH, installed from init thread)");
}
#endif /* DIAMOND_C6F1_PROBE */


static void install_hooks(void) {
    install_icon_cave();     /* no-op */
#ifdef DIAMOND_TT_WRAPPER
    install_tt_wrapper();
    install_tt_cave();       /* (disabled in this build) */
#else
    install_tt_cave();       /* deferred until first diamond */
#endif
#ifdef DIAMOND_VTABLE_OVERRIDE
    install_vtable_override();
    install_skip_cave();                     /* block skip so the gold+240 reveal plays */
#ifdef DIAMOND_C6F1_PROBE
    install_c6f1_probe();                    /* probe: does 0x46C1F1 fire while reveal armed? */
#endif
    diag_log("[diamond] hooks installed (VTABLE OVERRIDE: slot[1]=reveal+orig, per-object copy)");
#elif defined(DIAMOND_CB90_PROBE)
    /* PATH-1 PROBE: hook the untested non-SEH results fn 0x44CB90.
     * Logs entry/frame counters; reveals whether it fires per-frame. */
    install_cb90_probe();
    diag_log("[diamond] hooks installed (CB90 PROBE: 0x44CB90 trampoline, no reveal)");
#elif defined(DIAMOND_REVEAL_DISABLED)
    /* ISOLATION TEST: reveal patch completely OFF. No game hooks installed.
     * Keeps VEH + BASS proxy + flusher thread (the mod "baseline"). If the
     * crash persists here, the reveal patch is innocent and the fault is in
     * the baseline; if it clears, the 0x44D760 host is the cause. */
    diag_log("[diamond] ISOLATION BUILD: reveal cave DISABLED (no game patches)");
#else
    install_reveal_cave();
    diag_log("[diamond] hooks installed (reveal = full-wrapper SEH replacement of 0x44D760)");
#endif
}

/* ================================================================
 * DllMain
 * ================================================================ */

/* Vectored exception handler that logs the EXACT faulting address + what
 * stage the reveal was in. A single real crash raises a CASCADE of exceptions
 * (E06D7363/C0000005 during unwind), so we LATCH the FIRST one — the genuine
 * primary fault — and only log the rest as a suppressed cascade count. The
 * primary entry records the faulting EIP, whether it's inside the game module
 * or heap, and the first instruction bytes at EIP (to see if it's mid-cave).
 *
 * This is essential: earlier logs drowned the real fault in cascade noise with
 * a corrupted CONTEXT (esp=1.0f etc.); only the first exception's CONTEXT is
 * trustworthy. */
static volatile LONG g_primaryLogged = 0;
static LONG CALLBACK diamond_veh(PEXCEPTION_POINTERS ep) {
    DWORD eip = (DWORD)ep->ExceptionRecord->ExceptionAddress;
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    /* If we already latched the primary, just count the cascade (logged once). */
    if (InterlockedExchange(&g_primaryLogged, 1)) {
        static volatile LONG cascadeDone = 0;
        if (!InterlockedExchange(&cascadeDone, 1))
            diag_logf("[diamond] VEH: (cascade) subsequent exceptions suppressed; final code=%08X", code);
        return EXCEPTION_CONTINUE_SEARCH;
    }
    /* ---- PRIMARY fault ---- */
    /* Is EIP inside the game module? 0x400000 image base, ~0x110000 (0x400000..0x510000). */
    const char *where = (eip >= EXE_BASE && eip < EXE_BASE + 0x200000) ? "EXE" : "HEAP/OTHER";
    diag_logf("[diamond] ** PRIMARY ** EIP=%08X [%s] ERRC=%08X caveProbe=%d",
        eip, where, code, (DWORD)g_caveProbe);
    /* instruction bytes at EIP (only if readable) */
    {
        BYTE b[8]; int n=0;
        for (int i=0;i<8;i++){ if(!IsBadReadPtr((void*)(eip+i),1)) { b[n++] = *(BYTE*)(eip+i);} else break; }
        if (n){ char h[40]; for(int i=0;i<n;i++) sprintf(h+i*2,"%02X",b[i]); h[n*2]=0; diag_logf("[diamond]   EIP bytes: %s", h); }
    }
    diag_logf("[diamond]   regs eax=%08X ebx=%08X ecx=%08X edx=%08X esi=%08X edi=%08X ebp=%08X esp=%08X",
        ep->ContextRecord->Eax, ep->ContextRecord->Ebx,
        ep->ContextRecord->Ecx, ep->ContextRecord->Edx,
        ep->ContextRecord->Esi, ep->ContextRecord->Edi,
        ep->ContextRecord->Ebp, ep->ContextRecord->Esp);
    {
        DWORD *sp = (DWORD*)ep->ContextRecord->Esp;
        int i;
        for (i = 0; i < 12; i++) {
            if (!IsBadReadPtr(sp + i, 4)) {
                DWORD ra = sp[i];
                if (ra >= EXE_BASE && ra < EXE_BASE + 0x200000)
                    diag_logf("[diamond]   stack[%d]=%08X", i, ra);
            }
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

/* ---- Init thread (proven ghost_triggers/warp pattern) ----
 * ALL .text patching is deferred to a background thread that first
 * Sleep(2000)ms, so the game's D3D/audio/font boot init completes before any
 * hook goes live. Patching from DllMain at DLL_PROCESS_ATTACH runs while the
 * Windows loader lock is held; VirtualAlloc + VirtualProtect + patch inside
 * DllMain crashes real Windows at RUNTIME 0-1s (fonts\\showcardgothic28 / the
 * LoadingScreen) — the loader-lock hazard. Wine tolerates it (43s OK), real
 * Windows does not. Deferring to a thread avoids the loader lock entirely.
 *
 * NOTE: Unlike the previous incarnations, this thread does NOT install a
 * per-frame present hook (0x46C1F1/0x455A90). Those ran during boot and
 * caused the now-understood real-Windows crash. Every hook here is a
 * results-screen-only address that NEVER executes during the LoadingScreen
 * (0x44E139 = golden-weasel draw; 0x42F927 = TT-menu append; 0x44CB90/0x44CBAA
 * = award/skip; 0x44D778 = arm-store). None fire at boot — precisely why the
 * stable era (pre present-hook refactor) was crash-free on real Windows. */
static DWORD WINAPI diamond_init_thread(LPVOID param) {
    Sleep(2000);
    diag_log("[diamond] init thread: installing hooks");
    install_hooks();
    diag_log("[diamond] init thread: done");
    return 0;
}

/* Background flusher: drains the in-memory trace ring to the log file. Runs on
 * its own thread, NEVER inside the game's SEH frame, so CRT fopen/fprintf is
 * safe here. Calls diag_logf (which writes to g_log). Wakes every 250ms. */
static DWORD WINAPI diamond_flusher_thread(LPVOID param) {
    int seen = 0;
    for (;;) {
        Sleep(250);
        ring_lock();
        int w = g_ringW;
        ring_unlock();
        while (seen < w) {
            int slot = (seen % RING_ROWS);
            char line[RING_LEN];
            ring_lock(); memcpy(line, g_ring[slot], RING_LEN); ring_unlock();
            line[RING_LEN-1] = 0;
            diag_logf("%s", line);
            seen++;
        }
    }
    return 0;
}

/* Start the flusher thread (once, idempotent) — from DllMain as a plain
 * CreateThread outside the loader-lock risk (it only reads memory + logs). */
static void start_flusher(void) {
    if (g_flusherStarted) return;
    g_flusherStarted = 1;
    CreateThread(NULL, 0, diamond_flusher_thread, NULL, 0, NULL);
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
        load_real_bass();
        get_own_dir(g_logPath, sizeof(g_logPath));
        snprintf(g_logPath,    sizeof(g_logPath), "%s\\\\diamond_weasel_mod.log", g_logPath);
        diag_log("=== DIAMOND WEASEL MOD LOADED ===");
        start_flusher();   /* drains trace ring to disk on its own thread */
        AddVectoredExceptionHandler(1, diamond_veh);
        init_thresholds();
        load_unlocks();
        /* NOTE: the 0x46C1F1 present hook is NOT installed here, and has been
         * REMOVED from the mod entirely. The white-fade is driven from the
         * award-screen vtable-update host (diamond_set_add + weasel_mult),
         * which is the only place that actually renders during the award
         * screen. Installing any JMP->heap redirect at boot is a real-Windows
         * LoadingScreen crash (RUNTIME 00:00:01, C0000005, heap EIP) — and the
         * GameUpdate epilogue doesn't even run while a modal award screen is
         * up, so the present-hook overlay could never have drawn there. Wine
         * tolerates the boot redirect; real Windows does not. */
        /* Defer ALL .text patching to a background thread (Sleep 2s) so we
         * never run VirtualAlloc/VirtualProtect under the loader lock. */
        CreateThread(NULL, 0, diamond_init_thread, NULL, 0, NULL);
    }
    return TRUE;
}
/*
 * ice_mod v1 — N:ICE / water phase-shift system for Hamsterball Physicus
 *
 * RodentRacer spec (2026-08-25 thread):
 *   - N:ICE reference objects: glass slipperiness + glass reflection/transparency
 *   - Global phase flag flipped by E:FREEZE / E:THAW event planes
 *   - Cross-fade: outgoing surface opacity 1->0 over FadeSeconds, THEN swap
 *     render+collision membership (ice <-> water)
 *   - Cold mechanic: linear ramp while touching ice (default 5s) -> ice-blue
 *     tint + input force halved; linear reverse ramp after leaving
 *
 * ── Ghidra-verified mechanics replicated ────────────────────────────────
 *  GLASS SLIP: GlassRace dispatcher (board vtable[29] = 0x417760) sets
 *    ball+0xC5C = 15 on every N:GLASS touch. Ball_ApplyForceV2 (0x4016F0)
 *    multiplies input force x0.20 while ball+0xC5C != 0; Ball_Update
 *    decrements it each frame. We replicate globally: our DispatchCollision
 *    hook sets ball+0xC5C = 15 on N:ICE touches (any level), and a small
 *    detour at 0x40174A scales input force smoothly (cold ramp).
 *  GLASS SHINE: Scene_LoadLevelGlass (0x417640) sets ball+0x280 = 1 for all
 *    balls; Ball_Render helper (0x402C10) wraps the ball draw in
 *    SetRenderState(D3DRS_SPECULARENABLE=0x37) on/off. We set ball+0x280
 *    while the ball is cold (icy sheen) and clear it when warm.
 *  DFLOOR LIFECYCLE: LevelBoard_Neon_ctor loads template MeshWorlds into
 *    board slots; Neon_CreateDynamicObjects spawns ArenaStands via
 *    Stands_ctor(obj, templateLevel) which CLONES the template's spatial
 *    trees (SpatialTree_CloneToLevel) -> instant collision; render object =
 *    Level_RenderCtor(new 0x10D0, obj), registered into the render list at
 *    board+0x8B0 -> +0x18. DFLOOR show/hide = AthenaList_Append /
 *    AthenaList_Remove on that list (FUN_004373e0 decompiled).
 *  CROSS-FADE: material array per MeshBuffer: MB+0x24 count, MB+0x28 array;
 *    stride 0x50; ambient RGBA at +0x14, diffuse RGBA at +0x04. Writing the
 *    alpha component drives per-object opacity.
 *
 * ── Registration lists ──────────────────────────────────────────────────
 *   render list : [[board+0x8B0]+0x18]   (DFLOOR-style show/hide)
 *   collision   : [sceneobj+0x18]        (scene collision tree list,
 *                                         sceneobj = [[scene+0x8AC]+0x480])
 *   Objects are NEVER put in board+0x2578 (update list) - they are static.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

typedef unsigned __int64 QWORD;

/* ═══════════════════════════════════════════════════════════════════
 *  BASS proxy exports (the required game imports)
 * ═══════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;
static void lazy_load_real_bass(void);

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    lazy_load_real_bass();
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    lazy_load_real_bass();
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}
typedef int  (__stdcall *BASS_MusicLoad_t)(int, const char*, QWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(int a, const char* b, QWORD c, DWORD d, DWORD e, DWORD f) {
    lazy_load_real_bass();
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
typedef int  (__stdcall *BASS_SampleLoad_t)(int, const char*, QWORD, DWORD, DWORD, DWORD);
static BASS_SampleLoad_t real_BASS_SampleLoad = NULL;
__declspec(dllexport) int __stdcall BASS_SampleLoad(int a, const char* b, QWORD c, DWORD d, DWORD e, DWORD f) {
    lazy_load_real_bass();
    if (real_BASS_SampleLoad) return real_BASS_SampleLoad(a, b, c, d, e, f);
    return 0;
}
typedef int  (__stdcall *BASS_StreamCreateFile_t)(int, const char*, QWORD, QWORD, DWORD);
static BASS_StreamCreateFile_t real_BASS_StreamCreateFile = NULL;
__declspec(dllexport) int __stdcall BASS_StreamCreateFile(int a, const char* b, QWORD c, QWORD d, DWORD e) {
    lazy_load_real_bass();
    if (real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(a, b, c, d, e);
    return 0;
}
typedef int  (__stdcall *BASS_SampleGetChannel_t)(DWORD, BOOL);
static BASS_SampleGetChannel_t real_BASS_SampleGetChannel = NULL;
__declspec(dllexport) int __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) {
    lazy_load_real_bass();
    if (real_BASS_SampleGetChannel) return real_BASS_SampleGetChannel(a, b);
    return 0;
}
typedef int  (__stdcall *BASS_ChannelPlay_t)(DWORD, BOOL);
static BASS_ChannelPlay_t real_BASS_ChannelPlay = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) {
    lazy_load_real_bass();
    if (real_BASS_ChannelPlay) return real_BASS_ChannelPlay(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    lazy_load_real_bass();
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    lazy_load_real_bass();
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    lazy_load_real_bass();
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    lazy_load_real_bass();
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    lazy_load_real_bass();
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    lazy_load_real_bass();
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    lazy_load_real_bass();
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════
 *  Game addresses / offsets (all Ghidra-verified 2026-08-25)
 * ═══════════════════════════════════════════════════════════════════ */

#define GLOBAL_APP            0x005341E0   /* [this] = App */
#define GLOBAL_SCENE          0x005341E4   /* [this] = Scene/board (race levels) */
#define APP_BOARD             0x178        /* App+0x178 = board */
#define APP_QUIT_FLAG         0x159        /* App+0x159 = quitting byte */

#define BOARD_BALL_LIST       0x29D4       /* AthenaList of balls */
#define BOARD_RENDER_LIST     0x8B0        /* board+0x8B0 -> +0x18 = render list */
#define BOARD_COL_LIST        0x10EC       /* board+0x10EC = collision list */
#define BOARD_SCENE_OBJ       0x8AC        /* board/scene+0x8AC = Level */
#define LEVEL_SCENE_OBJ       0x480        /* Level+0x480 = SceneObject */
#define SCENE_COL_LIST        0x18         /* sceneobj+0x18 = scene collision list */
#define SCENE_REF_COUNT       0x898        /* sceneobj+0x898 = S1 ref count */
#define SCENE_REF_ITEMS       0xCA0        /* sceneobj+0xCA0 = S1 items ptr */

#define PAUSE_FLAG            0x874        /* board+0x874 != 0 = paused */

/* ball offsets */
#define BALL_PLAYERIDX        0x18         /* 0 = player 1 */
#define BALL_GLASS_TIMER      0xC5C        /* slip timer (15 set on glass touch) */
#define BALL_SHINE_FLAG       0x280        /* specular ball render flag */
#define BALL_COLOR_R          0x2AC
#define BALL_COLOR_G          0x2B0
#define BALL_COLOR_B          0x2B4

/* Stands-family entity */
#define STANDS_ALLOC          0x1104
#define OFF_STANDS_POS        0x10D0       /* X/Y/Z floats */
#define OFF_STANDS_BOARD      0x10E4
#define OFF_STANDS_RENDER     0x10E8

/* functions */
#define FN_operator_new       0x004BA57B   /* __cdecl(size) */
#define FN_Stands_ctor        0x00462850   /* __thiscall(this, srcLevel) */
#define FN_Level_RenderCtor   0x00461690   /* __thiscall(this, parentLevel) */
#define FN_AthenaList_Append  0x00453780   /* __thiscall(list, item) */
#define FN_AthenaList_Remove  0x004534D0   /* __thiscall(list, item) */

typedef void* (__cdecl *operator_new_t)(unsigned int);
typedef void* (__thiscall *StandsCtor_t)(void*, void*);
typedef void* (__thiscall *LevelRenderCtor_t)(void*, void*);
typedef void  (__thiscall *AthenaOp_t)(void*, void*);

#define pfn_operator_new   ((operator_new_t)FN_operator_new)
#define pfn_Stands_ctor    ((StandsCtor_t)FN_Stands_ctor)
#define pfn_RenderCtor     ((LevelRenderCtor_t)FN_Level_RenderCtor)
#define pfn_Append         ((AthenaOp_t)FN_AthenaList_Append)
#define pfn_Remove         ((AthenaOp_t)FN_AthenaList_Remove)

/* MeshWorld/MeshBuffer material access */
#define MW_MB_COUNT           0x24         /* MeshWorld+0x24 = MeshBuffer count */
#define MW_MB_ARRAY           0x28         /* MeshWorld+0x28 = MeshBuffer array */

/* hooks */
#define HOOK_DCE              0x0040C5D0   /* DispatchCollisionEvents */
#define HOOK_APPLYFORCE       0x0040174A   /* MOV EAX,[ESI+0xC5C] in Ball_ApplyForceV2 */
#define HOOK_APPLYFORCE_RET   0x00401750
#define HOOK_TICK             0x0046C1F1   /* GameUpdate epilogue: 5E 83 C4 08 C3 */
#define HOOK_TICK_RET         0x0046C1F6

/* ═══════════════════════════════════════════════════════════════════
 *  Configuration (plain floats; auto-generated on first run)
 * ═══════════════════════════════════════════════════════════════════ */

#define CFG_COLD_SECONDS    5.0f
#define CFG_WARMUP_SECONDS  5.0f
#define CFG_INPUT_SCALE     0.5f
#define CFG_TINT_BLUE       0.35f
#define CFG_FADE_SECONDS    1.0f

static float cfg_cold_seconds   = CFG_COLD_SECONDS;
static float cfg_warmup_seconds = CFG_WARMUP_SECONDS;
static float cfg_input_scale    = CFG_INPUT_SCALE;
static float cfg_tint_blue      = CFG_TINT_BLUE;
static float cfg_fade_seconds   = CFG_FADE_SECONDS;

static void get_dll_dir(char *out, int len) {
    MEMORY_BASIC_INFORMATION mbi;
    HMODULE hm = NULL;
    if (VirtualQuery((LPCVOID)&get_dll_dir, &mbi, sizeof(mbi)) && mbi.AllocationBase)
        hm = (HMODULE)mbi.AllocationBase;
    if (!GetModuleFileNameA(hm, out, len)) out[0] = 0;
    char *s = strrchr(out, '\\');
    if (s) *(s + 1) = 0;
}

static void load_config(void) {
    char path[MAX_PATH];
    get_dll_dir(path, MAX_PATH);
    lstrcatA(path, "ice_mod.ini");

    FILE *f = fopen(path, "r");
    if (!f) {
        f = fopen(path, "w");
        if (f) {
            fprintf(f, "# ice_mod configuration (plain floats)\n");
            fprintf(f, "# Seconds touching ice until fully cold (input halved, deep blue)\n");
            fprintf(f, "ColdSeconds=%.2f\n", CFG_COLD_SECONDS);
            fprintf(f, "# Seconds to warm back up after leaving ice\n");
            fprintf(f, "WarmupSeconds=%.2f\n", CFG_WARMUP_SECONDS);
            fprintf(f, "# Input force multiplier at full cold (0.5 = half speed)\n");
            fprintf(f, "InputScale=%.2f\n", CFG_INPUT_SCALE);
            fprintf(f, "# How blue the ball tints at full cold (0..1, smaller = bluer)\n");
            fprintf(f, "TintBlue=%.2f\n", CFG_TINT_BLUE);
            fprintf(f, "# Ice/water cross-fade duration in seconds\n");
            fprintf(f, "FadeSeconds=%.2f\n", CFG_FADE_SECONDS);
            fclose(f);
        }
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *h = strchr(line, '#');
        if (h) *h = 0;
        float v;
        if      (sscanf(line, " ColdSeconds = %f", &v) == 1 && v > 0.05f) cfg_cold_seconds = v;
        else if (sscanf(line, " WarmupSeconds = %f", &v) == 1 && v > 0.05f) cfg_warmup_seconds = v;
        else if (sscanf(line, " InputScale = %f", &v) == 1 && v > 0.05f && v <= 1.0f) cfg_input_scale = v;
        else if (sscanf(line, " TintBlue = %f", &v) == 1 && v >= 0.0f && v <= 1.0f) cfg_tint_blue = v;
        else if (sscanf(line, " FadeSeconds = %f", &v) == 1 && v > 0.05f) cfg_fade_seconds = v;
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Debug log (dedicated file)
 * ═══════════════════════════════════════════════════════════════════ */

static void ilog(const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    buf[sizeof(buf) - 1] = 0;

    char path[MAX_PATH];
    get_dll_dir(path, MAX_PATH);
    lstrcatA(path, "ice_mod.log");
    FILE *f = fopen(path, "a");
    if (f) { fputs(buf, f); fputc('\n', f); fclose(f); }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Mod state
 * ═══════════════════════════════════════════════════════════════════ */

#define MAX_OBJECTS 32

typedef struct {
    DWORD obj;         /* Stands-family entity (cloned collision trees) */
    DWORD render;      /* Level_RenderCtor render object */
    int   in_render;   /* currently in render list */
} PhaseObject;

static PhaseObject g_ice[MAX_OBJECTS];
static int   g_ice_count = 0;
static PhaseObject g_water[MAX_OBJECTS];
static int   g_water_count = 0;

enum { PHASE_WATER = 0, PHASE_TO_ICE = 1, PHASE_ICE = 2, PHASE_TO_WATER = 3 };
static int    g_phase        = PHASE_WATER;
static float  g_fade_t       = 0.0f;

static DWORD  g_last_board   = 0;
static int    g_level_done   = 0;

/* cold system */
static float  g_cold         = 0.0f;    /* 0 = warm .. 1 = fully cold */
static int    g_touch_flag   = 0;       /* set by DCE hook, consumed per frame */
static volatile float g_input_scale = 1.0f;  /* read by ApplyForceV2 cave */

static BYTE  *g_applyforce_cave = NULL;
static BYTE  *g_tick_cave       = NULL;

static BYTE  g_dce_original[8];
static BYTE *g_dce_trampoline = NULL;
static BYTE *g_dce_stub       = NULL;

/* ═══════════════════════════════════════════════════════════════════
 *  Helpers
 * ═══════════════════════════════════════════════════════════════════ */

static int game_is_quitting(void) {
    DWORD app = *(DWORD *)GLOBAL_APP;
    if (!app || app < 0x10000 || IsBadReadPtr((void *)app, 0x160)) return 1;
    return *(BYTE *)(app + APP_QUIT_FLAG) != 0;
}

static DWORD get_board(void) {
    DWORD app = *(DWORD *)GLOBAL_APP;
    if (!app || app < 0x10000 || IsBadReadPtr((void *)app, 0x200)) return 0;
    DWORD board = *(DWORD *)(app + APP_BOARD);
    if (!board || board < 0x10000 || IsBadReadPtr((void *)board, 0x3000)) return 0;
    return board;
}

static DWORD get_player_ball(DWORD board) {
    if (!board) return 0;
    DWORD listBase = board + BOARD_BALL_LIST;
    if (IsBadReadPtr((void *)listBase, 0x410)) return 0;
    int count = *(int *)(listBase + 0x04);
    if (count < 1 || count > 64) return 0;
    DWORD items = *(DWORD *)(listBase + 0x40C);
    if (!items || IsBadReadPtr((void *)items, count * 4)) return 0;
    for (int i = 0; i < count; i++) {
        DWORD ball = *(DWORD *)(items + i * 4);
        if (!ball || ball < 0x10000 || IsBadReadPtr((void *)ball, 0x200)) continue;
        if (*(DWORD *)(ball + BALL_PLAYERIDX) == 0) return ball;
    }
    return 0;
}

/* Write per-MeshBuffer material alpha (opacity) for a render object's MeshWorld */
static void set_render_opacity(DWORD render_obj, float opacity) {
    if (!render_obj || render_obj < 0x10000 || IsBadReadPtr((void *)render_obj, 0x10)) return;
    DWORD mw = *(DWORD *)((char *)render_obj + 0x08);
    if (!mw || mw < 0x10000 || IsBadReadPtr((void *)mw, 0x30)) return;
    DWORD count = *(DWORD *)((char *)mw + MW_MB_COUNT);
    DWORD *items = *(DWORD **)((char *)mw + MW_MB_ARRAY);
    if (!items || count == 0 || count > 256) return;
    for (DWORD i = 0; i < count; i++) {
        DWORD mb = items[i];
        if (!mb || mb < 0x10000 || IsBadReadPtr((void *)mb, 0x30)) continue;
        DWORD matCount = *(DWORD *)((char *)mb + 0x24);
        float *mats = *(float **)((char *)mb + 0x28);
        if (!mats || matCount == 0 || matCount > 1024) continue;
        if (IsBadReadPtr((void *)mats, matCount * 0x50)) continue;
        for (DWORD j = 0; j < matCount; j++) {
            float *m = mats + j * (0x50 / 4);
            m[(0x04 + 12) / 4] = opacity;   /* diffuse  alpha */
            m[(0x14 + 12) / 4] = opacity;   /* ambient  alpha */
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Spawn: ArenaStands lifecycle (template Level -> Stands_ctor clone)
 * ═══════════════════════════════════════════════════════════════════ */

/* Find an existing collision-list Level whose MeshWorld is EMPTY (count==0)
 * to use as the Stands_ctor clone source (collision trees come from its
 * +0x18 list, which non-empty Levels always have). */
static DWORD find_template_level(DWORD board) {
    if (IsBadReadPtr((void *)((char *)board + BOARD_COL_LIST), 0x410)) return 0;
    int count = *(int *)((char *)board + BOARD_COL_LIST + 0x04);
    if (count < 1 || count > 4096) return 0;
    DWORD items = *(DWORD *)((char *)board + BOARD_COL_LIST + 0x40C);
    if (!items || items < 0x10000 || IsBadReadPtr((void *)items, count * 4)) return 0;

    DWORD fallback = 0;
    for (int i = 0; i < count; i++) {
        DWORD lvl = *(DWORD *)(items + i * 4);
        if (!lvl || lvl < 0x10000 || IsBadReadPtr((void *)lvl, 0x10)) continue;
        DWORD mw = *(DWORD *)(lvl + 0x08);
        if (!mw || mw < 0x10000 || IsBadReadPtr((void *)mw, 0x30)) continue;
        DWORD mbc = *(DWORD *)((char *)mw + MW_MB_COUNT);
        if (mbc == 0) return lvl;
        if (!fallback) fallback = lvl;
    }
    return fallback;
}

static int spawn_phase_object(DWORD board, DWORD template_level, float x, float y,
                              float z, PhaseObject *out) {
    memset(out, 0, sizeof(*out));
    if (!template_level) return 0;

    DWORD obj = (DWORD)pfn_operator_new(STANDS_ALLOC);
    if (!obj) return 0;
    if (!pfn_Stands_ctor((void *)obj, (void *)template_level)) return 0;

    *(DWORD *)((char *)obj + OFF_STANDS_BOARD) = board;
    *(float *)((char *)obj + OFF_STANDS_POS + 0) = x;
    *(float *)((char *)obj + OFF_STANDS_POS + 4) = y;
    *(float *)((char *)obj + OFF_STANDS_POS + 8) = z;

    DWORD rmem = (DWORD)pfn_operator_new(0x10D0);
    if (!rmem) return 0;
    DWORD render = (DWORD)pfn_RenderCtor((void *)rmem, (void *)obj);
    if (!render) return 0;
    *(DWORD *)((char *)obj + OFF_STANDS_RENDER) = render;

    if (!IsBadReadPtr((void *)(obj + 0x434), 4) && *(DWORD *)(obj + 0x434))
        *(DWORD *)((char *)render + 0x434) = *(DWORD *)(obj + 0x434);
    if (!IsBadReadPtr((void *)((char *)render + 0x431), 1))
        *(BYTE *)((char *)render + 0x431) = 0;

    out->obj = obj;
    out->render = render;
    out->in_render = 0;
    return 1;
}

static void render_list_add(DWORD board, DWORD render_obj) {
    DWORD rl_head = *(DWORD *)((char *)board + BOARD_RENDER_LIST);
    if (!rl_head || rl_head < 0x10000 || IsBadReadPtr((void *)(rl_head + 0x18), 0x410)) return;
    pfn_Append((void *)(rl_head + 0x18), (void *)render_obj);
}

static void render_list_remove(DWORD board, DWORD render_obj) {
    DWORD rl_head = *(DWORD *)((char *)board + BOARD_RENDER_LIST);
    if (!rl_head || rl_head < 0x10000 || IsBadReadPtr((void *)(rl_head + 0x18), 0x410)) return;
    pfn_Remove((void *)(rl_head + 0x18), (void *)render_obj);
}

static void collision_list_add(DWORD board, DWORD obj) {
    DWORD scene = *(DWORD *)GLOBAL_SCENE;
    if (!scene || scene < 0x10000 || IsBadReadPtr((void *)(scene + BOARD_SCENE_OBJ), 4)) return;
    DWORD level = *(DWORD *)((char *)scene + BOARD_SCENE_OBJ);
    if (!level || level < 0x10000 || IsBadReadPtr((void *)(level + LEVEL_SCENE_OBJ), 4)) return;
    DWORD sceneobj = *(DWORD *)((char *)level + LEVEL_SCENE_OBJ);
    if (!sceneobj || sceneobj < 0x10000 || IsBadReadPtr((void *)((char *)sceneobj + SCENE_COL_LIST), 0x410)) return;
    pfn_Append((void *)((char *)sceneobj + SCENE_COL_LIST), (void *)obj);
}

static void collision_list_remove(DWORD board, DWORD obj) {
    DWORD scene = *(DWORD *)GLOBAL_SCENE;
    if (!scene || scene < 0x10000 || IsBadReadPtr((void *)(scene + BOARD_SCENE_OBJ), 4)) return;
    DWORD level = *(DWORD *)((char *)scene + BOARD_SCENE_OBJ);
    if (!level || level < 0x10000 || IsBadReadPtr((void *)(level + LEVEL_SCENE_OBJ), 4)) return;
    DWORD sceneobj = *(DWORD *)((char *)level + LEVEL_SCENE_OBJ);
    if (!sceneobj || sceneobj < 0x10000 || IsBadReadPtr((void *)((char *)sceneobj + SCENE_COL_LIST), 0x410)) return;
    pfn_Remove((void *)((char *)sceneobj + SCENE_COL_LIST), (void *)obj);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Level setup: scan S1 ref points for ICE / WATER markers
 * ═══════════════════════════════════════════════════════════════════ */

static void setup_level(DWORD board) {
    g_level_done = 1;
    g_ice_count = 0;
    g_water_count = 0;
    g_phase = PHASE_WATER;
    g_fade_t = 0.0f;
    g_cold = 0.0f;

    DWORD scene = *(DWORD *)GLOBAL_SCENE;
    if (!scene || scene < 0x10000 || IsBadReadPtr((void *)(scene + BOARD_SCENE_OBJ), 4)) return;
    DWORD level = *(DWORD *)((char *)scene + BOARD_SCENE_OBJ);
    if (!level || level < 0x10000 || IsBadReadPtr((void *)(level + LEVEL_SCENE_OBJ), 4)) return;
    DWORD sceneobj = *(DWORD *)((char *)level + LEVEL_SCENE_OBJ);
    if (!sceneobj || sceneobj < 0x10000) return;
    if (IsBadReadPtr((void *)((char *)sceneobj + SCENE_REF_COUNT), 4)) return;
    int count = *(int *)((char *)sceneobj + SCENE_REF_COUNT);
    if (count <= 0 || count > 4000) return;
    if (IsBadReadPtr((void *)((char *)sceneobj + SCENE_REF_ITEMS), 4)) return;
    DWORD items_array = *(DWORD *)((char *)sceneobj + SCENE_REF_ITEMS);
    if (!items_array || items_array < 0x10000) return;

    DWORD template_level = 0;

    for (int i = 0; i < count; i++) {
        if (IsBadReadPtr((void *)(items_array + i * 4), 4)) break;
        DWORD item = *(DWORD *)(items_array + i * 4);
        if (!item || item < 0x10000 || IsBadReadPtr((void *)item, 16)) continue;
        char *name = *(char **)item;
        if (!name || IsBadReadPtr(name, 8)) continue;

        int is_ice   = (_strnicmp(name, "ICE",   3) == 0);
        int is_water = (_strnicmp(name, "WATER", 5) == 0);
        if (!is_ice && !is_water) continue;

        float rx = *(float *)(item + 4);
        float ry = *(float *)(item + 8);
        float rz = *(float *)(item + 12);

        if (!template_level) {
            template_level = find_template_level(board);
            if (!template_level) {
                ilog("SETUP: ref '%s' found but no template Level available", name);
                break;
            }
        }

        if (is_ice && g_ice_count < MAX_OBJECTS) {
            if (spawn_phase_object(board, template_level, rx, ry, rz, &g_ice[g_ice_count])) {
                ilog("SETUP: ICE ref '%s' (%.1f, %.1f, %.1f) obj=%p render=%p",
                     name, rx, ry, rz, (void *)g_ice[g_ice_count].obj,
                     (void *)g_ice[g_ice_count].render);
                g_ice_count++;
            }
        } else if (is_water && g_water_count < MAX_OBJECTS) {
            if (spawn_phase_object(board, template_level, rx, ry, rz, &g_water[g_water_count])) {
                ilog("SETUP: WATER ref '%s' (%.1f, %.1f, %.1f) obj=%p render=%p",
                     name, rx, ry, rz, (void *)g_water[g_water_count].obj,
                     (void *)g_water[g_water_count].render);
                g_water_count++;
            }
        }
    }

    if (g_water_count == 0 && g_ice_count == 0) {
        ilog("SETUP: no ICE/WATER refs on this level (board=%p)", (void *)board);
        return;
    }

    /* Initial state = WATER: water visible + solid, ice dormant */
    for (int i = 0; i < g_water_count; i++) {
        render_list_add(board, g_water[i].render);
        g_water[i].in_render = 1;
        set_render_opacity(g_water[i].render, 1.0f);
        collision_list_add(board, g_water[i].obj);
    }
    ilog("SETUP: phase=WATER, ice=%d water=%d (board=%p)",
         g_ice_count, g_water_count, (void *)board);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Swaps
 * ═══════════════════════════════════════════════════════════════════ */

static void do_swap_to_ice(DWORD board) {
    for (int i = 0; i < g_water_count; i++) {
        if (g_water[i].in_render) { render_list_remove(board, g_water[i].render); g_water[i].in_render = 0; }
        collision_list_remove(board, g_water[i].obj);
    }
    for (int i = 0; i < g_ice_count; i++) {
        if (!g_ice[i].in_render) { render_list_add(board, g_ice[i].render); g_ice[i].in_render = 1; }
        set_render_opacity(g_ice[i].render, 1.0f);
        collision_list_add(board, g_ice[i].obj);
    }
    ilog("PHASE: swapped to ICE (board=%p)", (void *)board);
}

static void do_swap_to_water(DWORD board) {
    for (int i = 0; i < g_ice_count; i++) {
        if (g_ice[i].in_render) { render_list_remove(board, g_ice[i].render); g_ice[i].in_render = 0; }
        collision_list_remove(board, g_ice[i].obj);
    }
    for (int i = 0; i < g_water_count; i++) {
        if (!g_water[i].in_render) { render_list_add(board, g_water[i].render); g_water[i].in_render = 1; }
        set_render_opacity(g_water[i].render, 1.0f);
        collision_list_add(board, g_water[i].obj);
    }
    ilog("PHASE: swapped to WATER (board=%p)", (void *)board);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Per-frame tick (main thread, frame epilogue host)
 * ═══════════════════════════════════════════════════════════════════ */

static void ice_tick(void) {
    if (game_is_quitting()) return;

    DWORD board = get_board();
    if (!board) return;

    if (board != g_last_board) {
        g_last_board = board;
        g_level_done = 0;
        g_ice_count = 0;
        g_water_count = 0;
        g_phase = PHASE_WATER;
        g_cold = 0.0f;
        ilog("LEVEL: new board %p", (void *)board);
    }
    if (!g_level_done) {
        setup_level(board);
        return;
    }
    if (g_ice_count == 0 && g_water_count == 0) return;

    int paused = *(BYTE *)((char *)board + PAUSE_FLAG) != 0;

    if (!paused) {
        /* ---- phase machine ---- */
        if (g_phase == PHASE_TO_ICE || g_phase == PHASE_TO_WATER) {
            g_fade_t += 1.0f / (cfg_fade_seconds * 60.0f);
            if (g_fade_t >= 1.0f) {
                if (g_phase == PHASE_TO_ICE) {
                    do_swap_to_ice(board);
                    g_phase = PHASE_ICE;
                } else {
                    do_swap_to_water(board);
                    g_phase = PHASE_WATER;
                }
                g_fade_t = 0.0f;
            }
        }

        /* ---- cold ramp (linear up while touching, linear down after) ---- */
        if (g_touch_flag) {
            g_touch_flag = 0;
            if (g_phase == PHASE_ICE || g_phase == PHASE_TO_ICE) {
                g_cold += 1.0f / (cfg_cold_seconds * 60.0f);
                if (g_cold > 1.0f) g_cold = 1.0f;
            }
        } else if (g_cold > 0.0f) {
            g_cold -= 1.0f / (cfg_warmup_seconds * 60.0f);
            if (g_cold < 0.0f) g_cold = 0.0f;
        }

        g_input_scale = 1.0f - (1.0f - cfg_input_scale) * g_cold;
    }

    /* ---- cross-fade opacity ---- */
    if ((g_phase == PHASE_TO_ICE || g_phase == PHASE_TO_WATER) && !paused) {
        if (g_phase == PHASE_TO_ICE) {
            for (int i = 0; i < g_water_count; i++)
                set_render_opacity(g_water[i].render, 1.0f - g_fade_t);
            for (int i = 0; i < g_ice_count; i++)
                set_render_opacity(g_ice[i].render, g_fade_t);
        } else {
            for (int i = 0; i < g_ice_count; i++)
                set_render_opacity(g_ice[i].render, 1.0f - g_fade_t);
            for (int i = 0; i < g_water_count; i++)
                set_render_opacity(g_water[i].render, g_fade_t);
        }
    }

    /* ---- ball tint + shine ---- */
    DWORD ball = get_player_ball(board);
    if (ball && !IsBadReadPtr((void *)ball, 0x2C0)) {
        if (g_cold > 0.001f) {
            float g = 1.0f - (1.0f - cfg_tint_blue) * g_cold;
            *(float *)((char *)ball + BALL_COLOR_R) = 1.0f;
            *(float *)((char *)ball + BALL_COLOR_G) = g;
            *(float *)((char *)ball + BALL_COLOR_B) = g;
            *(BYTE *)((char *)ball + BALL_SHINE_FLAG) = 1;
        } else {
            *(float *)((char *)ball + BALL_COLOR_R) = 1.0f;
            *(float *)((char *)ball + BALL_COLOR_G) = 1.0f;
            *(float *)((char *)ball + BALL_COLOR_B) = 1.0f;
            /* Only clear the shine flag on levels where WE set it
               (never fight Glass Race's own management). */
            if (g_ice_count > 0)
                *(BYTE *)((char *)ball + BALL_SHINE_FLAG) = 0;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Hook utilities
 * ═══════════════════════════════════════════════════════════════════ */

static void patch_bytes(DWORD addr, BYTE *bytes, int len) {
    DWORD old;
    VirtualProtect((void *)addr, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy((void *)addr, bytes, len);
    VirtualProtect((void *)addr, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void *)addr, len);
}

static BYTE *alloc_executable(int size) {
    return (BYTE *)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE,
                                PAGE_EXECUTE_READWRITE);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Hook 1: DispatchCollisionEvents (trampoline, proven portal_mod pattern)
 *  Detects E:FREEZE / E:THAW flips and N:ICE touches.
 * ═══════════════════════════════════════════════════════════════════ */

static void __cdecl dce_handler(DWORD board, DWORD ball, DWORD *coll_data) {
    (void)board;
    if (!ball || ball < 0x10000 || IsBadReadPtr((void *)ball, 0xC60)) return;
    if (!coll_data || IsBadReadPtr((void *)coll_data, 8)) return;
    DWORD mesh = coll_data[1];
    if (!mesh || mesh < 0x10000 || IsBadReadPtr((void *)(mesh + 0x864), 4)) return;
    char *name = *(char **)(mesh + 0x864);
    if (!name || IsBadReadPtr(name, 6)) return;

    if (_strnicmp(name, "E:FREEZE", 8) == 0) {
        if (g_level_done && (g_phase == PHASE_WATER || g_phase == PHASE_TO_WATER)) {
            g_phase = PHASE_TO_ICE;
            g_fade_t = 0.0f;
            ilog("EVENT: E:FREEZE -> fading to ICE");
        }
    }
    else if (_strnicmp(name, "E:THAW", 6) == 0) {
        if (g_level_done && (g_phase == PHASE_ICE || g_phase == PHASE_TO_ICE)) {
            g_phase = PHASE_TO_WATER;
            g_fade_t = 0.0f;
            ilog("EVENT: E:THAW -> fading to WATER");
        }
    }
    else if (_strnicmp(name, "N:ICE", 5) == 0) {
        /* Same slip mechanic the Glass Race dispatcher uses for N:GLASS:
           refresh the decay timer; Ball_ApplyForceV2 sees nonzero -> slippery.
           Ball_Update decrements it each frame, so it self-clears on leave. */
        *(DWORD *)((char *)ball + BALL_GLASS_TIMER) = 15;
        g_touch_flag = 1;
    }
}

static void build_dce_hook(void) {
    memcpy(g_dce_original, (void *)HOOK_DCE, 8);

    g_dce_trampoline = alloc_executable(16);
    if (!g_dce_trampoline) return;
    memcpy(g_dce_trampoline, g_dce_original, 8);
    g_dce_trampoline[8] = 0xE9;
    *(DWORD *)(g_dce_trampoline + 9) =
        (DWORD)(HOOK_DCE + 8) - (DWORD)(g_dce_trampoline + 13);

    /* At DCE entry: ECX=board, [ESP+4]=ball, [ESP+8]=coll_data, RET 0x8.
     * After pushad+pushfd (36 bytes): ball@ESP+0x28, coll@ESP+0x2C. */
    g_dce_stub = alloc_executable(64);
    if (!g_dce_stub) return;
    {
        int i = 0;
        BYTE *c = g_dce_stub;
        c[i++] = 0x60;                                   /* pushad */
        c[i++] = 0x9C;                                   /* pushfd */
        c[i++] = 0xFF; c[i++] = 0x74; c[i++] = 0x24; c[i++] = 0x2C;
        c[i++] = 0xFF; c[i++] = 0x74; c[i++] = 0x24; c[i++] = 0x2C;
        c[i++] = 0x51;                                   /* push ecx (board) */
        c[i++] = 0xE8;                                   /* call dce_handler */
        *(DWORD *)(c + i) = (DWORD)&dce_handler - (DWORD)(c + i + 4);
        i += 4;
        c[i++] = 0x83; c[i++] = 0xC4; c[i++] = 0x0C;     /* add esp,12 */
        c[i++] = 0x9D;                                   /* popfd */
        c[i++] = 0x61;                                   /* popad */
        c[i++] = 0xB8;                                   /* mov eax, trampoline */
        *(DWORD *)(c + i) = (DWORD)g_dce_trampoline;
        i += 4;
        c[i++] = 0xFF; c[i++] = 0xE0;                    /* jmp eax */

        BYTE patch[8];
        patch[0] = 0xE9;
        *(DWORD *)(patch + 1) = (DWORD)g_dce_stub - HOOK_DCE - 5;
        patch[5] = 0x90; patch[6] = 0x90; patch[7] = 0x90;
        patch_bytes(HOOK_DCE, patch, 8);
        ilog("HOOK: DispatchCollisionEvents installed");
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Hook 2: Ball_ApplyForceV2 smooth input scale
 *
 *  Original at 0x40174A: 8B 86 5C 0C 00 00  (MOV EAX,[ESI+0xC5C], 6 bytes)
 *  ESI = ball. ST0 = input magnitude (loaded unconditionally at 0x40172E;
 *  nothing between there and here pops ST0). Following code does
 *  TEST EAX,EAX so flag clobbering is safe, and EAX is fully rewritten.
 *  Cave: FMUL ST0,[g_input_scale]; re-emit MOV; jmp back. No saves needed.
 * ═══════════════════════════════════════════════════════════════════ */

static void build_applyforce_hook(void) {
    static const BYTE expect[6] = { 0x8B, 0x86, 0x5C, 0x0C, 0x00, 0x00 };
    if (memcmp((const void *)HOOK_APPLYFORCE, expect, 6) != 0) {
        ilog("HOOK: ApplyForceV2 bytes mismatch - skipped (safe)");
        return;
    }

    g_applyforce_cave = alloc_executable(32);
    if (!g_applyforce_cave) return;
    {
        int i = 0;
        BYTE *c = g_applyforce_cave;
        c[i++] = 0xD8; c[i++] = 0x0D;                    /* FMUL dword [scale] */
        *(DWORD *)(c + i) = (DWORD)&g_input_scale; i += 4;
        c[i++] = 0x8B; c[i++] = 0x86;                    /* MOV EAX,[esi+C5C] */
        *(DWORD *)(c + i) = 0x00000C5C; i += 4;
        c[i++] = 0xE9;                                   /* jmp back */
        *(DWORD *)(c + i) = HOOK_APPLYFORCE_RET - (DWORD)(c + i + 4);

        BYTE patch[6];
        patch[0] = 0xE9;
        *(DWORD *)(patch + 1) = (DWORD)g_applyforce_cave - HOOK_APPLYFORCE - 5;
        patch_bytes(HOOK_APPLYFORCE, patch, 6);
        ilog("HOOK: Ball_ApplyForceV2 input-scale installed");
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Hook 3: frame epilogue tick host (boot-safe, proven pattern)
 *  Original at 0x46C1F1: 5E 83 C4 08 C3 (POP ESI / ADD ESP,8 / RET)
 * ═══════════════════════════════════════════════════════════════════ */

static void build_tick_hook(void) {
    static const BYTE expect[5] = { 0x5E, 0x83, 0xC4, 0x08, 0xC3 };
    if (memcmp((const void *)HOOK_TICK, expect, 5) != 0) {
        ilog("HOOK: tick epilogue bytes mismatch - skipped (safe)");
        return;
    }

    g_tick_cave = alloc_executable(32);
    if (!g_tick_cave) return;
    {
        int i = 0;
        BYTE *c = g_tick_cave;
        c[i++] = 0x60;                                   /* pushad */
        c[i++] = 0x9C;                                   /* pushfd */
        c[i++] = 0xB8;                                   /* mov eax, ice_tick */
        *(DWORD *)(c + i) = (DWORD)&ice_tick; i += 4;
        c[i++] = 0xFF; c[i++] = 0xD0;                    /* call eax */
        c[i++] = 0x9D;                                   /* popfd */
        c[i++] = 0x61;                                   /* popad */
        memcpy(c + i, expect, 5); i += 5;                /* re-emit epilogue */
        c[i++] = 0xE9;                                   /* jmp back */
        *(DWORD *)(c + i) = HOOK_TICK_RET - (DWORD)(c + i + 4);

        BYTE patch[5];
        patch[0] = 0xE9;
        *(DWORD *)(patch + 1) = (DWORD)g_tick_cave - HOOK_TICK - 5;
        patch_bytes(HOOK_TICK, patch, 5);
        ilog("HOOK: frame epilogue tick installed");
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  BASS proxy init
 * ═══════════════════════════════════════════════════════════════════ */

static void lazy_load_real_bass(void) {
    if (g_hRealBass) return;

    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "bass_real.dll");
        g_hRealBass = LoadLibraryA(path);
    }
    if (!g_hRealBass) {
        GetSystemDirectoryA(path, MAX_PATH);
        lstrcatA(path, "\\bass.dll");
        g_hRealBass = LoadLibraryA(path);
    }
    if (!g_hRealBass) return;

    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_SampleLoad = (BASS_SampleLoad_t)GetProcAddress(g_hRealBass, "BASS_SampleLoad");
    real_BASS_StreamCreateFile = (BASS_StreamCreateFile_t)GetProcAddress(g_hRealBass, "BASS_StreamCreateFile");
    real_BASS_SampleGetChannel = (BASS_SampleGetChannel_t)GetProcAddress(g_hRealBass, "BASS_SampleGetChannel");
    real_BASS_ChannelPlay = (BASS_ChannelPlay_t)GetProcAddress(g_hRealBass, "BASS_ChannelPlay");
    real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    real_BASS_Init = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_Free = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Stop = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_Start = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
}

static DWORD WINAPI patch_thread(LPVOID param) {
    (void)param;
    Sleep(2000);
    load_config();
    ilog("=== ice_mod v1 started ===");
    build_dce_hook();
    build_applyforce_hook();
    build_tick_hook();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)hinstDLL;
    (void)lpvReserved;
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    }
    return TRUE;
}

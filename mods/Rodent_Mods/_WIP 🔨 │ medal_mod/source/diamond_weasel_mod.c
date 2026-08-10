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
__declspec(dllexport) void __stdcall BASS_Start(void) {
    if (real_BASS_Start) real_BASS_Start();
}
typedef int (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) void __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) real_BASS_Stop();
}
typedef int (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
typedef int (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
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
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { (void)a;(void)b; return 1; }
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

#define ICON_LOAD_HOOK     0x42A304
#define GOLD_DRAW_HOOK     0x44EFD2   /* call 0x42c7c0 (gold draw) */
#define SPRITE_DRAW        0x42C7C0

/* ================================================================
 * Mod globals
 * ================================================================ */
static DWORD g_diamondSprite = 0;
static float g_secret[15]    = {0};
static int   g_hasSecret[15] = {0};
static BYTE  g_won[15]       = {0};
static char  g_iconFile[64]  = "diamondweasel.png";
static int   g_iconLoaded    = 0;
static int   g_configLoaded  = 0;

static char  g_logPath[MAX_PATH] = {0};
static char  g_cfgPath[MAX_PATH] = {0};
static char  g_unlockPath[MAX_PATH] = {0};
static FILE *g_log = NULL;

static unsigned char *g_iconCave = NULL;
static unsigned char *g_dispCave = NULL;

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
 * Config
 * ================================================================ */
static int race_index_from_name(const char *name) {
    static const char *races[15] = {
        "WARM-UP","BEGINNER","INTERMEDIATE","DIZZY","TOWER","UP","NEON",
        "EXPERT","ODD","TOOB","WOBBLY","GLASS","SKY","MASTER","IMPOSSIBLE"
    };
    int i;
    if (!name) return -1;
    for (i = 0; i < 15; i++) if (_stricmp(name, races[i]) == 0) return i;
    return -1;
}
static void load_config(void) {
    FILE *f; char line[256]; int cur = -1;
    f = fopen(g_cfgPath, "r");
    if (!f) { diag_logf("[diamond] no config: %s", g_cfgPath); return; }
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n' || *p == '\r' || *p == 0) continue;
        if (*p == '[') {
            char *end = strchr(p, ']');
            if (end) { *end = 0; cur = race_index_from_name(p + 1); }
            continue;
        }
        if (cur < 0 || cur > 14) continue;
        if (_strnicmp(p, "SECRET=", 7) == 0) {
            g_secret[cur] = (float)atof(p + 7);
            g_hasSecret[cur] = 1;
            diag_logf("[diamond] race %d SECRET=%.2f", cur, g_secret[cur]);
        } else if (_strnicmp(p, "ICON=", 5) == 0) {
            strncpy(g_iconFile, p + 5, sizeof(g_iconFile) - 1);
            g_iconFile[sizeof(g_iconFile) - 1] = 0;
            char *nl = strchr(g_iconFile, '\n'); if (nl) *nl = 0;
            nl = strchr(g_iconFile, '\r'); if (nl) *nl = 0;
            diag_logf("[diamond] icon = %s", g_iconFile);
        }
    }
    fclose(f);
    g_configLoaded = 1;
    diag_log("[diamond] config loaded");
}
static void load_unlocks(void) {
    FILE *f = fopen(g_unlockPath, "rb");
    if (!f) { diag_logf("[diamond] no unlocks file"); return; }
    size_t n = fread(g_won, 1, 15, f);
    fclose(f);
    diag_logf("[diamond] loaded %d unlock flags", (int)n);
}
static void save_unlocks(void) {
    FILE *f = fopen(g_unlockPath, "wb");
    if (!f) return;
    fwrite(g_won, 1, 15, f);
    fclose(f);
    diag_log("[diamond] unlocks saved");
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
static float get_player_time_cs(DWORD app) {
    if (IsBadReadPtr((void*)(app + APP_BOARD), 4)) return 0.0f;
    DWORD board = *(DWORD*)(app + APP_BOARD);
    if (!board) return 0.0f;
    if (IsBadReadPtr((void*)(board + BOARD_TIME), 4)) return 0.0f;
    return *(float*)(board + BOARD_TIME);
}

/* ================================================================
 * Core logic (called from caves)
 * ================================================================ */
__attribute__((used)) void diamond_check_award(DWORD app) {
    int race; float cs, thr;
    if (!app || !g_configLoaded) return;
    race = get_race_index();
    if (race < 0 || race > 14) return;
    if (!g_hasSecret[race]) return;
    cs = get_player_time_cs(app);
    thr = g_secret[race] * 100.0f;
    if (cs > 0.0f && cs <= thr) {
        if (!g_won[race]) {
            g_won[race] = 1;
            diag_logf("[diamond] RACE %d SECRET EARNED (%.2f <= %.2f)", race, cs, thr);
            save_unlocks();
        }
    }
}
__attribute__((used)) void diamond_load_icon_impl(DWORD app);
__attribute__((used)) int diamond_should_show(DWORD app) {
    int race; float cs, thr;
    if (!app || !g_configLoaded) return 0;
    if (!g_iconLoaded) diamond_load_icon_impl(app);   /* lazy-load; manager valid at results time */
    race = get_race_index();
    if (race < 0 || race > 14) return 0;
    if (!g_hasSecret[race]) return 0;
    if (!g_diamondSprite) return 0;
    cs = get_player_time_cs(app);
    thr = g_secret[race] * 100.0f;
    if (cs > 0.0f && cs <= thr) {
        if (!g_won[race]) { g_won[race] = 1; save_unlocks(); }
        return 1;
    }
    return 0;
}
__attribute__((used)) void diamond_load_icon_impl(DWORD app) {
    DWORD mgr, vt, load;
    if (g_iconLoaded) return;
    if (!app || !g_configLoaded) return;
    mgr = *(DWORD*)(app + APP_MGR);
    if (!mgr) return;
    if (IsBadReadPtr((void*)mgr, 4)) return;
    vt = *(DWORD*)mgr;
    if (!vt) return;
    if (IsBadReadPtr((void*)(vt + 0x58), 4)) return;
    load = *(DWORD*)(vt + 0x58);
    if (!load) return;
    /* __thiscall: ecx=mgr, push &slot, push <str>, call [vt+0x58].
     * MinGW __thiscall fn pointers silently fail — use inline asm. */
    __asm__ volatile(
        "pushl %2\n\t"        /* &g_diamondSprite (slot) */
        "pushl %3\n\t"        /* g_iconFile (str) */
        "movl %0, %%ecx\n\t"  /* mgr */
        "call *%1\n\t"        /* load */
        "addl $8, %%esp\n\t"
        : : "r"(mgr), "r"(load), "r"(&g_diamondSprite), "r"(g_iconFile)
        : "eax", "ecx", "edx", "memory"
    );
    g_iconLoaded = 1;
    diag_logf("[diamond] icon loaded: %s -> %08X", g_iconFile, g_diamondSprite);
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
 *   cave: pushad; call [eax+0x58]; mov ecx,[esi+0x22C]; push esi;
 *         call diamond_load_icon_impl; add esp,4; popad; jmp 0x42A30D
 */
static void install_icon_cave(void) {
    DWORD patchAddr = EXE_BASE + (ICON_LOAD_HOOK - EXE_BASE);
    DWORD retAddr = patchAddr + 9;
    g_iconCave = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE,
                                              PAGE_EXECUTE_READWRITE);
    if (!g_iconCave) return;
    unsigned char *p = g_iconCave;
    p[0]=0x60; p+=1;                                   /* pushad */
    p[0]=0xFF; p[1]=0x50; p[2]=0x58; p+=3;              /* call [eax+0x58] */
    p[0]=0x8B; p[1]=0x8E; p[2]=0x2C; p[3]=0x02; p[4]=0x00; p[5]=0x00; p+=6; /* mov ecx,[esi+0x22C] */
    p[0]=0x56; p+=1;                                   /* push esi */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_load_icon_impl-(DWORD)(p+5); p+=5; /* call */
    p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;              /* add esp,4 */
    p[0]=0x61; p+=1;                                   /* popad */
    write_jmp(p, retAddr); p+=5;
    unsigned char patch[9];
    memset(patch, 0x90, 9);
    write_jmp(patch, (DWORD)g_iconCave);
    patch_bytes((void*)patchAddr, patch, 9);
    diag_log("[diamond] icon cave installed at 0x42A304");
}

/* Cave B: results-screen DIAMOND draw after GOLD at 0x44EFD2 (covers 5 bytes).
 *   original: call 0x42c7c0   (draws gold sprite; ecx=gold sprite, stack has x,y)
 *   cave:     call 0x42c7c0            ; re-emit gold draw (existing stack args)
 *             pushad
 *             push [esi+0xc]           ; App (lazy-load icon + secret check)
 *             call diamond_should_show ; returns 1 if secret earned
 *             add esp,4
 *             test eax,eax
 *             jz skip
 *             mov ecx,[g_diamondSprite]
 *             push DIAMOND_Y
 *             push DIAMOND_X
 *             call 0x42c7c0            ; draw diamond
 *             add esp,8
 *           skip: popad
 *             jmp 0x44EFD7
 *   Diamond position: over the GOLDEN WEASEL (0x208, 0x63). The diamond is
 *   the 5th medal in the sequence (drawn after gold) but rendered at the
 *   golden weasel's spot, directly over it.
 */
static void install_disp_cave(void) {
    DWORD patchAddr = EXE_BASE + (GOLD_DRAW_HOOK - EXE_BASE);
    DWORD retAddr = patchAddr + 5;   /* 0x44EFD7 */
    const DWORD DIAMOND_X = 0x208;   /* golden weasel x */
    const DWORD DIAMOND_Y = 0x63;    /* golden weasel y */
    g_dispCave = (unsigned char*)VirtualAlloc(NULL, 160, MEM_COMMIT|MEM_RESERVE,
                                              PAGE_EXECUTE_READWRITE);
    if (!g_dispCave) return;
    unsigned char *p = g_dispCave;
    /* call 0x42c7c0 (re-emit gold draw) — E8 rel32 */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(EXE_BASE+SPRITE_DRAW)-(DWORD)(p+5); p+=5;
    /* pushad */
    p[0]=0x60; p+=1;
    /* push [esi+0xc] — FF 76 0C */
    p[0]=0xFF; p[1]=0x76; p[2]=0x0C; p+=3;
    /* call diamond_should_show */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)diamond_should_show-(DWORD)(p+5); p+=5;
    /* add esp,4 */
    p[0]=0x83; p[1]=0xC4; p[2]=0x04; p+=3;
    /* test eax,eax */
    p[0]=0x85; p[1]=0xC0; p+=2;
    /* jz skip */
    p[0]=0x74; p[1]=0x18; p+=2;
    /* mov ecx,[g_diamondSprite] — 8B 0D <addr> */
    p[0]=0x8B; p[1]=0x0D; *(DWORD*)(p+2)=(DWORD)&g_diamondSprite; p+=6;
    /* push DIAMOND_Y */
    p[0]=0x68; *(DWORD*)(p+1)=DIAMOND_Y; p+=5;
    /* push DIAMOND_X */
    p[0]=0x68; *(DWORD*)(p+1)=DIAMOND_X; p+=5;
    /* call 0x42c7c0 */
    p[0]=0xE8; *(DWORD*)(p+1)=(DWORD)(EXE_BASE+SPRITE_DRAW)-(DWORD)(p+5); p+=5;
    /* add esp,8 */
    p[0]=0x83; p[1]=0xC4; p[2]=0x08; p+=3;
    /* skip: popad */
    p[0]=0x61; p+=1;
    /* jmp retAddr */
    write_jmp(p, retAddr); p+=5;
    unsigned char patch[5];
    memset(patch, 0x90, 5);
    write_jmp(patch, (DWORD)g_dispCave);
    patch_bytes((void*)patchAddr, patch, 5);
    diag_log("[diamond] diamond-after-gold cave installed at 0x44EFD2");
}

/* ================================================================
 * Install
 * ================================================================ */
static void install_hooks(void) {
    install_icon_cave();
    install_disp_cave();
    diag_log("[diamond] hooks installed");
}

/* ================================================================
 * DllMain
 * ================================================================ */
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
        load_real_bass();
        get_own_dir(g_logPath, sizeof(g_logPath));
        snprintf(g_cfgPath,    sizeof(g_cfgPath), "%s\\diamond_weasel_config.txt", g_logPath);
        snprintf(g_unlockPath, sizeof(g_unlockPath), "%s\\diamond_weasel_unlocks.dat", g_logPath);
        snprintf(g_logPath,    sizeof(g_logPath), "%s\\diamond_weasel_mod.log", g_logPath);
        diag_log("=== DIAMOND WEASEL MOD LOADED ===");
        load_config();
        load_unlocks();
        install_hooks();
    }
    return TRUE;
}
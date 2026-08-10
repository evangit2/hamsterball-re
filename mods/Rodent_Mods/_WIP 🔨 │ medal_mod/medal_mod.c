/*
 * medal_mod.c — Medal Time Mod (bass.dll proxy)
 *
 * Overrides the per-race medal thresholds (WEASEL/GOLD/SILVER/BRONZE)
 * that the game loads from Data/RaceData.xml at startup.
 *
 * The game's RaceData.xml parser (0x40A120, __thiscall ECX=App) is called
 * once per race from each board constructor. The caller pushes the race
 * block name ("BEGINNERRACE", "DIZZYRACE", ...) on the stack, then calls
 * with ECX=App. The parser fills:
 *   App+0x2998 = TIME    (centiseconds)
 *   App+0x2990 = PAR     (9 + PAR*100)
 *   App+0x2994 = WEASEL  (9 + WEASEL*100)
 *   App+0x29A0 = GOLD    (9 + GOLD*100)
 *   App+0x29A4 = SILVER  (9 + SILVER*100)
 *   App+0x29A8 = BRONZE  (9 + BRONZE*100)
 *   App+0x299C = CAM     (raw float)
 *
 * The TT results screen awards medals when player time <= threshold:
 *   WEASEL: time <= App+0x2994  -> flag App+0x8BF+idx*4 = 1
 *   BRONZE: time <= App+0x29A8  -> flag App+0x8BC+idx*4 = 1
 *   SILVER: time <= App+0x29A4  -> flag App+0x8BD+idx*4 = 1
 *   GOLD:   time <= App+0x29A0  -> flag App+0x8BE+idx*4 = 1
 * (idx = tournament race index from profile+8)
 *
 * Hook design:
 *   1. ENTRY hook (0x40A120): captures the race-name argument pushed by
 *      the caller ([esp+4] at entry) into g_raceName, then jumps through
 *      a trampoline that runs the original 7-byte prologue and resumes
 *      at 0x40A127.
 *   2. TAIL hook (0x40A3F6): after the game's own parse + medal-target
 *      selection, calls medal_override_impl(App) which looks up config
 *      overrides by race name and overwrites the 4 threshold slots.
 *
 * Everything downstream (results screen awards, HUD target time) reads the
 * same App slots, so the overrides take effect automatically.
 *
 * Config: medal_config.txt (next to bass.dll)
 *   [BEGINNER]
 *   WEASEL=6.6
 *   GOLD=7.6
 *   SILVER=10.3
 *   BRONZE=15.0
 *   ... one section per race, in TOURNAMENT order:
 *   WARMUP (no medals), BEGINNER, INTERMEDIATE, DIZZY, TOWER, UP, NEON,
 *   EXPERT, ODD, TOOB, WOBBLY, GLASS, SKY, MASTER, IMPOSSIBLE.
 *   Missing sections keep the game's original values.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll medal_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/* ================================================================
 * BASS type definitions (proxy)
 * ================================================================ */
typedef unsigned long long QWORD;
typedef DWORD HSTREAM;
typedef DWORD HMUSIC;
typedef DWORD HFX;

/* ================================================================
 * Game constants
 * ================================================================ */
#define APP_PROFILE_PTR         0x220        /* PlayerProfile* */
#define PROFILE_RACE_INDEX      0x08         /* race index (tournament order) */
#define APP_MEDAL_PAR           0x2990
#define APP_MEDAL_WEASEL        0x2994
#define APP_MEDAL_TIME          0x2998
#define APP_MEDAL_CAM           0x299C
#define APP_MEDAL_GOLD          0x29A0
#define APP_MEDAL_SILVER        0x29A4
#define APP_MEDAL_BRONZE        0x29A8
#define APP_MEDAL_CURRENT       0x29AC
#define APP_MEDAL_FLAGS         0x8BC        /* 20 DWORDs, byte flags per race */

/* Parser entry: 0x40A120 (PUSH -1; PUSH 0x4C9560; ...) */
#define PARSER_ENTRY_HOOK       0x0040A120
#define PARSER_ENTRY_BYTES      6            /* JMP (5B) + NOP (1B) covers PUSH -1 (2B) + first 4B of PUSH imm32 */
#define PARSER_ENTRY_RESUME     0x0040A127   /* resume after the 7-byte prologue */

/* Parser tail: 0x40A3F6 (LEA ECX,[ESP+0x24]; MOV DWORD[ESP+0x44],-1) */
#define PARSER_TAIL_HOOK        0x0040A3F6
#define PARSER_TAIL_BYTES       11
#define PARSER_TAIL_RETURN      0x0040A402   /* CALL 0x4695d0 */

/* ================================================================
 * Globals
 * ================================================================ */
static HINSTANCE g_hInst = NULL;
static int g_hookInstalled = 0;
static int g_configLoaded = 0;

static char g_logPath[MAX_PATH];       /* full path to medal_mod_log.txt */
static char g_configPath[MAX_PATH];    /* full path to medal_config.txt */

/* Per-race overrides, indexed by tournament race index (0-14) */
static float g_overrideWeasel[15];
static float g_overrideGold[15];
static float g_overrideSilver[15];
static float g_overrideBronze[15];
static int   g_hasWeasel[15], g_hasGold[15], g_hasSilver[15], g_hasBronze[15];

/* Captured race name from parser entry; overrides applied by name at tail */
static char g_raceName[64];

/* Hook infrastructure */
static BYTE g_parserEntryOrig[PARSER_ENTRY_BYTES];
static BYTE g_parserTailOrig[PARSER_TAIL_BYTES];
static unsigned char *g_parserTrampoline = NULL;
static const void *g_parserReturn = (void*)PARSER_TAIL_RETURN;

/* ================================================================
 * BASS forwarding
 * ================================================================ */
static HMODULE g_hRealBass = NULL;
typedef int  (__stdcall *BASS_Init_t)(DWORD, DWORD, DWORD, HWND, void*);
typedef void (__stdcall *BASS_Free_t)(void);
typedef DWORD(__stdcall *BASS_StreamCreateFile_t)(BOOL, void*, QWORD, QWORD, DWORD);
typedef int  (__stdcall *BASS_StreamFree_t)(DWORD);
typedef DWORD(__stdcall *BASS_MusicLoad_t)(BOOL, void*, QWORD, QWORD, DWORD, DWORD);
typedef int  (__stdcall *BASS_MusicFree_t)(DWORD);
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, DWORD, int);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int  (__stdcall *BASS_Start_t)(void);
typedef int  (__stdcall *BASS_Stop_t)(void);
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, int, int, int);
typedef int  (__stdcall *BASS_ChannelSetAttribute_t)(DWORD, DWORD, float);
typedef int  (__stdcall *BASS_ChannelGetData_t)(DWORD, void*, DWORD);
typedef DWORD(__stdcall *BASS_ChannelSetFX_t)(DWORD, DWORD, int);
typedef DWORD(__stdcall *BASS_ChannelGetPosition_t)(DWORD, DWORD);

static BASS_Init_t            real_BASS_Init;
static BASS_Free_t            real_BASS_Free;
static BASS_StreamCreateFile_t real_BASS_StreamCreateFile;
static BASS_StreamFree_t      real_BASS_StreamFree;
static BASS_MusicLoad_t       real_BASS_MusicLoad;
static BASS_MusicFree_t       real_BASS_MusicFree;
static BASS_MusicPlayEx_t     real_BASS_MusicPlayEx;
static BASS_SetConfig_t       real_BASS_SetConfig;
static BASS_Start_t           real_BASS_Start;
static BASS_Stop_t            real_BASS_Stop;
static BASS_ErrorGetCode_t    real_BASS_ErrorGetCode;
static BASS_ChannelStop_t     real_BASS_ChannelStop;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes;
static BASS_ChannelSetAttribute_t  real_BASS_ChannelSetAttribute;
static BASS_ChannelGetData_t  real_BASS_ChannelGetData;
static BASS_ChannelSetFX_t    real_BASS_ChannelSetFX;
static BASS_ChannelGetPosition_t real_BASS_ChannelGetPosition;

__declspec(dllexport) int __stdcall BASS_Init(DWORD a, DWORD b, DWORD c, HWND d, void *e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 0;
}
__declspec(dllexport) void __stdcall BASS_Free(void) {
    if (real_BASS_Free) real_BASS_Free();
}
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(BOOL a, void *b, QWORD c, QWORD d, DWORD e) {
    if (real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(a, b, c, d, e);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_StreamFree(DWORD a) {
    if (real_BASS_StreamFree) return real_BASS_StreamFree(a);
    return 1;
}
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(BOOL a, void *b, QWORD c, QWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_MusicFree(DWORD a) {
    if (real_BASS_MusicFree) return real_BASS_MusicFree(a);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, DWORD c, int d) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c, d);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 0;
}
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 0;
}
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, int b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) {
    if (real_BASS_ChannelSetAttribute) return real_BASS_ChannelSetAttribute(a, b, c);
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, (int)b, 0, 0);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) {
    if (real_BASS_ChannelGetData) return real_BASS_ChannelGetData(a, b, c);
    return 0;
}
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetFX(DWORD a, DWORD b, int c) {
    if (real_BASS_ChannelSetFX) return real_BASS_ChannelSetFX(a, b, c);
    return 0;
}
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) {
    if (real_BASS_ChannelGetPosition) return real_BASS_ChannelGetPosition(a, b);
    return 0;
}

/* ================================================================
 * Utilities
 * ================================================================ */
static void diag_log(const char *msg) {
    FILE *f = fopen(g_logPath, "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

static void diag_logf(const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    diag_log(buf);
}

static void patch_bytes(void *addr, const void *data, DWORD size) {
    DWORD old;
    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, data, size);
    VirtualProtect(addr, size, old, &old);
}

static void *alloc_executable(DWORD size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

/* ================================================================
 * Config parsing
 * ================================================================ */
/* Race section names by tournament race index (0-14).
 * Note: race index 0 = WARM-UP, which has no medal block in
 * RaceData.xml (the parser is never called for it). */
static const char *g_raceNames[15] = {
    "WARMUP",       /* idx 0 */
    "BEGINNER",     /* idx 1: BEGINNERRACE */
    "INTERMEDIATE", /* idx 2: INTERMEDIATERACE */
    "DIZZY",        /* idx 3: DIZZYRACE */
    "TOWER",        /* idx 4: TOWERRACE */
    "UP",           /* idx 5: UPRACE */
    "NEON",         /* idx 6: NEONRACE */
    "EXPERT",       /* idx 7: EXPERTRACE */
    "ODD",          /* idx 8: ODDRACE */
    "TOOB",         /* idx 9: TOOBRACE */
    "WOBBLY",       /* idx 10: WOBBLYRACE */
    "GLASS",        /* idx 11: GLASSRACE */
    "SKY",          /* idx 12: SKYRACE */
    "MASTER",       /* idx 13: MASTERRACE */
    "IMPOSSIBLE"    /* idx 14: IMPOSSIBLERACE */
};

/* XML block names pushed by the board ctors (used for name matching) */
static const char *g_xmlNames[15] = {
    "",                  /* idx 0: WARMUP has no block */
    "BEGINNERRACE",      /* idx 1 */
    "INTERMEDIATERACE",  /* idx 2 */
    "DIZZYRACE",         /* idx 3 */
    "TOWERRACE",         /* idx 4 */
    "UPRACE",            /* idx 5 */
    "NEONRACE",          /* idx 6 */
    "EXPERTRACE",        /* idx 7 */
    "ODDRACE",           /* idx 8 */
    "TOOBRACE",          /* idx 9 */
    "WOBBLYRACE",        /* idx 10 */
    "GLASSRACE",         /* idx 11 */
    "SKYRACE",           /* idx 12 */
    "MASTERRACE",        /* idx 13 */
    "IMPOSSIBLERACE"     /* idx 14 */
};

static int race_index_from_name(const char *name) {
    int i;
    if (!name || !*name) return -1;
    for (i = 0; i < 15; i++) {
        if (g_xmlNames[i][0] && _stricmp(name, g_xmlNames[i]) == 0) return i;
    }
    return -1;
}

static int find_race_section(const char *line) {
    int i;
    for (i = 0; i < 15; i++) {
        char sec[32];
        snprintf(sec, sizeof(sec), "[%s]", g_raceNames[i]);
        if (_stricmp(line, sec) == 0) return i;
    }
    return -1;
}

static void load_config(void) {
    FILE *f;
    char line[256];
    int curRace = -1;
    int i;

    /* Initialize: no overrides */
    for (i = 0; i < 15; i++) {
        g_hasWeasel[i] = g_hasGold[i] = g_hasSilver[i] = g_hasBronze[i] = 0;
        g_overrideWeasel[i] = g_overrideGold[i] = g_overrideSilver[i] = g_overrideBronze[i] = 0.0f;
    }

    f = fopen(g_configPath, "r");
    if (!f) {
        diag_log("[medal_mod] medal_config.txt not found — using game defaults");
        return;
    }

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        char key[64], val[64];

        /* trim whitespace */
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p == '\0' || *p == '#' || *p == ';') continue;

        /* section header? */
        if (*p == '[') {
            char *end = strchr(p, ']');
            if (end) {
                *end = '\0';
                curRace = find_race_section(p + 1);
            }
            continue;
        }

        if (curRace < 0) continue;

        /* key=value */
        if (sscanf(p, "%63[^=]=%63s", key, val) == 2) {
            float v = (float)atof(val);
            if (_stricmp(key, "WEASEL") == 0)  { g_overrideWeasel[curRace] = v;  g_hasWeasel[curRace] = 1; }
            else if (_stricmp(key, "GOLD") == 0)   { g_overrideGold[curRace] = v;   g_hasGold[curRace] = 1; }
            else if (_stricmp(key, "SILVER") == 0) { g_overrideSilver[curRace] = v; g_hasSilver[curRace] = 1; }
            else if (_stricmp(key, "BRONZE") == 0) { g_overrideBronze[curRace] = v; g_hasBronze[curRace] = 1; }
        }
    }
    fclose(f);

    diag_log("[medal_mod] medal_config.txt loaded");
    g_configLoaded = 1;
}

/* ================================================================
 * Parser hooks
 * ================================================================ */

/* Called from parser_entry_stub — must be non-static so the symbol
 * exists for the asm reference. */
__attribute__((used)) void capture_race_name(DWORD namePtr) {
    if (!namePtr) return;
    strncpy(g_raceName, (const char*)namePtr, sizeof(g_raceName) - 1);
    g_raceName[sizeof(g_raceName) - 1] = '\0';
    diag_logf("[medal_mod] parser called for race: %s", g_raceName);
}

/* Entry stub: runs when the parser is called. Reads the race-name
 * argument from the stack ([esp+4] at entry = [esp+16] after 3 pushes),
 * stores it, then jumps into the trampoline which runs the original
 * prologue and resumes at 0x40A127. */
__attribute__((naked, used)) static void parser_entry_stub(void) {
    __asm__ volatile(
        "pushl %%eax\n"
        "pushl %%ecx\n"
        "pushl %%edx\n"
        "movl 16(%%esp), %%eax\n"   /* race name ptr */
        "pushl %%eax\n"
        "call _capture_race_name\n"
        "addl $4, %%esp\n"
        "popl %%edx\n"
        "popl %%ecx\n"
        "popl %%eax\n"
        "jmp *%0\n"
        : : "m"(g_parserTrampoline) : "memory"
    );
}

/* Called from parser_tail_cave — must be non-static for the asm ref. */
__attribute__((used)) void medal_override_impl(DWORD app) {
    int i;

    if (!g_configLoaded) return;

    i = race_index_from_name(g_raceName);
    if (i < 0 || i > 14) return;

    diag_logf("[medal_mod] applying overrides for race %s (idx %d)", g_raceName, i);

    if (g_hasWeasel[i])
        *(int*)(app + APP_MEDAL_WEASEL) = (int)(g_overrideWeasel[i] * 100.0f) + 9;
    if (g_hasGold[i])
        *(int*)(app + APP_MEDAL_GOLD)   = (int)(g_overrideGold[i] * 100.0f) + 9;
    if (g_hasSilver[i])
        *(int*)(app + APP_MEDAL_SILVER) = (int)(g_overrideSilver[i] * 100.0f) + 9;
    if (g_hasBronze[i])
        *(int*)(app + APP_MEDAL_BRONZE) = (int)(g_overrideBronze[i] * 100.0f) + 9;
}

/* Tail cave: runs after the game's parse. EDI = App. Calls
 * medal_override_impl(App), then resumes original flow. */
__attribute__((naked, used)) static void parser_tail_cave(void) {
    __asm__ volatile(
        "pushl %%eax\n"
        "pushl %%ecx\n"
        "pushl %%edx\n"
        "pushl %%edi\n"          /* arg: App */
        "call _medal_override_impl\n"
        "addl $4, %%esp\n"
        "popl %%edx\n"
        "popl %%ecx\n"
        "popl %%eax\n"
        /* resume original flow: LEA ECX,[ESP+0x24]; MOV DWORD[ESP+0x44],-1 */
        "lea 0x24(%%esp), %%ecx\n"
        "movl $0xffffffff, 0x44(%%esp)\n"
        "jmp *%0\n"
        : : "m"(g_parserReturn) : "memory"
    );
}

/* ================================================================
 * Hook installation
 * ================================================================ */
static void install_parser_hook(void) {
    unsigned char *entry = (unsigned char*)PARSER_ENTRY_HOOK;
    unsigned char *tail = (unsigned char*)PARSER_TAIL_HOOK;
    DWORD oldProt;

    if (g_hookInstalled) return;

    /* --- Build entry trampoline --- */
    /* Original prologue at 0x40A120: 6A FF (PUSH -1) 68 60 95 4C 00 (PUSH 0x4C9560) */
    g_parserTrampoline = (unsigned char*)alloc_executable(32);
    if (!g_parserTrampoline) return;

    /* Save originals */
    memcpy(g_parserEntryOrig, entry, PARSER_ENTRY_BYTES);
    memcpy(g_parserTailOrig, tail, PARSER_TAIL_BYTES);

    /* Trampoline body: original 7-byte prologue + JMP back to 0x40A127 */
    g_parserTrampoline[0] = 0x6A; g_parserTrampoline[1] = 0xFF;       /* PUSH -1 */
    g_parserTrampoline[2] = 0x68;                                     /* PUSH imm32 */
    g_parserTrampoline[3] = 0x60; g_parserTrampoline[4] = 0x95;       /* 0x4C9560 */
    g_parserTrampoline[5] = 0x4C; g_parserTrampoline[6] = 0x00;
    g_parserTrampoline[7] = 0xE9;                                     /* JMP rel32 */
    *(DWORD*)(g_parserTrampoline + 8) =
        (DWORD)PARSER_ENTRY_RESUME - (DWORD)(g_parserTrampoline + 7) - 5;

    /* --- Patch entry: JMP -> parser_entry_stub --- */
    if (!VirtualProtect(entry, PARSER_ENTRY_BYTES, PAGE_EXECUTE_READWRITE, &oldProt)) {
        VirtualFree(g_parserTrampoline, 0, MEM_RELEASE);
        g_parserTrampoline = NULL;
        return;
    }
    entry[0] = 0xE9;
    *(DWORD*)(entry + 1) = (DWORD)&parser_entry_stub - (DWORD)entry - 5;
    entry[5] = 0x90;  /* pad */
    VirtualProtect(entry, PARSER_ENTRY_BYTES, oldProt, &oldProt);

    /* --- Patch tail: JMP -> parser_tail_cave --- */
    if (!VirtualProtect(tail, PARSER_TAIL_BYTES, PAGE_EXECUTE_READWRITE, &oldProt)) {
        /* roll back entry */
        patch_bytes(entry, g_parserEntryOrig, PARSER_ENTRY_BYTES);
        VirtualFree(g_parserTrampoline, 0, MEM_RELEASE);
        g_parserTrampoline = NULL;
        return;
    }
    tail[0] = 0xE9;
    *(DWORD*)(tail + 1) = (DWORD)&parser_tail_cave - (DWORD)tail - 5;
    memset(tail + 5, 0x90, PARSER_TAIL_BYTES - 5);
    VirtualProtect(tail, PARSER_TAIL_BYTES, oldProt, &oldProt);

    g_hookInstalled = 1;
    diag_logf("[medal_mod] parser entry hook 0x%08X -> stub 0x%08X, trampoline 0x%08X",
              PARSER_ENTRY_HOOK, (unsigned)&parser_entry_stub, (unsigned)g_parserTrampoline);
    diag_logf("[medal_mod] parser tail hook 0x%08X -> cave 0x%08X",
              PARSER_TAIL_HOOK, (unsigned)&parser_tail_cave);
}

static void restore_parser_hook(void) {
    if (!g_hookInstalled) return;
    patch_bytes((void*)PARSER_ENTRY_HOOK, g_parserEntryOrig, PARSER_ENTRY_BYTES);
    patch_bytes((void*)PARSER_TAIL_HOOK, g_parserTailOrig, PARSER_TAIL_BYTES);
    if (g_parserTrampoline) {
        VirtualFree(g_parserTrampoline, 0, MEM_RELEASE);
        g_parserTrampoline = NULL;
    }
    g_hookInstalled = 0;
    diag_log("[medal_mod] parser hooks restored");
}

/* ================================================================
 * Init thread
 * ================================================================ */
static DWORD WINAPI init_thread(LPVOID param) {
    (void)param;
    Sleep(2000);

    diag_log("=== MEDAL MOD INIT ===");
    diag_logf("  - Parser hooks: %s", g_hookInstalled ? "installed" : "NOT INSTALLED");
    diag_logf("  - Config loaded: %s", g_configLoaded ? "yes" : "no");
    diag_log("=== MEDAL MOD INITIALIZED ===");

    return 0;
}

/* ================================================================
 * DllMain
 * ================================================================ */
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    (void)reserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        g_hInst = hInst;
        DisableThreadLibraryCalls(hInst);

        /* Set up log + config paths (game directory, next to bass.dll) */
        {
            char modPath[MAX_PATH];
            if (GetModuleFileNameA(hInst, modPath, MAX_PATH)) {
                char *p = strrchr(modPath, '\\');
                if (p) *p = '\0';
                snprintf(g_logPath, MAX_PATH, "%s\\medal_mod_log.txt", modPath);
                snprintf(g_configPath, MAX_PATH, "%s\\medal_config.txt", modPath);
            } else {
                strcpy(g_logPath, "medal_mod_log.txt");
                strcpy(g_configPath, "medal_config.txt");
            }
        }

        diag_log("=== MEDAL MOD LOADED ===");

        /* CRITICAL: load_real_bass MUST be called before CreateThread */
        {
            char path[MAX_PATH];
            HMODULE hSelf = NULL;
            GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                              | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              (LPCSTR)&init_thread, &hSelf);
            if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
                char *p = strrchr(path, '\\');
                if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
            }
            if (!g_hRealBass) g_hRealBass = LoadLibraryA("bass_real.dll");
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
                real_BASS_ChannelGetPosition = (BASS_ChannelGetPosition_t)GetProcAddress(g_hRealBass, "BASS_ChannelGetPosition");
            }
        }

        /* Load config + install hooks synchronously in DllMain so the
         * hook is active BEFORE the App ctor runs the RaceData.xml
         * parser (race condition if done in init_thread). */
        load_config();
        install_parser_hook();

        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        restore_parser_hook();
        break;
    }
    return TRUE;
}

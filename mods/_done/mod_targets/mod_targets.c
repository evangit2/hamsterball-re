/*
 * mod_targets.c — Hamsterball Mod Manager DLL
 *
 * Acts as bass.dll proxy. Reads __mod_targets.txt to determine which
 * mod DLLs from the /mods/ subfolder to load at what times.
 *
 * - GLOBAL mods: always loaded
 * - LEVEL N mods: loaded only during race N (1-15)
 * - ARENA N mods: loaded only during arena N (1-15)
 * - Everything else: deactivated by default
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mod_targets.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * Build on Windows (MSVC):
 *   cl /LD mod_targets.c /Fe:bass.dll /link winmm.lib
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* ════════════════════════════════════════════════════════════════════
 * BASS PROXY EXPORTS — lazy-load forwarders to bass_real.dll
 * ════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;
static CRITICAL_SECTION g_bassLock;

/* Function pointer typedefs */
typedef int   (__stdcall *BASS_Init_t)(int, int, int, HWND, void*);
typedef void  (__stdcall *BASS_Free_t)(void);
typedef int   (__stdcall *BASS_Stop_t)(void);
typedef int   (__stdcall *BASS_Start_t)(void);
typedef int   (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int   (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(BOOL, void*, DWORD, DWORD, DWORD, DWORD);
typedef int   (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
typedef int   (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int   (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);

/* Real function pointers */
static BASS_Init_t                real_BASS_Init = NULL;
static BASS_Free_t                real_BASS_Free = NULL;
static BASS_Stop_t                real_BASS_Stop = NULL;
static BASS_Start_t               real_BASS_Start = NULL;
static BASS_SetConfig_t           real_BASS_SetConfig = NULL;
static BASS_ErrorGetCode_t        real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t           real_BASS_MusicLoad = NULL;
static BASS_MusicPlayEx_t         real_BASS_MusicPlayEx = NULL;
static BASS_ChannelStop_t         real_BASS_ChannelStop = NULL;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;

/* Lazily load bass_real.dll and resolve all function pointers */
static void load_real_bass(void)
{
    EnterCriticalSection(&g_bassLock);
    if (g_hRealBass) {
        LeaveCriticalSection(&g_bassLock);
        return;
    }

    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
        LeaveCriticalSection(&g_bassLock);
        return;
    }

    real_BASS_Init                = (BASS_Init_t)GetProcAddress(g_hRealBass, "_BASS_Init@20");
    real_BASS_Free                = (BASS_Free_t)GetProcAddress(g_hRealBass, "_BASS_Free@0");
    real_BASS_Stop                = (BASS_Stop_t)GetProcAddress(g_hRealBass, "_BASS_Stop@0");
    real_BASS_Start               = (BASS_Start_t)GetProcAddress(g_hRealBass, "_BASS_Start@0");
    real_BASS_SetConfig           = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "_BASS_SetConfig@8");
    real_BASS_ErrorGetCode        = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "_BASS_ErrorGetCode@0");
    real_BASS_MusicLoad           = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "_BASS_MusicLoad@24");
    real_BASS_MusicPlayEx         = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "_BASS_MusicPlayEx@12");
    real_BASS_ChannelStop         = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "_BASS_ChannelStop@4");
    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "_BASS_ChannelSetAttributes@16");

    /* If decorated names not found, try undecorated */
    if (!real_BASS_Init)
        real_BASS_Init = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
    if (!real_BASS_Free)
        real_BASS_Free = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
    if (!real_BASS_Stop)
        real_BASS_Stop = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
    if (!real_BASS_Start)
        real_BASS_Start = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
    if (!real_BASS_SetConfig)
        real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    if (!real_BASS_ErrorGetCode)
        real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    if (!real_BASS_MusicLoad)
        real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    if (!real_BASS_MusicPlayEx)
        real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    if (!real_BASS_ChannelStop)
        real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    if (!real_BASS_ChannelSetAttributes)
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");

    LeaveCriticalSection(&g_bassLock);
}

/* Exported BASS functions — forward to bass_real.dll */
__declspec(dllexport) int __stdcall BASS_Init(int a, int b, int c, HWND d, void* e)
{
    load_real_bass();
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 0; /* FALSE = no audio (safe) */
}

__declspec(dllexport) void __stdcall BASS_Free(void)
{
    load_real_bass();
    if (real_BASS_Free) real_BASS_Free();
}

__declspec(dllexport) int __stdcall BASS_Stop(void)
{
    load_real_bass();
    if (real_BASS_Stop) return real_BASS_Stop();
    return 0;
}

__declspec(dllexport) int __stdcall BASS_Start(void)
{
    load_real_bass();
    if (real_BASS_Start) return real_BASS_Start();
    return 0;
}

__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b)
{
    load_real_bass();
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 0;
}

__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void)
{
    load_real_bass();
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(BOOL a, void* b, DWORD c, DWORD d, DWORD e, DWORD f)
{
    load_real_bass();
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0; /* NULL = no music */
}

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c)
{
    load_real_bass();
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 0;
}

__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a)
{
    load_real_bass();
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 0;
}

__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d)
{
    load_real_bass();
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}


/* ════════════════════════════════════════════════════════════════════
 * MOD MANAGER
 * ════════════════════════════════════════════════════════════════════ */

#define MAX_MODS    64
#define MAX_NAME    260
#define MAX_LINE    1024
#define POLL_MS     500

/* Game memory addresses (Hamsterball V3.6) */
#define G_APP_ADDR             0x005341E0
#define APP_PROFILE_OFFSET     0x220
#define PROFILE_BOARD_OFFSET   0x0C
#define BOARD_RACE_NAME_OFFSET 0x29B4

/* Context IDs */
#define CTX_MENU     (-1)
#define CTX_UNKNOWN  (-2)
#define CTX_LEVEL    0    /* + index 0-14 */
#define CTX_ARENA    100  /* + index 0-14 */

/* Race names stored at board+0x29B4 (set by LevelBoard_*_ctor) */
static const char* RACE_NAMES[15] = {
    "WARM-UP RACE",      "BEGINNER RACE",    "INTERMEDIATE RACE", "DIZZY RACE",
    "TOWER RACE",        "UP RACE",          "NEON RACE",         "EXPERT RACE",
    "ODD RACE",          "TOOB RACE",        "WOBBLY RACE",       "GLASS RACE",
    "SKY RACE",          "MASTER RACE",      "IMPOSSIBLE RACE"
};

/* Arena names stored at board+0x29B4 (set by ArenaBoard_*_ctor) */
static const char* ARENA_NAMES[15] = {
    "WARM-UP ARENA",     "BEGINNER ARENA",   "INTERMEDIATE ARENA","DIZZY ARENA",
    "TOWER ARENA",       "UP ARENA",         "NEON ARENA",        "EXPERT ARENA",
    "ODD ARENA",         "TOOB ARENA",       "WOBBLY ARENA",      "GLASS ARENA",
    "SKY ARENA",         "MASTER ARENA",     "IMPOSSIBLE ARENA"
};

/* A loaded mod DLL */
typedef struct {
    char    name[MAX_NAME];
    HMODULE handle;
} LoadedMod;

/* Parsed config */
typedef struct {
    char global_mods[MAX_MODS][MAX_NAME];
    int  global_count;

    char level_mods[15][MAX_MODS][MAX_NAME];
    int  level_counts[15];

    char arena_mods[15][MAX_MODS][MAX_NAME];
    int  arena_counts[15];
} ModConfig;

/* Global state */
static char        g_gameDir[MAX_PATH] = ".";
static char        g_modsDir[MAX_PATH];
static LoadedMod   g_loaded[MAX_MODS];
static int         g_loadedCount = 0;
static ModConfig   g_config;
static CRITICAL_SECTION g_modLock;
static FILE*       g_log = NULL;

/* ── Logging ─────────────────────────────────────────────────────── */

static void log_msg(const char* fmt, ...)
{
    if (!g_log) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(g_log, fmt, args);
    va_end(args);
    fflush(g_log);
}

/* ── String helpers ─────────────────────────────────────────────── */

static char* trim(char* s)
{
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (*s == 0) return s;
    char* end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
        *end = 0;
        end--;
    }
    return s;
}

/* Parse a comma-separated list of mod names into out[][], return count */
static int parse_mod_list(char* str, char out[][MAX_NAME], int max)
{
    int count = 0;
    char* p = str;
    while (p && *p && count < max) {
        char* comma = strchr(p, ',');
        if (comma) *comma = 0;

        char* name = trim(p);
        if (*name) {
            strncpy(out[count], name, MAX_NAME - 1);
            out[count][MAX_NAME - 1] = 0;

            /* Append .dll if missing */
            int len = (int)strlen(out[count]);
            if (len < 4 || _stricmp(out[count] + len - 4, ".dll") != 0) {
                if (len + 4 < MAX_NAME) strcat(out[count], ".dll");
            }
            count++;
        }
        p = comma ? comma + 1 : NULL;
    }
    return count;
}

/* ── Config file ─────────────────────────────────────────────────── */

static void generate_default_config(const char* path)
{
    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;

    fprintf(f, "# Hamsterball Mod Manager Configuration\n");
    fprintf(f, "# Lines starting with # are comments.\n");
    fprintf(f, "# Mods not listed here are deactivated by default.\n");
    fprintf(f, "# Separate multiple mods with commas.\n\n");

    fprintf(f, "GLOBAL = \n\n");

    int i;
    for (i = 0; i < 15; i++)
        fprintf(f, "LEVEL %d = \n", i + 1);
    fprintf(f, "\n");
    for (i = 0; i < 15; i++)
        fprintf(f, "ARENA %d = \n", i + 1);

    fclose(f);
}

static void parse_config(const char* configPath)
{
    memset(&g_config, 0, sizeof(g_config));

    FILE* f = NULL;
    if (fopen_s(&f, configPath, "r") != 0 || !f) {
        log_msg("[mod_targets] Config not found, generating default: %s\n", configPath);
        generate_default_config(configPath);
        return;
    }

    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, f)) {
        char* p = trim(line);
        if (*p == 0 || *p == '#') continue;

        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = 0;

        char* section = trim(p);
        char* value   = trim(eq + 1);

        if (_stricmp(section, "GLOBAL") == 0) {
            g_config.global_count = parse_mod_list(value, g_config.global_mods, MAX_MODS);
        }
        else if (_strnicmp(section, "LEVEL", 5) == 0) {
            int idx = atoi(section + 5) - 1;
            if (idx >= 0 && idx < 15)
                g_config.level_counts[idx] = parse_mod_list(value, g_config.level_mods[idx], MAX_MODS);
        }
        else if (_strnicmp(section, "ARENA", 5) == 0) {
            int idx = atoi(section + 5) - 1;
            if (idx >= 0 && idx < 15)
                g_config.arena_counts[idx] = parse_mod_list(value, g_config.arena_mods[idx], MAX_MODS);
        }
    }

    fclose(f);
}

/* ── Level/arena detection ───────────────────────────────────────── */

/* Safely read a pointer from game memory */
static int safe_read_ptr(void* addr, void** out)
{
    if (!addr) return 0;
    if (IsBadReadPtr(addr, 4)) return 0;
    *out = *(void**)addr;
    if (!*out) return 0;
    if (IsBadReadPtr(*out, 4)) return 0;
    return 1;
}

/* Safely read a string from game memory */
static int safe_read_str(char* str)
{
    if (!str) return 0;
    if (IsBadReadPtr(str, 1)) return 0;
    return 1;
}

/*
 * Determine current game context.
 * Returns:
 *   CTX_MENU            — in a menu / no board loaded
 *   CTX_LEVEL + n       — playing race n (0=WarmUp … 14=Impossible)
 *   CTX_ARENA + n       — playing arena n (0=WarmUp … 14=Impossible)
 *   CTX_UNKNOWN         — board exists but race name not recognised
 */
static int get_current_context(void)
{
    void* app = NULL;
    void* profile = NULL;
    void* board = NULL;
    char* raceName = NULL;

    if (!safe_read_ptr((void*)G_APP_ADDR, &app))         return CTX_MENU;
    if (!safe_read_ptr((char*)app + APP_PROFILE_OFFSET, &profile)) return CTX_MENU;
    if (!safe_read_ptr((char*)profile + PROFILE_BOARD_OFFSET, &board)) return CTX_MENU;
    if (!safe_read_ptr((char*)board + BOARD_RACE_NAME_OFFSET, (void**)&raceName)) return CTX_MENU;
    if (!safe_read_str(raceName)) return CTX_MENU;

    /* Check arena first (names contain "ARENA") */
    if (strstr(raceName, "ARENA")) {
        int i;
        for (i = 0; i < 15; i++)
            if (_stricmp(raceName, ARENA_NAMES[i]) == 0) return CTX_ARENA + i;
    }
    else if (strstr(raceName, "RACE")) {
        int i;
        for (i = 0; i < 15; i++)
            if (_stricmp(raceName, RACE_NAMES[i]) == 0) return CTX_LEVEL + i;
    }

    return CTX_UNKNOWN;
}

/* ── Mod load / unload ───────────────────────────────────────────── */

static int mod_in_list(const char* name, char list[][MAX_NAME], int count)
{
    int i;
    for (i = 0; i < count; i++)
        if (_stricmp(name, list[i]) == 0) return 1;
    return 0;
}

/* Should this mod be active in the given context? */
static int should_be_active(const char* name, int context)
{
    /* GLOBAL mods are always active */
    if (mod_in_list(name, g_config.global_mods, g_config.global_count)) return 1;

    if (context >= CTX_LEVEL && context < CTX_LEVEL + 15) {
        int idx = context - CTX_LEVEL;
        if (mod_in_list(name, g_config.level_mods[idx], g_config.level_counts[idx])) return 1;
    }

    if (context >= CTX_ARENA && context < CTX_ARENA + 15) {
        int idx = context - CTX_ARENA;
        if (mod_in_list(name, g_config.arena_mods[idx], g_config.arena_counts[idx])) return 1;
    }

    return 0;
}

static int find_loaded(const char* name)
{
    int i;
    for (i = 0; i < g_loadedCount; i++)
        if (_stricmp(g_loaded[i].name, name) == 0) return i;
    return -1;
}

static void load_mod(const char* name)
{
    if (g_loadedCount >= MAX_MODS) return;
    if (find_loaded(name) >= 0) return;

    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%s\\%s", g_modsDir, name);

    HMODULE hMod = LoadLibraryA(path);
    if (hMod) {
        strncpy(g_loaded[g_loadedCount].name, name, MAX_NAME - 1);
        g_loaded[g_loadedCount].name[MAX_NAME - 1] = 0;
        g_loaded[g_loadedCount].handle = hMod;
        g_loadedCount++;
        log_msg("[mod_targets] LOADED: %s\n", name);
    } else {
        log_msg("[mod_targets] FAILED to load: %s (err=%d path=%s)\n",
                name, GetLastError(), path);
    }
}

static void unload_mod(int index)
{
    if (index < 0 || index >= g_loadedCount) return;

    log_msg("[mod_targets] UNLOADED: %s\n", g_loaded[index].name);

    if (g_loaded[index].handle)
        FreeLibrary(g_loaded[index].handle);

    int i;
    for (i = index; i < g_loadedCount - 1; i++)
        g_loaded[i] = g_loaded[i + 1];
    g_loadedCount--;
}

/* Load all mods needed for a context that aren't already loaded */
static void activate_context(int context)
{
    EnterCriticalSection(&g_modLock);

    int i;

    /* GLOBAL mods */
    for (i = 0; i < g_config.global_count; i++)
        load_mod(g_config.global_mods[i]);

    /* Level mods */
    if (context >= CTX_LEVEL && context < CTX_LEVEL + 15) {
        int idx = context - CTX_LEVEL;
        for (i = 0; i < g_config.level_counts[idx]; i++)
            load_mod(g_config.level_mods[idx][i]);
    }

    /* Arena mods */
    if (context >= CTX_ARENA && context < CTX_ARENA + 15) {
        int idx = context - CTX_ARENA;
        for (i = 0; i < g_config.arena_counts[idx]; i++)
            load_mod(g_config.arena_mods[idx][i]);
    }

    LeaveCriticalSection(&g_modLock);
}

/* Unload mods that should NOT be active in the given context */
static void deactivate_for_context(int context)
{
    EnterCriticalSection(&g_modLock);

    int i = 0;
    while (i < g_loadedCount) {
        if (!should_be_active(g_loaded[i].name, context)) {
            unload_mod(i);
            /* list shifted — don't increment */
        } else {
            i++;
        }
    }

    LeaveCriticalSection(&g_modLock);
}

/* ── Manager thread ──────────────────────────────────────────────── */

static DWORD WINAPI manager_thread(LPVOID param)
{
    /* Wait for game to finish basic initialisation */
    Sleep(2000);

    /* Determine game directory (where bass.dll was loaded from) */
    HMODULE hSelf = NULL;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&manager_thread,
        &hSelf);
    GetModuleFileNameA(hSelf, g_gameDir, MAX_PATH);

    char* slash = strrchr(g_gameDir, '\\');
    if (slash) *slash = 0;

    snprintf(g_modsDir, MAX_PATH, "%s\\mods", g_gameDir);

    /* Open log */
    char logPath[MAX_PATH];
    snprintf(logPath, MAX_PATH, "%s\\mod_targets_log.txt", g_gameDir);
    fopen_s(&g_log, logPath, "a");

    log_msg("\n══════════════════════════════════════════\n");
    log_msg("[mod_targets] Mod Manager v1.0 — started\n");
    log_msg("[mod_targets] Game dir: %s\n", g_gameDir);
    log_msg("[mod_targets] Mods dir: %s\n", g_modsDir);

    /* Parse config (auto-generate if missing) */
    char configPath[MAX_PATH];
    snprintf(configPath, MAX_PATH, "%s\\__mod_targets.txt", g_gameDir);
    parse_config(configPath);

    /* Log config summary */
    log_msg("[mod_targets] Config summary:\n");
    {
        int i, j;
        for (i = 0; i < g_config.global_count; i++)
            log_msg("  GLOBAL: %s\n", g_config.global_mods[i]);
        for (i = 0; i < 15; i++)
            for (j = 0; j < g_config.level_counts[i]; j++)
                log_msg("  LEVEL %d: %s\n", i+1, g_config.level_mods[i][j]);
        for (i = 0; i < 15; i++)
            for (j = 0; j < g_config.arena_counts[i]; j++)
                log_msg("  ARENA %d: %s\n", i+1, g_config.arena_mods[i][j]);
    }

    /* Load GLOBAL mods immediately */
    activate_context(CTX_UNKNOWN);

    log_msg("[mod_targets] Global mods loaded. Monitoring loop starting.\n");

    /* ── Monitoring loop ── */
    int prevContext = CTX_MENU;
    int firstCheck  = 1;

    while (1) {
        Sleep(POLL_MS);

        int ctx = get_current_context();

        if (ctx != prevContext || firstCheck) {
            log_msg("[mod_targets] Context: %d → %d\n", prevContext, ctx);

            deactivate_for_context(ctx);
            activate_context(ctx);

            prevContext = ctx;
            firstCheck  = 0;
        }
    }

    return 0;
}


/* ════════════════════════════════════════════════════════════════════
 * DLL ENTRY POINT
 * ════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        InitializeCriticalSection(&g_bassLock);
        InitializeCriticalSection(&g_modLock);
        /* Start manager thread (don't LoadLibrary from DllMain) */
        {
            HANDLE h = CreateThread(NULL, 0, manager_thread, NULL, 0, NULL);
            if (h) CloseHandle(h);
        }
        break;

    case DLL_PROCESS_DETACH:
        /* Unload all mods on exit */
        EnterCriticalSection(&g_modLock);
        while (g_loadedCount > 0)
            unload_mod(0);
        LeaveCriticalSection(&g_modLock);

        if (g_log) { fclose(g_log); g_log = NULL; }
        DeleteCriticalSection(&g_bassLock);
        DeleteCriticalSection(&g_modLock);
        break;
    }
    return TRUE;
}

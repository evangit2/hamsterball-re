/*
 * mkn_custom_filenames — Custom Race & Arena Level Loader
 * ====================================================
 *
 * Reads "mkn_custom_filenames.txt" from the game directory and replaces the
 * hard-coded level path strings that are PUSHed onto the stack before
 * LoadMeshWorld / Scene_SetupLevelN calls.
 *
 * The game has 30 PUSH imm32 instructions (0x68 + 4-byte string address):
 *   - 15 for Race tracks (tournament progression)
 *   - 15 for Arena tracks (multiplayer Rumble mode)
 *
 * Each PUSH loads a string like "levels\\level1" or "levels\\arena-WarmUp".
 * We allocate our own strings (e.g. "levels\\custom_level") in this DLL's
 * memory and overwrite the 4-byte immediate in each PUSH to point to it.
 *
 * The user edits mkn_custom_filenames.txt to:
 *   1. Rename a level: change "level1" to "my_level" → loads "levels\\my_level.meshworld"
 *   2. Reorder levels: swap entries 3 and 7 → Intermediate takes Neon's slot and vice versa
 *
 * If mkn_custom_filenames.txt is missing, a default one is created next to
 * this DLL, then defaults are used for this run.
 *
 * The " A" prefix before "levels\level5" in the original binary is part of a
 * multi-purpose data block. Our patch points to the clean "levels\level5" string
 * at offset +2, which is the correct null-terminated path.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Wine/Xvfb: MessageBoxA hangs without a visible desktop.
 * Use file-based logging instead. */
static void mod_log(const char *fmt, ...) {
    FILE *f = NULL;
    if (fopen_s(&f, "mkn_custom_filenames_log.txt", "a") != 0 || !f) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
    fclose(f);
}

/* ── BASS proxy exports (REQUIRED — all 10 game imports) ────────────── */
static HMODULE g_hRealBass = NULL;
static HMODULE g_hSelf = NULL;   /* own DLL handle (for config path next to DLL) */

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

typedef int  (__stdcall *BASS_MusicLoad_t)(const char*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(const char* a, DWORD b, DWORD c, DWORD d, DWORD e) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e);
    return 0;
}

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

typedef int  (__stdcall *BASS_Init_t)(int, int, int, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, int b, int c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef void (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) void __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) real_BASS_ChannelStop(a);
}

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}

/* ── Mod: Custom Filenames ─────────────────────────────────────────── */

/* RVA addresses of the 30 PUSH instructions.
 * Each is: 68 XX XX XX 00 (PUSH imm32, 5 bytes).
 * We overwrite bytes [1..4] (the string pointer) with our custom string.
 *
 * Race levels — tournament progression order (1-15):
 */
static const DWORD g_race_push_rvas[15] = {
    0x0d202,  /* #1  Warm-up       → "levels\level1"          */
    0x11115,  /* #2  Beginner      → "levels\levelcascade"    */
    0x0d2c2,  /* #3  Intermediate  → "levels\level2"          */
    0x0d3d4,  /* #4  Dizzy         → "levels\level3"          */
    0x0d712,  /* #5  Tower         → "levels\level4"          */
    0x1158b,  /* #6  Up            → "levels\levelup"         */
    0x162b5,  /* #7  Neon          → "levels\leveldark"       */
    0x0e1d2,  /* #8  Expert        → "levels\level5"          */
    0x0ead3,  /* #9  Odd           → "levels\level6"          */
    0x0fa65,  /* #10 Toob          → "levels\level8"          */
    0x0f3a2,  /* #11 Wobbly        → "levels\level7"          */
    0x17683,  /* #12 Glass         → "levels\levelglass"       */
    0x10874,  /* #13 Sky           → "levels\level9"          */
    0x11fa2,  /* #14 Master        → "levels\level10"         */
    0x17f62,  /* #15 Impossible    → "levels\levelimpossible" */
};

/* Arena levels — Rumble mode order (1-15): */
static const DWORD g_arena_push_rvas[15] = {
    0x13c62,  /* #1  Warm-up       → "levels\arena-WarmUp"        */
    0x13d25,  /* #2  Beginner      → "levels\arena-beginner"      */
    0x141c2,  /* #3  Intermediate  → "levels\arena-intermediate"  */
    0x14282,  /* #4  Dizzy         → "levels\arena-dizzy"         */
    0x144f2,  /* #5  Tower         → "levels\arena-tower"         */
    0x149a2,  /* #6  Up            → "levels\arena-up"            */
    0x16f83,  /* #7  Neon          → "levels\arena-neon"          */
    0x14b52,  /* #8  Expert        → "levels\arena-expert"        */
    0x14d22,  /* #9  Odd           → "levels\arena-Odd"           */
    0x14f45,  /* #10 Toob          → "levels\arena-Toob"          */
    0x153e2,  /* #11 Wobbly        → "levels\arena-Wobbly"        */
    0x17e32,  /* #12 Glass         → "levels\arena-glass"         */
    0x15904,  /* #13 Sky           → "levels\arena-Sky"          */
    0x160c2,  /* #14 Master        → "levels\arena-Master"       */
    0x18582,  /* #15 Impossible    → "levels\arena-impossible"   */
};

/* Maximum length for a level filename (e.g. "level1", "arena-WarmUp", "my_custom_level") */
#define MAX_LEVEL_NAME 64

/* Allocated custom strings — kept alive for game's lifetime */
static char g_race_strings[15][MAX_LEVEL_NAME + 16];   /* "levels\" + name + "\0" */
static char g_arena_strings[15][MAX_LEVEL_NAME + 16];

/* ── Parse mkn_custom_filenames.txt ───────────────────────────────────── */

static void trim(char *s) {
    /* Trim leading whitespace */
    char *p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    /* Trim trailing whitespace/CR/LF */
    int len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' ||
                       s[len-1] == ' '  || s[len-1] == '\t')) {
        s[--len] = '\0';
    }
}

static int parse_config(const char *config_path,
                        char race_names[15][MAX_LEVEL_NAME + 16],
                        char arena_names[15][MAX_LEVEL_NAME + 16]) {
    FILE *f = NULL;
    int parsed_races = 0, parsed_arenas = 0;

    if (fopen_s(&f, config_path, "r") != 0 || !f) {
        mod_log("[mkn_custom_filenames] Could not open config file: %s — using defaults", config_path);
        return 0;
    }

    char line[256];
    int section = 0;  /* 0=none, 1=races, 2=arenas */

    while (fgets(line, sizeof(line), f)) {
        trim(line);

        /* Skip empty lines and comments */
        if (line[0] == '\0' || line[0] == '#') continue;

        /* Check for section headers */
        if (_stricmp(line, "RACES:") == 0 ||
            _stricmp(line, "TOURNAMENT_RACES:") == 0 ||
            _stricmp(line, "PRACTICE_RACES:") == 0) {
            section = 1;
            continue;
        }
        if (_stricmp(line, "ARENAS:") == 0) {
            section = 2;
            continue;
        }

        /* Parse "N=filename" */
        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *num_str = line;
        char *name_str = eq + 1;

        trim(num_str);
        trim(name_str);

        if (name_str[0] == '\0') continue;

        int slot = atoi(num_str);
        if (slot < 1 || slot > 15) continue;

        /* Build full path: "levels\<filename>" */
        if (section == 1) {
            /* Race: "levels\<filename>" */
            snprintf(race_names[slot-1], MAX_LEVEL_NAME + 16,
                     "levels\\%s", name_str);
            parsed_races++;
        } else if (section == 2) {
            /* Arena: "levels\<filename>" */
            snprintf(arena_names[slot-1], MAX_LEVEL_NAME + 16,
                     "levels\\%s", name_str);
            parsed_arenas++;
        }
    }

    fclose(f);

    mod_log("[mkn_custom_filenames] Parsed %d races, %d arenas", parsed_races, parsed_arenas);
    for (int i = 0; i < 15 && i < parsed_races; i++) {
        mod_log("  Race #%d: %s", i+1, race_names[i]);
    }
    for (int i = 0; i < 15 && i < parsed_arenas; i++) {
        mod_log("  Arena #%d: %s", i+1, arena_names[i]);
    }

    return (parsed_races + parsed_arenas);
}

/* ── Apply patches ─────────────────────────────────────────────────── */

static void patch_push_instruction(DWORD push_rva, const char *new_string) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;

    /* Address of the PUSH instruction */
    DWORD push_addr = base + push_rva;

    /* Verify it's a PUSH imm32 (0x68) */
    if (*((BYTE*)push_addr) != 0x68) {
        mod_log("[mkn_custom_filenames] WARNING: expected 0x68 at RVA 0x%05x, got 0x%02x — skipping",
                 push_rva, *((BYTE*)push_addr));
        return;
    }

    /* The 4 bytes after 0x68 are the string pointer (push_addr + 1) */
    DWORD *imm_ptr = (DWORD*)(push_addr + 1);
    DWORD oldProt;

    if (VirtualProtect(imm_ptr, 4, PAGE_READWRITE, &oldProt)) {
        *imm_ptr = (DWORD)new_string;
        VirtualProtect(imm_ptr, 4, oldProt, &oldProt);
    }
}

/* ── Default config (written next to DLL if txt missing) ────────────── */

static const char kDefaultConfig[] =
    "# mkn_custom_filenames — Tournament & Practice & Arena Level Configuration\r\n"
    "# Lines starting with # are comments. Blank lines are ignored.\r\n"
    "# Format: <slot>=<level filename>\r\n"
    "# Rearrange entries to change the tournament/arena order.\r\n"
    "# Delete or comment out a line to use the game's default for that slot.\r\n"
    "#\r\n"
    "# NOTE: Tournament and Practice modes share the same 15 level files.\r\n"
    "\r\n"
    "TOURNAMENT_RACES:\r\n"
    "1=level1\r\n"
    "2=levelcascade\r\n"
    "3=level2\r\n"
    "4=level3\r\n"
    "5=level4\r\n"
    "6=levelup\r\n"
    "7=leveldark\r\n"
    "8=level5\r\n"
    "9=level6\r\n"
    "10=level8\r\n"
    "11=level7\r\n"
    "12=levelglass\r\n"
    "13=level9\r\n"
    "14=level10\r\n"
    "15=levelimpossible\r\n"
    "\r\n"
    "PRACTICE_RACES:\r\n"
    "1=level1\r\n"
    "2=levelcascade\r\n"
    "3=level2\r\n"
    "4=level3\r\n"
    "5=level4\r\n"
    "6=levelup\r\n"
    "7=leveldark\r\n"
    "8=level5\r\n"
    "9=level6\r\n"
    "10=level8\r\n"
    "11=level7\r\n"
    "12=levelglass\r\n"
    "13=level9\r\n"
    "14=level10\r\n"
    "15=levelimpossible\r\n"
    "\r\n"
    "ARENAS:\r\n"
    "1=arena-WarmUp\r\n"
    "2=arena-beginner\r\n"
    "3=arena-intermediate\r\n"
    "4=arena-dizzy\r\n"
    "5=arena-tower\r\n"
    "6=arena-up\r\n"
    "7=arena-neon\r\n"
    "8=arena-expert\r\n"
    "9=arena-Odd\r\n"
    "10=arena-Toob\r\n"
    "11=arena-Wobbly\r\n"
    "12=arena-glass\r\n"
    "13=arena-Sky\r\n"
    "14=arena-Master\r\n"
    "15=arena-impossible\r\n";

/* If path does not exist, create it with defaults. Runs before parse. */
static void ensure_config_exists(const char *path) {
    if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) return;
    FILE *f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) {
        mod_log("[mkn_custom_filenames] Could not create default config: %s", path);
        return;
    }
    fputs(kDefaultConfig, f);
    fclose(f);
    mod_log("[mkn_custom_filenames] Created default config: %s", path);
}

static void apply_mkn_custom_filenames(void) {
    char config_path[MAX_PATH];

    /* Config lives next to THIS dll (bass.dll proxy in game dir). */
    if (g_hSelf && GetModuleFileNameA(g_hSelf, config_path, MAX_PATH) > 0) {
        char *last_slash = strrchr(config_path, '\\');
        if (last_slash) {
            *(last_slash + 1) = '\0';
            strncat(config_path, "mkn_custom_filenames.txt",
                    MAX_PATH - strlen(config_path) - 1);
        } else {
            strcpy_s(config_path, MAX_PATH, "mkn_custom_filenames.txt");
        }
    } else {
        /* Fallback: game directory from Hamsterball.exe module path */
        HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
        if (hExe) {
            char exe_path[MAX_PATH];
            if (GetModuleFileNameA(hExe, exe_path, MAX_PATH) > 0) {
                char *last_slash = strrchr(exe_path, '\\');
                if (last_slash) {
                    *(last_slash + 1) = '\0';
                    snprintf(config_path, MAX_PATH, "%smkn_custom_filenames.txt", exe_path);
                } else {
                    strcpy_s(config_path, MAX_PATH, "mkn_custom_filenames.txt");
                }
            } else {
                strcpy_s(config_path, MAX_PATH, "mkn_custom_filenames.txt");
            }
        } else {
            strcpy_s(config_path, MAX_PATH, "mkn_custom_filenames.txt");
        }
    }

    /* Create default txt next to DLL on first run */
    ensure_config_exists(config_path);

    /* Initialize with defaults (in case config is missing or incomplete) */
    static const char *default_races[15] = {
        "levels\\level1",       "levels\\levelcascade", "levels\\level2",
        "levels\\level3",       "levels\\level4",       "levels\\levelup",
        "levels\\leveldark",    "levels\\level5",       "levels\\level6",
        "levels\\level8",       "levels\\level7",       "levels\\levelglass",
        "levels\\level9",       "levels\\level10",      "levels\\levelimpossible"
    };
    static const char *default_arenas[15] = {
        "levels\\arena-WarmUp",        "levels\\arena-beginner",
        "levels\\arena-intermediate",   "levels\\arena-dizzy",
        "levels\\arena-tower",          "levels\\arena-up",
        "levels\\arena-neon",           "levels\\arena-expert",
        "levels\\arena-Odd",            "levels\\arena-Toob",
        "levels\\arena-Wobbly",         "levels\\arena-glass",
        "levels\\arena-Sky",            "levels\\arena-Master",
        "levels\\arena-impossible"
    };

    /* Copy defaults into our string buffers */
    for (int i = 0; i < 15; i++) {
        strncpy_s(g_race_strings[i], MAX_LEVEL_NAME + 16,
                  default_races[i], _TRUNCATE);
        strncpy_s(g_arena_strings[i], MAX_LEVEL_NAME + 16,
                  default_arenas[i], _TRUNCATE);
    }

    /* Try to parse config file (overwrites defaults where specified) */
    parse_config(config_path, g_race_strings, g_arena_strings);

    /* Patch all 30 PUSH instructions */
    for (int i = 0; i < 15; i++) {
        patch_push_instruction(g_race_push_rvas[i], g_race_strings[i]);
        patch_push_instruction(g_arena_push_rvas[i], g_arena_strings[i]);
    }
}

/* ── BASS proxy init ────────────────────────────────────────────────── */

static void init_bass_proxy(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) return;

    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)
        GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)
        GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_MusicLoad = (BASS_MusicLoad_t)
        GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_Free = (BASS_Free_t)
        GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Init = (BASS_Init_t)
        GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_ChannelStop = (BASS_ChannelStop_t)
        GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)
        GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_SetConfig = (BASS_SetConfig_t)
        GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_Start = (BASS_Start_t)
        GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop = (BASS_Stop_t)
        GetProcAddress(g_hRealBass, "BASS_Stop");
}

/* ── DLL Main ───────────────────────────────────────────────────────── */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            g_hSelf = (HMODULE)hinstDLL;
            init_bass_proxy();
            apply_mkn_custom_filenames();
            break;
    }
    return TRUE;
}

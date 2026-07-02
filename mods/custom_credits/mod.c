/*
 * custom_credits — Editable Credits Screen Text
 * ==============================================
 *
 * Reads "custom_credits.txt" from the game directory and replaces the
 * hard-coded credits strings that are PUSHed onto the stack before
 * AthenaList_Append calls in CreditsScreen_ctor (0x4254e0).
 *
 * The credits screen has 44 lines (30 text + 14 blank separators).
 * Each line is a PUSH imm32 instruction (0x68 + 4-byte string pointer).
 * We overwrite the 4-byte immediate to point to our own string buffer.
 *
 * Config format (custom_credits.txt):
 *   N=text     — Line N (1-44) gets the text after =
 *   #comment   — Comment line, ignored
 *   N=         — Empty text = blank separator line
 *
 * If the config file is missing, it is auto-generated with defaults.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* ── Logging ───────────────────────────────────────────────────────── */
static void mod_log(const char *fmt, ...) {
    FILE *f = NULL;
    if (fopen_s(&f, "custom_credits_log.txt", "a") != 0 || !f) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
    fclose(f);
}

/* ── BASS proxy exports (REQUIRED — all 10 game imports) ────────────── */
static HMODULE g_hRealBass = NULL;

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

/* ── Mod: Custom Credits ───────────────────────────────────────────── */

#define NUM_CREDITS_LINES 44
#define MAX_CREDITS_LEN   80

/* RVA addresses of the 44 PUSH instructions in CreditsScreen_ctor (0x4254e0).
 * Each is: 68 XX XX XX 00 (PUSH imm32, 5 bytes).
 * We overwrite bytes [1..4] (the string pointer) with our custom string. */
static const DWORD g_credits_push_rvas[NUM_CREDITS_LINES] = {
    0x25535,  /* # 1  -RAPTISOFT GAMES PRESENTS */
    0x2556A,  /* # 2  HAMSTERBALL */
    0x25576,  /* # 3  (separator) */
    0x25582,  /* # 4  (separator) */
    0x2558E,  /* # 5  -GAME DESIGN */
    0x2559A,  /* # 6  JOHN RAPTIS */
    0x255A6,  /* # 7  (separator) */
    0x255B2,  /* # 8  -PROGRAMMING */
    0x255BE,  /* # 9  NICK RAPTIS */
    0x255CA,  /* #10  (separator) */
    0x255D6,  /* #11  -PHYSICS */
    0x255E2,  /* #12  PAUL NETTLE */
    0x255EE,  /* #13  *www.fluidstudios.com */
    0x255FA,  /* #14  (separator) */
    0x25606,  /* #15  -LEVEL AND CREATURE DESIGN */
    0x25612,  /* #16  JOHN RAPTIS */
    0x2561E,  /* #17  GARY CLAIR */
    0x2562A,  /* #18  *www.garyclair.com */
    0x25636,  /* #19  (separator) */
    0x25642,  /* #20  -MUSIC */
    0x2564E,  /* #21  SKAVEN */
    0x2565A,  /* #22  *www.futurecrew.com/skaven */
    0x25666,  /* #23  (separator) */
    0x25672,  /* #24  -RODENT MODELS */
    0x2567E,  /* #25  ENDBOSSGAMES */
    0x2568A,  /* #26  *www.endbossgames.com */
    0x25696,  /* #27  (separator) */
    0x256A2,  /* #28  -LEVEL MODELS */
    0x256AE,  /* #29  STEVE WARNER */
    0x256BA,  /* #30  (separator) */
    0x256C6,  /* #31  -PLAY BALANCING */
    0x256D2,  /* #32  BRADY WRIGHT */
    0x256DE,  /* #33  *www.anbsoft.com */
    0x256EA,  /* #34  (separator) */
    0x256F6,  /* #35  -GAME INSTALLER */
    0x25702,  /* #36  BRIAN FIETE */
    0x2570E,  /* #37  *www.popcap.com */
    0x2571A,  /* #38  (separator) */
    0x25726,  /* #39  (separator) */
    0x25732,  /* #40  -AND SPECIAL THANKS TO */
    0x2573E,  /* #41  -ALL THE FINE FOLKS AT */
    0x2574A,  /* #42  -FLIPCODE.COM! */
    0x25756,  /* #43  (separator) */
    0x25762   /* #44  (separator) */
};

/* Default credits text (from original game binary) */
static const char * const g_default_credits[NUM_CREDITS_LINES] = {
    "-RAPTISOFT GAMES PRESENTS",
    "HAMSTERBALL",
    "",
    "",
    "-GAME DESIGN",
    "JOHN RAPTIS",
    "",
    "-PROGRAMMING",
    "NICK RAPTIS",
    "",
    "-PHYSICS",
    "PAUL NETTLE",
    "*www.fluidstudios.com",
    "",
    "-LEVEL AND CREATURE DESIGN",
    "JOHN RAPTIS",
    "GARY CLAIR",
    "*www.garyclair.com",
    "",
    "-MUSIC",
    "SKAVEN",
    "*www.futurecrew.com/skaven",
    "",
    "-RODENT MODELS",
    "ENDBOSSGAMES",
    "*www.endbossgames.com",
    "",
    "-LEVEL MODELS",
    "STEVE WARNER",
    "",
    "-PLAY BALANCING",
    "BRADY WRIGHT",
    "*www.anbsoft.com",
    "",
    "-GAME INSTALLER",
    "BRIAN FIETE",
    "*www.popcap.com",
    "",
    "",
    "-AND SPECIAL THANKS TO",
    "-ALL THE FINE FOLKS AT",
    "-FLIPCODE.COM!",
    "",
    ""
};

/* Allocated custom strings — kept alive for game's lifetime.
 * Each buffer holds up to MAX_CREDITS_LEN chars + null terminator.
 * We use a 2D static array so the pointers are stable. */
static char g_credits_strings[NUM_CREDITS_LINES][MAX_CREDITS_LEN + 1];

/* The empty string for separators — points to a static buffer */
static char g_empty_string[1] = "";

/* ── Config helpers ───────────────────────────────────────────────── */

static void trim(char *s) {
    /* Trim leading whitespace */
    char *p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    /* Trim trailing whitespace/CR/LF */
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' ||
                       s[len-1] == ' '  || s[len-1] == '\t')) {
        s[--len] = '\0';
    }
}

static void generate_default_config(const char *config_path) {
    FILE *f = NULL;
    if (fopen_s(&f, config_path, "w") != 0 || !f) {
        mod_log("[custom_credits] Could not generate default config: %s", config_path);
        return;
    }

    fprintf(f, "# Custom Credits Configuration\n");
    fprintf(f, "# Edit the text after = on each line (1-44).\n");
    fprintf(f, "# Empty lines (N=) become blank separator lines in the credits scroll.\n");
    fprintf(f, "# Lines starting with # are comments.\n\n");

    for (int i = 0; i < NUM_CREDITS_LINES; i++) {
        const char *text = g_default_credits[i];
        if (text[0] == '\0') {
            fprintf(f, "%d=\n", i + 1);
        } else {
            fprintf(f, "%d=%s\n", i + 1, text);
        }
    }

    fclose(f);
    mod_log("[custom_credits] Generated default config: %s", config_path);
}

static int parse_config(const char *config_path) {
    FILE *f = NULL;
    int parsed = 0;

    if (fopen_s(&f, config_path, "r") != 0 || !f) {
        mod_log("[custom_credits] Config not found, generating default: %s", config_path);
        generate_default_config(config_path);
        return 0;
    }

    char line[256];

    while (fgets(line, sizeof(line), f)) {
        trim(line);

        /* Skip empty lines and comments */
        if (line[0] == '\0' || line[0] == '#') continue;

        /* Parse "N=text" */
        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *num_str = line;
        char *text_str = eq + 1;

        trim(num_str);
        trim(text_str);

        int slot = atoi(num_str);
        if (slot < 1 || slot > NUM_CREDITS_LINES) continue;

        /* Copy text into our static buffer */
        int idx = slot - 1;
        strncpy_s(g_credits_strings[idx], MAX_CREDITS_LEN + 1,
                  text_str, _TRUNCATE);
        parsed++;
    }

    fclose(f);
    mod_log("[custom_credits] Parsed %d lines from config", parsed);
    return parsed;
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
        mod_log("[custom_credits] WARNING: expected 0x68 at RVA 0x%05X, got 0x%02X — skipping",
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

static void apply_custom_credits(void) {
    char config_path[MAX_PATH];

    /* Get game directory from Hamsterball.exe module path */
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (hExe) {
        char exe_path[MAX_PATH];
        if (GetModuleFileNameA(hExe, exe_path, MAX_PATH) > 0) {
            char *last_slash = strrchr(exe_path, '\\');
            if (last_slash) {
                *(last_slash + 1) = '\0';
                snprintf(config_path, MAX_PATH, "%scustom_credits.txt", exe_path);
            } else {
                strcpy_s(config_path, MAX_PATH, "custom_credits.txt");
            }
        } else {
            strcpy_s(config_path, MAX_PATH, "custom_credits.txt");
        }
    } else {
        strcpy_s(config_path, MAX_PATH, "custom_credits.txt");
    }

    mod_log("[custom_credits] Config path: %s", config_path);

    /* Initialize with defaults */
    for (int i = 0; i < NUM_CREDITS_LINES; i++) {
        strncpy_s(g_credits_strings[i], MAX_CREDITS_LEN + 1,
                  g_default_credits[i], _TRUNCATE);
    }

    /* Parse config file (overwrites defaults where specified) */
    parse_config(config_path);

    /* Patch all 44 PUSH instructions.
     * For empty strings (separators), point to our g_empty_string buffer
     * instead of the game's 0x4d1354 — functionally identical but keeps
     * things clean. */
    for (int i = 0; i < NUM_CREDITS_LINES; i++) {
        const char *str = g_credits_strings[i];
        if (str[0] == '\0') {
            str = g_empty_string;
        }
        patch_push_instruction(g_credits_push_rvas[i], str);
    }

    mod_log("[custom_credits] All %d lines patched", NUM_CREDITS_LINES);
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
            init_bass_proxy();
            apply_custom_credits();
            break;
    }
    return TRUE;
}

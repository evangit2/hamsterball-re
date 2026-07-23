/*
 * CustomCredits_MinGW.cpp — MinGW HB+ port of custom_credits bass.dll proxy mod.
 *
 * Reads "mkn_plus_custom_credits.txt" from the DLL's folder and replaces the
 * hard-coded credits strings that are PUSHed onto the stack before
 * AthenaList_Append calls in CreditsScreen_ctor (0x4254e0).
 *
 * The credits screen has 44 lines (30 text + 14 blank separators).
 * Each line is a PUSH imm32 instruction (0x68 + 4-byte string pointer).
 * We overwrite the 4-byte immediate to point to our own string buffer.
 *
 * Config format (mkn_plus_custom_credits.txt):
 *   N=text     — Line N (1-44) gets the text after =
 *   #comment   — Comment line, ignored
 *   N=         — Empty text = blank separator line
 *
 * If the config file is missing, it is auto-generated with defaults.
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

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

static char g_configPath[MAX_PATH] = "";
static bool g_pathReady = false;
static bool g_patched = false;
static void* g_storedApi = NULL;

/* ── Config helpers ───────────────────────────────────────────────── */

/* Build config path relative to THIS DLL's folder via VirtualQuery */
static void buildConfigPath(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)buildConfigPath, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) {
                if (*p == '\\' || *p == '/') last = p;
                p++;
            }
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_configPath, dllPath, MAX_PATH - 1);
                nc_strncpy(g_configPath + nc_strlen(g_configPath),
                           "mkn_plus_custom_credits.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "mkn_plus_custom_credits.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_pathReady = true;
}

static void trim(char *s) {
    /* Trim leading whitespace */
    char *p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) {
        size_t len = nc_strlen(p);
        nc_memcpy(s, p, len + 1);
    }
    /* Trim trailing whitespace/CR/LF */
    int len = (int)nc_strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' ||
                       s[len-1] == ' '  || s[len-1] == '\t')) {
        s[--len] = '\0';
    }
}

/* Write a null-terminated string to a file handle */
static void writeString(HANDLE h, const char* s) {
    DWORD written;
    WriteFile(h, s, (DWORD)nc_strlen(s), &written, NULL);
}

static void generateDefaultConfig(void) {
    if (!g_pathReady) return;
    HANDLE h = CreateFileA(g_configPath, GENERIC_WRITE, 0,
                           NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    writeString(h, "# Custom Credits Configuration\r\n");
    writeString(h, "# Edit the text after = on each line (1-44).\r\n");
    writeString(h, "# Empty lines (N=) become blank separator lines in the credits scroll.\r\n");
    writeString(h, "# Lines starting with # are comments.\r\n\r\n");

    for (int i = 0; i < NUM_CREDITS_LINES; i++) {
        const char *text = g_default_credits[i];
        char line[MAX_CREDITS_LEN + 16];
        if (text[0] == '\0') {
            nc_snprintf(line, sizeof(line), "%d=\r\n", i + 1);
        } else {
            nc_snprintf(line, sizeof(line), "%d=%s\r\n", i + 1, text);
        }
        writeString(h, line);
    }

    CloseHandle(h);
}

/* Simple atoi for small numbers */
static int nc_atoi(const char* s) {
    int result = 0;
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result;
}

static void parseConfig(void) {
    if (!g_pathReady) return;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        generateDefaultConfig();
        return;
    }

    char buf[8192];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    /* Skip BOM */
    char* p = buf;
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) {
        p += 3;
    }

    /* Process line by line */
    while (*p) {
        /* Find end of line */
        char* line_start = p;
        while (*p && *p != '\r' && *p != '\n') p++;
        char* line_end = p;

        /* Skip line ending */
        if (*p == '\r') p++;
        if (*p == '\n') p++;

        int line_len = (int)(line_end - line_start);
        if (line_len == 0) continue;

        /* Null-terminate the line */
        *line_end = '\0';
        trim(line_start);

        /* Skip empty lines and comments */
        if (line_start[0] == '\0' || line_start[0] == '#') continue;

        /* Parse "N=text" */
        char* eq = line_start;
        while (*eq && *eq != '=') eq++;
        if (!*eq) continue;

        *eq = '\0';
        char* num_str = line_start;
        char* text_str = eq + 1;

        trim(num_str);
        trim(text_str);

        int slot = nc_atoi(num_str);
        if (slot < 1 || slot > NUM_CREDITS_LINES) continue;

        int idx = slot - 1;
        nc_strncpy(g_credits_strings[idx], text_str, MAX_CREDITS_LEN);
        g_credits_strings[idx][MAX_CREDITS_LEN] = '\0';
    }
}

/* ── Apply patches ─────────────────────────────────────────────────── */

static void patchPushInstruction(DWORD push_rva, const char *new_string) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;

    DWORD push_addr = base + push_rva;

    /* Verify it's a PUSH imm32 (0x68) */
    if (*((BYTE*)push_addr) != 0x68) {
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

static void applyCustomCredits(void) {
    if (g_patched) return;
    g_patched = true;

    buildConfigPath();

    /* Initialize with defaults */
    for (int i = 0; i < NUM_CREDITS_LINES; i++) {
        nc_strncpy(g_credits_strings[i], g_default_credits[i], MAX_CREDITS_LEN);
        g_credits_strings[i][MAX_CREDITS_LEN] = '\0';
    }

    /* Parse config file (overwrites defaults where specified) */
    parseConfig();

    /* Patch all 44 PUSH instructions.
     * For empty strings (separators), point to our g_empty_string buffer. */
    for (int i = 0; i < NUM_CREDITS_LINES; i++) {
        const char *str = g_credits_strings[i];
        if (str[0] == '\0') {
            str = g_empty_string;
        }
        patchPushInstruction(g_credits_push_rvas[i], str);
    }
}

/* ── HB+ vtable ───────────────────────────────────────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Custom Credits"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Ported from bass.dll proxy"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    applyCustomCredits();
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall game_update_impl(void*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

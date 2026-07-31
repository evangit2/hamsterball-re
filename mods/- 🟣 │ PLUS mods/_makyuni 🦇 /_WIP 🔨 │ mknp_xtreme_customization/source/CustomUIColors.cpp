/*
 * CustomUIColors.cpp — HB+ v2.0 Mod
 *
 * Reads mkn_plus_xtreme_customization.txt and patches menu UI colors in Hamsterball.
 *
 * Config format (mkn_plus_xtreme_customization.txt):
 *   - LOADING SCREEN -
 *   "Click here" button - Off = #FFFFFF
 *   "Click here" button - On = #FFFFFF
 *   "Click here" button - Txt = "CLICK HERE TO PLAY!"
 *   LoadingScreen HBLogo - L = #FFFFFFFF
 *   ...
 *   - MAIN MENU -
 *   MainMenu Colors - HBversion = #FFFFFFFF
 *   MainMenu Colors - HBversion - Shadow = #000000FF
 *   MainMenu String - Let's Play = "LET'S PLAY!"
 *   ...
 *
 * Color format:
 *   #RRGGBB   — for RGB-only sites (Vec3_Init, no alpha)
 *   #RRGGBBAA — for RGBA sites (Matrix_Scale4x4, with alpha)
 *
 * String format:
 *   "text here" — quoted string, max length = original string length
 *
 * How it works:
 *   1. Initialize(): creates default config if missing, reads config, installs code caves
 *   2. Code caves redirect PUSH instructions to push global float variables
 *   3. onGameUpdate(): re-reads config every ~2 seconds, updates globals
 *   4. Next frame uses new colors automatically
 *   5. String replacements: VirtualProtect .rdata, write in-place, restore protection
 *
 * Patches 28 color sites + 17 string sites:
 *   - 2x 3-arg Vec3_Init (Click Here buttons, RGB only)
 *   - 26x 4-arg Matrix_Scale4x4 (standard RGBA, including HBversion Shadow)
 *   - 1x 4-arg + CALL (Loader Swirl, all PUSH EBX)
 *   - 1x 4-arg interleaved with FPU (Hamster + ball)
 *   - 17x in-place string replacements (menu text)
 *
 * Build (Visual Studio):
 *   Compile as DLL, link with HamsterballAPI.h
 *   Place in game's Mods\ folder
 *
 * Build (MinGW cross-compile for testing):
 *   See build.sh
 */

#include "HamsterballAPI.h"
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Color Site Definitions
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Cave types */
#define CAVE_RGB         0   /* 3-arg: PUSH [B], PUSH [G], PUSH [R], JMP (23 bytes) */
#define CAVE_RGBA        1   /* 4-arg: PUSH [A], PUSH [B], PUSH [G], PUSH [R], JMP (29 bytes) */
#define CAVE_RGBA_CALL   2   /* 4-arg + CALL: PUSH x4, CALL Matrix_Scale4x4, JMP (34 bytes) */
#define CAVE_RGBA_FPU    3   /* 4-arg interleaved: PUSH x3, FMUL, PUSH, JMP (35 bytes) */

#define MATRIX_SCALE_4X4 0x453150
#define VEC3_INIT        0x00453180

/* FMUL instruction bytes for LS_Hamster_ball interleaved site */
static const unsigned char FMUL_BYTES[] = { 0xD8, 0x8E, 0x5C, 0x2D, 0x00, 0x00 }; /* FMUL [ESI+0x2d5c] */

struct ColorSite {
    const char* key;           /* Config file key (lowercase substring to match) */
    DWORD patchAddr;           /* Address to patch with JMP */
    DWORD returnAddr;          /* Address to JMP back to */
    int patchSize;             /* Bytes to overwrite (>= 5 for JMP) */
    int caveType;              /* CAVE_RGB / CAVE_RGBA / CAVE_RGBA_CALL / CAVE_RGBA_FPU */
    int colorIdx;              /* Index into g_colors array */
};

/* Global color storage: [site_index][R=0, G=1, B=2, A=3] */
#define NUM_COLOR_SITES 28
static float g_colors[NUM_COLOR_SITES][4];

/* All defaults are white (1.0, 1.0, 1.0, 1.0) — matches original game values */
static ColorSite g_sites[] = {
    /* --- Loading Screen --- */
    /* 0: Click Here Off (3-arg Vec3_Init, RGB) */
    {"\"click here\" button - off",  0x0042D5FD, 0x0042D606, 9,  CAVE_RGB,      0},
    /* 1: Click Here On (3-arg Vec3_Init, RGB) */
    {"\"click here\" button - on",   0x0042D624, 0x0042D62D, 9,  CAVE_RGB,      1},
    /* 2: HBLogo Left */
    {"hblogo - l",                   0x0042D375, 0x0042D389, 20, CAVE_RGBA,     2},
    /* 3: HBLogo Right */
    {"hblogo - r",                   0x0042D3D2, 0x0042D3E6, 20, CAVE_RGBA,     3},
    /* 4: Hamster only */
    {"hamster - only",               0x0042D472, 0x0042D486, 20, CAVE_RGBA,     4},
    /* 5: Hamster + ball (interleaved with FPU) */
    {"hamster + ball",               0x0042D4D3, 0x0042D4DD, 10, CAVE_RGBA_FPU, 5},
    /* 6: Loader Swirl (4x PUSH EBX + CALL) */
    {"loader swirl",                 0x0042D5A0, 0x0042D5A9, 9,  CAVE_RGBA_CALL,6},
    /* 7: Background */
    {"background",                   0x0042D2B4, 0x0042D2C8, 20, CAVE_RGBA,     7},
    /* 8: Raptisoft Logo */
    {"raptisoft logo",               0x0042DA60, 0x0042DA74, 20, CAVE_RGBA,     8},

    /* --- Main Menu --- */
    /* 9: HB version text */
    {"hbversion",                    0x00426433, 0x00426441, 14, CAVE_RGBA,     9},
    /* 27: HB version text Shadow (black by default: 0,0,0,1.0) */
    {"hbversion - shadow",           0x0042641A, 0x00426425, 11, CAVE_RGBA,    27},
    /* 10: Button #1 - Let's Play! */
    {"button #1",                    0x0042DE82, 0x0042DE96, 20, CAVE_RGBA,    10},
    /* 11: Button #2 - High Scores */
    {"button #2",                    0x0042DED8, 0x0042DEEC, 20, CAVE_RGBA,    11},
    /* 12: Button #3 - Options */
    {"button #3",                    0x0042DF0D, 0x0042DF21, 20, CAVE_RGBA,    12},
    /* 13: Button #4 - Credits */
    {"button #4",                    0x0042DF42, 0x0042DF56, 20, CAVE_RGBA,    13},
    /* 14: Button #5 - Exit to Desktop */
    {"button #5",                    0x0042E002, 0x0042E016, 20, CAVE_RGBA,    14},
    /* 15: LP Button #1 - Tournament */
    {"lp button #1",                 0x0042E092, 0x0042E0A6, 20, CAVE_RGBA,    15},
    /* 16: LP Button #2 - TimeTrials */
    {"lp button #2",                 0x0042E0E9, 0x0042E0FD, 20, CAVE_RGBA,    16},
    /* 17: LP Button #2b - Locked */
    {"lp button #2b",                0x0042E15E, 0x0042E172, 20, CAVE_RGBA,    17},
    /* 18: LP Button #3 - MirrorTourney */
    {"lp button #3",                 0x0042E12E, 0x0042E142, 20, CAVE_RGBA,    18},
    /* 19: LP Button #4 - PartyGames */
    {"lp button #4",                 0x0042E19C, 0x0042E1B0, 20, CAVE_RGBA,    19},
    /* 20: LP Button #5 - Previous */
    {"lp button #5",                 0x0042E1D1, 0x0042E1E5, 20, CAVE_RGBA,    20},

    /* --- Time Trials Pause Menu --- */
    /* 21: Resume */
    {"button #1 - resume",           0x0042E722, 0x0042E736, 20, CAVE_RGBA,    21},
    /* 22: Restart Race */
    {"button #2 - restart",          0x0042E76F, 0x0042E783, 20, CAVE_RGBA,    22},
    /* 23: Options */
    {"button #3 - options",          0x0042E7A4, 0x0042E7B8, 20, CAVE_RGBA,    23},
    /* 24: Quit Race */
    {"button #4 - quit",             0x0042E7E2, 0x0042E7F6, 20, CAVE_RGBA,    24},

    /* --- 2P Game --- */
    /* 25: Side Strip (both L and R share the same color) */
    {"side strip",                   0x00431A5F, 0x00431A6F, 16, CAVE_RGBA,    25},
    /* 25b: Side Strip Right (mirror, same color index) */
    {"side strip r",                 0x00431AA3, 0x00431AB3, 16, CAVE_RGBA,    25},
    /* 26: Win Strip */
    {"win strip",                    0x0044D68D, 0x0044D69B, 14, CAVE_RGBA,    26},
};

#define NUM_SITES (sizeof(g_sites) / sizeof(g_sites[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * String Site Definitions
 * ═══════════════════════════════════════════════════════════════════════════ */

struct StringSite {
    const char* key;           /* Config file key (lowercase substring to match) */
    DWORD address;             /* Address of string in .rdata */
    int maxLength;             /* Max chars (excluding null) = original string length */
};

/* String addresses verified via Ghidra string search */
static StringSite g_string_sites[] = {
    /* Loading Screen */
    {"button - txt",            0x004D3EAC, 19},  /* "CLICK HERE TO PLAY!" */

    /* Main Menu */
    {"string - let's play",     0x004D3F10, 11},  /* "LET'S PLAY!" */
    {"string - options",        0x004D3F00,  7},  /* "OPTIONS" */
    {"string - credits",        0x004D3EF4,  7},  /* "CREDITS" */
    {"string - exit to desktop",0x004D3EC3, 16},  /* "DEXIT TO DESKTOP" */

    /* Main Menu - Choose a Game */
    {"string - choose a game",  0x004D3FF0, 14},  /* "CHOOSE A GAME!" */
    {"string - tournament",     0x004D3FE0, 10},  /* "TOURNAMENT" */
    {"string - time trials",    0x004D3FCC, 11},  /* "TIME TRIALS" */
    {"string - locked",         0x004D3FBC,  6},  /* "LOCKED" */
    {"string - mirror tournament",0x004D3FA8,18},  /* "MIRROR TOURNAMENT" */
    {"string - party games",    0x004D3F94, 11},  /* "PARTY GAMES" */
    {"string - previous",       0x004D3F88,  8},  /* "PREVIOUS" */

    /* Time Trials Menu */
    {"string - choose a time trial", 0x004D4644, 25}, /* "CHOOSE A TIME TRIAL RACE!" */
    {"string - previous menu", 0x004D426C, 13},  /* "PREVIOUS MENU" */
    {"string - pause - resume", 0x004D410C,  6},  /* "RESUME" */
    {"string - pause - restart",0x004D4198, 12},  /* "RESTART RACE" */
    {"string - pause - quit",   0x004D4188, 14},  /* "QUIT THIS RACE" */
};

#define NUM_STRING_SITES (sizeof(g_string_sites) / sizeof(g_string_sites[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * Config File Management
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_config_path[MAX_PATH] = {0};
static unsigned char* g_cave_mem = NULL;

static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char* p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "mkn_plus_xtreme_customization.txt");
    } else {
        strcpy(g_config_path, "mkn_plus_xtreme_customization.txt");
    }
}

static const char* DEFAULT_CONFIG =
    "- LOADING SCREEN -\r\n"
    "\"Click here\" button - Off = #FFFFFF\r\n"
    "\"Click here\" button - On = #FFFFFF\r\n"
    "\"Click here\" button - Txt = \"CLICK HERE TO PLAY!\"\r\n"
    "LoadingScreen HBLogo - L = #FFFFFFFF\r\n"
    "LoadingScreen HBLogo - R = #FFFFFFFF\r\n"
    "LoadingScreen Hamster - only = #FFFFFFFF\r\n"
    "LoadingScreen Hamster + ball = #FFFFFFFF\r\n"
    "LoadingScreen Loader Swirl = #FFFFFFFF\r\n"
    "LoadingScreen Background = #FFFFFFFF\r\n"
    "LoadingScreen Raptisoft Logo = #FFFFFFFF\r\n"
    "\r\n"
    "\r\n"
    "- MAIN MENU -\r\n"
    "MainMenu Colors - HBversion = #FFFFFFFF\r\n"
    "MainMenu Colors - HBversion - Shadow = #000000FF\r\n"
    "MainMenu Colors - Button #1 - Let's Play! = #FFFFFFFF\r\n"
    "MainMenu Colors - Button #2 - High Scores = #FFFFFFFF\r\n"
    "MainMenu Colors - Button #3 - Options = #FFFFFFFF\r\n"
    "MainMenu Colors - Button #4 - Credits = #FFFFFFFF\r\n"
    "MainMenu Colors - Button #5 - Exit to Desktop = #FFFFFFFF\r\n"
    "MainMenu Colors - LP Button #1 - Tournament = #FFFFFFFF\r\n"
    "MainMenu Colors - LP Button #2 - TimeTrials = #FFFFFFFF\r\n"
    "MainMenu Colors - LP Button #2b - Locked = #FFFFFFFF\r\n"
    "MainMenu Colors - LP Button #3 - MirrorTourney = #FFFFFFFF\r\n"
    "MainMenu Colors - LP Button #4 - PartyGames = #FFFFFFFF\r\n"
    "MainMenu Colors - LP Button #5 - Previous = #FFFFFFFF\r\n"
    "MainMenu String - Let's Play = \"LET'S PLAY!\"\r\n"
    "MainMenu String - Options = \"OPTIONS\"\r\n"
    "MainMenu String - Credits = \"CREDITS\"\r\n"
    "MainMenu String - Exit to Desktop = \"DEXIT TO DESKTOP\"\r\n"
    "MainMenu String - Choose a Game! = \"CHOOSE A GAME!\"\r\n"
    "MainMenu String - Tournament = \"TOURNAMENT\"\r\n"
    "MainMenu String - Time Trials = \"TIME TRIALS\"\r\n"
    "MainMenu String - Locked = \"LOCKED\"\r\n"
    "MainMenu String - Mirror Tournament = \"MIRROR TOURNAMENT\"\r\n"
    "MainMenu String - Party Games = \"PARTY GAMES\"\r\n"
    "MainMenu String - Previous = \"PREVIOUS\"\r\n"
    "\r\n"
    "\r\n"
    "- TOURNEY MENU -\r\n"
    "soon.\r\n"
    "\r\n"
    "\r\n"
    "- MIRROR MENU -\r\n"
    "soon.\r\n"
    "\r\n"
    "\r\n"
    "- TIME TRIALS MENU -\r\n"
    "TimeTrials Pause - Button #1 - Resume = #FFFFFFFF\r\n"
    "TimeTrials Pause - Button #2 - Restart Race = #FFFFFFFF\r\n"
    "TimeTrials Pause - Button #3 - Options = #FFFFFFFF\r\n"
    "TimeTrials Pause - Button #4 - Quit Race = #FFFFFFFF\r\n"
    "TimeTrials String - Choose a Time Trial Race = \"CHOOSE A TIME TRIAL RACE!\"\r\n"
    "TimeTrials String - Pause - Previous Menu = \"PREVIOUS MENU\"\r\n"
    "TimeTrials String - Pause - Resume = \"RESUME\"\r\n"
    "TimeTrials String - Pause - Restart Race = \"RESTART RACE\"\r\n"
    "TimeTrials String - Pause - Quit this Race = \"QUIT THIS RACE\"\r\n"
    "\r\n"
    "\r\n"
    "- 2P GAME -\r\n"
    "2Pmenu - Side Strip = #FFFFFFFF\r\n"
    "2Pgame - Win Strip = #FFFFFFFF\r\n"
    "\r\n"
    "\r\n"
    "- 4P GAME -\r\n"
    "soon.\r\n"
    "\r\n"
    "\r\n"
    "- OTHER -\r\n"
    "soon.\r\n";

static void create_default_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(h, DEFAULT_CONFIG, (DWORD)strlen(DEFAULT_CONFIG), &written, NULL);
            CloseHandle(h);
        }
    } else {
        CloseHandle(h);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hex Color Parser
 * ═══════════════════════════════════════════════════════════════════════════ */

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Parse #RRGGBB or #RRGGBBAA. Returns 1 on success.
 * For #RRGGBB, alpha defaults to 255 (1.0f). */
static int parse_hex_color(const char* text, DWORD* out_hex)
{
    const char* p = text;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '#') p++;
    else if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;

    char buf[16] = {0};
    int digits = 0;
    while (digits < 8 && hex_digit(*p) >= 0) {
        buf[digits++] = *p++;
    }

    if (digits >= 8) {
        /* #RRGGBBAA */
        *out_hex = 0;
        for (int i = 0; i < 8; i++)
            *out_hex = (*out_hex << 4) | hex_digit(buf[i]);
        return 1;
    } else if (digits >= 6) {
        /* #RRGGBB — pad alpha to FF */
        *out_hex = 0;
        for (int i = 0; i < 6; i++)
            *out_hex = (*out_hex << 4) | hex_digit(buf[i]);
        *out_hex = (*out_hex << 8) | 0xFF;
        return 1;
    } else if (digits >= 3) {
        /* #RGB — expand to #RRGGBBAA */
        DWORD r = hex_digit(buf[0]) * 0x11;
        DWORD g = hex_digit(buf[1]) * 0x11;
        DWORD b = hex_digit(buf[2]) * 0x11;
        *out_hex = (r << 24) | (g << 16) | (b << 8) | 0xFF;
        return 1;
    }
    return 0;
}

static void hex_to_floats(DWORD hex, float* r, float* g, float* b, float* a)
{
    *r = ((hex >> 24) & 0xFF) / 255.0f;
    *g = ((hex >> 16) & 0xFF) / 255.0f;
    *b = ((hex >> 8)  & 0xFF) / 255.0f;
    *a = ( hex        & 0xFF) / 255.0f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Quoted String Parser
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Extract content between double quotes after '=' sign.
 * Returns 1 on success, 0 on failure.
 * Output is truncated to maxOut chars (excluding null). */
static int extract_quoted_string(const char* line, char* out, int maxOut)
{
    const char* eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;

    const char* q1 = strchr(eq, '"');
    if (!q1) return 0;
    q1++;
    const char* q2 = strchr(q1, '"');
    if (!q2) return 0;

    int len = (int)(q2 - q1);
    if (len > maxOut) len = maxOut;

    memcpy(out, q1, len);
    out[len] = '\0';
    return 1;
}

/* Check if a line contains a quoted string value (rather than a hex color) */
static int line_has_quoted_string(const char* line)
{
    const char* eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;
    return (*eq == '"');
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config Reader
 * ═══════════════════════════════════════════════════════════════════════════ */

static int line_matches(const char* line, const char* needle)
{
    /* Case-insensitive substring search */
    int needle_len = (int)strlen(needle);
    int line_len = (int)strlen(line);
    if (needle_len == 0 || needle_len > line_len) return 0;

    for (int i = 0; i <= line_len - needle_len; i++) {
        int match = 1;
        for (int j = 0; j < needle_len; j++) {
            char a = line[i + j];
            char b = needle[j];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) { match = 0; break; }
        }
        if (match) return 1;
    }
    return 0;
}

static int line_has_eq(const char* line)
{
    return strchr(line, '=') != NULL;
}

static int is_soon(const char* line)
{
    const char* eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;
    return (_strnicmp(eq, "soon", 4) == 0);
}

/* Extract hex color from after '=' sign */
static int extract_color(const char* line, DWORD* out_hex)
{
    const char* eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;
    if (*eq == '\0' || *eq == '\r' || *eq == '\n') return 0;
    return parse_hex_color(eq, out_hex);
}

/* Apply string replacement: VirtualProtect .rdata, write in-place, restore */
static void apply_string(const StringSite* site, const char* text)
{
    DWORD old_protect;
    if (!VirtualProtect((void*)site->address, site->maxLength + 1,
                        PAGE_READWRITE, &old_protect))
        return;
    memcpy((void*)site->address, text, strlen(text) + 1);
    VirtualProtect((void*)site->address, site->maxLength + 1, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), (void*)site->address, site->maxLength + 1);
}

static void read_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[8192] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;

    /* Parse line by line */
    char* line = buf;
    while (*line) {
        char* nl = line;
        while (*nl && *nl != '\n') nl++;
        int len = (int)(nl - line);
        if (len > 0 && line[len-1] == '\r') len--;

        if (len >= 5 && len < 200 && line_has_eq(line) && !is_soon(line)) {
            /* Check if this is a string line (value starts with ") */
            if (line_has_quoted_string(line)) {
                /* Try matching against string sites */
                for (int i = 0; i < (int)NUM_STRING_SITES; i++) {
                    if (line_matches(line, g_string_sites[i].key)) {
                        char text[256];
                        if (extract_quoted_string(line, text, g_string_sites[i].maxLength)) {
                            apply_string(&g_string_sites[i], text);
                        }
                        break;
                    }
                }
            } else {
                /* Color line — try matching against color sites */
                for (int i = 0; i < (int)NUM_SITES; i++) {
                    /* Skip "side strip r" — it's handled by "side strip" match */
                    if (i == 26) continue; /* index 26 is "side strip r", skip */

                    if (line_matches(line, g_sites[i].key)) {
                        if (i == 25) {
                            DWORD hex;
                            if (extract_color(line, &hex)) {
                                float r, g, b, a;
                                hex_to_floats(hex, &r, &g, &b, &a);
                                g_colors[25][0] = r; g_colors[25][1] = g;
                                g_colors[25][2] = b; g_colors[25][3] = a;
                            }
                            break;
                        }

                        DWORD hex;
                        if (extract_color(line, &hex)) {
                            int idx = g_sites[i].colorIdx;
                            float r, g, b, a;
                            hex_to_floats(hex, &r, &g, &b, &a);
                            g_colors[idx][0] = r; g_colors[idx][1] = g;
                            g_colors[idx][2] = b; g_colors[idx][3] = a;
                        }
                        break;
                    }
                }
            }
        }

        if (*nl == '\n') nl++;
        line = nl;
        if (*line == '\0') break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Cave Builders
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Write a PUSH [addr] instruction (6 bytes: FF 35 <addr>) */
static int write_push_mem(unsigned char* p, float* val)
{
    p[0] = 0xFF; p[1] = 0x35;
    *(DWORD*)(p + 2) = (DWORD)val;
    return 6;
}

/* Write a JMP rel32 instruction (5 bytes: E9 <offset>) */
static int write_jmp(unsigned char* p, DWORD target)
{
    p[0] = 0xE9;
    *(DWORD*)(p + 1) = target - ((DWORD)p + 5);
    return 5;
}

/* Write a CALL rel32 instruction (5 bytes: E8 <offset>) */
static int write_call(unsigned char* p, DWORD target)
{
    p[0] = 0xE8;
    *(DWORD*)(p + 1) = target - ((DWORD)p + 5);
    return 5;
}

/*
 * Build a code cave for a color site.
 * Returns the number of bytes written to the cave buffer.
 */
static int build_cave(unsigned char* cave, const ColorSite* site)
{
    int idx = site->colorIdx;
    float* r = &g_colors[idx][0];
    float* g = &g_colors[idx][1];
    float* b = &g_colors[idx][2];
    float* a = &g_colors[idx][3];
    int p = 0;

    switch (site->caveType) {
    case CAVE_RGB:
        /* 3-arg: PUSH [B], PUSH [G], PUSH [R], JMP back (23 bytes) */
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;

    case CAVE_RGBA:
        /* 4-arg: PUSH [A], PUSH [B], PUSH [G], PUSH [R], JMP back (29 bytes) */
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;

    case CAVE_RGBA_CALL:
        /* 4-arg + CALL: PUSH [A], PUSH [B], PUSH [G], PUSH [R], CALL, JMP back (34 bytes) */
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_call(cave + p, MATRIX_SCALE_4X4);
        p += write_jmp(cave + p, site->returnAddr);
        break;

    case CAVE_RGBA_FPU:
        /* Interleaved: PUSH [A], PUSH [B], PUSH [G], FMUL, PUSH [R], JMP back (35 bytes) */
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        /* Copy FMUL instruction (6 bytes) */
        memcpy(cave + p, FMUL_BYTES, sizeof(FMUL_BYTES));
        p += sizeof(FMUL_BYTES);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    }

    return p;
}

/* Patch a code site: write JMP to cave + NOP padding */
static void patch_site(const ColorSite* site, unsigned char* cave_addr)
{
    DWORD old_protect;
    if (!VirtualProtect((void*)site->patchAddr, site->patchSize,
                        PAGE_EXECUTE_READWRITE, &old_protect))
        return;

    /* Write JMP to cave */
    *(unsigned char*)site->patchAddr = 0xE9;
    *(DWORD*)(site->patchAddr + 1) = (DWORD)cave_addr - (site->patchAddr + 5);

    /* NOP pad remaining bytes */
    for (int i = 5; i < site->patchSize; i++)
        *(unsigned char*)(site->patchAddr + i) = 0x90;

    VirtualProtect((void*)site->patchAddr, site->patchSize, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), (void*)site->patchAddr, site->patchSize);
}

/* Install all code caves */
static void install_patches(void)
{
    /* Calculate total cave memory needed */
    int max_cave_size = 0;
    for (int i = 0; i < (int)NUM_SITES; i++) {
        int size = 0;
        switch (g_sites[i].caveType) {
        case CAVE_RGB:        size = 23; break;
        case CAVE_RGBA:       size = 29; break;
        case CAVE_RGBA_CALL:  size = 34; break;
        case CAVE_RGBA_FPU:   size = 35; break;
        }
        if (size > max_cave_size) max_cave_size = size;
    }

    /* Allocate one page for all caves (more than enough) */
    int total = (int)NUM_SITES * max_cave_size + 64;
    g_cave_mem = (unsigned char*)VirtualAlloc(NULL, total,
                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_cave_mem) return;

    int offset = 0;
    for (int i = 0; i < (int)NUM_SITES; i++) {
        unsigned char* cave = g_cave_mem + offset;
        int cave_size = build_cave(cave, &g_sites[i]);
        patch_site(&g_sites[i], cave);
        offset += cave_size + 4; /* 4-byte alignment padding */
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ v2.0 Mod Class
 * ═══════════════════════════════════════════════════════════════════════════ */

class CustomUIColorsMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    int reload_counter = 0;

public:
    const char* GetModName() override    { return "Custom UI Colors"; }
    const char* GetAuthorName() override { return "MAKYUNI"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        /* Initialize all colors to white (1.0, 1.0, 1.0, 1.0) */
        for (int i = 0; i < NUM_COLOR_SITES; i++) {
            g_colors[i][0] = 1.0f; /* R */
            g_colors[i][1] = 1.0f; /* G */
            g_colors[i][2] = 1.0f; /* B */
            g_colors[i][3] = 1.0f; /* A */
        }

        /* Set shadow default to black (0, 0, 0, 1.0) */
        g_colors[27][0] = 0.0f; /* R */
        g_colors[27][1] = 0.0f; /* G */
        g_colors[27][2] = 0.0f; /* B */
        g_colors[27][3] = 1.0f; /* A */

        /* Set up config and patches */
        init_config_path();
        create_default_config();
        read_config();
        install_patches();
    }

    void onGameUpdate() override {
        /* Reload config every ~2 seconds (120 ticks at 60fps) */
        if (++reload_counter >= 120) {
            reload_counter = 0;
            read_config();
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new CustomUIColorsMod();
}

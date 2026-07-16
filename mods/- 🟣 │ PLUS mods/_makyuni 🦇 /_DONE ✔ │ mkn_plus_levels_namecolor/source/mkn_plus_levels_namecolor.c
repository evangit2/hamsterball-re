/*
 * mkn_plus_levels_namecolor.c — BASS.dll proxy mod
 *
 * Per-level and per-arena name + color customization.
 *
 * Features:
 *   1. LEVEL colors (timer blot during gameplay) — runtime polling board+0x1508
 *   2. LEVEL menu colors (race selection text) — patching PUSH floats in PracticeMenu_ctor
 *   3. LEVEL names (race selection text string) — patching PUSH string pointers
 *   4. ARENA colors (timer blot during gameplay) — runtime polling board+0x1508
 *   5. ARENA menu colors (arena selection text) — patching PUSH floats in ArenaMenu_ctor
 *   6. ARENA names (arena selection text string) — patching PUSH string pointers
 *
 * Config file: mkn_plus_levels_namecolor.txt (next to bass.dll)
 * Sections: [LEVELS] and [ARENAS]
 * Prefixes: name: (string), board: (timer only), menu: (menu only), none (both)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mkn_plus_levels_namecolor.c \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
 * ═══════════════════════════════════════════════════════════════════════════ */

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
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
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
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

/* Extra stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }
    if (g_hRealBass) {
        #define LOAD(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)
        LOAD(BASS_ChannelSetAttributes);
        LOAD(BASS_MusicPlayEx);
        LOAD(BASS_SetConfig);
        LOAD(BASS_Init);
        LOAD(BASS_Free);
        LOAD(BASS_Start);
        LOAD(BASS_Stop);
        LOAD(BASS_ErrorGetCode);
        LOAD(BASS_MusicLoad);
        LOAD(BASS_ChannelStop);
        #undef LOAD
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level/Arena Definitions
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_PTR_ADDR     0x005341E0
#define BOARD_COLOR_R    0x1508
#define BOARD_COLOR_G    0x150C
#define BOARD_COLOR_B    0x1510
#define BOARD_COLOR_A    0x1514
#define NUM_LEVELS       15
#define NAME_BUF_SIZE    64

typedef struct {
    const char *name;       /* config key name */
    /* Menu color patch addresses (unlocked branch PUSH operands) */
    DWORD menu_a_addr;
    DWORD menu_b_addr;
    DWORD menu_g_addr;
    DWORD menu_r_addr;
    int   code_cave;        /* 1 = needs code cave (has 6a 00 push) */
    DWORD push_seq_start;   /* code cave: start of push sequence */
    int   push_seq_size;    /* code cave: total bytes */
    DWORD return_addr;      /* code cave: CALL address to jump back to */
    /* Name string patch */
    DWORD name_push_addr;   /* address of PUSH <string_ptr> instruction */
    const char *default_name; /* default display name */
    const char *board_name; /* board+0x29b4 string for runtime ID */
} EntryDef;

/* ── PracticeMenu_ctor (0x42EA30) — Level Race Menu ────────────────────── */
static EntryDef g_levels[NUM_LEVELS] = {
    /* 0: WarmUp */
    {"WarmUp", 0x42EE85, 0x42EE8A, 0x42EE8F, 0x42EE94, 0, 0,0,0,
     0x42EEA7, "WARM-UP RACE", "WARM-UP RACE"},
    /* 1: Beginner */
    {"Beginner", 0x42EEC4, 0x42EEC9, 0x42EECE, 0x42EED3, 0, 0,0,0,
     0x42EEE1, "BEGINNER RACE", "BEGINNER RACE"},
    /* 2: Intermediate */
    {"Intermediate", 0x42EEFE, 0x42EF03, 0x42EF08, 0x42EF0D, 0, 0,0,0,
     0x42EF1B, "INTERMEDIATE RACE", "INTERMEDIATE RACE"},
    /* 3: Dizzy */
    {"Dizzy", 0x42EF51, 0x42EF56, 0x42EF5B, 0x42EF60, 0, 0,0,0,
     0x42EFA0, "DIZZY RACE", "DIZZY RACE"},
    /* 4: Tower */
    {"Tower", 0x42EFCB, 0x42EFD0, 0x42EFD5, 0x42EFDA, 0, 0,0,0,
     0x42F01A, "TOWER RACE", "TOWER RACE"},
    /* 5: Up */
    {"Up", 0x42F045, 0x42F04A, 0x42F04F, 0x42F054, 0, 0,0,0,
     0x42F094, "UP RACE", "UP RACE"},
    /* 6: Neon — code cave (B=push 0) */
    {"Neon", 0,0,0,0, 1, 0x42F0C7, 17, 0x42F0D8,
     0x42F114, "NEON RACE", "NEON RACE"},
    /* 7: Expert */
    {"Expert", 0x42F13F, 0x42F144, 0x42F149, 0x42F14E, 0, 0,0,0,
     0x42F18E, "EXPERT RACE", "EXPERT RACE"},
    /* 8: Odd — code cave (B=push 0) */
    {"Odd", 0,0,0,0, 1, 0x42F1B8, 17, 0x42F1C9,
     0x42F205, "ODD RACE", "ODD RACE"},
    /* 9: Toob */
    {"Toob", 0x42F239, 0x42F23E, 0x42F243, 0x42F248, 0, 0,0,0,
     0x42F288, "TOOB RACE", "TOOB RACE"},
    /* 10: Wobbly */
    {"Wobbly", 0x42F2B3, 0x42F2B8, 0x42F2BD, 0x42F2C2, 0, 0,0,0,
     0x42F302, "WOBBLY RACE", "WOBBLY RACE"},
    /* 11: Glass */
    {"Glass", 0x42F32D, 0x42F332, 0x42F337, 0x42F33C, 0, 0,0,0,
     0x42F37C, "GLASS RACE", "GLASS RACE"},
    /* 12: Sky */
    {"Sky", 0x42F3B0, 0x42F3B5, 0x42F3BA, 0x42F3BF, 0, 0,0,0,
     0x42F3FF, "SKY RACE", "SKY RACE"},
    /* 13: Master */
    {"Master", 0x42F42A, 0x42F42F, 0x42F434, 0x42F439, 0, 0,0,0,
     0x42F479, "MASTER RACE", "MASTER RACE"},
    /* 14: Impossible — code cave (B=push 0, G=push 0) */
    {"Impossible", 0,0,0,0, 1, 0x42F4A3, 14, 0x42F4B1,
     0x42F4ED, "IMPOSSIBLE RACE", "IMPOSSIBLE RACE"},
};

/* ── ArenaMenu_ctor (0x42FC40) — Arena Rumble Menu ─────────────────────── */
static EntryDef g_arenas[NUM_LEVELS] = {
    /* 0: WarmUp Arena */
    {"WarmUp", 0x42FC73, 0x42FC78, 0x42FC7D, 0x42FC82, 0, 0,0,0,
     0x42FC9E, "WARM-UP ARENA", "WARM-UP ARENA"},
    /* 1: Beginner Arena */
    {"Beginner", 0x42FCB6, 0x42FCBB, 0x42FCC0, 0x42FCC5, 0, 0,0,0,
     0x42FCD3, "BEGINNER ARENA", "BEGINNER ARENA"},
    /* 2: Intermediate Arena */
    {"Intermediate", 0x42FCEB, 0x42FCF0, 0x42FCF5, 0x42FCFA, 0, 0,0,0,
     0x42FD08, "INTERMEDIATE ARENA", "INTERMEDIATE ARENA"},
    /* 3: Dizzy Arena */
    {"Dizzy", 0x42FD5B, 0x42FD60, 0x42FD65, 0x42FD6A, 0, 0,0,0,
     0x42FDAA, "DIZZY ARENA", "DIZZY ARENA"},
    /* 4: Tower Arena */
    {"Tower", 0x42FDD0, 0x42FDD5, 0x42FDDA, 0x42FDDF, 0, 0,0,0,
     0x42FE1F, "TOWER ARENA", "TOWER ARENA"},
    /* 5: Up Arena */
    {"Up", 0x42FE45, 0x42FE4A, 0x42FE4F, 0x42FE54, 0, 0,0,0,
     0x42FE94, "UP ARENA", "UP ARENA"},
    /* 6: Neon Arena — code cave (B=push 0) */
    {"Neon", 0,0,0,0, 1, 0x42FEC2, 17, 0x42FED3,
     0x42FF0F, "NEON ARENA", "NEON ARENA"},
    /* 7: Expert Arena */
    {"Expert", 0x42FF35, 0x42FF3A, 0x42FF3F, 0x42FF44, 0, 0,0,0,
     0x42FF84, "EXPERT ARENA", "EXPERT ARENA"},
    /* 8: Odd Arena — code cave (B=push 0) */
    {"Odd", 0,0,0,0, 1, 0x42FFA9, 17, 0x42FFBA,
     0x42FFF6, "ODD ARENA", "ODD ARENA"},
    /* 9: Toob Arena */
    {"Toob", 0x430025, 0x43002A, 0x43002F, 0x430034, 0, 0,0,0,
     0x430074, "TOOB ARENA", "TOOB ARENA"},
    /* 10: Wobbly Arena */
    {"Wobbly", 0x43009A, 0x43009F, 0x4300A4, 0x4300A9, 0, 0,0,0,
     0x4300E9, "WOBBLY ARENA", "WOBBLY ARENA"},
    /* 11: Glass Arena */
    {"Glass", 0x43010F, 0x430114, 0x430119, 0x43011E, 0, 0,0,0,
     0x43015E, "GLASS ARENA", "GLASS ARENA"},
    /* 12: Sky Arena */
    {"Sky", 0x43018D, 0x430192, 0x430197, 0x43019C, 0, 0,0,0,
     0x4301DC, "SKY ARENA", "SKY ARENA"},
    /* 13: Master Arena */
    {"Master", 0x430202, 0x430207, 0x43020C, 0x430211, 0, 0,0,0,
     0x430251, "MASTER ARENA", "MASTER ARENA"},
    /* 14: Impossible Arena */
    {"Impossible", 0x430277, 0x43027C, 0x430281, 0x430286, 0, 0,0,0,
     0x4302C6, "IMPOSSIBLE ARENA", "IMPOSSIBLE ARENA"},
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Config Storage
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int  valid;      /* 1 if color found in config */
    DWORD rgb;       /* 0xRRGGBB */
} ColorEntry;

typedef struct {
    int  valid;      /* 1 if name found in config */
    char text[NAME_BUF_SIZE];
} NameEntry;

static ColorEntry g_level_board_colors[NUM_LEVELS];
static ColorEntry g_level_menu_colors[NUM_LEVELS];
static NameEntry  g_level_names[NUM_LEVELS];
static ColorEntry g_arena_board_colors[NUM_LEVELS];
static ColorEntry g_arena_menu_colors[NUM_LEVELS];
static NameEntry  g_arena_names[NUM_LEVELS];

static char g_config_path[MAX_PATH] = {0};
static int  g_caves_installed = 0;

/* Name string buffers — allocated at init, patched into PUSH operands */
static char *g_name_buffers = NULL;  /* 30 * NAME_BUF_SIZE bytes */

/* ═══════════════════════════════════════════════════════════════════════════
 * Utility
 * ═══════════════════════════════════════════════════════════════════════════ */

static char to_lower(char c) { return (c >= 'A' && c <= 'Z') ? (c + 32) : c; }
static int str_eq_ci(const char *a, const char *b) {
    while (*a && *b) { if (to_lower(*a) != to_lower(*b)) return 0; a++; b++; }
    return (*a == 0 && *b == 0);
}
static int str_eq_ci_n(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        if (to_lower(a[i]) != to_lower(b[i])) return 0;
        if (a[i] == 0) return 0;
    }
    return 1;
}
static int find_entry(const char *name) {
    for (int i = 0; i < NUM_LEVELS; i++)
        if (str_eq_ci(name, g_levels[i].name)) return i;
    return -1;
}

static DWORD parse_hex(const char *s)
{
    if (*s == '#') s++;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
    DWORD hex = 0; int digits = 0;
    while (digits < 6 && *s) {
        char c = to_lower(*s); int val;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else break;
        hex = (hex << 4) | val; digits++; s++;
    }
    if (digits >= 6) return hex;
    if (digits >= 3) {
        int r = ((hex >> 8) & 0xF) * 0x11;
        int g = ((hex >> 4) & 0xF) * 0x11;
        int b = (hex & 0xF) * 0x11;
        return ((DWORD)r << 16) | ((DWORD)g << 8) | b;
    }
    return 0xFFFFFFFF;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config Parser
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Current section: 0=LEVELS, 1=ARENAS */
static int g_section = 0;

static void parse_config_line(char *line)
{
    while (*line == ' ' || *line == '\t') line++;
    if (*line == ';' || *line == '#' || *line == '\r' || *line == '\n' || *line == '\0')
        return;

    /* Section detection: look for "LEVELS" or "ARENAS" in comment lines */
    /* (handled separately in read_config) */

    /* Check prefix: "name:", "board:", "menu:" */
    int prefix = 0;  /* 0=color(both), 1=board, 2=menu, 3=name */
    if (str_eq_ci_n(line, "name:", 5))  { prefix = 3; line += 5; }
    else if (str_eq_ci_n(line, "board:", 6)) { prefix = 1; line += 6; }
    else if (str_eq_ci_n(line, "menu:", 5))  { prefix = 2; line += 5; }

    /* Find '=' */
    char *eq = strchr(line, '=');
    if (!eq) return;
    *eq = '\0';

    /* Trim level name */
    char *name = line;
    char *end = eq - 1;
    while (end > name && (*end == ' ' || *end == '\t')) { *end = '\0'; end--; }

    char *val = eq + 1;
    while (*val == ' ' || *val == '\t') val++;

    int idx = find_entry(name);
    if (idx < 0) return;

    /* Determine target arrays based on section */
    ColorEntry *board_arr = (g_section == 0) ? g_level_board_colors : g_arena_board_colors;
    ColorEntry *menu_arr  = (g_section == 0) ? g_level_menu_colors  : g_arena_menu_colors;
    NameEntry  *name_arr  = (g_section == 0) ? g_level_names        : g_arena_names;

    if (prefix == 3) {
        /* Name string: parse quoted string */
        if (*val != '"') return;
        val++;
        int len = 0;
        while (*val && *val != '"' && len < NAME_BUF_SIZE - 1) {
            name_arr[idx].text[len++] = *val++;
        }
        name_arr[idx].text[len] = '\0';
        name_arr[idx].valid = 1;
    } else {
        DWORD color = parse_hex(val);
        if (color == 0xFFFFFFFF) return;
        if (prefix == 0 || prefix == 1) {
            board_arr[idx].valid = 1;
            board_arr[idx].rgb = color;
        }
        if (prefix == 0 || prefix == 2) {
            menu_arr[idx].valid = 1;
            menu_arr[idx].rgb = color;
        }
    }
}

static void read_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[16384] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;

    /* Clear all entries */
    for (int i = 0; i < NUM_LEVELS; i++) {
        g_level_board_colors[i].valid = 0;
        g_level_menu_colors[i].valid = 0;
        g_level_names[i].valid = 0;
        g_arena_board_colors[i].valid = 0;
        g_arena_menu_colors[i].valid = 0;
        g_arena_names[i].valid = 0;
    }
    g_section = 0;  /* default to LEVELS */

    /* Parse line by line */
    char *p = buf;
    while (*p) {
        char *nl = p;
        while (*nl && *nl != '\n') nl++;

        /* Check for section header in comment lines */
        char *line_start = p;
        while (*line_start == ' ' || *line_start == '\t') line_start++;
        if (*line_start == '#' || *line_start == ';') {
            /* Section headers: skip comment marker, whitespace, and decoration
             * chars (=, -, *) to find the first word, then check for LEVELS/ARENAS */
            char *content = line_start + 1;
            while (*content == ' ' || *content == '\t' || *content == '=' ||
                   *content == '-' || *content == '*') content++;
            if (content[0]=='L' && content[1]=='E' && content[2]=='V' &&
                content[3]=='E' && content[4]=='L' && content[5]=='S')
                g_section = 0;
            else if (content[0]=='A' && content[1]=='R' && content[2]=='E' &&
                     content[3]=='N' && content[4]=='A' && content[5]=='S')
                g_section = 1;
        }

        char saved = *nl;
        *nl = '\0';
        parse_config_line(p);
        *nl = saved;
        if (*nl == '\n') nl++;
        p = nl;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Patching
 * ═══════════════════════════════════════════════════════════════════════════ */

static int patch_float(DWORD addr, float value) {
    DWORD old;
    if (!VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &old)) return 0;
    *(float*)addr = value;
    VirtualProtect((void*)addr, 4, old, &old);
    return 1;
}
static int patch_byte(DWORD addr, BYTE value) {
    DWORD old;
    if (!VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &old)) return 0;
    *(BYTE*)addr = value;
    VirtualProtect((void*)addr, 1, old, &old);
    return 1;
}
static int patch_dword(DWORD addr, DWORD value) {
    DWORD old;
    if (!VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &old)) return 0;
    *(DWORD*)addr = value;
    VirtualProtect((void*)addr, 4, old, &old);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Cave System (for levels/arenas with 6a 00 push-0 instructions)
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE *g_cave_mem = NULL;
static int g_cave_offset = 0;

static int init_code_cave_page(void) {
    if (g_cave_mem) return 1;
    g_cave_mem = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    return g_cave_mem != NULL;
}
static BYTE* cave_alloc(int size) {
    if (!g_cave_mem || g_cave_offset + size > 4096) return NULL;
    BYTE *p = g_cave_mem + g_cave_offset;
    g_cave_offset += size;
    return p;
}

static int install_code_cave(EntryDef *e) {
    if (!init_code_cave_page()) return 0;
    BYTE *cave = cave_alloc(25);
    if (!cave) return 0;

    cave[0] = 0x68; *(DWORD*)(cave+1) = 0;     /* push A */
    cave[5] = 0x68; *(DWORD*)(cave+6) = 0;     /* push B */
    cave[10] = 0x68; *(DWORD*)(cave+11) = 0;   /* push G */
    cave[15] = 0x68; *(DWORD*)(cave+16) = 0;   /* push R */
    cave[20] = 0xE9;
    *(DWORD*)(cave+21) = e->return_addr - ((DWORD)cave + 25);

    DWORD orig = e->push_seq_start;
    int sz = e->push_seq_size;
    /* Write JMP rel32 in one shot: E9 + 4-byte displacement */
    patch_byte(orig, 0xE9);
    patch_dword(orig + 1, (DWORD)cave - (orig + 5));
    for (int i = 5; i < sz; i++) patch_byte(orig + i, 0x90);

    e->menu_a_addr = (DWORD)(cave + 1);
    e->menu_b_addr = (DWORD)(cave + 6);
    e->menu_g_addr = (DWORD)(cave + 11);
    e->menu_r_addr = (DWORD)(cave + 16);
    return 1;
}

static void install_all_code_caves(void) {
    if (g_caves_installed) return;
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (g_levels[i].code_cave)  install_code_cave(&g_levels[i]);
        if (g_arenas[i].code_cave) install_code_cave(&g_arenas[i]);
    }
    g_caves_installed = 1;
}

/* Patch all menu colors */
static void patch_menu_colors(EntryDef *entries, ColorEntry *colors) {
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (!colors[i].valid) continue;
        float r = ((colors[i].rgb >> 16) & 0xFF) / 255.0f;
        float g = ((colors[i].rgb >> 8) & 0xFF) / 255.0f;
        float b = (colors[i].rgb & 0xFF) / 255.0f;
        float a = 1.0f;
        patch_float(entries[i].menu_a_addr, a);
        patch_float(entries[i].menu_b_addr, b);
        patch_float(entries[i].menu_g_addr, g);
        patch_float(entries[i].menu_r_addr, r);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Name String Patching
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_name_buffers(void) {
    if (g_name_buffers) return;
    g_name_buffers = (char*)VirtualAlloc(NULL, 30 * NAME_BUF_SIZE,
                                         MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!g_name_buffers) return;

    /* Copy default names and patch PUSH operands */
    for (int i = 0; i < NUM_LEVELS; i++) {
        char *buf = g_name_buffers + i * NAME_BUF_SIZE;
        lstrcpyA(buf, g_levels[i].default_name);
        patch_dword(g_levels[i].name_push_addr + 1, (DWORD)buf);
    }
    for (int i = 0; i < NUM_LEVELS; i++) {
        char *buf = g_name_buffers + (15 + i) * NAME_BUF_SIZE;
        lstrcpyA(buf, g_arenas[i].default_name);
        patch_dword(g_arenas[i].name_push_addr + 1, (DWORD)buf);
    }
}

static void update_name_buffers(void) {
    if (!g_name_buffers) return;
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (g_level_names[i].valid) {
            char *buf = g_name_buffers + i * NAME_BUF_SIZE;
            lstrcpyA(buf, g_level_names[i].text);
        }
    }
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (g_arena_names[i].valid) {
            char *buf = g_name_buffers + (15 + i) * NAME_BUF_SIZE;
            lstrcpyA(buf, g_arena_names[i].text);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Board Color Polling (runtime timer blot colors)
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD find_board(void) {
    DWORD app = *(DWORD*)APP_PTR_ADDR;
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)app, 0x300)) return 0;

    DWORD profile = *(DWORD*)((BYTE*)app + 0x220);
    if (profile && !IsBadReadPtr((void*)profile, 0x20)) {
        DWORD board = *(DWORD*)((BYTE*)profile + 0x0C);
        if (board && board > 0x10000 && !IsBadReadPtr((void*)board, 0x2000)) {
            float test = *(float*)((BYTE*)board + BOARD_COLOR_R);
            if (test >= 0.0f && test <= 1.0f) return board;
        }
    }
    for (int off = 0x100; off < 0xA00; off += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + off);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x2000)) continue;
        float r = *(float*)((BYTE*)candidate + BOARD_COLOR_R);
        float g = *(float*)((BYTE*)candidate + BOARD_COLOR_G);
        float b = *(float*)((BYTE*)candidate + BOARD_COLOR_B);
        if (r >= 0.0f && r <= 1.0f && g >= 0.0f && g <= 1.0f &&
            b >= 0.0f && b <= 1.0f) return candidate;
    }
    return 0;
}

static void apply_board_colors(void) {
    DWORD board = find_board();
    if (!board) return;
    if (IsBadReadPtr((void*)(board + 0x29B4), 4)) return;
    char *board_name = *(char**)(board + 0x29B4);
    if (!board_name || IsBadReadPtr(board_name, 4)) return;

    /* Try matching level names first, then arena names */
    ColorEntry *colors = NULL;
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (str_eq_ci(board_name, g_levels[i].board_name)) {
            colors = &g_level_board_colors[i];
            break;
        }
    }
    if (!colors) {
        for (int i = 0; i < NUM_LEVELS; i++) {
            if (str_eq_ci(board_name, g_arenas[i].board_name)) {
                colors = &g_arena_board_colors[i];
                break;
            }
        }
    }
    if (!colors || !colors->valid) return;

    float r = ((colors->rgb >> 16) & 0xFF) / 255.0f;
    float g = ((colors->rgb >> 8) & 0xFF) / 255.0f;
    float b = (colors->rgb & 0xFF) / 255.0f;

    if (!IsBadWritePtr((void*)(board + BOARD_COLOR_R), 4)) {
        *(float*)(board + BOARD_COLOR_R) = r;
        *(float*)(board + BOARD_COLOR_G) = g;
        *(float*)(board + BOARD_COLOR_B) = b;
        *(float*)(board + BOARD_COLOR_A) = 1.0f;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config File Generation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void create_default_config(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); return; }

    const char *header =
        "# Level Names and Colors Config File\n"
        "# Format1: LevelName= #RRGGBB  (hex RGB, like HTML colors)\n"
        "# Format2: LevelName= \"string\" (text string between \" \")\n"
        "# Lines starting with ; or # are comments\n"
        "#\n"
        "# PREFIXES:\n"
        "# (Under the \"LEVELS\" section)\n"
        "#   LevelName=        #RRGGBB  -- applies to BOTH timer and menu\n"
        "#   board:LevelName=  #RRGGBB  -- only timer oval/text during gameplay\n"
        "#   menu:LevelName=   #RRGGBB  -- only race selection menu text\n"
        "#   name:LevelName=   (string) -- name of the Race.\n"
        "#\n"
        "# (Under the \"ARENAS\" section)\n"
        "#   ArenaName=        #RRGGBB  -- applies to BOTH timer and menu\n"
        "#   board:ArenaName=  #RRGGBB  -- only timer oval/text during gameplay\n"
        "#   menu:ArenaName=   #RRGGBB  -- only arena selection menu text\n"
        "#   name:ArenaName=   (string) -- name of the Arena.\n"
        "#\n"
        "# Re-reads every 2 seconds. Edit at runtime!\n\n\n";

    DWORD written;
    h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    WriteFile(h, header, lstrlenA(header), &written, NULL);

    const char *levels_section =
        "# ============== LEVELS ==============\n\n"
        "# - NAMES - #\n";
    WriteFile(h, levels_section, lstrlenA(levels_section), &written, NULL);

    const char *lvl_names[] = {
        "name:WarmUp=\t\t\"WARM-UP RACE\"\n",
        "name:Beginner=\t\t\"BEGINNER RACE\"\n",
        "name:Intermediate=\t\"INTERMEDIATE RACE\"\n",
        "name:Dizzy=\t\t\"DIZZY RACE\"\n",
        "name:Tower=\t\t\"TOWER RACE\"\n",
        "name:Up=\t\t\"UP RACE\"\n",
        "name:Neon=\t\t\"NEON RACE\"\n",
        "name:Expert=\t\t\"EXPERT RACE\"\n",
        "name:Odd=\t\t\"ODD RACE\"\n",
        "name:Toob=\t\t\"TOOB RACE\"\n",
        "name:Wobbly=\t\t\"WOBBLY RACE\"\n",
        "name:Glass=\t\t\"GLASS RACE\"\n",
        "name:Sky=\t\t\"SKY RACE\"\n",
        "name:Master=\t\t\"MASTER RACE\"\n",
        "name:Impossible=\t\"IMPOSSIBLE RACE\"\n",
    };
    for (int i = 0; i < 15; i++)
        WriteFile(h, lvl_names[i], lstrlenA(lvl_names[i]), &written, NULL);

    const char *colors_header = "\n# - COLORS - #\n";
    WriteFile(h, colors_header, lstrlenA(colors_header), &written, NULL);

    const char *lvl_colors[] = {
        "WarmUp=\t\t\t#FFFFFF\n", "Beginner=\t\t#FFFFFF\n",
        "Intermediate=\t\t#FFFFFF\n", "Dizzy=\t\t\t#FFFFFF\n",
        "Tower=\t\t\t#FFFFFF\n", "Up=\t\t\t#FFFFFF\n",
        "Neon=\t\t\t#FFFFFF\n", "Expert=\t\t\t#FFFFFF\n",
        "Odd=\t\t\t#FFFFFF\n", "Toob=\t\t\t#FFFFFF\n",
        "Wobbly=\t\t\t#FFFFFF\n", "Glass=\t\t\t#FFFFFF\n",
        "Sky=\t\t\t#FFFFFF\n", "Master=\t\t\t#FFFFFF\n",
        "Impossible=\t\t#FFFFFF\n",
    };
    for (int i = 0; i < 15; i++)
        WriteFile(h, lvl_colors[i], lstrlenA(lvl_colors[i]), &written, NULL);

    const char *sep = "\n# =====================================\n\n\n";
    WriteFile(h, sep, lstrlenA(sep), &written, NULL);

    const char *arenas_section =
        "# ============== ARENAS ==============\n\n"
        "# - NAMES - #\n";
    WriteFile(h, arenas_section, lstrlenA(arenas_section), &written, NULL);

    const char *are_names[] = {
        "name:WarmUp=\t\t\"WARM-UP ARENA\"\n",
        "name:Beginner=\t\t\"BEGINNER ARENA\"\n",
        "name:Intermediate=\t\"INTERMEDIATE ARENA\"\n",
        "name:Dizzy=\t\t\"DIZZY ARENA\"\n",
        "name:Tower=\t\t\"TOWER ARENA\"\n",
        "name:Up=\t\t\"UP ARENA\"\n",
        "name:Neon=\t\t\"NEON ARENA\"\n",
        "name:Expert=\t\t\"EXPERT ARENA\"\n",
        "name:Odd=\t\t\"ODD ARENA\"\n",
        "name:Toob=\t\t\"TOOB ARENA\"\n",
        "name:Wobbly=\t\t\"WOBBLY ARENA\"\n",
        "name:Glass=\t\t\"GLASS ARENA\"\n",
        "name:Sky=\t\t\"SKY ARENA\"\n",
        "name:Master=\t\t\"MASTER ARENA\"\n",
        "name:Impossible=\t\"IMPOSSIBLE ARENA\"\n",
    };
    for (int i = 0; i < 15; i++)
        WriteFile(h, are_names[i], lstrlenA(are_names[i]), &written, NULL);

    const char *acolors_header = "\n# - COLORS - #\n";
    WriteFile(h, acolors_header, lstrlenA(acolors_header), &written, NULL);

    for (int i = 0; i < 15; i++) {
        const char *names[] = {"WarmUp","Beginner","Intermediate","Dizzy","Tower",
            "Up","Neon","Expert","Odd","Toob","Wobbly","Glass","Sky","Master","Impossible"};
        char line[128];
        int n = wsprintfA(line, "%s=\t\t\t#FFFFFF\n", names[i]);
        WriteFile(h, line, n, &written, NULL);
    }

    const char *footer = "\n# =====================================\n\n"
        "# ~ Script Made with help of BookwormKevin. Thank you!\n";
    WriteFile(h, footer, lstrlenA(footer), &written, NULL);

    CloseHandle(h);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Background Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI color_thread(LPVOID param) {
    (void)param;
    Sleep(3000);

    install_all_code_caves();
    init_name_buffers();

    DWORD last_config_check = 0;
    for (;;) {
        DWORD now = GetTickCount();
        if (now - last_config_check > 2000) {
            read_config();
            patch_menu_colors(g_levels, g_level_menu_colors);
            patch_menu_colors(g_arenas, g_arena_menu_colors);
            update_name_buffers();
            last_config_check = now;
        }
        apply_board_colors();
        Sleep(100);
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config Path Init
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_config_path(void) {
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) strcpy(p + 1, "mkn_plus_levels_namecolor.txt");
    else   strcpy(g_config_path, "mkn_plus_levels_namecolor.txt");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    (void)lpReserved;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        init_config_path();
        create_default_config();
        HANDLE h = CreateThread(NULL, 0, color_thread, NULL, 0, NULL);
        if (h) CloseHandle(h);
    }
    return TRUE;
}

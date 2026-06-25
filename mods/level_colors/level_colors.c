/*
 * level_colors.c — BASS.dll proxy mod
 *
 * Changes per-level base colors based on a colors.txt config file.
 *
 * Two color systems are modded:
 *   1. BOARD COLORS (timer oval + timer text tint during gameplay)
 *      - Stored at board+0x1508 (R), +0x150C (G), +0x1510 (B) as floats
 *      - Applied at runtime via background thread polling (no code patching)
 *      - Re-applied every 100ms since the game writes them once on board ctor
 *
 *   2. MENU COLORS (race selection menu text color in Practice/Time Trial)
 *      - Hardcoded as PUSH immediates in PracticeMenu_ctor (0x0042EA30)
 *      - Applied by patching the 4-byte float operands in the PUSH instructions
 *      - Patches are applied once at startup (code is in .text, persists)
 *
 * Config file format (colors.txt next to bass.dll):
 *   ; Per-level colors in hex RGB (like HTML colors, no alpha)
 *   ; Lines starting with ; or # are comments
 *   ; Format: LevelName=RRGGBB
 *   ;
 *   ; "board:" prefix = timer oval/text color during gameplay
 *   ; "menu:" prefix  = race selection menu text color
 *   ; no prefix       = applies to BOTH (recommended)
 *   ;
 *   WarmUp=FF00FF       ; pink/magenta
 *   Intermediate=0000FF ; blue
 *   Dizzy=00FF00        ; green
 *   Tower=FFBF00        ; orange
 *   Expert=FF0000       ; red
 *   Odd=FF8000          ; dark orange
 *   Wobbly=9ED64D       ; yellow-green
 *   Toob=8080FF         ; light blue
 *   Sky=0080FF          ; sky blue
 *   Beginner=FFBF40     ; gold
 *   Up=FF00FF           ; magenta
 *   Master=808080       ; gray
 *   Neon=FFFF00         ; yellow
 *   Impossible=FF0000   ; red
 *   Glass=FF00FF        ; magenta
 *
 *   ; Advanced: separate board vs menu colors
 *   ; board:WarmUp=FF00FF    ; only changes timer oval
 *   ; menu:WarmUp=FFBFFF     ; only changes menu text
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll level_colors.c -lwinmm \
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
 * Level Color Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define APP_PTR_ADDR     0x005341E0
#define BOARD_COLOR_R    0x1508
#define BOARD_COLOR_G    0x150C
#define BOARD_COLOR_B    0x1510
#define BOARD_COLOR_A    0x1514
#define BOARD_SCALE      0x4340

/* Level definitions */
typedef struct {
    const char *name;       /* config file name (case-insensitive) */
    int          id;        /* level index 0-14 */
    /* Menu color patch addresses (PUSH immediate + 1 for the float operand) */
    DWORD menu_r_addr;      /* address of R float in PUSH instruction */
    DWORD menu_g_addr;
    DWORD menu_b_addr;
} LevelDef;

static const LevelDef g_levels[] = {
    {"WarmUp",       0, 0x0042EE94, 0x0042EE8F, 0x0042EE8A},
    {"Beginner",     1, 0x0042EED3, 0x0042EECE, 0x0042EEC9},
    {"Intermediate", 2, 0x0042EF0D, 0x0042EF08, 0x0042EF03},
    {"Dizzy",        3, 0x0042EF60, 0x0042EF5B, 0x0042EF56},
    {"Tower",        4, 0x0042EFDA, 0x0042EFD5, 0x0042EFD0},
    {"Up",           5, 0x0042F054, 0x0042F04F, 0x0042F04A},
    {"Neon",         6, 0x0042F0D4, 0x0042F0CF, 0x0042F0CD},
    {"Expert",       7, 0x0042F14E, 0x0042F149, 0x0042F144},
    {"Odd",          8, 0x0042F1C5, 0x0042F1C0, 0x0042F1BE},
    {"Toob",         9, 0x0042F248, 0x0042F243, 0x0042F23E},
    {"Wobbly",      10, 0x0042F2C2, 0x0042F2BD, 0x0042F2B8},
    {"Glass",       11, 0x0042F33C, 0x0042F337, 0x0042F332},
    {"Sky",         12, 0x0042F3BF, 0x0042F3BA, 0x0042F3B5},
    {"Master",      13, 0x0042F439, 0x0042F434, 0x0042F42F},
    {"Impossible",  14, 0x0042F4AD, 0x0042F4AB, 0x0042F4A9},
};
#define NUM_LEVELS 15

/* Color entry: parsed from config file */
typedef struct {
    int  valid;      /* 1 if this entry was found in config */
    DWORD rgb;       /* 0xRRGGBB */
} ColorEntry;

static ColorEntry g_board_colors[NUM_LEVELS];  /* for runtime board polling */
static ColorEntry g_menu_colors[NUM_LEVELS];   /* for menu code patching */
static char g_config_path[MAX_PATH] = {0};

/* ── Config file parsing ─────────────────────────────────────────────────── */

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

/* Find level by name (case-insensitive). Returns index or -1. */
static int find_level(const char *name)
{
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (str_eq_ci(name, g_levels[i].name)) return i;
    }
    return -1;
}

/* Parse hex color string. Returns 0xRRGGBB or 0xFFFFFFFF on error. */
static DWORD parse_hex(const char *s)
{
    if (*s == '#') s++;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;

    DWORD hex = 0;
    int digits = 0;
    while (digits < 6 && *s) {
        char c = to_lower(*s);
        int val;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else break;
        hex = (hex << 4) | val;
        digits++;
        s++;
    }
    if (digits >= 6) return hex;
    if (digits >= 3) {
        /* Short form: RGB → RRGGBB */
        int r = ((hex >> 8) & 0xF) * 0x11;
        int g = ((hex >> 4) & 0xF) * 0x11;
        int b = (hex & 0xF) * 0x11;
        return ((DWORD)r << 16) | ((DWORD)g << 8) | b;
    }
    return 0xFFFFFFFF;
}

/* Parse a single config line: "LevelName=RRGGBB" or "prefix:LevelName=RRGGBB" */
static void parse_config_line(char *line)
{
    /* Skip leading whitespace */
    while (*line == ' ' || *line == '\t') line++;
    if (*line == ';' || *line == '#' || *line == '\r' || *line == '\n' || *line == '\0')
        return;

    /* Check for prefix: "board:" or "menu:" */
    int prefix = 0;  /* 0=both, 1=board only, 2=menu only */
    if (str_eq_ci_n(line, "board:", 6)) { prefix = 1; line += 6; }
    else if (str_eq_ci_n(line, "menu:", 5)) { prefix = 2; line += 5; }

    /* Find '=' */
    char *eq = strchr(line, '=');
    if (!eq) return;
    *eq = '\0';

    /* Trim level name (remove trailing spaces) */
    char *name = line;
    char *end = eq - 1;
    while (end > name && (*end == ' ' || *end == '\t')) { *end = '\0'; end--; }

    /* Parse color value */
    char *val = eq + 1;
    while (*val == ' ' || *val == '\t') val++;
    DWORD color = parse_hex(val);
    if (color == 0xFFFFFFFF) return;

    /* Find level */
    int idx = find_level(name);
    if (idx < 0) return;

    /* Store color */
    if (prefix == 0 || prefix == 1) {
        g_board_colors[idx].valid = 1;
        g_board_colors[idx].rgb = color;
    }
    if (prefix == 0 || prefix == 2) {
        g_menu_colors[idx].valid = 1;
        g_menu_colors[idx].rgb = color;
    }
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

    /* Clear previous entries */
    for (int i = 0; i < NUM_LEVELS; i++) {
        g_board_colors[i].valid = 0;
        g_menu_colors[i].valid = 0;
    }

    /* Parse line by line */
    char *p = buf;
    while (*p) {
        char *nl = p;
        while (*nl && *nl != '\n') nl++;
        char saved = *nl;
        *nl = '\0';
        parse_config_line(p);
        *nl = saved;
        if (*nl == '\n') nl++;
        p = nl;
    }
}

/* Create default config file */
static void create_default_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); return; }

    const char *defaults =
        "; Level Colors Config File\n"
        "; Format: LevelName=RRGGBB (hex RGB, like HTML colors)\n"
        "; Lines starting with ; or # are comments\n"
        ";\n"
        "; Prefixes:\n"
        ";   LevelName=RRGGBB     — applies to BOTH timer and menu\n"
        ";   board:LevelName=RRGGBB — only timer oval/text during gameplay\n"
        ";   menu:LevelName=RRGGBB  — only race selection menu text\n"
        ";\n"
        "; Re-reads every 2 seconds. Edit at runtime!\n"
        "\n"
        "WarmUp=FF00FF\n"
        "Beginner=FFBF40\n"
        "Intermediate=0000FF\n"
        "Dizzy=00FF00\n"
        "Tower=FFBF00\n"
        "Up=FF00FF\n"
        "Neon=FFFF00\n"
        "Expert=FF0000\n"
        "Odd=FF8000\n"
        "Toob=8080FF\n"
        "Wobbly=9ED64D\n"
        "Glass=FF00FF\n"
        "Sky=0080FF\n"
        "Master=808080\n"
        "Impossible=FF0000\n";

    DWORD written;
    h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        WriteFile(h, defaults, lstrlenA(defaults), &written, NULL);
        CloseHandle(h);
    }
}

/* ── Menu color patching (code modification) ────────────────────────────── */

/* Write a 4-byte float into the code section at the given address.
 * Uses VirtualProtect to temporarily make the page writable. */
static int patch_float(DWORD addr, float value)
{
    DWORD oldProtect;
    if (!VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    *(float*)addr = value;
    VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
    return 1;
}

/* Patch all menu colors in PracticeMenu_ctor */
static void patch_menu_colors(void)
{
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (!g_menu_colors[i].valid) continue;

        float r = ((g_menu_colors[i].rgb >> 16) & 0xFF) / 255.0f;
        float g = ((g_menu_colors[i].rgb >> 8) & 0xFF) / 255.0f;
        float b = (g_menu_colors[i].rgb & 0xFF) / 255.0f;

        patch_float(g_levels[i].menu_r_addr, r);
        patch_float(g_levels[i].menu_g_addr, g);
        patch_float(g_levels[i].menu_b_addr, b);
    }
}

/* ── Board color polling (runtime memory write) ─────────────────────────── */

/* Find the active board/scene pointer.
 * App is at *(0x5341E0). The board IS the scene object.
 * We scan App offsets for a pointer that has valid data at +0x1508. */
static DWORD find_board(void)
{
    DWORD app = *(DWORD*)APP_PTR_ADDR;
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)app, 0x300)) return 0;

    /* Try App+0x220 (PlayerProfile) → +0xC (board) */
    DWORD profile = *(DWORD*)((BYTE*)app + 0x220);
    if (profile && !IsBadReadPtr((void*)profile, 0x20)) {
        DWORD board = *(DWORD*)((BYTE*)profile + 0x0C);
        if (board && board > 0x10000 && !IsBadReadPtr((void*)board, 0x2000)) {
            /* Verify it looks like a board: check +0x1508 is a valid float */
            float test = *(float*)((BYTE*)board + BOARD_COLOR_R);
            if (test >= 0.0f && test <= 1.0f) return board;
        }
    }

    /* Fallback: scan App for a pointer that has valid board color floats */
    for (int off = 0x100; off < 0xA00; off += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + off);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x2000)) continue;
        float r = *(float*)((BYTE*)candidate + BOARD_COLOR_R);
        float g = *(float*)((BYTE*)candidate + BOARD_COLOR_G);
        float b = *(float*)((BYTE*)candidate + BOARD_COLOR_B);
        if (r >= 0.0f && r <= 1.0f && g >= 0.0f && g <= 1.0f &&
            b >= 0.0f && b <= 1.0f) {
            return candidate;
        }
    }
    return 0;
}

/* Apply board colors at runtime */
static void apply_board_colors(void)
{
    DWORD board = find_board();
    if (!board) return;

    for (int i = 0; i < NUM_LEVELS; i++) {
        if (!g_board_colors[i].valid) continue;

        /* We don't know which level is currently active, so we check
         * if the board's CURRENT color matches this level's original color.
         * If it does, we apply the override. */
        /* Actually, simpler: just write to ALL valid levels. The board
         * only has one color set. But we don't know which level is active...
         *
         * Better approach: check board+0x29B4 (level name string) to identify
         * the current level, then apply only that level's override. */
    }

    /* Identify current level by checking board+0x29B4 (level name pointer) */
    if (IsBadReadPtr((void*)(board + 0x29B4), 4)) return;
    char *level_name = *(char**)(board + 0x29B4);
    if (!level_name || IsBadReadPtr(level_name, 4)) return;

    /* Match level name to find index */
    int current_level = -1;
    /* The board stores names like "WARM-UP RACE", "BEGINNER RACE", etc.
     * We need to match these to our level indices. */
    static const char *level_name_strings[] = {
        "WARM-UP RACE", "BEGINNER RACE", "INTERMEDIATE RACE", "DIZZY RACE",
        "TOWER RACE", "UP RACE", "NEON RACE", "EXPERT RACE", "ODD RACE",
        "TOOB RACE", "WOBBLY RACE", "GLASS RACE", "SKY RACE",
        "MASTER RACE", "IMPOSSIBLE RACE"
    };

    for (int i = 0; i < NUM_LEVELS; i++) {
        if (str_eq_ci(level_name, (char*)level_name_strings[i])) {
            current_level = i;
            break;
        }
    }

    if (current_level < 0) return;
    if (!g_board_colors[current_level].valid) return;

    /* Apply the color override */
    DWORD rgb = g_board_colors[current_level].rgb;
    float r = ((rgb >> 16) & 0xFF) / 255.0f;
    float g = ((rgb >> 8) & 0xFF) / 255.0f;
    float b = (rgb & 0xFF) / 255.0f;

    if (!IsBadWritePtr((void*)(board + BOARD_COLOR_R), 4)) {
        *(float*)(board + BOARD_COLOR_R) = r;
        *(float*)(board + BOARD_COLOR_G) = g;
        *(float*)(board + BOARD_COLOR_B) = b;
        *(float*)(board + BOARD_COLOR_A) = 1.0f;
    }
}

/* ── Background thread ─────────────────────────────────────────────────── */

static DWORD WINAPI color_thread(LPVOID param)
{
    (void)param;
    Sleep(3000);  /* Wait for game to fully load */

    DWORD last_config_check = 0;

    for (;;) {
        DWORD now = GetTickCount();

        /* Re-read config every 2 seconds */
        if (now - last_config_check > 2000) {
            read_config();
            patch_menu_colors();  /* Re-patch menu colors (in case config changed) */
            last_config_check = now;
        }

        /* Apply board colors every 100ms (game re-writes them on board ctor,
         * so we need to re-apply continuously during gameplay) */
        apply_board_colors();

        Sleep(100);
    }

    return 0;
}

/* ── Config path init ───────────────────────────────────────────────────── */

static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "colors.txt");
    } else {
        strcpy(g_config_path, "colors.txt");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        init_config_path();
        create_default_config();
        HANDLE hThread = CreateThread(NULL, 0, color_thread, NULL, 0, NULL);
        if (hThread) CloseHandle(hThread);
        break;
    }
    return TRUE;
}

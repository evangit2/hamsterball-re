/*
 * level_colors.c — BASS.dll proxy mod  (v2 — crash fix)
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
 *      - Applied by patching the 4-byte float operands in PUSH instructions
 *      - Patches are applied once at startup (code is in .text, persists)
 *
 * v2 FIX: Three levels (Neon, Odd, Impossible) use "push 0" (6a 00, 2 bytes)
 *   instead of "push imm32" (68 XX XX XX XX, 5 bytes) for zero-valued color
 *   components. The v1 mod assumed ALL pushes were 5-byte push-imm32 and wrote
 *   4 bytes at the operand offset, corrupting adjacent instructions and crashing
 *   the game when the Time Trial menu was opened.
 *   Fix: For those 3 levels, a CODE CAVE replaces the entire push sequence.
 *   The cave uses proper 5-byte push-imm32 for all 4 components, so the float
 *   operands can be safely patched like the other levels.
 *
 * Config file format (colors.txt next to bass.dll):
 *   ; Per-level colors in hex RGB (like HTML colors, no alpha)
 *   ; Lines starting with ; or # are comments
 *   ; Format: LevelName=RRGGBB
 *   ;
 *   ; "board:" prefix = timer oval/text color during gameplay
 *   ; "menu:" prefix  = race selection menu text color
 *   ; no prefix       = applies to BOTH (recommended)
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

/* The call target in PracticeMenu_ctor for all 15 levels */
#define COLOR_INIT_CALL  0x00453150

/* Level definitions */
typedef struct {
    const char *name;       /* config file name (case-insensitive) */
    int          id;        /* level index 0-14 */
    /* Menu color patch addresses (PUSH immediate + 1 for the float operand).
     * These are for the "completed" branch (level is unlocked).
     * For levels with code_cave=1, these are the operand offsets WITHIN the
     * code cave (set by install_code_cave), not the original .text addresses. */
    DWORD menu_a_addr;      /* address of A float in push instruction */
    DWORD menu_b_addr;
    DWORD menu_g_addr;
    DWORD menu_r_addr;
    int   code_cave;        /* 1 = uses code cave, 0 = direct patch */
    /* For code cave levels: original push sequence start and size */
    DWORD push_seq_start;   /* address of first push instruction */
    int   push_seq_size;    /* total bytes of push sequence */
    DWORD return_addr;      /* address of the call instruction after pushes */
} LevelDef;

/* Addresses verified via objdump disassembly of PracticeMenu_ctor (0x0042EA30).
 *
 * The push order is: A, B, G, R (pushed right-to-left for the call).
 * For levels with all-68 pushes, the operand is at push_addr+1.
 *   e.g. WarmUp A: push at 0x42EE84, operand at 0x42EE85
 *
 * For Neon(6), Odd(8), Impossible(14): the original code uses "6a 00"
 *   (push 0, 2 bytes) for zero-valued components. These use code caves.
 *   The menu_*_addr fields are filled at runtime by install_code_cave(). */
static LevelDef g_levels[] = {
    /* 0: WarmUp — all 68 pushes */
    {"WarmUp",       0, 0x42EE85, 0x42EE8A, 0x42EE8F, 0x42EE94, 0, 0, 0, 0},
    /* 1: Beginner — all 68 pushes */
    {"Beginner",     1, 0x42EEC4, 0x42EEC9, 0x42EECE, 0x42EED3, 0, 0, 0, 0},
    /* 2: Intermediate — all 68 pushes */
    {"Intermediate", 2, 0x42EEFE, 0x42EF03, 0x42EF08, 0x42EF0D, 0, 0, 0, 0},
    /* 3: Dizzy — all 68 pushes */
    {"Dizzy",        3, 0x42EF51, 0x42EF56, 0x42EF5B, 0x42EF60, 0, 0, 0, 0},
    /* 4: Tower — all 68 pushes */
    {"Tower",        4, 0x42EFCB, 0x42EFD0, 0x42EFD5, 0x42EFDA, 0, 0, 0, 0},
    /* 5: Up — all 68 pushes */
    {"Up",           5, 0x42F045, 0x42F04A, 0x42F04F, 0x42F054, 0, 0, 0, 0},
    /* 6: Neon — has 6a 00 (B=0.0) → CODE CAVE */
    {"Neon",         6, 0, 0, 0, 0, 1, 0x42F0C7, 17, 0x42F0D8},
    /* 7: Expert — all 68 pushes */
    {"Expert",       7, 0x42F13F, 0x42F144, 0x42F149, 0x42F14E, 0, 0, 0, 0},
    /* 8: Odd — has 6a 00 (B=0.0) → CODE CAVE */
    {"Odd",          8, 0, 0, 0, 0, 1, 0x42F1B8, 17, 0x42F1C9},
    /* 9: Toob — all 68 pushes */
    {"Toob",         9, 0x42F239, 0x42F23E, 0x42F243, 0x42F248, 0, 0, 0, 0},
    /* 10: Wobbly — all 68 pushes */
    {"Wobbly",      10, 0x42F2B3, 0x42F2B8, 0x42F2BD, 0x42F2C2, 0, 0, 0, 0},
    /* 11: Glass — all 68 pushes */
    {"Glass",       11, 0x42F32D, 0x42F332, 0x42F337, 0x42F33C, 0, 0, 0, 0},
    /* 12: Sky — all 68 pushes */
    {"Sky",         12, 0x42F3B0, 0x42F3B5, 0x42F3BA, 0x42F3BF, 0, 0, 0, 0},
    /* 13: Master — all 68 pushes */
    {"Master",      13, 0x42F42A, 0x42F42F, 0x42F434, 0x42F439, 0, 0, 0, 0},
    /* 14: Impossible — has 6a 00 (B=0.0, G=0.0) → CODE CAVE */
    {"Impossible",  14, 0, 0, 0, 0, 1, 0x42F4A3, 14, 0x42F4B1},
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
static int g_caves_installed = 0;

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

/* ── Menu color patching ────────────────────────────────────────────────── */

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

/* Write a byte at the given address (with VirtualProtect). */
static int patch_byte(DWORD addr, BYTE value)
{
    DWORD oldProtect;
    if (!VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    *(BYTE*)addr = value;
    VirtualProtect((void*)addr, 1, oldProtect, &oldProtect);
    return 1;
}

/* Write a 4-byte dword at the given address (with VirtualProtect). */
static int patch_dword(DWORD addr, DWORD value)
{
    DWORD oldProtect;
    if (!VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    *(DWORD*)addr = value;
    VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Cave System
 *
 * For levels whose original push sequence contains "6a 00" (push 0, 2-byte
 * instruction), we can't safely patch a 4-byte float operand into a 2-byte
 * instruction. Instead we:
 *   1. VirtualAlloc an executable page
 *   2. Write a trampoline: 4× push imm32 + jmp back_to_call
 *   3. At the original push sequence: write JMP to trampoline + NOP padding
 *
 * The trampoline's push immediates can then be patched via patch_float
 * just like the regular levels.
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE *g_cave_mem = NULL;   /* base of allocated cave page */
static int g_cave_offset = 0;     /* current write offset in cave page */

/* Allocate the code cave page (called once at startup). */
static int init_code_cave_page(void)
{
    if (g_cave_mem) return 1;
    g_cave_mem = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!g_cave_mem) return 0;
    return 1;
}

/* Allocate `size` bytes from the cave page (bump allocator). */
static BYTE* cave_alloc(int size)
{
    if (!g_cave_mem || g_cave_offset + size > 4096) return NULL;
    BYTE *p = g_cave_mem + g_cave_offset;
    g_cave_offset += size;
    return p;
}

/* Install a code cave for one level.
 * Replaces the original push sequence with: JMP cave + NOPs
 * Cave contains: 4× push imm32 + jmp return_addr
 * Updates g_levels[idx].menu_*_addr to point to the cave's push operands. */
static int install_code_cave(int idx)
{
    LevelDef *lv = &g_levels[idx];
    if (!init_code_cave_page()) return 0;

    /* Cave layout:
     *   push imm32 (A)   = 68 XX XX XX XX   (5 bytes)
     *   push imm32 (B)   = 68 XX XX XX XX   (5 bytes)
     *   push imm32 (G)   = 68 XX XX XX XX   (5 bytes)
     *   push imm32 (R)   = 68 XX XX XX XX   (5 bytes)
     *   jmp return_addr  = E9 XX XX XX XX   (5 bytes)
     * Total: 25 bytes */
    BYTE *cave = cave_alloc(25);
    if (!cave) return 0;

    /* Write the 4 push instructions (initial values = 0.0, will be patched later) */
    cave[0] = 0x68; *(DWORD*)(cave + 1) = 0;       /* push A */
    cave[5] = 0x68; *(DWORD*)(cave + 6) = 0;       /* push B */
    cave[10] = 0x68; *(DWORD*)(cave + 11) = 0;     /* push G */
    cave[15] = 0x68; *(DWORD*)(cave + 16) = 0;     /* push R */

    /* Write jmp return_addr (E9 = jmp rel32) */
    cave[20] = 0xE9;
    DWORD rel = lv->return_addr - ((DWORD)cave + 25);
    *(DWORD*)(cave + 21) = rel;

    /* Now patch the original push sequence:
     * Replace first 5 bytes with JMP to cave, fill rest with NOPs. */
    DWORD orig = lv->push_seq_start;
    int orig_size = lv->push_seq_size;

    /* JMP rel32 to cave */
    patch_byte(orig, 0xE9);
    DWORD jmp_rel = (DWORD)cave - (orig + 5);
    patch_dword(orig + 1, jmp_rel);

    /* NOP padding for remaining bytes */
    for (int i = 5; i < orig_size; i++) {
        patch_byte(orig + i, 0x90);
    }

    /* Update the level's menu addresses to point to the cave's push operands */
    lv->menu_a_addr = (DWORD)(cave + 1);
    lv->menu_b_addr = (DWORD)(cave + 6);
    lv->menu_g_addr = (DWORD)(cave + 11);
    lv->menu_r_addr = (DWORD)(cave + 16);

    return 1;
}

/* Install code caves for all levels that need them (call once at startup). */
static void install_all_code_caves(void)
{
    if (g_caves_installed) return;
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (g_levels[i].code_cave) {
            install_code_cave(i);
        }
    }
    g_caves_installed = 1;
}

/* Patch all menu colors.
 * For regular levels: patches the float operands in the original PUSH instructions.
 * For code-cave levels: patches the float operands in the cave's PUSH instructions.
 * Both use the same patch_float mechanism. */
static void patch_menu_colors(void)
{
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (!g_menu_colors[i].valid) continue;

        float r = ((g_menu_colors[i].rgb >> 16) & 0xFF) / 255.0f;
        float g = ((g_menu_colors[i].rgb >> 8) & 0xFF) / 255.0f;
        float b = (g_menu_colors[i].rgb & 0xFF) / 255.0f;
        float a = 1.0f;

        /* Alpha is always 1.0 (not configurable, but we patch it for completeness) */
        patch_float(g_levels[i].menu_a_addr, a);
        patch_float(g_levels[i].menu_b_addr, b);
        patch_float(g_levels[i].menu_g_addr, g);
        patch_float(g_levels[i].menu_r_addr, r);
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

    /* Identify current level by checking board+0x29B4 (level name pointer) */
    if (IsBadReadPtr((void*)(board + 0x29B4), 4)) return;
    char *level_name = *(char**)(board + 0x29B4);
    if (!level_name || IsBadReadPtr(level_name, 4)) return;

    /* Match level name to find index */
    int current_level = -1;
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

    /* Install code caves for levels that need them (Neon, Odd, Impossible).
     * Must be done before patch_menu_colors(). */
    install_all_code_caves();

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

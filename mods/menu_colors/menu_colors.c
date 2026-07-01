/*
 * menu_colors.c — BASS.dll proxy mod
 *
 * Reads menu_colors.txt and patches menu UI colors in Hamsterball.
 *
 * Config format (menu_colors.txt):
 *   - LOADING SCREEN -
 *   "Click here" button - Off = #FFFFFF
 *   "Click here" button - On = #FFFFFF
 *
 *   - MAIN MENU -
 *   HBversion = #FFFFFF, 1.0
 *
 *   - 2P MENU -
 *   2PmenuSideBar = #FFFFFF, 1.0
 *   2PwinBar = #FFFFFF, 1.0
 *
 * How it works:
 *   1. On load: creates menu_colors.txt next to the DLL (if missing)
 *   2. Patches Vec3_Init / Matrix_Scale4x4 call sites with code caves
 *      that PUSH custom float values from global variables
 *   3. Background thread re-reads config every ~2 seconds
 *   4. When config changes, global floats update → next frame uses new colors
 *
 * Patches:
 *   Loading screen "Click here" Off button (0x0042d5fd, 9 bytes):
 *     PUSH(Blue,5) PUSH(Green,2) PUSH(Red,2) → Vec3_Init(R,G,B), 3 args
 *
 *   Loading screen "Click here" On button (0x0042d624, 9 bytes):
 *     PUSH(Blue,5) PUSH(Green,2) PUSH(Red,2) → Vec3_Init, 3 args
 *
 *   Main menu HB version text (0x00426433, 14 bytes):
 *     PUSH(Alpha,5) PUSH(Blue,2) PUSH(Green,5) PUSH(Red,2)
 *     → Matrix_Scale4x4(R,G,B,A), 4 args
 *
 *   2P menu side bar left (0x00431a5f, 16 bytes):
 *     PUSH(Alpha,5) PUSH(Blue,1=EBX) PUSH(Green,5) PUSH(Red,5)
 *     → Matrix_Scale4x4(R,G,B,A), 4 args. Default: R=1,G=1,B=0,A=0.72 (yellow)
 *
 *   2P menu side bar right (0x00431aa3, 16 bytes):
 *     Mirror of left sidebar. Same defaults. Patched for consistency.
 *
 *   2P win bar (0x0044d68d, 14 bytes):
 *     PUSH(Alpha,5) PUSH(Blue,5) PUSH(Green,2) PUSH(Red,2)
 *     → Matrix_Scale4x4(R,G,B,A), 4 args. Default: R=0,G=0,B=1,A=0.75 (blue)
 *
 * Blue/Green/Red use PUSH imm8 (2 bytes) or PUSH register (1 byte) —
 * can't hold arbitrary floats. Code caves redirect to:
 *   PUSH [global_float] (6 bytes each) + JMP back.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll menu_colors.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (all 10 game imports + extras)
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
 * Menu Colors Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ── Patch addresses ─────────────────────────────────────────────────────── */

/* Loading screen "Click here" Off button — Vec3_Init(R,G,B), 3 args */
#define OFF_PATCH_ADDR   0x0042d5fd
#define OFF_RETURN_ADDR  0x0042d606
#define OFF_PATCH_SIZE   9

/* Loading screen "Click here" On button — Vec3_Init(R,G,B), 3 args */
#define ON_PATCH_ADDR    0x0042d624
#define ON_RETURN_ADDR   0x0042d62d
#define ON_PATCH_SIZE    9

/* Main menu HB version text — Matrix_Scale4x4(R,G,B,A), 4 args */
#define HBVER_PATCH_ADDR 0x00426433
#define HBVER_RETURN_ADDR 0x00426441
#define HBVER_PATCH_SIZE 14           /* 5 + 2 + 5 + 2 bytes */

/* 2P menu side bar (left) — Matrix_Scale4x4(R,G,B,A), 4 args */
#define SIDEBAR_L_PATCH_ADDR  0x00431a5f
#define SIDEBAR_L_RETURN_ADDR 0x00431a6f
#define SIDEBAR_L_PATCH_SIZE  16       /* 5 + 1 + 5 + 5 bytes */

/* 2P menu side bar (right, mirror) — Matrix_Scale4x4(R,G,B,A), 4 args */
#define SIDEBAR_R_PATCH_ADDR  0x00431aa3
#define SIDEBAR_R_RETURN_ADDR 0x00431ab3
#define SIDEBAR_R_PATCH_SIZE  16       /* 5 + 1 + 5 + 5 bytes */

/* 2P win bar — Matrix_Scale4x4(R,G,B,A), 4 args */
#define WINBAR_PATCH_ADDR  0x0044d68d
#define WINBAR_RETURN_ADDR 0x0044d69b
#define WINBAR_PATCH_SIZE  14          /* 5 + 5 + 2 + 2 bytes */

/* Cave sizes: 3-arg cave = 3×6+5 = 23, 4-arg cave = 4×6+5 = 29 */
#define CAVE_SIZE_3      23
#define CAVE_SIZE_4      29

/* ── Global color variables ──────────────────────────────────────────────── */

/* Defaults match original game values */
static float g_off_r = 1.0f, g_off_g = 1.0f, g_off_b = 1.0f;  /* white */
static float g_on_r  = 1.0f, g_on_g  = 1.0f, g_on_b  = 1.0f;  /* white */
static float g_hbver_r = 0.0f, g_hbver_g = 1.0f, g_hbver_b = 0.0f, g_hbver_a = 1.0f; /* green */
static float g_sidebar_r = 1.0f, g_sidebar_g = 1.0f, g_sidebar_b = 0.0f, g_sidebar_a = 0.72f; /* yellow */
static float g_winbar_r  = 0.0f, g_winbar_g  = 0.0f, g_winbar_b  = 1.0f, g_winbar_a  = 0.75f;  /* blue */

static char g_config_path[MAX_PATH] = {0};
static volatile LONG g_shutdown = 0;
static HANDLE g_thread_handle = NULL;

/* ── Config file management ──────────────────────────────────────────────── */

static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "menu_colors.txt");
    } else {
        strcpy(g_config_path, "menu_colors.txt");
    }
}

static void create_default_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        const char *default_content =
            "- LOADING SCREEN -\r\n"
            "\"Click here\" button - Off = #FFFFFF\r\n"
            "\"Click here\" button - On = #FFFFFF\r\n"
            "\r\n"
            "\r\n"
            "- MAIN MENU -\r\n"
            "HBversion = #FFFFFF, 1.0\r\n"
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
            "soon.\r\n"
            "\r\n"
            "\r\n"
            "- 2P MENU -\r\n"
            "2PmenuSideBar = #FFFFFF, 1.0\r\n"
            "2PwinBar = #FFFFFF, 1.0\r\n"
            "\r\n"
            "\r\n"
            "- 4P MENU -\r\n"
            "soon.\r\n"
            "\r\n"
            "\r\n"
            "- OTHER -\r\n"
            "soon.\r\n";
        DWORD written;
        h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            WriteFile(h, default_content, lstrlenA(default_content), &written, NULL);
            CloseHandle(h);
        }
    } else {
        CloseHandle(h);
    }
}

/* ── Hex color parser ────────────────────────────────────────────────────── */

static DWORD parse_hex_color(const char *text)
{
    const char *p = text;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '#') p++;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;

    DWORD hex = 0;
    int digits = 0;
    while (digits < 6 && *p) {
        char c = *p;
        if (c >= '0' && c <= '9') { hex = (hex << 4) | (c - '0'); digits++; }
        else if (c >= 'a' && c <= 'f') { hex = (hex << 4) | (c - 'a' + 10); digits++; }
        else if (c >= 'A' && c <= 'F') { hex = (hex << 4) | (c - 'A' + 10); digits++; }
        else break;
        p++;
    }
    if (digits >= 6) return hex;
    if (digits >= 3) {
        DWORD r = ((hex >> 8) & 0xF) * 0x11;
        DWORD g = ((hex >> 4) & 0xF) * 0x11;
        DWORD b = (hex & 0xF) * 0x11;
        return (r << 16) | (g << 8) | b;
    }
    return 0xFFFFFF;
}

/* Convert hex to float RGB */
static void hex_to_floats(DWORD hex, float *r, float *g, float *b)
{
    *r = ((hex >> 16) & 0xFF) / 255.0f;
    *g = ((hex >> 8)  & 0xFF) / 255.0f;
    *b = ( hex        & 0xFF) / 255.0f;
}

/* Parse a float value from text. Returns 1 on success. */
static int parse_float(const char *text, float *out)
{
    const char *p = text;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0' || *p == '\r' || *p == '\n') return 0;

    float result = 0.0f;
    int sign = 1;
    int got_digit = 0;

    if (*p == '-') { sign = -1; p++; }
    else if (*p == '+') { p++; }

    while (*p >= '0' && *p <= '9') {
        result = result * 10.0f + (*p - '0');
        got_digit = 1;
        p++;
    }
    if (*p == '.') {
        p++;
        float divisor = 10.0f;
        while (*p >= '0' && *p <= '9') {
            result += (*p - '0') / divisor;
            divisor *= 10.0f;
            got_digit = 1;
            p++;
        }
    }
    if (!got_digit) return 0;

    *out = sign * result;
    return 1;
}

/* ── Config reader ───────────────────────────────────────────────────────── */

/* Scan a line for a hex color after '=' sign. Returns 1 if found. */
static int extract_color_from_line(const char *line, DWORD *out_hex)
{
    const char *eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;
    if (*eq == '\0' || *eq == '\r' || *eq == '\n') return 0;
    if (strnicmp(eq, "soon", 4) == 0) return 0;
    *out_hex = parse_hex_color(eq);
    return 1;
}

/* Extract hex color + optional float alpha from "key = #RRGGBB, 1.0" */
static int extract_color_and_alpha(const char *line, DWORD *out_hex, float *out_alpha)
{
    const char *eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;
    if (*eq == '\0' || *eq == '\r' || *eq == '\n') return 0;
    if (strnicmp(eq, "soon", 4) == 0) return 0;

    *out_hex = parse_hex_color(eq);

    const char *comma = strchr(eq, ',');
    if (comma) {
        comma++;
        if (!parse_float(comma, out_alpha)) *out_alpha = 1.0f;
    } else {
        *out_alpha = 1.0f;
    }
    return 1;
}

/* Check if a line contains a case-insensitive substring */
static int line_contains(const char *line, const char *needle)
{
    int needle_len = lstrlenA(needle);
    int line_len = lstrlenA(line);
    int i, j;
    for (i = 0; i <= line_len - needle_len; i++) {
        int match = 1;
        for (j = 0; j < needle_len; j++) {
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

/* Read menu_colors.txt and update global color variables */
static void read_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[4096] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;

    float new_off_r = g_off_r, new_off_g = g_off_g, new_off_b = g_off_b;
    float new_on_r  = g_on_r,  new_on_g  = g_on_g,  new_on_b  = g_on_b;
    float new_hbver_r = g_hbver_r, new_hbver_g = g_hbver_g, new_hbver_b = g_hbver_b, new_hbver_a = g_hbver_a;
    float new_sb_r = g_sidebar_r, new_sb_g = g_sidebar_g, new_sb_b = g_sidebar_b, new_sb_a = g_sidebar_a;
    float new_wb_r = g_winbar_r, new_wb_g = g_winbar_g, new_wb_b = g_winbar_b, new_wb_a = g_winbar_a;

    char *line = buf;
    while (*line) {
        char *nl = line;
        while (*nl && *nl != '\n') nl++;
        int len = nl - line;
        if (len > 0 && line[len-1] == '\r') len--;

        if (len < 200) {
            if (line_contains(line, "hbversion") && line_contains(line, "=")) {
                DWORD hex; float alpha;
                if (extract_color_and_alpha(line, &hex, &alpha)) {
                    hex_to_floats(hex, &new_hbver_r, &new_hbver_g, &new_hbver_b);
                    new_hbver_a = alpha;
                }
            }
            else if (line_contains(line, "2pmenusidebar") && line_contains(line, "=")) {
                DWORD hex; float alpha;
                if (extract_color_and_alpha(line, &hex, &alpha)) {
                    hex_to_floats(hex, &new_sb_r, &new_sb_g, &new_sb_b);
                    new_sb_a = alpha;
                }
            }
            else if (line_contains(line, "2pwinbar") && line_contains(line, "=")) {
                DWORD hex; float alpha;
                if (extract_color_and_alpha(line, &hex, &alpha)) {
                    hex_to_floats(hex, &new_wb_r, &new_wb_g, &new_wb_b);
                    new_wb_a = alpha;
                }
            }
            else if (line_contains(line, "off") && line_contains(line, "=")) {
                DWORD hex;
                if (extract_color_from_line(line, &hex)) {
                    hex_to_floats(hex, &new_off_r, &new_off_g, &new_off_b);
                }
            }
            else if (line_contains(line, "on") && line_contains(line, "=")) {
                DWORD hex;
                if (extract_color_from_line(line, &hex)) {
                    hex_to_floats(hex, &new_on_r, &new_on_g, &new_on_b);
                }
            }
        }

        if (*nl == '\n') nl++;
        line = nl;
        if (*line == '\0') break;
    }

    /* Atomically update globals */
    g_off_r = new_off_r; g_off_g = new_off_g; g_off_b = new_off_b;
    g_on_r  = new_on_r;  g_on_g  = new_on_g;  g_on_b  = new_on_b;
    g_hbver_r = new_hbver_r; g_hbver_g = new_hbver_g;
    g_hbver_b = new_hbver_b;  g_hbver_a = new_hbver_a;
    g_sidebar_r = new_sb_r; g_sidebar_g = new_sb_g;
    g_sidebar_b = new_sb_b;  g_sidebar_a = new_sb_a;
    g_winbar_r = new_wb_r; g_winbar_g = new_wb_g;
    g_winbar_b = new_wb_b;  g_winbar_a = new_wb_a;
}

/* ── Code cave builders ──────────────────────────────────────────────────── */

/*
 * Build a 3-arg code cave (RGB) — replaces 3 consecutive PUSH instructions.
 * Cave (23 bytes): 3× PUSH [mem] (6 each) + JMP (5)
 */
static void build_cave3(unsigned char *cave, float *r, float *g, float *b, DWORD ret_addr)
{
    int idx = 0;
    /* PUSH [blue] — first (z, highest on stack) */
    cave[idx++] = 0xFF; cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)b; idx += 4;
    /* PUSH [green] — second (y) */
    cave[idx++] = 0xFF; cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)g; idx += 4;
    /* PUSH [red] — third (x, lowest) */
    cave[idx++] = 0xFF; cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)r; idx += 4;
    /* JMP ret_addr */
    cave[idx++] = 0xE9;
    *(DWORD*)(cave + idx) = ret_addr - ((DWORD)cave + idx + 4);
    idx += 4;
}

/*
 * Build a 4-arg code cave (RGBA) — replaces 4 consecutive PUSH instructions.
 * Cave (29 bytes): 4× PUSH [mem] (6 each) + JMP (5)
 */
static void build_cave4(unsigned char *cave, float *r, float *g, float *b, float *a, DWORD ret_addr)
{
    int idx = 0;
    /* PUSH [alpha] — first (w, highest on stack) */
    cave[idx++] = 0xFF; cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)a; idx += 4;
    /* PUSH [blue] — second (z) */
    cave[idx++] = 0xFF; cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)b; idx += 4;
    /* PUSH [green] — third (y) */
    cave[idx++] = 0xFF; cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)g; idx += 4;
    /* PUSH [red] — fourth (x, lowest) */
    cave[idx++] = 0xFF; cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)r; idx += 4;
    /* JMP ret_addr */
    cave[idx++] = 0xE9;
    *(DWORD*)(cave + idx) = ret_addr - ((DWORD)cave + idx + 4);
    idx += 4;
}

/* ── Code patcher ────────────────────────────────────────────────────────── */

static int patch_code(DWORD addr, int patch_size, DWORD cave_addr)
{
    DWORD old_protect;
    if (!VirtualProtect((void*)addr, patch_size, PAGE_EXECUTE_READWRITE, &old_protect))
        return 0;
    *(unsigned char*)addr = 0xE9;
    *(DWORD*)(addr + 1) = cave_addr - (addr + 5);
    int i;
    for (i = 5; i < patch_size; i++)
        *(unsigned char*)(addr + i) = 0x90;
    VirtualProtect((void*)addr, patch_size, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, patch_size);
    return 1;
}

/* Install all code caves */
static void install_patches(void)
{
    /* Allocate executable memory for 6 caves:
     *   2× 3-arg (23 bytes) + 4× 4-arg (29 bytes) = 46 + 116 = 162 bytes
     * Round to 256 */
    unsigned char *cave_mem = (unsigned char*)VirtualAlloc(
        NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!cave_mem) return;

    int offset = 0;

    /* Cave 0: Loading Off button (3-arg RGB) */
    build_cave3(cave_mem + offset, &g_off_r, &g_off_g, &g_off_b, OFF_RETURN_ADDR);
    patch_code(OFF_PATCH_ADDR, OFF_PATCH_SIZE, (DWORD)(cave_mem + offset));
    offset += CAVE_SIZE_3;

    /* Cave 1: Loading On button (3-arg RGB) */
    build_cave3(cave_mem + offset, &g_on_r, &g_on_g, &g_on_b, ON_RETURN_ADDR);
    patch_code(ON_PATCH_ADDR, ON_PATCH_SIZE, (DWORD)(cave_mem + offset));
    offset += CAVE_SIZE_3;

    /* Cave 2: HB version text (4-arg RGBA) */
    build_cave4(cave_mem + offset, &g_hbver_r, &g_hbver_g, &g_hbver_b, &g_hbver_a, HBVER_RETURN_ADDR);
    patch_code(HBVER_PATCH_ADDR, HBVER_PATCH_SIZE, (DWORD)(cave_mem + offset));
    offset += CAVE_SIZE_4;

    /* Cave 3: 2P sidebar left (4-arg RGBA) */
    build_cave4(cave_mem + offset, &g_sidebar_r, &g_sidebar_g, &g_sidebar_b, &g_sidebar_a, SIDEBAR_L_RETURN_ADDR);
    patch_code(SIDEBAR_L_PATCH_ADDR, SIDEBAR_L_PATCH_SIZE, (DWORD)(cave_mem + offset));
    offset += CAVE_SIZE_4;

    /* Cave 4: 2P sidebar right — mirror (4-arg RGBA) */
    build_cave4(cave_mem + offset, &g_sidebar_r, &g_sidebar_g, &g_sidebar_b, &g_sidebar_a, SIDEBAR_R_RETURN_ADDR);
    patch_code(SIDEBAR_R_PATCH_ADDR, SIDEBAR_R_PATCH_SIZE, (DWORD)(cave_mem + offset));
    offset += CAVE_SIZE_4;

    /* Cave 5: 2P win bar (4-arg RGBA) */
    build_cave4(cave_mem + offset, &g_winbar_r, &g_winbar_g, &g_winbar_b, &g_winbar_a, WINBAR_RETURN_ADDR);
    patch_code(WINBAR_PATCH_ADDR, WINBAR_PATCH_SIZE, (DWORD)(cave_mem + offset));
    offset += CAVE_SIZE_4;
}

/* ── Background thread ───────────────────────────────────────────────────── */

static DWORD WINAPI config_thread(LPVOID param)
{
    Sleep(2000);

    for (;;) {
        Sleep(30);
        if (g_shutdown) break;

        static DWORD poll_count = 0;
        if ((poll_count % 66) == 0) {
            read_config();
        }
        poll_count++;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        init_config_path();
        create_default_config();
        read_config();
        install_patches();
        g_thread_handle = CreateThread(NULL, 0, config_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        InterlockedExchange(&g_shutdown, 1);
        if (g_thread_handle) {
            WaitForSingleObject(g_thread_handle, 2000);
            CloseHandle(g_thread_handle);
        }
        break;
    }
    return TRUE;
}

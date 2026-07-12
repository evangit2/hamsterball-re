/*
 * CustomUIColors_MinGW.cpp — MinGW cross-compile version for testing.
 * Uses manual 17-entry vtable (HB+ v2.0) + nocrt.
 * The VS source (CustomUIColors.cpp) is the primary deliverable.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"

/* Redirect CRT calls to nc_ versions */
#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncpy nc_strncpy

/* ═══════════════════════════════════════════════════════════════════════════
 * Color Site Definitions
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CAVE_RGB         0
#define CAVE_RGBA        1
#define CAVE_RGBA_CALL   2
#define CAVE_RGBA_FPU    3

#define MATRIX_SCALE_4X4 0x00453150

static const unsigned char FMUL_BYTES[] = { 0xD8, 0x8E, 0x5C, 0x2D, 0x00, 0x00 };

struct ColorSite {
    const char* key;
    DWORD patchAddr;
    DWORD returnAddr;
    int patchSize;
    int caveType;
    int colorIdx;
};

#define NUM_COLOR_SITES 27
static float g_colors[NUM_COLOR_SITES][4];

static ColorSite g_sites[] = {
    {"\"click here\" button - off",  0x0042D5FD, 0x0042D606, 9,  CAVE_RGB,      0},
    {"\"click here\" button - on",   0x0042D624, 0x0042D62D, 9,  CAVE_RGB,      1},
    {"hblogo - l",                   0x0042D375, 0x0042D389, 20, CAVE_RGBA,     2},
    {"hblogo - r",                   0x0042D3D2, 0x0042D3E6, 20, CAVE_RGBA,     3},
    {"hamster - only",               0x0042D472, 0x0042D486, 20, CAVE_RGBA,     4},
    {"hamster + ball",               0x0042D4D3, 0x0042D4DD, 10, CAVE_RGBA_FPU, 5},
    {"loader swirl",                 0x0042D5A0, 0x0042D5A9, 9,  CAVE_RGBA_CALL,6},
    {"background",                   0x0042D2B4, 0x0042D2C8, 20, CAVE_RGBA,     7},
    {"raptisoft logo",               0x0042DA60, 0x0042DA74, 20, CAVE_RGBA,     8},
    {"hbversion",                    0x00426433, 0x00426441, 14, CAVE_RGBA,     9},
    {"button #1",                    0x0042DE82, 0x0042DE96, 20, CAVE_RGBA,    10},
    {"button #2",                    0x0042DED8, 0x0042DEEC, 20, CAVE_RGBA,    11},
    {"button #3",                    0x0042DF0D, 0x0042DF21, 20, CAVE_RGBA,    12},
    {"button #4",                    0x0042DF42, 0x0042DF56, 20, CAVE_RGBA,    13},
    {"button #5",                    0x0042E002, 0x0042E016, 20, CAVE_RGBA,    14},
    {"lp button #1",                 0x0042E092, 0x0042E0A6, 20, CAVE_RGBA,    15},
    {"lp button #2",                 0x0042E0E9, 0x0042E0FD, 20, CAVE_RGBA,    16},
    {"lp button #2b",                0x0042E15E, 0x0042E172, 20, CAVE_RGBA,    17},
    {"lp button #3",                 0x0042E12E, 0x0042E142, 20, CAVE_RGBA,    18},
    {"lp button #4",                 0x0042E19C, 0x0042E1B0, 20, CAVE_RGBA,    19},
    {"lp button #5",                 0x0042E1D1, 0x0042E1E5, 20, CAVE_RGBA,    20},
    {"button #1 - resume",           0x0042E722, 0x0042E736, 20, CAVE_RGBA,    21},
    {"button #2 - restart",          0x0042E76F, 0x0042E783, 20, CAVE_RGBA,    22},
    {"button #3 - options",          0x0042E7A4, 0x0042E7B8, 20, CAVE_RGBA,    23},
    {"button #4 - quit",             0x0042E7E2, 0x0042E7F6, 20, CAVE_RGBA,    24},
    {"side strip",                   0x00431A5F, 0x00431A6F, 16, CAVE_RGBA,    25},
    {"side strip r",                 0x00431AA3, 0x00431AB3, 16, CAVE_RGBA,    25},
    {"win strip",                    0x0044D68D, 0x0044D69B, 14, CAVE_RGBA,    26},
};

#define NUM_SITES (sizeof(g_sites) / sizeof(g_sites[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * Config File Management
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_config_path[MAX_PATH] = {0};
static unsigned char* g_cave_mem = NULL;
static int g_reload_counter = 0;

static void init_config_path(void)
{
    char exePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (len > 0) {
        char* last = NULL;
        char* p = exePath;
        while (*p) { if (*p == '\\' || *p == '/') last = p; p++; }
        if (last) {
            *(last + 1) = '\0';
            nc_strncpy(g_config_path, exePath, MAX_PATH - 1);
            nc_strncpy(g_config_path + nc_strlen(g_config_path),
                       "custom_ui_colors.txt", MAX_PATH - nc_strlen(g_config_path) - 1);
            g_config_path[MAX_PATH - 1] = '\0';
            return;
        }
    }
    nc_strncpy(g_config_path, "custom_ui_colors.txt", MAX_PATH - 1);
}

static const char* DEFAULT_CONFIG =
    "- LOADING SCREEN -\r\n"
    "\"Click here\" button - Off = #FFFFFF\r\n"
    "\"Click here\" button - On = #FFFFFF\r\n"
    "LoadingScreen HBLogo - L = #FFFFFFFF\r\n"
    "LoadingScreen HBLogo - R = #FFFFFFFF\r\n"
    "LoadingScreen Hamster - only = #FFFFFFFF\r\n"
    "LoadingScreen Hamster + ball = #FFFFFFFF\r\n"
    "LoadingScreen Loader Swirl = #FFFFFFFF\r\n"
    "LoadingScreen Background = #FFFFFFFF\r\n"
    "LoadingScreen Raptisoft Logo = #FFFFFFFF\r\n"
    "\r\n\r\n"
    "- MAIN MENU -\r\n"
    "MainMenu Colors - HBversion = #FFFFFFFF\r\n"
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
    "\r\n\r\n"
    "- TOURNEY MENU -\r\n"
    "soon.\r\n"
    "\r\n\r\n"
    "- MIRROR MENU -\r\n"
    "soon.\r\n"
    "\r\n\r\n"
    "- TIME TRIALS MENU -\r\n"
    "TimeTrials Pause - Button #1 - Resume = #FFFFFFFF\r\n"
    "TimeTrials Pause - Button #2 - Restart Race = #FFFFFFFF\r\n"
    "TimeTrials Pause - Button #3 - Options = #FFFFFFFF\r\n"
    "TimeTrials Pause - Button #4 - Quit Race = #FFFFFFFF\r\n"
    "\r\n\r\n"
    "- 2P GAME -\r\n"
    "2Pmenu - Side Strip = #FFFFFFFF\r\n"
    "2Pgame - Win Strip = #FFFFFFFF\r\n"
    "\r\n\r\n"
    "- 4P GAME -\r\n"
    "soon.\r\n"
    "\r\n\r\n"
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
            WriteFile(h, DEFAULT_CONFIG, (DWORD)nc_strlen(DEFAULT_CONFIG), &written, NULL);
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
        *out_hex = 0;
        for (int i = 0; i < 8; i++)
            *out_hex = (*out_hex << 4) | hex_digit(buf[i]);
        return 1;
    } else if (digits >= 6) {
        *out_hex = 0;
        for (int i = 0; i < 6; i++)
            *out_hex = (*out_hex << 4) | hex_digit(buf[i]);
        *out_hex = (*out_hex << 8) | 0xFF;
        return 1;
    } else if (digits >= 3) {
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
 * Config Reader
 * ═══════════════════════════════════════════════════════════════════════════ */

static int line_matches(const char* line, const char* needle)
{
    int needle_len = (int)nc_strlen(needle);
    int line_len = (int)nc_strlen(line);
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

static int is_soon(const char* line)
{
    const char* eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;
    return (nc_stricmp(eq, "soon") == 0);
}

static int extract_color(const char* line, DWORD* out_hex)
{
    const char* eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;
    if (*eq == '\0' || *eq == '\r' || *eq == '\n') return 0;
    return parse_hex_color(eq, out_hex);
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

    char* line = buf;
    while (*line) {
        char* nl = line;
        while (*nl && *nl != '\n') nl++;
        int len = (int)(nl - line);
        if (len > 0 && line[len-1] == '\r') len--;

        if (len >= 5 && len < 200 && strchr(line, '=') && !is_soon(line)) {
            for (int i = 0; i < (int)NUM_SITES; i++) {
                if (i == 26) continue; /* "side strip r" — handled by index 25 */

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

        if (*nl == '\n') nl++;
        line = nl;
        if (*line == '\0') break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Cave Builders
 * ═══════════════════════════════════════════════════════════════════════════ */

static int write_push_mem(unsigned char* p, float* val)
{
    p[0] = 0xFF; p[1] = 0x35;
    *(DWORD*)(p + 2) = (DWORD)val;
    return 6;
}

static int write_jmp(unsigned char* p, DWORD target)
{
    p[0] = 0xE9;
    *(DWORD*)(p + 1) = target - ((DWORD)p + 5);
    return 5;
}

static int write_call(unsigned char* p, DWORD target)
{
    p[0] = 0xE8;
    *(DWORD*)(p + 1) = target - ((DWORD)p + 5);
    return 5;
}

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
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    case CAVE_RGBA:
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    case CAVE_RGBA_CALL:
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_call(cave + p, MATRIX_SCALE_4X4);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    case CAVE_RGBA_FPU:
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        nc_memcpy(cave + p, (void*)FMUL_BYTES, sizeof(FMUL_BYTES));
        p += sizeof(FMUL_BYTES);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    }
    return p;
}

static void patch_site(const ColorSite* site, unsigned char* cave_addr)
{
    DWORD old_protect;
    if (!VirtualProtect((void*)site->patchAddr, site->patchSize,
                        PAGE_EXECUTE_READWRITE, &old_protect))
        return;
    *(unsigned char*)site->patchAddr = 0xE9;
    *(DWORD*)(site->patchAddr + 1) = (DWORD)cave_addr - (site->patchAddr + 5);
    for (int i = 5; i < site->patchSize; i++)
        *(unsigned char*)(site->patchAddr + i) = 0x90;
    VirtualProtect((void*)site->patchAddr, site->patchSize, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), (void*)site->patchAddr, site->patchSize);
}

static void install_patches(void)
{
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

    int total = (int)NUM_SITES * (max_cave_size + 4) + 64;
    g_cave_mem = (unsigned char*)VirtualAlloc(NULL, total,
                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_cave_mem) return;

    int offset = 0;
    for (int i = 0; i < (int)NUM_SITES; i++) {
        unsigned char* cave = g_cave_mem + offset;
        int cave_size = build_cave(cave, &g_sites[i]);
        patch_site(&g_sites[i], cave);
        offset += cave_size + 4;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Manual Vtable (HB+ v2.0, 17 entries)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void* (__thiscall *dtor_t)(void*, int);
typedef const char* (__thiscall *get_str_t)(void*);
typedef int (__thiscall *get_int_t)(void*);
typedef void (__thiscall *init_t)(void*, void*);
typedef void (__thiscall *void_cb_t)(void*);
typedef void (__thiscall *ball_update_t)(void*, void*);
typedef void (__thiscall *button_toggle_t)(void*, const char*, bool);
typedef void (__thiscall *slider_change_t)(void*, const char*, float);
typedef void (__thiscall *cycle_change_t)(void*, const char*, const char*);
typedef void (__thiscall *event_collide_t)(void*, void*, char*);
typedef void (__thiscall *ball_bump_t)(void*, void*, void*);
typedef void (__thiscall *render_apply_t)(void*, void*, float*);

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Custom UI Colors"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi)
{
    *(void**)((char*)thisptr + 4) = modApi;

    for (int i = 0; i < NUM_COLOR_SITES; i++) {
        g_colors[i][0] = 1.0f;
        g_colors[i][1] = 1.0f;
        g_colors[i][2] = 1.0f;
        g_colors[i][3] = 1.0f;
    }

    init_config_path();
    create_default_config();
    read_config();
    install_patches();
}

static void __thiscall game_update_impl(void* thisptr)
{
    if (++g_reload_counter >= 120) {
        g_reload_counter = 0;
        read_config();
    }
}

/* No-op stubs */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall scene_end_impl(void*) {}
static void __thiscall level_start_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,
    (void*)get_mod_name,
    (void*)get_author,
    (void*)get_version,
    (void*)get_contributors,
    (void*)init_impl,
    (void*)ball_update_impl,
    (void*)render_apply_impl,
    (void*)button_toggle_impl,
    (void*)slider_change_impl,
    (void*)cycle_change_impl,
    (void*)game_update_impl,
    (void*)event_collide_impl,
    (void*)text_render_impl,
    (void*)ball_bump_impl,
    (void*)scene_end_impl,
    (void*)level_start_impl,
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

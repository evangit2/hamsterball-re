/*
 * mknp_player_colors.cpp — Per-player ball colors (HB+ v2.1, MinGW)
 *
 * HB+ port of the ball_tint bass.dll proxy (v5). No BASS exports, no
 * threads: the HB+ loader scans Mods\, calls CreateModInstance, fires
 * Initialize(), then ticks onGameUpdate() every frame.
 *
 * WHAT THIS MOD DOES
 * ------------------
 * Tints player balls to hex colors read from mknp_player_colors.set
 * (jsonc, lives NEXT TO THIS DLL, e.g. Mods\). Supports all 4 players
 * across 1P/2P/4P modes. Config re-read ~every 2s for live editing.
 *
 * Set file (mknp_player_colors.set), 7 keys:
 *   p1_single: Player 1 color in 1-player mode
 *   p1_2p:     Player 1 color in 2-player mode
 *   p1_4p:     Player 1 color in 4-player mode (Arena/Rumble)
 *   p2_2p:     Player 2 color in 2-player mode
 *   p2_4p:     Player 2 color in 4-player mode (Arena/Rumble)
 *   p3_4p:     Player 3 color in 4-player mode (Arena/Rumble)
 *   p4_4p:     Player 4 color in 4-player mode (Arena/Rumble)
 *
 * Board color table (set by Board_ctor's Vec3_Init calls):
 *   board+0x3AB0 = P1, +0x3AC4 = P2, +0x3AD8 = P3, +0x3AEC = P4
 *   (4 RGBA floats each, 0x14 stride). Scene == Board via GetScene().
 * Player count from Scene.player_count (+0x3630, API struct field).
 *
 * Own-DLL dir resolved via VirtualQuery (mod_self), same as entities_plus.
 *
 * Author: MAKYUNI / Hamsterbot
 */

#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#include <windows.h>

#define strlen  nc_strlen
#define strncpy nc_strncpy
#define snprintf nc_snprintf
#define memcpy  nc_memcpy

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BOARD_COLOR_BASE    0x3AB0
#define BOARD_COLOR_STRIDE  0x14

#define NUM_COLORS 7
static unsigned int g_colors[NUM_COLORS] = {
    0xFFFFFF, 0x4A90D9, 0x2ECC71,
    0xFF6B35, 0xE74C3C, 0x9B59B6, 0xF1C40F
};

static const char* K_KEYS[NUM_COLORS] = {
    "p1_single", "p1_2p", "p1_4p",
    "p2_2p", "p2_4p", "p3_4p", "p4_4p"
};

static IModAPI* g_api = NULL;
static HMODULE  g_module = NULL;

static char g_dll_dir[MAX_PATH];
static char g_config_path[MAX_PATH];

static int g_started = 0;
static unsigned int g_tick = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE mod_self(void) {
    if (g_module) return g_module;
    MEMORY_BASIC_INFORMATION mbi;
    nc_memset(&mbi, 0, sizeof(mbi));
    if (VirtualQuery((void*)mod_self, &mbi, sizeof(mbi)))
        g_module = (HMODULE)mbi.AllocationBase;
    return g_module;
}

static void path_join(char* dst, const char* dir, const char* tail) {
    nc_strncpy(dst, dir, MAX_PATH - 1);
    dst[MAX_PATH - 1] = '\0';
    size_t dl = nc_strlen(dst);
    size_t i = 0;
    while (tail[i] && dl + 1 < MAX_PATH) dst[dl++] = tail[i++];
    dst[dl] = '\0';
}

/* Parse hex color: optional # / 0x, 6 digits (or 3-digit shorthand). */
static unsigned int parse_hex(const char* text) {
    const char* p = text;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p == '#') p++;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
    unsigned int hex = 0;
    int digits = 0;
    while (digits < 6 && *p) {
        char c = *p;
        unsigned int v;
        if (c >= '0' && c <= '9') v = (unsigned int)(c - '0');
        else if (c >= 'a' && c <= 'f') v = (unsigned int)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') v = (unsigned int)(c - 'A' + 10);
        else break;
        hex = (hex << 4) | v;
        digits++;
        p++;
    }
    if (digits >= 6) return hex;
    if (digits >= 3) {
        unsigned int r = ((hex >> 8) & 0xF) * 0x11;
        unsigned int g = ((hex >> 4) & 0xF) * 0x11;
        unsigned int b = (hex & 0xF) * 0x11;
        return (r << 16) | (g << 8) | b;
    }
    return 0xFFFFFF;
}

static void create_defaults(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); return; }
    const char* defs =
        "// mknp_player_colors settings (jsonc — // and /* */ comments allowed)\r\n"
        "{\r\n"
        "  \"p1_single\": \"FFFFFF\",   // P1 in 1-player mode\r\n"
        "  \"p1_2p\":     \"4A90D9\",   // P1 in 2-player mode\r\n"
        "  \"p1_4p\":     \"2ECC71\",   // P1 in 4-player mode (Arena/Rumble)\r\n"
        "  \"p2_2p\":     \"FF6B35\",   // P2 in 2-player mode\r\n"
        "  \"p2_4p\":     \"E74C3C\",   // P2 in 4-player mode (Arena/Rumble)\r\n"
        "  \"p3_4p\":     \"9B59B6\",   // P3 in 4-player mode (Arena/Rumble)\r\n"
        "  \"p4_4p\":     \"F1C40F\"    // P4 in 4-player mode (Arena/Rumble)\r\n"
        "}\r\n";
    h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD wrote = 0;
        WriteFile(h, defs, (DWORD)nc_strlen(defs), &wrote, NULL);
        CloseHandle(h);
    }
}

/* Strip jsonc comments (// to EOL, slash-star blocks) into dst. Strings respected. */
static int strip_jsonc(const char* src, int n, char* dst, int cap) {
    int i = 0, o = 0;
    int in_str = 0;
    while (i < n && o + 1 < cap) {
        char c = src[i];
        if (in_str) {
            dst[o++] = c;
            if (c == '\\' && i + 1 < n) { dst[o++] = src[++i]; i++; continue; }
            if (c == '"') in_str = 0;
            i++;
            continue;
        }
        if (c == '"') { in_str = 1; dst[o++] = c; i++; continue; }
        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            i += 2;
            while (i < n && src[i] != '\n') i++;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(src[i] == '*' && src[i + 1] == '/')) i++;
            i += 2;
            continue;
        }
        dst[o++] = c;
        i++;
    }
    dst[o] = '\0';
    return o;
}

/* Find "key" : "value" — copies value (no escapes) into out. Returns 1 on hit. */
static int find_key_str(const char* buf, const char* key, char* out, int cap) {
    int kl = 0;
    while (key[kl]) kl++;
    const char* p = buf;
    while (*p) {
        if (*p != '"') { p++; continue; }
        const char* k = p + 1;
        int i = 0;
        while (i < kl && k[i] == key[i]) i++;
        if (i != kl || k[i] != '"') { p++; continue; }
        p = k + i + 1;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p != ':') continue;
        p++;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p != '"') continue;
        p++;
        int o = 0;
        while (*p && *p != '"' && o + 1 < cap) out[o++] = *p++;
        out[o] = '\0';
        return (*p == '"');
    }
    return 0;
}

static void read_set(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    static char raw[2048]; /* static: -nostdlib has no __chkstk_ms */
    static char buf[2048];
    DWORD got = 0;
    ReadFile(h, raw, sizeof(raw) - 1, &got, NULL);
    CloseHandle(h);
    if (got == 0) return;
    raw[got] = '\0';

    strip_jsonc(raw, (int)got, buf, sizeof(buf));
    for (int k = 0; k < NUM_COLORS; k++) {
        char val[32];
        if (find_key_str(buf, K_KEYS[k], val, sizeof(val)))
            g_colors[k] = parse_hex(val);
        /* missing key: keep current value (partial sets allowed) */
    }
}

static void write_slot(DWORD board, int slot, unsigned int hex) {
    float r = (float)((hex >> 16) & 0xFF) / 255.0f;
    float g = (float)((hex >> 8) & 0xFF) / 255.0f;
    float b = (float)(hex & 0xFF) / 255.0f;
    DWORD addr = board + BOARD_COLOR_BASE + (DWORD)slot * BOARD_COLOR_STRIDE;
    *(float*)(addr + 0x00) = r;
    *(float*)(addr + 0x04) = g;
    *(float*)(addr + 0x08) = b;
    *(float*)(addr + 0x0C) = 1.0f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ callback wiring (17-entry vtable, MinGW __thiscall convention)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "mknp_player_colors"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    *(void**)((char*)thisptr + 4) = api;
    mod_self();

    g_dll_dir[0] = '\0';
    if (g_module && GetModuleFileNameA(g_module, g_dll_dir, MAX_PATH) > 0) {
        char* s = NULL;
        for (char* p = g_dll_dir; *p; p++) if (*p == '\\') s = p;
        if (s) s[1] = '\0';
        else g_dll_dir[0] = '\0';
    }
    if (g_dll_dir[0])
        path_join(g_config_path, g_dll_dir, "mknp_player_colors.set");
    else
        nc_strncpy(g_config_path, "mknp_player_colors.set", MAX_PATH - 1);

    create_defaults();
    read_set();
    g_started = 1;
}

static void __thiscall game_update(void*) {
    if (!g_started || !g_api) return;
    g_tick++;
    if ((g_tick % 120) == 0) read_set(); /* live edit ~every 2s */

    Scene* sc = HBAPI(g_api).GetScene();
    if (!sc) return;
    int np = sc->player_count;
    if (np < 1) np = 1;
    if (np > 4) np = 4;
    DWORD board = (DWORD)sc;

    int p1 = (np <= 1) ? 0 : (np == 2) ? 1 : 2;
    write_slot(board, 0, g_colors[p1]);
    if (np >= 2) write_slot(board, 1, g_colors[(np >= 3) ? 4 : 3]);
    if (np >= 3) write_slot(board, 2, g_colors[5]);
    if (np >= 4) write_slot(board, 3, g_colors[6]);
}

static void __thiscall ball_update(void*, void*) {}
static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall button_toggle(void*, const char*, bool) {}
static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall cycle_option_change(void*, const char*, const char*) {}
static void __thiscall event_collide(void*, void*, char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, void*, void*) {}
static void __thiscall scene_end(void*) {}
static void __thiscall level_start(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]  ~HamsterballAPI
    (void*)get_mod_name,         // [1]  GetModName
    (void*)get_author,           // [2]  GetAuthorName
    (void*)get_version,          // [3]  GetApiVersion
    (void*)get_contributors,     // [4]  GetContributors
    (void*)init_impl,            // [5]  Initialize
    (void*)ball_update,          // [6]  onBallUpdate
    (void*)render_apply,         // [7]  onRenderApply
    (void*)button_toggle,        // [8]  onButtonToggle
    (void*)slider_change,        // [9]  onSliderChange
    (void*)cycle_option_change,  // [10] onCycleOptionChange (v2.0+)
    (void*)game_update,          // [11] onGameUpdate
    (void*)event_collide,        // [12] onEventPlaneCollide
    (void*)text_render,          // [13] onTextRenderLoop
    (void*)ball_bump,            // [14] onBallBump
    (void*)scene_end,            // [15] onSceneEnd
    (void*)level_start,          // [16] onLevelStart
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

/* DllMain: loader-lock rules — DisableThreadLibraryCalls ONLY. */
extern "C" BOOL __stdcall DllMain(HINSTANCE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(h);
    return TRUE;
}

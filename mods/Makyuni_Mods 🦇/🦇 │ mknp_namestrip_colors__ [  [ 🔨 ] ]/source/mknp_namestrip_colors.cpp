/*
 * mknp_namestrip_colors.cpp — Per-level name-strip colors (HB+ v2.1, MinGW)
 *
 * HB+ port of the name_strips bass.dll proxy (now mknp_namestrip_colors). No BASS exports, no
 * threads: the HB+ loader scans Mods\, calls CreateModInstance, fires
 * Initialize() (installs the code cave one-shot), then ticks
 * onGameUpdate() every frame (level match + cave float updates).
 *
 * WHAT THIS MOD DOES
 * ------------------
 * Custom per-level RGBA colors for the horizontal strip behind the
 * Race/Arena name shown at the start of each race. Stock game uses one
 * global yellow strip (R=1.0, G=1.0, B=0.0, A=0.75, pushed at
 * 0x41B763-0x41B76B in Scene_DrawNameStrip). This mod redirects that push
 * sequence into a code cave with patchable float operands, updated per
 * level from mknp_namestrip_colors.txt (lives NEXT TO THIS DLL, e.g.
 * Mods\). Config re-read ~every 2s for live editing.
 *
 * Config format: "Level N =" / RGBA lines, N=1..15 races + "Arena N =".
 * See shipped txt. Levels missing from the file keep stock yellow.
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
#define memcmp  nc_memcmp

/* ═══════════════════════════════════════════════════════════════════════════
 * Addresses
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PATCH_ADDR       0x0041B763
#define PATCH_SIZE       14
#define RETURN_ADDR      0x0041B771
#define MATRIX_SCALE     0x00453150

static const BYTE g_orig[14] = {
    0x68, 0x00, 0x00, 0x40, 0x3F,
    0x6A, 0x00,
    0x55,
    0x55,
    0xE8, 0xDF, 0x79, 0x03, 0x00
};

#define DEFAULT_R  1.0f
#define DEFAULT_G  1.0f
#define DEFAULT_B  0.0f
#define DEFAULT_A  0.75f

#define NUM_LEVELS 15
#define NUM_ENTRIES 30

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static IModAPI* g_api = NULL;
static HMODULE  g_module = NULL;

static char g_dll_dir[MAX_PATH];
static char g_config_path[MAX_PATH];

static int g_started = 0;
static unsigned int g_tick = 0;
static int g_cave_ok = 0;

static BYTE* g_cave = NULL;
static DWORD g_cave_off[4]; /* [0]=R [1]=G [2]=B [3]=A imm32 offsets */

typedef struct { float r, g, b, a; int valid; } StripColor;
static StripColor g_colors[NUM_ENTRIES];

static const char* const g_level_names[NUM_LEVELS] = {
    "Warm-Up", "Beginner", "Intermediate", "Dizzy", "Tower",
    "Up", "Neon", "Expert", "Odd", "Toob",
    "Wobbly", "Glass", "Sky", "Master", "Impossible"
};

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

static char to_lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

static int contains_ci(const char* hay, const char* ndl) {
    if (!hay || !*hay || !ndl || !*ndl) return 0;
    size_t nl = nc_strlen(ndl), hl = nc_strlen(hay);
    if (nl > hl) return 0;
    for (size_t i = 0; i <= hl - nl; i++) {
        size_t j = 0;
        while (j < nl && to_lower(hay[i + j]) == to_lower(ndl[j])) j++;
        if (j == nl) return 1;
    }
    return 0;
}

static int match_level(const char* name) {
    static const char* const kw[NUM_LEVELS] = {
        "WARM", "BEGINNER", "INTERMED", "DIZZY", "TOWER",
        "UP", "NEON", "EXPERT", "ODD", "TOOB",
        "WOBBLY", "GLASS", "SKY", "MASTER", "IMPOSSIBLE"
    };
    if (!name || !*name) return -1;
    for (int i = 0; i < NUM_LEVELS; i++)
        if (contains_ci(name, kw[i])) return i;
    return -1;
}

/* manual float parse (no CRT strtof under -nostdlib) */
static int parse_float(const char* s, float* out) {
    while (*s == ' ' || *s == '\t') s++;
    if (!*s || *s == '\r' || *s == '\n') return 0;
    int neg = 0;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;
    unsigned int ip = 0;
    int digits = 0;
    while (*s >= '0' && *s <= '9') { ip = ip * 10 + (unsigned int)(*s - '0'); s++; digits = 1; }
    float v = (float)ip;
    if (*s == '.') {
        s++;
        float div = 10.0f;
        while (*s >= '0' && *s <= '9') {
            v += (float)(*s - '0') / div;
            div *= 10.0f;
            s++;
            digits = 1;
        }
    }
    if (!digits) return 0;
    *out = neg ? -v : v;
    return 1;
}

/* find "X =" (any case) then parse float after '=' */
static int extract_float(const char* line, char key, float* out) {
    for (const char* p = line; *p; p++) {
        if (to_lower(*p) == to_lower(key)) {
            const char* q = p + 1;
            while (*q == ' ' || *q == '\t') q++;
            if (*q == '=') return parse_float(q + 1, out);
        }
    }
    return 0;
}

static int parse_num(const char* s) {
    while (*s == ' ' || *s == '\t') s++;
    if (!*s) return -1;
    int v = 0, any = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; any = 1; }
    return any ? v : -1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config
 * ═══════════════════════════════════════════════════════════════════════════ */

static void read_config(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    static char buf[16384]; /* static: -nostdlib has no __chkstk_ms */
    DWORD got = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &got, NULL);
    CloseHandle(h);
    if (got == 0) return;
    buf[got] = '\0';

    for (int i = 0; i < NUM_ENTRIES; i++) g_colors[i].valid = 0;

    char* p = buf;
    while (*p) {
        char* nl = p;
        while (*nl && *nl != '\n') nl++;
        char saved = *nl;
        *nl = '\0';

        char* line = p;
        while (*line == ' ' || *line == '\t') line++;
        if (*line && *line != ';' && *line != '#' && *line != '\r') {
            int arena = contains_ci(line, "Arena") ? 1 : 0;
            int has_hdr = arena || contains_ci(line, "Level");
            if (has_hdr) {
                /* number = digits right after the Level/Arena keyword */
                const char* kw = arena ? "arena" : "level";
                const char* at = NULL;
                for (const char* q = line; *q; q++) {
                    int k = 1;
                    for (int i = 0; kw[i]; i++)
                        if (!q[i] || to_lower(q[i]) != kw[i]) { k = 0; break; }
                    if (k) { at = q + nc_strlen(kw); break; }
                }
                int n = at ? parse_num(at) : -1;
                if (n >= 1 && n <= 15) {
                    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
                    int fr = 0, fg = 0, fb = 0, fa = 0;
                    fr = extract_float(line, 'R', &r);
                    fg = extract_float(line, 'G', &g);
                    fb = extract_float(line, 'B', &b);
                    fa = extract_float(line, 'A', &a);
                    if (!(fr && fg && fb && fa) && saved == '\n') {
                        /* values live on the next line */
                        char* next = nl + 1;
                        char* nn = next;
                        while (*nn && *nn != '\n') nn++;
                        char s2 = *nn;
                        *nn = '\0';
                        if (!fr) fr = extract_float(next, 'R', &r);
                        if (!fg) fg = extract_float(next, 'G', &g);
                        if (!fb) fb = extract_float(next, 'B', &b);
                        if (!fa) fa = extract_float(next, 'A', &a);
                        *nn = s2;
                    }
                    if (fr || fg || fb || fa) {
                        int idx = (arena ? NUM_LEVELS : 0) + (n - 1);
                        g_colors[idx].r = r;
                        g_colors[idx].g = g;
                        g_colors[idx].b = b;
                        g_colors[idx].a = a;
                        g_colors[idx].valid = 1;
                    }
                }
            }
        }
        *nl = saved;
        p = (*nl == '\n') ? nl + 1 : nl;
    }
}

static void create_defaults(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); return; }
    static char buf[8192]; /* static: -nostdlib has no __chkstk_ms */
    int pos = 0;
    pos += wsprintfA(buf + pos,
        "; mknp_namestrip_colors config — per-level name-strip RGBA\r\n"
        "; Values are floats (0.0 to 1.0). Stock: R=1.0 G=1.0 B=0.0 A=0.75\r\n"
        "; Levels 1-15 = Races, Arenas 1-15 = Arenas\r\n"
        "; Re-read ~every 2 seconds. Edit at runtime!\r\n"
        "\r\n");
    for (int i = 0; i < NUM_LEVELS; i++) {
        pos += wsprintfA(buf + pos, "; %s\r\n", g_level_names[i]);
        pos += wsprintfA(buf + pos, "Level %d =\r\n", i + 1);
        pos += wsprintfA(buf + pos, "R = 1.0, G = 1.0, B = 1.0, A = 1.0\r\n\r\n");
    }
    for (int i = 0; i < NUM_LEVELS; i++) {
        pos += wsprintfA(buf + pos, "; %s Arena\r\n", g_level_names[i]);
        pos += wsprintfA(buf + pos, "Arena %d =\r\n", i + 1);
        pos += wsprintfA(buf + pos, "R = 1.0, G = 1.0, B = 1.0, A = 1.0\r\n\r\n");
    }
    h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD wrote = 0;
        WriteFile(h, buf, (DWORD)pos, &wrote, NULL);
        CloseHandle(h);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code cave (installed one-shot in Initialize)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void patch_mem(DWORD addr, const void* data, DWORD len) {
    DWORD old = 0;
    if (VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &old)) {
        const BYTE* s = (const BYTE*)data;
        BYTE* d = (BYTE*)addr;
        for (DWORD i = 0; i < len; i++) d[i] = s[i];
        VirtualProtect((void*)addr, len, old, &old);
    }
}

static void cave_write_float(int idx, float v) {
    if (!g_cave || !g_cave_ok) return;
    patch_mem((DWORD)g_cave + g_cave_off[idx], &v, 4);
}

static int install_cave(void) {
    if (g_cave_ok) return 1;
    if (nc_memcmp((void*)PATCH_ADDR, g_orig, PATCH_SIZE) != 0) return 0;

    g_cave = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                 PAGE_EXECUTE_READWRITE);
    if (!g_cave) return 0;

    DWORD off = 0;
    float inits[4] = { DEFAULT_A, DEFAULT_B, DEFAULT_G, DEFAULT_R };
    int map[4] = { 3, 2, 1, 0 }; /* push order A,B,G,R -> slot index */
    for (int i = 0; i < 4; i++) {
        g_cave_off[map[i]] = off + 1;
        g_cave[off] = 0x68;
        *(DWORD*)(g_cave + off + 1) = *(DWORD*)&inits[i];
        off += 5;
    }
    DWORD ca = (DWORD)g_cave + off;
    g_cave[off] = 0xE8;
    *(DWORD*)(g_cave + off + 1) = MATRIX_SCALE - (ca + 5);
    off += 5;
    DWORD ja = (DWORD)g_cave + off;
    g_cave[off] = 0xE9;
    *(DWORD*)(g_cave + off + 1) = RETURN_ADDR - (ja + 5);

    BYTE jmp[5] = { 0xE9, 0, 0, 0, 0 };
    *(DWORD*)(jmp + 1) = (DWORD)g_cave - (PATCH_ADDR + 5);
    patch_mem(PATCH_ADDR, jmp, 5);
    BYTE nops[9] = { 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90 };
    patch_mem(PATCH_ADDR + 5, nops, 9);

    g_cave_ok = 1;
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ callback wiring (17-entry vtable, MinGW __thiscall convention)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "mknp_namestrip_colors"; }
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
        path_join(g_config_path, g_dll_dir, "mknp_namestrip_colors.txt");
    else
        nc_strncpy(g_config_path, "mknp_namestrip_colors.txt", MAX_PATH - 1);

    create_defaults();
    read_config();
    install_cave(); /* silent skip on byte mismatch */
    g_started = 1;
}

static void __thiscall game_update(void*) {
    if (!g_started || !g_api || !g_cave_ok) return;
    g_tick++;
    if ((g_tick % 120) == 0) read_config(); /* live edit ~every 2s */

    Scene* sc = HBAPI(g_api).GetScene();
    if (!sc || !sc->name) return;
    if ((unsigned char)sc->name[0] < 0x20 || (unsigned char)sc->name[0] > 0x7E) return;

    int level = match_level(sc->name);
    if (level < 0) return;
    int idx = contains_ci(sc->name, "ARENA") ? level + NUM_LEVELS : level;

    float r = DEFAULT_R, g = DEFAULT_G, b = DEFAULT_B, a = DEFAULT_A;
    if (g_colors[idx].valid) {
        r = g_colors[idx].r;
        g = g_colors[idx].g;
        b = g_colors[idx].b;
        a = g_colors[idx].a;
    }
    cave_write_float(0, r);
    cave_write_float(1, g);
    cave_write_float(2, b);
    cave_write_float(3, a);
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

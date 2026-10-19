/*
 * mknp_race_descriptions.cpp — Custom tourney race texts (HB+ v2.1, MinGW)
 *
 * HB+ port of the race_descriptions bass.dll proxy (now mknp_race_descriptions). No BASS
 * exports, no threads: the HB+ loader scans Mods\, calls
 * CreateModInstance, fires Initialize(), where the pointer table is
 * patched one-shot.
 *
 * WHAT THIS MOD DOES
 * ------------------
 * Customizes the 15 tournament race description texts on the Tourney
 * Menu screen. Stock game stores them as a hardcoded pointer table at
 * 0x4F7148 (15 char*). This mod overwrites those pointers with strings
 * from mknp_race_descriptions.txt (lives NEXT TO THIS DLL, e.g. Mods\).
 *
 * Config format:
 *   [Level 1]
 *   Description text, one or more lines (joined with spaces).
 *   ...through [Level 15]. Lines starting with # or ; are comments.
 * Missing levels keep the stock description. The game word-wraps via
 * Font_WordWrap (375px), so long texts wrap automatically.
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
#define malloc  nc_malloc
#define free    nc_free

/* ═══════════════════════════════════════════════════════════════════════════
 * Table
 * ═══════════════════════════════════════════════════════════════════════════ */

#define DESC_TABLE_ADDR  0x004F7148
#define NUM_RACES        15

static const DWORD g_orig_ptrs[NUM_RACES] = {
    0x004D8238, 0x004D8128, 0x004D7FD8, 0x004D7E78, 0x004D7D20,
    0x004D7B88, 0x004D7A60, 0x004D78D8, 0x004D7718, 0x004D7570,
    0x004D7408, 0x004D7290, 0x004D7108, 0x004D6FB8, 0x004D6E60
};

static const char* const g_race_names[NUM_RACES] = {
    "Warm-Up Race", "Beginner Race", "Intermediate Race", "Dizzy Race",
    "Tower Race", "Up Race", "Neon Race", "Expert Race", "Odd Race",
    "Toob Race", "Wobbly Race", "Glass Race", "Sky Race", "Master Race",
    "Impossible Race"
};

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static IModAPI* g_api = NULL;
static HMODULE  g_module = NULL;

static char g_dll_dir[MAX_PATH];
static char g_config_path[MAX_PATH];

static char* g_descs[NUM_RACES] = { 0 };
static int   g_loaded[NUM_RACES] = { 0 };
static int   g_started = 0;

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

/* decimal parse, -1 on garbage */
static int parse_num(const char* s) {
    while (*s == ' ' || *s == '\t') s++;
    if (!*s) return -1;
    int v = 0, any = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; any = 1; }
    return any ? v : -1;
}

/* case-insensitive "level " prefix check */
static int is_level_hdr(const char* hdr, size_t len, int* num_out) {
    static const char* const pre = "level ";
    if (len < 7) return 0;
    for (int i = 0; i < 6; i++)
        if (to_lower(hdr[i]) != pre[i]) return 0;
    char tmp[16];
    size_t n = len - 6;
    if (n > 15) n = 15;
    for (size_t i = 0; i < n; i++) tmp[i] = hdr[6 + i];
    tmp[n] = '\0';
    int v = parse_num(tmp);
    if (v < 1 || v > NUM_RACES) return 0;
    *num_out = v;
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config
 * ═══════════════════════════════════════════════════════════════════════════ */

static void parse_buf(char* buf) {
    char* tmp[NUM_RACES] = { 0 };
    int   lens[NUM_RACES] = { 0 };
    int   caps[NUM_RACES] = { 0 };
    for (int i = 0; i < NUM_RACES; i++) {
        caps[i] = 512;
        tmp[i] = (char*)nc_malloc((size_t)caps[i]);
        if (tmp[i]) tmp[i][0] = '\0';
    }

    int cur = -1;
    char* p = buf;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        char* content = p;
        while (*p && *p != '\r' && *p != '\n') p++;
        int llen = (int)(p - content);
        if (*p == '\r') p++;
        if (*p == '\n') p++;
        if (llen == 0) continue;
        if (content[0] == '#' || content[0] == ';') continue;

        if (content[0] == '[') {
            const char* close = content;
            while (close < content + llen && *close != ']') close++;
            if (close < content + llen) {
                int n = 0;
                if (is_level_hdr(content + 1, (size_t)(close - content - 1), &n)) {
                    cur = n - 1;
                    if (tmp[cur]) { tmp[cur][0] = '\0'; lens[cur] = 0; }
                } else {
                    cur = -1;
                }
            }
            continue;
        }
        if (cur >= 0 && cur < NUM_RACES && tmp[cur]) {
            int need_sp = (lens[cur] > 0) ? 1 : 0;
            int actual = llen;
            while (actual > 0 && (content[actual-1] == ' ' || content[actual-1] == '\t'))
                actual--;
            int total = lens[cur] + need_sp + actual + 1;
            if (total > caps[cur]) {
                while (caps[cur] < total) caps[cur] *= 2;
                char* nb = (char*)nc_malloc((size_t)caps[cur]);
                if (!nb) continue;
                nc_memcpy(nb, tmp[cur], (size_t)(lens[cur] + 1));
                nc_free(tmp[cur]);
                tmp[cur] = nb;
            }
            if (need_sp) tmp[cur][lens[cur]++] = ' ';
            if (actual > 0) {
                nc_memcpy(tmp[cur] + lens[cur], content, (size_t)actual);
                lens[cur] += actual;
            }
            tmp[cur][lens[cur]] = '\0';
        }
    }
    for (int i = 0; i < NUM_RACES; i++) {
        if (tmp[i] && lens[i] > 0) {
            g_descs[i] = tmp[i];
            g_loaded[i] = 1;
        } else {
            if (tmp[i]) nc_free(tmp[i]);
            g_loaded[i] = 0;
        }
    }
}

static void generate_defaults(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    const char* head =
        "# mknp_race_descriptions — tourney menu texts per race\r\n"
        "# Format:\r\n"
        "#   [Level N]\r\n"
        "#   Your description text here.\r\n"
        "#   Multi-line descriptions are joined with spaces.\r\n"
        "# Lines starting with # or ; are comments.\r\n"
        "# Missing levels keep the stock game description.\r\n"
        "\r\n";
    DWORD wrote = 0;
    WriteFile(h, head, (DWORD)nc_strlen(head), &wrote, NULL);
    for (int i = 0; i < NUM_RACES; i++) {
        const char* desc = (const char*)g_orig_ptrs[i];
        char hdr[64];
        nc_snprintf(hdr, sizeof(hdr), "[Level %d]  # %s\r\n", i + 1, g_race_names[i]);
        WriteFile(h, hdr, (DWORD)nc_strlen(hdr), &wrote, NULL);
        WriteFile(h, desc, (DWORD)nc_strlen(desc), &wrote, NULL);
        WriteFile(h, "\r\n\r\n", 4, &wrote, NULL);
    }
    CloseHandle(h);
}

static void load_config(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        generate_defaults();
        return;
    }
    DWORD size = GetFileSize(h, NULL);
    if (size == 0 || size == INVALID_FILE_SIZE || size > 1024 * 1024) {
        CloseHandle(h);
        return;
    }
    char* buf = (char*)nc_malloc(size + 1);
    if (!buf) { CloseHandle(h); return; }
    DWORD got = 0;
    ReadFile(h, buf, size, &got, NULL);
    CloseHandle(h);
    buf[got] = '\0';
    parse_buf(buf);
    nc_free(buf);
}

static void apply_table(void) {
    DWORD* tbl = (DWORD*)DESC_TABLE_ADDR;
    DWORD old = 0;
    if (!VirtualProtect(tbl, NUM_RACES * 4, PAGE_READWRITE, &old)) return;
    for (int i = 0; i < NUM_RACES; i++)
        if (g_loaded[i] && g_descs[i]) tbl[i] = (DWORD)g_descs[i];
    VirtualProtect(tbl, NUM_RACES * 4, old, &old);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ callback wiring (17-entry vtable, MinGW __thiscall convention)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "mknp_race_descriptions"; }
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
        path_join(g_config_path, g_dll_dir, "mknp_race_descriptions.txt");
    else
        nc_strncpy(g_config_path, "mknp_race_descriptions.txt", MAX_PATH - 1);

    load_config();
    apply_table();
    g_started = 1;
}

static void __thiscall game_update(void*) {
    (void)g_started; /* one-shot at Initialize; nothing per-tick */
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

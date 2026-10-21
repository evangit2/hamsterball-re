/*
 * mknp_custom_credits.cpp — Editable Credits Screen Text (HB+ v2.1, MinGW)
 *
 * HB+ port of the custom_credits bass.dll proxy. No BASS exports, no
 * threads: the HB+ loader scans Mods\, calls CreateModInstance, fires
 * Initialize(), where the 44 string-pointer patches are applied one-shot.
 *
 * WHAT THIS MOD DOES
 * ------------------
 * Reads mknp_custom_credits.txt (lives NEXT TO THIS DLL, e.g. Mods\) and
 * replaces the hard-coded credits strings PUSHed before AthenaList_Append
 * calls in CreditsScreen_ctor (0x4254e0). Each of the 44 lines is a
 * PUSH imm32 (0x68 + string ptr); the 4-byte immediate is repointed at
 * our own buffer. Log: mknp_custom_credits_log.txt, also next to the DLL.
 *
 * Config format (mknp_custom_credits.txt):
 *   N=text     — Line N (1-44) gets the text after =
 *   #comment   — Comment line, ignored
 *   N=         — Empty text = blank separator line
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

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

#define NUM_CREDITS_LINES 44
#define MAX_CREDITS_LEN   80

static IModAPI* g_api = NULL;
static HMODULE  g_module = NULL;

static char g_dll_dir[MAX_PATH];    /* folder of this DLL, trailing \ (Mods\) */
static char g_config_path[MAX_PATH];/* mknp_custom_credits.txt next to DLL */
static char g_log_path[MAX_PATH];   /* mknp_custom_credits_log.txt next to DLL */

static int g_started = 0;

/* RVA addresses of the 44 PUSH instructions in CreditsScreen_ctor (0x4254e0).
 * Each is: 68 XX XX XX XX (PUSH imm32, 5 bytes). */
static const unsigned int g_push_rvas[NUM_CREDITS_LINES] = {
    0x25535, 0x2556A, 0x25576, 0x25582, 0x2558E, 0x2559A, 0x255A6, 0x255B2,
    0x255BE, 0x255CA, 0x255D6, 0x255E2, 0x255EE, 0x255FA, 0x25606, 0x25612,
    0x2561E, 0x2562A, 0x25636, 0x25642, 0x2564E, 0x2565A, 0x25666, 0x25672,
    0x2567E, 0x2568A, 0x25696, 0x256A2, 0x256AE, 0x256BA, 0x256C6, 0x256D2,
    0x256DE, 0x256EA, 0x256F6, 0x25702, 0x2570E, 0x2571A, 0x25726, 0x25732,
    0x2573E, 0x2574A, 0x25756, 0x25762
};

/* Default credits text (from original game binary) */
static const char* const g_defaults[NUM_CREDITS_LINES] = {
    "-RAPTISOFT GAMES PRESENTS", "HAMSTERBALL", "", "",
    "-GAME DESIGN", "JOHN RAPTIS", "", "-PROGRAMMING", "NICK RAPTIS", "",
    "-PHYSICS", "PAUL NETTLE", "*www.fluidstudios.com", "",
    "-LEVEL AND CREATURE DESIGN", "JOHN RAPTIS", "GARY CLAIR",
    "*www.garyclair.com", "", "-MUSIC", "SKAVEN",
    "*www.futurecrew.com/skaven", "", "-RODENT MODELS", "ENDBOSSGAMES",
    "*www.endbossgames.com", "", "-LEVEL MODELS", "STEVE WARNER", "",
    "-PLAY BALANCING", "BRADY WRIGHT", "*www.anbsoft.com", "",
    "-GAME INSTALLER", "BRIAN FIETE", "*www.popcap.com", "", "",
    "-AND SPECIAL THANKS TO", "-ALL THE FINE FOLKS AT", "-FLIPCODE.COM!",
    "", ""
};

/* Live string buffers (stable pointers, kept for game lifetime) */
static char g_lines[NUM_CREDITS_LINES][MAX_CREDITS_LEN + 1];
static char g_empty[1] = { 0 };

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

/* append-only log next to the DLL */
static void mod_log(const char* msg) {
    HANDLE h = CreateFileA(g_log_path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    SetFilePointer(h, 0, NULL, FILE_END);
    DWORD wrote = 0;
    WriteFile(h, msg, (DWORD)nc_strlen(msg), &wrote, NULL);
    WriteFile(h, "\r\n", 2, &wrote, NULL);
    CloseHandle(h);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config
 * ═══════════════════════════════════════════════════════════════════════════ */

static void trim(char* s) {
    while (*s == ' ' || *s == '\t') {
        char* p = s;
        while (*p) { *p = p[1]; p++; }
    }
    char* e = s + nc_strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t' ||
                     e[-1] == '\n' || e[-1] == '\r')) *--e = '\0';
}

/* decimal parse, returns -1 on garbage */
static int parse_num(const char* s) {
    if (!*s) return -1;
    int v = 0;
    for (; *s; s++) {
        if (*s < '0' || *s > '9') return -1;
        v = v * 10 + (*s - '0');
    }
    return v;
}

static void generate_defaults(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_WRITE, 0,
                           NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        mod_log("[mknp_custom_credits] Could not generate default config");
        return;
    }
    const char* head =
        "# Custom Credits Configuration\r\n"
        "# Edit the text after = on each line (1-44).\r\n"
        "# Empty lines (N=) become blank separator lines in the credits scroll.\r\n"
        "# Lines starting with # are comments.\r\n"
        "# This file lives next to the mod DLL (e.g. Mods\\)\r\n"
        "\r\n";
    DWORD wrote = 0;
    WriteFile(h, head, (DWORD)nc_strlen(head), &wrote, NULL);
    for (int i = 0; i < NUM_CREDITS_LINES; i++) {
        char line[128];
        if (!g_defaults[i][0])
            nc_snprintf(line, sizeof(line), "%d=\r\n", i + 1);
        else
            nc_snprintf(line, sizeof(line), "%d=%s\r\n", i + 1, g_defaults[i]);
        WriteFile(h, line, (DWORD)nc_strlen(line), &wrote, NULL);
    }
    CloseHandle(h);
    mod_log("[mknp_custom_credits] Generated default config");
}

static void load_config(void) {
    for (int i = 0; i < NUM_CREDITS_LINES; i++)
        nc_strncpy(g_lines[i], g_defaults[i], MAX_CREDITS_LEN);

    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        mod_log("[mknp_custom_credits] Config not found, generating default");
        generate_defaults();
        return;
    }
    static char buf[4096]; /* static: -nostdlib has no __chkstk_ms for 4KB frames */
    DWORD got = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &got, NULL);
    CloseHandle(h);
    buf[got] = '\0';

    int parsed = 0;
    char* line = buf;
    while (line && *line) {
        char* nl = NULL;
        for (char* p = line; *p; p++) if (*p == '\n') { nl = p; break; }
        if (nl) *nl = '\0';
        trim(line);
        if (line[0] && line[0] != '#') {
            char* eq = NULL;
            for (char* p = line; *p; p++) if (*p == '=') { eq = p; break; }
            if (eq) {
                *eq = '\0';
                trim(line);
                char* val = eq + 1;
                trim(val);
                int slot = parse_num(line);
                if (slot >= 1 && slot <= NUM_CREDITS_LINES) {
                    nc_strncpy(g_lines[slot - 1], val, MAX_CREDITS_LEN);
                    parsed++;
                }
            }
        }
        if (nl) line = nl + 1;
        else break;
    }
    char msg[64];
    nc_snprintf(msg, sizeof(msg), "[mknp_custom_credits] Parsed %d lines", parsed);
    mod_log(msg);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch apply (one-shot in Initialize)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_patches(void) {
    DWORD base = (DWORD)GetModuleHandleA(NULL); /* exe base = game base */

    char msg[128];
    nc_snprintf(msg, sizeof(msg), "[mknp_custom_credits] Config path: %s", g_config_path);
    mod_log(msg);

    int skipped = 0;
    for (int i = 0; i < NUM_CREDITS_LINES; i++) {
        const char* str = g_lines[i][0] ? g_lines[i] : g_empty;
        DWORD addr = base + g_push_rvas[i];
        if (*(BYTE*)addr != 0x68) {
            skipped++;
            continue;
        }
        DWORD* imm = (DWORD*)(addr + 1);
        DWORD old = 0;
        if (VirtualProtect(imm, 4, PAGE_READWRITE, &old)) {
            *imm = (DWORD)str;
            VirtualProtect(imm, 4, old, &old);
        }
    }
    if (skipped) {
        nc_snprintf(msg, sizeof(msg),
                    "[mknp_custom_credits] WARNING: %d lines skipped (no 0x68)", skipped);
        mod_log(msg);
    } else {
        mod_log("[mknp_custom_credits] All 44 lines patched");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ callback wiring (17-entry vtable, MinGW __thiscall convention)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "mknp_custom_credits"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    *(void**)((char*)thisptr + 4) = api;
    mod_self();

    /* DLL dir -> txt + log live next to this DLL (e.g. Mods\) */
    g_dll_dir[0] = '\0';
    if (g_module && GetModuleFileNameA(g_module, g_dll_dir, MAX_PATH) > 0) {
        char* s = NULL;
        for (char* p = g_dll_dir; *p; p++) if (*p == '\\') s = p;
        if (s) s[1] = '\0';
        else g_dll_dir[0] = '\0';
    }
    if (g_dll_dir[0]) {
        path_join(g_config_path, g_dll_dir, "mknp_custom_credits.txt");
        path_join(g_log_path, g_dll_dir, "mknp_custom_credits_log.txt");
    } else {
        nc_strncpy(g_config_path, "mknp_custom_credits.txt", MAX_PATH - 1);
        nc_strncpy(g_log_path, "mknp_custom_credits_log.txt", MAX_PATH - 1);
    }

    char init[2 * MAX_PATH + 64];
    nc_snprintf(init, sizeof(init), "[mknp_custom_credits] INIT log=%s set=%s",
                g_log_path, g_config_path);
    mod_log(init);

    load_config();
    apply_patches();
    g_started = 1;
}

static void __thiscall game_update(void*) {
    (void)g_started; /* patches are one-shot at Initialize; nothing per-tick */
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

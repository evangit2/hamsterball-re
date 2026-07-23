/*
 * CustomNameStrips_MinGW.cpp — Per-level RGBA colors for the name strip.
 *
 * HB+ port of custom_name_strips bass.dll proxy mod.
 *
 * Replaces the hardcoded push sequence at 0x41B763 (Scene_DrawNameStrip)
 * with a code cave containing patchable push-imm32 instructions.
 * onGameUpdate reads the level name via HB+ API and patches the cave's
 * float operands with per-level custom values from a config file.
 *
 * Config file: name_strips.txt (next to the DLL in Mods\)
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* Game addresses */
#define APP_PTR_ADDR     0x005341E0
#define BOARD_NAME_OFF   0x29B4   /* board+0x29B4 = char* race/arena name */

/* Patch site: the push sequence in FUN_0041B710 */
#define PATCH_ADDR       0x0041B763
#define PATCH_SIZE       14         /* 5+2+1+1+5 bytes */
#define RETURN_ADDR      0x0041B771 /* instruction after the call */
#define MATRIX_SCALE     0x00453150 /* Matrix_Scale4x4 target */

/* Original bytes at patch site (verified from EXE) */
static const BYTE g_original_bytes[14] = {
    0x68, 0x00, 0x00, 0x40, 0x3F,  /* push 0x3F400000 (A=0.75) */
    0x6A, 0x00,                     /* push 0 (B=0.0) */
    0x55,                           /* push ebp (G=1.0) */
    0x55,                           /* push ebp (R=1.0) */
    0xE8, 0xDF, 0x79, 0x03, 0x00   /* call Matrix_Scale4x4 */
};

/* Default RGBA (original game values) */
#define DEFAULT_R  1.0f
#define DEFAULT_G  1.0f
#define DEFAULT_B  0.0f
#define DEFAULT_A  0.75f

/* Level definitions: 15 races + 15 arenas = 30 entries */
#define NUM_LEVELS 15
#define NUM_ENTRIES 30

/* Level keywords for name matching (priority order: WARM before UP) */
static const char *g_level_keywords[NUM_LEVELS] = {
    "WARM", "BEGINNER", "INTERMED", "DIZZY", "TOWER",
    "UP", "NEON", "EXPERT", "ODD", "TOOB",
    "WOBBLY", "GLASS", "SKY", "MASTER", "IMPOSSIBLE"
};

/* RGBA storage: [0-14] = races, [15-29] = arenas */
typedef struct {
    float r, g, b, a;
    int   valid;
} StripColor;

static StripColor g_colors[NUM_ENTRIES];
static char g_configPath[MAX_PATH] = "";
static bool g_pathReady = false;

/* Code cave state */
static BYTE *g_cave_mem = NULL;
static DWORD g_cave_float_offsets[4];
static int g_cave_installed = 0;

static int g_last_level_idx = -1;
static DWORD g_config_tick = 0;

static void* g_storedApi = NULL;

/* ── String helpers ─────────────────────────────────────────────────── */

static char to_lower(char c) { return (c >= 'A' && c <= 'Z') ? (c + 32) : c; }

static int nc_stricmp2(const char *a, const char *b) {
    while (*a && *b) {
        char ca = to_lower(*a);
        char cb = to_lower(*b);
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static int stristr_ci(const char *haystack, const char *needle) {
    if (!haystack || !*haystack || !needle || !*needle) return 0;
    int nl = (int)nc_strlen(needle);
    int hl = (int)nc_strlen(haystack);
    for (int i = 0; i <= hl - nl; i++) {
        int j;
        for (j = 0; j < nl; j++) {
            if (to_lower(haystack[i + j]) != to_lower(needle[j])) break;
        }
        if (j == nl) return 1;
    }
    return 0;
}

static int nc_strnicmp(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        char ca = to_lower(a[i]);
        char cb = to_lower(b[i]);
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        if (!a[i]) return 0;
    }
    return 0;
}

/* Match level name → returns 0-14 or -1 */
static int match_level(const char *name) {
    if (!name || !*name) return -1;
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (stristr_ci(name, g_level_keywords[i])) return i;
    }
    return -1;
}

static int is_arena_name(const char *name) {
    return stristr_ci(name, "ARENA");
}

/* ── Config path: relative to THIS DLL ──────────────────────────────── */

static void buildConfigPath(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)buildConfigPath, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) {
                if (*p == '\\' || *p == '/') last = p;
                p++;
            }
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_configPath, dllPath, MAX_PATH - 1);
                nc_strncpy(g_configPath + nc_strlen(g_configPath),
                           "name_strips.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "name_strips.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_pathReady = true;
}

/* ── Float parsing ──────────────────────────────────────────────────── */

static int parse_float_val(const char *s, float *out) {
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0' || *s == '\r' || *s == '\n') return 0;
    int negative = 0;
    if (*s == '-') { negative = 1; s++; }
    else if (*s == '+') { s++; }
    int integerPart = 0;
    int hadDigits = 0;
    while (*s >= '0' && *s <= '9') {
        integerPart = integerPart * 10 + (*s - '0');
        s++;
        hadDigits = 1;
    }
    float frac = 0.0f;
    if (*s == '.') {
        s++;
        float div = 10.0f;
        while (*s >= '0' && *s <= '9') {
            frac += (*s - '0') / div;
            div *= 10.0f;
            s++;
            hadDigits = 1;
        }
    }
    if (!hadDigits) return 0;
    float val = (float)integerPart + frac;
    if (negative) val = -val;
    *out = val;
    return 1;
}

static int extract_float(const char *line, char key, float *out) {
    const char *p = line;
    while (*p) {
        if (to_lower(*p) == to_lower(key)) {
            const char *q = p + 1;
            while (*q == ' ' || *q == '\t') q++;
            if (*q == '=') {
                q++;
                return parse_float_val(q, out);
            }
        }
        p++;
    }
    return 0;
}

static int nc_atoi_simple(const char *s) {
    while (*s == ' ' || *s == '\t') s++;
    int val = 0;
    while (*s >= '0' && *s <= '9') { val = val * 10 + (*s - '0'); s++; }
    return val;
}

/* ── Config file parsing ────────────────────────────────────────────── */

static void read_config(void) {
    if (!g_pathReady) return;
    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[16384];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    /* Clear previous entries */
    for (int i = 0; i < NUM_ENTRIES; i++) g_colors[i].valid = 0;

    /* Parse line by line */
    char *p = buf;
    while (*p) {
        char *nl = p;
        while (*nl && *nl != '\n') nl++;
        char saved = *nl;
        *nl = '\0';

        char *line = p;
        while (*line == ' ' || *line == '\t') line++;
        if (*line == ';' || *line == '#' || *line == '\r' || *line == '\0') {
            *nl = saved;
            if (*nl == '\n') nl++;
            p = nl;
            continue;
        }

        int is_arena = 0;
        int level_num = 0;
        const char *prefix = NULL;

        if (stristr_ci(line, "Arena")) {
            is_arena = 1;
            prefix = "Arena";
        } else if (stristr_ci(line, "Level")) {
            is_arena = 0;
            prefix = "Level";
        }

        if (prefix) {
            char *num_start = line;
            while (*num_start) {
                if (to_lower(*num_start) == to_lower(prefix[0])) {
                    int match = 1;
                    for (int i = 0; prefix[i]; i++) {
                        if (to_lower(num_start[i]) != to_lower(prefix[i])) { match = 0; break; }
                    }
                    if (match) { num_start += (int)nc_strlen(prefix); break; }
                }
                num_start++;
            }
            while (*num_start == ' ' || *num_start == '\t') num_start++;
            level_num = nc_atoi_simple(num_start);
            if (level_num >= 1 && level_num <= 15) {
                float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
                int found_r = 0, found_g = 0, found_b = 0, found_a = 0;

                found_r = extract_float(line, 'R', &r);
                found_g = extract_float(line, 'G', &g);
                found_b = extract_float(line, 'B', &b);
                found_a = extract_float(line, 'A', &a);

                if (!(found_r && found_g && found_b && found_a)) {
                    *nl = saved;
                    if (*nl == '\n') {
                        char *next = nl + 1;
                        char *next_nl = next;
                        while (*next_nl && *next_nl != '\n') next_nl++;
                        char saved2 = *next_nl;
                        *next_nl = '\0';

                        if (!found_r) found_r = extract_float(next, 'R', &r);
                        if (!found_g) found_g = extract_float(next, 'G', &g);
                        if (!found_b) found_b = extract_float(next, 'B', &b);
                        if (!found_a) found_a = extract_float(next, 'A', &a);

                        *next_nl = saved2;
                    }
                    *nl = '\0';
                }

                if (found_r || found_g || found_b || found_a) {
                    int idx = (is_arena ? NUM_LEVELS : 0) + (level_num - 1);
                    g_colors[idx].r = r;
                    g_colors[idx].g = g;
                    g_colors[idx].b = b;
                    g_colors[idx].a = a;
                    g_colors[idx].valid = 1;
                }
            }
        }

        *nl = saved;
        if (*nl == '\n') nl++;
        p = nl;
    }
}

/* Create default config file */
static void create_default_config(void) {
    if (!g_pathReady) return;
    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); return; }

    char buf[8192];
    int pos = 0;
    pos += wsprintfA(buf + pos,
        "; Custom Name Strips Config File\r\n"
        "; Per-level RGBA colors for the strip behind the Race/Arena name\r\n"
        "; Values are floats (0.0 to 1.0)\r\n"
        "; Original game values: R=1.0, G=1.0, B=0.0, A=0.75 (yellow)\r\n"
        ";\r\n"
        "; Format:\r\n"
        ";   Level N =\r\n"
        ";   R = <float>, G = <float>, B = <float>, A = <float>\r\n"
        ";\r\n"
        "; Levels 1-15 = Races, Arenas 1-15 = Arenas\r\n"
        "; Re-reads every 2 seconds. Edit at runtime!\r\n"
        "\r\n");

    for (int i = 0; i < NUM_LEVELS; i++) {
        pos += wsprintfA(buf + pos, "Level %d =\r\n", i + 1);
        pos += wsprintfA(buf + pos, "R = 1.0, G = 1.0, B = 1.0, A = 1.0\r\n\r\n");
    }
    for (int i = 0; i < NUM_LEVELS; i++) {
        pos += wsprintfA(buf + pos, "Arena %d =\r\n", i + 1);
        pos += wsprintfA(buf + pos, "R = 1.0, G = 1.0, B = 1.0, A = 1.0\r\n\r\n");
    }

    DWORD written;
    h = CreateFileA(g_configPath, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        WriteFile(h, buf, pos, &written, NULL);
        CloseHandle(h);
    }
}

/* ── Code cave installation ─────────────────────────────────────────── */

static int patch_byte(DWORD addr, BYTE value) {
    DWORD oldProtect;
    if (!VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    *(BYTE*)addr = value;
    VirtualProtect((void*)addr, 1, oldProtect, &oldProtect);
    return 1;
}

static int patch_dword(DWORD addr, DWORD value) {
    DWORD oldProtect;
    if (!VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    *(DWORD*)addr = value;
    VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
    return 1;
}

static void patch_cave_float(int float_idx, float value) {
    if (!g_cave_mem || !g_cave_installed) return;
    DWORD addr = (DWORD)g_cave_mem + g_cave_float_offsets[float_idx];
    DWORD oldProtect;
    if (VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *(float*)addr = value;
        VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
    }
}

static int install_code_cave(void) {
    if (g_cave_installed) return 1;

    BYTE *patch_addr = (BYTE*)PATCH_ADDR;
    /* Verify original bytes */
    if (IsBadReadPtr(patch_addr, PATCH_SIZE)) return 0;
    for (int i = 0; i < PATCH_SIZE; i++) {
        if (patch_addr[i] != g_original_bytes[i]) return 0;
    }

    g_cave_mem = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!g_cave_mem) return 0;

    int off = 0;

    /* push A (imm32) — 5 bytes */
    g_cave_float_offsets[3] = off + 1;
    g_cave_mem[off] = 0x68;
    *(float*)(g_cave_mem + off + 1) = DEFAULT_A;
    off += 5;

    /* push B (imm32) — 5 bytes */
    g_cave_float_offsets[2] = off + 1;
    g_cave_mem[off] = 0x68;
    *(float*)(g_cave_mem + off + 1) = DEFAULT_B;
    off += 5;

    /* push G (imm32) — 5 bytes */
    g_cave_float_offsets[1] = off + 1;
    g_cave_mem[off] = 0x68;
    *(float*)(g_cave_mem + off + 1) = DEFAULT_G;
    off += 5;

    /* push R (imm32) — 5 bytes */
    g_cave_float_offsets[0] = off + 1;
    g_cave_mem[off] = 0x68;
    *(float*)(g_cave_mem + off + 1) = DEFAULT_R;
    off += 5;

    /* call Matrix_Scale4x4 — 5 bytes (relative call) */
    DWORD call_addr = (DWORD)g_cave_mem + off;
    DWORD call_target = MATRIX_SCALE;
    DWORD call_offset = call_target - (call_addr + 5);
    g_cave_mem[off] = 0xE8;
    *(DWORD*)(g_cave_mem + off + 1) = call_offset;
    off += 5;

    /* jmp back to RETURN_ADDR (0x41B771) — 5 bytes */
    DWORD jmp_addr = (DWORD)g_cave_mem + off;
    DWORD jmp_offset = RETURN_ADDR - (jmp_addr + 5);
    g_cave_mem[off] = 0xE9;
    *(DWORD*)(g_cave_mem + off + 1) = jmp_offset;
    off += 5;

    /* Patch the original code site: JMP to cave + NOPs */
    DWORD cave_addr = (DWORD)g_cave_mem;
    DWORD jmp_to_cave_offset = cave_addr - (PATCH_ADDR + 5);

    patch_byte(PATCH_ADDR, 0xE9);
    patch_dword(PATCH_ADDR + 1, jmp_to_cave_offset);

    for (int i = 5; i < PATCH_SIZE; i++) {
        patch_byte(PATCH_ADDR + i, 0x90);
    }

    g_cave_installed = 1;
    return 1;
}

/* ── Get current level index via HB+ API ────────────────────────────── */

static int get_current_level_idx(void) {
    if (!g_storedApi) return -1;
    HBPlusAPI hb = { g_storedApi };
    Scene* scene = hb.GetScene();
    if (!scene || IsBadReadPtr(scene, 0x900)) return -1;
    if (!scene->name) return -1;
    if (IsBadReadPtr(scene->name, 1)) return -1;

    char* name = scene->name;
    int level = match_level(name);
    if (level < 0) return -1;
    int arena = is_arena_name(name);
    return arena ? (level + NUM_LEVELS) : level;
}

/* ── Vtable implementations ─────────────────────────────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Custom Name Strips"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Ported to HB+ by Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    buildConfigPath();
    create_default_config();
    read_config();
    install_code_cave();
}

static void __thiscall game_update_impl(void* thisptr) {
    if (!g_cave_installed) return;

    /* Re-read config every 2 seconds */
    DWORD tick = GetTickCount();
    if (tick - g_config_tick > 2000) {
        read_config();
        g_config_tick = tick;
    }

    int idx = get_current_level_idx();
    if (idx != g_last_level_idx) {
        g_last_level_idx = idx;
    }

    if (idx >= 0) {
        StripColor *c;
        if (g_colors[idx].valid) {
            c = &g_colors[idx];
        } else {
            static StripColor def = {DEFAULT_R, DEFAULT_G, DEFAULT_B, DEFAULT_A, 1};
            c = &def;
        }
        patch_cave_float(0, c->r);
        patch_cave_float(1, c->g);
        patch_cave_float(2, c->b);
        patch_cave_float(3, c->a);
    } else {
        /* Use defaults when level unknown */
        patch_cave_float(0, DEFAULT_R);
        patch_cave_float(1, DEFAULT_G);
        patch_cave_float(2, DEFAULT_B);
        patch_cave_float(3, DEFAULT_A);
    }
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {
    g_last_level_idx = -1;
}
static void __thiscall scene_end_impl(void*) {
    g_last_level_idx = -1;
}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

/*
 * CustomRaceDescriptions_MinGW.cpp — MinGW HB+ port of custom_race_descriptions bass.dll proxy mod.
 *
 * Allows customizing the 15 tournament race description texts shown on the
 * Tourney Menu screen. The original game stores these as a hardcoded pointer
 * table at 0x4F7148 (15 char* entries, one per race). This mod overwrites
 * those pointers at runtime to point to strings loaded from a .txt file.
 *
 * Config file: mkn_plus_custom_race_descriptions.txt (next to the DLL in Mods\)
 *
 * The file format is:
 *
 *   [Level 1]
 *   Take your time on the Warm-Up race! This easy little...
 *   (multi-line descriptions are supported — everything between
 *    [Level N] and [Level N+1] is the description text)
 *
 *   [Level 2]
 *   Now let's try something a little more interesting...
 *
 *   ...through [Level 15]
 *
 * Lines starting with # or ; are comments. If a level is missing from
 * the file, the original game description is kept. If the file is missing
 * entirely, a default template is auto-generated.
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* Tournament description pointer table:
 *   Address 0x4F7148 in Hamsterball.exe (.data section, writable)
 *   15 entries, each a char* (4 bytes) pointing to the description string
 */
#define DESC_TABLE_ADDR  0x004F7148
#define NUM_RACES         15

/* Original description pointers (from the binary, verified via Ghidra) */
static const DWORD g_original_desc_ptrs[NUM_RACES] = {
    0x004D8238,  /*  0: Warm-Up    */
    0x004D8128,  /*  1: Beginner   */
    0x004D7FD8,  /*  2: Intermediate */
    0x004D7E78,  /*  3: Dizzy      */
    0x004D7D20,  /*  4: Tower      */
    0x004D7B88,  /*  5: Up         */
    0x004D7A60,  /*  6: Neon       */
    0x004D78D8,  /*  7: Expert     */
    0x004D7718,  /*  8: Odd        */
    0x004D7570,  /*  9: Toob       */
    0x004D7408,  /* 10: Wobbly     */
    0x004D7290,  /* 11: Glass      */
    0x004D7108,  /* 12: Sky        */
    0x004D6FB8,  /* 13: Master     */
    0x004D6E60   /* 14: Impossible */
};

/* Race names for the default config file */
static const char *g_race_names[NUM_RACES] = {
    "Warm-Up Race",
    "Beginner Race",
    "Intermediate Race",
    "Dizzy Race",
    "Tower Race",
    "Up Race",
    "Neon Race",
    "Expert Race",
    "Odd Race",
    "Toob Race",
    "Wobbly Race",
    "Glass Race",
    "Sky Race",
    "Master Race",
    "Impossible Race",
};

/* Storage for custom description strings (heap-allocated, kept alive) */
static char *g_custom_descs[NUM_RACES] = {NULL};
static int  g_desc_loaded[NUM_RACES] = {0};
static bool g_patched = false;

static char g_configPath[MAX_PATH] = "";
static bool g_pathReady = false;
static void* g_storedApi = NULL;

/* ── Config path via VirtualQuery ─────────────────────────────────── */

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
                           "mkn_plus_custom_race_descriptions.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "mkn_plus_custom_race_descriptions.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_pathReady = true;
}

/* ── File I/O helpers using Win32 only ─────────────────────────────── */

static char* readFileToBuffer(const char *path) {
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NULL;

    DWORD fileSize = GetFileSize(h, NULL);
    if (fileSize == 0 || fileSize > 1024 * 1024) {
        CloseHandle(h);
        return NULL;
    }

    char *buf = (char*)nc_malloc(fileSize + 1);
    if (!buf) {
        CloseHandle(h);
        return NULL;
    }

    DWORD bytesRead = 0;
    ReadFile(h, buf, fileSize, &bytesRead, NULL);
    CloseHandle(h);
    buf[bytesRead] = '\0';
    return buf;
}

static void writeString(HANDLE h, const char* s) {
    DWORD written;
    WriteFile(h, s, (DWORD)nc_strlen(s), &written, NULL);
}

/* ── Generate default config file ──────────────────────────────────── */

static void generateDefaultConfig(void) {
    if (!g_pathReady) return;
    HANDLE h = CreateFileA(g_configPath, GENERIC_WRITE, 0,
                           NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    writeString(h, "# Race Descriptions Configuration File\r\n");
    writeString(h, "# Edit the text below to customize the descriptions shown on the\r\n");
    writeString(h, "# Tournament Menu screen for each race.\r\n");
    writeString(h, "#\r\n");
    writeString(h, "# Format:\r\n");
    writeString(h, "#   [Level N]\r\n");
    writeString(h, "#   Your description text here.\r\n");
    writeString(h, "#   Multi-line descriptions are joined with spaces.\r\n");
    writeString(h, "#\r\n");
    writeString(h, "# Lines starting with # or ; are comments.\r\n");
    writeString(h, "# Missing levels will use the original game description.\r\n\r\n");

    for (int i = 0; i < NUM_RACES; i++) {
        /* Read original description string from the game's memory */
        const char *desc = (const char *)g_original_desc_ptrs[i];
        char header[128];
        nc_snprintf(header, sizeof(header), "[Level %d]  # %s\r\n", i + 1, g_race_names[i]);
        writeString(h, header);
        writeString(h, desc);
        writeString(h, "\r\n\r\n");
    }

    CloseHandle(h);
}

/* ── Simple case-insensitive strncmp ───────────────────────────────── */

static int nc_strnicmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; i++) {
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        if (ca == '\0') return 0;
    }
    return 0;
}

static int nc_atoi(const char* s) {
    int result = 0;
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result;
}

/* ── Config file parser ────────────────────────────────────────────── */

static void parseConfigFile(const char *buf) {
    const char *p = buf;
    int current_level = -1;  /* 0-14 = active level, -1 = none */

    /* We build each description in a temp buffer */
    char *temp_descs[NUM_RACES];
    int   temp_lens[NUM_RACES];
    int   temp_caps[NUM_RACES];

    /* Allocate initial buffers for each level */
    for (int i = 0; i < NUM_RACES; i++) {
        temp_caps[i] = 512;
        temp_descs[i] = (char*)nc_malloc(temp_caps[i]);
        if (temp_descs[i]) {
            temp_descs[i][0] = '\0';
            temp_lens[i] = 0;
        } else {
            temp_lens[i] = 0;
            temp_caps[i] = 0;
        }
    }

    /* Process line by line */
    while (*p) {
        /* Skip leading whitespace */
        while (*p == ' ' || *p == '\t') p++;
        const char *content = p;

        /* Find end of line */
        while (*p && *p != '\r' && *p != '\n') p++;
        int line_len = (int)(p - content);

        /* Skip line ending */
        if (*p == '\r') p++;
        if (*p == '\n') p++;

        /* Skip empty lines and comments */
        if (line_len == 0) continue;
        if (content[0] == '#' || content[0] == ';') continue;

        /* Check for [Level N] header */
        if (content[0] == '[') {
            /* Find closing bracket */
            const char *close = content;
            while (close < content + line_len && *close != ']') close++;
            if (close < content + line_len) {
                /* Extract "Level N" between brackets */
                int hdr_len = (int)(close - content - 1);
                const char *hdr = content + 1;
                /* Find "Level " prefix */
                if (hdr_len >= 6 && nc_strnicmp(hdr, "Level ", 6) == 0) {
                    int level_num = nc_atoi(hdr + 6);
                    if (level_num >= 1 && level_num <= NUM_RACES) {
                        current_level = level_num - 1;  /* 0-indexed */
                        if (temp_descs[current_level]) {
                            temp_descs[current_level][0] = '\0';
                            temp_lens[current_level] = 0;
                        }
                    } else {
                        current_level = -1;
                    }
                } else {
                    current_level = -1;
                }
            }
            continue;
        }

        /* Accumulate description text for current_level */
        if (current_level >= 0 && current_level < NUM_RACES && temp_descs[current_level]) {
            int need_space = (temp_lens[current_level] > 0) ? 1 : 0;
            int total = temp_lens[current_level] + need_space + line_len + 1;

            /* Grow buffer if needed */
            if (total > temp_caps[current_level]) {
                while (temp_caps[current_level] < total)
                    temp_caps[current_level] *= 2;
                char *newbuf = (char*)nc_malloc(temp_caps[current_level]);
                if (newbuf) {
                    nc_memcpy(newbuf, temp_descs[current_level], temp_lens[current_level] + 1);
                    nc_free(temp_descs[current_level]);
                    temp_descs[current_level] = newbuf;
                } else {
                    continue;
                }
            }

            /* Append space if needed */
            if (need_space) {
                temp_descs[current_level][temp_lens[current_level]++] = ' ';
            }

            /* Append the line content (trimmed of trailing whitespace) */
            int actual_len = line_len;
            while (actual_len > 0 &&
                   (content[actual_len - 1] == ' ' ||
                    content[actual_len - 1] == '\t')) {
                actual_len--;
            }
            if (actual_len > 0) {
                nc_memcpy(temp_descs[current_level] + temp_lens[current_level],
                       content, actual_len);
                temp_lens[current_level] += actual_len;
            }
            temp_descs[current_level][temp_lens[current_level]] = '\0';
        }
    }

    /* Transfer successfully parsed descriptions */
    for (int i = 0; i < NUM_RACES; i++) {
        if (temp_descs[i] && temp_lens[i] > 0) {
            g_custom_descs[i] = temp_descs[i];
            g_desc_loaded[i] = 1;
        } else {
            if (temp_descs[i]) nc_free(temp_descs[i]);
            g_desc_loaded[i] = 0;
        }
    }
}

/* ── Apply descriptions to game's pointer table ────────────────────── */

static void applyDescriptions(void) {
    DWORD *desc_table = (DWORD *)DESC_TABLE_ADDR;
    DWORD old_protect = 0;

    if (!VirtualProtect(desc_table, NUM_RACES * 4,
                        PAGE_READWRITE, &old_protect)) {
        return;
    }

    for (int i = 0; i < NUM_RACES; i++) {
        if (g_desc_loaded[i] && g_custom_descs[i]) {
            desc_table[i] = (DWORD)g_custom_descs[i];
        }
        /* Otherwise: leave the original pointer untouched */
    }

    VirtualProtect(desc_table, NUM_RACES * 4, old_protect, &old_protect);
}

/* ── Apply patches ─────────────────────────────────────────────────── */

static void applyCustomRaceDescriptions(void) {
    if (g_patched) return;
    g_patched = true;

    buildConfigPath();

    /* Try to read the config file */
    char *buf = readFileToBuffer(g_configPath);
    if (buf) {
        parseConfigFile(buf);
        nc_free(buf);
    } else {
        /* File doesn't exist — generate a default template */
        generateDefaultConfig();
    }

    /* Apply the descriptions to the game's pointer table */
    applyDescriptions();
}

/* ── HB+ vtable ───────────────────────────────────────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Custom Race Descriptions"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Ported from bass.dll proxy"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    applyCustomRaceDescriptions();
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall game_update_impl(void*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

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

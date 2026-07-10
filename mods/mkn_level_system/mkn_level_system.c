/*
 * mkn_level_system v5 — Extended Vtables + Custom Dispatch
 *
 * v4 FIX (retained):
 *   Patches ALL 15 level allocations to MAX_STRUCT_SIZE (0x6498) so any
 *   vtable function from any level can safely access any board offset.
 *   Custom vtable allocation: each level gets its own writable vtable copy.
 *
 * v5 NEW:
 *   Extended vtables from 36 to 128 entries (512 bytes each).
 *   Slots 0-35: game-native dispatch (called by hardcoded game code).
 *   Slots 36-127: custom dispatch (called by background thread ~60fps).
 *
 *   The background thread reads the current board's vtable pointer, finds
 *   which level's custom vtable it matches, then calls every non-zero entry
 *   in slots 36-127 with __thiscall(board) convention.
 *
 *   This lets you add custom per-frame functions to any level via:
 *     VTABLE 1 36 0x0041B130    ; custom per-frame function on Level 1
 *     VTABLE 1 37 MyCustomFunc   ; etc.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mkn_level_system.c \
 *     ../shared/bass_proxy.h -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../shared/bass_proxy.h"

/* ============================================================
 * Level table — board ctors, struct sizes, and patch points
 * ============================================================ */

#define MAX_STRUCT_SIZE 0x6498  /* Master level — largest board struct */

typedef struct {
    int   level_num;
    const char* name;
    DWORD ctor_addr;      /* board constructor function address     */
    DWORD struct_size;    /* original size passed to operator_new   */
    DWORD push_addr;      /* address of PUSH imm32 (struct size)   */
    DWORD call_addr;      /* address of CALL rel32 (board ctor)     */
    DWORD vtable_addr;    /* original vtable start in .data (36 entries) */
    DWORD ctor_vt_patch;  /* address of vtable imm32 in ctor (C7 06 + 2) */
} LevelEntry;

static const LevelEntry g_levels[15] = {
    { 1,  "WarmUp",       0x0041CA40, 0x436C, 0x00427109, 0x0042712C, 0x004D04A8, 0x0041CA73 },
    { 2,  "Beginner",     0x004200E0, 0x644C, 0x00427136, 0x0042715D, 0x004D1098, 0x0042012B },
    { 3,  "Intermediate", 0x0041CB20, 0x438C, 0x00427167, 0x0042718E, 0x004D05A0, 0x0041CB57 },
    { 4,  "Dizzy",        0x0041D060, 0x4BE0, 0x00427198, 0x004271BF, 0x004D0890, 0x0041D09B },
    { 5,  "Tower",        0x0041E340, 0x5418, 0x004271C9, 0x004271F0, 0x004D0A08, 0x0041E379 },
    { 6,  "Up",           0x00420390, 0x4790, 0x004271FA, 0x00427221, 0x004D11A0, 0x004203CC },
    { 7,  "Neon",         0x00424440, 0x4394, 0x0042722B, 0x00427252, 0x004D1DF0, 0x00424481 },
    { 8,  "Expert",       0x0041EA40, 0x4FD8, 0x0042725C, 0x00427283, 0x004D0B00, 0x0041EA7B },
    { 9,  "Odd",          0x0041ED80, 0x43B0, 0x0042728D, 0x004272B4, 0x004D0BC0, 0x0041EDB1 },
    { 10, "Toob",         0x0041F4B0, 0x646C, 0x004272BE, 0x004272E5, 0x004D0E78, 0x0041F4FB },
    { 11, "Wobbly",       0x0041F110, 0x4388, 0x004272EF, 0x00427316, 0x004D0D38, 0x0041F144 },
    { 12, "Glass",        0x00424A90, 0x4390, 0x00427320, 0x00427347, 0x004D1F90, 0x00424AD3 },
    { 13, "Sky",          0x0041F930, 0x47F8, 0x00427351, 0x00427374, 0x004D0FC8, 0x0041F96A },
    { 14, "Master",       0x004206D0, 0x6498, 0x0042737B, 0x0042739E, 0x004D12B0, 0x0042071C },
    { 15, "Impossible",   0x00424C20, 0x4380, 0x004273A5, 0x004273C8, 0x004D21C0, 0x00424C5D },
};

#define VTABLE_SIZE         512    /* 128 entries × 4 bytes             */
#define VTABLE_ENTRIES      128
#define VTABLE_NATIVE_SLOTS  36     /* slots 0-35: game-native dispatch   */
#define VTABLE_EXT_SLOTS    92     /* slots 36-127: custom dispatch      */

/* ============================================================
 * Custom vtable storage — 15 independent writable vtable copies
 * ============================================================ */

static DWORD g_custom_vtables[15] = {0};
static int   g_vtables_initialized = 0;
static volatile int g_dispatch_running = 0;

/* Original vtable entries (only 36 native slots saved for RESET) */
static DWORD g_orig_vtable[15][36] = {{0}};
static int   g_vtable_saved = 0;

static void init_custom_vtables(void) {
    if (g_vtables_initialized) return;

    for (int i = 0; i < 15; i++) {
        /* Allocate 512 bytes (128 entries) but only copy 144 bytes (36 native entries)
         * from the original. Slots 36-127 are zeroed (available for custom use). */
        DWORD addr = (DWORD)VirtualAlloc(NULL, VTABLE_SIZE,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!addr) continue;

        if (!IsBadReadPtr((void*)g_levels[i].vtable_addr, 144)) {
            memcpy((void*)addr, (void*)g_levels[i].vtable_addr, 144);
        }
        /* Slots 36-127 are already zeroed by VirtualAlloc */

        g_custom_vtables[i] = addr;

        /* Patch the board ctor to write our custom vtable address */
        patch_dword(g_levels[i].ctor_vt_patch, addr);
    }

    g_vtables_initialized = 1;
}

/* ============================================================
 * Patch ALL 15 level allocations to use MAX_STRUCT_SIZE.
 * This ensures any vtable function from any level can safely
 * access any board offset without heap corruption.
 * ============================================================ */

static void patch_all_struct_sizes(void) {
    for (int i = 0; i < 15; i++) {
        /* Only patch if the level's original size is smaller than max */
        if (g_levels[i].struct_size < MAX_STRUCT_SIZE) {
            patch_dword(g_levels[i].push_addr + 1, MAX_STRUCT_SIZE);
        }
    }
}

/* ============================================================
 * Vtable slot names
 * ============================================================ */

static const struct { int slot; const char* name; const char* desc; } g_vtableSlots[] = {
    { 0,  "dtor",           "Scalar deleting destructor (per-level)" },
    { 1,  "update",         "Board_Update (per-level variant)" },
    { 18, "setup",           "Setup function — loads .MESHWORLD file" },
    { 19, "initscene",       "InitScene — post-setup initialization" },
    { 24, "renderdynamic",   "RenderDynamic — renders dynamic objects" },
    { 29, "dispatchcollision","DispatchCollision — collision event handler" },
    { 32, "boardsetup",      "Board_Setup — creates level objects" },
    { 33, "levelspecific",   "Level-specific function (varies per level)" },
    { -1, NULL, NULL }
};

/* ============================================================
 * Function name → address table
 * ============================================================ */

typedef struct {
    const char* name;
    DWORD addr;
} FuncEntry;

static const FuncEntry g_functions[] = {
    /* Setup functions (vtable[18]) */
    { "GetLevelPath",              0x0040D1C0 },
    { "Scene_SetupLevelCascade",   0x004110D0 },
    { "Scene_LoadLevel2",          0x0040D280 },
    { "Scene_LoadLevel3",          0x0040D390 },
    { "Scene_LoadLevel4",          0x0040D6D0 },
    { "Scene_SetupLevel5",         0x0040E190 },
    { "Scene_SetupLevel6",          0x0040EA90 },
    { "Scene_SetupLevel7",          0x0040F360 },
    { "Scene_SetupLevel9",          0x00410830 },
    { "Scene_SetupLevelUp",         0x00411540 },
    { "Scene_SetupLevel10",         0x00411F60 },
    { "Scene_SetupLevelDark",       0x00416270 },
    { "CreateBumper",              0x0040FA20 },
    { "FUN_00417640",               0x00417640 },
    { "FUN_00417F20",               0x00417F20 },
    /* Destructors (vtable[0]) */
    { "WarmUp_dtor",               0x00425040 },
    { "Beginner_dtor",             0x00425160 },
    { "Intermediate_dtor",         0x00425060 },
    { "Dizzy_dtor",                0x00425080 },
    { "Tower_dtor",                0x004250A0 },
    { "Up_dtor",                   0x00425180 },
    { "Neon_dtor",                 0x00425420 },
    { "Expert_dtor",               0x004250C0 },
    { "Odd_dtor",                  0x004250E0 },
    { "Toob_dtor",                 0x00425120 },
    { "Wobbly_dtor",               0x00425100 },
    { "Glass_dtor",                0x00425460 },
    { "Sky_dtor",                  0x00425140 },
    { "Master_dtor",               0x004251A0 },
    { "Impossible_dtor",           0x004254A0 },
    /* InitScene (vtable[19]) */
    { "WarmUp_InitScene",          0x0041B130 },
    { "Beginner_InitScene",        0x00420240 },
    { "Up_InitScene",              0x00420660 },
    { "Neon_InitScene",            0x00424790 },
    { "Odd_InitScene",             0x0041B540 },
    { "Toob_InitScene",            0x0041F7E0 },
    /* RenderDynamic (vtable[24]) */
    { "WarmUp_RenderDyn",          0x0040B420 },
    { "Beginner_RenderDyn",        0x00411380 },
    { "Tower_RenderDyn",           0x0040DFA0 },
    { "Odd_RenderDyn",             0x0040F350 },
    { "Toob_RenderDyn",            0x00410670 },
    { "Wobbly_RenderDyn",          0x0040B600 },
    { "Sky_RenderDyn",             0x004110A0 },
    /* DispatchCollision (vtable[29]) */
    { "WarmUp_DispatchColl",       0x0040C5D0 },
    { "Beginner_DispatchColl",     0x004111E0 },
    { "Intermediate_DispatchColl", 0x0040D340 },
    { "Dizzy_DispatchColl",        0x0040D500 },
    { "Tower_DispatchColl",        0x0040DCD0 },
    { "Up_DispatchColl",           0x004119B0 },
    { "Neon_DispatchColl",         0x00416CA0 },
    { "Expert_DispatchColl",       0x0040E6A0 },
    { "Odd_DispatchColl",          0x0044B840 },
    { "Toob_DispatchColl",         0x00410020 },
    /* Board_Setup (vtable[32]) */
    { "WarmUp_BoardSetup",         0x0041C5B0 },
    { "Beginner_BoardSetup",       0x0041C5B0 },
    { "Dizzy_BoardSetup",          0x0041C5B0 },
    /* Other */
    { "Level_InitScene",           0x0040B090 },
    { "Graphics_SetProjection",    0x00454AB0 },
    { NULL, 0 }
};

/* Forward declarations for custom function system */
static int   lookup_custom_func(const char* name);
static DWORD get_custom_func_addr(const char* name);
static int   is_custom_func_addr(DWORD addr);
static void  execute_custom_func(int func_idx, DWORD board);
static void  load_custom_functions(void);

static DWORD lookup_function_addr(const char* name) {
    for (int i = 0; g_functions[i].name; i++) {
        if (_stricmp(g_functions[i].name, name) == 0)
            return g_functions[i].addr;
    }
    /* Check custom script functions */
    DWORD custom_addr = get_custom_func_addr(name);
    if (custom_addr) return custom_addr;
    return 0;
}

static int lookup_vtable_slot(const char* name) {
    for (int i = 0; g_vtableSlots[i].name; i++) {
        if (_stricmp(g_vtableSlots[i].name, name) == 0)
            return g_vtableSlots[i].slot;
    }
    return -1;
}

/* ============================================================
 * Original value storage (for RESET)
 * ============================================================ */

typedef struct {
    int   used;
    DWORD push_orig;
    DWORD call_orig;
} OrigEntry;

static OrigEntry g_orig[15] = {0};

static void save_originals(void) {
    if (g_vtable_saved) return;
    for (int i = 0; i < 15; i++) {
        DWORD vt = g_levels[i].vtable_addr;
        for (int s = 0; s < 36; s++) {
            if (!IsBadReadPtr((void*)(vt + s*4), 4))
                g_orig_vtable[i][s] = *(DWORD*)(vt + s*4);
        }
    }
    g_vtable_saved = 1;
}

/* ============================================================
 * Custom vtable read/write helpers
 * ============================================================ */

static DWORD read_custom_vtable_entry(int level_num, int slot) {
    if (level_num < 1 || level_num > 15) return 0;
    if (slot < 0 || slot >= VTABLE_ENTRIES) return 0;
    DWORD vt = g_custom_vtables[level_num - 1];
    if (!vt) return 0;
    return *(DWORD*)(vt + slot * 4);
}

static void write_custom_vtable_entry(int level_num, int slot, DWORD value) {
    if (level_num < 1 || level_num > 15) return;
    if (slot < 0 || slot >= VTABLE_ENTRIES) return;
    DWORD vt = g_custom_vtables[level_num - 1];
    if (!vt) return;
    *(DWORD*)(vt + slot * 4) = value;
}

static void swap_custom_vtable_entry(int a, int b, int slot) {
    if (a < 1 || a > 15 || b < 1 || b > 15 || a == b) return;
    DWORD va = read_custom_vtable_entry(a, slot);
    DWORD vb = read_custom_vtable_entry(b, slot);
    write_custom_vtable_entry(a, slot, vb);
    write_custom_vtable_entry(b, slot, va);
}

static void copy_custom_vtable(int a, int b) {
    if (a < 1 || a > 15 || b < 1 || b > 15 || a == b) return;
    DWORD vt_src = g_custom_vtables[b - 1];
    DWORD vt_dst = g_custom_vtables[a - 1];
    if (!vt_src || !vt_dst) return;
    memcpy((void*)vt_dst, (void*)vt_src, VTABLE_SIZE);
}

/* ============================================================
 * Diagnostic logging
 * ============================================================ */

static char g_logPath[MAX_PATH] = "";

static void diag_log(const char *msg) {
    if (g_logPath[0] == '\0') return;
    HANDLE hFile = CreateFileA(g_logPath,
        FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        SetFilePointer(hFile, 0, NULL, FILE_END);
        WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
        WriteFile(hFile, "\r\n", 2, &written, NULL);
        CloseHandle(hFile);
    }
}

static void diag_logf(const char *fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    diag_log(buf);
}

/* ============================================================
 * Patch helpers for Tournament_AdvanceRace switch cases
 * ============================================================ */

static void patch_push(int level_num, DWORD new_size) {
    if (level_num < 1 || level_num > 15) return;
    patch_dword(g_levels[level_num - 1].push_addr + 1, new_size);
}

static void patch_call(int level_num, DWORD new_ctor) {
    if (level_num < 1 || level_num > 15) return;
    DWORD call_addr = g_levels[level_num - 1].call_addr;
    DWORD offset = new_ctor - (call_addr + 5);
    patch_dword(call_addr + 1, offset);
}

static void save_push_call(int level_num) {
    if (level_num < 1 || level_num > 15) return;
    int idx = level_num - 1;
    if (g_orig[idx].used) return;
    DWORD pa = g_levels[idx].push_addr + 1;
    DWORD ca = g_levels[idx].call_addr + 1;
    if (!IsBadReadPtr((void*)pa, 4))
        g_orig[idx].push_orig = *(DWORD*)pa;
    if (!IsBadReadPtr((void*)ca, 4))
        g_orig[idx].call_orig = *(DWORD*)ca;
    g_orig[idx].used = 1;
}

static void restore_push_call(int level_num) {
    if (level_num < 1 || level_num > 15) return;
    int idx = level_num - 1;
    if (!g_orig[idx].used) return;
    patch_push(level_num, g_orig[idx].push_orig);
    patch_call(level_num,
        g_levels[idx].call_addr + 5 + g_orig[idx].call_orig);
    g_orig[idx].used = 0;
}

/* ============================================================
 * Config file parser
 * ============================================================ */

static char g_configPath[MAX_PATH] = "";

static void find_config_path(void) {
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&find_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_configPath, MAX_PATH);

    strcpy(g_logPath, g_configPath);
    char* p = strrchr(g_logPath, '\\');
    if (p) {
        strcpy(p + 1, "mkn_level_system.log");
    } else {
        strcpy(g_logPath, "mkn_level_system.log");
    }

    p = strrchr(g_configPath, '\\');
    if (p) {
        strcpy(p + 1, "mkn_level_system.txt");
    } else {
        strcpy(g_configPath, "mkn_level_system.txt");
    }
}

static void trim(char* s) {
    char* start = s;
    while (*start == ' ' || *start == '\t') start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' ||
          s[len-1] == '\r' || s[len-1] == '\n')) {
        s[--len] = '\0';
    }
}

static int lookup_level(const char* token) {
    int num = atoi(token);
    if (num >= 1 && num <= 15) return num;
    for (int i = 0; i < 15; i++) {
        if (_stricmp(g_levels[i].name, token) == 0)
            return g_levels[i].level_num;
    }
    return 0;
}

static void apply_config(void) {
    HANDLE hFile = CreateFileA(g_configPath,
        GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        diag_log("[mkn_level_system v4] Config file not found, no changes applied");
        return;
    }

    char buf[32768];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    diag_log("========================================");
    diag_log("[mkn_level_system v4] Config loaded, parsing...");
    diag_logf("Config path: %s", g_configPath);

    save_originals();
    for (int i = 0; i < 15; i++)
        save_push_call(i + 1);

    char* line = strtok(buf, "\n");
    int in_config = 0;
    int changes = 0;

    while (line) {
        trim(line);
        if (strstr(line, "CONFIG SECTION") && line[0] == '#') {
            in_config = 1;
            line = strtok(NULL, "\n");
            continue;
        }
        if (strstr(line, "END CONFIG")) break;
        if (line[0] == '\0' || line[0] == '#') {
            line = strtok(NULL, "\n");
            continue;
        }
        if (!in_config) {
            line = strtok(NULL, "\n");
            continue;
        }

        /* SWAP <A> <B> */
        if (_strnicmp(line, "SWAP ", 5) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 5, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    patch_push(a, g_levels[b - 1].struct_size);
                    patch_push(b, g_levels[a - 1].struct_size);
                    patch_call(a, g_levels[b - 1].ctor_addr);
                    patch_call(b, g_levels[a - 1].ctor_addr);
                    changes += 2;
                    diag_logf("SWAP L%d (%s) <-> L%d (%s)", a, g_levels[a-1].name, b, g_levels[b-1].name);
                }
            }
        }
        /* SWAPOBJ <A> <B> */
        else if (_strnicmp(line, "SWAPOBJ ", 8) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 8, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    swap_custom_vtable_entry(a, b, 32);
                    changes++;
                    diag_logf("SWAPOBJ L%d <-> L%d [vtable[32]]", a, b);
                }
            }
        }
        /* SWAPGEO <A> <B> */
        else if (_strnicmp(line, "SWAPGEO ", 8) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 8, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    swap_custom_vtable_entry(a, b, 18);
                    changes++;
                    diag_logf("SWAPGEO L%d <-> L%d [vtable[18]]", a, b);
                }
            }
        }
        /* SET <A> <B> */
        else if (_strnicmp(line, "SET ", 4) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 4, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    patch_push(a, g_levels[b - 1].struct_size);
                    patch_call(a, g_levels[b - 1].ctor_addr);
                    changes++;
                    diag_logf("SET L%d (%s) -> L%d (%s)", a, g_levels[a-1].name, b, g_levels[b-1].name);
                }
            }
        }
        /* COPYVT <A> <B> */
        else if (_strnicmp(line, "COPYVT ", 7) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 7, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    copy_custom_vtable(a, b);
                    changes++;
                    diag_logf("COPYVT L%d <- L%d [all 36 entries]", a, b);
                }
            }
        }
        /* SETUP <level> <func_name> */
        else if (_strnicmp(line, "SETUP ", 6) == 0) {
            char tokA[64] = "", fname[128] = "";
            if (sscanf(line + 6, "%63s %127s", tokA, fname) == 2) {
                int a = lookup_level(tokA);
                DWORD addr = lookup_function_addr(fname);
                if (a && addr) {
                    write_custom_vtable_entry(a, 18, addr);
                    changes++;
                    diag_logf("SETUP L%d -> %s (0x%08X)", a, fname, addr);
                }
            }
        }
        /* ADD <level> <func_name> */
        else if (_strnicmp(line, "ADD ", 4) == 0) {
            char tokA[64] = "", fname[128] = "";
            if (sscanf(line + 4, "%63s %127s", tokA, fname) == 2) {
                int a = lookup_level(tokA);
                DWORD extra_addr = lookup_function_addr(fname);
                if (a && extra_addr) {
                    DWORD current = read_custom_vtable_entry(a, 18);
                    if (current) {
                        int total_size = 9 * 2 + 1;
                        BYTE* mem = (BYTE*)VirtualAlloc(NULL, total_size,
                            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                        if (mem) {
                            int pos = 0;
                            mem[pos++] = 0x51; mem[pos++] = 0xB8;
                            *(DWORD*)(mem + pos) = current; pos += 4;
                            mem[pos++] = 0xFF; mem[pos++] = 0xD0; mem[pos++] = 0x59;
                            mem[pos++] = 0x51; mem[pos++] = 0xB8;
                            *(DWORD*)(mem + pos) = extra_addr; pos += 4;
                            mem[pos++] = 0xFF; mem[pos++] = 0xD0; mem[pos++] = 0x59;
                            mem[pos++] = 0xC3;
                            write_custom_vtable_entry(a, 18, (DWORD)mem);
                            changes++;
                            diag_logf("ADD L%d += %s [trampoline at 0x%08X]", a, fname, (DWORD)mem);
                        }
                    }
                }
            }
        }
        /* VTABLE <level> <slot> <func_name|0xHEXADDR|0> */
        else if (_strnicmp(line, "VTABLE ", 7) == 0) {
            char tokLevel[64] = "", tokSlot[64] = "", tokFunc[128] = "";
            if (sscanf(line + 7, "%63s %63s %127s", tokLevel, tokSlot, tokFunc) == 3) {
                int a = lookup_level(tokLevel);
                int slot = atoi(tokSlot);
                if (slot == 0 && tokSlot[0] != '0') {
                    slot = lookup_vtable_slot(tokSlot);
                }
                DWORD addr = 0;
                if (_stricmp(tokFunc, "0") == 0 || _stricmp(tokFunc, "null") == 0) {
                    addr = 0;
                } else {
                    addr = lookup_function_addr(tokFunc);
                    if (!addr && tokFunc[0] == '0' && (tokFunc[1] == 'x' || tokFunc[1] == 'X')) {
                        addr = (DWORD)strtoul(tokFunc + 2, NULL, 16);
                    }
                }
                if (a && slot >= 0 && slot < VTABLE_ENTRIES) {
                    write_custom_vtable_entry(a, slot, addr);
                    changes++;
                    diag_logf("VTABLE L%d [%d] = 0x%08X", a, slot, addr);
                } else {
                    if (!a) diag_logf("WARNING: VTABLE invalid level '%s'", tokLevel);
                    if (slot < 0 || slot >= VTABLE_ENTRIES)
                        diag_logf("WARNING: VTABLE invalid slot '%s'", tokSlot);
                }
            }
        }
        /* PATCH <func_name|0xHEXADDR> <offset> <hex_bytes>
         * Write raw bytes at an offset inside a function.
         * <func_name> can be any name from the function table (e.g. Scene_SetupLevelCascade)
         * <offset> is a decimal or 0x-prefixed hex offset from the function start
         * <hex_bytes> is a space-separated list of hex bytes (e.g. 90 90 90 90 90)
         * Examples:
         *   PATCH Scene_SetupLevelCascade 0x10 90 90 90 90 90
         *   PATCH Beginner_InitScene 5 90 90 90
         *   PATCH 0x004110D0 0x20 E9 00 00 00 00
         *   PATCH WarmUp_InitScene 0x1C C7 46 40 01 00 00 00
         */
        else if (_strnicmp(line, "PATCH ", 6) == 0) {
            char tokFunc[128] = "", tokOffset[64] = "";
            if (sscanf(line + 6, "%127s %63s", tokFunc, tokOffset) == 2) {
                /* Resolve function address */
                DWORD func_addr = lookup_function_addr(tokFunc);
                if (!func_addr && tokFunc[0] == '0' && (tokFunc[1] == 'x' || tokFunc[1] == 'X')) {
                    func_addr = (DWORD)strtoul(tokFunc + 2, NULL, 16);
                }
                /* Parse offset */
                DWORD offset = (DWORD)strtoul(tokOffset, NULL, 0);
                if (func_addr && offset < 0x10000) {
                    DWORD target = func_addr + offset;
                    /* Find the hex bytes part of the line */
                    char* bytes_start = line + 6; /* skip "PATCH " */
                    /* Skip func name */
                    while (*bytes_start && *bytes_start != ' ') bytes_start++;
                    while (*bytes_start == ' ') bytes_start++;
                    /* Skip offset */
                    while (*bytes_start && *bytes_start != ' ') bytes_start++;
                    while (*bytes_start == ' ') bytes_start++;

                    /* Parse hex bytes */
                    int byte_count = 0;
                    BYTE patch_bytes[256];
                    char* p = bytes_start;
                    while (*p && byte_count < 256) {
                        /* Skip whitespace */
                        while (*p == ' ' || *p == '\t') p++;
                        if (!*p) break;
                        /* Parse two hex digits */
                        int hi = -1, lo = -1;
                        if (*p >= '0' && *p <= '9') hi = *p - '0';
                        else if (*p >= 'A' && *p <= 'F') hi = *p - 'A' + 10;
                        else if (*p >= 'a' && *p <= 'f') hi = *p - 'a' + 10;
                        if (hi >= 0) {
                            p++;
                            if (*p >= '0' && *p <= '9') lo = *p - '0';
                            else if (*p >= 'A' && *p <= 'F') lo = *p - 'A' + 10;
                            else if (*p >= 'a' && *p <= 'f') lo = *p - 'a' + 10;
                            if (lo >= 0) {
                                patch_bytes[byte_count++] = (BYTE)(hi * 16 + lo);
                                p++;
                            }
                        } else {
                            break;
                        }
                    }
                    if (byte_count > 0) {
                        /* Apply patch with VirtualProtect */
                        DWORD oldProtect;
                        if (VirtualProtect((void*)target, byte_count,
                            PAGE_EXECUTE_READWRITE, &oldProtect)) {
                            memcpy((void*)target, patch_bytes, byte_count);
                            VirtualProtect((void*)target, byte_count,
                                oldProtect, &oldProtect);
                            /* Flush instruction cache */
                            FlushInstructionCache(GetCurrentProcess(),
                                (void*)target, byte_count);
                            changes++;
                            /* Log it */
                            char hexstr[256] = "";
                            int pos = 0;
                            for (int b = 0; b < byte_count && pos < 250; b++) {
                                pos += snprintf(hexstr + pos, sizeof(hexstr) - pos,
                                    "%02X ", patch_bytes[b]);
                            }
                            diag_logf("PATCH %s+0x%X (0x%08X) [%d bytes: %s]",
                                tokFunc, offset, target, byte_count, hexstr);
                        } else {
                            diag_logf("WARNING: PATCH VirtualProtect failed at 0x%08X", target);
                        }
                    } else {
                        diag_logf("WARNING: PATCH no hex bytes found");
                    }
                } else {
                    if (!func_addr) diag_logf("WARNING: PATCH unknown function '%s'", tokFunc);
                    if (offset >= 0x10000) diag_logf("WARNING: PATCH offset too large 0x%X", offset);
                }
            }
        }
        /* NOP <func_name|0xHEXADDR> <offset> <count>
         * NOP <count> bytes starting at func+offset.
         * Example:
         *   NOP Scene_SetupLevelCascade 0x10 5
         *   NOP Beginner_InitScene 5 3
         */
        else if (_strnicmp(line, "NOP ", 4) == 0) {
            char tokFunc[128] = "", tokOffset[64] = "", tokCount[64] = "";
            if (sscanf(line + 4, "%127s %63s %63s", tokFunc, tokOffset, tokCount) == 3) {
                DWORD func_addr = lookup_function_addr(tokFunc);
                if (!func_addr && tokFunc[0] == '0' && (tokFunc[1] == 'x' || tokFunc[1] == 'X')) {
                    func_addr = (DWORD)strtoul(tokFunc + 2, NULL, 16);
                }
                DWORD offset = (DWORD)strtoul(tokOffset, NULL, 0);
                int count = atoi(tokCount);
                if (func_addr && offset < 0x10000 && count > 0 && count <= 256) {
                    DWORD target = func_addr + offset;
                    DWORD oldProtect;
                    if (VirtualProtect((void*)target, count,
                        PAGE_EXECUTE_READWRITE, &oldProtect)) {
                        memset((void*)target, 0x90, count);
                        VirtualProtect((void*)target, count,
                            oldProtect, &oldProtect);
                        FlushInstructionCache(GetCurrentProcess(),
                            (void*)target, count);
                        changes++;
                        diag_logf("NOP %s+0x%X (0x%08X) [%d bytes]",
                            tokFunc, offset, target, count);
                    } else {
                        diag_logf("WARNING: NOP VirtualProtect failed at 0x%08X", target);
                    }
                } else {
                    if (!func_addr) diag_logf("WARNING: NOP unknown function '%s'", tokFunc);
                }
            }
        }
        /* WRITE <0xHEXADDR> <hex_bytes>
         * Write raw bytes at an absolute address.
         * Example:
         *   WRITE 0x004D04B0 00 00 00 00
         */
        else if (_strnicmp(line, "WRITE ", 6) == 0) {
            char tokAddr[64] = "";
            if (sscanf(line + 6, "%63s", tokAddr) == 1) {
                DWORD target = (DWORD)strtoul(tokAddr, NULL, 0);
                if (target > 0x400000) {
                    /* Find hex bytes */
                    char* bytes_start = line + 6;
                    while (*bytes_start && *bytes_start != ' ') bytes_start++;
                    while (*bytes_start == ' ') bytes_start++;
                    int byte_count = 0;
                    BYTE patch_bytes[256];
                    char* p = bytes_start;
                    while (*p && byte_count < 256) {
                        while (*p == ' ' || *p == '\t') p++;
                        if (!*p) break;
                        int hi = -1, lo = -1;
                        if (*p >= '0' && *p <= '9') hi = *p - '0';
                        else if (*p >= 'A' && *p <= 'F') hi = *p - 'A' + 10;
                        else if (*p >= 'a' && *p <= 'f') hi = *p - 'a' + 10;
                        if (hi >= 0) {
                            p++;
                            if (*p >= '0' && *p <= '9') lo = *p - '0';
                            else if (*p >= 'A' && *p <= 'F') lo = *p - 'A' + 10;
                            else if (*p >= 'a' && *p <= 'f') lo = *p - 'a' + 10;
                            if (lo >= 0) {
                                patch_bytes[byte_count++] = (BYTE)(hi * 16 + lo);
                                p++;
                            }
                        } else break;
                    }
                    if (byte_count > 0) {
                        DWORD oldProtect;
                        if (VirtualProtect((void*)target, byte_count,
                            PAGE_EXECUTE_READWRITE, &oldProtect)) {
                            memcpy((void*)target, patch_bytes, byte_count);
                            VirtualProtect((void*)target, byte_count,
                                oldProtect, &oldProtect);
                            FlushInstructionCache(GetCurrentProcess(),
                                (void*)target, byte_count);
                            changes++;
                            char hexstr[256] = "";
                            int pos = 0;
                            for (int b = 0; b < byte_count && pos < 250; b++) {
                                pos += snprintf(hexstr + pos, sizeof(hexstr) - pos,
                                    "%02X ", patch_bytes[b]);
                            }
                            diag_logf("WRITE 0x%08X [%d bytes: %s]", target, byte_count, hexstr);
                        } else {
                            diag_logf("WARNING: WRITE VirtualProtect failed at 0x%08X", target);
                        }
                    }
                }
            }
        }
        /* DUMP — log only native slots 0-35 */
        else if (_strnicmp(line, "DUMP", 4) == 0) {
            for (int i = 0; i < 15; i++) {
                diag_logf("  L%d (%s) [vt=0x%08X]:", i+1, g_levels[i].name, g_custom_vtables[i]);
                for (int s = 0; s < 36; s++) {
                    DWORD val = read_custom_vtable_entry(i+1, s);
                    const char* sname = "";
                    for (int k = 0; g_vtableSlots[k].name; k++) {
                        if (g_vtableSlots[k].slot == s) { sname = g_vtableSlots[k].name; break; }
                    }
                    const char* fname = "";
                    for (int k = 0; g_functions[k].name; k++) {
                        if (g_functions[k].addr == val) { fname = g_functions[k].name; break; }
                    }
                    int changed = (g_vtable_saved && val != g_orig_vtable[i][s]);
                    diag_logf("    [%2d] 0x%08X %-16s %-20s%s",
                        s, val, sname, fname, changed ? " ***" : "");
                }
                /* Also show extended slots that are non-zero */
                for (int s = 36; s < VTABLE_ENTRIES; s++) {
                    DWORD val = read_custom_vtable_entry(i+1, s);
                    if (val) {
                        diag_logf("    [%2d] 0x%08X (EXTENDED)", s, val);
                    }
                }
            }
            changes++;
        }
        /* RESET */
        else if (_strnicmp(line, "RESET", 5) == 0) {
            for (int i = 0; i < 15; i++) {
                restore_push_call(i + 1);
                DWORD vt = g_custom_vtables[i];
                if (vt) {
                    /* Restore native slots (0-35) from saved originals */
                    for (int s = 0; s < 36; s++) {
                        *(DWORD*)(vt + s * 4) = g_orig_vtable[i][s];
                    }
                    /* Clear extended slots (36-127) */
                    for (int s = 36; s < VTABLE_ENTRIES; s++) {
                        *(DWORD*)(vt + s * 4) = 0;
                    }
                }
            }
            changes++;
            diag_log("RESET: all levels restored");
        }

        line = strtok(NULL, "\n");
    }

    diag_logf("Applied %d config changes", changes);
    diag_log("========================================");
}

/* ============================================================
 * Extended vtable dispatch thread
 *
 * Background thread that calls extended vtable slots (36-127) every frame.
 * It reads the current board's vtable pointer, finds which level's custom
 * vtable it matches, then calls every non-zero entry in slots 36-127
 * with __thiscall(board) convention.
 *
 * This lets you add custom per-frame functions to any level via:
 *   VTABLE 1 36 0x0041B130    ; custom per-frame function on Level 1
 * ============================================================ */

typedef void (__thiscall *vtable_func_t)(DWORD this_ptr);

static DWORD g_dispatch_thread_id = 0;
static HANDLE g_dispatch_thread_handle = NULL;

/* Custom function system globals (declared early for dispatch_thread access) */
#define MAX_CUSTOM_FUNCS 64
#define MAX_FUNC_LINES 64
#define MAX_FUNC_VARS 16
#define MAX_FUNC_NAME 64

typedef struct {
    char lines[MAX_FUNC_LINES][256];
    int  num_lines;
} CustomFunc;

static CustomFunc g_custom_funcs[MAX_CUSTOM_FUNCS];
static char g_custom_func_names[MAX_CUSTOM_FUNCS][MAX_FUNC_NAME];
static int  g_num_custom_funcs = 0;

static DWORD WINAPI dispatch_thread(LPVOID param) {
    /* Wait for game to fully initialize */
    Sleep(3000);

    int diag_counter = 0;
    DWORD last_board = 0;

    while (g_dispatch_running) {
        /* Get current board via App → PlayerProfile → Board */
        if (IsBadReadPtr((void*)0x005341E0, 4)) { Sleep(16); continue; }
        DWORD app = *(DWORD*)0x005341E0;
        if (!app || app < 0x10000) { Sleep(16); continue; }
        if (IsBadReadPtr((void*)(app + 0x220), 4)) { Sleep(16); continue; }
        DWORD profile = *(DWORD*)(app + 0x220);
        if (!profile || profile < 0x10000) { Sleep(16); continue; }
        if (IsBadReadPtr((void*)(profile + 0x0C), 4)) { Sleep(16); continue; }
        DWORD board = *(DWORD*)(profile + 0x0C);
        if (!board || board < 0x10000) { Sleep(16); continue; }
        if (IsBadReadPtr((void*)board, 4)) { Sleep(16); continue; }

        /* Read the board's vtable pointer */
        DWORD board_vt = *(DWORD*)board;
        if (board_vt < 0x10000) { Sleep(16); continue; }

        /* Log when board changes (for debugging) */
        if (board != last_board) {
            diag_logf("[dispatch] Board changed: 0x%08X, vtable=0x%08X", board, board_vt);
            last_board = board;
            /* Log which level's custom vtable matches (or doesn't) */
            int found = 0;
            for (int i = 0; i < 15; i++) {
                if (g_custom_vtables[i] == board_vt) {
                    diag_logf("[dispatch] Matched custom vtable for L%d (%s)", i+1, g_levels[i].name);
                    /* Count non-zero extended slots */
                    int ext_count = 0;
                    for (int s = VTABLE_NATIVE_SLOTS; s < VTABLE_ENTRIES; s++) {
                        DWORD fa = *(DWORD*)(g_custom_vtables[i] + s * 4);
                        if (fa) ext_count++;
                    }
                    diag_logf("[dispatch] Level %d has %d extended slots active", i+1, ext_count);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                diag_logf("[dispatch] Board vtable 0x%08X does NOT match any custom vtable!", board_vt);
            }
        }

        /* Find which level's custom vtable matches */
        int level_idx = -1;
        for (int i = 0; i < 15; i++) {
            if (g_custom_vtables[i] == board_vt) {
                level_idx = i;
                break;
            }
        }

        /* If we found a match, dispatch extended slots */
        if (level_idx >= 0) {
            DWORD vt = g_custom_vtables[level_idx];
            for (int s = VTABLE_NATIVE_SLOTS; s < VTABLE_ENTRIES; s++) {
                DWORD func_addr = *(DWORD*)(vt + s * 4);
                if (!func_addr) continue;
                /* Check if this is a custom script function */
                if (is_custom_func_addr(func_addr)) {
                    int func_idx = func_addr - 0x10000000;
                    /* Periodic log for custom func execution */
                    if (diag_counter % 300 == 0) {
                        diag_logf("[dispatch] Calling custom func %d (%s) on slot %d",
                            func_idx,
                            (func_idx < g_num_custom_funcs) ? g_custom_func_names[func_idx] : "?",
                            s);
                    }
                    execute_custom_func(func_idx, board);
                } else if (!IsBadReadPtr((void*)func_addr, 1)) {
                    /* Native game function — call with __thiscall(board) */
                    vtable_func_t func = (vtable_func_t)func_addr;
                    func(board);
                }
            }
        }

        diag_counter++;
        /* ~60fps */
        Sleep(16);
    }

    return 0;
}

static void start_dispatch_thread(void) {
    g_dispatch_running = 1;
    g_dispatch_thread_handle = CreateThread(
        NULL, 0, dispatch_thread, NULL, 0, &g_dispatch_thread_id);
}

/* ============================================================
 * Custom script functions — read from mkn_level_functions.txt
 *
 * Syntax:
 *   FUNCTION MyFuncName
 *     CALL GameFuncName arg1 arg2 ...
 *     SET board+0x1A4 = 5.0
 *     SET board+0x168 = 0.0
 *     IF board+0x14C == 0
 *       CALL Scene_Update
 *     ENDIF
 *     READ board+0x3624 -> $var1
 *     SET board+0x3624 = $var1 + 0.025
 *   ENDFUNC
 *
 * Supported expressions:
 *   - board+0xOFFSET (dereference board pointer + offset)
 *   - 0xHEXADDR (absolute address)
 *   - $var (local variable)
 *   - Float literals: 5.0, 0.025, -1.0
 *   - Int literals: 42, 0x100
 *   - Arithmetic: +, -, *, /
 *   - Comparison: ==, !=, <, >, <=, >=
 *
 * Statements:
 *   CALL FuncName arg1 arg2 ...  — call a game function by name
 *   SET dest = expr              — write value to memory
 *   READ src -> $var             — read memory into variable
 *   IF expr                      — conditional block
 *   ENDIF
 *   NOP                          — do nothing
 *   LOG "message"                — write to log file
 * ============================================================ */

/* Custom function system globals moved earlier — see dispatch_thread section */

static int lookup_custom_func(const char* name) {
    for (int i = 0; i < g_num_custom_funcs; i++) {
        if (_stricmp(g_custom_func_names[i], name) == 0)
            return i;
    }
    return -1;
}

/* Get custom func address (returns a fake address that encodes the index) */
static DWORD get_custom_func_addr(const char* name) {
    int idx = lookup_custom_func(name);
    if (idx < 0) return 0;
    /* Use 0x10000000+idx as a sentinel address for custom functions */
    return 0x10000000 + idx;
}

static int is_custom_func_addr(DWORD addr) {
    return (addr >= 0x10000000 && addr < 0x10000000 + MAX_CUSTOM_FUNCS);
}

/* Helper: get or create variable index by name */
static int get_var_idx(const char* name, float* vars, char var_names[][32], int* num_vars) {
    int i;
    for (i = 0; i < *num_vars; i++) {
        if (_stricmp(var_names[i], name) == 0) return i;
    }
    if (*num_vars < MAX_FUNC_VARS) {
        int vi = (*num_vars)++;
        strncpy(var_names[vi], name, 31);
        var_names[vi][31] = '\0';
        vars[vi] = 0.0f;
        return vi;
    }
    return -1;
}

/* Execute a custom function with __thiscall(board) convention */
static void execute_custom_func(int func_idx, DWORD board) {
    if (func_idx < 0 || func_idx >= g_num_custom_funcs) return;
    CustomFunc* fn = &g_custom_funcs[func_idx];

    /* Local variables */
    float vars[MAX_FUNC_VARS];
    char var_names[MAX_FUNC_VARS][32];
    int num_vars = 0;
    int i;
    for (i = 0; i < MAX_FUNC_VARS; i++) { vars[i] = 0.0f; var_names[i][0] = '\0'; }

    int if_depth = 0;
    int if_active[MAX_FUNC_LINES];
    memset(if_active, 0, sizeof(if_active));
    if_active[0] = 1; /* start active */

    for (int li = 0; li < fn->num_lines; li++) {
        char* line = fn->lines[li];
        /* Skip if inside inactive IF block */
        if (!if_active[if_depth]) {
            /* But track IF/ENDIF nesting */
            if (_strnicmp(line, "IF ", 3) == 0) {
                if_depth++;
                if (if_depth < MAX_FUNC_LINES) if_active[if_depth] = 0;
            } else if (_strnicmp(line, "ENDIF", 5) == 0) {
                if_depth--;
                if (if_depth < 0) if_depth = 0;
            }
            continue;
        }

        /* CALL GameFuncName arg1 arg2 ... */
        if (_strnicmp(line, "CALL ", 5) == 0) {
            char fname[128] = "";
            char args_str[256] = "";
            int nargs = sscanf(line + 5, "%127s %255[^\n]", fname, args_str);
            if (nargs >= 1) {
                DWORD func_addr = lookup_function_addr(fname);
                if (!func_addr) func_addr = get_custom_func_addr(fname);
                if (func_addr && !IsBadReadPtr((void*)func_addr, 1)) {
                    /* Call with __thiscall convention: ECX=board, no args.
                     * MinGW doesn't support Intel syntax __asm blocks,
                     * so use a function pointer typedef. */
                    vtable_func_t func = (vtable_func_t)func_addr;
                    func(board);
                }
            }
        }
        /* SET dest = expr */
        else if (_strnicmp(line, "SET ", 4) == 0) {
            char dest[128] = "";
            char expr[256] = "";
            if (sscanf(line + 4, "%127[^=]= %255[^\n]", dest, expr) == 2 ||
                sscanf(line + 4, "%127s = %255[^\n]", dest, expr) == 2) {
                /* Trim dest */
                char* p = dest; while (*p == ' ') p++; memmove(dest, p, strlen(p)+1);
                p = dest + strlen(dest) - 1; while (p > dest && *p == ' ') *p-- = '\0';

                /* Evaluate expression (simple: just float literal or $var for now) */
                float val = 0.0f;
                if (expr[0] == '$') {
                    int vi = get_var_idx(expr + 1, vars, var_names, &num_vars);
                    if (vi >= 0) val = vars[vi];
                } else {
                    val = (float)atof(expr);
                }

                /* Write to destination */
                if (_strnicmp(dest, "board+", 6) == 0) {
                    DWORD offset = (DWORD)strtoul(dest + 6, NULL, 0);
                    if (!IsBadReadPtr((void*)(board + offset), 4)) {
                        *(float*)(board + offset) = val;
                    }
                } else if (dest[0] == '0' && dest[1] == 'x') {
                    DWORD addr = (DWORD)strtoul(dest, NULL, 16);
                    if (addr > 0x400000 && !IsBadReadPtr((void*)addr, 4)) {
                        *(float*)addr = val;
                    }
                } else if (dest[0] == '$') {
                    int vi = get_var_idx(dest + 1, vars, var_names, &num_vars);
                    if (vi >= 0) vars[vi] = val;
                }
            }
        }
        /* READ src -> $var */
        else if (_strnicmp(line, "READ ", 5) == 0) {
            char src[128] = "";
            char varname[64] = "";
            if (sscanf(line + 5, "%127s -> %63s", src, varname) == 2 ||
                sscanf(line + 5, "%127s %63s", src, varname) == 2) {
                float val = 0.0f;
                if (_strnicmp(src, "board+", 6) == 0) {
                    DWORD offset = (DWORD)strtoul(src + 6, NULL, 0);
                    if (!IsBadReadPtr((void*)(board + offset), 4)) {
                        val = *(float*)(board + offset);
                    }
                } else if (src[0] == '0' && src[1] == 'x') {
                    DWORD addr = (DWORD)strtoul(src, NULL, 16);
                    if (addr > 0x400000 && !IsBadReadPtr((void*)addr, 4)) {
                        val = *(float*)addr;
                    }
                }
                int vi = get_var_idx(varname + 1, vars, var_names, &num_vars); /* skip $ */
                if (vi >= 0) vars[vi] = val;
            }
        }
        /* IF expr */
        else if (_strnicmp(line, "IF ", 3) == 0) {
            if_depth++;
            if (if_depth < MAX_FUNC_LINES) {
                /* Simple condition: board+OFFSET == VALUE */
                char lhs[128] = "";
                char op[8] = "";
                char rhs[128] = "";
                if (sscanf(line + 3, "%127s %7s %127s", lhs, op, rhs) == 3) {
                    float lval = 0.0f, rval = 0.0f;
                    if (_strnicmp(lhs, "board+", 6) == 0) {
                        DWORD offset = (DWORD)strtoul(lhs + 6, NULL, 0);
                        if (!IsBadReadPtr((void*)(board + offset), 4))
                            lval = *(float*)(board + offset);
                    } else if (lhs[0] == '$') {
                        int vi = get_var_idx(lhs + 1, vars, var_names, &num_vars);
                        if (vi >= 0) lval = vars[vi];
                    } else {
                        lval = (float)atof(lhs);
                    }
                    if (rhs[0] == '$') {
                        int vi = get_var_idx(rhs + 1, vars, var_names, &num_vars);
                        if (vi >= 0) rval = vars[vi];
                    } else {
                        rval = (float)atof(rhs);
                    }
                    int result = 0;
                    if (_stricmp(op, "==") == 0) result = (lval == rval);
                    else if (_stricmp(op, "!=") == 0) result = (lval != rval);
                    else if (_stricmp(op, "<") == 0) result = (lval < rval);
                    else if (_stricmp(op, ">") == 0) result = (lval > rval);
                    else if (_stricmp(op, "<=") == 0) result = (lval <= rval);
                    else if (_stricmp(op, ">=") == 0) result = (lval >= rval);
                    if_active[if_depth] = result;
                } else {
                    if_active[if_depth] = 0;
                }
            }
        }
        /* ENDIF */
        else if (_strnicmp(line, "ENDIF", 5) == 0) {
            if_depth--;
            if (if_depth < 0) if_depth = 0;
        }
        /* LOG "message" */
        else if (_strnicmp(line, "LOG ", 4) == 0) {
            char* msg = line + 4;
            /* Remove quotes if present */
            if (*msg == '"') msg++;
            char* end = msg + strlen(msg) - 1;
            if (*end == '"') *end = '\0';
            diag_logf("[custom] %s", msg);
        }
        /* NOP — do nothing */
        else if (_strnicmp(line, "NOP", 3) == 0) {
            /* nothing */
        }
    }
}

/* Load custom functions from mkn_level_functions.txt */
static void load_custom_functions(void) {
    char func_path[MAX_PATH];
    strcpy(func_path, g_configPath);
    char* p = strrchr(func_path, '\\');
    if (p) {
        strcpy(p + 1, "mkn_level_functions.txt");
    } else {
        strcpy(func_path, "mkn_level_functions.txt");
    }

    diag_logf("[mkn_level_system v5] Looking for custom functions: %s", func_path);

    HANDLE hFile = CreateFileA(func_path,
        GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        diag_log("[mkn_level_system v5] No mkn_level_functions.txt found (custom functions disabled)");
        return;
    }

    char buf[65536];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    diag_logf("[mkn_level_system v5] Loading custom functions from %s", func_path);

    char* line = strtok(buf, "\n");
    int in_func = 0;
    int cur_func = -1;

    while (line) {
        /* Trim */
        while (*line == ' ' || *line == '\t') line++;
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n' ||
              line[len-1] == ' ' || line[len-1] == '\t')) {
            line[--len] = '\0';
        }
        /* Skip comments and empty lines */
        if (line[0] == '#' || line[0] == '\0') {
            line = strtok(NULL, "\n");
            continue;
        }

        /* FUNCTION name */
        if (_strnicmp(line, "FUNCTION ", 9) == 0) {
            if (g_num_custom_funcs >= MAX_CUSTOM_FUNCS) {
                diag_logf("WARNING: too many custom functions (max %d)", MAX_CUSTOM_FUNCS);
                break;
            }
            char fname[MAX_FUNC_NAME] = "";
            sscanf(line + 9, "%63s", fname);
            strncpy(g_custom_func_names[g_num_custom_funcs], fname, MAX_FUNC_NAME - 1);
            g_custom_funcs[g_num_custom_funcs].num_lines = 0;
            cur_func = g_num_custom_funcs;
            g_num_custom_funcs++;
            in_func = 1;
            diag_logf("  Custom function: %s (addr=0x%08X)", fname, get_custom_func_addr(fname));
        }
        /* ENDFUNC */
        else if (_strnicmp(line, "ENDFUNC", 7) == 0) {
            if (in_func && cur_func >= 0) {
                diag_logf("  %s: %d lines", g_custom_func_names[cur_func],
                          g_custom_funcs[cur_func].num_lines);
            }
            in_func = 0;
            cur_func = -1;
        }
        /* Function body line */
        else if (in_func && cur_func >= 0) {
            if (g_custom_funcs[cur_func].num_lines < MAX_FUNC_LINES) {
                strncpy(g_custom_funcs[cur_func].lines[g_custom_funcs[cur_func].num_lines],
                        line, 255);
                g_custom_funcs[cur_func].lines[g_custom_funcs[cur_func].num_lines][255] = '\0';
                g_custom_funcs[cur_func].num_lines++;
            }
        }

        line = strtok(NULL, "\n");
    }

    diag_logf("[mkn_level_system v5] Loaded %d custom functions", g_num_custom_funcs);
}

/* ============================================================
 * DLL Entry Point
 * ============================================================ */

static void mkn_init(void) {
    find_config_path();

    /* Step 1: Allocate custom vtables and patch ctors */
    init_custom_vtables();
    if (!g_vtables_initialized) {
        diag_log("[mkn_level_system v5] FATAL: custom vtable init failed!");
        return;
    }
    diag_log("[mkn_level_system v5] Custom vtables allocated (128 entries each) and ctors patched");

    /* Step 2: Patch ALL level allocations to max struct size */
    patch_all_struct_sizes();
    diag_logf("[mkn_level_system v5] All 15 levels patched to struct size 0x%X", MAX_STRUCT_SIZE);

    /* Step 3: Load custom script functions from mkn_level_functions.txt */
    load_custom_functions();

    /* Step 4: Parse config file for VTABLE/SET/SWAP/etc commands */
    apply_config();

    /* Step 5: Start extended vtable dispatch thread */
    start_dispatch_thread();
    diag_log("[mkn_level_system v5] Extended dispatch thread started (slots 36-127)");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        mkn_init();
    }
    return TRUE;
}

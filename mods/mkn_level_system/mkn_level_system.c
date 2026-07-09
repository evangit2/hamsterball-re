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

static DWORD lookup_function_addr(const char* name) {
    for (int i = 0; g_functions[i].name; i++) {
        if (_stricmp(g_functions[i].name, name) == 0)
            return g_functions[i].addr;
    }
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
        /* DUMP */
        else if (_strnicmp(line, "DUMP", 4) == 0) {
            for (int i = 0; i < 15; i++) {
                diag_logf("  L%d (%s) [vt=0x%08X]:", i+1, g_levels[i].name, g_custom_vtables[i]);
                for (int s = 0; s < VTABLE_ENTRIES; s++) {
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
            }
            changes++;
        }
        /* RESET */
        else if (_strnicmp(line, "RESET", 5) == 0) {
            for (int i = 0; i < 15; i++) {
                restore_push_call(i + 1);
                DWORD vt = g_custom_vtables[i];
                if (vt) {
                    for (int s = 0; s < VTABLE_ENTRIES; s++) {
                        *(DWORD*)(vt + s * 4) = g_orig_vtable[i][s];
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
 * DLL Entry Point
 * ============================================================ */

static void mkn_init(void) {
    find_config_path();

    /* Step 1: Allocate custom vtables and patch ctors */
    init_custom_vtables();
    if (!g_vtables_initialized) {
        diag_log("[mkn_level_system v4] FATAL: custom vtable init failed!");
        return;
    }
    diag_log("[mkn_level_system v4] Custom vtables allocated and ctors patched");

    /* Step 2: Patch ALL level allocations to max struct size.
     * This is the critical fix — ensures any vtable function from any
     * level can safely access any board offset without heap corruption. */
    patch_all_struct_sizes();
    diag_logf("[mkn_level_system v4] All 15 levels patched to struct size 0x%X", MAX_STRUCT_SIZE);

    /* Step 3: Parse config file for VTABLE/SET/SWAP/etc commands */
    apply_config();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        mkn_init();
    }
    return TRUE;
}

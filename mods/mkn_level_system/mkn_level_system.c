/*
 * mkn_level_system v2 — Level Swapper with Per-Level Object Control
 *
 * FIXES from v1:
 *   - v1 swapped only 72 bytes (half the 36-entry vtable) from the wrong
 *     offset, causing board dtor/update/render to mismatch setup/collision.
 *   - v1 didn't account for different board struct sizes per level
 *     (WarmUp=0x436c vs Dizzy=0x4be0), causing out-of-bounds access.
 *   - v1's SWAP didn't load level objects because Board_Setup (vtable[32])
 *     was in the unswapped half.
 *
 * v2 APPROACH:
 *   SWAP now patches Tournament_AdvanceRace (0x427080) — hooking the
 *   switch statement that dispatches board constructors. This patches
 *   both the PUSH <struct_size> and CALL <board_ctor> for each case,
 *   ensuring correct allocation + correct ctor + correct objects.
 *
 * NEW FEATURES:
 *   SWAPOBJ A B  — swap vtable[32] (Board_Setup) between two levels.
 *                  Changes which objects appear without changing geometry.
 *   SWAPGEO A B  — swap vtable[18] (setup function) between two levels.
 *                  Changes which .MESHWORLD loads without changing objects.
 *   RESET        — restore all original values.
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
 * in Tournament_AdvanceRace (0x427080).
 *
 * The switch at 0x427102 does:
 *   JMP [EAX*4 + 0x42761C]
 * Each case does:
 *   PUSH <struct_size>    ; at push_addr
 *   CALL operator_new     ; 0x4BA57B
 *   ...
 *   CALL <board_ctor>     ; at call_addr (rel32)
 *
 * Vtable layout (36 entries, 144 bytes):
 *   [0]-[17]  = common Board virtuals (dtor, update, render, etc.)
 *   [18]      = setup function (loads .MESHWORLD, calls Board_Setup)
 *   [32]      = Board_Setup (creates level objects via CreateLevelObjects)
 *   [33]-[35] = level-specific virtuals
 * ============================================================ */

typedef struct {
    int   level_num;
    const char* name;
    DWORD ctor_addr;      /* board constructor function address     */
    DWORD struct_size;    /* size passed to operator_new            */
    DWORD push_addr;      /* address of PUSH imm32 (struct size)   */
    DWORD call_addr;      /* address of CALL rel32 (board ctor)     */
    DWORD vtable_addr;    /* vtable start in .data (36 entries)     */
} LevelEntry;

/* Tournament_AdvanceRace switch cases (verified from disassembly) */
static const LevelEntry g_levels[15] = {
    { 1,  "WarmUp",       0x0041CA40, 0x436C, 0x00427109, 0x0042712C, 0x004D04A8 },
    { 2,  "Beginner",     0x004200E0, 0x644C, 0x00427136, 0x0042715D, 0x004D1098 },
    { 3,  "Intermediate", 0x0041CB20, 0x438C, 0x00427167, 0x0042718E, 0x004D05A0 },
    { 4,  "Dizzy",        0x0041D060, 0x4BE0, 0x00427198, 0x004271BF, 0x004D0890 },
    { 5,  "Tower",        0x0041E340, 0x5418, 0x004271C9, 0x004271F0, 0x004D0A08 },
    { 6,  "Up",           0x00420390, 0x4790, 0x004271FA, 0x00427221, 0x004D11A0 },
    { 7,  "Neon",         0x00424440, 0x4394, 0x0042722B, 0x00427252, 0x004D1DF0 },
    { 8,  "Expert",       0x0041EA40, 0x4FD8, 0x0042725C, 0x00427283, 0x004D0B00 },
    { 9,  "Odd",          0x0041ED80, 0x43B0, 0x0042728D, 0x004272B4, 0x004D0BC0 },
    { 10, "Toob",         0x0041F4B0, 0x646C, 0x004272BE, 0x004272E5, 0x004D0E78 },
    { 11, "Wobbly",       0x0041F110, 0x4388, 0x004272EF, 0x00427316, 0x004D0D38 },
    { 12, "Glass",        0x00424A90, 0x4390, 0x00427320, 0x00427347, 0x004D1F90 },
    { 13, "Sky",          0x0041F930, 0x47F8, 0x00427351, 0x00427374, 0x004D0FC8 },
    { 14, "Master",       0x004206D0, 0x6498, 0x0042737B, 0x0042739E, 0x004D12B0 },
    { 15, "Impossible",   0x00424C20, 0x4380, 0x004273A5, 0x004273C8, 0x004D21C0 },
};

#define VTABLE_SETUP_OFFSET  0x48   /* vtable[18] = setup function   */
#define VTABLE_OBJ_OFFSET   0x80   /* vtable[32] = Board_Setup       */
#define VTABLE_SIZE         144    /* 36 entries × 4 bytes            */

/* ============================================================
 * Function name → address table (for SETUP and ADD commands)
 * ============================================================ */

typedef struct {
    const char* name;
    DWORD addr;
} FuncEntry;

static const FuncEntry g_functions[] = {
    /* Setup functions (load .MESHWORLD files) */
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
    /* Sub-functions (for ADD chaining) */
    { "Level_InitScene",           0x0040B090 },
    { "WaterRipple_AllocBuffers", 0x0046A8A0 },
    { "AthenaList_Append",         0x00453780 },
    { "SceneObject_ctor",          0x00462850 },
    { "Vec3_Init",                 0x00453180 },
    { "Scene_RegisterObject",      0x00453BD0 },
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

/* ============================================================
 * Original value storage (for RESET)
 * ============================================================ */

typedef struct {
    int   used;
    DWORD push_orig;    /* original 4 bytes at push_addr+1 */
    DWORD call_orig;    /* original 4 bytes at call_addr+1 */
} OrigEntry;

static OrigEntry g_orig[15] = {0};

/* Original vtable entries (for SWAPOBJ/SWAPGEO reset) */
static DWORD g_orig_vtable18[15] = {0};
static DWORD g_orig_vtable32[15] = {0};
static int   g_vtable_saved = 0;

static void save_originals(void) {
    if (g_vtable_saved) return;
    for (int i = 0; i < 15; i++) {
        DWORD vt = g_levels[i].vtable_addr;
        if (!IsBadReadPtr((void*)(vt + VTABLE_SETUP_OFFSET), 4))
            g_orig_vtable18[i] = *(DWORD*)(vt + VTABLE_SETUP_OFFSET);
        if (!IsBadReadPtr((void*)(vt + VTABLE_OBJ_OFFSET), 4))
            g_orig_vtable32[i] = *(DWORD*)(vt + VTABLE_OBJ_OFFSET);
    }
    g_vtable_saved = 1;
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

/* Patch the PUSH imm32 (struct size) for a given level case */
static void patch_push(int level_num, DWORD new_size) {
    if (level_num < 1 || level_num > 15) return;
    DWORD addr = g_levels[level_num - 1].push_addr + 1; /* skip 0x68 opcode */
    patch_dword(addr, new_size);
}

/* Patch the CALL rel32 (board ctor) for a given level case */
static void patch_call(int level_num, DWORD new_ctor) {
    if (level_num < 1 || level_num > 15) return;
    DWORD call_addr = g_levels[level_num - 1].call_addr;
    /* CALL rel32: target = call_addr + 5 + offset */
    DWORD offset = new_ctor - (call_addr + 5);
    patch_dword(call_addr + 1, offset); /* skip 0xE8 opcode */
}

/* Save original PUSH+CALL values for a level */
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

/* Restore original PUSH+CALL for a level */
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
 * Vtable entry swap helpers
 * ============================================================ */

static DWORD read_vtable_entry(int level_num, int entry_offset) {
    if (level_num < 1 || level_num > 15) return 0;
    DWORD addr = g_levels[level_num - 1].vtable_addr + entry_offset;
    if (IsBadReadPtr((void*)addr, 4)) return 0;
    return *(DWORD*)addr;
}

static void write_vtable_entry(int level_num, int entry_offset, DWORD value) {
    if (level_num < 1 || level_num > 15) return;
    DWORD addr = g_levels[level_num - 1].vtable_addr + entry_offset;
    patch_dword(addr, value);
}

/* Swap a single vtable entry between two levels */
static void swap_vtable_entry(int a, int b, int entry_offset) {
    if (a < 1 || a > 15 || b < 1 || b > 15 || a == b) return;
    DWORD va = read_vtable_entry(a, entry_offset);
    DWORD vb = read_vtable_entry(b, entry_offset);
    write_vtable_entry(a, entry_offset, vb);
    write_vtable_entry(b, entry_offset, va);
    diag_logf("  vtable[0x%02X]: L%d=0x%08X <-> L%d=0x%08X",
        entry_offset, a, va, b, vb);
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

/* Lookup a level by number (1-15) or name (case-insensitive) */
static int lookup_level(const char* token) {
    /* Try number first */
    int num = atoi(token);
    if (num >= 1 && num <= 15) return num;
    /* Try name */
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
        diag_log("[mkn_level_system] Config file not found, no changes applied");
        return;
    }

    char buf[32768];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    diag_log("========================================");
    diag_log("[mkn_level_system v2] Config loaded, parsing...");
    diag_logf("Config path: %s", g_configPath);

    /* Save originals before any patches */
    save_originals();
    for (int i = 0; i < 15; i++)
        save_push_call(i + 1);

    /* Dump current state */
    diag_log("--- Current level assignments ---");
    for (int i = 0; i < 15; i++) {
        DWORD push_val = 0;
        if (!IsBadReadPtr((void*)(g_levels[i].push_addr + 1), 4))
            push_val = *(DWORD*)(g_levels[i].push_addr + 1);
        DWORD call_target = 0;
        if (!IsBadReadPtr((void*)(g_levels[i].call_addr + 1), 4)) {
            DWORD call_off = *(DWORD*)(g_levels[i].call_addr + 1);
            call_target = g_levels[i].call_addr + 5 + call_off;
        }
        const char* ctor_name = "???";
        for (int j = 0; j < 15; j++) {
            if (g_levels[j].ctor_addr == call_target) {
                ctor_name = g_levels[j].name;
                break;
            }
        }
        diag_logf("  L%-2d: size=0x%04X ctor=0x%08X (%s)%s",
            g_levels[i].level_num, push_val, call_target, ctor_name,
            (call_target == g_levels[i].ctor_addr) ? "" : " *** CHANGED ***");
    }

    char* line = strtok(buf, "\n");
    int in_config = 0;
    int changes = 0;

    while (line) {
        trim(line);
        /* Check for CONFIG SECTION / END CONFIG markers (even in comments) */
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

        /* SWAP <A> <B>
         * Full level swap: patches Tournament_AdvanceRace to redirect
         * case A → B's ctor (with B's struct size) and vice versa.
         * This is the SAFE way to swap — correct allocation, ctor,
         * geometry, objects, physics, everything. */
        if (_strnicmp(line, "SWAP ", 5) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 5, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    /* Swap PUSH (struct size) */
                    patch_push(a, g_levels[b - 1].struct_size);
                    patch_push(b, g_levels[a - 1].struct_size);
                    /* Swap CALL (board ctor) */
                    patch_call(a, g_levels[b - 1].ctor_addr);
                    patch_call(b, g_levels[a - 1].ctor_addr);
                    changes += 2;
                    diag_logf("SWAP L%d (%s) <-> L%d (%s) [full ctor redirect]",
                        a, g_levels[a-1].name, b, g_levels[b-1].name);
                } else {
                    diag_logf("WARNING: SWAP invalid levels '%s' '%s'", tokA, tokB);
                }
            }
        }

        /* SWAPOBJ <A> <B>
         * Swaps vtable[32] (Board_Setup) between two levels.
         * Changes which objects appear on each level without changing
         * the board ctor or geometry. The board ctor runs normally
         * (correct struct size, correct meshes), but when it calls
         * Board_Setup via vtable[32], it creates the OTHER level's
         * objects instead.
         *
         * WARNING: May crash if the target level's Board_Setup expects
         * mesh slots that the source level's ctor didn't initialize. */
        else if (_strnicmp(line, "SWAPOBJ ", 8) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 8, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    swap_vtable_entry(a, b, VTABLE_OBJ_OFFSET);
                    changes++;
                    diag_logf("SWAPOBJ L%d (%s) <-> L%d (%s) [vtable[32] Board_Setup]",
                        a, g_levels[a-1].name, b, g_levels[b-1].name);
                } else {
                    diag_logf("WARNING: SWAPOBJ invalid levels '%s' '%s'", tokA, tokB);
                }
            }
        }

        /* SWAPGEO <A> <B>
         * Swaps vtable[18] (setup function) between two levels.
         * Changes which .MESHWORLD file loads without changing the
         * board ctor or objects. The board ctor runs normally (correct
         * struct size), but the setup function loads the OTHER level's
         * geometry.
         *
         * WARNING: May crash if the board ctor initializes mesh slots
         * that the swapped setup function doesn't expect. */
        else if (_strnicmp(line, "SWAPGEO ", 8) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 8, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    swap_vtable_entry(a, b, VTABLE_SETUP_OFFSET);
                    changes++;
                    diag_logf("SWAPGEO L%d (%s) <-> L%d (%s) [vtable[18] setup]",
                        a, g_levels[a-1].name, b, g_levels[b-1].name);
                } else {
                    diag_logf("WARNING: SWAPGEO invalid levels '%s' '%s'", tokA, tokB);
                }
            }
        }

        /* SET <level> <target_level>
         * One-way: makes level A use level B's board ctor.
         * Patches PUSH (struct size) + CALL (board ctor) for case A.
         * This fully replaces level A with level B's geometry + objects + physics. */
        else if (_strnicmp(line, "SET ", 4) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 4, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    patch_push(a, g_levels[b - 1].struct_size);
                    patch_call(a, g_levels[b - 1].ctor_addr);
                    changes++;
                    diag_logf("SET L%d (%s) -> L%d (%s) [ctor redirect]",
                        a, g_levels[a-1].name, b, g_levels[b-1].name);
                } else {
                    diag_logf("WARNING: SET invalid levels '%s' '%s'", tokA, tokB);
                }
            }
        }

        /* SETUP <level> <func_name>
         * Replaces a level's vtable[18] (setup function) with a named function.
         * The setup function loads the .MESHWORLD file and initializes geometry.
         * Available function names are listed in mkn_level_system.txt. */
        else if (_strnicmp(line, "SETUP ", 6) == 0) {
            char tokA[64] = "", fname[128] = "";
            if (sscanf(line + 6, "%63s %127s", tokA, fname) == 2) {
                int a = lookup_level(tokA);
                DWORD addr = lookup_function_addr(fname);
                if (a && addr) {
                    write_vtable_entry(a, VTABLE_SETUP_OFFSET, addr);
                    changes++;
                    diag_logf("SETUP L%d (%s) -> %s (0x%08X) [vtable[18]]",
                        a, g_levels[a-1].name, fname, addr);
                } else {
                    if (!a) diag_logf("WARNING: SETUP invalid level '%s'", tokA);
                    if (!addr) diag_logf("WARNING: SETUP unknown function '%s'", fname);
                }
            }
        }

        /* ADD <level> <func_name>
         * Adds an extra function call after a level's setup function.
         * Creates a trampoline via VirtualAlloc that chains:
         *   call original_setup; call extra_func; ret
         * Then patches vtable[18] to point to the trampoline. */
        else if (_strnicmp(line, "ADD ", 4) == 0) {
            char tokA[64] = "", fname[128] = "";
            if (sscanf(line + 4, "%63s %127s", tokA, fname) == 2) {
                int a = lookup_level(tokA);
                DWORD extra_addr = lookup_function_addr(fname);
                if (a && extra_addr) {
                    /* Get current vtable[18] (might be original or already patched) */
                    DWORD current = read_vtable_entry(a, VTABLE_SETUP_OFFSET);
                    if (!current) {
                        diag_logf("WARNING: ADD cannot read vtable[18] for L%d", a);
                    } else {
                        /* Build trampoline: call current_setup, call extra, ret
                         * Layout (9 bytes per call + 1 ret):
                         *   51           push ecx      ; save this (thiscall)
                         *   B8 aa aa aa  mov eax, addr
                         *   FF D0        call eax
                         *   59           pop ecx       ; restore this
                         *   51           push ecx      ; save for extra
                         *   B8 bb bb bb  mov eax, extra
                         *   FF D0        call eax
                         *   59           pop ecx
                         *   C3           ret
                         */
                        int total_size = 9 * 2 + 1;  /* 2 calls + ret */
                        BYTE* mem = (BYTE*)VirtualAlloc(NULL, total_size,
                            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                        if (mem) {
                            int pos = 0;
                            /* Call current setup */
                            mem[pos++] = 0x51;  /* push ecx */
                            mem[pos++] = 0xB8;  /* mov eax, imm32 */
                            *(DWORD*)(mem + pos) = current; pos += 4;
                            mem[pos++] = 0xFF; mem[pos++] = 0xD0;  /* call eax */
                            mem[pos++] = 0x59;  /* pop ecx */
                            /* Call extra function */
                            mem[pos++] = 0x51;  /* push ecx */
                            mem[pos++] = 0xB8;  /* mov eax, imm32 */
                            *(DWORD*)(mem + pos) = extra_addr; pos += 4;
                            mem[pos++] = 0xFF; mem[pos++] = 0xD0;  /* call eax */
                            mem[pos++] = 0x59;  /* pop ecx */
                            /* Ret */
                            mem[pos++] = 0xC3;
                            /* Patch vtable[18] to trampoline */
                            write_vtable_entry(a, VTABLE_SETUP_OFFSET, (DWORD)mem);
                            changes++;
                            diag_logf("ADD L%d (%s) += %s (0x%08X) [trampoline at 0x%08X]",
                                a, g_levels[a-1].name, fname, extra_addr, (DWORD)mem);
                        } else {
                            diag_logf("WARNING: ADD VirtualAlloc failed for L%d", a);
                        }
                    }
                } else {
                    if (!a) diag_logf("WARNING: ADD invalid level '%s'", tokA);
                    if (!extra_addr) diag_logf("WARNING: ADD unknown function '%s'", fname);
                }
            }
        }

        /* RESET
         * Restores all original PUSH+CALL values and vtable entries. */
        else if (_strnicmp(line, "RESET", 5) == 0) {
            for (int i = 0; i < 15; i++) {
                restore_push_call(i + 1);
                write_vtable_entry(i + 1, VTABLE_SETUP_OFFSET, g_orig_vtable18[i]);
                write_vtable_entry(i + 1, VTABLE_OBJ_OFFSET, g_orig_vtable32[i]);
            }
            changes++;
            diag_log("RESET: all levels restored to original values");
        }

        line = strtok(NULL, "\n");
    }

    if (changes > 0) {
        diag_logf("Applied %d config changes", changes);
    } else {
        diag_log("No config changes (defaults)");
    }

    /* Dump final state */
    diag_log("--- Final level assignments ---");
    for (int i = 0; i < 15; i++) {
        DWORD push_val = 0;
        if (!IsBadReadPtr((void*)(g_levels[i].push_addr + 1), 4))
            push_val = *(DWORD*)(g_levels[i].push_addr + 1);
        DWORD call_target = 0;
        if (!IsBadReadPtr((void*)(g_levels[i].call_addr + 1), 4)) {
            DWORD call_off = *(DWORD*)(g_levels[i].call_addr + 1);
            call_target = g_levels[i].call_addr + 5 + call_off;
        }
        const char* ctor_name = "???";
        for (int j = 0; j < 15; j++) {
            if (g_levels[j].ctor_addr == call_target) {
                ctor_name = g_levels[j].name;
                break;
            }
        }
        DWORD vt18 = read_vtable_entry(i + 1, VTABLE_SETUP_OFFSET);
        DWORD vt32 = read_vtable_entry(i + 1, VTABLE_OBJ_OFFSET);
        diag_logf("  L%-2d: size=0x%04X ctor=%s vtable[18]=0x%08X vtable[32]=0x%08X%s",
            g_levels[i].level_num, push_val, ctor_name, vt18, vt32,
            (call_target == g_levels[i].ctor_addr &&
             vt18 == g_orig_vtable18[i] &&
             vt32 == g_orig_vtable32[i]) ? "" : " *** CHANGED ***");
    }
    diag_log("========================================");
}

/* ============================================================
 * DLL Entry Point
 * ============================================================ */

static void mkn_init(void) {
    find_config_path();
    apply_config();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        mkn_init();
    }
    return TRUE;
}

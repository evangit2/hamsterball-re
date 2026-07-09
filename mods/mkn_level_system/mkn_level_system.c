/*
 * mkn_level_system v3 — Level Swapper with Custom Vtable Allocation
 *
 * FIXES from v2:
 *   - v2's VTABLE commands wrote to the original .data vtable. When SET/SWAP
 *     redirected a level's ctor to another level's ctor, the ctor wrote the
 *     OTHER level's vtable pointer into the board — so VTABLE commands on
 *     the original level's vtable had NO EFFECT.
 *   - v2's SWAPGEO was dangerous: swapping only vtable[18] while keeping the
 *     original board ctor meant the struct was allocated at the wrong size.
 *     Beginner functions accessing board+0x642C crashed on WarmUp's 0x436C
 *     allocation (8KB out of bounds → heap corruption).
 *
 * v3 APPROACH:
 *   Custom vtable allocation. At init, we VirtualAlloc 15 independent vtable
 *   copies (144 bytes each), copy the originals, then patch each board ctor's
 *   vtable-write instruction to point to our custom copy. Now:
 *
 *   - VTABLE A slot func — writes to custom_vtable[A], ALWAYS works regardless
 *     of SET/SWAP, because the ctor always writes custom_vtable[A] into the
 *     board (even when redirected to another level's ctor+struct).
 *   - SET A B — patches both struct size AND ctor in Tournament_AdvanceRace.
 *     Level A gets B's ctor (correct struct size) but still uses
 *     custom_vtable[A] (which you can freely modify).
 *   - SWAP A B — full swap of ctor+struct, vtables stay independent.
 *   - SWAPGEO A B — swaps vtable[18] between custom copies (geometry only).
 *   - SWAPOBJ A B — swaps vtable[32] between custom copies (objects only).
 *   - COPYVT A B — copy ALL 36 entries from level B's custom vtable into
 *                  level A's custom vtable. Useful with SET: SET 1 2 gives
 *                  Level 1 Beginner's struct+ctor, then COPYVT 1 2 gives it
 *                  Beginner's vtable as a starting point to customize.
 *   - RESET — restore all custom vtables to originals + restore push/call.
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
    DWORD vtable_addr;    /* original vtable start in .data (36 entries) */
    DWORD ctor_vt_patch;  /* address of vtable imm32 in ctor (C7 06 + 2) */
} LevelEntry;

/* Tournament_AdvanceRace switch cases + ctor vtable-write patch points
 * (verified from disassembly — each ctor does C7 06 <vtable_addr>) */
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

#define VTABLE_SETUP_OFFSET  0x48   /* vtable[18] = setup function   */
#define VTABLE_OBJ_OFFSET   0x80   /* vtable[32] = Board_Setup       */
#define VTABLE_SIZE         144    /* 36 entries × 4 bytes            */
#define VTABLE_ENTRIES      36

/* ============================================================
 * Custom vtable storage — 15 independent writable vtable copies
 * ============================================================ */

static DWORD g_custom_vtables[15] = {0};  /* pointers to VirtualAlloc'd vtables */
static int   g_vtables_initialized = 0;

/* Initialize custom vtables: allocate, copy originals, patch ctors */
static void init_custom_vtables(void) {
    if (g_vtables_initialized) return;

    for (int i = 0; i < 15; i++) {
        /* Allocate writable memory for vtable copy */
        DWORD addr = (DWORD)VirtualAlloc(NULL, VTABLE_SIZE,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!addr) continue;

        /* Copy original vtable from .data */
        if (!IsBadReadPtr((void*)g_levels[i].vtable_addr, VTABLE_SIZE)) {
            memcpy((void*)addr, (void*)g_levels[i].vtable_addr, VTABLE_SIZE);
        }

        g_custom_vtables[i] = addr;

        /* Patch the board ctor to write our custom vtable address
         * instead of the original .data vtable address.
         * The ctor does: C7 06 <vtable_addr_le>  (mov [esi], imm32)
         * We patch the 4-byte immediate to our custom vtable. */
        patch_dword(g_levels[i].ctor_vt_patch, addr);
    }

    g_vtables_initialized = 1;
}

/* ============================================================
 * Vtable slot names — for documentation and VTABLE command
 * ============================================================ */

static const struct { int slot; const char* name; const char* desc; } g_vtableSlots[] = {
    { 0,  "dtor",           "Scalar deleting destructor (per-level)" },
    { 1,  "update",         "Board_Update (per-level variant)" },
    { 18, "setup",           "Setup function — loads .MESHWORLD file" },
    { 19, "initscene",       "InitScene — post-setup initialization" },
    { 24, "renderdynamic",   "RenderDynamic — renders dynamic objects" },
    { 29, "dispatchcollision","DispatchCollision — collision event handler" },
    { 32, "boardsetup",      "Board_Setup — creates level objects (Catapult, etc.)" },
    { 33, "levelspecific",   "Level-specific function (varies per level)" },
    { -1, NULL, NULL }
};

/* ============================================================
 * Function name → address table (for SETUP and ADD commands)
 * ============================================================ */

typedef struct {
    const char* name;
    DWORD addr;
} FuncEntry;

static const FuncEntry g_functions[] = {
    /* Setup functions (vtable[18]) — load .MESHWORLD files */
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
    { "FUN_00417640",               0x00417640 },  /* Glass setup */
    { "FUN_00417F20",               0x00417F20 },  /* Impossible setup */
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
    /* Board_Setup (vtable[32]) — creates level objects */
    { "WarmUp_BoardSetup",         0x0041C5B0 },
    { "Beginner_BoardSetup",       0x0041C5B0 },  /* shared with WarmUp */
    { "Dizzy_BoardSetup",          0x0041C5B0 },  /* shared */
    /* Sub-functions safe for ADD (all __thiscall with ECX=board*) */
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

/* Look up a vtable slot index by name (returns -1 if not found) */
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
    DWORD push_orig;    /* original 4 bytes at push_addr+1 */
    DWORD call_orig;    /* original 4 bytes at call_addr+1 */
} OrigEntry;

static OrigEntry g_orig[15] = {0};

/* Original vtable entries (full 36 slots for RESET) */
static DWORD g_orig_vtable[15][VTABLE_ENTRIES] = {{0}};
static int   g_vtable_saved = 0;

static void save_originals(void) {
    if (g_vtable_saved) return;
    for (int i = 0; i < 15; i++) {
        DWORD vt = g_levels[i].vtable_addr;
        for (int s = 0; s < VTABLE_ENTRIES; s++) {
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

/* Swap a single vtable entry between two levels' custom vtables */
static void swap_custom_vtable_entry(int a, int b, int slot) {
    if (a < 1 || a > 15 || b < 1 || b > 15 || a == b) return;
    DWORD va = read_custom_vtable_entry(a, slot);
    DWORD vb = read_custom_vtable_entry(b, slot);
    write_custom_vtable_entry(a, slot, vb);
    write_custom_vtable_entry(b, slot, va);
}

/* Copy all 36 entries from level B's custom vtable to level A's */
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
        diag_log("[mkn_level_system v3] Config file not found, no changes applied");
        return;
    }

    char buf[32768];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    diag_log("========================================");
    diag_log("[mkn_level_system v3] Config loaded, parsing...");
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
         * geometry, objects, physics, everything.
         * Custom vtables stay independent — VTABLE commands still work
         * per-level. */
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
         * Swaps vtable[32] (Board_Setup) between two levels' custom vtables.
         * Changes which objects appear on each level without changing geometry.
         * The board ctor runs normally (correct struct size), but when it calls
         * Board_Setup via vtable[32], it creates the OTHER level's objects. */
        else if (_strnicmp(line, "SWAPOBJ ", 8) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 8, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    swap_custom_vtable_entry(a, b, 32);
                    changes++;
                    diag_logf("SWAPOBJ L%d (%s) <-> L%d (%s) [vtable[32] Board_Setup]",
                        a, g_levels[a-1].name, b, g_levels[b-1].name);
                } else {
                    diag_logf("WARNING: SWAPOBJ invalid levels '%s' '%s'", tokA, tokB);
                }
            }
        }

        /* SWAPGEO <A> <B>
         * Swaps vtable[18] (setup function) between two levels' custom vtables.
         * Changes which .MESHWORLD file loads without changing the board ctor
         * or objects.
         * WARNING: May crash if the swapped setup function writes to board
         * offsets beyond the original level's struct size. Use SET instead
         * for safe cross-struct-size swaps. */
        else if (_strnicmp(line, "SWAPGEO ", 8) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 8, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    swap_custom_vtable_entry(a, b, 18);
                    changes++;
                    diag_logf("SWAPGEO L%d (%s) <-> L%d (%s) [vtable[18] setup]",
                        a, g_levels[a-1].name, b, g_levels[b-1].name);
                } else {
                    diag_logf("WARNING: SWAPGEO invalid levels '%s' '%s'", tokA, tokB);
                }
            }
        }

        /* SET <level> <target_level>
         * One-way: makes level A use level B's board ctor + struct size.
         * Patches PUSH (struct size) + CALL (board ctor) for case A.
         * Level A gets B's struct allocation + B's ctor, but still uses
         * custom_vtable[A] (which you can freely modify with VTABLE).
         * Tip: use COPYVT A B after SET to start from B's vtable layout. */
        else if (_strnicmp(line, "SET ", 4) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 4, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    patch_push(a, g_levels[b - 1].struct_size);
                    patch_call(a, g_levels[b - 1].ctor_addr);
                    changes++;
                    diag_logf("SET L%d (%s) -> L%d (%s) [ctor+struct redirect]",
                        a, g_levels[a-1].name, b, g_levels[b-1].name);
                } else {
                    diag_logf("WARNING: SET invalid levels '%s' '%s'", tokA, tokB);
                }
            }
        }

        /* COPYVT <A> <B>
         * Copy ALL 36 vtable entries from level B's custom vtable into
         * level A's custom vtable. Useful with SET: SET 1 2 gives Level 1
         * Beginner's struct+ctor, then COPYVT 1 2 gives it Beginner's
         * vtable as a starting point to customize further. */
        else if (_strnicmp(line, "COPYVT ", 7) == 0) {
            char tokA[64] = "", tokB[64] = "";
            if (sscanf(line + 7, "%63s %63s", tokA, tokB) == 2) {
                int a = lookup_level(tokA);
                int b = lookup_level(tokB);
                if (a && b && a != b) {
                    copy_custom_vtable(a, b);
                    changes++;
                    diag_logf("COPYVT L%d (%s) <- L%d (%s) [all 36 entries]",
                        a, g_levels[a-1].name, b, g_levels[b-1].name);
                } else {
                    diag_logf("WARNING: COPYVT invalid levels '%s' '%s'", tokA, tokB);
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
                    write_custom_vtable_entry(a, 18, addr);
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
                    DWORD current = read_custom_vtable_entry(a, 18);
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
                            write_custom_vtable_entry(a, 18, (DWORD)mem);
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

        /* VTABLE <level> <slot> <func_name|0xHEXADDR>
         * Write any function into any vtable slot for any level.
         * Writes to the level's CUSTOM vtable — works regardless of SET/SWAP.
         * <slot> can be a number (0-35) or a name (setup, boardsetup, etc.)
         * <func_name> can be a name from the function table or a hex address.
         * Use 0 or null to clear a slot (writes 0).
         * Examples:
         *   VTABLE 1 setup Scene_SetupLevelCascade
         *   VTABLE 3 0 Beginner_dtor
         *   VTABLE 5 boardsetup 0x0041C5B0
         *   VTABLE 1 9 Odd_DispatchColl
         *   VTABLE 1 2 0          ; clear slot 2 */
        else if (_strnicmp(line, "VTABLE ", 7) == 0) {
            char tokLevel[64] = "", tokSlot[64] = "", tokFunc[128] = "";
            if (sscanf(line + 7, "%63s %63s %127s", tokLevel, tokSlot, tokFunc) == 3) {
                int a = lookup_level(tokLevel);
                /* Parse slot: number or name */
                int slot = atoi(tokSlot);
                if (slot == 0 && tokSlot[0] != '0') {
                    slot = lookup_vtable_slot(tokSlot);
                }
                /* Parse func: name, hex address, or 0/null */
                DWORD addr = 0;
                if (_stricmp(tokFunc, "0") == 0 || _stricmp(tokFunc, "null") == 0) {
                    addr = 0;  /* explicit clear */
                } else {
                    addr = lookup_function_addr(tokFunc);
                    if (!addr && tokFunc[0] == '0' && (tokFunc[1] == 'x' || tokFunc[1] == 'X')) {
                        addr = (DWORD)strtoul(tokFunc + 2, NULL, 16);
                    }
                }
                if (a && slot >= 0 && slot < VTABLE_ENTRIES) {
                    write_custom_vtable_entry(a, slot, addr);
                    changes++;
                    diag_logf("VTABLE L%d (%s) [%d] = 0x%08X",
                        a, g_levels[a-1].name, slot, addr);
                } else {
                    if (!a) diag_logf("WARNING: VTABLE invalid level '%s'", tokLevel);
                    if (slot < 0 || slot >= VTABLE_ENTRIES)
                        diag_logf("WARNING: VTABLE invalid slot '%s' (use 0-35 or name)", tokSlot);
                    if (!addr && _stricmp(tokFunc, "0") != 0 && _stricmp(tokFunc, "null") != 0)
                        diag_logf("WARNING: VTABLE unknown function '%s'", tokFunc);
                }
            }
        }

        /* DUMP — log the full 36-entry vtable for every level */
        else if (_strnicmp(line, "DUMP", 4) == 0) {
            diag_log("--- Full custom vtable dump (36 entries × 15 levels) ---");
            for (int i = 0; i < 15; i++) {
                diag_logf("  L%d (%s) [custom vtable at 0x%08X]:",
                    i+1, g_levels[i].name, g_custom_vtables[i]);
                for (int s = 0; s < VTABLE_ENTRIES; s++) {
                    DWORD val = read_custom_vtable_entry(i+1, s);
                    /* Find slot name */
                    const char* sname = "";
                    for (int k = 0; g_vtableSlots[k].name; k++) {
                        if (g_vtableSlots[k].slot == s) { sname = g_vtableSlots[k].name; break; }
                    }
                    /* Find function name */
                    const char* fname = "";
                    for (int k = 0; g_functions[k].name; k++) {
                        if (g_functions[k].addr == val) { fname = g_functions[k].name; break; }
                    }
                    int changed = (g_vtable_saved && val != g_orig_vtable[i][s]);
                    diag_logf("    [%2d] 0x%08X %-16s %-20s%s",
                        s, val, sname, fname, changed ? " *** CHANGED ***" : "");
                }
            }
            changes++;
        }

        /* RESET
         * Restores all custom vtables to original values + push/call. */
        else if (_strnicmp(line, "RESET", 5) == 0) {
            for (int i = 0; i < 15; i++) {
                restore_push_call(i + 1);
                /* Restore custom vtable from saved originals */
                DWORD vt = g_custom_vtables[i];
                if (vt) {
                    for (int s = 0; s < VTABLE_ENTRIES; s++) {
                        *(DWORD*)(vt + s * 4) = g_orig_vtable[i][s];
                    }
                }
            }
            changes++;
            diag_log("RESET: all levels restored (push+call+custom vtable[0-35])");
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
        int changed = (call_target != g_levels[i].ctor_addr);
        /* Check all 36 vtable slots for changes */
        if (g_vtable_saved) {
            for (int s = 0; s < VTABLE_ENTRIES && !changed; s++) {
                if (read_custom_vtable_entry(i+1, s) != g_orig_vtable[i][s])
                    changed = 1;
            }
        }
        diag_logf("  L%-2d: size=0x%04X ctor=%s%s",
            g_levels[i].level_num, push_val, ctor_name,
            changed ? " *** CHANGED ***" : "");
    }
    diag_log("========================================");
}

/* ============================================================
 * DLL Entry Point
 * ============================================================ */

static void mkn_init(void) {
    find_config_path();

    /* Initialize custom vtables BEFORE parsing config */
    init_custom_vtables();

    if (!g_vtables_initialized) {
        diag_log("[mkn_level_system v3] FATAL: custom vtable init failed!");
        return;
    }

    diag_log("[mkn_level_system v3] Custom vtables allocated and ctors patched");
    for (int i = 0; i < 15; i++) {
        diag_logf("  L%d (%s): custom_vtable=0x%08X (orig=0x%08X, ctor_patch=0x%08X)",
            g_levels[i].level_num, g_levels[i].name,
            g_custom_vtables[i], g_levels[i].vtable_addr,
            g_levels[i].ctor_vt_patch);
    }

    apply_config();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        mkn_init();
    }
    return TRUE;
}

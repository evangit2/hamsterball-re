/*
 * mkn_level_system — Level Setup Function Swapper
 *
 * Reads mkn_level_system.txt config section to swap/insert per-level
 * special logic by patching the game's level data table.
 *
 * Config format (in mkn_level_system.txt, after CONFIG SECTION header):
 *   SET <level_number> <setup_function_name>
 *   SWAP <level_A> <level_B>
 *
 * Examples:
 *   SET 3 Scene_LoadLevel4     (Intermediate gets Dizzy's water ripple setup)
 *   SWAP 5 6                   (Tower and Up swap setup functions)
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
 * Level data table addresses — where the game stores setup
 * function pointers for each level slot.
 * ============================================================ */

typedef struct {
    int level_num;
    const char* name;
    DWORD func_addr;     /* address of the setup function */
    DWORD table_addr;    /* address in .data where the pointer is stored */
} LevelEntry;

/* Corrected mapping (race order → level file → setup function) */
static const LevelEntry g_levels[15] = {
    { 1,  "GetLevelPath",              0x0040D1C0, 0x004D04F0 }, /* Warm-Up    -> level1         */
    { 2,  "Scene_SetupLevelCascade",   0x004110D0, 0x004D10E0 }, /* Beginner    -> levelcascade   */
    { 3,  "Scene_LoadLevel2",          0x0040D280, 0x004D05E8 }, /* Intermediate-> level2         */
    { 4,  "Scene_LoadLevel3",          0x0040D390, 0x004D08D8 }, /* Dizzy       -> level3         */
    { 5,  "Scene_LoadLevel4",          0x0040D6D0, 0x004D0A50 }, /* Tower       -> level4         */
    { 6,  "Scene_SetupLevelUp",         0x00411540, 0x004D11E8 }, /* Up          -> levelup        */
    { 7,  "Scene_SetupLevelDark",       0x00416270, 0x004D1E38 }, /* Neon        -> leveldark      */
    { 8,  "Scene_SetupLevel5",          0x0040E190, 0x004D0B48 }, /* Expert      -> level5         */
    { 9,  "Scene_SetupLevel6",          0x0040EA90, 0x004D0C08 }, /* Odd         -> level6         */
    { 10, "CreateBumper",              0x0040FA20, 0x004D0EC0 }, /* Toob        -> level8         */
    { 11, "Scene_SetupLevel7",          0x0040F360, 0x004D0D80 }, /* Wobbly      -> level7         */
    { 12, "FUN_00417640",               0x00417640, 0x004D1FD8 }, /* Glass       -> levelglass     */
    { 13, "Scene_SetupLevel9",          0x00410830, 0x004D1010 }, /* Sky         -> level9         */
    { 14, "Scene_SetupLevel10",         0x00411F60, 0x004D12F8 }, /* Master      -> level10        */
    { 15, "FUN_00417F20",               0x00417F20, 0x004D2208 }, /* Impossible  -> levelimpossible*/
};

/* Function name lookup — maps any setup function name to its address */
static DWORD lookup_func_addr(const char* name) {
    for (int i = 0; i < 15; i++) {
        if (_stricmp(g_levels[i].name, name) == 0)
            return g_levels[i].func_addr;
    }
    return 0;
}

/* ============================================================
 * Level dispatch — the switch jump table in Tournament_AdvanceRace
 *
 * At 0x00427102: JMP [EAX*4 + 0x42761C]
 * The jump table at 0x42761C has 15 entries (one per race index 0-14).
 * Each entry points to the case body that calls operator_new + LevelBoard_*_ctor.
 *
 * Swapping entries in THIS table swaps which board class gets created,
 * which swaps EVERYTHING: setup function, objects, collision, render, etc.
 * ============================================================ */

#define SWITCH_JUMPTABLE_ADDR 0x0042761C
#define NUM_LEVELS 15

/* Swap two 4-byte entries in the switch jump table */
static void swap_switch_entries(int idx_a, int idx_b) {
    if (idx_a < 0 || idx_a >= NUM_LEVELS) return;
    if (idx_b < 0 || idx_b >= NUM_LEVELS) return;
    DWORD addr_a = SWITCH_JUMPTABLE_ADDR + idx_a * 4;
    DWORD addr_b = SWITCH_JUMPTABLE_ADDR + idx_b * 4;
    DWORD val_a = 0, val_b = 0;
    if (IsBadReadPtr((void*)addr_a, 4)) return;
    if (IsBadReadPtr((void*)addr_b, 4)) return;
    val_a = *(DWORD*)addr_a;
    val_b = *(DWORD*)addr_b;
    patch_dword(addr_a, val_b);
    patch_dword(addr_b, val_a);
}

/* Get current jump table entry */
static DWORD get_switch_entry(int idx) {
    if (idx < 0 || idx >= NUM_LEVELS) return 0;
    DWORD addr = SWITCH_JUMPTABLE_ADDR + idx * 4;
    if (IsBadReadPtr((void*)addr, 4)) return 0;
    return *(DWORD*)addr;
}

/* ============================================================
 * Diagnostic logging — writes to mkn_level_system.log next to DLL
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
 * Config file parser
 * ============================================================ */

static char g_configPath[MAX_PATH] = "";

static void find_config_path(void) {
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&find_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_configPath, MAX_PATH);

    /* Derive log path from DLL path */
    strcpy(g_logPath, g_configPath);
    char* p = strrchr(g_logPath, '\\');
    if (p) {
        strcpy(p + 1, "mkn_level_system.log");
    } else {
        strcpy(g_logPath, "mkn_level_system.log");
    }

    /* Derive config path from DLL path */
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

static void apply_config(void) {
    HANDLE hFile = CreateFileA(g_configPath,
        GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        diag_log("[mkn_level_system] Config file not found, no changes applied");
        return;
    }

    /* Read entire file (max 32KB) */
    char buf[32768];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    diag_log("========================================");
    diag_log("[mkn_level_system] Config file loaded, parsing...");
    diag_logf("Config path: %s", g_configPath);

    /* Dump current switch jump table state BEFORE changes */
    diag_log("--- Switch jump table BEFORE config ---");
    for (int i = 0; i < NUM_LEVELS; i++) {
        DWORD val = get_switch_entry(i);
        diag_logf("  [%2d] 0x%08X", i, val);
    }

    /* Parse line by line */
    char* line = strtok(buf, "\n");
    int in_config = 0;
    int changes = 0;

    while (line) {
        trim(line);

        /* Check for CONFIG SECTION / END CONFIG markers (even in comments) */
        if (strstr(line, "CONFIG SECTION")) {
            in_config = 1;
            line = strtok(NULL, "\n");
            continue;
        }
        if (strstr(line, "END CONFIG")) {
            break;
        }

        /* Skip empty lines and comments */
        if (line[0] == '\0' || line[0] == '#') {
            line = strtok(NULL, "\n");
            continue;
        }

        if (!in_config) {
            line = strtok(NULL, "\n");
            continue;
        }

        /* Parse SET <level_num> <func_name>
         * SET is not supported with the switch table approach — use SWAP instead.
         * (SET would require patching the jump table to point to arbitrary code,
         *  which is unsafe without a code cave.) */
        if (_strnicmp(line, "SET ", 4) == 0) {
            diag_log("SET is not supported in switch-table mode. Use SWAP instead.");
        }
        /* Parse SWAP <levelA> <levelB>
         * Swaps two entries in the switch jump table at 0x42761C.
         * This swaps which LevelBoard_*_ctor runs for each race index,
         * which swaps the ENTIRE level: board class, objects, collision, etc. */
        else if (_strnicmp(line, "SWAP ", 5) == 0) {
            int a = 0, b = 0;
            if (sscanf(line + 5, "%d %d", &a, &b) == 2) {
                /* Convert 1-based to 0-based */
                int idx_a = a - 1;
                int idx_b = b - 1;
                if (idx_a >= 0 && idx_a < NUM_LEVELS && idx_b >= 0 && idx_b < NUM_LEVELS) {
                    DWORD before_a = get_switch_entry(idx_a);
                    DWORD before_b = get_switch_entry(idx_b);
                    swap_switch_entries(idx_a, idx_b);
                    changes += 2;
                    diag_logf("SWAP level %d <-> level %d (switch jump table)", a, b);
                    diag_logf("  [%d] 0x%08X -> 0x%08X", idx_a, before_a, before_b);
                    diag_logf("  [%d] 0x%08X -> 0x%08X", idx_b, before_b, before_a);
                }
            }
        }

        line = strtok(NULL, "\n");
    }

    if (changes > 0) {
        diag_logf("Applied %d config changes", changes);
    } else {
        diag_log("No config changes (defaults)");
    }

    /* Dump switch jump table state AFTER changes */
    diag_log("--- Switch jump table AFTER config ---");
    for (int i = 0; i < NUM_LEVELS; i++) {
        DWORD val = get_switch_entry(i);
        diag_logf("  [%2d] 0x%08X", i, val);
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

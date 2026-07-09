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

/* Get the current function pointer stored in a level's data table slot */
static DWORD get_table_ptr(int level_num) {
    if (level_num < 1 || level_num > 15) return 0;
    DWORD addr = g_levels[level_num - 1].table_addr;
    if (IsBadReadPtr((void*)addr, 4)) return 0;
    return *(DWORD*)addr;
}

/* Write a function pointer into a level's data table slot */
static void set_table_ptr(int level_num, DWORD func_addr) {
    if (level_num < 1 || level_num > 15) return;
    DWORD addr = g_levels[level_num - 1].table_addr;
    patch_dword(addr, func_addr);
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

    /* Dump current table state BEFORE changes */
    diag_log("--- Table state BEFORE config ---");
    for (int i = 0; i < 15; i++) {
        DWORD table_addr = g_levels[i].table_addr;
        DWORD current = 0;
        if (!IsBadReadPtr((void*)table_addr, 4))
            current = *(DWORD*)table_addr;
        diag_logf("  Level %2d: table[0x%08X] = 0x%08X (expected 0x%08X %s)%s",
            g_levels[i].level_num, table_addr, current,
            g_levels[i].func_addr, g_levels[i].name,
            (current == g_levels[i].func_addr) ? "" : " *** MISMATCH ***");
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

        /* Parse SET <level_num> <func_name> */
        if (_strnicmp(line, "SET ", 4) == 0) {
            int level_num = 0;
            char func_name[128] = "";
            if (sscanf(line + 4, "%d %127s", &level_num, func_name) == 2) {
                DWORD addr = lookup_func_addr(func_name);
                if (addr && level_num >= 1 && level_num <= 15) {
                    set_table_ptr(level_num, addr);
                    changes++;
                    diag_logf("SET level %d -> %s (0x%08X)", level_num, func_name, addr);
                } else if (!addr) {
                    diag_logf("WARNING: unknown function '%s'", func_name);
                }
            }
        }
        /* Parse SWAP <levelA> <levelB> */
        else if (_strnicmp(line, "SWAP ", 5) == 0) {
            int a = 0, b = 0;
            if (sscanf(line + 5, "%d %d", &a, &b) == 2) {
                if (a >= 1 && a <= 15 && b >= 1 && b <= 15) {
                    DWORD ptr_a = get_table_ptr(a);
                    DWORD ptr_b = get_table_ptr(b);
                    if (ptr_a && ptr_b) {
                        set_table_ptr(a, ptr_b);
                        set_table_ptr(b, ptr_a);
                        changes += 2;
                        diag_logf("SWAP level %d <-> level %d (0x%08X <-> 0x%08X)", a, b, ptr_a, ptr_b);
                    }
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

    /* Dump table state AFTER changes */
    diag_log("--- Table state AFTER config ---");
    for (int i = 0; i < 15; i++) {
        DWORD table_addr = g_levels[i].table_addr;
        DWORD current = 0;
        if (!IsBadReadPtr((void*)table_addr, 4))
            current = *(DWORD*)table_addr;
        const char* match_name = "???";
        for (int j = 0; j < 15; j++) {
            if (g_levels[j].func_addr == current) {
                match_name = g_levels[j].name;
                break;
            }
        }
        diag_logf("  Level %2d: table[0x%08X] = 0x%08X (%s)%s",
            g_levels[i].level_num, table_addr, current, match_name,
            (current == g_levels[i].func_addr) ? "" : " *** CHANGED ***");
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

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

static const LevelEntry g_levels[15] = {
    { 1,  "GetLevelPath",              0x0040D1C0, 0x004D04F0 },
    { 2,  "Scene_LoadLevel2",          0x0040D280, 0x004D05E8 },
    { 3,  "Scene_LoadLevel3",          0x0040D390, 0x004D08D8 },
    { 4,  "Scene_LoadLevel4",          0x0040D6D0, 0x004D0A50 },
    { 5,  "Scene_SetupLevel5",         0x0040E190, 0x004D0B48 },
    { 6,  "Scene_SetupLevel6",          0x0040EA90, 0x004D0C08 },
    { 7,  "Scene_SetupLevel7",          0x0040F360, 0x004D0D80 },
    { 8,  "CreateBumper",              0x0040FA20, 0x004D0EC0 },
    { 9,  "Scene_SetupLevel9",          0x00410830, 0x004D1010 },
    { 10, "Scene_SetupLevelCascade",    0x004110D0, 0x004D10E0 },
    { 11, "Scene_SetupLevelUp",         0x00411540, 0x004D11E8 },
    { 12, "Scene_SetupLevel10",         0x00411F60, 0x004D12F8 },
    { 13, "Scene_SetupLevelDark",       0x00416270, 0x004D1E38 },
    { 14, "FUN_00417640",               0x00417640, 0x004D1FD8 },
    { 15, "FUN_00417F20",               0x00417F20, 0x004D2208 },
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
 * Config file parser
 * ============================================================ */

static char g_configPath[MAX_PATH] = "";

static void find_config_path(void) {
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&find_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_configPath, MAX_PATH);
    char* p = strrchr(g_configPath, '\\');
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
    if (hFile == INVALID_HANDLE_VALUE) return;

    /* Read entire file (max 32KB) */
    char buf[32768];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    /* Parse line by line */
    char* line = strtok(buf, "\n");
    int in_config = 0;
    int changes = 0;

    while (line) {
        trim(line);

        /* Skip empty lines and comments */
        if (line[0] == '\0' || line[0] == '#') {
            line = strtok(NULL, "\n");
            continue;
        }

        /* Look for CONFIG SECTION marker */
        if (_strnicmp(line, "CONFIG SECTION", 14) == 0) {
            in_config = 1;
            line = strtok(NULL, "\n");
            continue;
        }
        /* Stop at END CONFIG */
        if (_strnicmp(line, "END CONFIG", 10) == 0) {
            break;
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
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                        "[mkn_level_system] SET level %d -> %s (0x%08X)\n",
                        level_num, func_name, addr);
                    OutputDebugStringA(msg);
                } else if (!addr) {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                        "[mkn_level_system] WARNING: unknown function '%s'\n",
                        func_name);
                    OutputDebugStringA(msg);
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
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                            "[mkn_level_system] SWAP level %d <-> level %d\n", a, b);
                        OutputDebugStringA(msg);
                    }
                }
            }
        }

        line = strtok(NULL, "\n");
    }

    if (changes > 0) {
        char msg[128];
        snprintf(msg, sizeof(msg),
            "[mkn_level_system] Applied %d config changes\n", changes);
        OutputDebugStringA(msg);
    } else {
        OutputDebugStringA("[mkn_level_system] No config changes (defaults)\n");
    }
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

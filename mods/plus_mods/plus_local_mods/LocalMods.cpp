/*
 * LocalMods.cpp — Per-Level/Arena DLL Mod Loader for Hamsterball (HB+ API v2.0)
 *
 * Uses manual vtable construction (17-entry MSVC layout) for MinGW compatibility.
 * See skill: hamsterball-plus-modding references/mingw-vtable-fix.md
 *
 * Author: Hamsterbot
 */
#include <windows.h>
#include "HamsterballAPI.h"
#include "nocrt.h"
#include "hbplus_api.h"

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncpy nc_strncpy
#define strcpy nc_strcpy
#define strcat nc_strcat
#define snprintf nc_snprintf

// ============================================================================
// Constants
// ============================================================================

static constexpr int NUM_SLOTS = 15;
static constexpr int MAX_MODS_PER_SLOT = 16;
static constexpr int MAX_MOD_NAME_LEN = 128;
static constexpr int MAX_LOADED_MODS = 64;

static constexpr DWORD GLOBAL_SCENE_PTR = 0x005341E4;
static constexpr DWORD SCENE_NAME_OFFSET = 0x868;

static const char* RACE_NAMES[NUM_SLOTS] = {
    "Board (Warm-Up)",
    "Board (Beginner)",
    "Board (Intermediate)",
    "Board (Dizzy)",
    "Board (Tower)",
    "Board (Up)",
    "Board (Dark)",
    "Board (Expert)",
    "Board (Odd)",
    "Board (Toob)",
    "Board (Wobbly)",
    "Board (Glass)",
    "Board (Sky)",
    "Board (Master)",
    "Board (Impossible)"
};

static const char* ARENA_NAMES[NUM_SLOTS] = {
    "RumbleBoard (Warmup Arena)",
    "RumbleBoard (Beginner Arena)",
    "RumbleBoard (Intermediate Arena)",
    "RumbleBoard (Dizzy Arena)",
    "RumbleBoard (Tower Arena)",
    "RumbleBoard (Up Arena)",
    "RumbleBoard (Neon Arena)",
    "RumbleBoard (Expert Arena)",
    "RumbleBoard (Odd Arena)",
    "RumbleBoard (Toob Arena)",
    "RumbleBoard (Wobbly Arena)",
    "RumbleBoard (Sky Arena)",
    "RumbleBoard (Master Arena)",
    "RumbleBoard (Glass Arena)",
    "RumbleBoard (Impossible Arena)"
};

// ============================================================================
// Data Structures
// ============================================================================

struct ModEntry {
    char name[MAX_MOD_NAME_LEN];
};

struct SlotConfig {
    ModEntry mods[MAX_MODS_PER_SLOT];
    int count;
};

struct LoadedMod {
    HMODULE dllHandle;
    HamsterballAPI* instance;
    CreateModFunct createFunc;
    char name[MAX_MOD_NAME_LEN];
    bool active;
};

// ============================================================================
// State
// ============================================================================

static IModAPI* g_api = NULL;
static void* g_modObj = NULL;

static char g_gameDir[MAX_PATH];
static char g_localModsPath[MAX_PATH];
static char g_configPath[MAX_PATH];

static SlotConfig g_levelConfigs[NUM_SLOTS * 2];
static LoadedMod g_loadedMods[MAX_LOADED_MODS];
static int g_loadedCount = 0;
static int g_currentLevelIndex = -1;

// ============================================================================
// Path Setup
// ============================================================================

static void buildPaths() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char* lastSlash = strrchr(exePath, '\\');
    if (!lastSlash) lastSlash = strrchr(exePath, '/');

    if (lastSlash) {
        *(lastSlash + 1) = '\0';
        strcpy(g_gameDir, exePath);
        snprintf(g_localModsPath, MAX_PATH, "%sLocalmods\\", exePath);
        snprintf(g_configPath, MAX_PATH, "%slocal_mods.txt", exePath);
    } else {
        strcpy(g_gameDir, ".\\");
        strcpy(g_localModsPath, "Localmods\\");
        strcpy(g_configPath, "local_mods.txt");
    }
}

// ============================================================================
// Level Identification
// ============================================================================

static int identifyLevel() {
    if (IsBadReadPtr((void*)GLOBAL_SCENE_PTR, 4)) return -1;
    DWORD scene = *(DWORD*)GLOBAL_SCENE_PTR;
    if (!scene || scene < 0x10000) return -1;
    if (IsBadReadPtr((void*)(scene + SCENE_NAME_OFFSET), 4)) return -1;
    const char* name = *(const char**)(scene + SCENE_NAME_OFFSET);
    if (!name || IsBadReadPtr(name, 2)) return -1;
    if ((unsigned char)name[0] < 0x20 || (unsigned char)name[0] > 0x7E) return -1;

    for (int i = 0; i < NUM_SLOTS; i++) {
        if (strcmp(name, RACE_NAMES[i]) == 0) return i;
    }
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (strcmp(name, ARENA_NAMES[i]) == 0) return NUM_SLOTS + i;
    }
    return -1;
}

// ============================================================================
// Config File Parsing
// ============================================================================

static void createDefaultConfig() {
    HANDLE hFile = CreateFileA(g_configPath, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    DWORD written;
    const char* header =
        "==================== HB+  |  LOCAL MODS ====================\r\n"
        "\r\n"
        "LEVELS\r\n";
    WriteFile(hFile, header, (DWORD)strlen(header), &written, NULL);

    for (int i = 1; i <= NUM_SLOTS; i++) {
        char line[32];
        snprintf(line, sizeof(line), "%d = \r\n", i);
        WriteFile(hFile, line, (DWORD)strlen(line), &written, NULL);
    }

    const char* arenaHeader = "\r\nARENAS\r\n";
    WriteFile(hFile, arenaHeader, (DWORD)strlen(arenaHeader), &written, NULL);

    for (int i = 1; i <= NUM_SLOTS; i++) {
        char line[32];
        snprintf(line, sizeof(line), "%d = \r\n", i);
        WriteFile(hFile, line, (DWORD)strlen(line), &written, NULL);
    }

    const char* footer = "\r\n============================================================\r\n";
    WriteFile(hFile, footer, (DWORD)strlen(footer), &written, NULL);
    CloseHandle(hFile);
}

static void parseSlotLine(char* line, int section) {
    char* eq = strchr(line, '=');
    if (!eq) return;

    // Parse slot number
    char numBuf[16];
    char* p = line;
    int numIdx = 0;
    while (p < eq && *p && numIdx < 15) {
        if (*p >= '0' && *p <= '9') {
            numBuf[numIdx++] = *p;
        }
        p++;
    }
    numBuf[numIdx] = '\0';

    int slotNum = 0;
    for (int i = 0; numBuf[i]; i++) {
        slotNum = slotNum * 10 + (numBuf[i] - '0');
    }

    if (slotNum < 1 || slotNum > NUM_SLOTS) return;

    int slotIdx = (section == 1) ? (slotNum - 1) : (NUM_SLOTS + slotNum - 1);
    SlotConfig* cfg = &g_levelConfigs[slotIdx];
    cfg->count = 0;

    // Parse content after '='
    char* content = eq + 1;
    while (*content == ' ' || *content == '\t') content++;

    if (*content == '(') content++;
    char* endParen = strrchr(content, ')');
    if (endParen) *endParen = '\0';

    if (!*content) return;

    // Tokenize by comma — use nc_strtok here (safe: outer loop uses manual split)
    char* tok = nc_strtok(content, ",");
    while (tok && cfg->count < MAX_MODS_PER_SLOT) {
        while (*tok == ' ' || *tok == '\t') tok++;
        char* end = tok + strlen(tok);
        while (end > tok && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
            end--;
        }
        *end = '\0';

        if (*tok) {
            strncpy(cfg->mods[cfg->count].name, tok, MAX_MOD_NAME_LEN - 1);
            cfg->mods[cfg->count].name[MAX_MOD_NAME_LEN - 1] = '\0';
            cfg->count++;
        }
        tok = nc_strtok(NULL, ",");
    }
}

static void parseConfigBuffer(char* buffer) {
    int section = 0;

    // Manual line splitter — do NOT use nc_strtok here because
    // parseSlotLine also calls nc_strtok (single static pointer).
    char* pos = buffer;
    while (pos && *pos) {
        char* lineEnd = pos;
        while (*lineEnd && *lineEnd != '\r' && *lineEnd != '\n') lineEnd++;
        char saved = *lineEnd;
        *lineEnd = '\0';
        char* line = pos;

        while (*line == ' ' || *line == '\t') line++;

        if (*line != '\0' && *line != '#' && *line != '=') {
            if (nc_stricmp(line, "LEVELS") == 0) {
                section = 1;
            } else if (nc_stricmp(line, "ARENAS") == 0) {
                section = 2;
            } else if (nc_stricmp(line, "============================================================") == 0) {
                // footer
            } else if (strstr(line, "HB+") && strstr(line, "LOCAL MODS")) {
                // header
            } else if (section == 1 || section == 2) {
                parseSlotLine(line, section);
            }
        }

        if (saved == '\0') break;
        pos = lineEnd + 1;
        if (*pos == '\n') pos++;
    }
}

static void parseConfigFile() {
    for (int i = 0; i < NUM_SLOTS * 2; i++) {
        g_levelConfigs[i].count = 0;
        memset(g_levelConfigs[i].mods, 0, sizeof(g_levelConfigs[i].mods));
    }

    HANDLE hFile = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        createDefaultConfig();
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize > 65536) {
        CloseHandle(hFile);
        return;
    }

    char* buffer = (char*)nc_malloc(fileSize + 1);
    if (!buffer) {
        CloseHandle(hFile);
        return;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL) || bytesRead == 0) {
        nc_free(buffer);
        CloseHandle(hFile);
        return;
    }
    buffer[bytesRead] = '\0';
    CloseHandle(hFile);

    parseConfigBuffer(buffer);
    nc_free(buffer);
}

// ============================================================================
// DLL Loading / Unloading
// ============================================================================

static int findLoadedMod(const char* name) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (strcmp(g_loadedMods[i].name, name) == 0) return i;
    }
    return -1;
}

static bool loadMod(const char* dllName) {
    if (g_loadedCount >= MAX_LOADED_MODS) return false;

    // Append .dll if missing
    char fileName[MAX_MOD_NAME_LEN + 8];
    strncpy(fileName, dllName, MAX_MOD_NAME_LEN);
    fileName[MAX_MOD_NAME_LEN - 1] = '\0';
    size_t nameLen = strlen(fileName);
    if (nameLen >= 4 && nc_stricmp(fileName + nameLen - 4, ".dll") == 0) {
        // already has .dll
    } else {
        strcat(fileName, ".dll");
    }

    char fullPath[MAX_PATH * 2];
    snprintf(fullPath, sizeof(fullPath), "%s%s", g_localModsPath, fileName);

    HMODULE h = LoadLibraryA(fullPath);
    if (!h) return false;

    CreateModFunct createFunc = (CreateModFunct)GetProcAddress(h, "CreateModInstance");
    if (!createFunc) {
        FreeLibrary(h);
        return false;
    }

    HamsterballAPI* instance = createFunc();
    if (!instance) {
        FreeLibrary(h);
        return false;
    }

    // Initialize the mod (pass g_api from HB+)
    instance->Initialize(g_api);

    LoadedMod* lm = &g_loadedMods[g_loadedCount];
    lm->dllHandle = h;
    lm->instance = instance;
    lm->createFunc = createFunc;
    lm->active = true;
    strncpy(lm->name, dllName, MAX_MOD_NAME_LEN);
    lm->name[MAX_MOD_NAME_LEN - 1] = '\0';
    g_loadedCount++;

    return true;
}

static void unloadMod(int index) {
    if (index < 0 || index >= g_loadedCount) return;

    LoadedMod* lm = &g_loadedMods[index];

    if (lm->active && lm->instance) {
        lm->instance->onSceneEnd();
    }

    if (lm->instance) {
        delete lm->instance;
        lm->instance = NULL;
    }

    if (lm->dllHandle) {
        FreeLibrary(lm->dllHandle);
        lm->dllHandle = NULL;
    }

    lm->active = false;
    lm->name[0] = '\0';

    for (int i = index; i < g_loadedCount - 1; i++) {
        g_loadedMods[i] = g_loadedMods[i + 1];
    }
    g_loadedCount--;
}

static void unloadInactiveMods(int currentSlot) {
    SlotConfig* cfg = &g_levelConfigs[currentSlot];
    bool needed[MAX_LOADED_MODS];
    for (int i = 0; i < MAX_LOADED_MODS; i++) needed[i] = false;

    for (int i = 0; i < g_loadedCount; i++) {
        bool isNeeded = false;
        for (int j = 0; j < cfg->count; j++) {
            if (strcmp(g_loadedMods[i].name, cfg->mods[j].name) == 0) {
                isNeeded = true;
                break;
            }
        }
        needed[i] = isNeeded;
    }

    for (int i = g_loadedCount - 1; i >= 0; i--) {
        if (!needed[i]) unloadMod(i);
    }
}

static void unloadAllMods() {
    for (int i = g_loadedCount - 1; i >= 0; i--) {
        unloadMod(i);
    }
}

// ============================================================================
// Callback implementations (free functions, referenced by manual vtable)
// ============================================================================

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    g_modObj = thisptr;

    g_loadedCount = 0;
    g_currentLevelIndex = -1;
    memset(g_loadedMods, 0, sizeof(g_loadedMods));

    buildPaths();
    parseConfigFile();
}

static void __thiscall level_start_impl(void* thisptr) {
    parseConfigFile();
    g_currentLevelIndex = identifyLevel();

    if (g_currentLevelIndex < 0) return;

    int slot = g_currentLevelIndex;
    SlotConfig* cfg = &g_levelConfigs[slot];

    unloadInactiveMods(slot);

    for (int i = 0; i < cfg->count; i++) {
        const char* modName = cfg->mods[i].name;
        if (!modName[0]) continue;
        if (findLoadedMod(modName) >= 0) continue;
        loadMod(modName);
    }

    // Call onLevelStart on all active loaded mods
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onLevelStart();
        }
    }
}

static void __thiscall scene_end_impl(void* thisptr) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onSceneEnd();
        }
    }
    unloadAllMods();
    g_currentLevelIndex = -1;
}

static void __thiscall ball_update_impl(void* thisptr, Ball* ball) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onBallUpdate(ball);
        }
    }
}

static void __thiscall game_update_impl(void* thisptr) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onGameUpdate();
        }
    }
}

static void __thiscall text_render_impl(void* thisptr) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onTextRenderLoop();
        }
    }
}

static void __thiscall button_toggle_impl(void* thisptr, const char* buttonId, bool newState) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onButtonToggle(buttonId, newState);
        }
    }
}

static void __thiscall slider_change_impl(void* thisptr, const char* sliderId, float newValue) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onSliderChange(sliderId, newValue);
        }
    }
}

static void __thiscall event_collide_impl(void* thisptr, Ball* ball, char* eventPlaneID) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onEventPlaneCollide(ball, eventPlaneID);
        }
    }
}

static void __thiscall ball_bump_impl(void* thisptr, Ball* ball1, Ball* ball2) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onBallBump(ball1, ball2);
        }
    }
}

static void __thiscall render_apply_impl(void* thisptr, void* this_ptr, float* viewMatrix) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onRenderApply(this_ptr, viewMatrix);
        }
    }
}

static void __thiscall cycle_option_impl(void* thisptr, const char* cycleId, const char* newOption) {
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedMods[i].active && g_loadedMods[i].instance) {
            g_loadedMods[i].instance->onCycleOptionChange(cycleId, newOption);
        }
    }
}

// ============================================================================
// Manual Vtable (17-entry MSVC layout — v2.0 adds onCycleOptionChange at slot 10)
// ============================================================================

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    // Cleanup on unload
    unloadAllMods();
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Local Mods"; }
static const char* __thiscall get_author(void*) { return "Hamsterbot"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return ""; }

static void* g_vtable[17] = {
    (void*)sc_dtor,             // [0]  scalar deleting destructor
    (void*)get_mod_name,        // [1]  GetModName
    (void*)get_author,          // [2]  GetAuthorName
    (void*)get_version,         // [3]  GetApiVersion
    (void*)get_contributors,    // [4]  GetContributors
    (void*)init_impl,           // [5]  Initialize
    (void*)ball_update_impl,    // [6]  onBallUpdate
    (void*)render_apply_impl,   // [7]  onRenderApply
    (void*)button_toggle_impl,  // [8]  onButtonToggle
    (void*)slider_change_impl,  // [9]  onSliderChange
    (void*)cycle_option_impl,   // [10] onCycleOptionChange (v2.0)
    (void*)game_update_impl,    // [11] onGameUpdate
    (void*)event_collide_impl,  // [12] onEventPlaneCollide
    (void*)text_render_impl,    // [13] onTextRenderLoop
    (void*)ball_bump_impl,      // [14] onBallBump
    (void*)scene_end_impl,      // [15] onSceneEnd
    (void*)level_start_impl,    // [16] onLevelStart
};

// ============================================================================
// Export
// ============================================================================

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);  // 8 bytes: vtable ptr + IModAPI* member
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

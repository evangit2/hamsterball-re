/*
 * LocalMods.cpp — Per-Level/Arena DLL Mod Loader for Hamsterball (HB+ API v2.0)
 *
 * LoadOrder: This mod must be FIRST in the Mods\ folder load order so it can
 * load other mods before their Initialize() runs. Name the file so it sorts
 * first alphabetically, or ensure it's the first DLL HB+ picks up.
 *
 * HOW IT WORKS:
 *   1. On startup, reads local_mods.txt from the game root folder.
 *   2. Parses LEVELS (1-15) and ARENAS (1-15) sections, each listing
 *      mod DLL names to activate for that level/arena slot.
 *   3. On onLevelStart(), identifies the current level/arena via scene name.
 *   4. Loads (LoadLibraryA) the appropriate DLLs from \Localmods\ folder.
 *      Unloads any DLLs from the previous level that aren't needed.
 *   5. Each loaded DLL exports CreateModInstance() returning a HamsterballAPI*.
 *      The loader calls Initialize() on each, then onLevelStart().
 *   6. Forwards onBallUpdate(), onGameUpdate(), onSceneEnd() to loaded mods.
 *
 * LEVEL INDEX MAPPING:
 *   Races:  1=Warm-Up, 2=Beginner, 3=Intermediate, 4=Dizzy, 5=Tower,
 *           6=Up, 7=Neon(Dark), 8=Expert, 9=Odd, 10=Toob,
 *           11=Wobbly, 12=Glass, 13=Sky, 14=Master, 15=Impossible
 *   Arenas: 1=Warm-Up, 2=Beginner, 3=Intermediate, 4=Dizzy, 5=Tower,
 *           6=Up, 7=Neon, 8=Expert, 9=Odd, 10=Toob,
 *           11=Wobbly, 12=Sky, 13=Master, 14=Glass, 15=Impossible
 *
 * Build (MinGW): see build.sh
 * Build (VS): compile as 32-bit DLL, place in Mods\ folder.
 *
 * Author: Hamsterbot
 */
#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include "nocrt.h"
#include <windows.h>

// ============================================================================
// Constants
// ============================================================================

static constexpr int NUM_SLOTS = 15;
static constexpr int MAX_MODS_PER_SLOT = 16;
static constexpr int MAX_MOD_NAME_LEN = 128;
static constexpr int MAX_LOADED_MODS = 64;

// Global scene pointer address (direct memory access, bypasses IModAPI vtable)
static constexpr DWORD GLOBAL_SCENE_PTR = 0x005341E4;

// Scene struct offsets
static constexpr DWORD SCENE_NAME_OFFSET = 0x868;

// Race scene names (as stored in Scene->name)
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

// Arena scene names
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
    char name[MAX_MOD_NAME_LEN];  // DLL filename (e.g. "mymod.dll")
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
// Mod Class
// ============================================================================

class LocalModsLoader : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    char gameDir[MAX_PATH];
    char localModsPath[MAX_PATH];   // gameDir/Localmods/
    char configPath[MAX_PATH];      // gameDir/local_mods.txt


    // Parsed config: [0..14] = races, [15..29] = arenas
    SlotConfig levelConfigs[NUM_SLOTS * 2];

    // Currently loaded mods
    LoadedMod loadedMods[MAX_LOADED_MODS];
    int loadedCount;

    // Current level identification
    int currentLevelIndex;   // 0-14 = race, 15-29 = arena, -1 = unknown/menu
    bool initialized;

public:
    const char* GetModName() override    { return "Local Mods"; }
    const char* GetAuthorName() override  { return "Hamsterbot"; }
    int GetApiVersion() override          { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        loadedCount = 0;
        currentLevelIndex = -1;
        initialized = false;

        // Zero out configs
        nc_memset(levelConfigs, 0, sizeof(levelConfigs));
        nc_memset(loadedMods, 0, sizeof(loadedMods));

        // Build paths
        buildPaths();

        // Parse config file
        parseConfigFile();

        initialized = true;
    }

    void onLevelStart() override {
        // Re-read config in case user edited it
        parseConfigFile();

        // Identify level
        currentLevelIndex = identifyLevel();

        if (currentLevelIndex < 0) return;

        // Determine which mods should be active for this slot
        int slot = currentLevelIndex;
        SlotConfig* cfg = &levelConfigs[slot];

        // Unload mods that are no longer needed
        unloadInactiveMods(slot);

        // Load new mods for this slot
        for (int i = 0; i < cfg->count; i++) {
            const char* modName = cfg->mods[i].name;
            if (!modName[0]) continue;

            // Check if already loaded
            if (findLoadedMod(modName) >= 0) continue;

            // Load it
            loadMod(modName);
        }

        // Call onLevelStart on all active loaded mods
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onLevelStart();
            }
        }
    }

    void onSceneEnd() override {
        // Notify all loaded mods
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onSceneEnd();
            }
        }

        // Unload all mods when scene ends
        unloadAllMods();

        currentLevelIndex = -1;
    }

    void onBallUpdate(Ball* ball) override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onBallUpdate(ball);
            }
        }
    }

    void onGameUpdate() override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onGameUpdate();
            }
        }
    }

    void onTextRenderLoop() override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onTextRenderLoop();
            }
        }
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onButtonToggle(buttonId, newState);
            }
        }
    }

    void onSliderChange(const char* sliderId, float newValue) override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onSliderChange(sliderId, newValue);
            }
        }
    }

    void onEventPlaneCollide(Ball* ball, char* eventPlaneID) override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onEventPlaneCollide(ball, eventPlaneID);
            }
        }
    }

    void onBallBump(Ball* ball1, Ball* ball2) override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onBallBump(ball1, ball2);
            }
        }
    }

    void onRenderApply(void* this_ptr, float* viewMatrix) override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onRenderApply(this_ptr, viewMatrix);
            }
        }
    }

    void onCycleOptionChange(const char* cycleId, const char* newOption) override {
        for (int i = 0; i < loadedCount; i++) {
            if (loadedMods[i].active && loadedMods[i].instance) {
                loadedMods[i].instance->onCycleOptionChange(cycleId, newOption);
            }
        }
    }

private:
    // ========================================================================
    // Path Setup
    // ========================================================================

    void buildPaths() {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char* lastSlash = strrchr(exePath, '\\');
        if (!lastSlash) lastSlash = strrchr(exePath, '/');

        if (lastSlash) {
            *(lastSlash + 1) = '\0';
            nc_strcpy(gameDir, exePath);
            nc_snprintf(localModsPath, MAX_PATH, "%sLocalmods\\", exePath);
            nc_snprintf(configPath, MAX_PATH, "%slocal_mods.txt", exePath);
        } else {
            nc_strcpy(gameDir, ".\\");
            nc_strcpy(localModsPath, "Localmods\\");
            nc_strcpy(configPath, "local_mods.txt");
        }
    }

    // ========================================================================
    // Level Identification
    // ========================================================================

    int identifyLevel() {
        if (IsBadReadPtr((void*)GLOBAL_SCENE_PTR, 4)) return -1;
        DWORD scene = *(DWORD*)GLOBAL_SCENE_PTR;
        if (!scene || scene < 0x10000) return -1;
        if (IsBadReadPtr((void*)(scene + SCENE_NAME_OFFSET), 4)) return -1;
        const char* name = *(const char**)(scene + SCENE_NAME_OFFSET);
        if (!name || IsBadReadPtr(name, 2)) return -1;
        if ((unsigned char)name[0] < 0x20 || (unsigned char)name[0] > 0x7E) return -1;

        // Check races (index 0-14)
        for (int i = 0; i < NUM_SLOTS; i++) {
            if (strcmp(name, RACE_NAMES[i]) == 0) return i;
        }
        // Check arenas (index 15-29)
        for (int i = 0; i < NUM_SLOTS; i++) {
            if (strcmp(name, ARENA_NAMES[i]) == 0) return NUM_SLOTS + i;
        }
        return -1;
    }

    // ========================================================================
    // Config File Parsing
    // ========================================================================

    void parseConfigFile() {
        // Reset all configs
        for (int i = 0; i < NUM_SLOTS * 2; i++) {
            levelConfigs[i].count = 0;
            nc_memset(levelConfigs[i].mods, 0, sizeof(levelConfigs[i].mods));
        }

        HANDLE hFile = CreateFileA(configPath, GENERIC_READ, FILE_SHARE_READ,
                                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            // Config not found — create default
            createDefaultConfig();
            return;
        }

        // Read entire file into buffer
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

        // Parse the config
        parseConfigBuffer(buffer);
        nc_free(buffer);
    }

    void parseConfigBuffer(char* buffer) {
        // State machine: 0 = looking for section, 1 = in LEVELS, 2 = in ARENAS
        int section = 0;
        char* line = nc_strtok(buffer, "\r\n");

        while (line) {
            // Trim leading whitespace
            while (*line == ' ' || *line == '\t') line++;

            // Skip empty lines, comments, header
            if (*line == '\0' || *line == '#' || *line == '=') {
                line = nc_strtok(NULL, "\r\n");
                continue;
            }

            // Check for section headers
            if (nc_stricmp(line, "LEVELS") == 0) {
                section = 1;
                line = nc_strtok(NULL, "\r\n");
                continue;
            }
            if (nc_stricmp(line, "ARENAS") == 0) {
                section = 2;
                line = nc_strtok(NULL, "\r\n");
                continue;
            }
            if (nc_stricmp(line, "============================================================") == 0) {
                line = nc_strtok(NULL, "\r\n");
                continue;
            }
            if (strstr(line, "HB+") && strstr(line, "LOCAL MODS")) {
                line = nc_strtok(NULL, "\r\n");
                continue;
            }

            // Parse "N = (mod1, mod2, mod3)" lines
            if (section == 1 || section == 2) {
                parseSlotLine(line, section);
            }

            line = nc_strtok(NULL, "\r\n");
        }
    }

    void parseSlotLine(char* line, int section) {
        // Find the '=' sign
        char* eq = strchr(line, '=');
        if (!eq) return;

        // Parse slot number
        // Trim whitespace before '='
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

        // Slot index: section 1 (races) = slotNum-1, section 2 (arenas) = NUM_SLOTS + slotNum-1
        int slotIdx = (section == 1) ? (slotNum - 1) : (NUM_SLOTS + slotNum - 1);
        SlotConfig* cfg = &levelConfigs[slotIdx];

        // Parse content after '=' — may be wrapped in ( )
        char* content = eq + 1;
        while (*content == ' ' || *content == '\t') content++;

        // Remove outer parentheses if present
        if (*content == '(') content++;
        // Trim trailing ')'
        char* endParen = strrchr(content, ')');
        if (endParen) *endParen = '\0';

        // If nothing after =, slot is empty (default)
        if (!*content) return;

        // Tokenize by comma
        char* tok = nc_strtok(content, ",");
        while (tok && cfg->count < MAX_MODS_PER_SLOT) {
            // Trim whitespace
            while (*tok == ' ' || *tok == '\t') tok++;
            // Trim trailing whitespace
            char* end = tok + nc_strlen(tok);
            while (end > tok && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
                end--;
            }
            *end = '\0';

            if (*tok) {
                nc_strncpy(cfg->mods[cfg->count].name, tok, MAX_MOD_NAME_LEN - 1);
                cfg->mods[cfg->count].name[MAX_MOD_NAME_LEN - 1] = '\0';
                cfg->count++;
            }

            tok = nc_strtok(NULL, ",");
        }
    }

    void createDefaultConfig() {
        HANDLE hFile = CreateFileA(configPath, GENERIC_WRITE, 0, NULL,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return;

        DWORD written;

        // Header
        const char* header =
            "==================== HB+  |  LOCAL MODS ====================\r\n"
            "\r\n"
            "LEVELS\r\n";
        WriteFile(hFile, header, (DWORD)nc_strlen(header), &written, NULL);

        // Level slots 1-15
        for (int i = 1; i <= NUM_SLOTS; i++) {
            char line[32];
            nc_snprintf(line, sizeof(line), "%d = \r\n", i);
            WriteFile(hFile, line, (DWORD)nc_strlen(line), &written, NULL);
        }

        // Arena section
        const char* arenaHeader =
            "\r\n"
            "ARENAS\r\n";
        WriteFile(hFile, arenaHeader, (DWORD)nc_strlen(arenaHeader), &written, NULL);

        // Arena slots 1-15
        for (int i = 1; i <= NUM_SLOTS; i++) {
            char line[32];
            nc_snprintf(line, sizeof(line), "%d = \r\n", i);
            WriteFile(hFile, line, (DWORD)nc_strlen(line), &written, NULL);
        }

        // Footer
        const char* footer =
            "\r\n"
            "============================================================\r\n";
        WriteFile(hFile, footer, (DWORD)nc_strlen(footer), &written, NULL);

        CloseHandle(hFile);
    }

    // ========================================================================
    // DLL Loading / Unloading
    // ========================================================================

    int findLoadedMod(const char* name) {
        for (int i = 0; i < loadedCount; i++) {
            if (strcmp(loadedMods[i].name, name) == 0) return i;
        }
        return -1;
    }

    bool loadMod(const char* dllName) {
        if (loadedCount >= MAX_LOADED_MODS) return false;

        // Build full path: <gameDir>\Localmods\<dllName>
        char fullPath[MAX_PATH * 2];
        nc_snprintf(fullPath, sizeof(fullPath), "%s%s", localModsPath, dllName);

        HMODULE h = LoadLibraryA(fullPath);
        if (!h) return false;

        // Get CreateModInstance export
        CreateModFunct createFunc = (CreateModFunct)GetProcAddress(h, "CreateModInstance");
        if (!createFunc) {
            FreeLibrary(h);
            return false;
        }

        // Create mod instance
        HamsterballAPI* instance = createFunc();
        if (!instance) {
            FreeLibrary(h);
            return false;
        }

        // Check API version compatibility
        if (instance->GetApiVersion() != HAMSTERBALL_API_VERSION) {
            // Version mismatch — still try to load, but could be problematic
        }

        // Initialize the mod
        instance->Initialize(api);

        // Register in loaded mods array
        LoadedMod* lm = &loadedMods[loadedCount];
        lm->dllHandle = h;
        lm->instance = instance;
        lm->createFunc = createFunc;
        lm->active = true;
        nc_strncpy(lm->name, dllName, MAX_MOD_NAME_LEN);
        loadedCount++;

        return true;
    }

    void unloadMod(int index) {
        if (index < 0 || index >= loadedCount) return;

        LoadedMod* lm = &loadedMods[index];

        // Notify mod of scene end if active
        if (lm->active && lm->instance) {
            lm->instance->onSceneEnd();
        }

        // Free the mod instance (let its destructor clean up)
        if (lm->instance) {
            delete lm->instance;
            lm->instance = NULL;
        }

        // Unload the DLL
        if (lm->dllHandle) {
            FreeLibrary(lm->dllHandle);
            lm->dllHandle = NULL;
        }

        lm->active = false;
        lm->name[0] = '\0';

        // Shift array down
        for (int i = index; i < loadedCount - 1; i++) {
            loadedMods[i] = loadedMods[i + 1];
        }
        loadedCount--;

        // Zero the vacated slot
        loadedMods[loadedCount].dllHandle = NULL;
        loadedMods[loadedCount].instance = NULL;
        loadedMods[loadedCount].createFunc = NULL;
        loadedMods[loadedCount].active = false;
        loadedMods[loadedCount].name[0] = '\0';
    }

    void unloadInactiveMods(int currentSlot) {
        // Build list of mod names needed for current slot
        SlotConfig* cfg = &levelConfigs[currentSlot];
        bool needed[MAX_LOADED_MODS];
        for (int i = 0; i < MAX_LOADED_MODS; i++) needed[i] = false;

        // For each loaded mod, check if it's in the current slot's config
        for (int i = 0; i < loadedCount; i++) {
            bool isNeeded = false;
            for (int j = 0; j < cfg->count; j++) {
                if (strcmp(loadedMods[i].name, cfg->mods[j].name) == 0) {
                    isNeeded = true;
                    break;
                }
            }
            needed[i] = isNeeded;
        }

        // Unload mods that are no longer needed (iterate backwards)
        for (int i = loadedCount - 1; i >= 0; i--) {
            if (!needed[i]) {
                unloadMod(i);
            }
        }
    }

    void unloadAllMods() {
        for (int i = loadedCount - 1; i >= 0; i--) {
            unloadMod(i);
        }
    }
};

// ============================================================================
// Export
// ============================================================================

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new LocalModsLoader();
}

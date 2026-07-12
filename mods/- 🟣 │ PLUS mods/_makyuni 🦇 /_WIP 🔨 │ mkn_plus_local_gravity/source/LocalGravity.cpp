/*
 * LocalGravity.cpp — Per-Level Gravity Override (HB+ API v2.0)
 *
 * Reads gravity values from mkn_plus_local_gravity_set.txt (30 lines):
 *   Lines 1-15:  Race levels (Warm-Up through Impossible)
 *   Lines 16-30: Arena levels (Warm-Up through Sky)
 *
 * Each line is a single float (e.g. 0.5 = normal, 0.125 = low, 2.0 = heavy).
 * The file is re-read on every level start, so changes take effect immediately.
 *
 * Uses direct memory access (g_Scene at 0x5341E4) for scene name identification
 * to avoid vtable dispatch issues. The file is searched in the game exe
 * directory first, then the current working directory as fallback.
 *
 * Build (Visual Studio): compile as 32-bit DLL, place in Mods\ folder.
 * Build (MinGW): see LocalGravity_MinGW.cpp + nocrt.cpp
 */
#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <windows.h>
#include <cmath>
#include <cstdio>
#include <cstring>

static constexpr int NUM_LEVELS = 30;
static constexpr int NUM_RACES = 15;
static constexpr float DEFAULT_GRAVITY = 0.5f;

/* Global pointers for direct memory access */
static constexpr DWORD GLOBAL_SCENE_PTR = 0x005341E4;

static const char* RACE_NAMES[NUM_RACES] = {
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

static const char* ARENA_NAMES[NUM_RACES] = {
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

class mkn_plus_local_gravity : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    float gravityValues[NUM_LEVELS];
    int currentLevelIndex = -1;
    char configPath[MAX_PATH] = "";

public:
    const char* GetModName() override { return "Local Gravity"; }
    const char* GetAuthorName() override { return "BookwormKevin"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        for (int i = 0; i < NUM_LEVELS; i++) {
            gravityValues[i] = DEFAULT_GRAVITY;
        }

        /* Build config file path next to the game exe */
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char* lastSlash = strrchr(exePath, '\\');
        if (!lastSlash) lastSlash = strrchr(exePath, '/');
        if (lastSlash) {
            *(lastSlash + 1) = '\0';
            snprintf(configPath, MAX_PATH, "%smkn_plus_local_gravity_set.txt", exePath);
        } else {
            strncpy(configPath, "mkn_plus_local_gravity_set.txt", MAX_PATH - 1);
        }
        configPath[MAX_PATH - 1] = '\0';

        createDefaultConfig();
        loadConfig();

        CustomButton btn("local_gravity_enabled", "Local Gravity");
        btn.defaultState = true;
        api->CreateToggleButton(btn, this);
    }

    void onLevelStart() override {
        loadConfig();
        currentLevelIndex = -1;
    }

    void onSceneEnd() override {
        currentLevelIndex = -1;
    }

    void onBallUpdate(Ball* ball) override {
        if (!ball) return;
        if (!api->GetButtonState("local_gravity_enabled")) return;

        PhysicsObject* phys = ball->physics_object;
        if (!phys) return;

        /* Identify current level via direct memory access */
        if (currentLevelIndex == -1) {
            currentLevelIndex = identifyLevel();
        }
        if (currentLevelIndex < 0 || currentLevelIndex >= NUM_LEVELS) return;

        float gravityValue = gravityValues[currentLevelIndex];

        float gx = phys->gravity_x;
        float gy = phys->gravity_y;
        float gz = phys->gravity_z;

        float absX = fabsf(gx);
        float absY = fabsf(gy);
        float absZ = fabsf(gz);

        /* Determine the sign of the dominant axis */
        float dirX = 0.0f, dirY = 0.0f, dirZ = 0.0f;
        if (absY > 0.001f && absY >= absX && absY >= absZ) {
            dirY = (gy > 0) ? 1.0f : -1.0f;
        } else if (absX > 0.001f && absX >= absZ) {
            dirX = (gx > 0) ? 1.0f : -1.0f;
        } else if (absZ > 0.001f) {
            dirZ = (gz > 0) ? 1.0f : -1.0f;
        } else {
            dirY = -1.0f;
        }

        /* Scale the gravity direction vector by the config value */
        float scale = gravityValue;
        phys->gravity_x = dirX * scale;
        phys->gravity_y = dirY * scale;
        phys->gravity_z = dirZ * scale;
    }

private:
    /* Direct memory access to scene name — bypasses IModAPI vtable */
    int identifyLevel() {
        if (IsBadReadPtr((void*)GLOBAL_SCENE_PTR, 4)) return -1;
        DWORD scene = *(DWORD*)GLOBAL_SCENE_PTR;
        if (!scene || scene < 0x10000) return -1;
        /* Scene+0x868 = name (char*) */
        if (IsBadReadPtr((void*)(scene + 0x868), 4)) return -1;
        const char* name = *(const char**)(scene + 0x868);
        if (!name || IsBadReadPtr(name, 2)) return -1;
        if ((unsigned char)name[0] < 0x20 || (unsigned char)name[0] > 0x7E) return -1;

        for (int i = 0; i < NUM_RACES; i++) {
            if (strcmp(name, RACE_NAMES[i]) == 0) return i;
        }
        for (int i = 0; i < NUM_RACES; i++) {
            if (strcmp(name, ARENA_NAMES[i]) == 0) return NUM_RACES + i;
        }
        return -1;
    }

    void loadConfig() {
        for (int i = 0; i < NUM_LEVELS; i++) {
            gravityValues[i] = DEFAULT_GRAVITY;
        }

        /* Try the primary path first, then CWD as fallback */
        FILE* f = nullptr;
        fopen_s(&f, configPath, "r");
        if (!f) {
            fopen_s(&f, "mkn_plus_local_gravity_set.txt", "r");
        }
        if (!f) {
            createDefaultConfig();
            return;
        }

        char line[256];
        int index = 0;

        while (fgets(line, sizeof(line), f) && index < NUM_LEVELS) {
            /* Skip BOM on first line */
            if (index == 0 && (unsigned char)line[0] == 0xEF) {
                memmove(line, line + 3, strlen(line) - 2);
            }

            /* Skip empty lines and comments */
            char* p = line;
            while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
            if (*p == '\0' || *p == '#') continue;

            float val = 0.0f;
            if (sscanf(p, "%f", &val) == 1) {
                gravityValues[index] = val;
                index++;
            }
        }

        fclose(f);

        for (int i = index; i < NUM_LEVELS; i++) {
            gravityValues[i] = DEFAULT_GRAVITY;
        }
    }

    void createDefaultConfig() {
        FILE* f = nullptr;
        fopen_s(&f, configPath, "w");
        if (!f) return;

        fprintf(f, "# Local Gravity Configuration\n");
        fprintf(f, "# Each line is a float value for gravity on that level/arena.\n");
        fprintf(f, "# 0.5 = normal gravity (game default)\n");
        fprintf(f, "# 0.125 = low gravity (25%% of normal)\n");
        fprintf(f, "# 2.0 = double gravity\n");
        fprintf(f, "# Negative values reverse gravity direction.\n");
        fprintf(f, "# Lines 1-15: Race levels | Lines 16-30: Arena levels\n");
        fprintf(f, "\n");
        fprintf(f, "# --- Races ---\n");
        for (int i = 0; i < NUM_RACES; i++) {
            fprintf(f, "%.1f\n", DEFAULT_GRAVITY);
        }
        fprintf(f, "# --- Arenas ---\n");
        for (int i = 0; i < NUM_RACES; i++) {
            fprintf(f, "%.1f\n", DEFAULT_GRAVITY);
        }

        fclose(f);
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new mkn_plus_local_gravity();
}

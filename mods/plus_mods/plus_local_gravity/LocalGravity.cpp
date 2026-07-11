/*
 * LocalGravity.cpp — Per-Level Gravity Override (HB+ API)
 *
 * Reads gravity values from local_gravity_set.txt (30 lines):
 *   Lines 1-15:  Race levels (Warm-Up through Impossible)
 *   Lines 16-30: Arena levels (Warm-Up through Sky)
 *
 * Each line is a single float (e.g. 0.5 = normal, 0.125 = low, 2.0 = heavy).
 * The file is re-read on every level start, so changes take effect immediately.
 *
 * Based on the gravity_mod approach: writes gravity direction vectors
 * on the physics object + sets spin_rate as gravity scale.
 *
 * Difference from plus_low_gravity: that mod uses a global slider for ALL
 * levels. This mod reads per-level values from a config file, so each
 * level/arena can have its own gravity.
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

static constexpr float NORMAL_GRAVITY = 0.5f;
static constexpr int NUM_LEVELS = 30;

// Scene name tables (must match game's internal scene names)
struct SceneEntry {
    const char* sceneName;
};

static const char* RACE_NAMES[NUM_LEVELS / 2] = {
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

static const char* ARENA_NAMES[NUM_LEVELS / 2] = {
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

class LocalGravityMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    float gravityValues[NUM_LEVELS];  // 15 races + 15 arenas
    int currentLevelIndex = -1;      // -1 = not in a recognized level
    char configPath[MAX_PATH] = "";

    static constexpr float DEFAULT_GRAVITY = 0.5f;

public:
    const char* GetModName() override { return "Local Gravity"; }
    const char* GetAuthorName() override { return "BookwormKevin"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        // Initialize all gravity values to default
        for (int i = 0; i < NUM_LEVELS; i++) {
            gravityValues[i] = DEFAULT_GRAVITY;
        }

        // Build config file path next to the game exe
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash) {
            *(lastSlash + 1) = '\0';
            strncpy(configPath, exePath, MAX_PATH - 1);
            strncat(configPath, "local_gravity_set.txt", MAX_PATH - strlen(configPath) - 1);
        } else {
            strncpy(configPath, "local_gravity_set.txt", MAX_PATH - 1);
        }
        configPath[MAX_PATH - 1] = '\0';

        // Create default config file if it doesn't exist
        createDefaultConfig();

        // Load config on startup
        loadConfig();

        // Register a toggle button so the user can enable/disable
        CustomButton btn("local_gravity_enabled", "Local Gravity");
        btn.defaultState = true;
        api->CreateToggleButton(btn, this);
    }

    void onLevelStart() override {
        // Reload config file on every level start so changes take effect
        loadConfig();
        currentLevelIndex = -1;
    }

    void onSceneEnd() override {
        currentLevelIndex = -1;
    }

    void onBallUpdate(Ball* ball) override {
        if (!ball) return;

        // Check if mod is enabled
        if (!api->GetButtonState("local_gravity_enabled")) return;

        PhysicsObject* phys = ball->physics_object;
        if (!phys) return;

        // Identify current level from scene name
        if (currentLevelIndex == -1) {
            currentLevelIndex = identifyLevel();
        }

        // If we can't identify the level, don't touch gravity
        if (currentLevelIndex < 0 || currentLevelIndex >= NUM_LEVELS) return;

        float gravityValue = gravityValues[currentLevelIndex];

        // Read current gravity direction (set by game's Ball_Set*Gravity functions)
        float gx = phys->gravity_x;
        float gy = phys->gravity_y;
        float gz = phys->gravity_z;

        float absX = fabsf(gx);
        float absY = fabsf(gy);
        float absZ = fabsf(gz);

        // Clear all axes, then set only the dominant one
        phys->gravity_x = 0;
        phys->gravity_y = 0;
        phys->gravity_z = 0;

        if (absY > 0.001f && absY >= absX && absY >= absZ) {
            // Y-axis gravity (normal levels — game uses -Y for down)
            phys->gravity_y = (gravityValue < 0) ? 1.0f : -1.0f;
        } else if (absX > 0.001f && absX >= absZ) {
            // X-axis gravity (Odd Race walls — game uses -X for down)
            phys->gravity_x = (gravityValue < 0) ? 1.0f : -1.0f;
        } else if (absZ > 0.001f) {
            // Z-axis gravity (Odd Race flat — game uses +Z for down)
            phys->gravity_z = (gravityValue < 0) ? -1.0f : 1.0f;
        } else {
            // No gravity set yet, default to Y-down
            phys->gravity_y = (gravityValue < 0) ? 1.0f : -1.0f;
        }

        // spin_rate acts as gravity scale (default 5.0 in game)
        // Using fabsf to keep it positive — negative values don't behave well
        ball->spin_rate = fabsf(gravityValue);
    }

private:
    int identifyLevel() {
        if (!api) return -1;

        Scene* scene = api->GetScene();
        if (!scene) return -1;

        if (!scene->name || IsBadReadPtr(scene->name, 1)) return -1;

        // Check race names (indices 0-14)
        for (int i = 0; i < NUM_LEVELS / 2; i++) {
            if (strcmp(scene->name, RACE_NAMES[i]) == 0) {
                return i;
            }
        }

        // Check arena names (indices 15-29)
        for (int i = 0; i < NUM_LEVELS / 2; i++) {
            if (strcmp(scene->name, ARENA_NAMES[i]) == 0) {
                return NUM_LEVELS / 2 + i;
            }
        }

        return -1;
    }

    void loadConfig() {
        FILE* f = nullptr;
        fopen_s(&f, configPath, "r");
        if (!f) {
            // Config doesn't exist — create it with defaults
            createDefaultConfig();
            return;
        }

        char line[256];
        int index = 0;

        while (fgets(line, sizeof(line), f) && index < NUM_LEVELS) {
            // Skip BOM
            if (index == 0 && (unsigned char)line[0] == 0xEF) {
                memmove(line, line + 3, strlen(line) - 2);
            }

            // Skip empty lines and comments
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

        // Fill any missing entries with default
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
        for (int i = 0; i < NUM_LEVELS / 2; i++) {
            fprintf(f, "%.1f\n", DEFAULT_GRAVITY);
        }
        fprintf(f, "# --- Arenas ---\n");
        for (int i = 0; i < NUM_LEVELS / 2; i++) {
            fprintf(f, "%.1f\n", DEFAULT_GRAVITY);
        }

        fclose(f);
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new LocalGravityMod();
}

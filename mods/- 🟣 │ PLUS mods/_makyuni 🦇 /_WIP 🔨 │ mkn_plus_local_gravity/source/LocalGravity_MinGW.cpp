/*
 * LocalGravity_MinGW.cpp — MinGW cross-compile version of mkn_plus_local_gravity.
 *
 * Simple approach: multiply the game's default spin_rate (gravity scale, ball+0x2A4)
 * by a per-level multiplier read from mkn_plus_local_gravity_set.txt.
 *
 * 1.0 = normal gravity (game default)
 * 0.5 = half gravity
 * 2.0 = double gravity
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.0.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#define NUM_LEVELS 30
#define NUM_RACES 15
#define DEFAULT_MULTIPLIER 1.0f

/* Global pointers for direct memory access */
#define GLOBAL_SCENE_PTR 0x005341E4

/* The gravity multiplier — this is what the config file modifies */
static float mkn_gravity_multiplier = 1.0f;

/* Per-level multiplier values */
static float g_multipliers[NUM_LEVELS];
static int g_currentLevelIndex = -1;
static char g_configPath[MAX_PATH] = "";

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

/* Vtable implementations */
static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Local Gravity"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

/* Direct memory access to scene name */
static const char* getSceneNameDirect(void) {
    if (IsBadReadPtr((void*)GLOBAL_SCENE_PTR, 4)) return NULL;
    DWORD scene = *(DWORD*)GLOBAL_SCENE_PTR;
    if (!scene || scene < 0x10000) return NULL;
    if (IsBadReadPtr((void*)(scene + 0x868), 4)) return NULL;
    const char* name = *(const char**)(scene + 0x868);
    if (!name || IsBadReadPtr((void*)name, 2)) return NULL;
    if ((unsigned char)name[0] < 0x20 || (unsigned char)name[0] > 0x7E) return NULL;
    return name;
}

static int identifyLevel(void) {
    const char* sceneName = getSceneNameDirect();
    if (!sceneName) return -1;

    for (int i = 0; i < NUM_RACES; i++) {
        if (nc_strcmp(sceneName, RACE_NAMES[i]) == 0) return i;
    }
    for (int i = 0; i < NUM_RACES; i++) {
        if (nc_strcmp(sceneName, ARENA_NAMES[i]) == 0) return NUM_RACES + i;
    }
    return -1;
}

static void loadConfig(void) {
    for (int i = 0; i < NUM_LEVELS; i++) g_multipliers[i] = DEFAULT_MULTIPLIER;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        HANDLE h2 = CreateFileA("mkn_plus_local_gravity_set.txt", GENERIC_READ, FILE_SHARE_READ, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h2 == INVALID_HANDLE_VALUE) return;
        h = h2;
    }

    char buf[8192];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    buf[bytesRead] = '\0';

    /* Skip BOM */
    char* p = buf;
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) {
        p += 3;
    }

    int index = 0;
    while (*p && index < NUM_LEVELS) {
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p == '\0') break;
        if (*p == '#') {
            while (*p && *p != '\n') p++;
            continue;
        }
        /* Parse float manually */
        float val = 0.0f;
        int negative = 0;
        if (*p == '-') { negative = 1; p++; }
        else if (*p == '+') { p++; }
        int integerPart = 0;
        while (*p >= '0' && *p <= '9') {
            integerPart = integerPart * 10 + (*p - '0');
            p++;
        }
        float frac = 0.0f;
        if (*p == '.') {
            p++;
            float div = 10.0f;
            while (*p >= '0' && *p <= '9') {
                frac += (*p - '0') / div;
                div *= 10.0f;
                p++;
            }
        }
        val = (float)integerPart + frac;
        if (negative) val = -val;
        while (*p && *p != '\n') p++;

        g_multipliers[index] = val;
        index++;
    }
}

static void createDefaultConfig(void) {
    HANDLE h = CreateFileA(g_configPath, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    const char* header =
        "# Local Gravity Configuration\r\n"
        "# Multiplier for gravity on each level/arena.\r\n"
        "# 1.0 = normal gravity (game default)\r\n"
        "# 0.5 = half gravity\r\n"
        "# 2.0 = double gravity\r\n"
        "# Lines 1-15: Race levels | Lines 16-30: Arena levels\r\n"
        "\r\n"
        "# --- Races ---\r\n";

    DWORD written;
    WriteFile(h, header, (DWORD)nc_strlen(header), &written, NULL);

    char lineBuf[32];
    for (int i = 0; i < NUM_RACES; i++) {
        nc_snprintf(lineBuf, sizeof(lineBuf), "%d.%d\r\n", (int)DEFAULT_MULTIPLIER,
                    (int)((DEFAULT_MULTIPLIER - (int)DEFAULT_MULTIPLIER) * 10.0f + 0.5f));
        WriteFile(h, lineBuf, (DWORD)nc_strlen(lineBuf), &written, NULL);
    }

    const char* arenaHeader = "# --- Arenas ---\r\n";
    WriteFile(h, arenaHeader, (DWORD)nc_strlen(arenaHeader), &written, NULL);

    for (int i = 0; i < NUM_RACES; i++) {
        nc_snprintf(lineBuf, sizeof(lineBuf), "%d.%d\r\n", (int)DEFAULT_MULTIPLIER,
                    (int)((DEFAULT_MULTIPLIER - (int)DEFAULT_MULTIPLIER) * 10.0f + 0.5f));
        WriteFile(h, lineBuf, (DWORD)nc_strlen(lineBuf), &written, NULL);
    }

    CloseHandle(h);
}

static void buildConfigPath(void) {
    char exePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (len > 0) {
        char* last = NULL;
        char* p = exePath;
        while (*p) {
            if (*p == '\\' || *p == '/') last = p;
            p++;
        }
        if (last) {
            *(last + 1) = '\0';
            nc_strncpy(g_configPath, exePath, MAX_PATH - 1);
            nc_strncpy(g_configPath + nc_strlen(g_configPath),
                       "mkn_plus_local_gravity_set.txt", MAX_PATH - nc_strlen(g_configPath) - 1);
            g_configPath[MAX_PATH - 1] = '\0';
            return;
        }
    }
    nc_strncpy(g_configPath, "mkn_plus_local_gravity_set.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
}

static void* g_storedApi = NULL;

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;

    for (int i = 0; i < NUM_LEVELS; i++) g_multipliers[i] = DEFAULT_MULTIPLIER;

    buildConfigPath();

    DWORD attr = GetFileAttributesA(g_configPath);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        createDefaultConfig();
    }

    loadConfig();

    /* Register toggle button */
    HBPlusAPI hb = { modApi };
    CustomButton btn;
    btn.id = "local_gravity_enabled";
    btn.displayText = "Local Gravity";
    btn.defaultState = true;
    btn.trueText = "YES";
    btn.falseText = "NO";
    btn.color = Color();
    hb.CreateToggleButton(btn, thisptr);
}

static void __thiscall ball_update_impl(void* thisptr, void* ball) {
    if (!ball) return;

    Ball* b = (Ball*)ball;
    PhysicsObject* phys = b->physics_object;
    if (!phys) return;

    /* Check toggle state via API every frame (don't rely on cached state) */
    if (g_storedApi) {
        HBPlusAPI hb = { g_storedApi };
        if (!hb.GetButtonState("local_gravity_enabled")) return;
    }

    /* Identify level if not cached */
    if (g_currentLevelIndex == -1) {
        g_currentLevelIndex = identifyLevel();
    }
    if (g_currentLevelIndex < 0 || g_currentLevelIndex >= NUM_LEVELS) return;

    /* Set the gravity multiplier from config */
    mkn_gravity_multiplier = g_multipliers[g_currentLevelIndex];

    /* Read current gravity direction (set by game each frame)
       Game uses unit vectors: (0,-1,0) normal, (-1,0,0) tilted, (0,0,1) flat */
    float gx = phys->gravity_x;
    float gy = phys->gravity_y;
    float gz = phys->gravity_z;

    float absX = gx < 0 ? -gx : gx;
    float absY = gy < 0 ? -gy : gy;
    float absZ = gz < 0 ? -gz : gz;

    /* Clear all axes, then set only the dominant one (same as plus_low_gravity) */
    phys->gravity_x = 0;
    phys->gravity_y = 0;
    phys->gravity_z = 0;

    if (absY > 0.001f && absY >= absX && absY >= absZ) {
        phys->gravity_y = (mkn_gravity_multiplier < 0) ? 1.0f : -1.0f;
    } else if (absX > 0.001f && absX >= absZ) {
        phys->gravity_x = (mkn_gravity_multiplier < 0) ? 1.0f : -1.0f;
    } else if (absZ > 0.001f) {
        phys->gravity_z = (mkn_gravity_multiplier < 0) ? -1.0f : 1.0f;
    } else {
        phys->gravity_y = (mkn_gravity_multiplier < 0) ? 1.0f : -1.0f;
    }

    /* Multiply the game's default spin_rate (gravity scale) by our multiplier.
       Default spin_rate is 5.0. So multiplier 1.0 = 5.0, 0.5 = 2.5, 2.0 = 10.0 */
    b->gravity_magnitude = 5.0f * mkn_gravity_multiplier;
    if (mkn_gravity_multiplier < 0) {
        b->gravity_magnitude = 5.0f * (-mkn_gravity_multiplier);
    }
}

static void __thiscall button_toggle_impl(void* thisptr, const char* id, bool state) {
    /* Handled via GetButtonState in ball_update_impl */
}

static void __thiscall level_start_impl(void* thisptr) {
    loadConfig();
    g_currentLevelIndex = -1;
}

static void __thiscall scene_end_impl(void* thisptr) {
    g_currentLevelIndex = -1;
}

/* No-op implementations for unused callbacks */
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall game_update_impl(void*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}

/*
 * 17-entry vtable matching MSVC ABI for HB+ v2.0.
 */
static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10] NEW in v2.0
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

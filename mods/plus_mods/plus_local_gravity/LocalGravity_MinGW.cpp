/*
 * LocalGravity_MinGW.cpp — MinGW cross-compile version of LocalGravity mod.
 *
 * Uses manual 16-entry vtable to match MSVC ABI.
 * Uses nocrt functions (no msvcrt.dll dependency).
 * Uses hbplus_api.h wrapper for IModAPI vtable dispatch.
 *
 * Compile: see build.sh
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#define NUM_LEVELS 30
#define NUM_RACES 15
#define DEFAULT_GRAVITY 0.5f

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

/* State */
static float g_gravityValues[NUM_LEVELS];
static int g_currentLevelIndex = -1;
static char g_configPath[MAX_PATH] = "";
static bool g_enabled = true;

/* Function pointer typedefs for vtable */
typedef void* (__thiscall *dtor_t)(void* thisptr, int flags);
typedef const char* (__thiscall *get_name_t)(void*);
typedef int (__thiscall *get_int_t)(void*);
typedef void (__thiscall *init_t)(void* thisptr, void* modApi);
typedef void (__thiscall *ball_update_t)(void* thisptr, void* ball);
typedef void (__thiscall *button_toggle_t)(void* thisptr, const char* id, bool state);
typedef void (__thiscall *level_start_t)(void* thisptr);
typedef void (__thiscall *scene_end_t)(void* thisptr);

/* Vtable implementations */
static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Local Gravity"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static int identifyLevel(void* api) {
    if (!api) return -1;
    HBPlusAPI hb = { api };
    Scene* scene = hb.GetScene();
    if (!scene) return -1;
    if (!scene->name || IsBadReadPtr(scene->name, 1)) return -1;

    for (int i = 0; i < NUM_RACES; i++) {
        if (nc_strcmp(scene->name, RACE_NAMES[i]) == 0) return i;
    }
    for (int i = 0; i < NUM_RACES; i++) {
        if (nc_strcmp(scene->name, ARENA_NAMES[i]) == 0) return NUM_RACES + i;
    }
    return -1;
}

static void loadConfig(void) {
    for (int i = 0; i < NUM_LEVELS; i++) g_gravityValues[i] = DEFAULT_GRAVITY;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

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
        /* Skip whitespace and newlines */
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p == '\0') break;
        /* Skip comment lines */
        if (*p == '#') {
            while (*p && *p != '\n') p++;
            continue;
        }
        /* Parse float manually (no sscanf in nocrt) */
        float val = 0.0f;
        int negative = 0;
        if (*p == '-') { negative = 1; p++; }
        else if (*p == '+') { p++; }
        int integerPart = 0;
        int hasInteger = 0;
        while (*p >= '0' && *p <= '9') {
            integerPart = integerPart * 10 + (*p - '0');
            hasInteger = 1;
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
        /* Skip rest of line */
        while (*p && *p != '\n') p++;

        g_gravityValues[index] = val;
        index++;
    }
}

static void createDefaultConfig(void) {
    HANDLE h = CreateFileA(g_configPath, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    const char* header =
        "# Local Gravity Configuration\r\n"
        "# Each line is a float value for gravity on that level/arena.\r\n"
        "# 0.5 = normal gravity (game default)\r\n"
        "# 0.125 = low gravity (25% of normal)\r\n"
        "# 2.0 = double gravity\r\n"
        "# Negative values reverse gravity direction.\r\n"
        "# Lines 1-15: Race levels | Lines 16-30: Arena levels\r\n"
        "\r\n"
        "# --- Races ---\r\n";

    DWORD written;
    WriteFile(h, header, (DWORD)nc_strlen(header), &written, NULL);

    char lineBuf[32];
    for (int i = 0; i < NUM_RACES; i++) {
        /* Manual float-to-string: 0.5 -> "0.5\r\n" */
        int intPart = (int)DEFAULT_GRAVITY;
        int fracPart = (int)((DEFAULT_GRAVITY - intPart) * 10.0f + 0.5f);
        nc_snprintf(lineBuf, sizeof(lineBuf), "%d.%d\r\n", intPart, fracPart);
        WriteFile(h, lineBuf, (DWORD)nc_strlen(lineBuf), &written, NULL);
    }

    const char* arenaHeader = "# --- Arenas ---\r\n";
    WriteFile(h, arenaHeader, (DWORD)nc_strlen(arenaHeader), &written, NULL);

    for (int i = 0; i < NUM_RACES; i++) {
        nc_snprintf(lineBuf, sizeof(lineBuf), "%d.%d\r\n", (int)DEFAULT_GRAVITY,
                    (int)((DEFAULT_GRAVITY - (int)DEFAULT_GRAVITY) * 10.0f + 0.5f));
        WriteFile(h, lineBuf, (DWORD)nc_strlen(lineBuf), &written, NULL);
    }

    CloseHandle(h);
}

static void __thiscall init_impl(void* thisptr, void* modApi) {
    /* Store api pointer in object */
    *(void**)((char*)thisptr + 4) = modApi;

    /* Initialize gravity values */
    for (int i = 0; i < NUM_LEVELS; i++) g_gravityValues[i] = DEFAULT_GRAVITY;

    /* Build config path next to game exe */
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char* lastSlash = nc_strncpy(g_configPath, exePath, MAX_PATH);
    /* Find last backslash */
    char* bs = g_configPath;
    char* last = NULL;
    while (*bs) {
        if (*bs == '\\') last = bs;
        bs++;
    }
    if (last) {
        *(last + 1) = '\0';
        nc_strncpy(g_configPath + nc_strlen(g_configPath), "local_gravity_set.txt",
                   MAX_PATH - nc_strlen(g_configPath) - 1);
    } else {
        nc_strncpy(g_configPath, "local_gravity_set.txt", MAX_PATH - 1);
    }
    g_configPath[MAX_PATH - 1] = '\0';

    /* Create default config if it doesn't exist */
    DWORD attr = GetFileAttributesA(g_configPath);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        createDefaultConfig();
    }

    /* Load config */
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
    if (!g_enabled) return;

    void* api = *(void**)((char*)thisptr + 4);
    if (!api) return;

    HBPlusAPI hb = { api };

    /* Identify level if not cached */
    if (g_currentLevelIndex == -1) {
        g_currentLevelIndex = identifyLevel(api);
    }
    if (g_currentLevelIndex < 0 || g_currentLevelIndex >= NUM_LEVELS) return;

    float gravityValue = g_gravityValues[g_currentLevelIndex];

    /* Get physics object from ball */
    Ball* b = (Ball*)ball;
    PhysicsObject* phys = b->physics_object;
    if (!phys) return;

    /* Read current gravity direction */
    float gx = phys->gravity_x;
    float gy = phys->gravity_y;
    float gz = phys->gravity_z;

    float absX = gx < 0 ? -gx : gx;
    float absY = gy < 0 ? -gy : gy;
    float absZ = gz < 0 ? -gz : gz;

    /* Clear all axes */
    phys->gravity_x = 0;
    phys->gravity_y = 0;
    phys->gravity_z = 0;

    /* Set only the dominant axis */
    if (absY > 0.001f && absY >= absX && absY >= absZ) {
        phys->gravity_y = (gravityValue < 0) ? 1.0f : -1.0f;
    } else if (absX > 0.001f && absX >= absZ) {
        phys->gravity_x = (gravityValue < 0) ? 1.0f : -1.0f;
    } else if (absZ > 0.001f) {
        phys->gravity_z = (gravityValue < 0) ? -1.0f : 1.0f;
    } else {
        phys->gravity_y = (gravityValue < 0) ? 1.0f : -1.0f;
    }

    /* spin_rate = gravity scale (default 5.0 in game) */
    b->spin_rate = (gravityValue < 0) ? -gravityValue : gravityValue;
}

static void __thiscall button_toggle_impl(void* thisptr, const char* id, bool state) {
    if (id && nc_strcmp(id, "local_gravity_enabled") == 0) {
        g_enabled = state;
    }
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
static void __thiscall game_update_impl(void*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}

/* 16-entry vtable matching MSVC layout */
static void* g_vtable[16] = {
    (void*)sc_dtor,              // [0]  scalar deleting destructor
    (void*)get_mod_name,         // [1]  GetModName
    (void*)get_author,           // [2]  GetAuthorName
    (void*)get_version,          // [3]  GetApiVersion
    (void*)get_contributors,     // [4]  GetContributors
    (void*)init_impl,            // [5]  Initialize
    (void*)ball_update_impl,     // [6]  onBallUpdate
    (void*)render_apply_impl,    // [7]  onRenderApply
    (void*)button_toggle_impl,   // [8]  onButtonToggle
    (void*)slider_change_impl,   // [9]  onSliderChange
    (void*)game_update_impl,     // [10] onGameUpdate
    (void*)event_collide_impl,   // [11] onEventPlaneCollide
    (void*)text_render_impl,     // [12] onTextRenderLoop
    (void*)ball_bump_impl,       // [13] onBallBump
    (void*)scene_end_impl,       // [14] onSceneEnd
    (void*)level_start_impl,     // [15] onLevelStart
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);  /* 8 bytes: vtable ptr + IModAPI* member */
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;  /* api = nullptr */
    return (HamsterballAPI*)obj;
}

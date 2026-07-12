/*
 * LocalGravity_MinGW.cpp — Port of working LowGravity mod.
 * Reads gravity value from mkn_plus_local_gravity_set.txt instead of a slider.
 * Config file is next to the DLL in the Mods\ folder.
 * Re-reads every onBallUpdate for live editing.
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.0.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"

static float g_gravityValue = 5.0f;  /* default matches game default spin_rate */
static char g_configPath[MAX_PATH] = "";
static bool g_pathReady = false;

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Local Gravity"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

/* Build config path relative to THIS DLL's folder */
static void buildConfigPath(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)sc_dtor, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) {
                if (*p == '\\' || *p == '/') last = p;
                p++;
            }
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_configPath, dllPath, MAX_PATH - 1);
                nc_strncpy(g_configPath + nc_strlen(g_configPath),
                           "mkn_plus_local_gravity_set.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "mkn_plus_local_gravity_set.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_pathReady = true;
}

/* Read a single float from the config file */
static void reloadConfig(void) {
    if (!g_pathReady) return;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[256];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    buf[bytesRead] = '\0';

    /* Skip BOM */
    char* p = buf;
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) {
        p += 3;
    }

    /* Skip whitespace and comments */
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p == '#') {
        while (*p && *p != '\n') p++;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
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

    /* Only update if we parsed something meaningful */
    if (bytesRead > 0) {
        g_gravityValue = val;
    }
}

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    buildConfigPath();
    reloadConfig();
}

static void __thiscall ball_update_impl(void* thisptr, void* ball) {
    if (!ball) return;

    /* Re-read config every frame for live editing */
    reloadConfig();

    PhysicsObject* phys = ((Ball*)ball)->physics_object;
    if (!phys) return;

    float slider = g_gravityValue;

    /* Read current gravity direction (set by game's Ball_Set*Gravity functions)
       Game uses 3 unit vectors: (0,-1,0) normal, (-1,0,0) tilted, (0,0,1) flat */
    float gx = phys->gravity_x;
    float gy = phys->gravity_y;
    float gz = phys->gravity_z;

    float absX = gx < 0 ? -gx : gx;
    float absY = gy < 0 ? -gy : gy;
    float absZ = gz < 0 ? -gz : gz;

    /* Clear all axes, then set only the dominant one */
    phys->gravity_x = 0;
    phys->gravity_y = 0;
    phys->gravity_z = 0;

    if (absY > 0.001f && absY >= absX && absY >= absZ) {
        phys->gravity_y = (slider < 0) ? 1.0f : -1.0f;
    } else if (absX > 0.001f && absX >= absZ) {
        phys->gravity_x = (slider < 0) ? 1.0f : -1.0f;
    } else if (absZ > 0.001f) {
        phys->gravity_z = (slider < 0) ? -1.0f : 1.0f;
    } else {
        phys->gravity_y = (slider < 0) ? 1.0f : -1.0f;
    }

    /* This doesn't behave well with negative values,
       but it works much better with large values than the physics object gravity.
       The default of it is 5. */
    ((Ball*)ball)->gravity_magnitude = (slider < 0) ? -slider : slider;
}

/* No-op implementations for unused callbacks */
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall game_update_impl(void*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

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
    (void*)cycle_change_impl,    // [10]
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

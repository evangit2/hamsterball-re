/*
 * PortalMod_MinGW.cpp - MinGW cross-compile version of Portal Mod.
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 *
 * Portal system: E:PORTAL(N) event plane triggers teleport to PORTALPOS(N)
 * with optional PORTALVEC(N) for exit direction. Preserves incoming speed.
 * Plays Zip sound at destination.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"
#include <math.h>

static inline float portal_sqrtf(float x) {
    float result;
    __asm__ __volatile__("sqrtss %1, %0" : "=x"(result) : "x"(x));
    return result;
}

/* Offsets verified via Ghidra decompilation of OddBoard_CollisionHandler (0x0040ED30)
 * and ToobBoard E:BRANCH handler (0x00410020). */
static constexpr DWORD BALL_TELEPORT_FLAG = 0xC3C;  // BYTE, set to 1
static constexpr DWORD BALL_TELEPORT_X    = 0xC40;
static constexpr DWORD BALL_TELEPORT_Y    = 0xC44;
static constexpr DWORD BALL_TELEPORT_Z    = 0xC48;
static constexpr DWORD BALL_PHYS_PTR      = 0x1A4;  // physics_object
static constexpr DWORD PHYS_VEL_X         = 0xCA4;
static constexpr DWORD PHYS_VEL_Y         = 0xCA8;
static constexpr DWORD PHYS_VEL_Z         = 0xCAC;
static constexpr DWORD BALL_RADIUS         = 0x284;
static constexpr DWORD BALL_PLAYER_ID      = 0x018;

/* Scene offsets */
static constexpr DWORD SCENE_LEVEL_PTR     = 0x8AC;
static constexpr DWORD LEVEL_SCENEOBJ      = 0x480;
static constexpr DWORD SCENEOBJ_COUNT       = 0x898;
static constexpr DWORD SCENEOBJ_ARRAY       = 0xCA0;

/* Ref point entry: [0]=name ptr, [4]=x(float), [8]=y(float), [0xC]=z(float) */
static constexpr DWORD REF_NAME             = 0x00;
static constexpr DWORD REF_X                = 0x04;
static constexpr DWORD REF_Y                = 0x08;
static constexpr DWORD REF_Z                = 0x0C;

static void* g_storedApi = NULL;
static int g_cooldowns[4] = {0, 0, 0, 0};

/* ---- nocrt helpers for portal mod ---- */

static int portal_strnicmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; i++) {
        char ca = a[i], cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        if (ca == 0) return 0;
    }
    return 0;
}

static int portal_stricmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static int portal_atoi(const char* s) {
    int result = 0;
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result;
}

static int portal_snprintf(char* buf, int size, const char* fmt, ...) {
    /* Simple snprintf for "PORTALPOS(%d)" pattern only */
    va_list args;
    va_start(args, fmt);
    /* Use wvsprintfA from user32 */
    typedef int (WINAPI *wvsprintfA_t)(LPSTR, LPCSTR, va_list);
    static wvsprintfA_t pWvsprintfA = NULL;
    if (!pWvsprintfA) {
        HMODULE h = GetModuleHandleA("user32.dll");
        if (h) pWvsprintfA = (wvsprintfA_t)GetProcAddress(h, "wvsprintfA");
    }
    if (!pWvsprintfA) { buf[0] = 0; va_end(args); return 0; }
    int result = pWvsprintfA(buf, fmt, args);
    va_end(args);
    if (result >= size) { buf[size-1] = 0; result = size - 1; }
    return result;
}

/* ---- Ref point lookup ---- */

static int lookupRefPoint(DWORD levelPtr, const char* name, float* outX, float* outY, float* outZ) {
    if (IsBadReadPtr((void*)(levelPtr + LEVEL_SCENEOBJ), 4)) return 0;
    DWORD sceneobj = *(DWORD*)(levelPtr + LEVEL_SCENEOBJ);
    if (!sceneobj || sceneobj < 0x10000) return 0;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_COUNT), 4)) return 0;
    int count = *(int*)(sceneobj + SCENEOBJ_COUNT);
    if (count <= 0 || count > 10000) return 0;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_ARRAY), 4)) return 0;
    DWORD arrayBase = *(DWORD*)(sceneobj + SCENEOBJ_ARRAY);
    if (!arrayBase || arrayBase < 0x10000) return 0;

    DWORD* entries = (DWORD*)arrayBase;
    if (!entries || IsBadReadPtr(entries, count * 4)) return 0;

    for (int i = 0; i < count; i++) {
        DWORD entry = entries[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        char* entryName = *(char**)(entry + REF_NAME);
        if (!entryName || IsBadReadPtr(entryName, 64)) continue;

        if (portal_stricmp(entryName, name) == 0) {
            *outX = *(float*)(entry + REF_X);
            *outY = *(float*)(entry + REF_Y);
            *outZ = *(float*)(entry + REF_Z);
            return 1;
        }
    }
    return 0;
}

/* ---- Vtable functions ---- */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Portal Mod"; }
static const char* __thiscall get_author(void*) { return "RodentRacer"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
}

static void __thiscall ball_update_impl(void* thisptr, void* ballPtr) {
    if (!ballPtr) return;
    char* ball = (char*)ballPtr;
    int playerID = *(int*)(ball + BALL_PLAYER_ID);
    if (playerID < 0 || playerID >= 4) return;
    if (g_cooldowns[playerID] > 0)
        g_cooldowns[playerID]--;
}

static void __thiscall event_collide_impl(void* thisptr, void* ballPtr, char* eventPlaneID) {
    if (!ballPtr || !eventPlaneID) return;

    char* ball = (char*)ballPtr;
    int playerID = *(int*)(ball + BALL_PLAYER_ID);
    if (playerID < 0 || playerID >= 4) return;
    if (g_cooldowns[playerID] > 0) return;

    /* Parse E:PORTAL(N) */
    if (portal_strnicmp(eventPlaneID, "E:PORTAL(", 9) != 0) return;

    int portalNum = portal_atoi(eventPlaneID + 9);
    if (portalNum < 1) return;

    /* Get scene -> level ptr */
    HBPlusAPI hb = { g_storedApi };
    Scene* scene = hb.GetScene();
    if (!scene) return;

    DWORD sceneAddr = (DWORD)scene;
    if (IsBadReadPtr((void*)(sceneAddr + SCENE_LEVEL_PTR), 4)) return;
    DWORD levelPtr = *(DWORD*)(sceneAddr + SCENE_LEVEL_PTR);
    if (!levelPtr || levelPtr < 0x10000) return;

    /* Look up PORTALPOS(N) */
    char posName[64];
    portal_snprintf(posName, sizeof(posName), "PORTALPOS(%d)", portalNum);
    float destX, destY, destZ;
    if (!lookupRefPoint(levelPtr, posName, &destX, &destY, &destZ)) return;

    /* Look up PORTALVEC(N) for exit direction */
    char vecName[64];
    portal_snprintf(vecName, sizeof(vecName), "PORTALVEC(%d)", portalNum);
    float vecX = 0, vecY = 0, vecZ = 0;
    int hasVec = lookupRefPoint(levelPtr, vecName, &vecX, &vecY, &vecZ);

    /* Read current velocity to preserve speed magnitude */
    DWORD physPtr = *(DWORD*)(ball + BALL_PHYS_PTR);
    float velX = 0, velY = 0, velZ = 0;
    float currentSpeed = 0;

    if (physPtr && !IsBadReadPtr((void*)physPtr, 0xCB0)) {
        velX = *(float*)(physPtr + PHYS_VEL_X);
        velY = *(float*)(physPtr + PHYS_VEL_Y);
        velZ = *(float*)(physPtr + PHYS_VEL_Z);
        currentSpeed = portal_sqrtf(velX * velX + velY * velY + velZ * velZ);
    }

    /* Use native teleport flag system */
    *(BYTE*)(ball + BALL_TELEPORT_FLAG) = 1;
    *(float*)(ball + BALL_TELEPORT_X) = destX;
    *(float*)(ball + BALL_TELEPORT_Y) = destY + *(float*)(ball + BALL_RADIUS) + 1.0f;
    *(float*)(ball + BALL_TELEPORT_Z) = destZ;

    /* Set exit velocity */
    if (physPtr && !IsBadReadPtr((void*)physPtr, 0xCB0)) {
        if (hasVec) {
            /* Direction = PORTALVEC - PORTALPOS, normalized */
            float dx = vecX - destX;
            float dy = vecY - destY;
            float dz = vecZ - destZ;
            float len = portal_sqrtf(dx * dx + dy * dy + dz * dz);

            if (len > 0.001f) {
                float scale = currentSpeed / len;
                *(float*)(physPtr + PHYS_VEL_X) = dx * scale;
                *(float*)(physPtr + PHYS_VEL_Y) = dy * scale;
                *(float*)(physPtr + PHYS_VEL_Z) = dz * scale;
            }
        } else {
            /* No PORTALVEC - keep input direction, preserve speed */
            *(float*)(physPtr + PHYS_VEL_X) = velX;
            *(float*)(physPtr + PHYS_VEL_Y) = velY;
            *(float*)(physPtr + PHYS_VEL_Z) = velZ;
        }
    }

    /* Play zip sound at destination */
    App* app = hb.GetApp();
    if (app) {
        hb.Play3dSoundEffect(app->sounds.zip, Vec3(destX, destY, destZ), 1.0f);
    }

    g_cooldowns[playerID] = 30;
}

/* No-op implementations for unused callbacks */
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall game_update_impl(void*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

/* Manual 17-entry vtable (HB+ v2.0/v2.1) */
static void* g_vtable[17] = {
    (void*)sc_dtor,              /* [0]  ~HamsterballAPI */
    (void*)get_mod_name,         /* [1]  GetModName */
    (void*)get_author,           /* [2]  GetAuthorName */
    (void*)get_version,          /* [3]  GetApiVersion */
    (void*)get_contributors,     /* [4]  GetContributors */
    (void*)init_impl,            /* [5]  Initialize */
    (void*)ball_update_impl,     /* [6]  onBallUpdate */
    (void*)render_apply_impl,    /* [7]  onRenderApply */
    (void*)button_toggle_impl,   /* [8]  onButtonToggle */
    (void*)slider_change_impl,    /* [9]  onSliderChange */
    (void*)cycle_change_impl,    /* [10] onCycleOptionChange */
    (void*)game_update_impl,     /* [11] onGameUpdate */
    (void*)event_collide_impl,   /* [12] onEventPlaneCollide */
    (void*)text_render_impl,     /* [13] onTextRenderLoop */
    (void*)ball_bump_impl,       /* [14] onBallBump */
    (void*)scene_end_impl,       /* [15] onSceneEnd */
    (void*)level_start_impl,     /* [16] onLevelStart */
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
